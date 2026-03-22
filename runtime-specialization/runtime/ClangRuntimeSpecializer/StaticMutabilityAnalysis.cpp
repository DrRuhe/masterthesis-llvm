#include "StaticMutabilityAnalysis.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Analysis/MemorySSA.h"
#include <queue>
#include <algorithm>

namespace clangRuntimeSpecializer {

using namespace llvm;

void StaticMutabilityAnalysis::inferReadOnlyFields(Function& F, AAResults& AA, MemorySSA* MSSA) {
    if (F.isDeclaration()) return;
    std::map<Value*, PointerInfo> PointerMap;
    std::queue<Value*> Queue;
    
    // 1. Initialize worklist with pointer arguments
    for (Argument& Arg : F.args()) {
        if (Arg.getType()->isPointerTy()) {
            PointerMap[&Arg] = {&Arg, {}, false};
            Queue.push(&Arg);
        }
    }

    // 1b. Initialize worklist with constant pointers (inttoptr)
    for (auto &BB : F) {
        for (auto &I : BB) {
            for (auto &Op : I.operands()) {
                Value *V = Op.get();
                if (auto *CE = dyn_cast<ConstantExpr>(V)) {
                    if (CE->getOpcode() == Instruction::IntToPtr) {
                        if (isa<ConstantInt>(CE->getOperand(0))) {
                             if (PointerMap.find(CE) == PointerMap.end()) {
                                 PointerMap[CE] = {CE, {}, false};
                                 Queue.push(CE);
                             }
                        }
                    }
                }
            }
        }
    }

    if (PointerMap.empty()) {
        return;
    }

    std::set<Value*> Visited;

    // Phase A: Pointer Worklist and Escape Analysis
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
                    bool AllConstant = true;
                    for (auto& Idx : GEP->indices()) {
                        if (auto* CI = dyn_cast<ConstantInt>(Idx)) {
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
                        PointerMap[V].Escaped = true;
                    }
                }
            } else if (auto* BC = dyn_cast<BitCastInst>(U)) {
                PointerMap[BC] = Info;
                Queue.push(BC);
            } else if (auto* ASC = dyn_cast<AddrSpaceCastInst>(U)) {
                PointerMap[ASC] = Info;
                Queue.push(ASC);
            } else if (auto* SI = dyn_cast<StoreInst>(U)) {
                if (SI->getValueOperand() == V) {
                    PointerMap[V].Escaped = true;
                }
            } else if (auto* RI = dyn_cast<ReturnInst>(U)) {
                PointerMap[V].Escaped = true;
            } else if (auto* CB = dyn_cast<CallBase>(U)) {
                for (unsigned i = 0; i < CB->arg_size(); ++i) {
                    if (CB->getArgOperand(i) == V) {
                        if (!CB->doesNotCapture(i)) {
                            PointerMap[V].Escaped = true;
                        }
                    }
                }
            }
        }
    }

    // Phase B: Mutation Tracking
    // We can use MemorySSA to find all MemoryDefs efficiently.
    std::vector<Instruction*> PotentialWrites;
    for (auto &BB : F) {
        for (auto &I : BB) {
            auto *Access = MSSA->getMemoryAccess(&I);
            if (Access && isa<MemoryDef>(Access)) {
                PotentialWrites.push_back(&I);
            }
        }
    }

    // Phase C: Load Classification
    for (BasicBlock& BB : F) {
        for (Instruction& I : BB) {
            if (auto* LI = dyn_cast<LoadInst>(&I)) {
                Value* Ptr = LI->getPointerOperand();
                if (PointerMap.count(Ptr)) {
                    PointerInfo& LInfo = PointerMap[Ptr];
                    
                    bool Escaped = false;
                    for (auto const& [V, Info] : PointerMap) {
                        if (Info.Escaped && LInfo.OriginArg == Info.OriginArg) {
                            if (Info.Path.Indices.size() <= LInfo.Path.Indices.size()) {
                                bool IsPrefix = true;
                                for (size_t i = 0; i < Info.Path.Indices.size(); ++i) {
                                    if (Info.Path.Indices[i] != LInfo.Path.Indices[i]) {
                                        IsPrefix = false;
                                        break;
                                    }
                                }
                                if (IsPrefix) {
                                    Escaped = true;
                                    break;
                                }
                            }
                        }
                    }

                    if (Escaped) continue;

                    bool Mutated = false;
                    for (Instruction* Write : PotentialWrites) {
                        auto ModRef = AA.getModRefInfo(Write, MemoryLocation::get(LI));
                        if (isModSet(ModRef)) {
                            Mutated = true;
                            break;
                        }
                    }

                    if (!Mutated) {
                        LI->setMetadata(LLVMContext::MD_invariant_load, MDNode::get(F.getContext(), {}));
                    }
                }
            }
        }
    }
}

PreservedAnalyses StaticMutabilityAnalysis::StaticMutabilityAnalysisPass::run(Function &F, FunctionAnalysisManager &FAM) {
    auto &AA = FAM.getResult<AAManager>(F);
    auto &MSSA = FAM.getResult<MemorySSAAnalysis>(F).getMSSA();
    inferReadOnlyFields(F, AA, &MSSA);
    return PreservedAnalyses::all();
}

} // namespace clangRuntimeSpecializer
