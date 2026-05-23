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
#include <stdexcept>
#include <string>

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

    auto InvokeNormal = [&](auto&&... a) {
        return std::invoke(F, std::forward<decltype(a)>(a)...);
    };
    using R = decltype(std::apply(InvokeNormal, normalArgs));

    auto modStats = ClangRuntimeSpecializer::getModuleStats();
    auto PrevLevel = ClangRuntimeSpecializer::getLogLevel();
    ClangRuntimeSpecializer::setLogLevel(ClangRuntimeSpecializer::LogLevel::None);
    for (auto _ : state) {
        benchmark::DoNotOptimize(std::apply([&](auto&&... A) {
            return RS->template specializeOnly<R>(F, opts, std::forward<decltype(A)>(A)...);
        }, specArgs));
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

    auto InvokeNormal = [&](auto&&... a) {
        return std::invoke(F, std::forward<decltype(a)>(a)...);
    };
    using R = decltype(std::apply(InvokeNormal, specArgs));

    // One-time setup: compile the specialized function before the timed loop.
    auto PrevLevel = ClangRuntimeSpecializer::getLogLevel();
    ClangRuntimeSpecializer::setLogLevel(ClangRuntimeSpecializer::LogLevel::None);
    auto SpecFnPtr = std::apply([&](auto&&... A) {
        return RS->template specializeOnly<R>(F, opts, std::forward<decltype(A)>(A)...);
    }, specArgs);
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
        throw std::runtime_error(
            "CRS_PASS_TRACE_DIR must be set before running benchmarkJITAnalysis. "
            "Use record_benchmark.py or export CRS_PASS_TRACE_DIR=/path/to/dir.");

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

    auto InvokeNormal = [&](auto&&... a) {
        return std::invoke(F, std::forward<decltype(a)>(a)...);
    };
    using R = decltype(std::apply(InvokeNormal, specArgs));

    // Chrome trace requires CRS_CHROME_TRACE_DIR; use it to build the output path.
    const char* ChromeDir = std::getenv("CRS_CHROME_TRACE_DIR");
    if (!ChromeDir)
        throw std::runtime_error(
            "CRS_CHROME_TRACE_DIR must be set before running benchmarkJITAnalysis. "
            "Use record_benchmark.py or export CRS_CHROME_TRACE_DIR=/path/to/dir.");
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
        benchmark::DoNotOptimize(std::apply([&](auto&&... A) {
            return RS->template specializeOnly<R>(F, opts, std::forward<decltype(A)>(A)...);
        }, specArgs));
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

    const char* ChromeDir = std::getenv("CRS_CHROME_TRACE_DIR");
    if (!ChromeDir)
        throw std::runtime_error(
            "CRS_CHROME_TRACE_DIR must be set before running benchmarkJITAnalysis. "
            "Use record_benchmark.py or export CRS_CHROME_TRACE_DIR=/path/to/dir.");
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
