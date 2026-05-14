#include "UC12Kernels.h"
#include "ClangRuntimeSpecializerBenchmark.h"
#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <algorithm>
#include <cstring>

static constexpr int64_t N_ROWS12      = 10'000'000;
static constexpr int     ROW_STRIDE12  = 24;
static constexpr int     KEY_OFFSET12  = 0;
static constexpr int     VALUE_OFFSET12 = 8;
static constexpr int     N_BUCKETS12   = 1024;

// Global dataset: N_ROWS12 rows of ROW_STRIDE12 bytes each.
// int32 keys at offset 0, doubles at offset 8.
static std::vector<uint8_t> g_rows12 = [] {
    std::vector<uint8_t> data(N_ROWS12 * ROW_STRIDE12, 0);
    std::mt19937_64 rng(42);
    std::uniform_int_distribution<int32_t> key_dist(0, N_BUCKETS12 - 1);
    std::uniform_real_distribution<double> val_dist(0.0, 1.0);
    for (int64_t i = 0; i < N_ROWS12; ++i) {
        uint8_t* row = data.data() + i * ROW_STRIDE12;
        int32_t key = key_dist(rng);
        std::memcpy(row + KEY_OFFSET12, &key, sizeof(int32_t));
        double val = val_dist(rng);
        std::memcpy(row + VALUE_OFFSET12, &val, sizeof(double));
    }
    return data;
}();

// Output buckets — zeroed between iterations in specialized_exec benchmark.
static std::vector<double> g_buckets12(N_BUCKETS12, 0.0);

// BM_UC12_unspecialized: direct call with runtime schema constants
static void BM_UC12_unspecialized(benchmark::State& state) {
    for (auto _ : state) {
        std::fill(g_buckets12.begin(), g_buckets12.end(), 0.0);
        grouped_sum(g_rows12.data(), N_ROWS12, ROW_STRIDE12, KEY_OFFSET12,
                    VALUE_OFFSET12, g_buckets12.data(), N_BUCKETS12);
        benchmark::DoNotOptimize(g_buckets12.data());
    }
}
BENCHMARK(BM_UC12_unspecialized)
    ->Name("BM_g:uc12_groupby;n:groupby_sum;t:unspecialized;")
    ->Unit(benchmark::kMillisecond);

// BM_UC12_jit_overhead: measure JIT compilation cost only (single iteration)
static void BM_UC12_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_groupby_specialized(ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12));
    }
}
BENCHMARK(BM_UC12_jit_overhead)
    ->Name("BM_g:uc12_groupby;n:groupby_sum;t:jit_overhead;")
    ->Unit(benchmark::kMillisecond);

// BM_UC12_specialized_exec: factory called once before loop, then execute specialized fn
static void BM_UC12_specialized_exec(benchmark::State& state) {
    auto spec = create_groupby_specialized(ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12);
    for (auto _ : state) {
        std::fill(g_buckets12.begin(), g_buckets12.end(), 0.0);
        spec(g_rows12.data(), N_ROWS12, g_buckets12.data());
        benchmark::DoNotOptimize(g_buckets12.data());
    }
}
BENCHMARK(BM_UC12_specialized_exec)
    ->Name("BM_g:uc12_groupby;n:groupby_sum;t:specialized_exec;")
    ->Unit(benchmark::kMillisecond);

#ifndef ALL_BENCHMARKS_BUILD
int main(int argc, char** argv) {
    // Validate correctness before running benchmarks
    validate_groupby_specialized(ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12);

    static RSSMemoryManager g_rss_mgr;
    benchmark::RegisterMemoryManager(&g_rss_mgr);
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
#endif // ALL_BENCHMARKS_BUILD
