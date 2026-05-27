#pragma once

#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/ADT/DenseMap.h"
#include <optional>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>

namespace clangRuntimeSpecializer {

// Returns true if [addr, addr+size) is mapped and readable in the current process.
// Uses mincore(2): returns ENOMEM if any page in the range is not mapped.
// Caches results within one resolution pass to avoid repeated syscalls for the same page.
inline bool isAddressReadable(uintptr_t addr, size_t size,
                              llvm::DenseMap<uintptr_t, bool>& PageCache) {
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
        bool mapped = (::mincore(reinterpret_cast<void*>(page),
                                 static_cast<size_t>(PageSize), &vec) == 0);
        PageCache[page] = mapped;
        if (!mapped) return false;
    }
    return true;
}

// Resolve a load instruction whose pointer chain resolves to a constant host address.
// Returns a Constant* if the load value can be read from host memory, std::nullopt otherwise.
// Only handles loads marked !invariant.load (caller must check).
// PageCache should be reused across calls within the same transformation pass.
inline std::optional<llvm::Constant*> resolveInvariantLoadToConstant(
        llvm::LoadInst& LI,
        llvm::DenseMap<uintptr_t, bool>& PageCache) {
    using namespace llvm;
    const DataLayout& DL = LI.getModule()->getDataLayout();
    Value* Ptr = LI.getPointerOperand();

    APInt Offset(DL.getPointerSizeInBits(), 0);
    Value* Base = Ptr->stripAndAccumulateConstantOffsets(DL, Offset, true);

    uintptr_t BaseAddr = 0;
    if (auto* CI = dyn_cast<ConstantInt>(Base)) {
        BaseAddr = static_cast<uintptr_t>(CI->getZExtValue());
    } else if (auto* CE = dyn_cast<ConstantExpr>(Base)) {
        if (CE->getOpcode() == Instruction::IntToPtr) {
            if (auto* AddrCI = dyn_cast<ConstantInt>(CE->getOperand(0)))
                BaseAddr = static_cast<uintptr_t>(AddrCI->getZExtValue());
        }
    } else if (auto* I2P = dyn_cast<IntToPtrInst>(Base)) {
        if (auto* AddrCI = dyn_cast<ConstantInt>(I2P->getOperand(0)))
            BaseAddr = static_cast<uintptr_t>(AddrCI->getZExtValue());
    }

    if (BaseAddr == 0)
        return std::nullopt;

    // Use getSExtValue so negative GEP offsets subtract correctly.
    uintptr_t FinalAddr = BaseAddr + static_cast<uintptr_t>(Offset.getSExtValue());
    Type* LoadTy = LI.getType();
    LLVMContext& Ctx = LI.getContext();

    if (LoadTy->isIntegerTy()) {
        unsigned BitWidth = LoadTy->getIntegerBitWidth();
        if (BitWidth > 64) return std::nullopt;
        size_t ReadSize = (BitWidth + 7) / 8;
        if (!isAddressReadable(FinalAddr, ReadSize, PageCache)) return std::nullopt;
        uint64_t Value = 0;
        switch (BitWidth) {
            case 1:  Value = (*reinterpret_cast<uint8_t*>(FinalAddr)) & 1; break;
            case 8:  Value = *reinterpret_cast<uint8_t*>(FinalAddr); break;
            case 16: Value = *reinterpret_cast<uint16_t*>(FinalAddr); break;
            case 32: Value = *reinterpret_cast<uint32_t*>(FinalAddr); break;
            case 64: Value = *reinterpret_cast<uint64_t*>(FinalAddr); break;
            default: std::memcpy(&Value, reinterpret_cast<void*>(FinalAddr), ReadSize); break;
        }
        return ConstantInt::get(LoadTy, Value);
    } else if (LoadTy->isFloatTy()) {
        if (!isAddressReadable(FinalAddr, sizeof(float), PageCache)) return std::nullopt;
        return ConstantFP::get(LoadTy, *reinterpret_cast<float*>(FinalAddr));
    } else if (LoadTy->isDoubleTy()) {
        if (!isAddressReadable(FinalAddr, sizeof(double), PageCache)) return std::nullopt;
        return ConstantFP::get(LoadTy, *reinterpret_cast<double*>(FinalAddr));
    } else if (LoadTy->isPointerTy()) {
        if (!isAddressReadable(FinalAddr, DL.getPointerSize(), PageCache)) return std::nullopt;
        uintptr_t PtrVal = *reinterpret_cast<uintptr_t*>(FinalAddr);
        return ConstantExpr::getIntToPtr(
            ConstantInt::get(Type::getIntNTy(Ctx, DL.getPointerSizeInBits()),
                             static_cast<uint64_t>(PtrVal)),
            LoadTy);
    }
    return std::nullopt;
}

} // namespace clangRuntimeSpecializer
