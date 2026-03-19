#include "StaticMutabilityAnalysis.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/Analysis/ValueTracking.h"
#include <queue>

namespace clangRuntimeSpecializer {

using namespace llvm;

std::set<FieldPath> StaticMutabilityAnalysis::inferReadOnlyFields(Function& F, AAResults& AA) {
    std::map<Value*, PointerInfo> Worklist;
    std::set<Value*> Visited;
    
    // 1. Initialize worklist with pointer arguments
    for (Argument& Arg : F.args()) {
        if (Arg.getType()->isPointerTy()) {
            Worklist[&Arg] = {&Arg, {}};
        }
    }

    std::set<Value*> MutablePointers;
    std::queue<Value*> Queue;
    for (auto const& [V, Info] : Worklist) {
        Queue.push(V);
    }

    // Map each tracked pointer to its origin and field path
    std::map<Value*, PointerInfo> PointerMap = Worklist;

    while (!Queue.empty()) {
        Value* V = Queue.front();
        Queue.pop();
        if (Visited.count(V)) continue;
        Visited.insert(V);

        PointerInfo Info = PointerMap[V];

        for (User* U : V->users()) {
            if (auto* GEP = dyn_cast<GetElementPtrInst>(U)) {
                if (GEP->getPointerOperand() == V) {
                    PointerInfo NewInfo = Info;
                    // If GEP has constant indices, we can track the field.
                    // Otherwise, we mark the whole path as mutable (conservative).
                    bool AllConstant = true;
                    for (auto It = GEP->idx_begin() + 1; It != GEP->idx_end(); ++It) {
                        if (auto* CI = dyn_cast<ConstantInt>(*It)) {
                            NewInfo.Path.Indices.push_back(CI->getZExtValue());
                        } else {
                            AllConstant = false;
                            break;
                        }
                    }
                    if (AllConstant) {
                        PointerMap[GEP] = NewInfo;
                        Queue.push(GEP);
                    } else {
                        // Opaque access - everything from here is mutable
                        MutablePointers.insert(V);
                    }
                }
            } else if (auto* BC = dyn_cast<BitCastInst>(U)) {
                PointerMap[BC] = Info;
                Queue.push(BC);
            } else if (auto* ASC = dyn_cast<AddrSpaceCastInst>(U)) {
                PointerMap[ASC] = Info;
                Queue.push(ASC);
            } else if (auto* SI = dyn_cast<StoreInst>(U)) {
                if (SI->getPointerOperand() == V) {
                    MutablePointers.insert(V);
                }
            } else if (auto* RMW = dyn_cast<AtomicRMWInst>(U)) {
                if (RMW->getPointerOperand() == V) {
                    MutablePointers.insert(V);
                }
            } else if (auto* CX = dyn_cast<AtomicCmpXchgInst>(U)) {
                if (CX->getPointerOperand() == V) {
                    MutablePointers.insert(V);
                }
            } else if (auto* CB = dyn_cast<CallBase>(U)) {
                // If the pointer is passed to a function that might modify it
                bool IsArg = false;
                for (unsigned i = 0; i < CB->arg_size(); ++i) {
                    if (CB->getArgOperand(i) == V) {
                        IsArg = true;
                        break;
                    }
                }
                if (IsArg) {
                   auto ModRef = AA.getModRefInfo(CB, V, LocationSize::beforeOrAfterPointer());
                   if (isModSet(ModRef)) {
                       MutablePointers.insert(V);
                   }
                }
            }
            // Add more cases: captures, etc.
        }
    }

    // Now we have a list of pointers that are mutated.
    // For each load, we can check if its pointer is NOT in MutablePointers or any of its subpaths are.
    // Actually, it's easier to just mark everything NOT in MutablePointers as potential invariant.
    
    // We need to return something that allows the caller to mark loads.
    // Let's change the return or the logic: 
    // The goal is to mark LOADs as !invariant.load.
    
    for (BasicBlock& BB : F) {
        for (Instruction& I : BB) {
            if (auto* LI = dyn_cast<LoadInst>(&I)) {
                Value* Ptr = LI->getPointerOperand();
                if (PointerMap.count(Ptr)) {
                    bool IsMutable = false;
                    // Check if this pointer or any "parent" or "child" is mutable.
                    // This is complex because a write to a parent struct marks all children as potentially modified.
                    // A write to a child marks part of the parent as modified.
                    
                    // Simplified: if this exact GEP path (or any prefix of it) was written to, it's mutable.
                    // Also if any suffix was written to (e.g. write to struct field when we loaded the struct, 
                    // though usually we load primitives).
                    
                    for (Value* MP : MutablePointers) {
                        PointerInfo MInfo = PointerMap[MP];
                        if (MInfo.OriginArg == PointerMap[Ptr].OriginArg) {
                            // Check if MInfo.Path is a prefix of PointerMap[Ptr].Path or vice versa.
                            const auto& P1 = MInfo.Path.Indices;
                            const auto& P2 = PointerMap[Ptr].Path.Indices;
                            bool Conflict = true;
                            for (size_t i = 0; i < std::min(P1.size(), P2.size()); ++i) {
                                if (P1[i] != P2[i]) {
                                    Conflict = false;
                                    break;
                                }
                            }
                            if (Conflict) {
                                IsMutable = true;
                                break;
                            }
                        }
                    }
                    
                    if (!IsMutable) {
                        LI->setMetadata(LLVMContext::MD_invariant_load, MDNode::get(F.getContext(), {}));
                    }
                }
            }
        }
    }

    return {}; // Dummy return for now.
}

PreservedAnalyses StaticMutabilityAnalysis::StaticMutabilityAnalysisPass::run(Function &F, FunctionAnalysisManager &FAM) {
    auto &AA = FAM.getResult<AAManager>(F);
    inferReadOnlyFields(F, AA);
    return PreservedAnalyses::all();
}

} // namespace clangRuntimeSpecializer
