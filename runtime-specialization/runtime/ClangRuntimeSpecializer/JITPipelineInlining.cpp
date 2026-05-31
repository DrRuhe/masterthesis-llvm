#include "JITPipelineInlining.h"

#include "DevirtualizeConstantVtableCalls.h"
#include "StaticMutabilityAnalysis.h"
#include "InvariantLoadToConstant.h"

#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/IPO/SCCP.h"
#include "llvm/Transforms/IPO/GlobalOpt.h"
#include "llvm/Transforms/IPO/GlobalDCE.h"
#include "llvm/Transforms/IPO/WholeProgramDevirt.h"
#include "llvm/Transforms/Scalar/SROA.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Scalar/InstSimplifyPass.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/LICM.h"
#include "llvm/Transforms/Scalar/LoopUnrollPass.h"
#include "llvm/Transforms/Scalar/LoopRotation.h"
#include "llvm/Transforms/Scalar/JumpThreading.h"
#include "llvm/Transforms/Scalar/EarlyCSE.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/IPO/FunctionAttrs.h"
#include "llvm/IR/PassInstrumentation.h"

namespace clangRuntimeSpecializer {

namespace {

// Marks call sites alwaysinline when at least one argument is a compile-time constant.
// This restricts inlining to sites where specialization can actually propagate the
// constant, avoiding unnecessary IR expansion at non-constant-arg call sites.
struct ConstantArgAlwaysInlinePass
    : llvm::PassInfoMixin<ConstantArgAlwaysInlinePass> {
  llvm::PreservedAnalyses run(llvm::Module &M,
                              llvm::ModuleAnalysisManager &) {
    bool Changed = false;
    for (auto &F : M) {
      for (auto &BB : F) {
        for (auto &I : BB) {
          auto *CB = llvm::dyn_cast<llvm::CallBase>(&I);
          if (!CB) continue;
          auto *Callee = CB->getCalledFunction();
          // Only direct calls to non-declaration callees not already
          // alwaysinline on the function definition.
          if (!Callee || Callee->isDeclaration()) continue;
          if (Callee->hasFnAttribute(llvm::Attribute::AlwaysInline))
            continue;

          bool HasConstantArg = llvm::any_of(
              CB->args(), [](const llvm::Use &U) {
                return llvm::isa<llvm::Constant>(U.get());
              });

          if (HasConstantArg &&
              !CB->hasFnAttr(llvm::Attribute::AlwaysInline)) {
            CB->addFnAttr(llvm::Attribute::AlwaysInline);
            CB->removeFnAttr(llvm::Attribute::NoInline);
            Changed = true;
          }
        }
      }
    }
    return Changed ? llvm::PreservedAnalyses::none()
                   : llvm::PreservedAnalyses::all();
  }
};

} // namespace

llvm::Error runInliningPipeline(PipelineRunArgs& Args) {
  llvm::Module& M = Args.Mod;
  const ClangRuntimeSpecializer::Options& Opts = Args.Opts;
  llvm::PassBuilder& PB = Args.PB;
  llvm::ModuleAnalysisManager& MAM = Args.MAM;
  // FAM, CGAM, LAM are already cross-registered into MAM; pipelines use MAM directly.
  (void)Args.FAM;
  (void)Args.CGAM;
  (void)Args.LAM;
  std::string& CurrentGroup = Args.CurrentGroup;
  int& CurrentFixpointIter = Args.CurrentFixpointIter;
  const bool LargeModule = Args.IsLargeModule;

  // Clear blocking attributes so the inliner can act on call-site annotations.
  for (auto &F : M) {
    if (!F.isDeclaration()) {
      F.removeFnAttr(llvm::Attribute::NoInline);
      F.removeFnAttr(llvm::Attribute::OptimizeNone);
    }
  }

  // Initial phase: cleanup + attribute inference + initial inlining
  CurrentGroup = "initial";
  {
    llvm::ModulePassManager InitialMPM;

    // 3a. Cleanup alloca/store/load before analysis
    llvm::FunctionPassManager FPM;
    FPM.addPass(llvm::SROAPass(llvm::SROAOptions::ModifyCFG));
    FPM.addPass(llvm::EarlyCSEPass());
    FPM.addPass(llvm::InstCombinePass());
    InitialMPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(FPM)));

    // 3b. Attribute inference - deduces readonly, readnone, etc.
    InitialMPM.addPass(llvm::ReversePostOrderFunctionAttrsPass());

    // 3c. Static Mutability Analysis to infer read-only fields.
    InitialMPM.addPass(llvm::createModuleToFunctionPassAdaptor(
        StaticMutabilityAnalysis::StaticMutabilityAnalysisPass()));

    // Inline constant-arg call sites on small modules, then immediately resolve
    // invariant loads: the lambda struct pointer (inttoptr i64 addr) is now visible
    // in the wrapper body after inlining, so field loads tagged !invariant.load can
    // be replaced with literal IR constants in the initial phase.
    if (!LargeModule) {
      InitialMPM.addPass(ConstantArgAlwaysInlinePass());
      InitialMPM.addPass(llvm::AlwaysInlinerPass(/*InsertLifetimeIntrinsics=*/true));
      InitialMPM.addPass(llvm::createModuleToFunctionPassAdaptor(InvariantLoadToConstantPass()));
    }
    InitialMPM.run(M, MAM);
  }

  // Fixpoint iteration loop
  const int MaxFixpointIterations = Opts.MaxFixpointIterations;
  size_t PrevInstCount = 0;

  for (int Iteration = 0; Iteration < MaxFixpointIterations; ++Iteration) {
    CurrentGroup = "fixpoint";
    CurrentFixpointIter = Iteration;

    // Note: we intentionally do NOT call MAM.invalidate(M, none()) here.
    // Letting the pass manager manage analysis lifetime via PreservedAnalyses
    // return values avoids a destruction sequence that fires debug assertions
    // in LLVM's ValueHandleBase chain (AddToUseList line 1170).

    llvm::ModulePassManager FixpointMPM;

    // 1. Interprocedural Sparse Conditional Constant Propagation.
    // AllowFuncSpec=false: function specialization clones values but doesn't
    // update the ValueHandles map, triggering a debug assertion in LazyValueInfo
    // (AssertingVH) when the O3 CVP processes the cloned module.
    //
    // Large-module restriction: IPSCCP is omitted for large modules.  It
    // creates AssumptionCache WeakVH handles on every call, which grow the
    // per-LLVMContext pImpl->ValueHandles DenseMap.  In a fixpoint loop the
    // DenseMap can rehash repeatedly; each rehash frees the old bucket array,
    // leaving chain-head PrevPtr fields stale.  A subsequent RemoveFromUseList
    // then writes through the stale pointer into freed/reused memory, corrupting
    // instruction metadata bits or other DenseMap internals.  This is
    // debug-mode only (AssertingVH/WeakVH are plain pointers in release builds)
    // but makes the debug benchmark unusable.  The inliner + GlobalOpt path
    // below already propagates the runtime constants we care about for large
    // modules without creating a problematic number of value handles.
    if (!LargeModule) {
      FixpointMPM.addPass(llvm::IPSCCPPass(
          llvm::IPSCCPOptions(/*AllowFuncSpec=*/false)));
    }

    // 1b. Devirtualize indirect calls through constant vtable pointers
    FixpointMPM.addPass(DevirtualizeConstantVtableCallsPass());

    // 1b-post. Prune functions made dead by IPSCCP/devirt before inlining.
    FixpointMPM.addPass(llvm::GlobalDCEPass());

    // 1b2. Infer attributes again after optimistic resolution
    FixpointMPM.addPass(llvm::ReversePostOrderFunctionAttrsPass());

    // 1c. Static Mutability Analysis to infer read-only fields.
    FixpointMPM.addPass(llvm::createModuleToFunctionPassAdaptor(
        StaticMutabilityAnalysis::StaticMutabilityAnalysisPass()));

    // 1d. Replace invariant loads with constants from host memory.
    FixpointMPM.addPass(llvm::createModuleToFunctionPassAdaptor(
        InvariantLoadToConstantPass()));

    // 1e. Constant-arg propagation: inline constant-arg call sites.
    FixpointMPM.addPass(ConstantArgAlwaysInlinePass());
    FixpointMPM.addPass(llvm::AlwaysInlinerPass());

    // 2. Global optimizations - includes devirtualization
    FixpointMPM.addPass(llvm::GlobalOptPass());

    // 2b. Whole-program devirtualization
    FixpointMPM.addPass(llvm::WholeProgramDevirtPass());

    // 3. Pre-inlining function-level optimizations.
    //
    // Large-module restriction: ALL function-level passes (via FPM adaptor) are
    // omitted from the fixpoint for large modules.  The same pImpl->ValueHandles
    // stale-PrevPtr issue described above for IPSCCP applies to any analysis
    // that uses AssertingVH or PoisoningVH (LazyValueInfo → JumpThreading,
    // InstCombine; AliasSetTracker → LICM; GVN::BlockRPONumber → GVN).
    // Debug-mode only; release builds are unaffected.
    if (!LargeModule) {
      llvm::FunctionPassManager PreInlineFPM;
      PreInlineFPM.addPass(llvm::EarlyCSEPass(/*UseMemorySSA=*/true));
      PreInlineFPM.addPass(llvm::SROAPass(llvm::SROAOptions::ModifyCFG));
      PreInlineFPM.addPass(llvm::JumpThreadingPass());
      PreInlineFPM.addPass(llvm::SimplifyCFGPass());
      PreInlineFPM.addPass(llvm::InstCombinePass());
      FixpointMPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(PreInlineFPM)));
    }

    // 6. CRITICAL: Post-inlining optimizations (small modules only; see above).
    if (!LargeModule) {
      llvm::FunctionPassManager PostInlineFPM;

      // Static Mutability Analysis + InvariantLoadToConstantPass
      PostInlineFPM.addPass(StaticMutabilityAnalysis::StaticMutabilityAnalysisPass());
      PostInlineFPM.addPass(InvariantLoadToConstantPass());

      PostInlineFPM.addPass(llvm::InstCombinePass());
      PostInlineFPM.addPass(llvm::SROAPass(llvm::SROAOptions::ModifyCFG));
      PostInlineFPM.addPass(llvm::InstCombinePass());
      PostInlineFPM.addPass(llvm::EarlyCSEPass(/*UseMemorySSA=*/true));
      PostInlineFPM.addPass(llvm::JumpThreadingPass());
      PostInlineFPM.addPass(llvm::SimplifyCFGPass());

      {
        llvm::LoopPassManager LPM;
        LPM.addPass(llvm::LoopRotatePass());
        LPM.addPass(llvm::LICMPass(llvm::LICMOptions()));
        PostInlineFPM.addPass(llvm::createFunctionToLoopPassAdaptor(
            std::move(LPM), /*UseMemorySSA=*/true));
      }

      {
        llvm::LoopUnrollOptions UnrollOpts;
        UnrollOpts.setPartial(true);
        UnrollOpts.setRuntime(true);
        UnrollOpts.setUpperBound(true);
        UnrollOpts.setFullUnrollMaxCount(Opts.LoopUnrollCount);
        PostInlineFPM.addPass(llvm::LoopUnrollPass(UnrollOpts));
      }

      PostInlineFPM.addPass(llvm::InstCombinePass());
      PostInlineFPM.addPass(llvm::SimplifyCFGPass());
      PostInlineFPM.addPass(llvm::InstSimplifyPass());

      FixpointMPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(PostInlineFPM)));
    }

    // Run the fixpoint iteration pass pipeline
    FixpointMPM.run(M, MAM);

    // Count instructions to check for convergence
    size_t InstCount = 0;
    for (auto &F : M)
      for (auto &BB : F)
        InstCount += BB.size();

    if (InstCount == PrevInstCount)
      break;

    PrevInstCount = InstCount;
  }

  CurrentFixpointIter = -1;

  // Final GlobalDCE after all fixpoint iterations.
  {
    CurrentGroup = "postfix";
    llvm::ModulePassManager PostFixpointMPM;
    PostFixpointMPM.addPass(llvm::GlobalDCEPass());
    PostFixpointMPM.run(M, MAM);
  }

  // Final O3 pass for cleanup and additional optimizations
  if (Opts.EnableO3Final) {
    CurrentGroup = "final";
    llvm::ModulePassManager FinalMPM =
        PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);
    FinalMPM.run(M, MAM);
  }

  return llvm::Error::success();
}

} // namespace clangRuntimeSpecializer
