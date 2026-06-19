#pragma once
#include "ClangRuntimeSpecializer.h"
#include <algorithm>
#include <atomic>
#include <benchmark/benchmark.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/MemoryBuffer.h"

// Tracks process RSS growth across each benchmark run using Google Benchmark's
// MemoryManager API. Register once in main() with:
//   static RSSMemoryManager g_rss_mgr;
//   benchmark::RegisterMemoryManager(&g_rss_mgr);
// Results appear as max_bytes_used in JSON output.
class RSSMemoryManager : public benchmark::MemoryManager {
    size_t rss_before_ = 0;

    static size_t readVmRSS() {
        FILE* f = fopen("/proc/self/status", "r");
        if (!f) return 0;
        size_t rss = 0;
        char line[128];
        while (fgets(line, sizeof(line), f))
            if (sscanf(line, "VmRSS: %zu kB", &rss) == 1) break;
        fclose(f);
        return rss * 1024;
    }
public:
    void Start() override { rss_before_ = readVmRSS(); }
    void Stop(Result& result) override {
        size_t after = readVmRSS();
        result.max_bytes_used = after > rss_before_
            ? static_cast<int64_t>(after - rss_before_) : 0;
        result.num_allocs = 0;
    }
};

namespace clangRuntimeSpecializer {

namespace detail {

inline llvm::StringRef trimBenchmarkSuffix(llvm::StringRef BenchmarkName) {
    size_t Slash = BenchmarkName.find('/');
    return Slash == llvm::StringRef::npos ? BenchmarkName
                                          : BenchmarkName.take_front(Slash);
}

inline std::optional<std::string> extractBenchmarkTag(
    llvm::StringRef BenchmarkName,
    llvm::StringRef Prefix) {
    llvm::StringRef Trimmed = trimBenchmarkSuffix(BenchmarkName);
    size_t Start = Trimmed.find(Prefix);
    if (Start == llvm::StringRef::npos)
        return std::nullopt;
    Start += Prefix.size();
    size_t End = Trimmed.find(';', Start);
    if (End == llvm::StringRef::npos || End <= Start)
        return std::nullopt;
    return Trimmed.slice(Start, End).str();
}

inline bool isUCBenchmarkGroup(llvm::StringRef BenchmarkName) {
    auto Group = extractBenchmarkTag(BenchmarkName, "BM_g:");
    return Group && llvm::StringRef(*Group).starts_with("uc");
}

inline const std::unordered_set<std::string>& expectedUCKernels() {
    static const std::unordered_set<std::string> Kernels = {
        "apply_row_delta",
        "batch_delta",
        "box_filter",
        "column_scan",
        "count_matching_rows",
        "edge_detection",
        "email_match",
        "generic_sort",
        "grouped_count",
        "grouped_minmax",
        "grouped_sum",
        "multi_agg_delta",
        "multi_key_sort",
        "multi_pattern_match",
        "multi_predicate",
        "separable_gaussian",
        "struct_sort",
        "url_match",
    };
    return Kernels;
}

struct UCPipelineConfigData {
    llvm::StringMap<ClangRuntimeSpecializer::Options> ByKernel;
};

inline int64_t requireInt(const llvm::json::Object& Obj, llvm::StringRef Key) {
    if (std::optional<int64_t> V = Obj.getInteger(Key))
        return *V;
    throw std::runtime_error("UC pipeline JSON: missing integer field '" +
                             Key.str() + "'");
}

inline double requireNumber(const llvm::json::Object& Obj, llvm::StringRef Key) {
    if (std::optional<double> V = Obj.getNumber(Key))
        return *V;
    throw std::runtime_error("UC pipeline JSON: missing numeric field '" +
                             Key.str() + "'");
}

inline bool requireBool(const llvm::json::Object& Obj, llvm::StringRef Key) {
    if (std::optional<bool> V = Obj.getBoolean(Key))
        return *V;
    throw std::runtime_error("UC pipeline JSON: missing boolean field '" +
                             Key.str() + "'");
}

inline const UCPipelineConfigData* getUCPipelineConfigIfEnabled() {
    static std::once_flag Once;
    static std::unique_ptr<UCPipelineConfigData> Data;
    static std::string InitError;
    static bool Enabled = false;

    std::call_once(Once, [] {
        const char* Path = std::getenv("CRS_UC_PIPELINE_CONFIG_JSON");
        if (!Path || Path[0] == '\0')
            return;
        Enabled = true;
        auto BufferOrErr = llvm::MemoryBuffer::getFile(Path);
        if (!BufferOrErr) {
            InitError = "failed to read " + std::string(Path);
            return;
        }
        llvm::Expected<llvm::json::Value> Parsed =
            llvm::json::parse(BufferOrErr.get()->getBuffer());
        if (!Parsed) {
            InitError = "failed to parse JSON in " + std::string(Path);
            llvm::consumeError(Parsed.takeError());
            return;
        }
        auto* RootObj = Parsed->getAsObject();
        if (!RootObj) {
            InitError = "top-level JSON must be an object";
            return;
        }
        auto* Entries = RootObj->getArray("entries");
        if (!Entries) {
            InitError = "top-level JSON is missing the 'entries' array";
            return;
        }

        auto ParsedData = std::make_unique<UCPipelineConfigData>();
        for (const llvm::json::Value& EntryValue : *Entries) {
            const auto* EntryObj = EntryValue.getAsObject();
            if (!EntryObj) {
                InitError = "each entry must be a JSON object";
                return;
            }
            auto Kernel = EntryObj->getString("kernel");
            if (!Kernel) {
                InitError = "entry is missing the 'kernel' field";
                return;
            }
            if (!expectedUCKernels().count(Kernel->str())) {
                InitError = "unexpected UC kernel in JSON: " + Kernel->str();
                return;
            }
            const auto* OptionsObj = EntryObj->getObject("options");
            if (!OptionsObj) {
                InitError = "entry '" + Kernel->str() +
                            "' is missing the 'options' object";
                return;
            }
            if (ParsedData->ByKernel.count(*Kernel)) {
                InitError = "duplicate UC kernel in JSON: " + Kernel->str();
                return;
            }

            ClangRuntimeSpecializer::Options Opts =
                ClangRuntimeSpecializer::Options::Default();
            Opts.withOptimizationPipeline(
                static_cast<int>(requireInt(*OptionsObj, "optimization_pipeline")));
            Opts.withMaxFixpointIterations(
                static_cast<int>(requireInt(*OptionsObj, "max_fixpoint_iterations")));
            Opts.withLoopUnrollCount(
                static_cast<int>(requireInt(*OptionsObj, "loop_unroll_count")));
            const size_t LargeModuleThreshold = static_cast<size_t>(
                requireInt(*OptionsObj, "large_module_instr_threshold"));
            switch (Opts.OptimizationPipelineToUse) {
            case 0:
                Opts.withP0LargeModuleThreshold(LargeModuleThreshold);
                break;
            case 1:
                Opts.withP1LargeModuleThreshold(LargeModuleThreshold);
                break;
            case 2:
                Opts.withP2LargeModuleThreshold(LargeModuleThreshold);
                break;
            default:
                InitError = "invalid pipeline id for kernel: " + Kernel->str();
                return;
            }
            Opts.withEarlyPrune(requireBool(*OptionsObj, "enable_early_prune"));
            Opts.withO3Final(requireBool(*OptionsObj, "enable_o3_final"));

            if (Opts.OptimizationPipelineToUse == 1) {
                Opts.withP1InlineThreshold(static_cast<int>(
                    requireInt(*OptionsObj, "p1_inline_threshold")));
                Opts.withP1MaxModuleGrowth(
                    requireNumber(*OptionsObj, "p1_max_module_growth"));
            }
            if (Opts.OptimizationPipelineToUse == 2) {
                Opts.withP2MinFuncSize(static_cast<unsigned>(
                    requireInt(*OptionsObj, "p2_min_func_size")));
                Opts.withP2MaxClones(static_cast<unsigned>(
                    requireInt(*OptionsObj, "p2_max_clones")));
                Opts.withP2FuncSpecIters(static_cast<unsigned>(
                    requireInt(*OptionsObj, "p2_func_spec_iters")));
                Opts.withP2ForceSpec(requireBool(*OptionsObj, "p2_force_spec"));
                Opts.withP2SpecOnAddr(requireBool(*OptionsObj, "p2_spec_on_addr"));
                Opts.withP2SpecLiteral(requireBool(*OptionsObj, "p2_spec_literal"));
            }
            ParsedData->ByKernel[Kernel->str()] = Opts;
        }

        for (const std::string& Kernel : expectedUCKernels()) {
            if (!ParsedData->ByKernel.count(Kernel)) {
                InitError = "UC pipeline JSON is missing kernel '" + Kernel + "'";
                return;
            }
        }
        Data = std::move(ParsedData);
    });

    if (!Enabled)
        return nullptr;
    if (!InitError.empty())
        throw std::runtime_error("CRS_UC_PIPELINE_CONFIG_JSON: " + InitError);
    return Data.get();
}

inline ClangRuntimeSpecializer::Options resolveUCPipelineOptions(
    llvm::StringRef BenchmarkName,
    const ClangRuntimeSpecializer::Options& Fallback) {
    const UCPipelineConfigData* Data = getUCPipelineConfigIfEnabled();
    if (!Data || !isUCBenchmarkGroup(BenchmarkName))
        return Fallback;
    auto Kernel = extractBenchmarkTag(BenchmarkName, ";n:");
    if (!Kernel) {
        throw std::runtime_error("UC benchmark name is missing ';n:' tag: " +
                                 BenchmarkName.str());
    }
    auto It = Data->ByKernel.find(*Kernel);
    if (It == Data->ByKernel.end()) {
        throw std::runtime_error("UC pipeline JSON has no entry for kernel '" +
                                 *Kernel + "'");
    }
    return It->second;
}

inline void applyUCPipelineOptionsForBenchmark(llvm::StringRef BenchmarkName) {
    const UCPipelineConfigData* Data = getUCPipelineConfigIfEnabled();
    if (!Data || !isUCBenchmarkGroup(BenchmarkName))
        return;
    auto* RS = ClangRuntimeSpecializer::init();
    RS->setOptions(resolveUCPipelineOptions(
        BenchmarkName, ClangRuntimeSpecializer::Options::Default()));
}

inline void resetUCPipelineOptionsAfterBenchmark() {
    if (getUCPipelineConfigIfEnabled())
        ClangRuntimeSpecializer::init()->setOptions(
            ClangRuntimeSpecializer::Options::Default());
}

} // namespace detail

template <class R, class Fn, class Tuple, size_t... I>
__attribute__((always_inline))
auto specializeOnlyFromTupleImpl(
    ClangRuntimeSpecializer* RS,
    Fn F,
    const ClangRuntimeSpecializer::Options& opts,
    Tuple& args,
    std::index_sequence<I...>) {
    return RS->template specializeOnly<R>(F, opts, std::get<I>(args)...);
}

template <class R, class Fn, class Tuple>
__attribute__((always_inline))
auto specializeOnlyFromTuple(
    ClangRuntimeSpecializer* RS,
    Fn F,
    const ClangRuntimeSpecializer::Options& opts,
    Tuple& args) {
    using TupleT = std::remove_reference_t<Tuple>;
    return specializeOnlyFromTupleImpl<R>(
        RS, F, opts, args, std::make_index_sequence<std::tuple_size_v<TupleT>>{});
}

template <class R, auto F, class Tuple, size_t... I>
__attribute__((always_inline))
auto specializeOnlyFromTupleNTTPImpl(
    ClangRuntimeSpecializer* RS,
    const ClangRuntimeSpecializer::Options& opts,
    Tuple& args,
    std::index_sequence<I...>) {
    return RS->template specializeOnly<R>(F, opts, std::get<I>(args)...);
}

template <class R, auto F, class Tuple>
__attribute__((always_inline))
auto specializeOnlyFromTupleNTTP(
    ClangRuntimeSpecializer* RS,
    const ClangRuntimeSpecializer::Options& opts,
    Tuple& args) {
    using TupleT = std::remove_reference_t<Tuple>;
    return specializeOnlyFromTupleNTTPImpl<R, F>(
        RS, opts, args, std::make_index_sequence<std::tuple_size_v<TupleT>>{});
}

// Phase 1: Measure unspecialized execution only.
template <class Fn, class Tuple>
__attribute__((always_inline))
void benchmarkUnspecialized(
    benchmark::State& state,
    Fn F,
    Tuple args)
{
    auto InvokeNormal = [&](auto&&... a) {
        return std::invoke(F, std::forward<decltype(a)>(a)...);
    };
    using R = decltype(std::apply(InvokeNormal, args));

    for (auto _ : state) {
        benchmark::DoNotOptimize(args);
        if constexpr (std::is_void_v<R>)
            std::apply(InvokeNormal, args);
        else
            benchmark::DoNotOptimize(std::apply(InvokeNormal, args));
    }
}

// Phase 2: Measure JIT compilation overhead only.
// Register with: ->Iterations(1)->UseManualTime()
// Measures only JIT compilation time; dylib teardown happens after the timed region.
template <class Fn, class Tuple>
__attribute__((always_inline))
void benchmarkJITOverhead(
    benchmark::State& state,
    Fn F,
    Tuple normalArgs,
    Tuple specArgs,
    ClangRuntimeSpecializer::Options opts = ClangRuntimeSpecializer::Options::Default())
{
    auto* RS = ClangRuntimeSpecializer::init();
    opts = detail::resolveUCPipelineOptions(state.name(), opts);

    auto InvokeNormal = [&](auto&&... a) {
        return std::invoke(F, std::forward<decltype(a)>(a)...);
    };
    using R = decltype(std::apply(InvokeNormal, normalArgs));

    auto modStats = ClangRuntimeSpecializer::getModuleStats();
    auto PrevLevel = ClangRuntimeSpecializer::getLogLevel();
    ClangRuntimeSpecializer::setLogLevel(ClangRuntimeSpecializer::LogLevel::None);
    for (auto _ : state) {
        auto t0 = std::chrono::high_resolution_clock::now();
        auto SpecFn = specializeOnlyFromTuple<R>(RS, F, opts, specArgs);
        auto t1 = std::chrono::high_resolution_clock::now();
        benchmark::DoNotOptimize(SpecFn);
        // Report only the compile time; SpecFn destructs (munmap) after SetIterationTime.
        state.SetIterationTime(std::chrono::duration<double>(t1 - t0).count());
    }
    ClangRuntimeSpecializer::setLogLevel(PrevLevel);
    auto txStats = ClangRuntimeSpecializer::getLastTransformStats();

    state.counters["jit_module_fns"]    = (double)modStats.FunctionCount;
    state.counters["jit_module_instrs"] = (double)modStats.InstructionCount;
    state.counters["jit_blob_kb"]       = (double)(modStats.BitcodeSizeBytes / 1024);
    state.counters["jit_pruned_fns"]    = (double)txStats.FunctionCountAfterPrune;
    state.counters["jit_pruned_instrs"] = (double)txStats.InstructionCountAfterPrune;
}

// NTTP variant: one instantiation per function symbol. This keeps the function
// target constant through helper wrappers and avoids signature-only sharing.
template <auto F, class Tuple>
__attribute__((always_inline))
void benchmarkJITOverhead(
    benchmark::State& state,
    Tuple normalArgs,
    Tuple specArgs,
    ClangRuntimeSpecializer::Options opts = ClangRuntimeSpecializer::Options::Default())
{
    auto* RS = ClangRuntimeSpecializer::init();
    opts = detail::resolveUCPipelineOptions(state.name(), opts);

    auto InvokeNormal = [&](auto&&... a) {
        return std::invoke(F, std::forward<decltype(a)>(a)...);
    };
    using R = decltype(std::apply(InvokeNormal, normalArgs));

    auto modStats = ClangRuntimeSpecializer::getModuleStats();
    auto PrevLevel = ClangRuntimeSpecializer::getLogLevel();
    ClangRuntimeSpecializer::setLogLevel(ClangRuntimeSpecializer::LogLevel::None);
    for (auto _ : state) {
        auto t0 = std::chrono::high_resolution_clock::now();
        auto SpecFn = specializeOnlyFromTupleNTTP<R, F>(RS, opts, specArgs);
        auto t1 = std::chrono::high_resolution_clock::now();
        benchmark::DoNotOptimize(SpecFn);
        state.SetIterationTime(std::chrono::duration<double>(t1 - t0).count());
    }
    ClangRuntimeSpecializer::setLogLevel(PrevLevel);
    auto txStats = ClangRuntimeSpecializer::getLastTransformStats();

    state.counters["jit_module_fns"]    = (double)modStats.FunctionCount;
    state.counters["jit_module_instrs"] = (double)modStats.InstructionCount;
    state.counters["jit_blob_kb"]       = (double)(modStats.BitcodeSizeBytes / 1024);
    state.counters["jit_pruned_fns"]    = (double)txStats.FunctionCountAfterPrune;
    state.counters["jit_pruned_instrs"] = (double)txStats.InstructionCountAfterPrune;
}

// Phase 3: Measure specialized execution only (setup: compile once before loop).
template <class Fn, class Tuple>
__attribute__((always_inline))
void benchmarkSpecializedExec(
    benchmark::State& state,
    Fn F,
    Tuple specArgs,
    ClangRuntimeSpecializer::Options opts = ClangRuntimeSpecializer::Options::Default())
{
    auto* RS = ClangRuntimeSpecializer::init();
    opts = detail::resolveUCPipelineOptions(state.name(), opts);

    auto InvokeNormal = [&](auto&&... a) {
        return std::invoke(F, std::forward<decltype(a)>(a)...);
    };
    using R = decltype(std::apply(InvokeNormal, specArgs));

    // One-time setup: compile the specialized function before the timed loop.
    auto PrevLevel = ClangRuntimeSpecializer::getLogLevel();
    ClangRuntimeSpecializer::setLogLevel(ClangRuntimeSpecializer::LogLevel::None);
    auto SpecFnPtr = specializeOnlyFromTuple<R>(RS, F, opts, specArgs);
    ClangRuntimeSpecializer::setLogLevel(PrevLevel);

    for (auto _ : state) {
        if constexpr (std::is_void_v<R>)
            SpecFnPtr();
        else
            benchmark::DoNotOptimize(SpecFnPtr());
    }
}

template <auto F, class Tuple>
__attribute__((always_inline))
void benchmarkSpecializedExec(
    benchmark::State& state,
    Tuple specArgs,
    ClangRuntimeSpecializer::Options opts = ClangRuntimeSpecializer::Options::Default())
{
    auto* RS = ClangRuntimeSpecializer::init();
    opts = detail::resolveUCPipelineOptions(state.name(), opts);

    auto InvokeNormal = [&](auto&&... a) {
        return std::invoke(F, std::forward<decltype(a)>(a)...);
    };
    using R = decltype(std::apply(InvokeNormal, specArgs));

    auto PrevLevel = ClangRuntimeSpecializer::getLogLevel();
    ClangRuntimeSpecializer::setLogLevel(ClangRuntimeSpecializer::LogLevel::None);
    auto SpecFnPtr = specializeOnlyFromTupleNTTP<R, F>(RS, opts, specArgs);
    ClangRuntimeSpecializer::setLogLevel(PrevLevel);

    for (auto _ : state) {
        if constexpr (std::is_void_v<R>)
            SpecFnPtr();
        else
            benchmark::DoNotOptimize(SpecFnPtr());
    }
}

// ---------------------------------------------------------------------------
// Pass-trace JSON utilities
// ---------------------------------------------------------------------------

// Serialize a pass trace to a JSON file named <benchmark_name>_pass_trace.json.
// Non-filename characters in benchmark_name are replaced with '_'.
inline void writePassTraceJSON(const std::string& BenchmarkName,
                               const std::vector<ClangRuntimeSpecializer::PassRecord>& Trace) {
    const char* TraceDir = std::getenv("CRS_PASS_TRACE_DIR");
    if (!TraceDir)
        return;

    std::string Filename = BenchmarkName + "_pass_trace.json";
    for (char& C : Filename)
        if (C != '.' && C != '-' && C != '_' &&
            !(C >= 'a' && C <= 'z') && !(C >= 'A' && C <= 'Z') && !(C >= '0' && C <= '9'))
            C = '_';

    std::ofstream Out(std::string(TraceDir) + "/" + Filename);
    if (!Out.is_open()) return;

    Out << "[\n";
    for (size_t I = 0; I < Trace.size(); ++I) {
        const auto& R = Trace[I];
        Out << "  {"
            << "\"name\":\"" << R.Name << "\""
            << ",\"group\":\"" << R.Group << "\""
            << ",\"fixpoint_iter\":" << R.FixpointIter
            << ",\"fns_before\":" << R.FnsBefore
            << ",\"fns_after\":" << R.FnsAfter
            << ",\"instrs_before\":" << R.InstrsBefore
            << ",\"instrs_after\":" << R.InstrsAfter
            << ",\"bbs_before\":" << R.BBsBefore
            << ",\"bbs_after\":" << R.BBsAfter
            << ",\"wall_time_ms\":" << R.WallTimeMs
            << ",\"ir_changed\":" << (R.IRChanged ? "true" : "false")
            << "}";
        if (I + 1 < Trace.size()) Out << ",";
        Out << "\n";
    }
    Out << "]\n";
}

// Single-invocation analysis benchmark: measures JIT time once, then writes a
// per-pass trace JSON alongside the benchmark output.
// Register with: ->Iterations(1)->UseManualTime()
template <class Fn, class Tuple>
__attribute__((always_inline))
void benchmarkJITAnalysis(
    benchmark::State& state,
    Fn F,
    Tuple specArgs,
    ClangRuntimeSpecializer::Options opts = ClangRuntimeSpecializer::Options::Default())
{
    auto* RS = ClangRuntimeSpecializer::init();
    opts = detail::resolveUCPipelineOptions(state.name(), opts);

    auto InvokeNormal = [&](auto&&... a) {
        return std::invoke(F, std::forward<decltype(a)>(a)...);
    };
    using R = decltype(std::apply(InvokeNormal, specArgs));

    // Chrome trace requires CRS_CHROME_TRACE_DIR; use it to build the output path.
    const char* ChromeDir = std::getenv("CRS_CHROME_TRACE_DIR");
    if (!ChromeDir) {
        state.SkipWithError(
            "CRS_CHROME_TRACE_DIR must be set before running benchmarkJITAnalysis. "
            "Use record_benchmark.py or export CRS_CHROME_TRACE_DIR=/path/to/dir.");
        return;
    }
    std::string ChromeTraceFilename = state.name() + "_chrome_trace.json";
    for (char& C : ChromeTraceFilename)
        if (C != '.' && C != '-' && C != '_' &&
            !(C >= 'a' && C <= 'z') && !(C >= 'A' && C <= 'Z') && !(C >= '0' && C <= '9'))
            C = '_';
    opts.TimeTraceOutputPath = std::string(ChromeDir) + "/" + ChromeTraceFilename;
    opts.EnablePassTrace = true;

    auto PrevLevel = ClangRuntimeSpecializer::getLogLevel();
    ClangRuntimeSpecializer::setLogLevel(ClangRuntimeSpecializer::LogLevel::None);
    for (auto _ : state) {
        auto T0 = std::chrono::steady_clock::now();
        benchmark::DoNotOptimize(specializeOnlyFromTuple<R>(RS, F, opts, specArgs));
        double Ms = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - T0).count();
        state.SetIterationTime(Ms / 1000.0);
    }
    ClangRuntimeSpecializer::setLogLevel(PrevLevel);

    writePassTraceJSON(state.name(), ClangRuntimeSpecializer::getLastPassTrace());
}

template <auto F, class Tuple>
__attribute__((always_inline))
void benchmarkJITAnalysis(
    benchmark::State& state,
    Tuple specArgs,
    ClangRuntimeSpecializer::Options opts = ClangRuntimeSpecializer::Options::Default())
{
    auto* RS = ClangRuntimeSpecializer::init();
    opts = detail::resolveUCPipelineOptions(state.name(), opts);

    auto InvokeNormal = [&](auto&&... a) {
        return std::invoke(F, std::forward<decltype(a)>(a)...);
    };
    using R = decltype(std::apply(InvokeNormal, specArgs));

    const char* ChromeDir = std::getenv("CRS_CHROME_TRACE_DIR");
    if (!ChromeDir) {
        state.SkipWithError(
            "CRS_CHROME_TRACE_DIR must be set before running benchmarkJITAnalysis. "
            "Use record_benchmark.py or export CRS_CHROME_TRACE_DIR=/path/to/dir.");
        return;
    }
    std::string ChromeTraceFilename = state.name() + "_chrome_trace.json";
    for (char& C : ChromeTraceFilename)
        if (C != '.' && C != '-' && C != '_' &&
            !(C >= 'a' && C <= 'z') && !(C >= 'A' && C <= 'Z') && !(C >= '0' && C <= '9'))
            C = '_';
    opts.TimeTraceOutputPath = std::string(ChromeDir) + "/" + ChromeTraceFilename;
    opts.EnablePassTrace = true;

    auto PrevLevel = ClangRuntimeSpecializer::getLogLevel();
    ClangRuntimeSpecializer::setLogLevel(ClangRuntimeSpecializer::LogLevel::None);
    for (auto _ : state) {
        auto T0 = std::chrono::steady_clock::now();
        benchmark::DoNotOptimize(specializeOnlyFromTupleNTTP<R, F>(RS, opts, specArgs));
        double Ms = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - T0).count();
        state.SetIterationTime(Ms / 1000.0);
    }
    ClangRuntimeSpecializer::setLogLevel(PrevLevel);

    writePassTraceJSON(state.name(), ClangRuntimeSpecializer::getLastPassTrace());
}

// Factory-based analysis benchmark for use cases that specialize lambdas via
// create_*_specialized() helpers. The factory must perform exactly one JIT
// specialization when invoked.
template <class Factory>
__attribute__((always_inline))
void benchmarkLambdaJITAnalysis(
    benchmark::State& state,
    Factory&& factory,
    ClangRuntimeSpecializer::Options opts = ClangRuntimeSpecializer::Options::Default())
{
    auto* RS = ClangRuntimeSpecializer::init();
    opts = detail::resolveUCPipelineOptions(state.name(), opts);

    const char* ChromeDir = std::getenv("CRS_CHROME_TRACE_DIR");
    if (!ChromeDir) {
        state.SkipWithError(
            "CRS_CHROME_TRACE_DIR must be set before running benchmarkJITAnalysis. "
            "Use record_benchmark.py or export CRS_CHROME_TRACE_DIR=/path/to/dir.");
        return;
    }
    std::string ChromeTraceFilename = state.name() + "_chrome_trace.json";
    for (char& C : ChromeTraceFilename)
        if (C != '.' && C != '-' && C != '_' &&
            !(C >= 'a' && C <= 'z') && !(C >= 'A' && C <= 'Z') && !(C >= '0' && C <= '9'))
            C = '_';
    opts.TimeTraceOutputPath = std::string(ChromeDir) + "/" + ChromeTraceFilename;
    opts.EnablePassTrace = true;

    auto PrevLevel = ClangRuntimeSpecializer::getLogLevel();
    ClangRuntimeSpecializer::setLogLevel(ClangRuntimeSpecializer::LogLevel::None);
    RS->setOptions(opts);
    auto&& FactoryRef = factory;
    for (auto _ : state) {
        auto T0 = std::chrono::steady_clock::now();
        benchmark::DoNotOptimize(std::invoke(FactoryRef));
        double Ms = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - T0).count();
        state.SetIterationTime(Ms / 1000.0);
    }
    RS->setOptions(ClangRuntimeSpecializer::Options::Default());
    ClangRuntimeSpecializer::setLogLevel(PrevLevel);

    writePassTraceJSON(state.name(), ClangRuntimeSpecializer::getLastPassTrace());
}

// ---------------------------------------------------------------------------
// Budget-aware sweep infrastructure
// ---------------------------------------------------------------------------

// Fallback measurement: median of 7 warm calls.
// Used by JIT/exec sweep lambdas if the unspecialized benchmark hasn't run yet.
template <class Fn, class Tuple>
inline int64_t measureMedianCallNs(Fn F, Tuple args) {
    constexpr int N = 7;
    double samples[N];
    for (int i = 0; i < N; ++i) {
        auto t0 = std::chrono::steady_clock::now();
        benchmark::DoNotOptimize(std::apply([&](auto&&... a) {
            return std::invoke(F, std::forward<decltype(a)>(a)...);
        }, args));
        samples[i] = std::chrono::duration<double, std::nano>(
            std::chrono::steady_clock::now() - t0).count();
    }
    std::nth_element(samples, samples + N/2, samples + N);
    return static_cast<int64_t>(samples[N/2]);
}

// Register three benchmark lambdas for a single function:
//   1. unspecialized baseline (also measures expectedNs)
//   2. JIT overhead sweep over budget scale factors
//   3. specialized exec sweep over budget scale factors
//
// Scale factors are expressed in basis points (100 = 1.0x, 316 ≈ 3.16x, 1000 = 10x).
// The ->Ranges({{10, 5000}}) produces a geometric sweep: 10, 31, 100, 316, 1000, 3162, 5000.
template <class Fn, class Tuple>
void registerBudgetBenchmarks(
    const std::string& group,
    const std::string& name,
    Fn F,
    Tuple normalArgs,
    Tuple specArgs)
{
    using CRS = ClangRuntimeSpecializer;
    auto sharedNs = std::make_shared<std::atomic<int64_t>>(0);

    // 1. Unspecialized baseline: timing is collected inside the state loop via UseManualTime().
    //    Each iteration time is recorded into sharedNs so sweep benchmarks can read it.
    benchmark::RegisterBenchmark(
        ("BM_g:" + group + ";n:" + name + ";t:unspecialized;").c_str(),
        [F, normalArgs, sharedNs](benchmark::State& state) mutable {
            for (auto _ : state) {
                auto t0 = std::chrono::steady_clock::now();
                benchmark::DoNotOptimize(std::apply([&](auto&&... a) {
                    return std::invoke(F, std::forward<decltype(a)>(a)...);
                }, normalArgs));
                double elapsedNs = std::chrono::duration<double, std::nano>(
                    std::chrono::steady_clock::now() - t0).count();
                state.SetIterationTime(elapsedNs * 1e-9);
                sharedNs->store(static_cast<int64_t>(elapsedNs));
            }
            state.counters["expected_call_ns"] = static_cast<double>(sharedNs->load());
        })->UseManualTime();

    // 2. JIT overhead budget sweep (state.range(0) = scale in basis points)
    benchmark::RegisterBenchmark(
        ("BM_g:" + group + ";n:" + name + ";t:jit_budget_sweep;").c_str(),
        [F, normalArgs, specArgs, sharedNs](benchmark::State& state) mutable {
            int64_t ns = sharedNs->load();
            if (ns == 0) ns = measureMedianCallNs(F, normalArgs);
            double scale = state.range(0) / 100.0;
            auto opts = CRS::Options::FromExpectedRuntime(static_cast<double>(ns), scale);
            benchmarkJITOverhead(state, F, normalArgs, specArgs, opts);
            state.counters["expected_call_ns"] = static_cast<double>(ns);
            state.counters["budget_scale"]     = scale;
            state.counters["budget_fixpoint"]  = static_cast<double>(opts.MaxFixpointIterations);
            state.counters["budget_unroll"]    = static_cast<double>(opts.LoopUnrollCount);
        })->Ranges({{10, 5000}});

    // 3. Specialized exec budget sweep
    benchmark::RegisterBenchmark(
        ("BM_g:" + group + ";n:" + name + ";t:exec_budget_sweep;").c_str(),
        [F, specArgs, sharedNs](benchmark::State& state) mutable {
            int64_t ns = sharedNs->load();
            if (ns == 0) ns = measureMedianCallNs(F, specArgs);
            double scale = state.range(0) / 100.0;
            auto opts = CRS::Options::FromExpectedRuntime(static_cast<double>(ns), scale);
            benchmarkSpecializedExec(state, F, specArgs, opts);
            state.counters["expected_call_ns"] = static_cast<double>(ns);
            state.counters["budget_scale"]     = scale;
            state.counters["budget_fixpoint"]  = static_cast<double>(opts.MaxFixpointIterations);
            state.counters["budget_unroll"]    = static_cast<double>(opts.LoopUnrollCount);
        })->Ranges({{10, 5000}});
}

} // namespace clangRuntimeSpecializer
