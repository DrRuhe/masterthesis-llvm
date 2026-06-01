#include "JITPipelineIPSCCP.h"
#include "InvariantLoadToConstant.h"
#include "JitSCCP/JitIPSCCPPass.h"
#include "StaticMutabilityAnalysis.h"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Transforms/IPO/FunctionAttrs.h"
#include "llvm/Transforms/IPO/GlobalDCE.h"
#include "llvm/Transforms/IPO/GlobalOpt.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Support/ModRef.h"
#include "llvm/Transforms/Scalar/DCE.h"
#include "llvm/Transforms/Scalar/SROA.h"
#include "llvm/Passes/PassBuilder.h"

namespace clangRuntimeSpecializer {

llvm::Error runIPSCCPPipeline(PipelineRunArgs& Args) {
  llvm::Module& M = Args.Mod;
  const ClangRuntimeSpecializer::Options& Opts = Args.Opts;
  llvm::ModuleAnalysisManager& MAM = Args.MAM;
  std::string& CurrentGroup = Args.CurrentGroup;

  // ---- Phase 1: linkage scrub ------------------------------------------------
  // BFS from specialized_wrapper_* callees: convert AvailableExternalLinkage →
  // InternalLinkage so the JIT compiles their bodies. Strip AlwaysInline from
  // all functions and call sites so JitIPSCCPPass drives specialization (not
  // the AlwaysInliner pre-pass). Same rationale as Pipeline 1 scrub.
  llvm::SmallPtrSet<llvm::Function*, 32> ToConvert;
  for (auto& F : M) {
    if (F.isDeclaration() || !F.getName().starts_with("specialized_wrapper_"))
      continue;
    llvm::SmallVector<llvm::Function*, 32> Worklist;
    for (auto& BB : F)
      for (auto& I : BB)
        if (auto* CB = llvm::dyn_cast<llvm::CallBase>(&I))
          if (auto* Callee = CB->getCalledFunction())
            if (!Callee->isDeclaration())
              Worklist.push_back(Callee);
    while (!Worklist.empty()) {
      llvm::Function* Cur = Worklist.pop_back_val();
      if (!ToConvert.insert(Cur).second) continue;
      for (auto& BB : *Cur)
        for (auto& I : BB)
          if (auto* CB = llvm::dyn_cast<llvm::CallBase>(&I))
            if (auto* Callee = CB->getCalledFunction())
              if (!Callee->isDeclaration() && !ToConvert.count(Callee))
                Worklist.push_back(Callee);
    }
  }
  for (auto& F : M) {
    if (F.isDeclaration()) continue;
    if (ToConvert.count(&F) && F.hasAvailableExternallyLinkage())
      F.setLinkage(llvm::GlobalValue::InternalLinkage);
    F.removeFnAttr(llvm::Attribute::AlwaysInline);
  }
  for (auto& F : M)
    for (auto& BB : F)
      for (auto& I : BB)
        if (auto* CB = llvm::dyn_cast<llvm::CallBase>(&I))
          CB->removeFnAttr(llvm::Attribute::AlwaysInline);

  // ---- Phase 2: early GlobalDCE + JitIPSCCP + post-DCE -----------------------
  CurrentGroup = "initial";
  {
    llvm::ModulePassManager MPM;
    MPM.addPass(llvm::GlobalDCEPass());
    // SROA promotes stack-allocated vtable pointers to SSA values before the
    // solver runs, so the solver sees vtable pointers as direct constants rather
    // than loads from alloca addresses it cannot resolve at JIT compile time.
    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(
        llvm::SROAPass(llvm::SROAOptions::ModifyCFG)));
    // Annotate invariant loads before the IPSCCP solver runs.
    // StaticMutabilityAnalysis marks loads from fields that are never written to
    // in the IR as !invariant.load.  JitIPSCCPPass then folds those annotated
    // loads from host memory — both for direct inttoptr constants (path A) and
    // for pointers propagated as constants across callee boundaries (path B).
    // The !invariant.load gate on both paths ensures only provably-non-mutated
    // fields are folded; fields the function also stores to remain unfoldable.
    // Run unconditionally (including large modules): this is a cheap annotation
    // pass, and skipping it would leave loads un-annotated so both paths are
    // blocked, which can cause P2FuncSpec to produce broken specializations.
    {
      llvm::FunctionPassManager PreFPM;
      PreFPM.addPass(StaticMutabilityAnalysis::StaticMutabilityAnalysisPass());
      MPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(PreFPM)));
    }
    MPM.addPass(JitIPSCCPPass(Opts.P2FuncSpec));
    MPM.addPass(llvm::GlobalDCEPass());
    // DCE removes dead instructions left after IPSCCP constant materialisation
    // (e.g. loads whose results were replaced with constants in the lattice but
    // the dead instruction itself was not erased by the solver).
    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(llvm::DCEPass()));
    // GlobalOpt optimises trivial internal functions (constant-return
    // elimination, calling-convention narrowing, dead-arg removal).
    MPM.addPass(llvm::GlobalOptPass());
    MPM.run(M, MAM);

    // After DCE + GlobalOpt, mark internal functions with no memory-accessing
    // instructions as memory(none) so that InstCombine can eliminate dead calls
    // to them in callers.  ReversePostOrderFunctionAttrsPass does not reliably
    // infer this for leaf functions in the JIT CGSCC context, so we set it
    // directly.
    for (auto& F : M) {
      if (F.isDeclaration() || !F.hasInternalLinkage())
        continue;
      bool HasMemAccess = false;
      for (auto& BB : F)
        for (auto& I : BB)
          if (llvm::isa<llvm::LoadInst>(I) || llvm::isa<llvm::StoreInst>(I) ||
              llvm::isa<llvm::AtomicRMWInst>(I) ||
              llvm::isa<llvm::AtomicCmpXchgInst>(I) ||
              (llvm::isa<llvm::CallBase>(I) &&
               !llvm::cast<llvm::CallBase>(I).doesNotAccessMemory()))
            HasMemAccess = true;
      if (!HasMemAccess) {
        F.setMemoryEffects(llvm::MemoryEffects::none());
        F.setWillReturn();
        F.setDoesNotThrow();
      }
    }
  }

  // Phase 2b: eliminate dead calls to the now-memory(none) callees.
  {
    llvm::ModulePassManager MPM;
    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(llvm::InstCombinePass()));
    MPM.addPass(llvm::GlobalDCEPass());
    MPM.run(M, MAM);
  }

  // ---- Phase 3: cleanup before optional O3 -----------------------------------
  // No explicit AlwaysInliner here: O3's cost-model inliner handles all
  // inlining, using the constant-argument bonus to inline SCCP-specialised
  // targets even when they are large.
  CurrentGroup = "final";
  {
    llvm::ModulePassManager MPM;
    MPM.addPass(llvm::GlobalDCEPass());

    // Fold any remaining !invariant.load values to IR immediates so O3 sees
    // literal constants rather than pointer-chain loads when it inlines targets.
    // Gated on !LargeModule to match spec-005 safety rule.
    if (!Args.IsLargeModule) {
      llvm::FunctionPassManager FPM;
      FPM.addPass(StaticMutabilityAnalysis::StaticMutabilityAnalysisPass());
      FPM.addPass(InvariantLoadToConstantPass());
      FPM.addPass(llvm::InstCombinePass());
      FPM.addPass(llvm::SimplifyCFGPass());
      MPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(FPM)));
    }
    MPM.addPass(llvm::ReversePostOrderFunctionAttrsPass());
    MPM.addPass(llvm::GlobalDCEPass());

    MPM.run(M, MAM);
  }

  // ---- Phase 4: O3 final (optional) -----------------------------------------
  // O3 inlines targets based on cost model (constant-arg bonus means SCCP-
  // specialised callees are aggressively inlined), then vectorises and unrolls
  // the merged body. Gate matches P0's EnableO3Final convention.
  if (Opts.EnableO3Final) {
    CurrentGroup = "final_o3";
    llvm::ModulePassManager FinalMPM =
        Args.PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);
    FinalMPM.run(M, MAM);
  }

  return llvm::Error::success();
}

} // namespace clangRuntimeSpecializer
