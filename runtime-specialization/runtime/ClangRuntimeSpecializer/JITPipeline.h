#ifndef CLANG_RUNTIME_SPECIALIZER_JITPIPELINE_H
#define CLANG_RUNTIME_SPECIALIZER_JITPIPELINE_H

#include "ClangRuntimeSpecializer.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassInstrumentation.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/Error.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"

namespace clangRuntimeSpecializer {

/// Parameter bundle passed from the IR transform dispatch layer to a pipeline
/// implementation. All fields are references into caller-owned storage — the
/// pipeline must not outlive the dispatch call.
struct PipelineRunArgs {
  /// The module to transform (modified in place).
  llvm::Module& Mod;

  /// Active options for this JIT invocation (read-only).
  const ClangRuntimeSpecializer::Options& Opts;

  /// Pre-constructed PassBuilder with the target machine already set.
  llvm::PassBuilder& PB;

  /// Pre-populated analysis managers (cross-registered).
  llvm::ModuleAnalysisManager& MAM;
  llvm::FunctionAnalysisManager& FAM;
  llvm::CGSCCAnalysisManager& CGAM;
  llvm::LoopAnalysisManager& LAM;

  /// Pre-populated PassInstrumentationCallbacks carrying trace callbacks.
  llvm::PassInstrumentationCallbacks& PIC;

  /// Pipeline writes this to label trace records per phase.
  /// Expected values: "prune", "initial", "fixpoint", "postfix", "final".
  std::string& CurrentGroup;

  /// Pipeline writes this inside fixpoint loops; -1 when not in a fixpoint loop.
  int& CurrentFixpointIter;

  /// True when the module exceeds \c Opts.LargeModuleInstrThreshold instructions
  /// after the prune step. Pipelines may use this to select conservative settings
  /// (e.g. skip aggressive loop unrolling).
  bool IsLargeModule;
};

/// One registered pipeline implementation.
struct PipelineEntry {
  /// Human-readable label used in logs and diagnostics (e.g. "inlining",
  /// "func-spec").
  llvm::StringRef Name;

  /// The pipeline implementation. Called by the IR transform dispatch layer
  /// when \c Options::OptimizationPipelineToUse selects this entry.
  /// Must return \c llvm::Error::success() on success or a descriptive error.
  llvm::Error (*Run)(PipelineRunArgs&);
};

/// Returns the static registry of all compiled-in pipeline implementations.
/// The returned array is valid for the lifetime of the process.
/// Defined in \c JITPipelineRegistry.cpp (created in a later phase).
llvm::ArrayRef<PipelineEntry> getRegisteredPipelines();

} // namespace clangRuntimeSpecializer

#endif // CLANG_RUNTIME_SPECIALIZER_JITPIPELINE_H
