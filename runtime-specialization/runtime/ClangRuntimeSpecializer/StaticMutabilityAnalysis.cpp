#include "StaticMutabilityAnalysis.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/Support/raw_ostream.h"
#include <queue>
#include <algorithm>

namespace clangRuntimeSpecializer {

using namespace llvm;

std::map<Value*, StaticMutabilityAnalysis::PointerInfo> StaticMutabilityAnalysis::runCaptureAnalysis(Function& F) {
    std::map<Value*, PointerInfo> PointerMap;
    if (F.isDeclaration()) return PointerMap;
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
        return PointerMap;
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
                if (SI->getPointerOperand() == V) {
                    PointerMap[V].Mutated = true;
                }
            } else if (auto* RI = dyn_cast<ReturnInst>(U)) {
                PointerMap[V].Escaped = true;
            } else if (auto* CB = dyn_cast<CallBase>(U)) {
                for (unsigned i = 0; i < CB->arg_size(); ++i) {
                    if (CB->getArgOperand(i) == V) {
                        if (!CB->doesNotCapture(i)) {
                            PointerMap[V].Escaped = true;
                        }
                        if (!CB->onlyReadsMemory(i)) {
                            PointerMap[V].Mutated = true;
                        }
                    }
                }
            }
        }
    }
    return PointerMap;
}

void StaticMutabilityAnalysis::inferReadOnlyFields(Function& F, AAResults& AA, MemorySSA* MSSA) {
    if (F.isDeclaration()) return;
    std::map<Value*, PointerInfo> PointerMap = runCaptureAnalysis(F);
    
    if (PointerMap.empty()) {
        return;
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
                    
                    bool FieldEscaped = false;
                    bool FieldMutated = false;
                    for (auto const& [V, Info] : PointerMap) {
                        if (Info.OriginArg != LInfo.OriginArg) continue;

                        // Check if Info.Path is a prefix of LInfo.Path (Info is parent/self)
                        bool IsPrefix = true;
                        if (Info.Path.Indices.size() <= LInfo.Path.Indices.size()) {
                            for (size_t i = 0; i < Info.Path.Indices.size(); ++i) {
                                if (Info.Path.Indices[i] != LInfo.Path.Indices[i]) {
                                    IsPrefix = false;
                                    break;
                                }
                            }
                        } else {
                            IsPrefix = false;
                        }

                        if (IsPrefix && Info.Escaped) FieldEscaped = true;

                        // Check if paths overlap (prefix in either direction) for mutation
                        bool Overlaps = true;
                        size_t MinLen = std::min(Info.Path.Indices.size(), LInfo.Path.Indices.size());
                        for (size_t i = 0; i < MinLen; ++i) {
                            if (Info.Path.Indices[i] != LInfo.Path.Indices[i]) {
                                Overlaps = false;
                                break;
                            }
                        }
                        if (Overlaps && Info.Mutated) FieldMutated = true;
                    }

                    if (FieldEscaped) continue;

                    bool ActuallyMutated = FieldMutated;
                    if (!ActuallyMutated) {
                        for (Instruction* Write : PotentialWrites) {
                            // If our field hasn't escaped, we can ignore writes to pointers
                            // that are not derived from the same origin, because they
                            // cannot alias with our private memory.
                            if (auto* SI = dyn_cast<StoreInst>(Write)) {
                                if (PointerMap.count(SI->getPointerOperand())) {
                                    if (PointerMap[SI->getPointerOperand()].OriginArg != LInfo.OriginArg)
                                        continue;
                                } else {
                                    // Store to unknown pointer. 
                                    // Since our field hasn't escaped, it cannot be targeted.
                                    continue;
                                }
                            } else if (auto* CB = dyn_cast<CallBase>(Write)) {
                                // Calls can only mutate if the pointer escaped.
                                continue;
                            }

                            auto ModRef = AA.getModRefInfo(Write, MemoryLocation::get(LI));
                            if (isModSet(ModRef)) {
                                ActuallyMutated = true;
                                break;
                            }
                        }
                    }

                    if (!ActuallyMutated) {
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
    // Investigation: count !invariant.load annotations added to this function.
    unsigned Annotated = 0;
    for (auto &BB : F)
        for (auto &I : BB)
            if (auto *LI = dyn_cast<LoadInst>(&I))
                if (LI->getMetadata(LLVMContext::MD_invariant_load))
                    ++Annotated;
    if (Annotated > 0)
        llvm::errs() << "[CRS-STAT] StaticMutability: fn=" << F.getName()
                     << " annotated_invariant_loads=" << Annotated << "\n";
    return PreservedAnalyses::all();
}

} // namespace clangRuntimeSpecializer
