#include "InvariantLoadToConstant.h"
#include "PointerChainResolver.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/raw_ostream.h"

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

    unsigned Replaced = 0;
    for (auto *LI : ToReplace) {
        auto MaybeConst = resolveInvariantLoadToConstant(*LI, PageCache);
        if (MaybeConst.has_value() && *MaybeConst) {
            LI->replaceAllUsesWith(*MaybeConst);
            LI->eraseFromParent();
            ++Replaced;
            Changed = true;
        }
    }
    // Investigation: report how many invariant loads were converted to literals.
    if (!ToReplace.empty())
        llvm::errs() << "[CRS-STAT] InvariantLoadToConst: fn=" << F.getName()
                     << " candidates=" << ToReplace.size()
                     << " replaced=" << Replaced << "\n";

    if (Changed) {
        PreservedAnalyses PA;
        PA.preserveSet<CFGAnalyses>();
        return PA;
    }
    return PreservedAnalyses::all();
}

} // namespace clangRuntimeSpecializer
