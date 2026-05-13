#pragma once

#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <chrono>
#include <future>
#include <thread>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Cloning.h"

// Sentinel detected by IRDumpingPass to find the lambda proxy function.
// Defined in ClangRuntimeSpecializer.cpp as a no-op; noinline prevents DCE.
extern "C" __attribute__((noinline)) void __crs_op_hint(void*);

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

  // Forward declaration for SpecializedFunction friendship.
  class ClangRuntimeSpecializer;

  namespace detail {
    // Defined in ClangRuntimeSpecializer.cpp; calls consumeError(ES->removeJITDylib(*Dylib)).
    // Lives outside the header to avoid pulling LLVM RTTI symbols into user translation units.
    void removeJITDylibNoexcept(llvm::orc::ExecutionSession* ES,
                                llvm::orc::JITDylib* Dylib) noexcept;

    // T002: Extract return type and explicit arg types from a lambda's operator().
    // Primary template — triggers static_assert for generic lambdas or non-callables.
    template <class F> struct LambdaTraits {
      static_assert(sizeof(F) == 0,
        "specializeLambda: Lambda must have a single non-generic operator(). "
        "Generic lambdas (auto params) are not supported.");
    };
    template <class L, class R, class... Args>
    struct LambdaTraits<R (L::*)(Args...) const> {
      using RetType   = R;
      using ArgTypes  = std::tuple<Args...>;
      using FnPtrType = R(*)(Args...);
    };
    template <class L, class R, class... Args>
    struct LambdaTraits<R (L::*)(Args...)> {  // mutable lambda
      using RetType   = R;
      using ArgTypes  = std::tuple<Args...>;
      using FnPtrType = R(*)(Args...);
    };

    // Convenience alias: tuple of explicit arg types of Lambda::operator().
    template <class Lambda>
    using LambdaExplicitArgs =
        typename LambdaTraits<decltype(&Lambda::operator())>::ArgTypes;

    // T003: Map a C++ type to its LLVM IR Type* without needing a runtime value.
    template <class T>
    static llvm::Type* serializeTypeToLLVM(llvm::LLVMContext& Ctx) {
      using D = std::decay_t<T>;
      if constexpr (std::is_same_v<D, bool>)
        return llvm::Type::getInt1Ty(Ctx);
      else if constexpr (std::is_integral_v<D>)
        return llvm::Type::getIntNTy(Ctx, static_cast<unsigned>(sizeof(D) * 8));
      else if constexpr (std::is_same_v<D, float>)
        return llvm::Type::getFloatTy(Ctx);
      else if constexpr (std::is_floating_point_v<D>)
        return llvm::Type::getDoubleTy(Ctx);
      else  // pointer or class → opaque pointer
        return llvm::PointerType::getUnqual(Ctx);
    }

    // Expand a std::tuple<T0, T1, ...> into a vector of LLVM Type*.
    template <class Tuple, std::size_t... I>
    std::vector<llvm::Type*> tupleToLLVMTypes(llvm::LLVMContext& Ctx,
                                              std::index_sequence<I...>) {
      return { serializeTypeToLLVM<std::tuple_element_t<I, Tuple>>(Ctx)... };
    }

    // Typed proxy for a lambda's operator().
    // The IRDumpingPass finds this function by reading arg 0 of __crs_op_hint via
    // dyn_cast<Function>(arg->stripPointerCasts()) — pure LLVM API, no name parsing.
    // Its body calls (*self)(args...), so the optimizer inlines through to operator().
    template <class R, class Lambda, class... Args>
    R __crs_lambda_op_proxy(Lambda* self, Args... args) {
      return (*self)(args...);
    }

    // Force the lambda's operator() to be emitted and make the proxy visible to
    // the IRDumpingPass.  Taking the address of __crs_lambda_op_proxy ODR-uses it
    // and its body, which in turn ODR-uses operator().  The pass finds the proxy by
    // detecting calls to the __crs_op_hint sentinel and reading its first argument.
    template <class R, class Lambda, class... Args>
    __attribute__((noinline))
    void forceLambdaOpEmit(Lambda& /*unused*/, std::tuple<Args...>*) {
      __crs_op_hint(reinterpret_cast<void*>(&__crs_lambda_op_proxy<R, Lambda, Args...>));
    }
  } // namespace detail

  /// RAII wrapper for a JIT-compiled specialization.
  /// Owns the associated JITDylib — frees compiled machine code on destruction.
  template <class R>
  class SpecializedFunction {
    friend class ClangRuntimeSpecializer;

    SpecializedFunction(R(*fp)(), llvm::orc::JITDylib& dylib,
                        llvm::orc::ExecutionSession& es) noexcept
        : FnPtr(fp), Dylib(&dylib), ES(&es) {}

  public:
    SpecializedFunction() noexcept = default;

    SpecializedFunction(SpecializedFunction&& o) noexcept
        : FnPtr(o.FnPtr), Dylib(o.Dylib), ES(o.ES)
    { o.FnPtr = nullptr; o.Dylib = nullptr; o.ES = nullptr; }

    SpecializedFunction& operator=(SpecializedFunction&& o) noexcept {
        if (this != &o) {
            cleanup();
            FnPtr = o.FnPtr; Dylib = o.Dylib; ES = o.ES;
            o.FnPtr = nullptr; o.Dylib = nullptr; o.ES = nullptr;
        }
        return *this;
    }

    SpecializedFunction(const SpecializedFunction&) = delete;
    SpecializedFunction& operator=(const SpecializedFunction&) = delete;

    ~SpecializedFunction() { cleanup(); }

    R operator()() const { return FnPtr(); }
    R call()       const { return FnPtr(); }

    explicit operator bool() const noexcept { return FnPtr != nullptr; }

  private:
    void cleanup() noexcept {
        if (Dylib && ES)
            detail::removeJITDylibNoexcept(ES, Dylib);
        FnPtr = nullptr; Dylib = nullptr; ES = nullptr;
    }

    R(*FnPtr)()                   = nullptr;
    llvm::orc::JITDylib*          Dylib = nullptr;
    llvm::orc::ExecutionSession*  ES    = nullptr;
  };

  // T004: RAII wrapper for a partially-specialized lambda.
  // Owns the JITDylib; frees compiled machine code on destruction.
  // operator()(Args...) calls the JIT-compiled function with runtime args.
  template <class R, class... Args>
  class SpecializedLambda {
    friend class ClangRuntimeSpecializer;

    SpecializedLambda(R(*fp)(Args...), llvm::orc::JITDylib& dylib,
                      llvm::orc::ExecutionSession& es) noexcept
        : FnPtr(fp), Dylib(&dylib), ES(&es) {}

  public:
    SpecializedLambda() noexcept = default;

    SpecializedLambda(SpecializedLambda&& o) noexcept
        : FnPtr(o.FnPtr), Dylib(o.Dylib), ES(o.ES)
    { o.FnPtr = nullptr; o.Dylib = nullptr; o.ES = nullptr; }

    SpecializedLambda& operator=(SpecializedLambda&& o) noexcept {
        if (this != &o) {
            cleanup();
            FnPtr = o.FnPtr; Dylib = o.Dylib; ES = o.ES;
            o.FnPtr = nullptr; o.Dylib = nullptr; o.ES = nullptr;
        }
        return *this;
    }

    SpecializedLambda(const SpecializedLambda&) = delete;
    SpecializedLambda& operator=(const SpecializedLambda&) = delete;

    ~SpecializedLambda() { cleanup(); }

    R operator()(Args... args) const {
        if (!FnPtr)
            throw ClangRuntimeSpecializerError("SpecializedLambda: not initialized or timed out");
        return FnPtr(std::forward<Args>(args)...);
    }
    R call(Args... args) const { return operator()(std::forward<Args>(args)...); }

    explicit operator bool() const noexcept { return FnPtr != nullptr; }

  private:
    void cleanup() noexcept {
        if (Dylib && ES)
            detail::removeJITDylibNoexcept(ES, Dylib);
        FnPtr = nullptr; Dylib = nullptr; ES = nullptr;
    }

    R(*FnPtr)(Args...)             = nullptr;
    llvm::orc::JITDylib*          Dylib = nullptr;
    llvm::orc::ExecutionSession*  ES    = nullptr;
  };

  class ClangRuntimeSpecializer {
  public:
    enum class LogLevel {
      None,
      Error,
      Warning,
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
      int OptimizationPipelineToUse = 0;    // 0 = inlining pipeline, 1 = function-specialization pipeline

      // --- Budget metadata (set by FromExpectedRuntime; stored for logging/counter export) ---
      double ExpectedCallDurationNs = 0.0;  // 0 = not set
      double BudgetScale            = 1.0;

      // --- Timeout ---
      unsigned JITTimeoutSeconds = 0;       // 0 = no timeout; best-effort wall-clock limit on JIT compile

      // --- Debug / analysis ---
      bool Optimize = true;                          // when false → skips all JIT optimization passes
      bool EnableInstructionInstrumentation = false;
      bool KeepDebugInfo = false;
      bool PrintFixpointIterations = false;
      std::string TimeTraceOutputPath;       // Chrome trace JSON path; "" = disabled

      // --- Preset factories ---
      static Options Default() {
        // ENV-var overrides (read once per process — each optimizer trial is a fresh subprocess).
        static const int      kFixpoint        = (int)_envOr("CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS",      10.0);
        static const int      kUnroll          = (int)_envOr("CRS_DEFAULT_LOOP_UNROLL_COUNT",            128.0);
        static const size_t   kLargeMod        = (size_t)_envOr("CRS_DEFAULT_LARGE_MODULE_INSTR_THRESHOLD", 10000.0);
        static const bool     kEarlyPrune      = _envOr("CRS_DEFAULT_EARLY_PRUNE", 1.0) != 0.0;
        static const bool     kO3Final         = _envOr("CRS_DEFAULT_O3_FINAL",    1.0) != 0.0;
        static const int      kPipeline        = (int)_envOr("CRS_DEFAULT_PIPELINE",                    0.0);
        static const unsigned kFuncSpecMaxGroups = (unsigned)_envOr("CRS_DEFAULT_FUNC_SPEC_MAX_GROUPS",  0.0);
        Options O;
        O.MaxFixpointIterations     = kFixpoint;
        O.LoopUnrollCount           = kUnroll;
        O.LargeModuleInstrThreshold = kLargeMod;
        O.EnableEarlyPrune          = kEarlyPrune;
        O.EnableO3Final             = kO3Final;
        O.OptimizationPipelineToUse = kPipeline;
        O.FuncSpecMaxGroups         = kFuncSpecMaxGroups;
        return O;
      }
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
        // ENV-var-overridable magic numbers (read once per process on first call).
        // Optimizer trials run as fresh subprocesses, so static init is correct.
        static const double kLog2Min     = _envOr("CRS_LOG2_MIN",         10.0);
        static const double kLog2Max     = _envOr("CRS_LOG2_MAX",         30.0);
        static const double kFixpointMax = _envOr("CRS_FIXPOINT_MAX",     20.0);
        static const double kUnrollMax   = _envOr("CRS_UNROLL_MAX",      256.0);
        static const double kLargeModMax = _envOr("CRS_LARGE_MODULE_MAX", 50000.0);
        static const double kO3CutoffNs  = _envOr("CRS_O3_CUTOFF_NS",    1000.0);

        const double scaledNs = std::max(callDurationNs, 1.0) * std::max(budgetScale, 0.01);
        const double t = std::clamp((std::log2(scaledNs) - kLog2Min) / (kLog2Max - kLog2Min),
                                     0.0, 1.0);
        if (scaledNs < kO3CutoffNs) {
          Options O = Options::O3Only();
          O.ExpectedCallDurationNs = callDurationNs; O.BudgetScale = budgetScale; return O;
        }
        auto lerp = [](double a, double b, double tt) { return a + tt * (b - a); };
        Options O;
        O.MaxFixpointIterations     = static_cast<int>(std::round(lerp(0.0, kFixpointMax, t)));
        O.LoopUnrollCount           = static_cast<int>(std::round(lerp(1.0, kUnrollMax,   t)));
        O.LargeModuleInstrThreshold = static_cast<size_t>(std::round(lerp(0.0, kLargeModMax, t)));
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
      Options& withOptimizationPipeline(int P)        { OptimizationPipelineToUse = P; return *this; }
      Options& withExpectedCallDurationNs(double V)   { ExpectedCallDurationNs = V; return *this; }
      Options& withBudgetScale(double V)              { BudgetScale = V; return *this; }
      Options& withJITTimeoutSeconds(unsigned V)      { JITTimeoutSeconds = V; return *this; }

    private:
      static double _envOr(const char* name, double def) noexcept {
        const char* v = std::getenv(name);
        if (!v) return def;
        try { return std::stod(v); } catch (...) { return def; }
      }
    };

    static ClangRuntimeSpecializer* init();

    ClangRuntimeSpecializer* setOptions(Options Opts) {
      CurrentOptions = std::move(Opts);
      return this;
    }

    // Implementation detail: overload disambiguation trait (not for external use).
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

    template <class R, class... ARGS,
              std::enable_if_t<FirstArgIsNotOptions<ARGS...>::value, int> = 0>
    __attribute__((noinline))
    auto specializeOnly(const char* funcName, ARGS&&... Args) -> SpecializedFunction<R> {
      auto Res = specializeOnlyImpl(funcName, CurrentOptions, std::forward<ARGS>(Args)...);
      if (Res.TimedOut) return SpecializedFunction<R>{};
      return SpecializedFunction<R>(reinterpret_cast<R(*)()>(Res.Addr), *Res.Dylib, JIT->getExecutionSession());
    }

    template <class R, class... ARGS>
    __attribute__((noinline))
    auto specializeOnly(const char* funcName, const Options& opts, ARGS&&... Args) -> SpecializedFunction<R> {
      auto Res = specializeOnlyImpl(funcName, opts, std::forward<ARGS>(Args)...);
      if (Res.TimedOut) return SpecializedFunction<R>{};
      return SpecializedFunction<R>(reinterpret_cast<R(*)()>(Res.Addr), *Res.Dylib, JIT->getExecutionSession());
    }

    // specializeOnly with function pointer — no funcName string needed.
    // The IRDumpingPass detects these call sites, extracts the function name from the
    // compile-time-constant pointer, creates @__crs_resolved_name_K, and rewrites to
    // specializeOnlyResolved(resolvedName, func, args...).
    template <class R, class F, class... ARGS,
              std::enable_if_t<std::is_function_v<F>, int> = 0>
    __attribute__((noinline))
    auto specializeOnly(F* func, ARGS&&... Args) -> SpecializedFunction<R> {
      return specializeOnlyResolved<R>(nullptr, func, std::forward<ARGS>(Args)...);
    }

    template <class R, class F, class... ARGS,
              std::enable_if_t<std::is_function_v<F>, int> = 0>
    __attribute__((noinline))
    auto specializeOnly(F* func, const Options& opts, ARGS&&... Args) -> SpecializedFunction<R> {
      return specializeOnlyResolved<R>(nullptr, func, opts, std::forward<ARGS>(Args)...);
    }

    // callSpecialized with function pointer — no funcName string needed.
    // Same IRDumpingPass rewriting as specializeOnly above.
    template <class R, class F, class... ARGS,
              std::enable_if_t<std::is_function_v<F>, int> = 0>
    __attribute__((noinline))
    R callSpecialized(F* func, ARGS&&... Args) {
      return callSpecializedResolved<R>(nullptr, func, std::forward<ARGS>(Args)...);
    }

    template <class R, class F, class... ARGS,
              std::enable_if_t<std::is_function_v<F>, int> = 0>
    __attribute__((noinline))
    R callSpecialized(F* func, const Options& opts, ARGS&&... Args) {
      return callSpecializedResolved<R>(nullptr, func, opts, std::forward<ARGS>(Args)...);
    }

    // specializeLambda — bake in lambda closure, keep explicit args variable.
    // The IRDumpingPass detects calls to these overloads (noinline preserves the call site
    // through the plugin pass), resolves the lambda operator() mangled name, and rewrites
    // each call site to specializeLambdaResolved(resolvedName, lambda [, opts]).
    // The nullptr passed here forces template instantiation of specializeLambdaResolved
    // so the pass can find the function definition in the module.
    template <class R, class Lambda,
              std::enable_if_t<FirstArgIsNotOptions<Lambda>::value, int> = 0>
    __attribute__((noinline))
    auto specializeLambda(Lambda& lambda) {
      detail::forceLambdaOpEmit<R>(lambda,
          static_cast<detail::LambdaExplicitArgs<Lambda>*>(nullptr));
      return specializeLambdaResolved<R>(nullptr, lambda);
    }

    template <class R, class Lambda>
    __attribute__((noinline))
    auto specializeLambda(Lambda& lambda, const Options& opts) {
      detail::forceLambdaOpEmit<R>(lambda,
          static_cast<detail::LambdaExplicitArgs<Lambda>*>(nullptr));
      return specializeLambdaResolved<R>(nullptr, lambda, opts);
    }

    ~ClangRuntimeSpecializer();

  private:

    struct JITResult {
      uintptr_t            Addr     = 0;
      llvm::orc::JITDylib* Dylib   = nullptr;
      bool                 TimedOut = false;
    };

    template <class... ARGS>
    JITResult specializeOnlyImpl(const char* funcName, const Options& Opts, ARGS&&... Args) {
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
      std::unique_ptr<llvm::Module> NewModule;
      // In debug builds, use a fresh LLVMContext for each specialization call.
      // Reusing TSCtx causes pImpl->ValueHandles to accumulate WeakVH/AssertingVH
      // handles across calls (created by optimization passes such as JumpThreading,
      // LazyValueInfo, AssumptionCache).  Each call grows the DenseMap, frees old
      // buckets, and potentially leaves stale chain-head PrevPtrs.  Subsequent
      // pImpl->ValueNames insertions during the next bitcode parse can then observe
      // these stale writes and hit "No name entry found!" (debug-only).
      // A per-call fresh context has no accumulated handles — optimization handles
      // are created and destroyed entirely within that context's lifetime.
      // In release builds CloneModule into TSCtx is fine (AssertingVH/WeakVH are
      // plain pointers without the debug DenseMap tracking).
#ifndef NDEBUG
      auto FreshLLVMCtx = std::make_unique<llvm::LLVMContext>();
      llvm::LLVMContext* FreshCtxPtr = FreshLLVMCtx.get();
      llvm::orc::ThreadSafeContext NewTSCtx(std::move(FreshLLVMCtx));
      {
        llvm::SmallVector<char, 0> BC;
        llvm::raw_svector_ostream OS(BC);
        llvm::WriteBitcodeToFile(*TargetFunc->getParent(), OS);
        auto Buf = llvm::MemoryBuffer::getMemBufferCopy(
            llvm::StringRef(BC.data(), BC.size()),
            TargetFunc->getParent()->getName());
        llvm::SMDiagnostic Err;
        NewModule = llvm::parseIR(*Buf, Err, *FreshCtxPtr);
        if (!NewModule)
          llvm::report_fatal_error("specializeOnlyImpl: bitcode re-parse failed");
      }
#else
      llvm::orc::ThreadSafeContext NewTSCtx = TSCtx;
      NewModule = llvm::CloneModule(*TargetFunc->getParent());
#endif
      auto* TargetFuncInNewModule = NewModule->getFunction(TargetFunc->getName());
      encourageInlining(TargetFuncInNewModule);

      llvm::LLVMContext& Ctx = NewModule->getContext();

      llvm::FunctionType* const FTy = llvm::FunctionType::get(TargetFuncInNewModule->getReturnType(), false);

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

      if (TargetFuncInNewModule->getReturnType()->isVoidTy()) {
          Builder.CreateRetVoid();
      } else {
          Builder.CreateRet(CallInst);
      }

      prepareModuleForJIT(*NewModule, UniqueWrapperName);

      auto TSM = llvm::orc::ThreadSafeModule(std::move(NewModule), std::move(NewTSCtx));

      CurrentCallOptions = Opts;

      if (Opts.JITTimeoutSeconds > 0) {
        // Run JIT in a detached thread; wait up to JITTimeoutSeconds.
        // If timeout elapses, return a null result. The background thread
        // completes independently — no further JIT calls should be made
        // until it finishes (callers guard with g_last_jit_timed_out).
        std::string wrapperCopy = UniqueWrapperName;
        std::string funcNameStr(funcName);
        auto promise = std::make_shared<std::promise<JITResult>>();
        auto future = promise->get_future();
        std::thread([this, tsm = std::move(TSM), wrapperCopy, funcNameStr,
                     p = std::move(promise)]() mutable {
          try {
            p->set_value(addModuleAndLookup(std::move(tsm), wrapperCopy, funcNameStr));
          } catch (...) {
            try { p->set_exception(std::current_exception()); } catch (...) {}
          }
        }).detach();

        if (future.wait_for(std::chrono::seconds(Opts.JITTimeoutSeconds)) ==
            std::future_status::timeout) {
          log(LogLevel::Warning,
              (llvm::Twine("JIT timeout (") + std::to_string(Opts.JITTimeoutSeconds) +
               "s) for: " + funcName).str());
          return {0, nullptr, /*TimedOut=*/true};
        }
        return future.get();
      }

      return addModuleAndLookup(std::move(TSM), UniqueWrapperName, std::string(funcName));
    }

    template <class R, class... ARGS>
    R callImpl(const char* funcName, const Options& Opts, ARGS&&... Args) {
      auto Res = specializeOnlyImpl(funcName, Opts, std::forward<ARGS>(Args)...);
      if (Res.TimedOut)
        throw ClangRuntimeSpecializerError(
            std::string("callSpecialized: JIT timed out for ") + funcName);
      SpecializedFunction<R> Fn(reinterpret_cast<R(*)()>(Res.Addr), *Res.Dylib, JIT->getExecutionSession());
      if constexpr (std::is_void_v<R>) {
          Fn();
          return;
      } else {
          return Fn();
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
    Options CurrentOptions = Options::Default();  // default options used by specializeOnly() / callSpecialized()
    Options CurrentCallOptions;  // per-invocation options set by specializeOnlyImpl() before JIT
    std::vector<std::unique_ptr<char[]>> SerializationBuffers;

    void checkInitialization(const char* funcName) const;
    llvm::Function* getTargetFunction(const char* funcName);
    void validateArgs(llvm::Function* TargetFunc, size_t NumArgs) const;
    std::string createUniqueWrapperName() const;
    void prepareModuleForJIT(llvm::Module& M, const std::string& WrapperName) const;
    JITResult addModuleAndLookup(llvm::orc::ThreadSafeModule TSM, const std::string& WrapperName,
                                  const std::string& OrigFuncName = {});
    uint64_t dumpJITAssembly(const std::string& OrigFuncName, uintptr_t Addr);
    static void encourageInlining(llvm::Function* F);

    // T005: Partial specialization — lambda closure is baked in; explicit arg types remain variable.
    // Returns JITResult containing function pointer of type R(*)(ExplicitArgs...).
    template <class Lambda>
    JITResult specializeLambdaImpl(const char* funcName, Lambda& lambda, const Options& Opts) {
      using ArgTuple = detail::LambdaExplicitArgs<Lambda>;
      constexpr std::size_t NArgs = std::tuple_size_v<ArgTuple>;

      // Zero-arg lambda: no explicit parameters — the closure is the sole baked argument.
      // Delegate to specializeOnlyImpl, unifying this code path with specializeOnly.
      if constexpr (NArgs == 0) {
        return specializeOnlyImpl(funcName, Opts, &lambda);
      }

      checkInitialization(funcName);
      llvm::Function* TargetFunc = getTargetFunction(funcName);

      // The lambda closure is the single constant arg; the explicit args stay variable.
      // So the kernel is expected to have NArgs+1 parameters total.
      validateArgs(TargetFunc, NArgs + 1);

      log(LogLevel::Info, (llvm::Twine("Specializing lambda call to: ") + funcName).str());

      std::string UniqueWrapperName = createUniqueWrapperName() + "_lambda" +
                                       (Opts.Optimize ? "" : "_no_opt");

      std::unique_ptr<llvm::Module> NewModule;
#ifndef NDEBUG
      auto FreshLLVMCtx = std::make_unique<llvm::LLVMContext>();
      llvm::LLVMContext* FreshCtxPtr = FreshLLVMCtx.get();
      llvm::orc::ThreadSafeContext NewTSCtx(std::move(FreshLLVMCtx));
      {
        llvm::SmallVector<char, 0> BC;
        llvm::raw_svector_ostream OS(BC);
        llvm::WriteBitcodeToFile(*TargetFunc->getParent(), OS);
        auto Buf = llvm::MemoryBuffer::getMemBufferCopy(
            llvm::StringRef(BC.data(), BC.size()),
            TargetFunc->getParent()->getName());
        llvm::SMDiagnostic Err;
        NewModule = llvm::parseIR(*Buf, Err, *FreshCtxPtr);
        if (!NewModule)
          llvm::report_fatal_error("specializeLambdaImpl: bitcode re-parse failed");
      }
#else
      llvm::orc::ThreadSafeContext NewTSCtx = TSCtx;
      NewModule = llvm::CloneModule(*TargetFunc->getParent());
#endif
      auto* TargetFuncInNewModule = NewModule->getFunction(TargetFunc->getName());
      encourageInlining(TargetFuncInNewModule);

      llvm::LLVMContext& Ctx = NewModule->getContext();

      // Build wrapper function type: R(ExplicitArgType0, ExplicitArgType1, ...)
      std::vector<llvm::Type*> ParamTys =
          detail::tupleToLLVMTypes<ArgTuple>(Ctx, std::make_index_sequence<NArgs>{});
      llvm::FunctionType* WrapperTy =
          llvm::FunctionType::get(TargetFuncInNewModule->getReturnType(), ParamTys, false);
      llvm::Function* WrapperFn = llvm::Function::Create(
          WrapperTy, llvm::Function::ExternalLinkage, UniqueWrapperName, *NewModule);
      if (!Opts.Optimize)
        WrapperFn->addFnAttr("force-no-optimize");

      llvm::BasicBlock* BB = llvm::BasicBlock::Create(Ctx, "entry", WrapperFn);
      llvm::IRBuilder<> Builder(BB);

      // Serialize the lambda object (closure struct) as the first constant arg.
      std::vector<llvm::Value*> CallArgs;
      CallArgs.push_back(serializeArgumentToIR(Builder, lambda));

      // Forward the wrapper's live parameters (the explicit args) unchanged.
      for (auto& Arg : WrapperFn->args())
        CallArgs.push_back(&Arg);

      auto* CI = Builder.CreateCall(TargetFuncInNewModule->getFunctionType(),
                                    TargetFuncInNewModule, CallArgs);
      CI->setAttributes(TargetFuncInNewModule->getAttributes());
      CI->addFnAttr(llvm::Attribute::AlwaysInline);

      if (TargetFuncInNewModule->getReturnType()->isVoidTy())
        Builder.CreateRetVoid();
      else
        Builder.CreateRet(CI);

      prepareModuleForJIT(*NewModule, UniqueWrapperName);

      auto TSM = llvm::orc::ThreadSafeModule(std::move(NewModule), std::move(NewTSCtx));
      CurrentCallOptions = Opts;

      return addModuleAndLookup(std::move(TSM), UniqueWrapperName, std::string(funcName));
    }

    // Unpack ArgTuple into SpecializedLambda<R, Args...> using tag dispatch.
    template <class R, class... Args>
    SpecializedLambda<R, Args...> makeSpecLambda(JITResult Res, std::tuple<Args...>*) {
      if (Res.TimedOut || !Res.Addr) return {};
      using FP = R(*)(Args...);
      return SpecializedLambda<R, Args...>(
          reinterpret_cast<FP>(Res.Addr), *Res.Dylib, JIT->getExecutionSession());
    }

    template <class R, class Lambda>
    auto specializeLambdaWithOpts(const char* funcName, Lambda& lambda, const Options& opts) {
      using ArgTuple = detail::LambdaExplicitArgs<Lambda>;
      auto Res = specializeLambdaImpl<Lambda>(funcName, lambda, opts);
      return makeSpecLambda<R>(Res, static_cast<ArgTuple*>(nullptr));
    }

  public:
    // Internal variants called only from IRDumpingPass-rewritten call sites.
    // The pass inserts the compile-time-resolved mangled name as the first argument.
    // A null resolvedName means the TU was not compiled with the plugin.

    template <class R, class Lambda>
    auto specializeLambdaResolved(const char* resolvedName, Lambda& lambda) {
      if (!resolvedName)
        throw ClangRuntimeSpecializerDumpedIRError(
            "specializeLambda: TU was not compiled with the IRDumpingPass plugin");
      return specializeLambdaWithOpts<R>(resolvedName, lambda, CurrentOptions);
    }

    template <class R, class Lambda>
    auto specializeLambdaResolved(const char* resolvedName, Lambda& lambda, const Options& opts) {
      if (!resolvedName)
        throw ClangRuntimeSpecializerDumpedIRError(
            "specializeLambda: TU was not compiled with the IRDumpingPass plugin");
      return specializeLambdaWithOpts<R>(resolvedName, lambda, opts);
    }

    template <class R, class F, class... ARGS>
    __attribute__((noinline))
    auto specializeOnlyResolved(const char* resolvedName, F* /*unused*/, ARGS&&... Args) -> SpecializedFunction<R> {
      if (!resolvedName)
        throw ClangRuntimeSpecializerDumpedIRError(
            "specializeOnly: TU was not compiled with the IRDumpingPass plugin");
      auto Res = specializeOnlyImpl(resolvedName, CurrentOptions, std::forward<ARGS>(Args)...);
      if (Res.TimedOut) return SpecializedFunction<R>{};
      return SpecializedFunction<R>(reinterpret_cast<R(*)()>(Res.Addr), *Res.Dylib, JIT->getExecutionSession());
    }

    template <class R, class F, class... ARGS>
    __attribute__((noinline))
    auto specializeOnlyResolved(const char* resolvedName, F* /*unused*/, const Options& opts, ARGS&&... Args) -> SpecializedFunction<R> {
      if (!resolvedName)
        throw ClangRuntimeSpecializerDumpedIRError(
            "specializeOnly: TU was not compiled with the IRDumpingPass plugin");
      auto Res = specializeOnlyImpl(resolvedName, opts, std::forward<ARGS>(Args)...);
      if (Res.TimedOut) return SpecializedFunction<R>{};
      return SpecializedFunction<R>(reinterpret_cast<R(*)()>(Res.Addr), *Res.Dylib, JIT->getExecutionSession());
    }

    template <class R, class F, class... ARGS>
    __attribute__((noinline))
    R callSpecializedResolved(const char* resolvedName, F* /*unused*/, ARGS&&... Args) {
      if (!resolvedName)
        throw ClangRuntimeSpecializerDumpedIRError(
            "callSpecialized: TU was not compiled with the IRDumpingPass plugin");
      return callImpl<R>(resolvedName, CurrentOptions, std::forward<ARGS>(Args)...);
    }

    template <class R, class F, class... ARGS>
    __attribute__((noinline))
    R callSpecializedResolved(const char* resolvedName, F* /*unused*/, const Options& opts, ARGS&&... Args) {
      if (!resolvedName)
        throw ClangRuntimeSpecializerDumpedIRError(
            "callSpecialized: TU was not compiled with the IRDumpingPass plugin");
      return callImpl<R>(resolvedName, opts, std::forward<ARGS>(Args)...);
    }

  private:
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

  // ── callSpecializedResolved and callSpecialized (funcptr) free variants ────────
  // Defined early so specializeOrFallbackResolved / assertSpecializedIsEquivalentResolved
  // can call them. The IRDumpingPass detects noinline calls to callSpecialized and
  // rewrites them to callSpecializedResolved with the compile-time-resolved name.

  template <class R, class F, class... ARGS,
            std::enable_if_t<std::is_function_v<F>, int> = 0>
  R callSpecializedResolved(const char* resolvedName, F* func, ARGS&&... Args) {
    auto* RS = ClangRuntimeSpecializer::init();
    return RS->callSpecializedResolved<R>(resolvedName, func, std::forward<ARGS>(Args)...);
  }

  template <class R, class F, class... ARGS,
            std::enable_if_t<std::is_function_v<F>, int> = 0>
  R callSpecializedResolved(const char* resolvedName, F* func,
                            const ClangRuntimeSpecializer::Options& opts, ARGS&&... Args) {
    auto* RS = ClangRuntimeSpecializer::init();
    return RS->callSpecializedResolved<R>(resolvedName, func, opts, std::forward<ARGS>(Args)...);
  }

  template <class R, class F, class... ARGS,
            std::enable_if_t<std::is_function_v<F>, int> = 0>
  __attribute__((noinline))
  R callSpecialized(F* func, ARGS&&... Args) {
    return callSpecializedResolved<R>(nullptr, func, std::forward<ARGS>(Args)...);
  }

  template <class R, class F, class... ARGS,
            std::enable_if_t<std::is_function_v<F>, int> = 0>
  __attribute__((noinline))
  R callSpecialized(F* func, const ClangRuntimeSpecializer::Options& opts, ARGS&&... Args) {
    return callSpecializedResolved<R>(nullptr, func, opts, std::forward<ARGS>(Args)...);
  }

  // ─────────────────────────────────────────────────────────────────────────────
  // Resolved variants for specializeOrFallback and assertSpecializedIsEquivalent.
  // The IRDumpingPass detects calls to the user-facing (noinline) wrappers,
  // finds these Resolved variants in the callee body via findSpecOnlyResolvedInBody,
  // and rewrites the outer call sites to invoke these directly with the baked-in name.

  template <class Callable, class... ARGS>
  decltype(auto) specializeOrFallbackResolved(const char* resolvedName, Callable C, ARGS&&... Args) {
    auto Invoke = [&]() -> decltype(auto) {
      return std::invoke(C, std::forward<ARGS>(Args)...);
    };
    if constexpr (std::is_function_v<std::remove_pointer_t<std::decay_t<Callable>>>) {
      try {
        if (ClangRuntimeSpecializer::init()) {
          using R = decltype(Invoke());
          if constexpr (std::is_void_v<R>) {
            callSpecializedResolved<void>(resolvedName, C, std::forward<ARGS>(Args)...);
            return;
          } else {
            return callSpecializedResolved<R>(resolvedName, C, std::forward<ARGS>(Args)...);
          }
        }
      } catch (const std::exception& E) {
        CRS_LOG(Error, (llvm::Twine("Specialization failed: ") + E.what()).str());
      } catch (...) {
        CRS_LOG(Error, "Specialization failed with an unknown error.");
      }
    }
    if constexpr (std::is_void_v<decltype(Invoke())>) {
      Invoke();
      return;
    } else {
      return Invoke();
    }
  }

  // specializeOrFallback: noinline so IRDumpingPass can detect the call site and
  // extract the compile-time-constant function pointer to resolve the name.
  // The IRDumpingPass rewrites calls from user code to specializeOrFallbackResolved.
  // For non-function-pointer callables (lambdas, method ptrs), falls back to
  // std::invoke without attempting specialization.
  template <class Callable, class... ARGS>
  __attribute__((noinline))
  decltype(auto) specializeOrFallback(Callable C, ARGS&&... Args) {
    return specializeOrFallbackResolved(nullptr, C, std::forward<ARGS>(Args)...);
  }

  // Internal Resolved variant for assertSpecializedIsEquivalent.
  // The IRDumpingPass finds this call inside the user-facing wrapper's body
  // and rewrites the outer call site to invoke this directly with the baked-in name.
  template <class Fn, class Tuple, class Comparator>
  void assertSpecializedIsEquivalentResolved(const char* resolvedName, Fn F,
      Tuple&& normalArgs, Tuple&& specArgs, Comparator&& comp) {
    auto InvokeNormal = [&](auto&&... args) {
        return std::invoke(F, std::forward<decltype(args)>(args)...);
    };

    using R = decltype(std::apply(InvokeNormal, normalArgs));

    if constexpr (std::is_void_v<R>) {
        std::apply(InvokeNormal, normalArgs);
        std::apply([&](auto&&... CallArgs) {
            callSpecializedResolved<void>(resolvedName, F, std::forward<decltype(CallArgs)>(CallArgs)...);
        }, specArgs);
    } else {
        R ResOrig = std::apply(InvokeNormal, normalArgs);
        R ResSpec = std::apply([&](auto&&... CallArgs) -> R {
            return callSpecializedResolved<R>(resolvedName, F, std::forward<decltype(CallArgs)>(CallArgs)...);
        }, specArgs);
        if constexpr (HasEqualityOperator<R>::value) {
            if (ResOrig != ResSpec) {
                throw ClangRuntimeSpecializerChangesBehaviorError("Comparison failed: return values differ");
            }
        }
    }

    comp();
    CRS_LOG(Info, "Successfully specialized! No differences could be observed.");
  }

  // assertSpecializedIsEquivalent: noinline so IRDumpingPass can detect the call
  // site, extract the compile-time-constant function pointer F, and rewrite the
  // call to assertSpecializedIsEquivalentResolved(resolvedName, F, ...).
  template <class Fn, class Tuple, class Comparator>
  __attribute__((noinline))
  void assertSpecializedIsEquivalent(Fn F, Tuple&& normalArgs, Tuple&& specArgs, Comparator&& comp) {
    return assertSpecializedIsEquivalentResolved(nullptr, F,
        std::forward<Tuple>(normalArgs), std::forward<Tuple>(specArgs),
        std::forward<Comparator>(comp));
  }


  template <class Fn, class... ARGS>
  __attribute__((always_inline))
  void compareFunctionInstructionCounts(Fn F, ARGS... Args) {
    auto* RS = ClangRuntimeSpecializer::init();

    // Copy arguments for both calls to ensure same initial state
    auto ArgsOrig = std::make_tuple(Args...);
    auto ArgsSpec = std::make_tuple(Args...);

    using R = decltype(F(Args...));

    ClangRuntimeSpecializer::InstructionCounts Before, After;

    // Call baseline instrumented (no optimization) via free callSpecialized
    RS->resetCounters();
    if constexpr (std::is_void_v<R>) {
        std::apply([&](auto&&... CallArgs) {
            callSpecialized<void>(F,
                ClangRuntimeSpecializer::Options::NoOptimize().withInstructionInstrumentation(true),
                std::forward<std::decay_t<decltype(CallArgs)>>(CallArgs)...);
        }, ArgsOrig);
    } else {
        std::apply([&](auto&&... CallArgs) {
            callSpecialized<R>(F,
                ClangRuntimeSpecializer::Options::NoOptimize().withInstructionInstrumentation(true),
                std::forward<std::decay_t<decltype(CallArgs)>>(CallArgs)...);
        }, ArgsOrig);
    }
    Before = RS->getCurrentCounters();

    // Call specialized instrumented (with optimization) via free callSpecialized
    RS->resetCounters();
    if constexpr (std::is_void_v<R>) {
        std::apply([&](auto&&... CallArgs) {
            callSpecialized<void>(F,
                ClangRuntimeSpecializer::Options::Default().withInstructionInstrumentation(true),
                std::forward<decltype(CallArgs)>(CallArgs)...);
        }, ArgsSpec);
    } else {
        std::apply([&](auto&&... CallArgs) {
            callSpecialized<R>(F,
                ClangRuntimeSpecializer::Options::Default().withInstructionInstrumentation(true),
                std::forward<decltype(CallArgs)>(CallArgs)...);
        }, ArgsSpec);
    }
    After = RS->getCurrentCounters();

    ClangRuntimeSpecializer::printComparisonTable("", Before, After);
  }

  // ─── Legacy string-name overloads (kept for backward compat; removal is T033) ───

  template <class Callable, class... ARGS>
  __attribute__((always_inline))
  decltype(auto) specializeOrFallback(const char* funcName, Callable C, ARGS&&... Args) {
    auto Invoke = [&]() -> decltype(auto) {
      return std::invoke(C, std::forward<ARGS>(Args)...);
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
  void assertSpecializedIsEquivalent(const char* funcName, Fn F, Tuple&& normalArgs, Tuple&& specArgs, Comparator&& comp) {
    auto* RS = ClangRuntimeSpecializer::init();

    auto InvokeNormal = [&](auto&&... args) {
        return std::invoke(F, std::forward<decltype(args)>(args)...);
    };

    using R = decltype(std::apply(InvokeNormal, normalArgs));

    if constexpr (std::is_void_v<R>) {
        std::apply(InvokeNormal, normalArgs);
        std::apply([&](auto&&... CallArgs) {
            RS->callSpecialized<void>(funcName, std::forward<decltype(CallArgs)>(CallArgs)...);
        }, specArgs);
    } else {
        R ResOrig = std::apply(InvokeNormal, normalArgs);
        R ResSpec = std::apply([&](auto&&... CallArgs) -> R {
            return RS->callSpecialized<R>(funcName, std::forward<decltype(CallArgs)>(CallArgs)...);
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

    auto ArgsOrig = std::make_tuple(Args...);
    auto ArgsSpec = std::make_tuple(Args...);

    using R = decltype(F(Args...));

    ClangRuntimeSpecializer::InstructionCounts Before, After;

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

  // ─── End legacy overloads ─────────────────────────────────────────────────────

  // assertSpecializedLambdaIsEquivalent — calls specializeLambda once, checks all inputs.
  // Throws ClangRuntimeSpecializerChangesBehaviorError on return-value mismatch.
  template <class R, class Lambda, class InputRange>
  __attribute__((always_inline))
  void assertSpecializedLambdaIsEquivalent(Lambda& lambda, const InputRange& inputs) {
    auto* RS = ClangRuntimeSpecializer::init();
    auto spec = RS->specializeLambda<R>(lambda);
    for (const auto& args : inputs) {
      R expected = std::apply(lambda, args);
      R actual   = std::apply(spec,   args);
      if constexpr (HasEqualityOperator<R>::value) {
        if (expected != actual)
          throw ClangRuntimeSpecializerChangesBehaviorError(
              "assertSpecializedLambdaIsEquivalent: result mismatch");
      }
    }
    CRS_LOG(Info, "assertSpecializedLambdaIsEquivalent passed");
  }

  // Internal resolved variants — called only from IRDumpingPass-rewritten call sites.
  // The free functions mirror the member *Resolved variants so the pass can find
  // the right instantiation by looking inside the specializeLambda body.
  template <class R, class Lambda>
  auto specializeLambdaResolved(const char* resolvedName, Lambda& lambda) {
    auto* RS = ClangRuntimeSpecializer::init();
    return RS->specializeLambdaResolved<R>(resolvedName, lambda);
  }

  template <class R, class Lambda>
  auto specializeLambdaResolved(const char* resolvedName, Lambda& lambda,
                                const ClangRuntimeSpecializer::Options& opts) {
    auto* RS = ClangRuntimeSpecializer::init();
    return RS->specializeLambdaResolved<R>(resolvedName, lambda, opts);
  }

  // Specialize a lambda using the IRDumpingPass-resolved operator() name.
  // The IRDumpingPass rewrites this call to specializeLambdaResolved(resolvedName, lambda).
  // The specializeLambdaResolved(nullptr,...) calls below force template instantiation
  // so the pass can find the resolved function definition in the module.
  template <class R, class Lambda>
  __attribute__((noinline))
  auto specializeLambda(Lambda& lambda) {
    detail::forceLambdaOpEmit<R>(lambda,
        static_cast<detail::LambdaExplicitArgs<Lambda>*>(nullptr));
    return specializeLambdaResolved<R>(nullptr, lambda);
  }

  template <class R, class Lambda>
  __attribute__((noinline))
  auto specializeLambda(Lambda& lambda,
                        const ClangRuntimeSpecializer::Options& opts) {
    detail::forceLambdaOpEmit<R>(lambda,
        static_cast<detail::LambdaExplicitArgs<Lambda>*>(nullptr));
    return specializeLambdaResolved<R>(nullptr, lambda, opts);
  }

  // Internal resolved variants for specializeOnly — free-function wrappers
  // around the member *Resolved variants. The IRDumpingPass finds these in the body of the
  // free specializeOnly overloads below and uses them as rewrite targets.
  template <class R, class F, class... ARGS,
            std::enable_if_t<std::is_function_v<F>, int> = 0>
  auto specializeOnlyResolved(const char* resolvedName, F* func, ARGS&&... Args)
      -> SpecializedFunction<R> {
    auto* RS = ClangRuntimeSpecializer::init();
    return RS->specializeOnlyResolved<R>(resolvedName, func, std::forward<ARGS>(Args)...);
  }

  template <class R, class F, class... ARGS,
            std::enable_if_t<std::is_function_v<F>, int> = 0>
  auto specializeOnlyResolved(const char* resolvedName, F* func,
                              const ClangRuntimeSpecializer::Options& opts, ARGS&&... Args)
      -> SpecializedFunction<R> {
    auto* RS = ClangRuntimeSpecializer::init();
    return RS->specializeOnlyResolved<R>(resolvedName, func, opts, std::forward<ARGS>(Args)...);
  }

  // specializeOnly with function pointer — free function variant.
  // The IRDumpingPass detects these call sites, extracts the function name from the
  // compile-time-constant pointer, and rewrites to specializeOnlyResolved(resolvedName, func, args...).
  template <class R, class F, class... ARGS,
            std::enable_if_t<std::is_function_v<F>, int> = 0>
  __attribute__((noinline))
  auto specializeOnly(F* func, ARGS&&... Args) -> SpecializedFunction<R> {
    return specializeOnlyResolved<R>(nullptr, func, std::forward<ARGS>(Args)...);
  }

  template <class R, class F, class... ARGS,
            std::enable_if_t<std::is_function_v<F>, int> = 0>
  __attribute__((noinline))
  auto specializeOnly(F* func, const ClangRuntimeSpecializer::Options& opts, ARGS&&... Args)
      -> SpecializedFunction<R> {
    return specializeOnlyResolved<R>(nullptr, func, opts, std::forward<ARGS>(Args)...);
  }

} // namespace clangRuntimeSpecializer
