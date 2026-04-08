
#include <benchmark/benchmark.h>
#include "ClangRuntimeSpecializerBenchmark.h"
#include <cmath>

// ── Scenario 1: mypow (loop with known iteration count) ──────────────────────

// A simple function to specialize.
// It performs some computation that is easier to optimize if x is known.
extern "C" int mypow_bench(int x) {
    int result = 1;
    for (int i = 0; i < x; i++) {
        result *= 3;
    }
    return result;
}

inline constexpr char Fn_mypow[] = "mypow_bench";

// ── Scenario 2: config_count (branch elimination via pointer-to-config) ───────

struct Config {
    int threshold;
    bool direction;  // true = (val >= threshold), false = (val < threshold)
};

// Count integers in [0, n) matching the threshold condition.
// Specializing for a fixed cfg pointer eliminates the conditional branch and
// exposes n as a compile-time constant for loop unrolling/vectorization.
extern "C" int config_count(Config* cfg, int n) __asm__("config_count");
int config_count(Config* cfg, int n) {
    int count = 0;
    for (int i = 0; i < n; i++)
        count += ((i >= cfg->threshold) == cfg->direction);
    return count;
}

inline constexpr char Fn_config_count[] = "config_count";
static Config g_config{512, true};

// ── Scenario 3: A::add (method field constant-folding) ───────────────────────

// Specializing for a fixed this pointer makes the 'value' field a compile-time
// constant, folding the entire addition to a constant return value.
class A {
    int value;
public:
    explicit A(int val) : value(val) {}
    __attribute__((noinline)) int add(int a, int b) const __asm__("A::add") {
        return value + a + b;
    }
};

inline constexpr char Fn_A_add[] = "A::add";
static A g_a_instance{42};

// ── IR embedding trigger ─────────────────────────────────────────────────────

// We need to ensure callSpecialized is actually called in the IR so the
// compile-time pass embeds IR for all three functions.
volatile bool g_dummy_trigger = false;

extern "C" __attribute__((used)) void dummy_registration() {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    if (g_dummy_trigger) {
        RS->callSpecialized<Fn_mypow, int>(0);
        RS->callSpecialized<Fn_config_count, int>((Config*)nullptr, 0);
        RS->callSpecialized<Fn_A_add, int>((const A*)nullptr, 0, 0);
    }
}

extern "C" void BM_mypow_unspecialized(benchmark::State& state) {
    int x = state.range(0);
    clangRuntimeSpecializer::benchmarkUnspecialized<Fn_mypow>(
        state,
        mypow_bench,
        std::make_tuple(x));
}
BENCHMARK(BM_mypow_unspecialized)->Range(1, 2<<6);

extern "C" void BM_mypow_jit_overhead(benchmark::State& state) {
    int x = state.range(0);
    clangRuntimeSpecializer::benchmarkJITOverhead<Fn_mypow>(
        state,
        mypow_bench,
    std::make_tuple(x),
        std::make_tuple(x));
}
BENCHMARK(BM_mypow_jit_overhead)->Range(1, 2<<6);

extern "C" void BM_mypow_specialized_exec(benchmark::State& state) {
    int x = state.range(0);
    clangRuntimeSpecializer::benchmarkSpecializedExec<Fn_mypow>(
        state,
        mypow_bench,
        std::make_tuple(x));
}
BENCHMARK(BM_mypow_specialized_exec)->Range(1, 2<<6);

// ── config_count benchmarks ───────────────────────────────────────────────────

void BM_config_count_unspecialized(benchmark::State& state) {
    int n = state.range(0);
    clangRuntimeSpecializer::benchmarkUnspecialized<Fn_config_count>(
        state, config_count, std::make_tuple(&g_config, n));
}
BENCHMARK(BM_config_count_unspecialized)->Range(64, 2<<10);

void BM_config_count_jit_overhead(benchmark::State& state) {
    int n = state.range(0);
    clangRuntimeSpecializer::benchmarkJITOverhead<Fn_config_count>(
        state, config_count,
        std::make_tuple(&g_config, n),
        std::make_tuple(&g_config, n));
}
BENCHMARK(BM_config_count_jit_overhead)->Range(64, 2<<10)->MinWarmUpTime(1.0);

void BM_config_count_specialized_exec(benchmark::State& state) {
    int n = state.range(0);
    clangRuntimeSpecializer::benchmarkSpecializedExec<Fn_config_count>(
        state, config_count, std::make_tuple(&g_config, n));
}
BENCHMARK(BM_config_count_specialized_exec)->Range(64, 2<<10);

// ── A::add benchmarks (method with known this pointer) ───────────────────────

void BM_method_add_unspecialized(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_a_instance);
        benchmark::DoNotOptimize(g_a_instance.add(7, 11));
    }
}
BENCHMARK(BM_method_add_unspecialized);

void BM_method_add_jit_overhead(benchmark::State& state) {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    auto Prev = clangRuntimeSpecializer::ClangRuntimeSpecializer::getLogLevel();
    clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(
        clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::None);
    for (auto _ : state)
        benchmark::DoNotOptimize(RS->specializeOnly<Fn_A_add, int>(&g_a_instance, 7, 11));
    clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(Prev);
}
BENCHMARK(BM_method_add_jit_overhead);

void BM_method_add_specialized_exec(benchmark::State& state) {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    auto Prev = clangRuntimeSpecializer::ClangRuntimeSpecializer::getLogLevel();
    clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(
        clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::None);
    uintptr_t Addr = RS->specializeOnly<Fn_A_add, int>(&g_a_instance, 7, 11);
    clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(Prev);
    auto SpecFn = reinterpret_cast<int(*)()>(Addr);
    for (auto _ : state)
        benchmark::DoNotOptimize(SpecFn());
}
BENCHMARK(BM_method_add_specialized_exec);

#ifndef ALL_BENCHMARKS_BUILD
BENCHMARK_MAIN();
#endif
