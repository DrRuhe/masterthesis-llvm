#include "JITPipelineInlining.h"
#include "JITPipelineFuncSpec.h"
#include "JITPipelineIPSCCP.h"

namespace clangRuntimeSpecializer {

static const PipelineEntry kPipelines[] = {
    {"inlining",   runInliningPipeline},
    {"func-spec",  runFuncSpecPipeline},
    {"jit-ipsccp", runIPSCCPPipeline},
};

llvm::ArrayRef<PipelineEntry> getRegisteredPipelines() {
    return llvm::ArrayRef(kPipelines);
}

} // namespace clangRuntimeSpecializer
