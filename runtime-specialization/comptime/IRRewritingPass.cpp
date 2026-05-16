#include "RuntimeSpecializerPass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include <algorithm>
#include <cassert>

using namespace llvm;

// ── Call-site rewriting helpers (spec 010) ──────────────────────────────────

namespace {

struct SpecLambdaSite {
  CallBase* CI;           // CallInst or InvokeInst (inlined assertSpecialized helpers)
  Function* OpFunc;       // __crs_lambda_op_proxy instantiation (the JIT target)
  bool HasOpts;           // user-facing call included explicit Options argument
  unsigned LambdaArgIdx;  // index where resolvedName is inserted in the rewritten call
};

// Find the lambda proxy function for a specializeLambda call site.
//
// Strategy (pure LLVM API — no mangled-name parsing):
//   1. Inside SpecLambdaFn's body, find a call to forceLambdaOpEmit.
//   2. Inside forceLambdaOpEmit's body, find a call to __crs_op_hint.
//   3. Return dyn_cast<Function>(hint_call.arg(0)->stripPointerCasts()).
//
// forceLambdaOpEmit is noinline so it is not folded into SpecLambdaFn at -O0;
// the two-level traversal handles that.
Function* findLambdaProxyFunc(Function* SpecLambdaFn) {
  for (auto& BB : *SpecLambdaFn)
    for (auto& I : BB)
      if (auto* CI = dyn_cast<CallBase>(&I))
        if (auto* ForceFn = CI->getCalledFunction())
          if (!ForceFn->isDeclaration() &&
              ForceFn->getName().contains("forceLambdaOpEmit"))
            for (auto& BB2 : *ForceFn)
              for (auto& I2 : BB2)
                if (auto* CI2 = dyn_cast<CallBase>(&I2))
                  if (auto* HintFn = CI2->getCalledFunction())
                    if (HintFn->getName() == "__crs_op_hint" && CI2->arg_size() >= 1)
                      return dyn_cast<Function>(
                          CI2->getArgOperand(0)->stripPointerCasts());
  return nullptr;
}

// Find where to insert resolvedName by locating the ConstantPointerNull in the
// specializeLambdaResolved call inside SpecLambdaFn's body.
//
// The user-facing specializeLambda body calls:
//   free fn:   Resolved(sret, nullptr, lambda)        → nullptr at index 1
//   member fn: Resolved(sret, this,   nullptr, lambda) → nullptr at index 2
// That index equals LambdaArgIdx for the outer call site.
unsigned findLambdaArgIdx(Function* SpecLambdaFn) {
  for (auto& BB : *SpecLambdaFn)
    for (auto& I : BB)
      if (auto* CI = dyn_cast<CallBase>(&I))
        if (auto* F = CI->getCalledFunction())
          if (F->getName().contains("specializeLambdaResolved"))
            for (unsigned i = 0; i < CI->arg_size(); ++i)
              if (isa<ConstantPointerNull>(CI->getArgOperand(i)))
                return i;
  return 1; // fallback: free-function convention
}

// Returns true for calls to the user-facing specializeLambda overloads
// (those WITHOUT "Resolved" in the name).
bool isSpecializeLambdaUserCall(StringRef MangledName) {
  return MangledName.contains("specializeLambda") &&
         !MangledName.contains("specializeLambdaResolved");
}

// Returns true for calls to the user-facing specializeOnly/callSpecialized/
// specializeOrFallback/assertSpecializedIsEquivalent overloads that take a
// function pointer as first or second argument.
bool isSpecOnlyFuncPtrUserCall(StringRef MangledName) {
  if (MangledName.contains("Resolved")) return false;
  return MangledName.contains("specializeOnly") ||
         MangledName.contains("callSpecialized") ||
         MangledName.contains("specializeOrFallback") ||
         MangledName.contains("assertSpecializedIsEquivalent") ||
         MangledName.contains("compareFunctionInstructionCounts");
}

// Look inside a specializeLambda function body for its call to
// specializeLambdaResolved — that call exists because the user-facing overload
// calls specializeLambdaResolved(nullptr, lambda) to force template instantiation.
// Uses CallBase to handle both CallInst and InvokeInst (-fexceptions).
Function* findResolvedFuncInBody(Function* SpecLambdaFn) {
  for (auto& BB : *SpecLambdaFn)
    for (auto& I : BB)
      if (auto* CB = dyn_cast<CallBase>(&I))
        if (auto* F = CB->getCalledFunction())
          if (F->getName().contains("specializeLambdaResolved"))
            return F;
  return nullptr;
}

// For a specializeOnly/callSpecialized/specializeOrFallback/assertSpecializedIsEquivalent
// (F* func, args...) call site, find the Resolved variant by inspecting the callee's body.
// The user-facing overload calls specializeOnlyResolved / callSpecializedResolved /
// specializeOrFallbackResolved / assertSpecializedIsEquivalentResolved inside its body.
// Uses CallBase (covers both CallInst and InvokeInst) so the check works even when
// compiled with -fexceptions, which turns throwing calls into InvokeInst.
Function* findSpecOnlyResolvedInBody(Function* SpecOnlyFn) {
  for (auto& BB : *SpecOnlyFn)
    for (auto& I : BB)
      if (auto* CB = dyn_cast<CallBase>(&I))
        if (auto* F = CB->getCalledFunction())
          if (F->getName().contains("specializeOnlyResolved") ||
              F->getName().contains("callSpecializedResolved") ||
              F->getName().contains("specializeOrFallbackResolved") ||
              F->getName().contains("assertSpecializedIsEquivalentResolved") ||
              F->getName().contains("compareFunctionInstructionCountsResolved"))
            return F;
  return nullptr;
}

// Scan the arguments of a call site for one that is a compile-time constant
// function (i.e., a direct reference to a function global).  Returns the
// Function* if found, nullptr otherwise (FR-025 non-constant pointer check).
Function* findFunctionPtrArg(CallBase* CI) {
  for (unsigned i = 0; i < CI->arg_size(); ++i) {
    Value* Stripped = CI->getArgOperand(i)->stripPointerCasts();
    if (auto* F = dyn_cast<Function>(Stripped))
      return F;
  }
  return nullptr;
}

// Data for a specializeOnly/callSpecialized(F* func, args...) call site.
struct SpecFuncPtrSite {
  CallBase* CI;
  std::string ResolvedName;  // mangled name of the target function
  unsigned FuncPtrArgIdx;    // index of the function pointer in CI's arg list
};

} // namespace

PreservedAnalyses IRRewritingPass::run(Module &M, ModuleAnalysisManager &AM) {
  // Guard: if already rewritten, skip (prevents double-rewriting in LTO or
  // multi-pass scenarios).
  if (M.getNamedMetadata("crs.rewriting_done"))
    return PreservedAnalyses::all();

  LLVMContext &Ctx = M.getContext();

  // ── PHASE 0: Detect specializeLambda call sites (spec 010, FR-001/FR-008) ──
  SmallVector<SpecLambdaSite, 8> SpecLambdaSites;
  SmallPtrSet<Function*, 8> LambdaTargets;
  unsigned SiteIdx = 0;

  for (auto& F : M) {
    if (F.isDeclaration()) continue;
    if (F.getName().contains("specializeLambda") ||
        F.getName().contains("specializeOnly") ||
        F.getName().contains("callSpecialized") ||
        F.getName().contains("specializeOrFallback") ||
        F.getName().contains("assertSpecializedIsEquivalent"))
      continue;

    for (auto& BB : F) {
      for (auto& I : BB) {
        auto* CI = dyn_cast<CallBase>(&I);
        if (!CI) continue;
        auto* Callee = CI->getCalledFunction();
        if (!Callee || !isSpecializeLambdaUserCall(Callee->getName())) continue;

        unsigned LambdaArgIdx = findLambdaArgIdx(Callee);

        if (LambdaArgIdx >= CI->arg_size())
          continue;

        bool HasOpts = (LambdaArgIdx + 1 < CI->arg_size());

        Function* OpFunc = findLambdaProxyFunc(Callee);
        if (!OpFunc) {
          report_fatal_error(
              "IRRewritingPass: could not find __crs_op_hint call in forceLambdaOpEmit "
              "for specializeLambda — ensure ClangRuntimeSpecializer.h defines "
              "forceLambdaOpEmit with the sentinel pattern and the TU is compiled "
              "with -fpass-plugin");
        }

        SpecLambdaSites.push_back({CI, OpFunc, HasOpts, LambdaArgIdx});
        LambdaTargets.insert(OpFunc);
      }
    }
  }

  // ── PHASE 0b: Detect specializeOnly/callSpecialized(F* func, args...) sites ──
  SmallVector<SpecFuncPtrSite, 8> SpecFuncPtrSites;

  for (auto& F : M) {
    if (F.isDeclaration()) continue;
    if (F.getName().contains("specializeLambda") ||
        F.getName().contains("specializeOnly") ||
        F.getName().contains("callSpecialized") ||
        F.getName().contains("specializeOrFallback") ||
        F.getName().contains("assertSpecializedIsEquivalent") ||
        F.getName().contains("compareFunctionInstructionCounts"))
      continue;

    for (auto& BB : F) {
      for (auto& I : BB) {
        auto* CI = dyn_cast<CallBase>(&I);
        if (!CI) continue;
        auto* Callee = CI->getCalledFunction();
        if (!Callee) continue;
        if (!isSpecOnlyFuncPtrUserCall(Callee->getName())) continue;

        Function* TargetFn = findFunctionPtrArg(CI);
        if (!TargetFn) continue;

        unsigned FuncPtrArgIdx = 0;
        for (unsigned i = 0; i < CI->arg_size(); ++i) {
          Value* Stripped = CI->getArgOperand(i)->stripPointerCasts();
          if (dyn_cast<Function>(Stripped) == TargetFn) {
            FuncPtrArgIdx = i;
            break;
          }
        }

        SpecFuncPtrSites.push_back({CI, TargetFn->getName().str(), FuncPtrArgIdx});
      }
    }
  }

  // Promote lambda proxy functions to ExternalLinkage in M so the -O3 optimizer
  // (which runs between IRRewritingPass and IRDumpingPass) cannot prune them via
  // GlobalDCE or DeadArgElim. IRDumpingPass will reset them to InternalLinkage
  // after GlobalDCE on the clone.
  for (auto* OpFunc : LambdaTargets)
    if (!OpFunc->isDeclaration() &&
        (OpFunc->hasInternalLinkage() || OpFunc->hasPrivateLinkage() ||
         OpFunc->hasLinkOnceODRLinkage()))
      OpFunc->setLinkage(GlobalValue::ExternalLinkage);

  // FR-010: Collect specialization target function names from the ORIGINAL module
  // before any transforms. Restricted to originally-externally-visible functions
  // so every collected name is a GlobalDCE root and survives compile-time DCE.
  // Also include lambda operator() targets (which may have internal linkage).
  SmallVector<std::string, 64> FuncNames;
  for (auto &F : M) {
    if (!F.isDeclaration() && !F.getName().starts_with("__clangRS") &&
        !F.hasInternalLinkage() && !F.hasPrivateLinkage())
      FuncNames.push_back(F.getName().str());
  }
  for (auto* OpFunc : LambdaTargets) {
    std::string Name = OpFunc->getName().str();
    if (llvm::find(FuncNames, Name) == FuncNames.end())
      FuncNames.push_back(Name);
  }

  // Write FuncNames to !crs.func_names named metadata for IRDumpingPass to read.
  auto *FuncNamesMD = M.getOrInsertNamedMetadata("crs.func_names");
  for (const auto &Name : FuncNames) {
    MDString *S = MDString::get(Ctx, Name);
    FuncNamesMD->addOperand(MDNode::get(Ctx, {S}));
  }

  // Set !crs.rewriting_done flag to prevent double-rewriting.
  M.getOrInsertNamedMetadata("crs.rewriting_done");

  // ── PHASE N: Rewrite specializeLambda call sites in M (spec 010) ────────
  Constant* Zero32 = ConstantInt::get(Type::getInt32Ty(Ctx), 0);

  for (auto& [CI, OpFunc, HasOpts, LambdaArgIdx] : SpecLambdaSites) {
    Function* Callee = CI->getCalledFunction();

    Function* ResolvedFn = findResolvedFuncInBody(Callee);
    if (!ResolvedFn) {
      report_fatal_error(
          "IRRewritingPass: could not find specializeLambdaResolved in the IR module. "
          "Ensure ClangRuntimeSpecializer.h defines the Resolved overloads and the "
          "TU is compiled with the IRRewriting plugin.");
    }

    StringRef ResolvedName = OpFunc->getName();
    ArrayType* NameArrTy =
        ArrayType::get(Type::getInt8Ty(Ctx), ResolvedName.size() + 1);
    auto* NameGV = new GlobalVariable(
        M, NameArrTy, /*isConstant=*/true,
        GlobalValue::PrivateLinkage,
        ConstantDataArray::getString(Ctx, ResolvedName, /*AddNull=*/true),
        "__crs_resolved_name_" + std::to_string(SiteIdx));
    NameGV->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

    Constant* NameGEPIdxs[] = {Zero32, Zero32};
    Constant* NamePtr = ConstantExpr::getInBoundsGetElementPtr(
        NameArrTy, NameGV, NameGEPIdxs);

    SmallVector<Value*, 4> NewArgs;
    for (unsigned i = 0; i < LambdaArgIdx; i++)
      NewArgs.push_back(CI->getArgOperand(i));
    NewArgs.push_back(NamePtr);
    for (unsigned i = LambdaArgIdx; i < CI->arg_size(); i++)
      NewArgs.push_back(CI->getArgOperand(i));

    CallBase* NewCB;
    if (auto* II = dyn_cast<InvokeInst>(CI)) {
      auto* NewII = InvokeInst::Create(
          ResolvedFn->getFunctionType(), ResolvedFn,
          II->getNormalDest(), II->getUnwindDest(),
          NewArgs, "", CI->getIterator());
      NewII->copyMetadata(*CI);
      NewCB = NewII;
    } else {
      auto* NewCI2 = CallInst::Create(
          ResolvedFn->getFunctionType(), ResolvedFn, NewArgs, "",
          CI->getIterator());
      NewCI2->copyMetadata(*CI);
      NewCB = NewCI2;
    }
    CI->replaceAllUsesWith(NewCB);
    CI->eraseFromParent();

    ++SiteIdx;
  }

  // ── PHASE N2: Rewrite specializeOnly/callSpecialized(F* func, ...) sites ──
  for (auto& [CI, ResolvedName, FuncPtrArgIdx] : SpecFuncPtrSites) {
    Function* Callee = CI->getCalledFunction();

    Function* ResolvedFn = findSpecOnlyResolvedInBody(Callee);
    if (!ResolvedFn) {
      report_fatal_error(
          "IRRewritingPass: could not find specializeOnlyResolved/callSpecializedResolved "
          "in the IR module.  Ensure ClangRuntimeSpecializer.h defines the Resolved "
          "overloads and the TU is compiled with the IRRewriting plugin.");
    }

    ArrayType* NameArrTy =
        ArrayType::get(Type::getInt8Ty(Ctx), ResolvedName.size() + 1);
    auto* NameGV = new GlobalVariable(
        M, NameArrTy, /*isConstant=*/true,
        GlobalValue::PrivateLinkage,
        ConstantDataArray::getString(Ctx, ResolvedName, /*AddNull=*/true),
        "__crs_resolved_name_" + std::to_string(SiteIdx));
    NameGV->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

    Constant* NameGEPIdxs[] = {Zero32, Zero32};
    Constant* NamePtr = ConstantExpr::getInBoundsGetElementPtr(
        NameArrTy, NameGV, NameGEPIdxs);

    SmallVector<Value*, 6> NewArgs;
    for (unsigned i = 0; i < FuncPtrArgIdx; i++)
      NewArgs.push_back(CI->getArgOperand(i));
    NewArgs.push_back(NamePtr);
    for (unsigned i = FuncPtrArgIdx; i < CI->arg_size(); i++)
      NewArgs.push_back(CI->getArgOperand(i));

    CallBase* NewCB;
    if (auto* II = dyn_cast<InvokeInst>(CI)) {
      auto* NewII = InvokeInst::Create(
          ResolvedFn->getFunctionType(), ResolvedFn,
          II->getNormalDest(), II->getUnwindDest(),
          NewArgs, "", CI->getIterator());
      NewII->copyMetadata(*CI);
      NewCB = NewII;
    } else {
      auto* NewCI2 = CallInst::Create(
          ResolvedFn->getFunctionType(), ResolvedFn, NewArgs, "",
          CI->getIterator());
      NewCI2->copyMetadata(*CI);
      NewCB = NewCI2;
    }
    CI->replaceAllUsesWith(NewCB);
    CI->eraseFromParent();

    ++SiteIdx;
  }

  return PreservedAnalyses::none();
}
