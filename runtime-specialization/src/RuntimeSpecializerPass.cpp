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
  // C ABI Vorschlag:
  // void* __runtime_specialize_hook(const char* function_ref,
  //                                uint64_t argc,
  //                                void** args,
  //                                const void* ir_dump,
  //                                uint64_t ir_dump_len);
  // Rückgabe: pointer auf parameterlose Funktion (als roher void*/i8*).
  LLVMContext &Ctx = M.getContext();

  Type *I8PtrTy = PointerType::getUnqual(Ctx);    // i8*
  Type *I8PtrPtrTy = PointerType::getUnqual(Ctx); // i8**
  Type *I64Ty = Type::getInt64Ty(Ctx);

  FunctionType *FTy = FunctionType::get(
      /*Result=*/I8PtrTy,
      {I8PtrTy, I64Ty, I8PtrPtrTy, I8PtrTy, I64Ty},
      /*isVarArg=*/false);

  return M.getOrInsertFunction("__runtime_specialize_hook", FTy);
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

        IRBuilder<> B(TargetCall); // Insert unmittelbar vor dem Call

        auto [IRPtrGV, IRLenGV] = getOrCreateIRDumpGlobals(M);
        FunctionCallee Hook = getOrCreateRuntimeHook(M);

        LLVMContext &Ctx = M.getContext();
        Type *I8PtrTy = PointerType::getUnqual(Ctx);
        Type *I8PtrPtrTy = PointerType::getUnqual(Ctx);
        Type *I64Ty = Type::getInt64Ty(Ctx);

        const uint64_t Argc = TargetCall->arg_size();

        ArrayType *ArgsArrTy = ArrayType::get(I8PtrTy, Argc);
        AllocaInst *ArgsArrAlloca = B.CreateAlloca(ArgsArrTy, nullptr, "rs.args");

        for (uint64_t i = 0; i < Argc; ++i) {
          Value *ArgV = TargetCall->getArgOperand(i);
          Type *ArgTy = ArgV->getType();

          AllocaInst *ArgSlot = B.CreateAlloca(ArgTy, nullptr, Twine("rs.arg.") + Twine(i));
          B.CreateStore(ArgV, ArgSlot);

          Value *ArgSlotI8 = B.CreateBitCast(ArgSlot, I8PtrTy, Twine("rs.arg.") + Twine(i) + ".i8");

          Value *Zero = ConstantInt::get(I64Ty, 0);
          Value *Idx  = ConstantInt::get(I64Ty, i);
          Value *ElemPtr = B.CreateInBoundsGEP(ArgsArrTy, ArgsArrAlloca, {Zero, Idx},
                                               Twine("rs.args.gep.") + Twine(i));

          B.CreateStore(ArgSlotI8, ElemPtr);
        }

        // CreateGlobalStringPtr ist deprecated -> CreateGlobalString + GEP auf erstes Element
        GlobalVariable *FuncNameGV = B.CreateGlobalString(CalledFn->getName(), "rs.func");
        Type *I32Ty = Type::getInt32Ty(Ctx);
        Value *Zero32 = ConstantInt::get(I32Ty, 0);
        Value *FuncNamePtr = B.CreateInBoundsGEP(
            FuncNameGV->getValueType(),
            FuncNameGV,
            {Zero32, Zero32},
            "rs.func.ptr");

        Value *ArgcV = ConstantInt::get(I64Ty, Argc);
        Value *ArgsAsI8PtrPtr = B.CreateBitCast(ArgsArrAlloca, I8PtrPtrTy, "rs.args.i8pp");

        Value *IRDumpPtr = B.CreateLoad(I8PtrTy, IRPtrGV, "rs.irdump.ptr");
        Value *IRDumpLen = B.CreateLoad(I64Ty, IRLenGV, "rs.irdump.len");

        CallInst *HookRes = B.CreateCall(Hook, {FuncNamePtr, ArgcV, ArgsAsI8PtrPtr, IRDumpPtr, IRDumpLen}, "rs.hook.res");

        Type *RetTy = TargetCall->getType();
        FunctionType *ZeroArgFTy = FunctionType::get(RetTy, /*isVarArg=*/false);

        // Typed function-pointer types sind deprecated (opaque pointers). HookRes ist bereits "ptr".
        Value *SpecFnPtr = B.CreateBitCast(HookRes, PointerType::getUnqual(Ctx), "rs.spec.fn");

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