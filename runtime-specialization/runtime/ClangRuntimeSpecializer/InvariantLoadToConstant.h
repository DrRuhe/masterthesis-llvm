#pragma once
#include "llvm/IR/PassManager.h"

namespace clangRuntimeSpecializer {

class InvariantLoadToConstantPass : public llvm::PassInfoMixin<InvariantLoadToConstantPass> {
public:
    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM);
};

} // namespace clangRuntimeSpecializer
