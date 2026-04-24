#ifndef CLANG_RUNTIME_SPECIALIZER_JITPIPELINEINLINING_H
#define CLANG_RUNTIME_SPECIALIZER_JITPIPELINEINLINING_H

#include "JITPipeline.h"

namespace clangRuntimeSpecializer {

/// Pipeline 0: inlining-based specialization pipeline.
///
/// Runs the initial phase, fixpoint loop (IPSCCP + devirt + inlining),
/// post-fixpoint GlobalDCE, and optional O3 final pass.
llvm::Error runInliningPipeline(PipelineRunArgs& Args);

} // namespace clangRuntimeSpecializer

#endif // CLANG_RUNTIME_SPECIALIZER_JITPIPELINEINLINING_H
