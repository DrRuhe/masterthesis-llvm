#include "ConstantArgFunctionSpecializationPass.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/raw_ostream.h"
#include <atomic>
#include <map>
#include <vector>

namespace clangRuntimeSpecializer {

namespace {

// SpecKey: the subset of argument positions that are constants at a call site.
// LLVM uniques constants within a context, so pointer equality is value equality.
using SpecKey = std::vector<std::pair<unsigned, llvm::Constant *>>;

// Only inspect formal parameters (up to F->arg_size()); vararg extras are skipped.
SpecKey buildSpecKey(llvm::CallBase *CB, unsigned NumFormalParams) {
    SpecKey Key;
    unsigned Limit = std::min(CB->arg_size(), NumFormalParams);
    for (unsigned I = 0; I < Limit; ++I) {
        if (auto *C = llvm::dyn_cast<llvm::Constant>(CB->getArgOperand(I)))
            Key.emplace_back(I, C);
    }
    return Key;
}

static std::atomic<unsigned> SpecId{0};

} // namespace

llvm::PreservedAnalyses
ConstantArgFunctionSpecializationPass::run(llvm::Module &M,
                                           llvm::ModuleAnalysisManager &) {
    // Collect work items first to avoid iterator invalidation during cloning.
    // work[F] = { groups (SpecKey -> call sites), unspecializedSites }
    struct WorkItem {
        llvm::Function *F;
        std::map<SpecKey, std::vector<llvm::CallBase *>> Groups;
        std::vector<llvm::CallBase *> UnspecializedSites;
    };
    std::vector<WorkItem> Work;

    for (llvm::Function &F : M) {
        if (F.isDeclaration() || F.arg_empty())
            continue;
        if (!TargetName.empty() && F.getName() != TargetName)
            continue;

        WorkItem Item;
        Item.F = &F;
        const unsigned NumFormals = F.arg_size();

        for (llvm::User *U : F.users()) {
            auto *CB = llvm::dyn_cast<llvm::CallBase>(U);
            if (!CB || CB->getCalledFunction() != &F)
                continue;
            // Skip calls not inside a function body (e.g. global initializers).
            if (!CB->getParent() || !CB->getParent()->getParent())
                continue;

            SpecKey Key = buildSpecKey(CB, NumFormals);
            if (Key.empty())
                Item.UnspecializedSites.push_back(CB);
            else
                Item.Groups[Key].push_back(CB);
        }

        if (Item.Groups.empty())
            continue;
        if (MaxGroups > 0 && Item.Groups.size() > MaxGroups)
            continue;

        Work.push_back(std::move(Item));
    }

    if (Work.empty())
        return llvm::PreservedAnalyses::all();

    bool Changed = false;

    for (auto &Item : Work) {
        llvm::Function *F = Item.F;
        auto &Groups = Item.Groups;
        auto &UnspecializedSites = Item.UnspecializedSites;
        // Investigation: report how many distinct constant-arg patterns exist.
        llvm::errs() << "[CRS-STAT] ConstantArgSpec: fn=" << F->getName()
                     << " specialization_groups=" << Groups.size()
                     << " unspecialized_sites=" << UnspecializedSites.size() << "\n";

        if (Groups.size() == 1 && UnspecializedSites.empty()) {
            // (A) In-place specialization: replace each constant arg with its value
            //     directly in F's body.
            const SpecKey &Key = Groups.begin()->first;
            for (auto [ArgIdx, C] : Key) {
                llvm::Argument *Arg = F->getArg(ArgIdx);
                Arg->replaceAllUsesWith(C);
            }
            F->removeFnAttr(llvm::Attribute::NoInline);
            F->removeFnAttr(llvm::Attribute::OptimizeNone);
            Changed = true;
        } else {
            // (B) One clone per SpecKey; redirect each group's call sites to its clone.
            for (auto &[Key, CallSites] : Groups) {
                llvm::ValueToValueMapTy VMap;
                // Pre-populate VMap so CloneFunction substitutes constants directly.
                for (auto [ArgIdx, C] : Key)
                    VMap[F->getArg(ArgIdx)] = C;

                llvm::Function *Clone = llvm::CloneFunction(F, VMap);
                Clone->setName(F->getName() + "_cspec_" +
                               std::to_string(SpecId.fetch_add(1)));
                Clone->setLinkage(llvm::GlobalValue::InternalLinkage);
                Clone->removeFnAttr(llvm::Attribute::NoInline);
                Clone->removeFnAttr(llvm::Attribute::OptimizeNone);

                for (llvm::CallBase *CB : CallSites)
                    CB->setCalledFunction(Clone);

                Changed = true;
            }
            // Original F is left untouched for UnspecializedSites.
            // If all call sites were in Groups (no UnspecializedSites), F becomes
            // unreferenced and will be removed by the subsequent GlobalDCE pass.
        }
    }

    if (!Changed)
        return llvm::PreservedAnalyses::all();

    llvm::PreservedAnalyses PA;
    PA.preserveSet<llvm::CFGAnalyses>();
    return PA;
}

} // namespace clangRuntimeSpecializer
