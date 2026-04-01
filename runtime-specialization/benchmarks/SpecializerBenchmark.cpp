
#include <benchmark/benchmark.h>
#include "ClangRuntimeSpecializerBenchmark.h"
#include <cmath>

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

// We need to ensure callSpecialized is actually called in the IR so the
// compile-time pass embeds IR for mypow_bench.
volatile bool g_dummy_trigger = false;

extern "C" __attribute__((used)) void dummy_registration() {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    if (g_dummy_trigger) {
        RS->callSpecialized<Fn_mypow, int>(0);
    }
}

extern "C" void BM_mypow_unspecialized(benchmark::State& state) {
    int x = state.range(0);
    clangRuntimeSpecializer::benchmarkUnspecialized<Fn_mypow>(
        state,
        mypow_bench,
        std::make_tuple(x));
}
BENCHMARK(BM_mypow_unspecialized)->Arg(10);

extern "C" void BM_mypow_jit_overhead(benchmark::State& state) {
    int x = state.range(0);
    clangRuntimeSpecializer::benchmarkJITOverhead<Fn_mypow>(
        state,
        mypow_bench,
    std::make_tuple(x),
        std::make_tuple(x));
}
BENCHMARK(BM_mypow_jit_overhead)->Arg(10)->Repetitions(5)->MinTime(2.0);

extern "C" void BM_mypow_specialized_exec(benchmark::State& state) {
    int x = state.range(0);
    clangRuntimeSpecializer::benchmarkSpecializedExec<Fn_mypow>(
        state,
        mypow_bench,
        std::make_tuple(x));
}
BENCHMARK(BM_mypow_specialized_exec)->Arg(10);

BENCHMARK_MAIN();
