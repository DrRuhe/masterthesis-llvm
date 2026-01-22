#include "RuntimeSpecializerPass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassBuilder.h"


using namespace llvm;


static void printSpecializationInfo(Value *Instance, CallInst *MemberCall) {
  errs() << "--- Specialization Info ---\n";

  // 1. Instanz Info
  Type *InstTy = Instance->getType();
  if (InstTy->isPointerTy()) {
    // Versuche den Typnamen zu verschönern (C++ Demangling passiert hier nicht,
    // aber wir sehen den IR-Typ)

    errs() << "Instance: '" << Instance->getName() << "' of type " << Instance->getType() <<"\n";
  }

  // 2. Memberfunktion Info
  Function *Callee = MemberCall->getCalledFunction();
  if (Callee) {
    errs() << "Called member function: '" << Callee->getName() << "'\n";
  } else {
    errs() << "Called member function via pointer/virtual call\n";
  }

  // 3. Argumente (beachte: Arg 0 ist bei Member-Calls meist 'this')
  errs() << "Arguments:\n";
  for (unsigned i = 1; i < MemberCall->arg_size(); ++i) {
    errs() << "  Arg " << i << ": " << *MemberCall->getArgOperand(i) << "\n";
  }
  errs() << "---------------------------\n";
}

static void reportInvalidAnnotation(Function &F, CallInst *CI, StringRef Msg) {
  F.getContext().diagnose(DiagnosticInfoUnsupported(
      F, "RuntimeSpecializer: " + Msg, CI->getDebugLoc(), DS_Warning));
}

PreservedAnalyses RuntimeSpecializerPass::run(Module &M, ModuleAnalysisManager &AM) {
  bool Changed = false;
  bool FoundSpecializableCode = false;

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

        // 2. Finde den Ursprung des annotierten Wertes (Backtracking)
        Value *AnnotatedPtr = CI->getArgOperand(0)->stripPointerCasts();

        CallInst *TargetCall = nullptr;
        // Suche nach dem Store, der das Resultat des Aufrufs in die Variable schreibt
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

        Value *Instance = TargetCall->getArgOperand(0)->stripPointerCasts();

        // 4. Ausgabe der gesammelten Infos
        printSpecializationInfo(Instance, TargetCall);
        FoundSpecializableCode = true;
        Changed = true;
      }
    }
  }

  if (FoundSpecializableCode) {
    static constexpr const char *kPtrName  = "RuntimeSpecializeableIR_ptr";
    static constexpr const char *kLenName  = "RuntimeSpecializeableIR_len";

    // These should be stable ABI-style symbols consumable by a shared library.
    if (!M.getNamedGlobal(kPtrName)) {
      LLVMContext &Ctx = M.getContext();
      PointerType *PtrTy = PointerType::getUnqual(Ctx);
      auto *PtrGV = new GlobalVariable(
          M,
          PtrTy,
          /*isConstant=*/true,
          GlobalValue::ExternalLinkage,
          /*Initializer=*/ConstantPointerNull::get(PtrTy),
          kPtrName);
      PtrGV->setUnnamedAddr(GlobalValue::UnnamedAddr::None);
      Changed = true;
    }

    if (!M.getNamedGlobal(kLenName)) {
      LLVMContext &Ctx = M.getContext();
      Type *LenTy = Type::getInt64Ty(Ctx);
      auto *LenGV = new GlobalVariable(
          M,
          LenTy,
          /*isConstant=*/true,
          GlobalValue::ExternalLinkage,
          /*Initializer=*/ConstantInt::get(LenTy, 0),
          kLenName);
      LenGV->setUnnamedAddr(GlobalValue::UnnamedAddr::None);
      Changed = true;
    }
  }

  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
