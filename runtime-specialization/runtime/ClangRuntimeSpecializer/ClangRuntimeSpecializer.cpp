#include "ClangRuntimeSpecializer.h"

#include <cstdio>
#include <dlfcn.h>
#include <utility>
#include <algorithm>
#include <vector>

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

extern "C" {
    uint64_t g_inst_count = 0;
    uint64_t g_load_count = 0;
    uint64_t g_store_count = 0;
    uint64_t g_call_count = 0;
    uint64_t g_arith_count = 0;
    uint64_t g_cmp_count = 0;
    uint64_t g_branch_count = 0;
    uint64_t g_ret_count = 0;
    uint64_t g_other_count = 0;
}

namespace clangRuntimeSpecializer {

  static ClangRuntimeSpecializer::LogLevel CurrentLogLevel = ClangRuntimeSpecializer::LogLevel::Debug;

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

  void ClangRuntimeSpecializer::resetCounters() {
    g_inst_count = 0;
    g_load_count = 0;
    g_store_count = 0;
    g_call_count = 0;
    g_arith_count = 0;
    g_cmp_count = 0;
    g_branch_count = 0;
    g_ret_count = 0;
    g_other_count = 0;
  }

  ClangRuntimeSpecializer::InstructionCounts ClangRuntimeSpecializer::getCurrentCounters() {
    return { g_inst_count, g_load_count, g_store_count, g_call_count, g_arith_count, g_cmp_count, g_branch_count, g_ret_count, g_other_count };
  }

  void ClangRuntimeSpecializer::printComparisonTable(const char* funcName, const InstructionCounts& Before, const InstructionCounts& After) {
      std::fprintf(stdout, "Comparing instruction counts from specializing %s:\n", funcName);
      std::fprintf(stdout, "%10s %10s %-15s %s\n", "before", "after", "instruction", "change");

      struct Row {
          const char* Name;
          uint64_t B;
          uint64_t A;
      };

      auto printRow = [](const Row& r) {
          int64_t Diff = static_cast<int64_t>(r.A) - static_cast<int64_t>(r.B);
          double Percent = (r.B == 0) ? (r.A == 0 ? 0.0 : 100.0) : (static_cast<double>(std::abs(Diff)) / r.B) * 100.0;
          if (Diff > 0) {
              std::fprintf(stdout, "%10lu %10lu %-15s +%ld, %.0f%%\n", r.B, r.A, r.Name, Diff, Percent);
          } else if (Diff < 0) {
              std::fprintf(stdout, "%10lu %10lu %-15s %ld, -%.0f%%\n", r.B, r.A, r.Name, Diff, Percent);
          } else {
              std::fprintf(stdout, "%10lu %10lu %-15s 0, 0%%\n", r.B, r.A, r.Name);
          }
      };

      printRow({"total", Before.Total, After.Total});
      std::fprintf(stdout, "\n");

      std::vector<Row> IndividualRows = {
          {"load", Before.Loads, After.Loads},
          {"store", Before.Stores, After.Stores},
          {"call", Before.Calls, After.Calls},
          {"arith", Before.Arith, After.Arith},
          {"cmp", Before.Cmp, After.Cmp},
          {"branch", Before.Branches, After.Branches},
          {"ret", Before.Returns, After.Returns},
          {"other", Before.Other, After.Other}
      };

      std::sort(IndividualRows.begin(), IndividualRows.end(), [](const Row& a, const Row& b) {
          if (a.B != b.B) return a.B > b.B;
          return std::string(a.Name) < std::string(b.Name);
      });

      for (const auto& r : IndividualRows) {
          printRow(r);
      }
  }

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

    // Explicitly export instrumentation counters to the JIT.
    if (isInstructionInstrumentationEnabled()) {
      auto &JD = Instance->JIT->getMainJITDylib();
      llvm::orc::SymbolMap Symbols;
      Symbols[Instance->JIT->mangleAndIntern("g_inst_count")] = { llvm::orc::ExecutorAddr::fromPtr(&g_inst_count), llvm::JITSymbolFlags::Exported };
      Symbols[Instance->JIT->mangleAndIntern("g_load_count")] = { llvm::orc::ExecutorAddr::fromPtr(&g_load_count), llvm::JITSymbolFlags::Exported };
      Symbols[Instance->JIT->mangleAndIntern("g_store_count")] = { llvm::orc::ExecutorAddr::fromPtr(&g_store_count), llvm::JITSymbolFlags::Exported };
      Symbols[Instance->JIT->mangleAndIntern("g_call_count")] = { llvm::orc::ExecutorAddr::fromPtr(&g_call_count), llvm::JITSymbolFlags::Exported };
      Symbols[Instance->JIT->mangleAndIntern("g_arith_count")] = { llvm::orc::ExecutorAddr::fromPtr(&g_arith_count), llvm::JITSymbolFlags::Exported };
      Symbols[Instance->JIT->mangleAndIntern("g_cmp_count")] = { llvm::orc::ExecutorAddr::fromPtr(&g_cmp_count), llvm::JITSymbolFlags::Exported };
      Symbols[Instance->JIT->mangleAndIntern("g_branch_count")] = { llvm::orc::ExecutorAddr::fromPtr(&g_branch_count), llvm::JITSymbolFlags::Exported };
      Symbols[Instance->JIT->mangleAndIntern("g_ret_count")] = { llvm::orc::ExecutorAddr::fromPtr(&g_ret_count), llvm::JITSymbolFlags::Exported };
      Symbols[Instance->JIT->mangleAndIntern("g_other_count")] = { llvm::orc::ExecutorAddr::fromPtr(&g_other_count), llvm::JITSymbolFlags::Exported };
      cantFail(JD.define(llvm::orc::absoluteSymbols(Symbols)));
    }
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

            // After optimization, perform dynamic instruction counting instrumentation if enabled.
            if (Instance->isInstructionInstrumentationEnabled()) {
              for (auto &F : M) {
                if (F.isDeclaration()) continue;
                // Instrument only the specialized wrapper or everything that was inlined into it.
                // For simplicity, let's instrument all functions in the module.
                for (auto &BB : F) {
                  uint64_t BBInstCount = 0;
                  uint64_t BBLoadCount = 0;
                  uint64_t BBStoreCount = 0;
                  uint64_t BBCallCount = 0;
                  uint64_t BBArithCount = 0;
                  uint64_t BBCmpCount = 0;
                  uint64_t BBBranchCount = 0;
                  uint64_t BBRetCount = 0;
                  uint64_t BBOtherCount = 0;
                  for (auto &I : BB) {
                    BBInstCount++;
                    if (llvm::isa<llvm::LoadInst>(&I)) BBLoadCount++;
                    else if (llvm::isa<llvm::StoreInst>(&I)) BBStoreCount++;
                    else if (llvm::isa<llvm::CallBase>(&I)) BBCallCount++;
                    else if (llvm::isa<llvm::BinaryOperator>(&I) || llvm::isa<llvm::UnaryOperator>(&I)) BBArithCount++;
                    else if (llvm::isa<llvm::CmpInst>(&I)) BBCmpCount++;
                    else if (llvm::isa<llvm::ReturnInst>(&I)) BBRetCount++;
                    else if (I.isTerminator()) BBBranchCount++;
                    else BBOtherCount++;
                  }

                  if (BBInstCount > 0) {
                    llvm::IRBuilder<> Builder(&*BB.getFirstInsertionPt());
                    auto CreateIncrement = [&](const char* Name, uint64_t Count) {
                      if (Count == 0) return;
                      auto *CounterGV = M.getGlobalVariable(Name, true);
                      if (!CounterGV) {
                        CounterGV = new llvm::GlobalVariable(M, llvm::Type::getInt64Ty(M.getContext()), false,
                                                             llvm::GlobalValue::ExternalLinkage, nullptr, Name);
                      }
                      Builder.CreateAtomicRMW(llvm::AtomicRMWInst::Add, CounterGV,
                                             Builder.getInt64(Count), llvm::MaybeAlign(),
                                             llvm::AtomicOrdering::Monotonic);
                    };

                    CreateIncrement("g_inst_count", BBInstCount);
                    CreateIncrement("g_load_count", BBLoadCount);
                    CreateIncrement("g_store_count", BBStoreCount);
                    CreateIncrement("g_call_count", BBCallCount);
                    CreateIncrement("g_arith_count", BBArithCount);
                    CreateIncrement("g_cmp_count", BBCmpCount);
                    CreateIncrement("g_branch_count", BBBranchCount);
                    CreateIncrement("g_ret_count", BBRetCount);
                    CreateIncrement("g_other_count", BBOtherCount);
                  }
                }
              }
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
