#ifndef LLVM_TRANSFORMS_SPECIALIZE_RUNTIMESPECIALIZERPASS_H
#define LLVM_TRANSFORMS_SPECIALIZE_RUNTIMESPECIALIZERPASS_H

#include "llvm/IR/PassManager.h"

namespace llvm {

    class RuntimeSpecializerPass : public PassInfoMixin<RuntimeSpecializerPass> {
    public:
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
    };

    class RuntimeSpecializeableIRFinalizerPass : public PassInfoMixin<RuntimeSpecializeableIRFinalizerPass> {
    public:
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
    };

} // namespace llvm

#endif // LLVM_TRANSFORMS_SPECIALIZE_RUNTIMESPECIALIZERPASS_H