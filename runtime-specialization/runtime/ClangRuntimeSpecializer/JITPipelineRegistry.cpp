#include "JITPipelineInlining.h"
#include "JITPipelineFuncSpec.h"

namespace clangRuntimeSpecializer {

static const PipelineEntry kPipelines[] = {
    {"inlining",  runInliningPipeline},
    {"func-spec", runFuncSpecPipeline},
};

llvm::ArrayRef<PipelineEntry> getRegisteredPipelines() {
    return llvm::ArrayRef(kPipelines);
}

} // namespace clangRuntimeSpecializer
