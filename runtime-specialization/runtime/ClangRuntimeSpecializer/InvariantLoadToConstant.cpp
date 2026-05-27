#include "InvariantLoadToConstant.h"
#include "PointerChainResolver.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/ADT/DenseMap.h"

namespace clangRuntimeSpecializer {

using namespace llvm;

PreservedAnalyses InvariantLoadToConstantPass::run(Function &F, FunctionAnalysisManager &FAM) {
    bool Changed = false;
    llvm::DenseMap<uintptr_t, bool> PageCache;

    std::vector<LoadInst*> ToReplace;
    for (auto &BB : F)
        for (auto &I : BB)
            if (auto *LI = dyn_cast<LoadInst>(&I))
                if (LI->getMetadata(LLVMContext::MD_invariant_load))
                    ToReplace.push_back(LI);

    for (auto *LI : ToReplace) {
        auto MaybeConst = resolveInvariantLoadToConstant(*LI, PageCache);
        if (MaybeConst.has_value() && *MaybeConst) {
            LI->replaceAllUsesWith(*MaybeConst);
            LI->eraseFromParent();
            Changed = true;
        }
    }

    if (Changed) {
        PreservedAnalyses PA;
        PA.preserveSet<CFGAnalyses>();
        return PA;
    }
    return PreservedAnalyses::all();
}

} // namespace clangRuntimeSpecializer
