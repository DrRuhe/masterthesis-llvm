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
#include "llvm/ADT/SmallVector.h"
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

  // FR-010: Collect specialization target function names from the ORIGINAL module
  // before any transforms. Restricted to originally-externally-visible functions
  // so every collected name is a GlobalDCE root and survives compile-time DCE.
  SmallVector<std::string, 64> FuncNames;
  for (auto &F : M) {
    if (!F.isDeclaration() && !F.getName().starts_with("__clangRS") &&
        !F.hasInternalLinkage() && !F.hasPrivateLinkage())
      FuncNames.push_back(F.getName().str());
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

  // FR-005 (pre-DCE): walk constant global initializers transitively to find
  // vtable/RTTI method implementations; set them WeakODRLinkage so they survive
  // GlobalDCE and DevirtualizeConstantVtableCallsPass can find them at JIT time.
  {
    SmallVector<Constant *, 32> WorkList;
    SmallPtrSet<Constant *, 32> Visited;
    for (auto &G : ClonedM->globals())
      if (G.isConstant() && G.hasInitializer())
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
  for (auto *F : DCERoots)
    if (!F->isDeclaration() && !F->hasWeakODRLinkage())
      F->setLinkage(GlobalValue::InternalLinkage);

  // === END PREPROCESSING ===

  // 1) Serialize the preprocessed clone to LLVM bitcode in-memory.
  SmallVector<char, 0> BitcodeBuffer;
  raw_svector_ostream OS(BitcodeBuffer);
  WriteBitcodeToFile(*ClonedM, OS);

  LLVMContext &Ctx = M.getContext();
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

  return PreservedAnalyses::none();
}