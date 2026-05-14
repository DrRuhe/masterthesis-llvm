#include "UC14Kernels.h"
#include "ClangRuntimeSpecializerBenchmark.h"
#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <algorithm>
#include <cstdint>
#include <numeric>

static constexpr int64_t N_SORT = 1'000'000;

// Global dataset: 1M int64_t values, initially shuffled
static std::vector<int64_t> g_data = [] {
    std::vector<int64_t> d(N_SORT);
    std::iota(d.begin(), d.end(), 0);
    std::shuffle(d.begin(), d.end(), std::mt19937{42});
    return d;
}();

// BM_UC14_unspecialized: direct call with runtime comparator pointer
static void BM_UC14_unspecialized(benchmark::State& state) {
    std::mt19937 rng{42};
    for (auto _ : state) {
        state.PauseTiming();
        std::shuffle(g_data.begin(), g_data.end(), rng);
        state.ResumeTiming();
        generic_sort(g_data.data(), N_SORT, sizeof(int64_t), &int64_asc_cmp);
    }
}
BENCHMARK(BM_UC14_unspecialized)
    ->Name("BM_g:uc14_sort;n:sort_int64;t:unspecialized;")
    ->Unit(benchmark::kMillisecond);

// BM_UC14_jit_overhead: measure JIT compilation cost only (single iteration)
static void BM_UC14_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_sort_specialized(&int64_asc_cmp, sizeof(int64_t)));
    }
}
BENCHMARK(BM_UC14_jit_overhead)
    ->Name("BM_g:uc14_sort;n:sort_int64;t:jit_overhead;")
    ->Unit(benchmark::kMillisecond);

// BM_UC14_specialized_exec: factory called once before loop, execute specialized sort
static void BM_UC14_specialized_exec(benchmark::State& state) {
    auto spec = create_sort_specialized(&int64_asc_cmp, sizeof(int64_t));
    std::mt19937 local_rng{42};
    for (auto _ : state) {
        state.PauseTiming();
        std::shuffle(g_data.begin(), g_data.end(), local_rng);
        state.ResumeTiming();
        spec(g_data.data(), N_SORT);
    }
}
BENCHMARK(BM_UC14_specialized_exec)
    ->Name("BM_g:uc14_sort;n:sort_int64;t:specialized_exec;")
    ->Unit(benchmark::kMillisecond);

#ifndef ALL_BENCHMARKS_BUILD
int main(int argc, char** argv) {
    // Validate correctness before running benchmarks
    validate_sort_specialized(&int64_asc_cmp, sizeof(int64_t));

    static RSSMemoryManager g_rss_mgr;
    benchmark::RegisterMemoryManager(&g_rss_mgr);
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
#endif // ALL_BENCHMARKS_BUILD
