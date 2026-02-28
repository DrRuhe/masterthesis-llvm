#include "ClangRuntimeSpecializer.h"

#include <cstdio>
#include <dlfcn.h>
#include <utility>

#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/IRTransformLayer.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/IPO/ModuleInliner.h"
#include "llvm/Analysis/InlineCost.h"

extern "C" void clang_runtime_specializer_link_anchor() {}

namespace {

  struct RuntimeSpecializableData {
    const void* Ptr;
    std::uint64_t Len;
  };

  RuntimeSpecializableData read_runtime_specializable_data() {
    void* ptrSym = dlsym(RTLD_DEFAULT, "RuntimeSpecializeableIR_ptr");
    void* lenSym = dlsym(RTLD_DEFAULT, "RuntimeSpecializeableIR_len");

    if (!ptrSym || !lenSym) {
      return {nullptr, 0};
    }

    auto* PtrVar = reinterpret_cast<void* const*>(ptrSym);
    auto* LenVar = reinterpret_cast<const std::uint64_t*>(lenSym);

    return { *PtrVar, static_cast<std::uint64_t>(*LenVar) };
  }

  std::unique_ptr<llvm::Module> parse_module_from_runtime_data(const RuntimeSpecializableData& data,
                                                               llvm::LLVMContext& ctx) {
    llvm::StringRef Bytes(reinterpret_cast<const char*>(data.Ptr), data.Len);
    llvm::MemoryBufferRef Buffer(Bytes, "RuntimeSpecializeableIR");

    llvm::Expected<std::unique_ptr<llvm::Module>> M =
        llvm::parseBitcodeFile(Buffer, ctx);

    if (!M) {
      std::string Err = llvm::toString(M.takeError());
      throw clangRuntimeSpecializer::ClangRuntimeSpecializerDumpedIRError("Failed to parse bitcode: " + Err);
    }

    return std::move(*M);
  }

} // namespace

namespace clangRuntimeSpecializer {

  static ClangRuntimeSpecializer::LogLevel CurrentLogLevel = ClangRuntimeSpecializer::LogLevel::Info;

  void ClangRuntimeSpecializer::setLogLevel(LogLevel Level) {
      CurrentLogLevel = Level;
  }

  ClangRuntimeSpecializer::LogLevel ClangRuntimeSpecializer::getLogLevel() {
      return CurrentLogLevel;
  }

  __attribute__((always_inline))
  void ClangRuntimeSpecializer::log(LogLevel Level, const char* FuncName, const llvm::Twine Message)
  {
    if (static_cast<int>(getLogLevel()) >= static_cast<int>(Level)) {
        log(Level, FuncName, Message.str().c_str());
    }
  }

  __attribute__((always_inline))
  void ClangRuntimeSpecializer::log(LogLevel Level, const char* FuncName, const char* Message) {
      if (static_cast<int>(getLogLevel()) >= static_cast<int>(Level)) {
        const char* LevelStr = "UNKNOWN";
        switch (Level) {
          case LogLevel::None: LevelStr = "NONE"; break;
          case LogLevel::Error: LevelStr = "ERROR"; break;
          case LogLevel::Info: LevelStr = "INFO"; break;
          case LogLevel::Debug: LevelStr = "DEBUG"; break;
        }
        std::fprintf(stdout, "%s: [%s] %s\n", LevelStr, FuncName, Message);
      }
  }



  static std::unique_ptr<ClangRuntimeSpecializer> Instance;

  ClangRuntimeSpecializer* ClangRuntimeSpecializer::init() {
    if (Instance) {
      return Instance.get();
    }
    
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    RuntimeSpecializableData data = read_runtime_specializable_data();
    if (!data.Ptr || data.Len == 0) {
      std::fprintf(stderr, "No dumped IR found in the executable.\n");
      std::abort();
    }

    Instance.reset(new ClangRuntimeSpecializer);

    auto JITExp = llvm::orc::LLJITBuilder().create();
    if (!JITExp) {
      std::string ErrMsg = llvm::toString(JITExp.takeError());
      throw ClangRuntimeSpecializerError("Failed to create JIT: " + ErrMsg);
    }
    Instance->JIT = std::move(*JITExp);

    Instance->JIT->getMainJITDylib().addGenerator(
        llvm::cantFail(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
            Instance->JIT->getDataLayout().getGlobalPrefix())));

    // Install an IR transform to run optimizations and log the optimized IR of the
    // specialized wrapper function before compilation.
    Instance->JIT->getIRTransformLayer().setTransform(
        [](llvm::orc::ThreadSafeModule TSM, llvm::orc::MaterializationResponsibility &R)
            -> llvm::Expected<llvm::orc::ThreadSafeModule> {
          TSM.withModuleDo([&](llvm::Module &M) {
            llvm::PassBuilder PB;
            llvm::LoopAnalysisManager LAM;
            llvm::FunctionAnalysisManager FAM;
            llvm::CGSCCAnalysisManager CGAM;
            llvm::ModuleAnalysisManager MAM;

            PB.registerModuleAnalyses(MAM);
            PB.registerCGSCCAnalyses(CGAM);
            PB.registerFunctionAnalyses(FAM);
            PB.registerLoopAnalyses(LAM);
            PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

            // Enable very aggressive inlining prior to the regular O3 pipeline.
            // We first run AlwaysInliner to respect any alwaysinline hints, then
            // a ModuleInlinerPass configured with extremely high thresholds and
            // relaxed deferral/recursion settings to inline as much as possible.
            {
              // Ensure that internal functions can be inlined by making them linkonce_odr
              // or similar if they were just internal. Actually, for JIT it should be fine,
              // but let's make sure the target functions are not marked as "noinline".
              for (auto &F : M) {
                if (!F.isDeclaration()) {
                  F.removeFnAttr(llvm::Attribute::NoInline);
                  F.removeFnAttr(llvm::Attribute::OptimizeNone);
                }
              }

              llvm::ModulePassManager AggressiveMPM;

              // Respect alwaysinline attributes.
              AggressiveMPM.addPass(llvm::AlwaysInlinerPass(/*InsertLifetimeIntrinsics=*/true));

              // Configure aggressive inline parameters.
              llvm::InlineParams IP = llvm::getInlineParams();
              IP.DefaultThreshold = 100000;
              IP.HintThreshold = 100000;
              IP.ColdThreshold = 100000;
              IP.OptSizeThreshold = 100000;
              IP.OptMinSizeThreshold = 100000;
              IP.HotCallSiteThreshold = 100000;
              IP.LocallyHotCallSiteThreshold = 100000;
              IP.ColdCallSiteThreshold = 100000;
              IP.ComputeFullInlineCost = true;
              IP.EnableDeferral = false;
              IP.AllowRecursiveCall = true;

              AggressiveMPM.addPass(llvm::ModuleInlinerPass(IP));
              AggressiveMPM.run(M, MAM);
            }

            // After aggressive inlining, run the regular O3 pipeline to clean up
            // and perform further optimizations on the now inlined code.
            {
              llvm::ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);
              MPM.run(M, MAM);
            }

            for (auto &F : M) {
              if (!F.isDeclaration() && F.getName().starts_with("specialized_wrapper_")) {
                log(LogLevel::Debug, "IRTransform", [&] {
                    return llvm::formatv("Optimized specialized function IR:\n{0}", printLLVM(&F)).str();
                });
              }
            }
          });
          return std::move(TSM);
        });

    Instance->Module = parse_module_from_runtime_data(data, Instance->Context);

    return Instance.get();
  }

  ClangRuntimeSpecializer::ClangRuntimeSpecializer() {}

  llvm::CallBase* ClangRuntimeSpecializer::findCallSpecializedFunctionInModule(const char* FunctionName, const char* UID) const {
    auto readCStringFromGlobal = [&](llvm::GlobalVariable *GV) -> std::string {
      if (!GV) return {};
      if (auto *CDA = llvm::dyn_cast<llvm::ConstantDataArray>(GV->getInitializer())) {
        if (CDA->isCString()) return CDA->getAsCString().str();
      }
      return {};
    };

    auto getAnnotationValueForFunction = [&](const llvm::Function &F, llvm::StringRef Key) -> std::optional<std::string> {
      llvm::GlobalVariable *AnnGV = Module->getGlobalVariable("llvm.global.annotations");
      if (!AnnGV || !AnnGV->hasInitializer()) return std::nullopt;
      auto *CA = llvm::dyn_cast<llvm::ConstantArray>(AnnGV->getInitializer());
      if (!CA) return std::nullopt;
      for (unsigned i = 0; i < CA->getNumOperands(); ++i) {
        auto *Elt = llvm::dyn_cast<llvm::ConstantStruct>(CA->getOperand(i));
        if (!Elt || Elt->getNumOperands() < 4) continue;
        // 0: ptr to annotated global (function), 1: ptr to anno string, 4: extra args (optional)
        llvm::Value *Op0 = Elt->getOperand(0);
        llvm::Value *Op1 = Elt->getOperand(1);
        llvm::Value *Op4 = (Elt->getNumOperands() >= 5) ? Elt->getOperand(4) : nullptr;

        if (auto *Op0C = llvm::dyn_cast<llvm::Constant>(Op0)) {
          if (auto *Target = Op0C->stripPointerCasts()) {
            if (Target == &F) {
              // Extract key
              std::string KeyStr;
              if (auto *Op1C = llvm::dyn_cast<llvm::Constant>(Op1)) {
                if (auto *KeyGV = llvm::dyn_cast<llvm::GlobalVariable>(Op1C->stripPointerCasts())) {
                  KeyStr = readCStringFromGlobal(KeyGV);
                }
              }

              if (KeyStr == Key) {
                // Try to extract first argument string from args struct if present
                if (Op4) {
                  if (auto *Op4C = llvm::dyn_cast<llvm::Constant>(Op4)) {
                    if (auto *ArgsGV = llvm::dyn_cast<llvm::GlobalVariable>(Op4C->stripPointerCasts())) {
                      if (auto *ArgsInit = llvm::dyn_cast<llvm::ConstantStruct>(ArgsGV->getInitializer())) {
                        if (ArgsInit->getNumOperands() >= 1) {
                          if (auto *Arg0C = llvm::dyn_cast<llvm::Constant>(ArgsInit->getOperand(0))) {
                            if (auto *StrGV = llvm::dyn_cast<llvm::GlobalVariable>(Arg0C->stripPointerCasts())) {
                              std::string V = readCStringFromGlobal(StrGV);
                              if (!V.empty()) return V;
                            }
                          }
                        }
                      }
                    }
                  }
                }
                // No value available
                return std::string();
              }
            }
          }
        }
      }
      return std::nullopt;
    };

    for (auto &F : *Module) {
      std::string FName = F.getName().str();
      if (FName.find(FunctionName) != std::string::npos) {
        auto AnnoVal = getAnnotationValueForFunction(F, CALL_SPECIALIZED_FUNC_NAME_ANNOTATION_NAME);
        if (!AnnoVal) continue;

        if (*AnnoVal == UID) {
          log(LogLevel::Info, "findCallSpecializedFunctionInModule",
              llvm::formatv("Found Function {0} responsible for specializing {1}", FName, UID));
          for (auto &U : F.uses()) {
            if (auto *CB = llvm::dyn_cast<llvm::CallBase>(U.getUser())) {
              if (CB->getCalledFunction() == &F) {
                return CB;
              }
            }
          }
        }
      }
    }

    throw ClangRuntimeSpecializerDumpedIRError("Could not find callsite for UID: " + std::string(UID));
  }

  llvm::Value* ClangRuntimeSpecializer::serializeValueToIR(llvm::IRBuilder<>& Builder, llvm::Type* Type, const void* ValuePtr) {
    log(LogLevel::Debug, "serializeValueToIR", [&] {
        return llvm::formatv("Serializing value of type {0}", printLLVM(Type)).str();
    });

    llvm::Value* Result = nullptr;
    if (Type->isIntegerTy()) {
      unsigned BitWidth = Type->getIntegerBitWidth();
      if (BitWidth <= 64) {
        uint64_t Val = 0;
        std::memcpy(&Val, ValuePtr, (BitWidth + 7) / 8);
        Result = llvm::ConstantInt::get(Type, Val);
      } else {
        throw ClangRuntimeSpecializerArgSerializationError("Integers > 64 bits are not supported yet.");
      }
    } else if (Type->isFloatTy()) {
      float Val;
      std::memcpy(&Val, ValuePtr, sizeof(float));
      Result = llvm::ConstantFP::get(Builder.getContext(), llvm::APFloat(Val));
    } else if (Type->isDoubleTy()) {
      double Val;
      std::memcpy(&Val, ValuePtr, sizeof(double));
      Result = llvm::ConstantFP::get(Builder.getContext(), llvm::APFloat(Val));
    } else if (Type->isPointerTy()) {
      uintptr_t Val;
      std::memcpy(&Val, ValuePtr, sizeof(uintptr_t));
      llvm::Type* Ty = llvm::Type::getInt64Ty(Builder.getContext());
      llvm::Constant* IntVal = llvm::ConstantInt::get(Ty, static_cast<uint64_t>(Val));
      Result = llvm::ConstantExpr::getIntToPtr(IntVal, Type);
    } else if (Type->isStructTy()) {
      llvm::StructType* STy = llvm::cast<llvm::StructType>(Type);
      const llvm::DataLayout& DL = Module->getDataLayout();
      const llvm::StructLayout* SL = DL.getStructLayout(STy);

      std::vector<llvm::Constant*> Elements;
      for (unsigned i = 0; i < STy->getNumElements(); ++i) {
        llvm::Type* ElemTy = STy->getElementType(i);
        uint64_t Offset = SL->getElementOffset(i);
        const void* ElemPtr = static_cast<const char*>(ValuePtr) + Offset;

        llvm::Value* ElemVal = serializeValueToIR(Builder, ElemTy, ElemPtr);
        if (auto* C = llvm::dyn_cast_or_null<llvm::Constant>(ElemVal)) {
          Elements.push_back(C);
        } else {
          throw ClangRuntimeSpecializerArgSerializationError("Failed to serialize struct element " + std::to_string(i));
        }
      }
      Result = llvm::ConstantStruct::get(STy, Elements);
    } else if (Type->isArrayTy()) {
      llvm::ArrayType* ATy = llvm::cast<llvm::ArrayType>(Type);
      llvm::Type* ElemTy = ATy->getElementType();
      const llvm::DataLayout& DL = Module->getDataLayout();
      uint64_t ElemSize = DL.getTypeAllocSize(ElemTy);

      std::vector<llvm::Constant*> Elements;
      for (uint64_t i = 0; i < ATy->getNumElements(); ++i) {
        const void* ElemPtr = static_cast<const char*>(ValuePtr) + (i * ElemSize);
        llvm::Value* ElemVal = serializeValueToIR(Builder, ElemTy, ElemPtr);
        if (auto* C = llvm::dyn_cast_or_null<llvm::Constant>(ElemVal)) {
          Elements.push_back(C);
        } else {
          throw ClangRuntimeSpecializerArgSerializationError("Failed to serialize array element " + std::to_string(i));
        }
      }
      Result = llvm::ConstantArray::get(ATy, Elements);
    } else {
      throw ClangRuntimeSpecializerArgSerializationError("Unsupported type for serialization: " + printLLVM(Type));
    }

    if (Result) {
        log(LogLevel::Debug, "serializeValueToIR", [&] {
            return llvm::formatv("Serialized to LLVM value: {0}", printLLVM(Result)).str();
        });
    }

    return Result;
  }

  ClangRuntimeSpecializer::~ClangRuntimeSpecializer() = default;

} // namespace clangRuntimeSpecializer
