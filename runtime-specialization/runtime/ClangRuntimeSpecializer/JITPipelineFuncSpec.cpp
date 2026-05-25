#include "JITPipelineFuncSpec.h"

#include "ConstantArgFunctionSpecializationPass.h"
#include "InvariantLoadToConstant.h"
#include "StaticMutabilityAnalysis.h"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/InlineAdvisor.h"
#include "llvm/Transforms/IPO/DeadArgumentElimination.h"
#include "llvm/Transforms/IPO/FunctionAttrs.h"
#include "llvm/Transforms/IPO/GlobalDCE.h"
#include "llvm/Transforms/IPO/GlobalOpt.h"
#include "llvm/Transforms/IPO/Inliner.h"
#include "llvm/Transforms/IPO/ModuleInliner.h"
#include "llvm/Transforms/IPO/SCCP.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/DCE.h"
#include "llvm/Transforms/Scalar/EarlyCSE.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"
#include "llvm/Transforms/Scalar/LoopRotation.h"
#include "llvm/Transforms/Scalar/LoopUnrollPass.h"
#include "llvm/Transforms/Scalar/SROA.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"

namespace clangRuntimeSpecializer {

static size_t countInstrs(llvm::Module& M) {
  size_t Count = 0;
  for (auto &F : M)
    for (auto &BB : F)
      Count += BB.size();
  return Count;
}

llvm::Error runFuncSpecPipeline(PipelineRunArgs& Args) {
  llvm::Module& M = Args.Mod;
  const ClangRuntimeSpecializer::Options& Opts = Args.Opts;
  llvm::ModuleAnalysisManager& MAM = Args.MAM;
  std::string& CurrentGroup = Args.CurrentGroup;
  int& CurrentFixpointIter = Args.CurrentFixpointIter;
  const bool LargeModule = Args.IsLargeModule;

  // ---- Phase 1: linkage scrub --------------------------------------------
  // Convert AvailableExternal functions reachable from the wrapper's target
  // back to Internal (so the JIT compiles their bodies) and strip
  // AlwaysInline from the target + transitive callees + call sites so
  // ConstantArgFunctionSpecializationPass + the cost-based inliner can drive
  // specialization. Global variables keep their available_externally linkage
  // and resolve from the host at JIT link time.
  std::string Pipeline1TargetFuncName;
  for (auto &F : M) {
    if (F.isDeclaration() || !F.getName().starts_with("specialized_wrapper_"))
      continue;
    for (auto &BB : F)
      for (auto &I : BB)
        if (auto *CB = llvm::dyn_cast<llvm::CallBase>(&I))
          if (auto *Callee = CB->getCalledFunction())
            if (!Callee->isDeclaration())
              Pipeline1TargetFuncName = Callee->getName().str();
  }

  llvm::SmallPtrSet<llvm::Function*, 32> ToConvert;
  if (!Pipeline1TargetFuncName.empty()) {
    if (auto *TF = M.getFunction(Pipeline1TargetFuncName)) {
      llvm::SmallVector<llvm::Function*, 32> Worklist;
      Worklist.push_back(TF);
      while (!Worklist.empty()) {
        llvm::Function *Cur = Worklist.pop_back_val();
        if (!ToConvert.insert(Cur).second) continue;
        for (auto &BB : *Cur)
          for (auto &I : BB)
            if (auto *CB = llvm::dyn_cast<llvm::CallBase>(&I))
              if (auto *Callee = CB->getCalledFunction())
                if (!Callee->isDeclaration() && !ToConvert.count(Callee))
                  Worklist.push_back(Callee);
      }
    }
  }
  for (auto &F : M) {
    if (F.isDeclaration()) continue;
    if (ToConvert.count(&F)) {
      // Only convert AvailableExternally functions to Internal — that is the
      // sole purpose of this scrub (let the JIT compile their bodies instead
      // of resolving from the host). WeakODR functions already have their
      // bodies compiled by the JIT and MUST stay WeakODR: if downgraded to
      // Internal they become Scope::Local ELF symbols that JITLink excludes
      // from InternedResult, causing a "Missing definitions" link error.
      if (F.hasAvailableExternallyLinkage())
        F.setLinkage(llvm::GlobalValue::InternalLinkage);
    }
    // Strip alwaysinline from everything, including the wrapper. The
    // wrapper is the JIT entry — nothing inlines INTO the JIT host. Leaving
    // alwaysinline on the caller blocks the cost-based inliner from
    // inlining callees INTO the wrapper, which is the entire point of P1
    // (absorb the cspec clone into the wrapper when the budget allows).
    F.removeFnAttr(llvm::Attribute::AlwaysInline);
  }
  for (auto &F : M)
    for (auto &BB : F)
      for (auto &I : BB)
        if (auto *CB = llvm::dyn_cast<llvm::CallBase>(&I))
          CB->removeFnAttr(llvm::Attribute::AlwaysInline);

  // ---- Phase 2: fixpoint loop --------------------------------------------
  // Clone the target with constants baked in, fold the constants inside the
  // clone, then run a *cost-based* inliner so the clone is absorbed into the
  // wrapper only when its inline cost fits the configured budget. Soft cap
  // on module growth terminates the loop early if specialization would blow
  // up code size.
  const int MaxFixpointIterations = Opts.MaxFixpointIterations;
  const size_t StartingInsts = countInstrs(M);
  const double GrowthCap = Opts.P1MaxModuleGrowth;
  const size_t InstCap = GrowthCap > 0.0
      ? static_cast<size_t>(static_cast<double>(StartingInsts) * GrowthCap)
      : 0;
  size_t PrevInsts = 0;

  // Default MaxGroups to 1 to avoid multi-pattern clone explosion; caller can
  // raise via FuncSpecMaxGroups if wider coverage is needed.
  unsigned MaxGroups = Opts.FuncSpecMaxGroups;
  if (MaxGroups == 0) MaxGroups = 1;

  for (int Iter = 0; Iter < MaxFixpointIterations; ++Iter) {
    CurrentGroup = "fixpoint";
    CurrentFixpointIter = Iter;

    llvm::ModulePassManager MPM;

    // (1) Clone the target with constants baked into the clone's body.
    MPM.addPass(clangRuntimeSpecializer::ConstantArgFunctionSpecializationPass(
        MaxGroups, Pipeline1TargetFuncName));

    // (2) Fold the just-substituted constants inside the clone so the
    // inliner cost model sees a small body. Function-level passes are
    // gated behind !LargeModule per the spec-005 ValueHandle workaround.
    //
    // StaticMutabilityAnalysis + InvariantLoadToConstantPass replace loads
    // from host-resident immutable memory (e.g. lambda closure captures
    // passed as a constant ptr) with their host values. These are
    // orthogonal to the inlining-budget property — they substitute
    // constants without growing module size — so P1 includes them.
    if (!LargeModule) {
      llvm::FunctionPassManager FPM;
      FPM.addPass(llvm::SROAPass(llvm::SROAOptions::ModifyCFG));
      FPM.addPass(llvm::EarlyCSEPass(/*UseMemorySSA=*/true));
      FPM.addPass(StaticMutabilityAnalysis::StaticMutabilityAnalysisPass());
      FPM.addPass(InvariantLoadToConstantPass());
      FPM.addPass(llvm::InstCombinePass());
      FPM.addPass(llvm::SimplifyCFGPass());
      {
        llvm::LoopPassManager LPM;
        LPM.addPass(llvm::LoopRotatePass());
        FPM.addPass(llvm::createFunctionToLoopPassAdaptor(
            std::move(LPM), /*UseMemorySSA=*/true));
      }
      llvm::LoopUnrollOptions UO;
      UO.setPartial(false).setRuntime(false).setUpperBound(true)
        .setFullUnrollMaxCount(Opts.LoopUnrollCount);
      FPM.addPass(llvm::LoopUnrollPass(UO));
      // GVN does store-load forwarding through constant pointers (e.g. the
      // baked-in result-pointer arg of a kernel that writes via `*result =
      // ...`). Without it, repeated stores/loads to the same host address
      // survive and the inliner sees a non-trivial body.
      FPM.addPass(llvm::GVNPass());
      FPM.addPass(llvm::InstCombinePass());
      FPM.addPass(llvm::SimplifyCFGPass());
      FPM.addPass(llvm::DCEPass());
      MPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(FPM)));
    }

    // (3) Propagate constants across function boundaries and prune dead
    // originals. IPSCCP shares the spec-005 hazard, so gate it too.
    if (!LargeModule) {
      MPM.addPass(llvm::IPSCCPPass(
          llvm::IPSCCPOptions(/*AllowFuncSpec=*/false)));
    }
    // Infer readonly/readnone/etc. on the (now-folded) clones so a call to
    // them is treated as side-effect-free by the inliner and downstream DCE.
    MPM.addPass(llvm::ReversePostOrderFunctionAttrsPass());
    MPM.addPass(llvm::GlobalOptPass());
    MPM.addPass(llvm::GlobalDCEPass());

    // (4) Cost-based inlining of clones into the wrapper. Threshold is
    // user-configurable via Opts.P1InlineThreshold; default 225 mirrors
    // LLVM's O3 default. *Critical*: we do NOT use AlwaysInlinerPass here —
    // doing so would defeat the budget and reproduce the code-size
    // explosion that Pipeline 0 has on large modules.
    //
    // DeadArgumentElimination first prunes unused formal parameters (case-A
    // in-place specialization leaves the formal args in place even after
    // their uses are replaced with constants).
    MPM.addPass(llvm::DeadArgumentEliminationPass());
    // Run the cost-based ModuleInliner first — it handles all the regular
    // SCC-style inlining decisions.
    {
      llvm::InlineParams IParams = llvm::getInlineParams(Opts.P1InlineThreshold);
      MPM.addPass(llvm::ModuleInlinerPass(IParams));
    }
    // Function-attrs inference after inlining so DCE picks up the now-dead
    // mypow if all of its calls got inlined.
    MPM.addPass(llvm::ReversePostOrderFunctionAttrsPass());

    // (5) Wrap-up cleanup of the wrapper after inlining.
    if (!LargeModule) {
      llvm::FunctionPassManager Post;
      Post.addPass(llvm::SROAPass(llvm::SROAOptions::ModifyCFG));
      Post.addPass(llvm::InstCombinePass());
      Post.addPass(llvm::SimplifyCFGPass());
      MPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(Post)));
    }
    MPM.addPass(llvm::GlobalDCEPass());

    MPM.run(M, MAM);

    // Budget-aware direct inlining for the zero-arg wrapper case.
    // LLVM's cost-based inliner sometimes declines to inline a specialised
    // target into a zero-arg wrapper (the `specializeOnly` path: wrapper has
    // signature R() and a single call with all constant args). We force the
    // inline when the callee body fits the per-callsite budget. We skip the
    // lambda-wrapper case (non-empty wrapper signature) because the cost-
    // based inliner already handles it correctly and forcing on top has been
    // observed to corrupt closure-pointer reads on O3 builds.
    if (!Pipeline1TargetFuncName.empty()) {
      llvm::Function *Target = M.getFunction(Pipeline1TargetFuncName);
      const size_t Budget =
          Opts.P1InlineThreshold > 0
              ? static_cast<size_t>(Opts.P1InlineThreshold)
              : 0;
      if (Target && !Target->isDeclaration()) {
        size_t TargetSize = 0;
        for (auto &BB : *Target) TargetSize += BB.size();
        if (TargetSize <= Budget) {
          llvm::SmallVector<llvm::CallBase *, 4> CallsToInline;
          for (auto &F : M) {
            if (!F.isDeclaration() &&
                F.getName().starts_with("specialized_wrapper_") &&
                F.arg_empty()) {
              for (auto &BB : F)
                for (auto &I : BB)
                  if (auto *CB = llvm::dyn_cast<llvm::CallBase>(&I))
                    if (CB->getCalledFunction() == Target)
                      CallsToInline.push_back(CB);
            }
          }
          for (llvm::CallBase *CB : CallsToInline) {
            llvm::InlineFunctionInfo IFI;
            (void)llvm::InlineFunction(*CB, IFI, /*MergeAttributes=*/true);
          }
        }
      }
    }

    const size_t Insts = countInstrs(M);

    if (InstCap > 0 && Insts > InstCap) {
      ClangRuntimeSpecializer::log(
          ClangRuntimeSpecializer::LogLevel::Warning,
          (llvm::Twine("Pipeline 1 module size cap exceeded: ") +
           llvm::Twine(Insts) + " > " + llvm::Twine(InstCap) +
           " (start=" + llvm::Twine(StartingInsts) +
           "); terminating fixpoint.").str());
      break;
    }

    if (Insts == PrevInsts)
      break;
    PrevInsts = Insts;
  }
  CurrentFixpointIter = -1;

  // ---- Phase 3: final small cleanup --------------------------------------
  // No full O3 — O3 contains its own SCC inliner that would ignore the
  // budget and reproduce code-size explosion on large modules.
  {
    CurrentGroup = "final";
    llvm::ModulePassManager FinalMPM;
    if (!LargeModule) {
      llvm::FunctionPassManager FPM;
      FPM.addPass(llvm::InstCombinePass());
      FPM.addPass(llvm::SimplifyCFGPass());
      FinalMPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(FPM)));
    }
    // Re-run attribute inference so the post-inlined wrapper picks up
    // noundef / memory(none) / etc., matching P0's IR shape.
    FinalMPM.addPass(llvm::ReversePostOrderFunctionAttrsPass());
    FinalMPM.addPass(llvm::GlobalDCEPass());
    FinalMPM.run(M, MAM);
  }

  return llvm::Error::success();
}

} // namespace clangRuntimeSpecializer
