#pragma once

#include <algorithm>
#include <cmath>
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

    static void log(LogLevel Level, const llvm::Twine Message);
    static void log(LogLevel Level, const char* Message);

    template <typename Callback, typename = std::enable_if_t<std::is_invocable_v<Callback>>>
    static void log(LogLevel Level, Callback&& CB) {
      if (static_cast<int>(getLogLevel()) >= static_cast<int>(Level)) {
          log(Level, CB());
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
#define CRS_LOG(Level, Msg) clangRuntimeSpecializer::ClangRuntimeSpecializer::log(clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Level, Msg)


    struct Options {
      // --- Pipeline configuration ---
      int MaxFixpointIterations = 10;        // 0 = skip fixpoint loop entirely
      size_t LargeModuleInstrThreshold = 10000; // instrs after prune; > threshold → conservative unroll
      int LoopUnrollCount = 128;             // full-unroll max count (small modules only)
      bool EnableEarlyPrune = true;          // run GlobalDCE before fixpoint
      bool EnableO3Final = true;             // run O3 as final pass
      unsigned FuncSpecMaxGroups = 0;        // 0 = unlimited; skip function if it has more distinct constant-arg groups

      // --- Budget metadata (set by FromExpectedRuntime; stored for logging/counter export) ---
      double ExpectedCallDurationNs = 0.0;  // 0 = not set
      double BudgetScale            = 1.0;

      // --- Debug / analysis ---
      bool Optimize = true;                          // when false → skips all JIT optimization passes
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
      static Options NoOptimize() {
        Options O; O.MaxFixpointIterations = 0; O.EnableEarlyPrune = false;
        O.EnableO3Final = false; O.Optimize = false; return O;
      }
      static Options FromExpectedRuntime(double callDurationNs, double budgetScale = 1.0) {
        const double scaledNs = std::max(callDurationNs, 1.0) * std::max(budgetScale, 0.01);
        // log2 range: kMinNs=1e3 (log2≈10), kMaxNs=1e9 (log2≈30) → t in [0,1]
        constexpr double kLog2Min = 10.0, kLog2Max = 30.0;
        const double t = std::clamp((std::log2(scaledNs) - kLog2Min) / (kLog2Max - kLog2Min),
                                     0.0, 1.0);
        if (scaledNs < 1e3) {
          Options O = Options::O3Only();
          O.ExpectedCallDurationNs = callDurationNs; O.BudgetScale = budgetScale; return O;
        }
        auto lerp = [](double a, double b, double tt) { return a + tt * (b - a); };
        Options O;
        O.MaxFixpointIterations     = static_cast<int>(std::round(lerp(0.0, 20.0,    t)));
        O.LoopUnrollCount           = static_cast<int>(std::round(lerp(1.0, 256.0,   t)));
        O.LargeModuleInstrThreshold = static_cast<size_t>(std::round(lerp(0.0, 50000.0, t)));
        O.EnableEarlyPrune = true; O.EnableO3Final = true;
        O.ExpectedCallDurationNs = callDurationNs; O.BudgetScale = budgetScale;
        return O;
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
      Options& withOptimize(bool V)                   { Optimize = V; return *this; }
      Options& withFuncSpecMaxGroups(unsigned N)      { FuncSpecMaxGroups = N; return *this; }
      Options& withExpectedCallDurationNs(double V)   { ExpectedCallDurationNs = V; return *this; }
      Options& withBudgetScale(double V)              { BudgetScale = V; return *this; }
    };

    static ClangRuntimeSpecializer* init();

    ClangRuntimeSpecializer* setOptions(Options Opts) {
      CurrentOptions = std::move(Opts);
      return this;
    }

    // Helper trait: true when the first element of ARGS is not Options.
    // Used to prevent overload ambiguity when Options is passed as the first arg.
    template <class...>
    struct FirstArgIsNotOptions : std::true_type {};
    template <class A, class... Rest>
    struct FirstArgIsNotOptions<A, Rest...>
        : std::bool_constant<!std::is_same_v<std::decay_t<A>, Options>> {};

    template <class R, class... ARGS,
              std::enable_if_t<FirstArgIsNotOptions<ARGS...>::value, int> = 0>
    __attribute__((noinline))
    R callSpecialized(const char* funcName, ARGS&&... Args) {
      return callImpl<R>(funcName, CurrentOptions, std::forward<ARGS>(Args)...);
    }

    template <class R, class... ARGS>
    __attribute__((noinline))
    R callSpecialized(const char* funcName, const Options& opts, ARGS&&... Args) {
      return callImpl<R>(funcName, opts, std::forward<ARGS>(Args)...);
    }

    template <class... ARGS,
              std::enable_if_t<FirstArgIsNotOptions<ARGS...>::value, int> = 0>
    __attribute__((noinline))
    uintptr_t specializeOnly(const char* funcName, ARGS&&... Args) {
      return specializeOnlyImpl(funcName, CurrentOptions, std::forward<ARGS>(Args)...);
    }

    template <class... ARGS>
    __attribute__((noinline))
    uintptr_t specializeOnly(const char* funcName, const Options& opts, ARGS&&... Args) {
      return specializeOnlyImpl(funcName, opts, std::forward<ARGS>(Args)...);
    }

    ~ClangRuntimeSpecializer();

  private:

    template <class... ARGS>
    uintptr_t specializeOnlyImpl(const char* funcName, const Options& Opts, ARGS&&... Args) {
      checkInitialization(funcName);
      llvm::Function *TargetFunc = getTargetFunction(funcName);

      validateArgs(TargetFunc, sizeof...(ARGS));

      if (Opts.EnableInstructionInstrumentation) {
          log(LogLevel::Info,  (llvm::Twine("Instrumenting call to: ") + funcName).str());
      } else {
          log(LogLevel::Info, (llvm::Twine("Specializing call to: ") + funcName).str());
      }
      // Clone only the blob module that contains the target function.
      // This avoids cloning the full merged module when multiple TUs are linked.
      std::string UniqueWrapperName = createUniqueWrapperName() + (Opts.Optimize ? "" : "_no_opt");
      auto NewModule = llvm::CloneModule(*TargetFunc->getParent());
      auto* TargetFuncInNewModule = NewModule->getFunction(TargetFunc->getName());
      encourageInlining(TargetFuncInNewModule);

      llvm::LLVMContext& Ctx = NewModule->getContext();

      llvm::FunctionType* const FTy = llvm::FunctionType::get(TargetFunc->getReturnType(), false);

      llvm::Function* const NewFunc = llvm::Function::Create(FTy, llvm::Function::ExternalLinkage, UniqueWrapperName, *NewModule);
      if (Opts.EnableInstructionInstrumentation) {
          NewFunc->addFnAttr("force-instrument");
      }
      if (!Opts.Optimize) {
          NewFunc->addFnAttr("force-no-optimize");
      }

      llvm::BasicBlock* const Entry = llvm::BasicBlock::Create(Ctx, "entry", NewFunc);
      llvm::IRBuilder<> Builder(Entry);

      // Serialize the runtime arguments to IR constants and create a call to the target function with them.
      std::vector<llvm::Value*> ArgValues = serializeArgumentsToIR(Builder, std::forward<ARGS>(Args)...);

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

    template <class R, class... ARGS>
    R callImpl(const char* funcName, const Options& Opts, ARGS&&... Args) {
      uintptr_t Addr = specializeOnlyImpl(funcName, Opts, std::forward<ARGS>(Args)...);
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

    explicit ClangRuntimeSpecializer();

      template <class T>
      llvm::Value* serializeArgumentToIR(llvm::IRBuilder<>& builder, T&& value) {
          using Decayed = std::decay_t<T>;
          // TODO implement proper serialization logic for all sorts of types.
          if constexpr (std::is_integral_v<Decayed> && !std::is_same_v<Decayed, bool>) {
              log(LogLevel::Debug, (llvm::Twine("Serializing value of type i") + llvm::Twine(sizeof(Decayed) * 8)).str());
              llvm::Type* Ty = llvm::Type::getIntNTy(builder.getContext(),
                                                    static_cast<unsigned>(sizeof(Decayed) * 8));
              return llvm::ConstantInt::get(Ty, static_cast<std::uint64_t>(value));
          } else if constexpr (std::is_floating_point_v<Decayed>) {
              log(LogLevel::Debug, "Serializing value of floating point type");
              if constexpr (std::is_same_v<Decayed, float>) {
                  return llvm::ConstantFP::get(builder.getContext(), llvm::APFloat(value));
              } else {
                  return llvm::ConstantFP::get(builder.getContext(), llvm::APFloat(static_cast<double>(value)));
              }
          } else if constexpr (std::is_pointer_v<Decayed> || std::is_class_v<Decayed>) {
              log(LogLevel::Debug, "Serializing value of type pointer or class");
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
      std::vector<llvm::Value*> serializeArgumentsToIR(llvm::IRBuilder<>& builder, Args&&... args) {
          std::vector<llvm::Value*> argValues;
          auto serializeAndLog = [&](auto&& arg) {
              auto* v = serializeArgumentToIR(builder, std::forward<decltype(arg)>(arg));
              log(LogLevel::Debug, (llvm::Twine("Arg Serialized to: ") + printLLVM(v)).str());
              argValues.push_back(v);
          };
          (serializeAndLog(std::forward<Args>(args)), ...);
          return argValues;
      }


  };

  template <class MemFn, class OBJ, class... ARGS>
  __attribute__((always_inline))
  decltype(auto) specializeMethodOrFallback(const char* funcName, MemFn MF, OBJ&& Obj, ARGS&&... Args) {
    auto Invoke = [&]() -> decltype(auto) {
      return (std::forward<OBJ>(Obj).*MF)(std::forward<ARGS>(Args)...);
    };
    try {
      if (auto* RS = ClangRuntimeSpecializer::init()) {
        using R = decltype(Invoke());
        if constexpr (std::is_void_v<R>) {
          RS->callSpecialized<void>(funcName, std::forward<OBJ>(Obj), std::forward<ARGS>(Args)...);
          return;
        } else {
          return RS->callSpecialized<R>(funcName, std::forward<OBJ>(Obj), std::forward<ARGS>(Args)...);
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

  template <class Fn, class... ARGS>
  __attribute__((always_inline))
  decltype(auto) specializeFunctionOrFallback(const char* funcName, Fn F, ARGS&&... Args) {
    auto Invoke = [&]() -> decltype(auto) {
      return F(std::forward<ARGS>(Args)...);
    };

    try {
      if (auto* RS = ClangRuntimeSpecializer::init()) {
        using R = decltype(Invoke());
        if constexpr (std::is_void_v<R>) {
          RS->callSpecialized<void>(funcName, std::forward<ARGS>(Args)...);
          return;
        } else {
          return RS->callSpecialized<R>(funcName, std::forward<ARGS>(Args)...);
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

  template <class Fn, class Tuple, class Comparator>
  __attribute__((always_inline))
  void assertSpecializedFunctionIsEquivalent(const char* funcName, Fn F, Tuple&& normalArgs, Tuple&& specArgs, Comparator&& comp) {
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
            RS->callSpecialized<void>(funcName, std::forward<decltype(CallArgs)>(CallArgs)...);
        }, specArgs);
    } else {
        // Call original
        R ResOrig = std::apply(InvokeNormal, normalArgs);

        // Call specialized
        R ResSpec = std::apply([&](auto&&... CallArgs) -> R {
            return RS->callSpecialized<R>(funcName, std::forward<decltype(CallArgs)>(CallArgs)...);
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

  template <class MemFn, class Tuple, class Comparator>
  __attribute__((always_inline))
  void assertSpecializedMethodIsEquivalent(const char* funcName, MemFn Mf, Tuple&& normalArgs, Tuple&& specArgs, Comparator&& comp) {
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
            RS->callSpecialized<void>(funcName,
              std::forward<decltype(CallArgs)>(CallArgs)...);
        }, specArgs);
    } else {
        // Call original
        R ResOrig = std::apply(InvokeNormal, normalArgs);

        // Call specialized
        R ResSpec = std::apply([&](auto&&... CallArgs) -> R {
            return RS->callSpecialized<R>(funcName,
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

  template <class Fn, class... ARGS>
  __attribute__((always_inline))
  void compareFunctionInstructionCounts(const char* funcName, Fn F, ARGS... Args) {
    auto* RS = ClangRuntimeSpecializer::init();

    // Copy arguments for both calls to ensure same initial state
    auto ArgsOrig = std::make_tuple(Args...);
    auto ArgsSpec = std::make_tuple(Args...);

    using R = decltype(F(Args...));

    ClangRuntimeSpecializer::InstructionCounts Before, After;

    // Call baseline instrumented (no optimization)
    RS->resetCounters();
    if constexpr (std::is_void_v<R>) {
        std::apply([&](auto&&... CallArgs) {
            RS->callSpecialized<void>(funcName,
                ClangRuntimeSpecializer::Options::NoOptimize().withInstructionInstrumentation(true),
                std::forward<std::decay_t<decltype(CallArgs)>>(CallArgs)...);
        }, ArgsOrig);
    } else {
        std::apply([&](auto&&... CallArgs) {
            RS->callSpecialized<R>(funcName,
                ClangRuntimeSpecializer::Options::NoOptimize().withInstructionInstrumentation(true),
                std::forward<std::decay_t<decltype(CallArgs)>>(CallArgs)...);
        }, ArgsOrig);
    }
    Before = RS->getCurrentCounters();

    // Call specialized instrumented (with optimization)
    RS->resetCounters();
    if constexpr (std::is_void_v<R>) {
        std::apply([&](auto&&... CallArgs) {
            RS->callSpecialized<void>(funcName,
                ClangRuntimeSpecializer::Options::Default().withInstructionInstrumentation(true),
                std::forward<decltype(CallArgs)>(CallArgs)...);
        }, ArgsSpec);
    } else {
        std::apply([&](auto&&... CallArgs) {
            RS->callSpecialized<R>(funcName,
                ClangRuntimeSpecializer::Options::Default().withInstructionInstrumentation(true),
                std::forward<decltype(CallArgs)>(CallArgs)...);
        }, ArgsSpec);
    }
    After = RS->getCurrentCounters();

    ClangRuntimeSpecializer::printComparisonTable(funcName, Before, After);
  }

} // namespace clangRuntimeSpecializer
