#pragma once

#include "llvm/IR/PassManager.h"
#include "llvm/IR/Module.h"
#include <map>

namespace clangRuntimeSpecializer {

class VTableEscapeAnalysis : public llvm::PassInfoMixin<VTableEscapeAnalysis> {
public:
    llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM);

private:
    void* getRuntimeAddress(llvm::Function &F);
    llvm::Function* getFunctionAtAddress(void* Addr, llvm::Module &M);
    
    static std::map<void*, std::string> AddressToFunctionName;
    static bool initialized;
    void ensureInitialized(llvm::Module &M);
};

} // namespace clangRuntimeSpecializer
