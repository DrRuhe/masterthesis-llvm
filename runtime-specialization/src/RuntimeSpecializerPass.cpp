#include "RuntimeSpecializerPass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"

using namespace llvm;


static void reportInvalidAnnotation(Function &F, CallInst *CI, StringRef Msg) {
  F.getContext().diagnose(DiagnosticInfoUnsupported(
      F, "RuntimeSpecializer: " + Msg, CI->getDebugLoc(), DS_Warning));
}

static std::pair<GlobalVariable*, GlobalVariable*> getOrCreateIRDumpGlobals(Module &M) {
  static constexpr const char *kPtrName  = "RuntimeSpecializeableIR_ptr";
  static constexpr const char *kLenName  = "RuntimeSpecializeableIR_len";

  LLVMContext &Ctx = M.getContext();

  GlobalVariable *PtrGV = M.getNamedGlobal(kPtrName);
  if (!PtrGV) {
    PointerType *PtrTy = PointerType::getUnqual(Ctx); // i8*
    PtrGV = new GlobalVariable(
        M,
        PtrTy,
        /*isConstant=*/true,
        GlobalValue::ExternalLinkage,
        /*Initializer=*/ConstantPointerNull::get(PtrTy),
        kPtrName);
    PtrGV->setUnnamedAddr(GlobalValue::UnnamedAddr::None);
  }

  GlobalVariable *LenGV = M.getNamedGlobal(kLenName);
  if (!LenGV) {
    Type *LenTy = Type::getInt64Ty(Ctx);
    LenGV = new GlobalVariable(
        M,
        LenTy,
        /*isConstant=*/true,
        GlobalValue::ExternalLinkage,
        /*Initializer=*/ConstantInt::get(LenTy, 0),
        kLenName);
    LenGV->setUnnamedAddr(GlobalValue::UnnamedAddr::None);
  }

  return {PtrGV, LenGV};
}
static FunctionCallee getOrCreateRuntimeHook(Module &M) {
  // Opaque pointers: ptr ist ungetypt -> wir modellieren args einfach als ptr.
  //
  // C ABI:
  // void* __runtime_specialize_hook(const char* function_ref,
  //                                uint64_t argc,
  //                                void* args,            // ptr auf args[0]
  //                                const void* ir_dump,
  //                                uint64_t ir_dump_len);
  LLVMContext &Ctx = M.getContext();

  Type *PtrTy = PointerType::getUnqual(Ctx); // ptr
  Type *I64Ty = Type::getInt64Ty(Ctx);

  FunctionType *FTy = FunctionType::get(
      /*Result=*/PtrTy,
      {PtrTy, I64Ty, PtrTy, PtrTy, I64Ty},
      /*isVarArg=*/false);

  return M.getOrInsertFunction("llvmRuntimeSpecializationEntrypoint", FTy);
}

static Value *getOrCreateFunctionNameCStringPtr(Module &M, IRBuilder<> &B, StringRef Name) {
  LLVMContext &Ctx = M.getContext();
  Type *I32Ty = Type::getInt32Ty(Ctx);

  // Deterministischer Name, damit wir mehrfachen Output deduplizieren können.
  // (Hash reicht hier völlig; Kollision wäre theoretisch möglich, praktisch sehr unwahrscheinlich.)
  uint64_t H = hash_value(Name);
  std::string GVName = (Twine("rs.func.") + Twine::utohexstr(H)).str();

  if (auto *Existing = M.getGlobalVariable(GVName, /*AllowInternal=*/true)) {
    Value *Zero32 = ConstantInt::get(I32Ty, 0);
    return B.CreateInBoundsGEP(
        Existing->getValueType(),
        Existing,
        {Zero32, Zero32},
        "rs.func.ptr");
  }

  Constant *Init = ConstantDataArray::getString(Ctx, Name, /*AddNull=*/true);
  auto *GV = new GlobalVariable(
      M,
      Init->getType(),
      /*isConstant=*/true,
      GlobalValue::PrivateLinkage,
      Init,
      GVName);
  GV->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
  GV->setAlignment(Align(1));

  Value *Zero32 = ConstantInt::get(I32Ty, 0);
  return B.CreateInBoundsGEP(
      GV->getValueType(),
      GV,
      {Zero32, Zero32},
      "rs.func.ptr");
}



PreservedAnalyses RuntimeSpecializerPass::run(Module &M, ModuleAnalysisManager &AM) {
  bool Changed = false;

  for (auto &F : M) {
    for (auto &BB : F) {
      for (auto &I : BB) {
        auto *CI = dyn_cast<CallInst>(&I);
        if (!CI) continue;

        Function *Callee = CI->getCalledFunction();
        if (!Callee || !Callee->getName().starts_with("llvm.var.annotation"))
          continue;

        // 1. Validiere Annotation String
        auto *AnnotationOp = CI->getArgOperand(1)->stripPointerCasts();
        auto *GV = dyn_cast<GlobalVariable>(AnnotationOp);
        if (!GV) continue;
        auto *CDA = dyn_cast<ConstantDataArray>(GV->getInitializer());
        if (!CDA || !CDA->getAsString().starts_with("specialize"))
          continue;

        // 2. Finde die Store Instruction und die verbundene Call instruction indem durch die user der des AnnotatedPtrs interiert wird.
        Value *AnnotatedPtr = CI->getArgOperand(0)->stripPointerCasts();

        CallInst *TargetCall = nullptr;
        for (User *U : AnnotatedPtr->users()) {
          if (auto *SI = dyn_cast<StoreInst>(U)) {
            if (auto *Call = dyn_cast<CallInst>(SI->getValueOperand())) {
              TargetCall = Call;
              break;
            }
          }
        }

        if (!TargetCall) {
          reportInvalidAnnotation(F, CI, "Annotation muss direkt an einer Variablendeklaration mit Funktionsaufruf stehen.");
          continue;
        }

        // 3. Prüfe ob es ein Member-Call ist (erstes Argument ist 'this' Pointer)
        if (TargetCall->arg_size() < 1) {
          reportInvalidAnnotation(F, CI, "Aufruf scheint keine Memberfunktion einer Instanz zu sein.");
          continue;
        }

        // --- NEU: Rewrite/Instrumentierung des annotierten Calls ---------------
        Function *CalledFn = TargetCall->getCalledFunction();
        if (!CalledFn) {
          reportInvalidAnnotation(F, CI, "Indirekte/virtuelle Calls werden aktuell nicht unterstützt (getCalledFunction() == null).");
          continue;
        }

        IRBuilder<> B(TargetCall);

        auto [IRPtrGV, IRLenGV] = getOrCreateIRDumpGlobals(M);
        FunctionCallee Hook = getOrCreateRuntimeHook(M);

        LLVMContext &Ctx = M.getContext();
        Type *PtrTy = PointerType::getUnqual(Ctx); // ptr (opaque)
        Type *I64Ty = Type::getInt64Ty(Ctx);

        const uint64_t Argc = TargetCall->arg_size();
        Value *ArgcV = ConstantInt::get(I64Ty, Argc);

        // Dynamisches args-array: alloca ptr, i64 Argc
        AllocaInst *ArgsArrAlloca = B.CreateAlloca(PtrTy, ArgcV, "rs.args");

        for (uint64_t i = 0; i < Argc; i++) {
          Value *ArgV = TargetCall->getArgOperand(i);
          Type *ArgTy = ArgV->getType();

          // Pro Argument ein Stack-Slot, damit wir eine Adresse (&arg) bekommen.
          AllocaInst *ArgSlot = B.CreateAlloca(ArgTy, nullptr, Twine("rs.arg.") + Twine(i));
          B.CreateStore(ArgV, ArgSlot);

          Value *ArgSlotPtr = B.CreateBitCast(ArgSlot, PtrTy, Twine("rs.arg.") + Twine(i) + ".ptr");

          Value *Idx = ConstantInt::get(I64Ty, i);
          Value *ElemPtr = B.CreateInBoundsGEP(PtrTy, ArgsArrAlloca, Idx, Twine("rs.args.gep.") + Twine(i));
          B.CreateStore(ArgSlotPtr, ElemPtr);
        }

        // function_ref (dedupliziert)
        Value *FuncNamePtr = getOrCreateFunctionNameCStringPtr(M, B, CalledFn->getName());

        Value *IRDumpPtr = B.CreateLoad(PtrTy, IRPtrGV, "rs.irdump.ptr");
        Value *IRDumpLen = B.CreateLoad(I64Ty, IRLenGV, "rs.irdump.len");

        CallInst *HookRes = B.CreateCall(Hook, {FuncNamePtr, ArgcV, ArgsArrAlloca, IRDumpPtr, IRDumpLen}, "rs.hook.res");
        // parameterlose Ersatzfunktion: RetTy ()
        Type *RetTy = TargetCall->getType();
        FunctionType *ZeroArgFTy = FunctionType::get(RetTy, /*isVarArg=*/false);

        // Opaque pointers: Callee ist ptr, Typinfo kommt über FunctionType beim CreateCall
        Value *SpecFnPtr = B.CreateBitCast(HookRes, PtrTy, "rs.spec.fn");
        CallInst *NewCall = B.CreateCall(ZeroArgFTy, SpecFnPtr, {}, "rs.spec.call");

        if (!RetTy->isVoidTy())
          TargetCall->replaceAllUsesWith(NewCall);

        NewCall->setCallingConv(TargetCall->getCallingConv());
        TargetCall->eraseFromParent();

        Changed = true;
      }
    }
  }

  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}