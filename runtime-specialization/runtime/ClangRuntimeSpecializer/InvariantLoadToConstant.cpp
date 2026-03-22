#include "InvariantLoadToConstant.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include <cstring>

namespace clangRuntimeSpecializer {

using namespace llvm;

PreservedAnalyses InvariantLoadToConstantPass::run(Function &F, FunctionAnalysisManager &FAM) {
    bool Changed = false;
    const DataLayout &DL = F.getParent()->getDataLayout();
    
    std::vector<LoadInst*> ToReplace;
    for (auto &BB : F) {
        for (auto &I : BB) {
            if (auto *LI = dyn_cast<LoadInst>(&I)) {
                if (LI->getMetadata(LLVMContext::MD_invariant_load)) {
                    ToReplace.push_back(LI);
                }
            }
        }
    }

    for (auto *LI : ToReplace) {
        Value *Ptr = LI->getPointerOperand();
        
        APInt Offset(DL.getPointerSizeInBits(), 0);
        Value *Base = Ptr->stripAndAccumulateConstantOffsets(DL, Offset, true);
        
        uintptr_t BaseAddr = 0;
        if (auto *CI = dyn_cast<ConstantInt>(Base)) {
            BaseAddr = static_cast<uintptr_t>(CI->getZExtValue());
        } else if (auto *CE = dyn_cast<ConstantExpr>(Base)) {
            if (CE->getOpcode() == Instruction::IntToPtr) {
                if (auto *AddrCI = dyn_cast<ConstantInt>(CE->getOperand(0))) {
                    BaseAddr = static_cast<uintptr_t>(AddrCI->getZExtValue());
                }
            }
        } else if (auto *I2P = dyn_cast<IntToPtrInst>(Base)) {
            if (auto *AddrCI = dyn_cast<ConstantInt>(I2P->getOperand(0))) {
                BaseAddr = static_cast<uintptr_t>(AddrCI->getZExtValue());
            }
        }

        if (BaseAddr != 0) {
            uintptr_t FinalAddr = BaseAddr + static_cast<uintptr_t>(Offset.getZExtValue());
            Type *LoadTy = LI->getType();
            Constant *ConstVal = nullptr;

            if (LoadTy->isIntegerTy()) {
                unsigned BitWidth = LoadTy->getIntegerBitWidth();
                if (BitWidth <= 64) {
                    uint64_t Value = 0;
                    switch (BitWidth) {
                        case 1:  Value = (*reinterpret_cast<uint8_t*>(FinalAddr)) & 1; break;
                        case 8:  Value = *reinterpret_cast<uint8_t*>(FinalAddr); break;
                        case 16: Value = *reinterpret_cast<uint16_t*>(FinalAddr); break;
                        case 32: Value = *reinterpret_cast<uint32_t*>(FinalAddr); break;
                        case 64: Value = *reinterpret_cast<uint64_t*>(FinalAddr); break;
                        default: {
                            std::memcpy(&Value, reinterpret_cast<void*>(FinalAddr), (BitWidth + 7) / 8);
                        }
                    }
                    ConstVal = ConstantInt::get(LoadTy, Value);
                }
            } else if (LoadTy->isFloatTy()) {
                ConstVal = ConstantFP::get(LoadTy, *reinterpret_cast<float*>(FinalAddr));
            } else if (LoadTy->isDoubleTy()) {
                ConstVal = ConstantFP::get(LoadTy, *reinterpret_cast<double*>(FinalAddr));
            } else if (LoadTy->isPointerTy()) {
                uintptr_t PtrVal = *reinterpret_cast<uintptr_t*>(FinalAddr);
                ConstVal = ConstantExpr::getIntToPtr(ConstantInt::get(Type::getIntNTy(F.getContext(), DL.getPointerSizeInBits()), static_cast<uint64_t>(PtrVal)), LoadTy);
            }

            if (ConstVal) {
                LI->replaceAllUsesWith(ConstVal);
                LI->eraseFromParent();
                Changed = true;
            }
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
