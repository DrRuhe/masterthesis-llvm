#include "JITPipelineIPSCCP.h"
#include "InvariantLoadToConstant.h"
#include "JitSCCP/JitIPSCCPPass.h"
#include "StaticMutabilityAnalysis.h"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/IPO/FunctionAttrs.h"
#include "llvm/Transforms/IPO/GlobalDCE.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"

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
    MPM.addPass(JitIPSCCPPass(Opts.P2FuncSpec));
    MPM.addPass(llvm::GlobalDCEPass());
    MPM.addPass(llvm::ReversePostOrderFunctionAttrsPass());
    MPM.run(M, MAM);
  }

  // ---- Phase 3: inline specialization clones then cleanup --------------------
  CurrentGroup = "final";
  {
    llvm::ModulePassManager MPM;

    // Inline the clones created by JitFunctionSpecializer into their call sites.
    // AlwaysInliner honours alwaysinline attributes left by JitFunctionSpecializer
    // on the cloned stubs.
    MPM.addPass(llvm::AlwaysInlinerPass(/*InsertLifetimeIntrinsics=*/true));
    MPM.addPass(llvm::GlobalDCEPass());

    // Cleanup pass: fold any remaining invariant loads in inlined bodies and
    // simplify CFG. Gated on !LargeModule to match spec-005 safety rule.
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

  return llvm::Error::success();
}

} // namespace clangRuntimeSpecializer
