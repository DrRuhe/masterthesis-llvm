#include "RuntimeSpecializerPass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Passes/PassPlugin.h"
#include <cassert>

using namespace llvm;

static std::pair<GlobalVariable*, GlobalVariable*> getOrCreateIRDumpGlobals(Module &M) {
  static constexpr const char *kPtrName = "RuntimeSpecializeableIR_ptr";
  static constexpr const char *kLenName = "RuntimeSpecializeableIR_len";

  LLVMContext &Ctx = M.getContext();

  // ptr global: constant ptr null
  GlobalVariable *PtrGV = M.getNamedGlobal(kPtrName);
  if (!PtrGV) {
    Type *PtrTy = PointerType::getUnqual(Ctx); // opaque 'ptr'
    PtrGV = new GlobalVariable(
        M,
        PtrTy,
        /*isConstant=*/true,
        GlobalValue::ExternalLinkage,
        /*Initializer=*/ConstantPointerNull::get(cast<PointerType>(PtrTy)),
        kPtrName);
    PtrGV->setUnnamedAddr(GlobalValue::UnnamedAddr::None);
  }

  // len global: constant i64 0
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

PreservedAnalyses IRDumpingPass::run(Module &M, ModuleAnalysisManager &AM) {
  bool Changed = false;

  static constexpr const char *kDataName = "RuntimeSpecializeableIR_data";

  // If we already finalized once, don't do it again (keeps things predictable).
  if (M.getNamedGlobal(kDataName)) {
    return PreservedAnalyses::all();
  }

  auto [PtrGV, LenGV] = getOrCreateIRDumpGlobals(M);

  // 1) Serialize the entire module to LLVM bitcode in-memory.
  SmallVector<char, 0> BitcodeBuffer;
  raw_svector_ostream OS(BitcodeBuffer);
  WriteBitcodeToFile(M, OS);

  LLVMContext &Ctx = M.getContext();
  ArrayRef<uint8_t> Bytes(reinterpret_cast<const uint8_t *>(BitcodeBuffer.data()),
                          BitcodeBuffer.size());

  // 2) Create @RuntimeSpecializeableIR_data = constant [N x i8] ...
  ArrayType *DataTy = ArrayType::get(Type::getInt8Ty(Ctx), Bytes.size());
  Constant *DataInit = ConstantDataArray::get(Ctx, Bytes);

  auto *DataGV = new GlobalVariable(
      M,
      DataTy,
      /*isConstant=*/true,
      GlobalValue::InternalLinkage,
      DataInit,
      kDataName);
  DataGV->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

  // 3) Point ptr to the first byte, and set len.
  Constant *Zero32 = ConstantInt::get(Type::getInt32Ty(Ctx), 0);
  SmallVector<Constant *, 2> GEPIdx = {Zero32, Zero32};
  Constant *DataPtr = ConstantExpr::getInBoundsGetElementPtr(DataTy, DataGV, GEPIdx);
  DataPtr = ConstantExpr::getBitCast(DataPtr, PointerType::getUnqual(Ctx));

  PtrGV->setInitializer(DataPtr);
  LenGV->setInitializer(ConstantInt::get(Type::getInt64Ty(Ctx), Bytes.size()));

  Changed = true;
  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}