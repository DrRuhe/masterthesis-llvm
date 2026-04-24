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
#include "llvm/Transforms/Scalar/CorrelatedValuePropagation.h"
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

    // 3d. Replace invariant loads with constants from host memory.
    InitialMPM.addPass(llvm::createModuleToFunctionPassAdaptor(
        InvariantLoadToConstantPass()));

    // Inline constant-arg call sites on small modules.
    if (!LargeModule) {
      InitialMPM.addPass(ConstantArgAlwaysInlinePass());
      InitialMPM.addPass(llvm::AlwaysInlinerPass(/*InsertLifetimeIntrinsics=*/true));
    }
    InitialMPM.run(M, MAM);
  }

  // Fixpoint iteration loop
  const int MaxFixpointIterations = Opts.MaxFixpointIterations;
  size_t PrevInstCount = 0;

  for (int Iteration = 0; Iteration < MaxFixpointIterations; ++Iteration) {
    CurrentGroup = "fixpoint";
    CurrentFixpointIter = Iteration;

    llvm::ModulePassManager FixpointMPM;

    // 1. Interprocedural Sparse Conditional Constant Propagation.
    FixpointMPM.addPass(llvm::IPSCCPPass(
        llvm::IPSCCPOptions(/*AllowFuncSpec=*/true)));

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

    // 3. Pre-inlining function-level optimizations
    llvm::FunctionPassManager PreInlineFPM;
    PreInlineFPM.addPass(llvm::EarlyCSEPass(/*UseMemorySSA=*/true));
    PreInlineFPM.addPass(llvm::SROAPass(llvm::SROAOptions::ModifyCFG));
    PreInlineFPM.addPass(llvm::JumpThreadingPass());
    PreInlineFPM.addPass(llvm::CorrelatedValuePropagationPass());
    PreInlineFPM.addPass(llvm::SimplifyCFGPass());
    PreInlineFPM.addPass(llvm::InstCombinePass());
    FixpointMPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(PreInlineFPM)));

    // 6. CRITICAL: Post-inlining optimizations
    llvm::FunctionPassManager PostInlineFPM;

    // GVN - propagate constants through inlined code (KEY for devirtualization!)
    PostInlineFPM.addPass(llvm::GVNPass());

    // Static Mutability Analysis + InvariantLoadToConstantPass
    PostInlineFPM.addPass(StaticMutabilityAnalysis::StaticMutabilityAnalysisPass());
    PostInlineFPM.addPass(InvariantLoadToConstantPass());

    // InstCombine - fold loads of constant vtable pointers
    PostInlineFPM.addPass(llvm::InstCombinePass());

    // SROA again - eliminate redundant alloca/store/load patterns
    PostInlineFPM.addPass(llvm::SROAPass(llvm::SROAOptions::ModifyCFG));

    // InstCombine again after SROA
    PostInlineFPM.addPass(llvm::InstCombinePass());

    // EarlyCSE - cleanup redundant loads
    PostInlineFPM.addPass(llvm::EarlyCSEPass(/*UseMemorySSA=*/true));

    // JumpThreading - may find new opportunities
    PostInlineFPM.addPass(llvm::JumpThreadingPass());

    // CorrelatedValuePropagation
    PostInlineFPM.addPass(llvm::CorrelatedValuePropagationPass());

    // SimplifyCFG before loop optimization
    PostInlineFPM.addPass(llvm::SimplifyCFGPass());

    // Loop optimizations: rotate and LICM (requires MemorySSA)
    llvm::LoopPassManager LPM;
    LPM.addPass(llvm::LoopRotatePass());
    LPM.addPass(llvm::LICMPass(llvm::LICMOptions()));
    PostInlineFPM.addPass(llvm::createFunctionToLoopPassAdaptor(std::move(LPM), /*UseMemorySSA=*/true));

    // Loop unrolling. On small modules use aggressive settings to expose
    // more constants. On large modules limit to partial unrolling only.
    llvm::LoopUnrollOptions UnrollOpts;
    UnrollOpts.setPartial(true);
    if (!LargeModule) {
      UnrollOpts.setRuntime(true);
      UnrollOpts.setUpperBound(true);
      UnrollOpts.setFullUnrollMaxCount(Opts.LoopUnrollCount);
    }
    PostInlineFPM.addPass(llvm::LoopUnrollPass(UnrollOpts));

    // Post-unroll cleanup
    PostInlineFPM.addPass(llvm::InstCombinePass());
    PostInlineFPM.addPass(llvm::SimplifyCFGPass());
    PostInlineFPM.addPass(llvm::InstSimplifyPass());

    FixpointMPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(PostInlineFPM)));

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
