#include "RuntimeSpecializerPass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/Metadata.h"
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

  // Read FuncNames from !crs.func_names named metadata written by IRRewritingPass.
  // These names serve as DCE roots and are registered with the runtime for
  // funcName → blob-index lookup (FR-010).
  SmallVector<std::string, 64> FuncNames;
  if (auto *FuncNamesMD = M.getNamedMetadata("crs.func_names")) {
    for (const auto *Op : FuncNamesMD->operands()) {
      if (Op && Op->getNumOperands() > 0)
        if (auto *S = dyn_cast<MDString>(Op->getOperand(0)))
          FuncNames.push_back(S->getString().str());
    }
  }

  // Clone the module for compile-time preprocessing. All linkage transformations,
  // GlobalDCE, and structural changes happen on the clone only. The original
  // module M is left intact so normal compilation (with main, exported symbols,
  // etc.) is unaffected. Only the blob embedding (DataGV, CtorFn) is added to M.
  auto ClonedM = CloneModule(M);

  // === COMPILE-TIME PREPROCESSING on ClonedM (spec 004-ir-dump-preprocessing) ===

  // Promote FuncNames entries with internal/private linkage to ExternalLinkage
  // in ClonedM so they survive GlobalDCE as explicit DCE roots. This is required
  // because IRRewritingPass already rewrote call sites in M before cloning, so
  // lambda proxy functions may have lost their only callers (the specializeLambda
  // call was replaced by specializeLambdaResolved) and would otherwise be pruned.
  for (const auto &Name : FuncNames) {
    if (auto *F = ClonedM->getFunction(Name))
      if (!F->isDeclaration() && (F->hasInternalLinkage() || F->hasPrivateLinkage()))
        F->setLinkage(GlobalValue::ExternalLinkage);
  }

  // Identify DCE roots in the clone: all externally-visible functions (including
  // the lambda proxy functions just promoted above).
  SmallPtrSet<Function *, 32> DCERoots;
  for (auto &F : *ClonedM) {
    if (!F.isDeclaration() && !F.getName().starts_with("__clangRS") &&
        !F.hasInternalLinkage() && !F.hasPrivateLinkage())
      DCERoots.insert(&F);
  }

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

  // Collect InternalLinkage data globals from the preprocessed clone (spec 017).
  // These need host addresses at JIT time so specializeOnlyImpl can replace them
  // with inttoptr constants pointing to live host memory instead of zero copies.
  SmallVector<std::pair<std::string, GlobalVariable *>, 16> InternalGVs;
  for (auto &GV : ClonedM->globals()) {
    if (!GV.hasInternalLinkage() || GV.isDeclaration()) continue;
    StringRef Name = GV.getName();
    if (Name.starts_with("llvm.")) continue;
    if (Name.starts_with("_ZTV") || Name.starts_with("_ZTI") || Name.starts_with("_ZTS"))
      continue;
    if (Name.starts_with("RuntimeSpecializeableIR") || Name.starts_with("__clangRS"))
      continue;
    GlobalVariable *OrigGV = M.getNamedGlobal(Name);
    if (!OrigGV) continue;
    InternalGVs.emplace_back(Name.str(), OrigGV);
  }

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

  // 4b) Build names+addrs arrays for InternalLinkage globals (spec 017).
  //     The runtime uses these to replace JIT's zero copies with inttoptr constants.
  auto *PtrTy = PointerType::getUnqual(Ctx);
  Constant *GlobalNamesArrayPtr;
  Constant *GlobalAddrsArrayPtr;
  const uint64_t NumInternalGVs = InternalGVs.size();
  if (!InternalGVs.empty()) {
    SmallVector<Constant *, 16> GNamePtrs;
    SmallVector<Constant *, 16> GAddrPtrs;
    for (auto &[Name, OrigGV] : InternalGVs) {
      ArrayType *StrTy = ArrayType::get(Type::getInt8Ty(Ctx), Name.size() + 1);
      auto *StrGV = new GlobalVariable(M, StrTy, /*isConstant=*/true,
                                       GlobalValue::InternalLinkage,
                                       ConstantDataArray::getString(Ctx, Name, true), "");
      StrGV->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
      Constant *StrGEPIdxs[] = {Zero32, Zero32};
      GNamePtrs.push_back(ConstantExpr::getInBoundsGetElementPtr(StrTy, StrGV, StrGEPIdxs));
      GAddrPtrs.push_back(OrigGV); // OrigGV is the pointer to the host global's storage
    }
    ArrayType *NamesArrTy = ArrayType::get(PtrTy, NumInternalGVs);
    auto *NamesGV = new GlobalVariable(M, NamesArrTy, /*isConstant=*/true,
                                       GlobalValue::InternalLinkage,
                                       ConstantArray::get(NamesArrTy, GNamePtrs),
                                       "RuntimeSpecializeableIR_global_names");
    NamesGV->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
    Constant *NamesGEP[] = {Zero32, Zero32};
    GlobalNamesArrayPtr = ConstantExpr::getInBoundsGetElementPtr(NamesArrTy, NamesGV, NamesGEP);

    ArrayType *AddrsArrTy = ArrayType::get(PtrTy, NumInternalGVs);
    auto *AddrsGV = new GlobalVariable(M, AddrsArrTy, /*isConstant=*/true,
                                       GlobalValue::InternalLinkage,
                                       ConstantArray::get(AddrsArrTy, GAddrPtrs),
                                       "RuntimeSpecializeableIR_global_addrs");
    AddrsGV->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
    Constant *AddrsGEP[] = {Zero32, Zero32};
    GlobalAddrsArrayPtr =
        ConstantExpr::getInBoundsGetElementPtr(AddrsArrTy, AddrsGV, AddrsGEP);
  } else {
    GlobalNamesArrayPtr = ConstantPointerNull::get(PtrTy);
    GlobalAddrsArrayPtr = ConstantPointerNull::get(PtrTy);
  }

  // 5) Register this IR blob at program startup via a module constructor.
  //    v3 adds InternalLinkage global names+addrs so the runtime can resolve
  //    static globals to live host-memory pointers at specialization time (spec 017).
  FunctionType *RegTy = FunctionType::get(
      Type::getVoidTy(Ctx),
      {PtrTy, Type::getInt64Ty(Ctx),   // blob ptr, len
       PtrTy, Type::getInt64Ty(Ctx),   // funcs, nfuncs
       PtrTy, PtrTy, Type::getInt64Ty(Ctx)},  // global_names, global_addrs, nglobals
      /*isVarArg=*/false);
  FunctionCallee RegFn = M.getOrInsertFunction(
      "clang_runtime_specializer_register_blob_v3", RegTy);

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
      ConstantInt::get(Type::getInt64Ty(Ctx), FuncNames.size()),
      GlobalNamesArrayPtr,
      GlobalAddrsArrayPtr,
      ConstantInt::get(Type::getInt64Ty(Ctx), NumInternalGVs)
  });
  Builder.CreateRetVoid();

  appendToGlobalCtors(M, CtorFn, /*Priority=*/65535);

  return PreservedAnalyses::none();
}
