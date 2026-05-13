#include "RuntimeSpecializerPass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/IPO/GlobalDCE.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
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

static std::pair<GlobalVariable*, GlobalVariable*> getOrCreateIRDumpGlobals(Module &M) {
  static constexpr const char *kPtrName = "RuntimeSpecializeableIR_ptr";
  static constexpr const char *kLenName = "RuntimeSpecializeableIR_len";

  LLVMContext &Ctx = M.getContext();

  // ptr global: constant ptr null (internal — no cross-TU symbol conflict)
  GlobalVariable *PtrGV = M.getNamedGlobal(kPtrName);
  if (!PtrGV) {
    Type * const PtrTy = PointerType::getUnqual(Ctx); // opaque 'ptr'
    PtrGV = new GlobalVariable(
        M,
        PtrTy,
        /*isConstant=*/true,
        GlobalValue::InternalLinkage,
        /*Initializer=*/ConstantPointerNull::get(cast<PointerType>(PtrTy)),
        kPtrName);
    PtrGV->setUnnamedAddr(GlobalValue::UnnamedAddr::None);
  }

  // len global: constant i64 0 (internal)
  GlobalVariable *LenGV = M.getNamedGlobal(kLenName);
  if (!LenGV) {
    Type * const LenTy = Type::getInt64Ty(Ctx);
    LenGV = new GlobalVariable(
        M,
        LenTy,
        /*isConstant=*/true,
        GlobalValue::InternalLinkage,
        /*Initializer=*/ConstantInt::get(LenTy, 0),
        kLenName);
    LenGV->setUnnamedAddr(GlobalValue::UnnamedAddr::None);
  }

  return {PtrGV, LenGV};
}

PreservedAnalyses IRDumpingPass::run(Module &M, ModuleAnalysisManager &AM) {
  static constexpr const char *kDataName = "RuntimeSpecializeableIR_data";

  // If we already finalized once, don't do it again (keeps things predictable).
  if (M.getNamedGlobal(kDataName)) {
    return PreservedAnalyses::all();
  }

  LLVMContext &Ctx = M.getContext();

  // ── PHASE 0: Detect specializeLambda call sites (spec 010, FR-001/FR-008) ──
  // For each call to the user-facing specializeLambda(lambda [, opts]) overload,
  // discover the lambda's operator() mangled name via alloca-type + demangling.
  // Fatal error (FR-018) if the operator() cannot be resolved.
  SmallVector<SpecLambdaSite, 8> SpecLambdaSites;
  SmallPtrSet<Function*, 8> LambdaTargets;
  unsigned SiteIdx = 0;

  for (auto& F : M) {
    if (F.isDeclaration()) continue;
    // Skip functions that are themselves part of the specializer infrastructure
    // (free wrappers, member overloads, resolved variants, helper functions).
    // Their internal calls pass the function/lambda as a parameter — not an
    // alloca-backed local — so alloca-tracing would fail or produce wrong results.
    if (F.getName().contains("specializeLambda") ||
        F.getName().contains("specializeOnly") ||
        F.getName().contains("callSpecialized") ||
        F.getName().contains("specializeOrFallback") ||
        F.getName().contains("assertSpecializedIsEquivalent"))
      continue;

    for (auto& BB : F) {
      for (auto& I : BB) {
        // Match both call and invoke instructions (inlined always_inline helpers
        // like assertSpecializedLambdaIsEquivalent use invoke for EH cleanups).
        auto* CI = dyn_cast<CallBase>(&I);
        if (!CI) continue;
        auto* Callee = CI->getCalledFunction();
        if (!Callee || !isSpecializeLambdaUserCall(Callee->getName())) continue;

        // Find where resolvedName should be inserted by inspecting the Resolved
        // call inside the callee body — no mangled-name heuristics needed.
        unsigned LambdaArgIdx = findLambdaArgIdx(Callee);

        if (LambdaArgIdx >= CI->arg_size())
          continue; // malformed call — skip

        // Options arg (if present) immediately follows the lambda arg.
        bool HasOpts = (LambdaArgIdx + 1 < CI->arg_size());

        // Find the proxy function via the __crs_op_hint sentinel — pure LLVM API.
        Function* OpFunc = findLambdaProxyFunc(Callee);
        if (!OpFunc) {
          // FR-018: compile-time fatal error when proxy cannot be resolved.
          report_fatal_error(
              "IRDumpingPass: could not find __crs_op_hint call in forceLambdaOpEmit "
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
  // For each call to the user-facing specializeOnly/callSpecialized overload that
  // takes a function pointer, extract the compile-time-constant function name.
  // Fatal error (FR-025) if the pointer is not a compile-time constant.
  SmallVector<SpecFuncPtrSite, 8> SpecFuncPtrSites;

  for (auto& F : M) {
    if (F.isDeclaration()) continue;
    // Skip specializer infrastructure itself (free wrappers, resolved variants,
    // helper functions like specializeOrFallback/assertSpecializedIsEquivalent).
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
        if (!Callee || !isSpecOnlyFuncPtrUserCall(Callee->getName())) continue;

        // The user-facing overload takes (F* func, args...) where func is
        // a compile-time constant.  Find the function pointer argument.
        // If no Function* arg is found (e.g. function ptr passed through a
        // template wrapper parameter), skip — runtime dispatch still works.
        Function* TargetFn = findFunctionPtrArg(CI);
        if (!TargetFn) continue;

        // Find the function pointer's argument index (for rewriting).
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

  // Clone the module for compile-time preprocessing. All linkage transformations,
  // GlobalDCE, and structural changes happen on the clone only. The original
  // module M is left intact so normal compilation (with main, exported symbols,
  // etc.) is unaffected. Only the blob embedding (DataGV, CtorFn) is added to M.
  auto ClonedM = CloneModule(M);

  // === COMPILE-TIME PREPROCESSING on ClonedM (spec 004-ir-dump-preprocessing) ===

  // Identify DCE roots in the clone by matching FuncNames.
  SmallPtrSet<Function *, 32> DCERoots;
  for (auto &F : *ClonedM) {
    if (!F.isDeclaration() && !F.getName().starts_with("__clangRS") &&
        !F.hasInternalLinkage() && !F.hasPrivateLinkage())
      DCERoots.insert(&F);
  }
  // Lambda operator() targets may be internal — explicitly add them to DCERoots
  // so GlobalDCE keeps them even if they are only used via specializeLambda.
  for (auto* OpFunc : LambdaTargets)
    if (auto* ClonedOp = ClonedM->getFunction(OpFunc->getName()))
      DCERoots.insert(ClonedOp);

  // FR-005 (pre-DCE): walk C++ vtable/RTTI constant global initializers
  // transitively to find virtual method implementations; set them WeakODRLinkage
  // so they survive GlobalDCE and DevirtualizeConstantVtableCallsPass can find
  // them at JIT time. Restricted to C++ mangled vtable/typeinfo globals (_ZTV*,
  // _ZTI*, _ZTS*) to avoid treating C dispatch tables as vtables — those should
  // not prevent GlobalDCE from pruning unreachable C functions.
  {
    SmallVector<Constant *, 32> WorkList;
    SmallPtrSet<Constant *, 32> Visited;
    for (auto &G : ClonedM->globals())
      if (G.isConstant() && G.hasInitializer() &&
          (G.getName().starts_with("_ZTV") || G.getName().starts_with("_ZTI") ||
           G.getName().starts_with("_ZTS")))
        if (Visited.insert(G.getInitializer()).second)
          WorkList.push_back(G.getInitializer());
    while (!WorkList.empty()) {
      auto *C = WorkList.pop_back_val();
      if (auto *F = dyn_cast<Function>(C)) {
        if (!F->isDeclaration())
          F->setLinkage(GlobalValue::WeakODRLinkage);
      } else {
        for (unsigned I = 0, E = C->getNumOperands(); I != E; ++I)
          if (auto *Op = dyn_cast<Constant>(C->getOperand(I)))
            if (Visited.insert(Op).second)
              WorkList.push_back(Op);
      }
    }
  }

  // FR-005: constant globals with initializers → WeakODR (DCE root; DevirtPass
  // reads vtable initializers at JIT time). Non-constant non-internal globals →
  // AvailableExternally so the JIT resolves them from the host rather than
  // compiling new definitions. For Comdat globals (e.g., guard variables for
  // function-local statics from inline functions), clear the Comdat before
  // setting AvailableExternally — the LLVM verifier rejects AvailableExternally
  // on globals that still carry a Comdat. After clearing Comdat, these globals
  // become unreferenced AvailableExternally declarations and GlobalDCE prunes them.
  for (auto &G : ClonedM->globals()) {
    if (!G.isDeclaration() && !G.hasInternalLinkage() && !G.hasPrivateLinkage()) {
      if (G.isConstant() && G.hasInitializer())
        G.setLinkage(GlobalValue::WeakODRLinkage);
      else {
        G.setComdat(nullptr);
        G.setLinkage(GlobalValue::AvailableExternallyLinkage);
      }
    }
  }

  // FR-003: erase global_ctors/dtors from the clone. The host already ran them;
  // erasing makes any ctor-only functions unreachable, so GlobalDCE removes them.
  if (auto *GCtors = ClonedM->getGlobalVariable("llvm.global_ctors"))
    GCtors->eraseFromParent();
  if (auto *GDtors = ClonedM->getGlobalVariable("llvm.global_dtors"))
    GDtors->eraseFromParent();

  // FR-004: remove zero-sized globals from the clone (type `{}`); JITLink crashes
  // when a zero-byte ELF section is produced from malloc(0)=null on Linux.
  {
    const auto &DL = ClonedM->getDataLayout();
    SmallVector<GlobalVariable *, 16> ZeroSized;
    for (auto &G : ClonedM->globals())
      if (!G.isDeclaration() && DL.getTypeAllocSize(G.getValueType()) == 0)
        ZeroSized.push_back(&G);
    for (auto *G : ZeroSized) {
      G->replaceAllUsesWith(PoisonValue::get(G->getType()));
      G->eraseFromParent();
    }
  }

  // FR-006: run GlobalDCE on the clone. DCE roots = ExternalLinkage DCERoots +
  // WeakODR fns/globals. Prunes dead code before serialization.
  // Uses a fresh analysis manager to avoid cross-contaminating M's cached analyses.
  {
    PassBuilder PB;
    LoopAnalysisManager LAM;
    FunctionAnalysisManager FAM;
    CGSCCAnalysisManager CGAM;
    ModuleAnalysisManager CloneAM;
    PB.registerModuleAnalyses(CloneAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, CloneAM);

    ModulePassManager MPM;
    MPM.addPass(GlobalDCEPass());
    MPM.run(*ClonedM, CloneAM);
  }

  // FR-005 (post-DCE): set originally-externally-visible target functions to
  // InternalLinkage. Done after DCE so they served as DCE roots during pruning.
  // Skip WeakODR functions — the vtable BFS already marked them WeakODR, which
  // is the correct blob linkage for DevirtualizeConstantVtableCallsPass at JIT time.
  // Iterate ClonedM (not DCERoots directly) because GlobalDCE erases discardable
  // functions (AvailableExternally, LinkOnce) that have no uses, leaving dangling
  // pointers in DCERoots. Using live functions from ClonedM is always safe.
  for (auto &F : *ClonedM)
    if (DCERoots.count(&F) && !F.isDeclaration() && !F.hasWeakODRLinkage())
      F.setLinkage(GlobalValue::InternalLinkage);

  // === END PREPROCESSING ===

  // 1) Serialize the preprocessed clone to LLVM bitcode in-memory.
  SmallVector<char, 0> BitcodeBuffer;
  raw_svector_ostream OS(BitcodeBuffer);
  WriteBitcodeToFile(*ClonedM, OS);

  const ArrayRef<uint8_t> Bytes(reinterpret_cast<const uint8_t *>(BitcodeBuffer.data()),
                          BitcodeBuffer.size());

  // All remaining operations target the ORIGINAL module M (not the clone).
  const auto [PtrGV, LenGV] = getOrCreateIRDumpGlobals(M);

  // 2) Create @RuntimeSpecializeableIR_data = constant [N x i8] ...
  ArrayType * const DataTy = ArrayType::get(Type::getInt8Ty(Ctx), Bytes.size());
  Constant * const DataInit = ConstantDataArray::get(Ctx, Bytes);

  auto * const DataGV = new GlobalVariable(
      M,
      DataTy,
      /*isConstant=*/true,
      GlobalValue::InternalLinkage,
      DataInit,
      kDataName);
  DataGV->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

  // 3) Point ptr to the first byte, and set len.
  Constant * const Zero32 = ConstantInt::get(Type::getInt32Ty(Ctx), 0);
  const SmallVector<Constant *, 2> GEPIdx = {Zero32, Zero32};
  Constant * DataPtr = ConstantExpr::getInBoundsGetElementPtr(DataTy, DataGV, GEPIdx);
  DataPtr = ConstantExpr::getBitCast(DataPtr, PointerType::getUnqual(Ctx));

  PtrGV->setInitializer(DataPtr);
  LenGV->setInitializer(ConstantInt::get(Type::getInt64Ty(Ctx), Bytes.size()));

  // 4) Build an array of C-string pointers for the defined function names.
  //    The runtime uses this to map funcName -> blob index at init() time,
  //    so only the relevant per-TU module needs to be cloned per specialization.
  Constant *FuncsArrayPtr;
  if (!FuncNames.empty()) {
    SmallVector<Constant *, 64> NamePtrs;
    for (const auto &Name : FuncNames) {
      ArrayType *StrTy = ArrayType::get(Type::getInt8Ty(Ctx), Name.size() + 1);
      auto *StrGV = new GlobalVariable(M, StrTy, /*isConstant=*/true,
                                       GlobalValue::InternalLinkage,
                                       ConstantDataArray::getString(Ctx, Name, /*AddNull=*/true),
                                       "");
      StrGV->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
      Constant *StrGEPIdxs[] = {Zero32, Zero32};
      NamePtrs.push_back(ConstantExpr::getInBoundsGetElementPtr(StrTy, StrGV, StrGEPIdxs));
    }
    ArrayType *FuncArrayTy = ArrayType::get(PointerType::getUnqual(Ctx), FuncNames.size());
    auto *FuncsGV = new GlobalVariable(M, FuncArrayTy, /*isConstant=*/true,
                                       GlobalValue::InternalLinkage,
                                       ConstantArray::get(FuncArrayTy, NamePtrs),
                                       "RuntimeSpecializeableIR_funcs");
    FuncsGV->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
    Constant *FuncGEPIdxs[] = {Zero32, Zero32};
    FuncsArrayPtr = ConstantExpr::getInBoundsGetElementPtr(FuncArrayTy, FuncsGV, FuncGEPIdxs);
  } else {
    FuncsArrayPtr = ConstantPointerNull::get(PointerType::getUnqual(Ctx));
  }

  // 5) Register this IR blob at program startup via a module constructor.
  //    v2 includes the function name array so the runtime can build a
  //    funcName -> blob index map without parsing all blobs.
  FunctionType *RegTy = FunctionType::get(
      Type::getVoidTy(Ctx),
      {PointerType::getUnqual(Ctx), Type::getInt64Ty(Ctx),
       PointerType::getUnqual(Ctx), Type::getInt64Ty(Ctx)},
      /*isVarArg=*/false);
  FunctionCallee RegFn = M.getOrInsertFunction(
      "clang_runtime_specializer_register_blob_v2", RegTy);

  // Give the constructor a name unique to this TU (based on module identifier).
  std::string TUName = M.getModuleIdentifier();
  std::replace_if(TUName.begin(), TUName.end(),
                  [](char c) { return !isalnum(static_cast<unsigned char>(c)); }, '_');
  Function *CtorFn = Function::Create(
      FunctionType::get(Type::getVoidTy(Ctx), /*isVarArg=*/false),
      GlobalValue::InternalLinkage,
      "__clangRS_register_blob_" + TUName,
      &M);
  BasicBlock *BB = BasicBlock::Create(Ctx, "entry", CtorFn);
  IRBuilder<> Builder(BB);
  Builder.CreateCall(RegFn, {
      DataPtr,
      ConstantInt::get(Type::getInt64Ty(Ctx), Bytes.size()),
      FuncsArrayPtr,
      ConstantInt::get(Type::getInt64Ty(Ctx), FuncNames.size())
  });
  Builder.CreateRetVoid();

  appendToGlobalCtors(M, CtorFn, /*Priority=*/65535);

  // ── PHASE N: Rewrite specializeLambda call sites in M (spec 010) ────────
  // Replace each user-facing specializeLambda(lambda [, opts]) call with
  // specializeLambdaResolved(resolvedName, lambda [, opts]) in the original
  // module.  The resolved name is a private constant string global pointing to
  // the lambda operator() mangled name discovered in Phase 0.
  for (auto& [CI, OpFunc, HasOpts, LambdaArgIdx] : SpecLambdaSites) {
    Function* Callee = CI->getCalledFunction();

    // Find the specializeLambdaResolved instantiation by inspecting the body
    // of the specializeLambda function — the body calls Resolved(nullptr,...).
    Function* ResolvedFn = findResolvedFuncInBody(Callee);
    if (!ResolvedFn) {
      report_fatal_error(
          "IRDumpingPass: could not find specializeLambdaResolved in the IR module. "
          "Ensure ClangRuntimeSpecializer.h defines the Resolved overloads and the "
          "TU is compiled with the IRDumping plugin.");
    }

    // Emit @__crs_resolved_name_K = private constant [N+1 x i8] c"mangled\00"
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

    // Build rewritten arg list, inserting resolvedName just before the lambda arg.
    // LambdaArgIdx = 0 (free function): original args = (lambda [, opts])
    //               → rewritten = (resolvedName, lambda [, opts])
    // LambdaArgIdx = 1 (member function): original args = (this, lambda [, opts])
    //               → rewritten = (this, resolvedName, lambda [, opts])
    SmallVector<Value*, 4> NewArgs;
    for (unsigned i = 0; i < LambdaArgIdx; i++)
      NewArgs.push_back(CI->getArgOperand(i));   // args before lambda (e.g. 'this')
    NewArgs.push_back(NamePtr);                   // resolvedName (new)
    for (unsigned i = LambdaArgIdx; i < CI->arg_size(); i++)
      NewArgs.push_back(CI->getArgOperand(i));   // lambda_ref [, opts_ref]

    // Create the replacement: preserve invoke vs call to maintain EH semantics.
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
  // Replace with specializeOnlyResolved(resolvedName, func, ...) /
  // callSpecializedResolved(resolvedName, func, ...).
  for (auto& [CI, ResolvedName, FuncPtrArgIdx] : SpecFuncPtrSites) {
    Function* Callee = CI->getCalledFunction();

    // Find the Resolved variant by inspecting the callee body.
    Function* ResolvedFn = findSpecOnlyResolvedInBody(Callee);
    if (!ResolvedFn) {
      report_fatal_error(
          "IRDumpingPass: could not find specializeOnlyResolved/callSpecializedResolved "
          "in the IR module.  Ensure ClangRuntimeSpecializer.h defines the Resolved "
          "overloads and the TU is compiled with the IRDumping plugin.");
    }

    // Emit @__crs_resolved_name_K = private constant [N+1 x i8] c"mangled\00"
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

    // Build rewritten arg list: insert resolvedName just before the func pointer arg.
    // Original:  (sret?, [this,] func_ptr, args...)
    // Rewritten: (sret?, [this,] resolvedName, func_ptr, args...)
    SmallVector<Value*, 6> NewArgs;
    for (unsigned i = 0; i < FuncPtrArgIdx; i++)
      NewArgs.push_back(CI->getArgOperand(i));  // sret / this (if any)
    NewArgs.push_back(NamePtr);                  // resolvedName (new)
    for (unsigned i = FuncPtrArgIdx; i < CI->arg_size(); i++)
      NewArgs.push_back(CI->getArgOperand(i));  // func_ptr, args...

    // Preserve invoke vs call for EH semantics.
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