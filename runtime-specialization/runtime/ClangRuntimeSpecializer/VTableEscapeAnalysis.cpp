#include "VTableEscapeAnalysis.h"
#include "StaticMutabilityAnalysis.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/ModRef.h"
#include <dlfcn.h>
#include <map>

namespace clangRuntimeSpecializer {

using namespace llvm;

std::map<void*, std::string> VTableEscapeAnalysis::AddressToFunctionName;
bool VTableEscapeAnalysis::initialized = false;

static void* resolveValueToConstant(Value *V, const DataLayout &DL, int Depth = 0) {
    if (!V) return nullptr;
    if (Depth > 10) {
        return nullptr;
    }
    V = V->stripPointerCasts();
    if (auto *CI = dyn_cast<ConstantInt>(V)) {
        return reinterpret_cast<void*>(static_cast<uintptr_t>(CI->getZExtValue()));
    }
    if (auto *CE = dyn_cast<ConstantExpr>(V)) {
        if (CE->getOpcode() == Instruction::IntToPtr) {
            if (auto *AddrCI = dyn_cast<ConstantInt>(CE->getOperand(0))) {
                return reinterpret_cast<void*>(static_cast<uintptr_t>(AddrCI->getZExtValue()));
            }
        }
    }
    if (auto *I2P = dyn_cast<IntToPtrInst>(V)) {
        if (auto *AddrCI = dyn_cast<ConstantInt>(I2P->getOperand(0))) {
            return reinterpret_cast<void*>(static_cast<uintptr_t>(AddrCI->getZExtValue()));
        }
    }

    // Handle GEP
    if (auto *GEP = dyn_cast<GetElementPtrInst>(V)) {
        void *Base = resolveValueToConstant(GEP->getPointerOperand(), DL, Depth + 1);
        if (Base) {
            APInt Offset(DL.getPointerSizeInBits(), 0);
            if (GEP->accumulateConstantOffset(DL, Offset)) {
                return static_cast<char*>(Base) + Offset.getZExtValue();
            }
        }
    }

    // Handle Load
    if (auto *LI = dyn_cast<LoadInst>(V)) {
        void *Ptr = resolveValueToConstant(LI->getPointerOperand(), DL, Depth + 1);
        if (Ptr) {
            return *reinterpret_cast<void**>(Ptr);
        }
    }

    return nullptr;
}

void VTableEscapeAnalysis::ensureInitialized(Module &M) {
    if (initialized) return;
    const DataLayout &DL = M.getDataLayout();

    // 1. Try direct dlsym for all functions
    for (Function &F : M) {
        if (!F.hasName()) continue;
        void* Addr = dlsym(RTLD_DEFAULT, F.getName().str().c_str());
        if (Addr) {
            AddressToFunctionName[Addr] = F.getName().str();
        }
    }

    // 2. Try to find virtual functions by looking into vtables
    for (GlobalVariable &GV : M.globals()) {
        if (!GV.hasName() || !GV.getName().starts_with("_ZTV")) continue;

        void* GVAddr = dlsym(RTLD_DEFAULT, GV.getName().str().c_str());
        if (!GVAddr) continue;

        if (GV.hasInitializer()) {
            Constant *Init = GV.getInitializer();
            
            auto mapVTableElements = [&](Constant *C, uint64_t BaseOffset) {
                if (auto *ST = dyn_cast<ConstantStruct>(C)) {
                    for (unsigned i = 0; i < ST->getNumOperands(); ++i) {
                        uint64_t Offset = DL.getStructLayout(cast<StructType>(ST->getType()))->getElementOffset(i);
                        Constant *Elem = ST->getOperand(i);
                        if (auto *F = dyn_cast<Function>(Elem->stripPointerCasts())) {
                            void *FuncAddr = *reinterpret_cast<void**>(static_cast<char*>(GVAddr) + BaseOffset + Offset);
                            AddressToFunctionName[FuncAddr] = F->getName().str();
                        } else if (auto *InnerArray = dyn_cast<ConstantArray>(Elem)) {
                            for (unsigned j = 0; j < InnerArray->getNumOperands(); ++j) {
                                uint64_t ElemOffset = j * DL.getTypeAllocSize(InnerArray->getType()->getElementType());
                                if (auto *F = dyn_cast<Function>(InnerArray->getOperand(j)->stripPointerCasts())) {
                                    void *FuncAddr = *reinterpret_cast<void**>(static_cast<char*>(GVAddr) + BaseOffset + Offset + ElemOffset);
                                    AddressToFunctionName[FuncAddr] = F->getName().str();
                                }
                            }
                        }
                    }
                } else if (auto *AT = dyn_cast<ConstantArray>(C)) {
                    for (unsigned i = 0; i < AT->getNumOperands(); ++i) {
                        uint64_t Offset = i * DL.getTypeAllocSize(AT->getType()->getElementType());
                        if (auto *F = dyn_cast<Function>(AT->getOperand(i)->stripPointerCasts())) {
                            void *FuncAddr = *reinterpret_cast<void**>(static_cast<char*>(GVAddr) + BaseOffset + Offset);
                            AddressToFunctionName[FuncAddr] = F->getName().str();
                        }
                    }
                }
            };
            mapVTableElements(Init, 0);
        }
    }
    initialized = true;
}

Function* VTableEscapeAnalysis::getFunctionAtAddress(void* Addr, Module &M) {
    auto it = AddressToFunctionName.find(Addr);
    if (it != AddressToFunctionName.end()) {
        return M.getFunction(it->second);
    }
    return nullptr;
}

PreservedAnalyses VTableEscapeAnalysis::run(Module &M, ModuleAnalysisManager &MAM) {
    ensureInitialized(M);
    const DataLayout &DL = M.getDataLayout();
    bool Changed = false;

    std::vector<Function*> Worklist;
    for (Function &F : M) {
        if (!F.isDeclaration()) Worklist.push_back(&F);
    }

    bool LocalChanged;
    do {
        LocalChanged = false;
        for (Function *F : Worklist) {
            for (BasicBlock &BB : *F) {
                for (Instruction &I : BB) {
                    auto *CB = dyn_cast<CallBase>(&I);
                    if (!CB) continue;

                    Function *TargetF = CB->getCalledFunction();
                    if (!TargetF && CB->isIndirectCall()) {
                        Value *Callee = CB->getCalledOperand();
                        void *FuncPtr = resolveValueToConstant(Callee, DL);
                        if (FuncPtr) {
                            TargetF = getFunctionAtAddress(FuncPtr, M);
                        }
                    }

                    if (TargetF) {
                        bool TargetIsDirectCall = (CB->getCalledOperand() == TargetF);
                        
                        if (!TargetIsDirectCall) {
                            CB->setCalledFunction(TargetF);
                            Changed = true;
                            LocalChanged = true;
                            TargetIsDirectCall = true;
                        }

                        if (!TargetF->isDeclaration()) {
                            auto PointerMap = StaticMutabilityAnalysis::runCaptureAnalysis(*TargetF);
                            
                            for (unsigned i = 0; i < CB->arg_size(); ++i) {
                                if (i >= TargetF->arg_size()) break;
                                Argument *Param = TargetF->getArg(i);

                                if (!CB->doesNotCapture(i)) {
                                    if (PointerMap.count(Param) && !PointerMap[Param].Escaped) {
                                        CB->addParamAttr(i, Attribute::getWithCaptureInfo(CB->getContext(), CaptureInfo::none()));
                                        LocalChanged = true;
                                        Changed = true;
                                    }
                                }

                                if (!CB->onlyReadsMemory(i)) {
                                    if (TargetF->hasParamAttribute(i, Attribute::ReadOnly)) {
                                        CB->addParamAttr(i, Attribute::ReadOnly);
                                        LocalChanged = true;
                                        Changed = true;
                                    } else if (TargetF->hasParamAttribute(i, Attribute::ReadNone)) {
                                        CB->addParamAttr(i, Attribute::ReadNone);
                                        LocalChanged = true;
                                        Changed = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    } while (LocalChanged);

    return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

} // namespace clangRuntimeSpecializer
