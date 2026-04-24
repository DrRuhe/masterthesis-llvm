#include "JITPipelineFuncSpec.h"

#include "ConstantArgFunctionSpecializationPass.h"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"

namespace clangRuntimeSpecializer {

llvm::Error runFuncSpecPipeline(PipelineRunArgs& Args) {
  llvm::Module& M = Args.Mod;
  const ClangRuntimeSpecializer::Options& Opts = Args.Opts;
  llvm::ModuleAnalysisManager& MAM = Args.MAM;
  std::string& CurrentGroup = Args.CurrentGroup;
  int& CurrentFixpointIter = Args.CurrentFixpointIter;

  // Pipeline 1 setup: convert available_externally functions → internal (so the JIT
  // compiles the target function bodies, including pure-static helpers that are
  // invisible to dlopen(NULL)) and strip alwaysinline so
  // ConstantArgFunctionSpecializationPass drives constant propagation via cloning
  // rather than inlining.
  //
  // Global VARIABLES keep their available_externally linkage because the
  // conversion below only touches Functions. They resolve from the host process at
  // JIT link time, giving JIT-compiled bodies access to the host's
  // properly-initialised state (malloc pointers, VFS, etc.).
  //
  // Find the direct callee of the wrapper (= specialization target).
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

  // Simple policy: convert ONLY the target + anything it directly or
  // transitively calls that is currently AvailableExternal (has an IR body).
  // We keep a worklist to do this transitively.
  llvm::SmallPtrSet<llvm::Function*, 32> ToConvert;
  if (!Pipeline1TargetFuncName.empty()) {
    if (auto *TF = M.getFunction(Pipeline1TargetFuncName)) {
      // BFS over call graph within the module.
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
    if (ToConvert.count(&F))
      F.setLinkage(llvm::GlobalValue::InternalLinkage);
    if (!F.getName().starts_with("specialized_wrapper_"))
      F.removeFnAttr(llvm::Attribute::AlwaysInline);
  }
  for (auto &F : M) {
    for (auto &BB : F) {
      for (auto &I : BB) {
        if (auto *CB = llvm::dyn_cast<llvm::CallBase>(&I))
          CB->removeFnAttr(llvm::Attribute::AlwaysInline);
      }
    }
  }

  // DEBUG: skip fixpoint for P1 (MaxFixpointIterations = 0 preserves current behavior)
  const int MaxFixpointIterations = 0;

  for (int Iteration = 0; Iteration < MaxFixpointIterations; ++Iteration) {
    CurrentGroup = "fixpoint";
    CurrentFixpointIter = Iteration;

    llvm::ModulePassManager FixpointMPM;

    // Default to MaxGroups=1 to prevent multi-pattern clone explosion;
    // caller can raise via FuncSpecMaxGroups if wider coverage is needed.
    unsigned MaxGroups = Opts.FuncSpecMaxGroups;
    if (MaxGroups == 0) MaxGroups = 1;
    // Restrict to the wrapper's direct callee so we don't specialize
    // unrelated internal functions.
    FixpointMPM.addPass(clangRuntimeSpecializer::ConstantArgFunctionSpecializationPass(
        MaxGroups, Pipeline1TargetFuncName));

    FixpointMPM.run(M, MAM);

    // Count instructions to check for convergence
    size_t InstCount = 0;
    for (auto &F : M)
      for (auto &BB : F)
        InstCount += BB.size();

    // (no convergence check needed since MaxFixpointIterations = 0)
    (void)InstCount;
  }

  CurrentFixpointIter = -1;

  return llvm::Error::success();
}

} // namespace clangRuntimeSpecializer
