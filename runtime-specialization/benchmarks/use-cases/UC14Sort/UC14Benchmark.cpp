#include "UC14Kernels.h"
#include "ClangRuntimeSpecializerBenchmark.h"
#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <algorithm>
#include <cstdint>
#include <numeric>

static constexpr int64_t N_SORT_MAX = 200'000'000;

// Global dataset: N_SORT_MAX int64_t values.
// Benchmarks pass size-specific n_elements via state.range(0).
static std::vector<int64_t> g_data = [] {
    std::vector<int64_t> d(N_SORT_MAX);
    std::iota(d.begin(), d.end(), 0);
    std::shuffle(d.begin(), d.end(), std::mt19937{42});
    return d;
}();

static void BM_UC14_unspecialized(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    std::mt19937 rng{42};
    for (auto _ : state) {
        state.PauseTiming();
        std::shuffle(g_data.begin(), g_data.begin() + n_elements, rng);
        state.ResumeTiming();
        generic_sort(g_data.data(), n_elements, sizeof(int64_t), &int64_asc_cmp);
    }
}

static void BM_UC14_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_sort_specialized(&int64_asc_cmp, sizeof(int64_t)));
    }
}

static void BM_UC14_specialized_exec(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    auto spec = create_sort_specialized(&int64_asc_cmp, sizeof(int64_t));
    std::mt19937 local_rng{42};
    for (auto _ : state) {
        state.PauseTiming();
        std::shuffle(g_data.begin(), g_data.begin() + n_elements, local_rng);
        state.ResumeTiming();
        spec(g_data.data(), n_elements);
    }
}

#define UC14_BENCHMARK_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC14_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

UC14_BENCHMARK_SPEC(
    Arg(550'000),
    Arg(5'000'000),
    Arg(40'000'000),
    Arg(200'000'000)
)

#ifndef ALL_BENCHMARKS_BUILD
int main(int argc, char** argv) {
    validate_sort_specialized(&int64_asc_cmp, sizeof(int64_t));

    static RSSMemoryManager g_rss_mgr;
    benchmark::RegisterMemoryManager(&g_rss_mgr);
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    if (benchmark::GetBenchmarkFilter() == "")
        benchmark::SetBenchmarkFilter("s:SMALL|s:MEDIUM|s:LARGE");
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
#endif // ALL_BENCHMARKS_BUILD
