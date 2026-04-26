
#include <benchmark/benchmark.h>
#include "ClangRuntimeSpecializerBenchmark.h"
#include "SyntheticKernels.h"

extern "C" void BM_unspecialized____mypow(benchmark::State& state) {
    int x = state.range(0);
    clangRuntimeSpecializer::benchmarkUnspecialized<Fn_mypow>(
        state,
        mypow_bench,
        std::make_tuple(x));
}
BENCHMARK(BM_unspecialized____mypow)->Name("BM_g:synthetic;n:mypow;t:unspecialized;")->Range(1, 2<<6);

extern "C" void BM_jit_overhead_____mypow(benchmark::State& state) {
    int x = state.range(0);
    clangRuntimeSpecializer::benchmarkJITOverhead<Fn_mypow>(
        state,
        mypow_bench,
    std::make_tuple(x),
        std::make_tuple(x));
}
BENCHMARK(BM_jit_overhead_____mypow)->Name("BM_g:synthetic;n:mypow;t:jit_overhead;")->Range(1, 2<<6);

extern "C" void BM_specialized_exec_mypow(benchmark::State& state) {
    int x = state.range(0);
    clangRuntimeSpecializer::benchmarkSpecializedExec<Fn_mypow>(
        state,
        mypow_bench,
        std::make_tuple(x));
}
BENCHMARK(BM_specialized_exec_mypow)->Name("BM_g:synthetic;n:mypow;t:specialized_exec;")->Range(1, 2<<6);

// ── config_count benchmarks ───────────────────────────────────────────────────

void BM_unspecialized____config_count(benchmark::State& state) {
    int n = state.range(0);
    clangRuntimeSpecializer::benchmarkUnspecialized<Fn_config_count>(
        state, config_count, std::make_tuple(&g_config, n));
}
BENCHMARK(BM_unspecialized____config_count)->Name("BM_g:synthetic;n:config_count;t:unspecialized;")->Range(64, 2<<10);

void BM_jit_overhead_____config_count(benchmark::State& state) {
    int n = state.range(0);
    clangRuntimeSpecializer::benchmarkJITOverhead<Fn_config_count>(
        state, config_count,
        std::make_tuple(&g_config, n),
        std::make_tuple(&g_config, n));
}
BENCHMARK(BM_jit_overhead_____config_count)->Name("BM_g:synthetic;n:config_count;t:jit_overhead;")->Range(64, 2<<10)->MinWarmUpTime(1.0);

void BM_specialized_exec_config_count(benchmark::State& state) {
    int n = state.range(0);
    clangRuntimeSpecializer::benchmarkSpecializedExec<Fn_config_count>(
        state, config_count, std::make_tuple(&g_config, n));
}
BENCHMARK(BM_specialized_exec_config_count)->Name("BM_g:synthetic;n:config_count;t:specialized_exec;")->Range(64, 2<<10);

// ── A::add benchmarks (method with known this pointer) ───────────────────────

void BM_unspecialized____method_add(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_a_instance);
        benchmark::DoNotOptimize(g_a_instance.add(7, 11));
    }
}
BENCHMARK(BM_unspecialized____method_add)->Name("BM_g:synthetic;n:method_add;t:unspecialized;");

void BM_jit_overhead_____method_add(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkJITOverhead<Fn_A_add>(
        state,
        &A::add,
        std::make_tuple(&g_a_instance, 7, 11),
        std::make_tuple(&g_a_instance, 7, 11));
}
BENCHMARK(BM_jit_overhead_____method_add)->Name("BM_g:synthetic;n:method_add;t:jit_overhead;");

void BM_specialized_exec_method_add(benchmark::State& state) {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    auto Prev = clangRuntimeSpecializer::ClangRuntimeSpecializer::getLogLevel();
    clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(
        clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::None);
    auto SpecFn = RS->specializeOnly<int>(Fn_A_add, &g_a_instance, 7, 11);
    clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(Prev);
    for (auto _ : state)
        benchmark::DoNotOptimize(SpecFn());
}
BENCHMARK(BM_specialized_exec_method_add)->Name("BM_g:synthetic;n:method_add;t:specialized_exec;");

static int kRegisterMypowBudget = [] {
    clangRuntimeSpecializer::registerBudgetBenchmarks<Fn_mypow>(
        "synthetic", "mypow",
        mypow_bench,
        std::make_tuple(32),
        std::make_tuple(32));
    return 0;
}();

#ifndef ALL_BENCHMARKS_BUILD
int main(int argc, char** argv) {
    static RSSMemoryManager g_rss_mgr;
    benchmark::RegisterMemoryManager(&g_rss_mgr);
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
#endif
