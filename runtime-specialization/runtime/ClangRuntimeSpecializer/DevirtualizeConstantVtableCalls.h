#pragma once

#include "llvm/IR/PassManager.h"
#include "llvm/IR/Module.h"
#include <map>
#include <string>

namespace clangRuntimeSpecializer {

// Devirtualize indirect calls whose callee can be resolved through a chain of
// loads from compile-time-constant addresses (e.g. vtable pointer embedded in
// a heap object whose address was specialised to a constant inttoptr).
//
// Pattern matched (any nesting depth):
//   %vtable  = load ptr, ptr inttoptr(constant)   ; or any resolved chain
//   %fn      = load ptr, ptr %vtable              ; (or offset from vtable)
//   call %fn(args)
//
// When the final resolved address belongs to a function whose definition is
// present in the module the indirect call is replaced by a direct call,
// enabling the inliner to inline the callee in subsequent passes.
class DevirtualizeConstantVtableCallsPass
    : public llvm::PassInfoMixin<DevirtualizeConstantVtableCallsPass> {
public:
    llvm::PreservedAnalyses run(llvm::Module &M,
                                llvm::ModuleAnalysisManager &MAM);

private:
    static std::map<void *, std::string> AddressToFunctionName;
    static bool Initialized;

    void ensureInitialized(llvm::Module &M);
    llvm::Function *getFunctionAtAddress(void *Addr, llvm::Module &M);
};

} // namespace clangRuntimeSpecializer
