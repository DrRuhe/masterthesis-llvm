#pragma once
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Module.h"

namespace clangRuntimeSpecializer {

// For each function F called with one or more compile-time-constant arguments,
// this pass either:
//   (A) specialises F in-place when ALL call sites pass the same constant pattern, or
//   (B) clones F into one specialised copy per distinct constant-arg pattern and
//       redirects each call site to the appropriate clone.
//
// This avoids aggressive inlining (which blows up instruction count) by instead
// creating pre-specialised callees that are already small after constant folding.
// A more modest inliner can then inline these cheap clones selectively.
class ConstantArgFunctionSpecializationPass
    : public llvm::PassInfoMixin<ConstantArgFunctionSpecializationPass> {
public:
    // maxGroups: max distinct constant-arg patterns per function before skipping.
    // 0 = unlimited.
    explicit ConstantArgFunctionSpecializationPass(unsigned maxGroups = 0)
        : MaxGroups(maxGroups) {}
    llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM);
private:
    unsigned MaxGroups;
};

} // namespace clangRuntimeSpecializer
