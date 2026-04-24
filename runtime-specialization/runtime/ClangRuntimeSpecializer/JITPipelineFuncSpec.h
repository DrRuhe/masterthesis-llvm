#ifndef CLANG_RUNTIME_SPECIALIZER_JITPIPELINEFUNCSPEC_H
#define CLANG_RUNTIME_SPECIALIZER_JITPIPELINEFUNCSPEC_H

#include "JITPipeline.h"

namespace clangRuntimeSpecializer {

/// Pipeline 1: function-specialization/cloning pipeline.
///
/// Converts the specialization target and its transitive callees to Internal
/// linkage, then uses ConstantArgFunctionSpecializationPass to clone functions
/// with baked-in constant arguments rather than inlining them.
llvm::Error runFuncSpecPipeline(PipelineRunArgs& Args);

} // namespace clangRuntimeSpecializer

#endif // CLANG_RUNTIME_SPECIALIZER_JITPIPELINEFUNCSPEC_H
