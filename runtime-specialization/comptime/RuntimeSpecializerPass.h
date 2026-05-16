#ifndef LLVM_TRANSFORMS_SPECIALIZE_RUNTIMESPECIALIZERPASS_H
#define LLVM_TRANSFORMS_SPECIALIZE_RUNTIMESPECIALIZERPASS_H

#include "llvm/IR/PassManager.h"

namespace llvm {
    class IRRewritingPass : public PassInfoMixin<IRRewritingPass> {
    public:
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
    };

    class IRDumpingPass : public PassInfoMixin<IRDumpingPass> {
    public:
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
    };

} // namespace llvm

#endif // LLVM_TRANSFORMS_SPECIALIZE_RUNTIMESPECIALIZERPASS_H