#include "InvariantLoadToConstant.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/DenseMap.h"
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>

namespace clangRuntimeSpecializer {

using namespace llvm;

// Returns true if [addr, addr+size) is mapped and readable in the current process.
// Uses mincore(2): returns ENOMEM if any page in the range is not mapped.
// Caches results within this call to avoid repeated syscalls for the same page.
static bool isReadable(uintptr_t addr, size_t size,
                       llvm::DenseMap<uintptr_t, bool> &PageCache) {
    if (addr == 0 || size == 0)
        return false;
    static const long PageSize = ::sysconf(_SC_PAGESIZE);
    uintptr_t pageStart = addr & ~(static_cast<uintptr_t>(PageSize) - 1);
    uintptr_t pageEnd   = (addr + size + PageSize - 1) & ~(static_cast<uintptr_t>(PageSize) - 1);
    for (uintptr_t page = pageStart; page < pageEnd; page += PageSize) {
        auto It = PageCache.find(page);
        if (It != PageCache.end()) {
            if (!It->second) return false;
            continue;
        }
        unsigned char vec = 0;
        bool mapped = (::mincore(reinterpret_cast<void *>(page),
                                 static_cast<size_t>(PageSize), &vec) == 0);
        PageCache[page] = mapped;
        if (!mapped) return false;
    }
    return true;
}

PreservedAnalyses InvariantLoadToConstantPass::run(Function &F, FunctionAnalysisManager &FAM) {
    bool Changed = false;
    const DataLayout &DL = F.getParent()->getDataLayout();
    llvm::DenseMap<uintptr_t, bool> PageCache;
    
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
            // Use getSExtValue so negative GEP offsets (two's complement) subtract
            // correctly rather than wrapping to a huge positive address.
            uintptr_t FinalAddr = BaseAddr + static_cast<uintptr_t>(Offset.getSExtValue());
            Type *LoadTy = LI->getType();
            Constant *ConstVal = nullptr;

            if (LoadTy->isIntegerTy()) {
                unsigned BitWidth = LoadTy->getIntegerBitWidth();
                if (BitWidth <= 64) {
                    size_t ReadSize = (BitWidth + 7) / 8;
                    if (!isReadable(FinalAddr, ReadSize, PageCache)) continue;
                    uint64_t Value = 0;
                    switch (BitWidth) {
                        case 1:  Value = (*reinterpret_cast<uint8_t*>(FinalAddr)) & 1; break;
                        case 8:  Value = *reinterpret_cast<uint8_t*>(FinalAddr); break;
                        case 16: Value = *reinterpret_cast<uint16_t*>(FinalAddr); break;
                        case 32: Value = *reinterpret_cast<uint32_t*>(FinalAddr); break;
                        case 64: Value = *reinterpret_cast<uint64_t*>(FinalAddr); break;
                        default: {
                            std::memcpy(&Value, reinterpret_cast<void*>(FinalAddr), ReadSize);
                        }
                    }
                    ConstVal = ConstantInt::get(LoadTy, Value);
                }
            } else if (LoadTy->isFloatTy()) {
                if (!isReadable(FinalAddr, sizeof(float), PageCache)) continue;
                ConstVal = ConstantFP::get(LoadTy, *reinterpret_cast<float*>(FinalAddr));
            } else if (LoadTy->isDoubleTy()) {
                if (!isReadable(FinalAddr, sizeof(double), PageCache)) continue;
                ConstVal = ConstantFP::get(LoadTy, *reinterpret_cast<double*>(FinalAddr));
            } else if (LoadTy->isPointerTy()) {
                if (!isReadable(FinalAddr, DL.getPointerSize(), PageCache)) continue;
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
