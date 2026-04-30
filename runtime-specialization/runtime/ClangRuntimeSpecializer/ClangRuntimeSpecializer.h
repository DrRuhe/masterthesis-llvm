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
  }

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
