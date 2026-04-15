#pragma once
#include "ClangRuntimeSpecializer.h"
#include <benchmark/benchmark.h>
#include <cstdio>

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
template <const char* funcName, class Fn, class Tuple>
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
template <const char* funcName, class Fn, class Tuple>
__attribute__((always_inline))
void benchmarkJITOverhead(
    benchmark::State& state,
    Fn F,
    Tuple normalArgs,
    Tuple specArgs)
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
            return RS->template specializeOnly<funcName, R>(std::forward<decltype(A)>(A)...);
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
template <const char* funcName, class Fn, class Tuple>
__attribute__((always_inline))
void benchmarkSpecializedExec(
    benchmark::State& state,
    Fn F,
    Tuple specArgs)
{
    auto* RS = ClangRuntimeSpecializer::init();

    auto InvokeNormal = [&](auto&&... a) {
        return std::invoke(F, std::forward<decltype(a)>(a)...);
    };
    using R = decltype(std::apply(InvokeNormal, specArgs));

    // One-time setup: compile the specialized function before the timed loop.
    auto PrevLevel = ClangRuntimeSpecializer::getLogLevel();
    ClangRuntimeSpecializer::setLogLevel(ClangRuntimeSpecializer::LogLevel::None);
    uintptr_t Addr = std::apply([&](auto&&... A) {
        return RS->template specializeOnly<funcName, R>(std::forward<decltype(A)>(A)...);
    }, specArgs);
    ClangRuntimeSpecializer::setLogLevel(PrevLevel);
    auto SpecFnPtr = reinterpret_cast<R(*)()>(Addr);

    for (auto _ : state) {
        if constexpr (std::is_void_v<R>)
            SpecFnPtr();
        else
            benchmark::DoNotOptimize(SpecFnPtr());
    }
}

// --- Method variants ---

// Phase 1: Measure unspecialized method execution only.
template <const char* funcName, class MemFn, class Tuple>
__attribute__((always_inline))
void benchmarkUnspecializedMethod(
    benchmark::State& state,
    MemFn Mf,
    Tuple args)
{
    auto InvokeNormal = [&](auto&&... a) {
        return std::invoke(Mf, std::forward<decltype(a)>(a)...);
    };
    using R = decltype(std::apply(InvokeNormal, args));

    for (auto _ : state) {
        if constexpr (std::is_void_v<R>)
            std::apply(InvokeNormal, args);
        else
            benchmark::DoNotOptimize(std::apply(InvokeNormal, args));
    }
}

// Phase 2: Measure JIT compilation overhead for a method.
template <const char* funcName, class MemFn, class Tuple>
__attribute__((always_inline))
void benchmarkJITOverheadMethod(
    benchmark::State& state,
    MemFn Mf,
    Tuple normalArgs,
    Tuple specArgs)
{
    auto* RS = ClangRuntimeSpecializer::init();

    auto InvokeNormal = [&](auto&&... a) {
        return std::invoke(Mf, std::forward<decltype(a)>(a)...);
    };
    using R = decltype(std::apply(InvokeNormal, normalArgs));

    auto modStats = ClangRuntimeSpecializer::getModuleStats();
    auto PrevLevel = ClangRuntimeSpecializer::getLogLevel();
    ClangRuntimeSpecializer::setLogLevel(ClangRuntimeSpecializer::LogLevel::None);
    for (auto _ : state) {
        benchmark::DoNotOptimize(std::apply([&](auto&&... A) {
            return RS->template specializeOnly<funcName, R>(std::forward<decltype(A)>(A)...);
        }, specArgs));
    }
    auto txStats = ClangRuntimeSpecializer::getLastTransformStats();
    // Correctness check outside timing (log level stays None; errors throw exceptions)
    if constexpr (std::is_void_v<R>) {
        std::apply(InvokeNormal, normalArgs);
        std::apply([&](auto&&... A) {
            RS->template callSpecialized<funcName, void>(std::forward<decltype(A)>(A)...);
        }, specArgs);
    } else {
        R ResOrig = std::apply(InvokeNormal, normalArgs);
        R ResSpec = std::apply([&](auto&&... A) -> R {
            return RS->template callSpecialized<funcName, R>(std::forward<decltype(A)>(A)...);
        }, specArgs);
        if constexpr (HasEqualityOperator<R>::value) {
            if (ResOrig != ResSpec)
                throw ClangRuntimeSpecializerChangesBehaviorError(
                    (llvm::Twine("Comparison failed: return values differ for ") + funcName).str());
        }
    }
    ClangRuntimeSpecializer::setLogLevel(PrevLevel);

    state.counters["jit_module_fns"]    = (double)modStats.FunctionCount;
    state.counters["jit_module_instrs"] = (double)modStats.InstructionCount;
    state.counters["jit_blob_kb"]       = (double)(modStats.BitcodeSizeBytes / 1024);
    state.counters["jit_pruned_fns"]    = (double)txStats.FunctionCountAfterPrune;
    state.counters["jit_pruned_instrs"] = (double)txStats.InstructionCountAfterPrune;
}

// Phase 3: Measure specialized method execution only (setup: compile once before loop).
template <const char* funcName, class MemFn, class Tuple>
__attribute__((always_inline))
void benchmarkSpecializedExecMethod(
    benchmark::State& state,
    MemFn Mf,
    Tuple specArgs)
{
    auto* RS = ClangRuntimeSpecializer::init();

    auto InvokeNormal = [&](auto&&... a) {
        return std::invoke(Mf, std::forward<decltype(a)>(a)...);
    };
    using R = decltype(std::apply(InvokeNormal, specArgs));

    // One-time setup: compile the specialized function before the timed loop.
    auto PrevLevel = ClangRuntimeSpecializer::getLogLevel();
    ClangRuntimeSpecializer::setLogLevel(ClangRuntimeSpecializer::LogLevel::None);
    uintptr_t Addr = std::apply([&](auto&&... A) {
        return RS->template specializeOnly<funcName, R>(std::forward<decltype(A)>(A)...);
    }, specArgs);
    ClangRuntimeSpecializer::setLogLevel(PrevLevel);
    auto SpecFnPtr = reinterpret_cast<R(*)()>(Addr);

    for (auto _ : state) {
        if constexpr (std::is_void_v<R>)
            SpecFnPtr();
        else
            benchmark::DoNotOptimize(SpecFnPtr());
    }
}

} // namespace clangRuntimeSpecializer
