#include "UC8Kernels.h"
#include "ClangRuntimeSpecializerBenchmark.h"
#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <algorithm>
#include <cstring>

static constexpr int N_ROWS        = 10'000'000;
static constexpr int ROW_STRIDE    = 24;
static constexpr int GROUP_COL     = 4;
static constexpr int VALUE_COL     = 8;
static constexpr int N_BUCKETS     = 1024;

// Global dataset: N_ROWS rows of ROW_STRIDE bytes each.
// int32 group key at offset GROUP_COL, double value at offset VALUE_COL.
static std::vector<uint8_t> g_deltas = [] {
    std::vector<uint8_t> data(static_cast<size_t>(N_ROWS) * ROW_STRIDE, 0);
    std::mt19937 rng(42);
    std::uniform_int_distribution<int32_t> gk_dist(0, N_BUCKETS - 1);
    std::uniform_real_distribution<double>  val_dist(0.0, 100.0);
    std::uniform_int_distribution<uint8_t>  byte_dist(0, 255);
    for (int i = 0; i < N_ROWS; ++i) {
        uint8_t* row = data.data() + static_cast<size_t>(i) * ROW_STRIDE;
        // Garbage in non-payload bytes
        for (int b = 0; b < GROUP_COL; ++b)
            row[b] = byte_dist(rng);
        int32_t gk = gk_dist(rng);
        __builtin_memcpy(row + GROUP_COL, &gk, sizeof(int32_t));
        double val = val_dist(rng);
        __builtin_memcpy(row + VALUE_COL, &val, sizeof(double));
        for (int b = VALUE_COL + (int)sizeof(double); b < ROW_STRIDE; ++b)
            row[b] = byte_dist(rng);
    }
    return data;
}();

// Pre-allocated output buckets.
static std::vector<double> g_buckets(N_BUCKETS, 0.0);

// BM_UC8_unspecialized: direct call with runtime schema constants, per-row loop.
static void BM_UC8_unspecialized(benchmark::State& state) {
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        for (int i = 0; i < N_ROWS; ++i) {
            apply_row_delta(g_deltas.data() + static_cast<size_t>(i) * ROW_STRIDE,
                            g_buckets.data(),
                            N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
        }
        benchmark::DoNotOptimize(g_buckets.data());
    }
}
BENCHMARK(BM_UC8_unspecialized)
    ->Name("BM_g:uc8_ivm;n:ivm_sum;t:unspecialized;")
    ->Unit(benchmark::kMillisecond);

// BM_UC8_jit_overhead: measure JIT compilation cost only (single iteration).
static void BM_UC8_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_ivm_specialized(N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE));
    }
}
BENCHMARK(BM_UC8_jit_overhead)
    ->Name("BM_g:uc8_ivm;n:ivm_sum;t:jit_overhead;")
    ->Unit(benchmark::kMillisecond);

// BM_UC8_specialized_exec: factory once before loop, then execute specialized fn per row.
static void BM_UC8_specialized_exec(benchmark::State& state) {
    auto spec = create_ivm_specialized(N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        for (int i = 0; i < N_ROWS; ++i) {
            spec(g_deltas.data() + static_cast<size_t>(i) * ROW_STRIDE,
                 g_buckets.data());
        }
        benchmark::DoNotOptimize(g_buckets.data());
    }
}
BENCHMARK(BM_UC8_specialized_exec)
    ->Name("BM_g:uc8_ivm;n:ivm_sum;t:specialized_exec;")
    ->Unit(benchmark::kMillisecond);

#ifndef ALL_BENCHMARKS_BUILD
int main(int argc, char** argv) {
    // Validate correctness before running benchmarks.
    validate_ivm_specialized(N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);

    static RSSMemoryManager g_rss_mgr;
    benchmark::RegisterMemoryManager(&g_rss_mgr);
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
#endif // ALL_BENCHMARKS_BUILD
