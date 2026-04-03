#include "DevirtualizeConstantVtableCalls.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include <dlfcn.h>
#include <map>

namespace clangRuntimeSpecializer {

using namespace llvm;

std::map<void *, std::string> DevirtualizeConstantVtableCallsPass::AddressToFunctionName;
bool DevirtualizeConstantVtableCallsPass::Initialized = false;

// ---------------------------------------------------------------------------
// Address resolution
// ---------------------------------------------------------------------------

// Follow a value through loads and GEPs whose base addresses are all
// compile-time constants.  Returns the runtime address the value evaluates to,
// or nullptr if any step cannot be resolved to a constant.
static void *resolveToAddress(Value *V, const DataLayout &DL, int Depth = 0) {
    if (!V || Depth > 10)
        return nullptr;

    V = V->stripPointerCasts();

    if (auto *CE = dyn_cast<ConstantExpr>(V)) {
        // inttoptr(constant) – the common case after argument specialisation
        if (CE->getOpcode() == Instruction::IntToPtr) {
            if (auto *CI = dyn_cast<ConstantInt>(CE->getOperand(0)))
                return reinterpret_cast<void *>(
                    static_cast<uintptr_t>(CI->getZExtValue()));
        }
        // GEP over a global (e.g. getelementptr(@_ZTVFoo, i64 16)) –
        // the vtable pointer stored into stack-allocated objects.
        if (CE->getOpcode() == Instruction::GetElementPtr) {
            void *Base = resolveToAddress(CE->getOperand(0), DL, Depth + 1);
            if (Base) {
                APInt Offset(DL.getPointerSizeInBits(), 0);
                if (cast<GEPOperator>(CE)->accumulateConstantOffset(DL, Offset))
                    return static_cast<char *>(Base) + Offset.getZExtValue();
            }
        }
    }

    // Global variable – look up its runtime address via dlsym.
    if (auto *GV = dyn_cast<GlobalVariable>(V)) {
        return dlsym(RTLD_DEFAULT, GV->getName().str().c_str());
    }

    // GEP with a constant base and constant indices – compute the final address
    if (auto *GEP = dyn_cast<GetElementPtrInst>(V)) {
        void *Base = resolveToAddress(GEP->getPointerOperand(), DL, Depth + 1);
        if (Base) {
            APInt Offset(DL.getPointerSizeInBits(), 0);
            if (GEP->accumulateConstantOffset(DL, Offset))
                return static_cast<char *>(Base) + Offset.getZExtValue();
        }
        return nullptr;
    }

    // Load from a constant pointer – dereference it at JIT time
    if (auto *LI = dyn_cast<LoadInst>(V)) {
        void *Ptr = resolveToAddress(LI->getPointerOperand(), DL, Depth + 1);
        if (Ptr)
            return *reinterpret_cast<void **>(Ptr);

        // Load from a stack alloca: find the single direct store and substitute
        // its value.  This covers the vtable pointer stored by the constructor
        // into a stack-allocated object (store ptr gep(@_ZTVFoo, 16), ptr %obj).
        if (auto *AI = dyn_cast<AllocaInst>(
                LI->getPointerOperand()->stripPointerCasts())) {
            Value *StoredValue = nullptr;
            for (User *U : AI->users()) {
                auto *SI = dyn_cast<StoreInst>(U);
                if (!SI || SI->getPointerOperand() != AI)
                    continue;
                if (StoredValue) {
                    StoredValue = nullptr; // multiple stores – give up
                    break;
                }
                StoredValue = SI->getValueOperand();
            }
            if (StoredValue)
                return resolveToAddress(StoredValue, DL, Depth + 1);
        }
        return nullptr;
    }

    return nullptr;
}

// ---------------------------------------------------------------------------
// Address → LLVM Function map
// ---------------------------------------------------------------------------

void DevirtualizeConstantVtableCallsPass::ensureInitialized(Module &M) {
    if (Initialized)
        return;

    const DataLayout &DL = M.getDataLayout();

    // 1. Map each named function in the module to its runtime address via dlsym.
    for (Function &F : M) {
        if (!F.hasName())
            continue;
        if (void *Addr = dlsym(RTLD_DEFAULT, F.getName().str().c_str()))
            AddressToFunctionName[Addr] = F.getName().str();
    }

    // 2. Walk vtable globals and map each slot's runtime address to the
    //    corresponding LLVM function.  This covers virtual functions that may
    //    not be independently exported.
    for (GlobalVariable &GV : M.globals()) {
        if (!GV.hasName() || !GV.getName().starts_with("_ZTV"))
            continue;

        void *GVAddr = dlsym(RTLD_DEFAULT, GV.getName().str().c_str());
        if (!GVAddr || !GV.hasInitializer())
            continue;

        // Walk the initializer recursively and record every function pointer
        // together with its runtime slot address.
        std::function<void(Constant *, uint64_t)> Walk =
            [&](Constant *C, uint64_t BaseOff) {
                if (auto *ST = dyn_cast<ConstantStruct>(C)) {
                    StructLayout const *SL =
                        DL.getStructLayout(cast<StructType>(ST->getType()));
                    for (unsigned i = 0; i < ST->getNumOperands(); ++i) {
                        uint64_t Off = SL->getElementOffset(i);
                        Walk(ST->getOperand(i), BaseOff + Off);
                    }
                } else if (auto *AT = dyn_cast<ConstantArray>(C)) {
                    uint64_t ElemSz =
                        DL.getTypeAllocSize(AT->getType()->getElementType());
                    for (unsigned i = 0; i < AT->getNumOperands(); ++i)
                        Walk(AT->getOperand(i), BaseOff + i * ElemSz);
                } else if (auto *F =
                               dyn_cast<Function>(C->stripPointerCasts())) {
                    void *Slot =
                        static_cast<char *>(GVAddr) + BaseOff;
                    void *FuncAddr = *reinterpret_cast<void **>(Slot);
                    AddressToFunctionName[FuncAddr] = F->getName().str();
                }
            };

        Walk(GV.getInitializer(), 0);
    }

    Initialized = true;
}

Function *DevirtualizeConstantVtableCallsPass::getFunctionAtAddress(void *Addr,
                                                                Module &M) {
    auto It = AddressToFunctionName.find(Addr);
    if (It == AddressToFunctionName.end())
        return nullptr;
    return M.getFunction(It->second);
}

// ---------------------------------------------------------------------------
// Pass entry point
// ---------------------------------------------------------------------------

PreservedAnalyses
DevirtualizeConstantVtableCallsPass::run(Module &M, ModuleAnalysisManager &) {
    ensureInitialized(M);

    const DataLayout &DL = M.getDataLayout();
    bool Changed = false;

    // Collect (call, target) pairs first to avoid iterator invalidation.
    SmallVector<std::pair<CallBase *, Function *>, 16> ToDevirtualize;

    for (Function &F : M) {
        if (F.isDeclaration())
            continue;
        for (BasicBlock &BB : F) {
            for (Instruction &I : BB) {
                auto *CB = dyn_cast<CallBase>(&I);
                if (!CB || !CB->isIndirectCall())
                    continue;

                void *Addr =
                    resolveToAddress(CB->getCalledOperand(), DL);
                if (!Addr)
                    continue;

                Function *Target = getFunctionAtAddress(Addr, M);
                if (!Target || Target->isDeclaration())
                    continue;

                ToDevirtualize.emplace_back(CB, Target);
            }
        }
    }

    for (auto &[CB, Target] : ToDevirtualize) {
        CB->setCalledFunction(Target);
        Changed = true;
    }

    return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

} // namespace clangRuntimeSpecializer
