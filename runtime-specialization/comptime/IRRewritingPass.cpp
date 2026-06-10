#include "RuntimeSpecializerPass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include <algorithm>
#include <cassert>
#include <cstdint>

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

// Returns true for the already-resolved function-pointer APIs. Some wrapper
// patterns instantiate these directly with a nullptr resolvedName at -O0.
bool isSpecOnlyFuncPtrResolvedCall(StringRef MangledName) {
  return MangledName.contains("specializeOnlyResolved") ||
         MangledName.contains("callSpecializedResolved") ||
         MangledName.contains("specializeOrFallbackResolved") ||
         MangledName.contains("assertSpecializedIsEquivalentResolved") ||
         MangledName.contains("compareFunctionInstructionCountsResolved");
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

Function *resolveFunctionValue(Value *V, SmallPtrSetImpl<Value *> &Visited);

// Resolve an Argument value to a unique constant function by inspecting direct
// callers of the argument's parent function. Returns nullptr on ambiguity.
Function *resolveArgFromCallers(Argument *Arg,
                                SmallPtrSetImpl<Value *> &Visited) {
  Function *Parent = Arg->getParent();
  if (!Parent || Parent->isDeclaration())
    return nullptr;

  Function *UniqueTarget = nullptr;
  bool SawDirectCaller = false;
  for (User *U : Parent->users()) {
    auto *CallerCB = dyn_cast<CallBase>(U);
    if (!CallerCB || CallerCB->getCalledFunction() != Parent)
      continue;
    SawDirectCaller = true;

    if (Arg->getArgNo() >= CallerCB->arg_size())
      return nullptr;

    Value *Actual = CallerCB->getArgOperand(Arg->getArgNo())->stripPointerCasts();
    SmallPtrSet<Value *, 16> ActualVisited;
    for (Value *Seen : Visited)
      ActualVisited.insert(Seen);
    auto *ActualFn = resolveFunctionValue(Actual, ActualVisited);
    if (!ActualFn)
      return nullptr;

    if (!UniqueTarget)
      UniqueTarget = ActualFn;
    else if (UniqueTarget != ActualFn)
      return nullptr;
  }

  if (SawDirectCaller)
    return UniqueTarget;
  return nullptr;
}

// Follow a function-pointer value through trivial SSA/memory plumbing commonly
// produced at -O0 (loads/stores to allocas, argument forwarding).
Function *resolveFunctionValue(Value *V, SmallPtrSetImpl<Value *> &Visited) {
  V = V->stripPointerCasts();
  if (!Visited.insert(V).second)
    return nullptr;

  if (auto *F = dyn_cast<Function>(V))
    return F;

  if (auto *Arg = dyn_cast<Argument>(V))
    return resolveArgFromCallers(Arg, Visited);

  if (auto *LI = dyn_cast<LoadInst>(V))
    return resolveFunctionValue(LI->getPointerOperand(), Visited);

  // Handles captures/tuple fields where the function pointer is loaded via GEP.
  if (auto *GEP = dyn_cast<GetElementPtrInst>(V))
    return resolveFunctionValue(GEP->getPointerOperand(), Visited);

  if (auto *AI = dyn_cast<AllocaInst>(V)) {
    Value *Stored = nullptr;
    for (User *U : AI->users()) {
      if (auto *SI = dyn_cast<StoreInst>(U)) {
        if (SI->getPointerOperand() != AI)
          continue;
        Value *Candidate = SI->getValueOperand();
        if (!Stored)
          Stored = Candidate;
        else if (Stored != Candidate)
          return nullptr;
        continue;
      }
      if (isa<LoadInst>(U))
        continue;
      if (auto *II = dyn_cast<IntrinsicInst>(U)) {
        if (II->getIntrinsicID() == Intrinsic::lifetime_start ||
            II->getIntrinsicID() == Intrinsic::lifetime_end ||
            II->getIntrinsicID() == Intrinsic::dbg_declare ||
            II->getIntrinsicID() == Intrinsic::dbg_value ||
            II->getIntrinsicID() == Intrinsic::dbg_assign)
          continue;
      }
      return nullptr;
    }
    if (Stored)
      return resolveFunctionValue(Stored, Visited);
  }

  return nullptr;
}

// Resolve the function pointer argument used by a specialization call site.
//
// Fast path: direct compile-time constant function operand at this call site.
// Fallback: if the operand is a function argument of the current helper
// function, infer its unique constant target from direct callers of that helper.
// This covers wrapper patterns like benchmark helpers that forward `F` into
// specializeOnly/callSpecialized.
Function* findFunctionPtrArg(CallBase* CI) {
  for (unsigned i = 0; i < CI->arg_size(); ++i) {
    SmallPtrSet<Value *, 8> Visited;
    if (Function *Resolved = resolveFunctionValue(CI->getArgOperand(i), Visited))
      return Resolved;
  }

  return nullptr;
}

// Data for a specializeOnly/callSpecialized(F* func, args...) call site.
struct SpecFuncPtrSite {
  enum class RewriteMode : uint8_t {
    InsertBeforeFuncPtr,
    ReplaceResolvedNameArg,
  };

  CallBase* CI;
  std::string ResolvedName;  // mangled name of the target function
  unsigned FuncPtrArgIdx;    // index of the function pointer in CI's arg list
  unsigned ResolvedNameArgIdx; // only used in ReplaceResolvedNameArg mode
  RewriteMode Mode;
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

    for (auto& BB : F) {
      for (auto& I : BB) {
        auto* CI = dyn_cast<CallBase>(&I);
        if (!CI) continue;
        auto* Callee = CI->getCalledFunction();
        if (!Callee) continue;
        bool IsUserCall = isSpecOnlyFuncPtrUserCall(Callee->getName());
        bool IsResolvedCall = isSpecOnlyFuncPtrResolvedCall(Callee->getName());
        if (!IsUserCall && !IsResolvedCall) continue;

        Function* TargetFn = findFunctionPtrArg(CI);
        if (!TargetFn) continue;

        unsigned FuncPtrArgIdx = 0;
        bool FoundFuncPtrArg = false;
        for (unsigned i = 0; i < CI->arg_size(); ++i) {
          SmallPtrSet<Value *, 8> Visited;
          if (resolveFunctionValue(CI->getArgOperand(i), Visited) == TargetFn) {
            FuncPtrArgIdx = i;
            FoundFuncPtrArg = true;
            break;
          }
        }
        if (!FoundFuncPtrArg)
          continue;

        if (IsResolvedCall) {
          unsigned ResolvedNameArgIdx = UINT32_MAX;
          if (FuncPtrArgIdx > 0 &&
              isa<ConstantPointerNull>(CI->getArgOperand(FuncPtrArgIdx - 1))) {
            // In all *Resolved funcptr APIs, resolvedName is immediately before
            // the function pointer parameter.
            ResolvedNameArgIdx = FuncPtrArgIdx - 1;
          } else {
            for (unsigned i = 0; i < FuncPtrArgIdx; ++i) {
              if (isa<ConstantPointerNull>(CI->getArgOperand(i))) {
                ResolvedNameArgIdx = i;
                break;
              }
            }
          }
          if (ResolvedNameArgIdx == UINT32_MAX)
            continue;

          SpecFuncPtrSites.push_back({
              CI, TargetFn->getName().str(), FuncPtrArgIdx, ResolvedNameArgIdx,
              SpecFuncPtrSite::RewriteMode::ReplaceResolvedNameArg});
          continue;
        }

        SpecFuncPtrSites.push_back({
            CI, TargetFn->getName().str(), FuncPtrArgIdx, UINT32_MAX,
            SpecFuncPtrSite::RewriteMode::InsertBeforeFuncPtr});
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

  // FR-010: Collect only actual specialization targets from rewritten call sites.
  // Keeping this list precise avoids cross-TU name collisions for unrelated
  // inline/library functions and keeps funcName->blob dispatch stable.
  SmallVector<std::string, 64> FuncNames;
  for (auto* OpFunc : LambdaTargets) {
    std::string Name = OpFunc->getName().str();
    if (llvm::find(FuncNames, Name) == FuncNames.end())
      FuncNames.push_back(Name);
  }
  for (const auto &Site : SpecFuncPtrSites) {
    if (llvm::find(FuncNames, Site.ResolvedName) == FuncNames.end())
      FuncNames.push_back(Site.ResolvedName);
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
  for (auto& [CI, ResolvedName, FuncPtrArgIdx, ResolvedNameArgIdx, Mode] : SpecFuncPtrSites) {
    Function* Callee = CI->getCalledFunction();

    Function* ResolvedFn = nullptr;
    if (Mode == SpecFuncPtrSite::RewriteMode::InsertBeforeFuncPtr) {
      ResolvedFn = findSpecOnlyResolvedInBody(Callee);
      if (!ResolvedFn) {
        continue;
      }
    } else {
      ResolvedFn = Callee;
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
    if (Mode == SpecFuncPtrSite::RewriteMode::InsertBeforeFuncPtr) {
      for (unsigned i = 0; i < FuncPtrArgIdx; i++)
        NewArgs.push_back(CI->getArgOperand(i));
      NewArgs.push_back(NamePtr);
      for (unsigned i = FuncPtrArgIdx; i < CI->arg_size(); i++)
        NewArgs.push_back(CI->getArgOperand(i));
    } else {
      for (unsigned i = 0; i < CI->arg_size(); ++i)
        NewArgs.push_back(CI->getArgOperand(i));
      if (ResolvedNameArgIdx >= NewArgs.size())
        report_fatal_error("IRRewritingPass: invalid resolvedName arg index");
      NewArgs[ResolvedNameArgIdx] = NamePtr;
    }

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
