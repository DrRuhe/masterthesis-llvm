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
#include "llvm/Passes/PassPlugin.h"
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

  // 0) Collect defined function names before serialization so the runtime can
  //    build a funcName -> blob index map without parsing every blob at startup.
  SmallVector<std::string, 64> FuncNames;
  for (auto &F : M) {
    if (!F.isDeclaration() && !F.getName().starts_with("__clangRS"))
      FuncNames.push_back(F.getName().str());
  }

  const auto [PtrGV, LenGV] = getOrCreateIRDumpGlobals(M);

  // 1) Serialize the entire module to LLVM bitcode in-memory.
  SmallVector<char, 0> BitcodeBuffer;
  raw_svector_ostream OS(BitcodeBuffer);
  WriteBitcodeToFile(M, OS);

  LLVMContext &Ctx = M.getContext();
  const ArrayRef<uint8_t> Bytes(reinterpret_cast<const uint8_t *>(BitcodeBuffer.data()),
                          BitcodeBuffer.size());

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