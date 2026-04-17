#pragma once

#include <cstdarg>
#include <cstring>
#include <memory>
#include <string>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Cloning.h"

#define CALL_SPECIALIZED_FUNC_NAME_ANNOTATION_NAME "call_specialized_func_name"

// TODO refactor the error handling: create a special "ClangRuntimeSpecializationError" for this project.
namespace clangRuntimeSpecializer {

  class ClangRuntimeSpecializerError : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
  };

  class ClangRuntimeSpecializerArgSerializationError : public ClangRuntimeSpecializerError {
  public:
    using ClangRuntimeSpecializerError::ClangRuntimeSpecializerError;
  };

  class ClangRuntimeSpecializerDumpedIRError : public ClangRuntimeSpecializerError {
  public:
    using ClangRuntimeSpecializerError::ClangRuntimeSpecializerError;
  };

  class ClangRuntimeSpecializerChangesBehaviorError : public ClangRuntimeSpecializerError {
  public:
    using ClangRuntimeSpecializerError::ClangRuntimeSpecializerError;
  };

  template <typename T, typename = void>
  struct HasEqualityOperator : std::false_type {};

  template <typename T>
  struct HasEqualityOperator<T, std::void_t<decltype(std::declval<const T&>() == std::declval<const T&>())>> : std::true_type {};

  template <typename... Args>
  constexpr bool allComparable() {
    return (HasEqualityOperator<std::decay_t<Args>>::value && ...);
  }

  class ClangRuntimeSpecializer {
  public:
    struct WriteBack {
        llvm::Value* Source; // Changed from GlobalVariable* GV
        void* OriginalPtr;
        uint64_t Size;
    };

    enum class LogLevel {
      None,
      Error,
      Info,
      Debug
    };

    static void setLogLevel(LogLevel Level);
    static LogLevel getLogLevel();

    struct InstructionCounts {
      uint64_t Total;
      uint64_t Loads;
      uint64_t Stores;
      uint64_t Calls;
      uint64_t Arith;
      uint64_t Cmp;
      uint64_t Branches;
      uint64_t Returns;
      uint64_t Other;
    };

    struct JITModuleStats {
      size_t FunctionCount    = 0;  // non-declaration functions in merged module
      size_t InstructionCount = 0;  // total instructions in merged module
      size_t BitcodeSizeBytes = 0;  // raw size of all registered IR blobs combined
      size_t FunctionCountAfterPrune    = 0;  // after early GlobalDCE in IRTransformLayer
      size_t InstructionCountAfterPrune = 0;
    };

    // Per-pass record produced by PassInstrumentationCallbacks in the transform layer.
    struct PassRecord {
      std::string Name;       // LLVM pass class name, e.g. "GlobalDCEPass"
      std::string Group;      // "prune"|"initial"|"fixpoint"|"postfix"|"final"
      int FixpointIter;       // -1 if not inside fixpoint loop
      uint64_t FnsBefore, FnsAfter;
      uint64_t InstrsBefore, InstrsAfter;
      uint64_t BBsBefore, BBsAfter;
      double WallTimeMs;
      bool IRChanged;         // true when !PA.areAllPreserved()
    };

    static JITModuleStats getModuleStats();
    static JITModuleStats getLastTransformStats();
    static std::vector<PassRecord> getLastPassTrace();

    static void resetCounters();
    static InstructionCounts getCurrentCounters();
    static void printCounters();
    static void printComparisonTable(const char* funcName, const InstructionCounts& Before, const InstructionCounts& After);

    static void log(LogLevel Level, const char* FuncName, const llvm::Twine Message);
    static void log(LogLevel Level, const char* FuncName, const char* Message);

    template <typename Callback, typename = std::enable_if_t<std::is_invocable_v<Callback>>>
    static void log(LogLevel Level, const char* FuncName, Callback&& CB) {
      if (static_cast<int>(getLogLevel()) >= static_cast<int>(Level)) {
          log(Level, FuncName, CB());
      }
    }

    template <typename T>
    static std::string printLLVM(T Val) {
      std::string S;
      llvm::raw_string_ostream OS(S);
      if (Val) {
        Val->print(OS);
      } else {
        OS << "nullptr";
      }
      return S;
    }
#define CRS_LOG(Level, Msg) clangRuntimeSpecializer::ClangRuntimeSpecializer::log(clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Level, __FUNCTION__, Msg)


    struct Options {
      // --- Pipeline configuration ---
      int MaxFixpointIterations = 10;        // 0 = skip fixpoint loop entirely
      size_t LargeModuleInstrThreshold = 10000; // instrs after prune; > threshold → conservative unroll
      int LoopUnrollCount = 128;             // full-unroll max count (small modules only)
      bool EnableEarlyPrune = true;          // run GlobalDCE before fixpoint
      bool EnableO3Final = true;             // run O3 as final pass

      // --- Debug / analysis ---
      bool EnableInstructionInstrumentation = false;
      bool KeepDebugInfo = false;
      bool PrintFixpointIterations = false;
      std::string TimeTraceOutputPath;       // Chrome trace JSON path; "" = disabled

      // --- Preset factories ---
      static Options Default() { return {}; }
      static Options O3Only() {
        Options O; O.MaxFixpointIterations = 0; O.EnableEarlyPrune = false; return O;
      }
      static Options Aggressive() {
        Options O; O.MaxFixpointIterations = 20; O.LoopUnrollCount = 256;
        O.LargeModuleInstrThreshold = 50000; return O;
      }
      static Options Fast() {
        Options O; O.MaxFixpointIterations = 3;
        O.LargeModuleInstrThreshold = 0; return O;
      }

      // --- Fluent builder ---
      Options& withMaxFixpointIterations(int N)       { MaxFixpointIterations = N; return *this; }
      Options& withLargeModuleThreshold(size_t N)      { LargeModuleInstrThreshold = N; return *this; }
      Options& withLoopUnrollCount(int N)             { LoopUnrollCount = N; return *this; }
      Options& withEarlyPrune(bool V)                 { EnableEarlyPrune = V; return *this; }
      Options& withO3Final(bool V)                    { EnableO3Final = V; return *this; }
      Options& withTimeTraceOutput(std::string P)     { TimeTraceOutputPath = std::move(P); return *this; }
      Options& withKeepDebugInfo(bool V)              { KeepDebugInfo = V; return *this; }
      Options& withPrintFixpointIterations(bool V)    { PrintFixpointIterations = V; return *this; }
      Options& withInstructionInstrumentation(bool V) { EnableInstructionInstrumentation = V; return *this; }
    };

    static ClangRuntimeSpecializer* init();

    ClangRuntimeSpecializer* setOptions(Options Opts) {
      CurrentOptions = std::move(Opts);
      return this;
    }


    template <const char* funcName, class R, class... ARGS>
    [[clang::annotate(CALL_SPECIALIZED_FUNC_NAME_ANNOTATION_NAME, funcName)]]
    __attribute__((noinline))
    R callSpecialized(ARGS&&... Args) {
      return callImpl<funcName, R, false, true>(__FUNCTION__, std::forward<ARGS>(Args)...);
    }

    template <const char* funcName, class R, class... ARGS>
    [[clang::annotate(CALL_SPECIALIZED_FUNC_NAME_ANNOTATION_NAME, funcName)]]
    __attribute__((noinline))
    R call_instrumented(ARGS&&... Args) {
      return callImpl<funcName, R, true, true>(__FUNCTION__, std::forward<ARGS>(Args)...);
    }

    template <const char* funcName, class R, class... ARGS>
    [[clang::annotate(CALL_SPECIALIZED_FUNC_NAME_ANNOTATION_NAME, funcName)]]
    __attribute__((noinline))
    R call_instrumented_baseline(ARGS&&... Args) {
      return callImpl<funcName, R, true, false>(__FUNCTION__, std::forward<ARGS>(Args)...);
    }

    template <const char* funcName, class R, class... ARGS>
    [[clang::annotate(CALL_SPECIALIZED_FUNC_NAME_ANNOTATION_NAME, funcName)]]
    __attribute__((noinline))
    uintptr_t specializeOnly(ARGS&&... Args) {
      return specializeOnlyImpl<funcName, R, false, true>(__FUNCTION__, CurrentOptions, std::forward<ARGS>(Args)...);
    }

    template <const char* funcName, class R, class... ARGS>
    [[clang::annotate(CALL_SPECIALIZED_FUNC_NAME_ANNOTATION_NAME, funcName)]]
    __attribute__((noinline))
    uintptr_t specializeOnlyWithOptions(const Options& Opts, ARGS&&... Args) {
      return specializeOnlyImpl<funcName, R, false, true>(__FUNCTION__, Opts, std::forward<ARGS>(Args)...);
    }

    ~ClangRuntimeSpecializer();

  private:

    template <const char* funcName, class R, bool Instrument, bool Optimize = true, class... ARGS>
    uintptr_t specializeOnlyImpl(const char* CallerName, const Options& Opts, ARGS&&... Args) {
      checkInitialization(funcName);
      llvm::Function *TargetFunc = getTargetFunction(funcName);

      validateArgs(TargetFunc, sizeof...(ARGS));

      if constexpr (Instrument)
      {
          log(LogLevel::Info, CallerName, (llvm::Twine("Instrumenting call to: ") + funcName).str());
      }
      else
      {
          log(LogLevel::Info, CallerName, (llvm::Twine("Specializing call to: ") + funcName).str());
      }
      // Clone only the blob module that contains the target function.
      // This avoids cloning the full merged module when multiple TUs are linked.
      std::string UniqueWrapperName = createUniqueWrapperName() + (Optimize ? "" : "_no_opt");
      auto NewModule = llvm::CloneModule(*TargetFunc->getParent());
      auto* TargetFuncInNewModule = NewModule->getFunction(TargetFunc->getName());
      encourageInlining(TargetFuncInNewModule);

      llvm::LLVMContext& Ctx = NewModule->getContext();

      llvm::FunctionType* const FTy = llvm::FunctionType::get(TargetFunc->getReturnType(), false);

      llvm::Function* const NewFunc = llvm::Function::Create(FTy, llvm::Function::ExternalLinkage, UniqueWrapperName, *NewModule);
      if (Instrument) {
          NewFunc->addFnAttr("force-instrument");
      }
      if (!Optimize) {
          NewFunc->addFnAttr("force-no-optimize");
      }

      llvm::BasicBlock* const Entry = llvm::BasicBlock::Create(Ctx, "entry", NewFunc);
      llvm::IRBuilder<> Builder(Entry);

      // Serialize the runtime arguments to IR constants and create a call to the target function with them.
      std::vector<llvm::Value*> ArgValues = serializeArgumentsToIR(Builder, CallerName, std::forward<ARGS>(Args)...);

      auto * const CallInst = Builder.CreateCall(TargetFuncInNewModule->getFunctionType(), TargetFuncInNewModule, ArgValues);
      CallInst->setAttributes(TargetFuncInNewModule->getAttributes());
      CallInst->addFnAttr(llvm::Attribute::AlwaysInline);

      if (TargetFunc->getReturnType()->isVoidTy()) {
          Builder.CreateRetVoid();
      } else {
          Builder.CreateRet(CallInst);
      }

      prepareModuleForJIT(*NewModule, UniqueWrapperName);

      auto TSM = llvm::orc::ThreadSafeModule(std::move(NewModule), TSCtx);

      CurrentCallOptions = Opts;
      return addModuleAndLookup(std::move(TSM), UniqueWrapperName, std::string(funcName));
    }

    template <const char* funcName, class R, bool Instrument, bool Optimize = true, class... ARGS>
    R callImpl(const char* CallerName, ARGS&&... Args) {
      uintptr_t Addr = specializeOnlyImpl<funcName, R, Instrument, Optimize>(
          CallerName, CurrentOptions, std::forward<ARGS>(Args)...);
      auto SpecializedFnPtr = reinterpret_cast<R(*)()>(Addr);
      if constexpr (std::is_void_v<R>) {
          SpecializedFnPtr();
          return;
      } else {
          return SpecializedFnPtr();
      }
    }

    llvm::orc::ThreadSafeContext TSCtx;
    // Per-blob modules: one entry per registered blob, all in TSCtx's LLVMContext.
    // Cloning from BlobModules[i] instead of a full merged module reduces the
    // per-call CloneModule cost when multiple TUs are linked together.
    std::vector<std::unique_ptr<llvm::Module>> BlobModules;
    std::unordered_map<std::string, size_t> FuncToBlobIdx;
    std::unique_ptr<llvm::orc::LLJIT> JIT;
    uint64_t GlobalSpecializationCount = 0;
    Options CurrentOptions;      // default options used by specializeOnly() / callSpecialized()
    Options CurrentCallOptions;  // per-invocation options set by specializeOnlyImpl() before JIT
    std::vector<std::unique_ptr<char[]>> SerializationBuffers;

    void checkInitialization(const char* funcName) const;
    llvm::Function* getTargetFunction(const char* funcName);
    void validateArgs(llvm::Function* TargetFunc, size_t NumArgs) const;
    std::string createUniqueWrapperName() const;
    void prepareModuleForJIT(llvm::Module& M, const std::string& WrapperName) const;
    uintptr_t addModuleAndLookup(llvm::orc::ThreadSafeModule TSM, const std::string& WrapperName,
                                  const std::string& OrigFuncName = {});
    uint64_t dumpJITAssembly(const std::string& OrigFuncName, uintptr_t Addr);
    static void encourageInlining(llvm::Function* F);
    llvm::Function* buildWrapperIR(llvm::Module& M, const std::string& WrapperName, llvm::Function* TargetFunc,
                                   llvm::ArrayRef<llvm::Value*> SpecializedArgs, bool ForceInstrument, bool Optimize = true) const;

    explicit ClangRuntimeSpecializer();

      template <class T>
      llvm::Value* serializeArgumentToIR(llvm::IRBuilder<>& builder, T&& value) {
          using Decayed = std::decay_t<T>;
          // TODO implement proper serialization logic for all sorts of types.
          if constexpr (std::is_integral_v<Decayed> && !std::is_same_v<Decayed, bool>) {
              log(LogLevel::Debug, "serializeValueToIR", (llvm::Twine("Serializing value of type i") + llvm::Twine(sizeof(Decayed) * 8)).str());
              llvm::Type* Ty = llvm::Type::getIntNTy(builder.getContext(),
                                                    static_cast<unsigned>(sizeof(Decayed) * 8));
              return llvm::ConstantInt::get(Ty, static_cast<std::uint64_t>(value));
          } else if constexpr (std::is_floating_point_v<Decayed>) {
              log(LogLevel::Debug, "serializeValueToIR", "Serializing value of floating point type");
              if constexpr (std::is_same_v<Decayed, float>) {
                  return llvm::ConstantFP::get(builder.getContext(), llvm::APFloat(value));
              } else {
                  return llvm::ConstantFP::get(builder.getContext(), llvm::APFloat(static_cast<double>(value)));
              }
          } else if constexpr (std::is_pointer_v<Decayed> || std::is_class_v<Decayed>) {
              log(LogLevel::Debug, "serializeValueToIR", "Serializing value of type pointer or class");
              llvm::Type* Ty = llvm::Type::getInt64Ty(builder.getContext());
              std::uintptr_t addr;
              if constexpr (std::is_pointer_v<Decayed>) {
                  addr = reinterpret_cast<std::uintptr_t>(value);
              } else {
                  addr = reinterpret_cast<std::uintptr_t>(&value);
              }
              llvm::Constant* IntVal = llvm::ConstantInt::get(Ty, addr);
              return llvm::ConstantExpr::getIntToPtr(IntVal, llvm::PointerType::getUnqual(builder.getContext()));
          } else {
              throw ClangRuntimeSpecializerArgSerializationError("Cannot serialize argument to IR. ");
          }
      }

      template <class... Args>
      std::vector<llvm::Value*> serializeArgumentsToIR(llvm::IRBuilder<>& builder, const char* CallerName, Args&&... args) {
          std::vector<llvm::Value*> argValues;
          auto serializeAndLog = [&](auto&& arg) {
              auto* v = serializeArgumentToIR(builder, std::forward<decltype(arg)>(arg));
              log(LogLevel::Debug, CallerName, (llvm::Twine("Arg Serialized to: ") + printLLVM(v)).str());
              argValues.push_back(v);
          };
          (serializeAndLog(std::forward<Args>(args)), ...);
          return argValues;
      }


  };

  template <const char* funcName, class MemFn, class OBJ, class... ARGS>
  __attribute__((always_inline))
  decltype(auto) specializeMethodOrFallback(MemFn MF, OBJ&& Obj, ARGS&&... Args) {
    auto Invoke = [&]() -> decltype(auto) {
      return (std::forward<OBJ>(Obj).*MF)(std::forward<ARGS>(Args)...);
    };
    try {
      if (auto* RS = ClangRuntimeSpecializer::init()) {
        using R = decltype(Invoke());
        if constexpr (std::is_void_v<R>) {
          RS->callSpecialized<funcName,void>(std::forward<OBJ>(Obj), std::forward<ARGS>(Args)...);
          return;
        } else {
          return RS->callSpecialized<funcName,R>( std::forward<OBJ>(Obj), std::forward<ARGS>(Args)...);
        }
      }
    } catch (const std::exception& E) {
      CRS_LOG(Error, (llvm::Twine("Specialization failed: ") + E.what()).str());
    } catch (...) {
      CRS_LOG(Error, "Specialization failed with an unknown error.");
    }

    if constexpr (std::is_void_v<decltype(Invoke())>) {
      Invoke();
      return;
    } else {
      return Invoke();
    }
  }

  template <const char* funcName, class Fn, class... ARGS>
  __attribute__((always_inline))
  decltype(auto) specializeFunctionOrFallback(Fn F, ARGS&&... Args) {
    auto Invoke = [&]() -> decltype(auto) {
      return F(std::forward<ARGS>(Args)...);
    };

    try {
      if (auto* RS = ClangRuntimeSpecializer::init()) {
        using R = decltype(Invoke());
        if constexpr (std::is_void_v<R>) {
          RS->callSpecialized<funcName, void>(std::forward<ARGS>(Args)...);
          return;
        } else {
          return RS->callSpecialized<funcName, R>(std::forward<ARGS>(Args)...);
        }

      }
    } catch (const std::exception& E) {
      CRS_LOG(Error, (llvm::Twine("Specialization failed: ") + E.what()).str());
    } catch (...) {
      CRS_LOG(Error, "Specialization failed with an unknown error.");
    }

    if constexpr (std::is_void_v<decltype(Invoke())>) {
      Invoke();
      return;
    } else {
      return Invoke();
    }
  }

  template <const char* funcName, class Fn, class Tuple, class Comparator>
  __attribute__((always_inline))
  void assertSpecializedFunctionIsEquivalent(Fn F, Tuple&& normalArgs, Tuple&& specArgs, Comparator&& comp) {
    auto* RS = ClangRuntimeSpecializer::init();

    auto InvokeNormal = [&](auto&&... args) {
        return std::invoke(F, std::forward<decltype(args)>(args)...);
    };

    using R = decltype(std::apply(InvokeNormal, normalArgs));

    if constexpr (std::is_void_v<R>) {
        // Call original
        std::apply(InvokeNormal, normalArgs);

        // Call specialized
        std::apply([&](auto&&... CallArgs) {
            RS->callSpecialized<funcName, void>(std::forward<decltype(CallArgs)>(CallArgs)...);
        }, specArgs);
    } else {
        // Call original
        R ResOrig = std::apply(InvokeNormal, normalArgs);

        // Call specialized
        R ResSpec = std::apply([&](auto&&... CallArgs) -> R {
            return RS->callSpecialized<funcName, R>(std::forward<decltype(CallArgs)>(CallArgs)...);
        }, specArgs);

        // Compare return values
        if constexpr (HasEqualityOperator<R>::value) {
            if (ResOrig != ResSpec) {
                throw ClangRuntimeSpecializerChangesBehaviorError((llvm::Twine("Comparison failed: return values differ for ") + funcName).str());
            }
        }
    }

    // Compare modified arguments/state
    comp();

    CRS_LOG(Info, (llvm::Twine("Successfully specialized ") + funcName + "! No differences could be observed.").str());
  }


  // TODO this does NOT work with deep objects.
  //

  template <const char* funcName, class MemFn, class Tuple, class Comparator>
  __attribute__((always_inline))
  void assertSpecializedMethodIsEquivalent(MemFn Mf, Tuple&& normalArgs, Tuple&& specArgs, Comparator&& comp) {
    auto* RS = ClangRuntimeSpecializer::init();

    auto InvokeNormal = [&](auto&&... args) {
        return std::invoke(Mf, std::forward<decltype(args)>(args)...);
    };

    using R = decltype(std::apply(InvokeNormal, normalArgs));

    if constexpr (std::is_void_v<R>) {
        // Call original
        std::apply(InvokeNormal, normalArgs);

        // Call specialized
        std::apply([&](auto&&... CallArgs) {
            RS->callSpecialized<funcName, void>(
              std::forward<decltype(CallArgs)>(CallArgs)...);
        }, specArgs);
    } else {
        // Call original
        R ResOrig = std::apply(InvokeNormal, normalArgs);

        // Call specialized
        R ResSpec = std::apply([&](auto&&... CallArgs) -> R {
            return RS->callSpecialized<funcName, R>(
              std::forward<decltype(CallArgs)>(CallArgs)...);
        }, specArgs);

        if constexpr (HasEqualityOperator<R>::value) {
            if (ResOrig != ResSpec) {
                throw ClangRuntimeSpecializerChangesBehaviorError((llvm::Twine("Comparison failed: return values differ for ") + funcName).str());
            }
        }
    }

    comp();

    CRS_LOG(Info, (llvm::Twine("Successfully specialized ") + funcName + "! No differences could be observed.").str());
  }

  template <const char* funcName, class Fn, class... ARGS>
  __attribute__((always_inline))
  void compareFunctionInstructionCounts(Fn F, ARGS... Args) {
    auto* RS = ClangRuntimeSpecializer::init();
    RS->setOptions(ClangRuntimeSpecializer::Options::Default().withInstructionInstrumentation(true));

    // Copy arguments for both calls to ensure same initial state
    auto ArgsOrig = std::make_tuple(Args...);
    auto ArgsSpec = std::make_tuple(Args...);

    using R = decltype(F(Args...));

    ClangRuntimeSpecializer::InstructionCounts Before, After;

    // Call baseline instrumented
    RS->resetCounters();
    if constexpr (std::is_void_v<R>) {
        std::apply([&](auto&&... CallArgs) {
            RS->call_instrumented_baseline<funcName, void>(std::forward<std::decay_t<decltype(CallArgs)>>(CallArgs)...);
        }, ArgsOrig);
    } else {
        std::apply([&](auto&&... CallArgs) {
            RS->call_instrumented_baseline<funcName, R>(std::forward<std::decay_t<decltype(CallArgs)>>(CallArgs)...);
        }, ArgsOrig);
    }
    Before = RS->getCurrentCounters();

    // Call specialized instrumented
    RS->resetCounters();
    if constexpr (std::is_void_v<R>) {
        std::apply([&](auto&&... CallArgs) {
            RS->call_instrumented<funcName, void>(std::forward<decltype(CallArgs)>(CallArgs)...);
        }, ArgsSpec);
    } else {
        std::apply([&](auto&&... CallArgs) {
            RS->call_instrumented<funcName, R>(std::forward<decltype(CallArgs)>(CallArgs)...);
        }, ArgsSpec);
    }
    After = RS->getCurrentCounters();

    ClangRuntimeSpecializer::printComparisonTable(funcName, Before, After);
  }

} // namespace clangRuntimeSpecializer
