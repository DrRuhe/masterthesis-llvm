#include "UC8Kernels.h"
#include "ClangRuntimeSpecializerBenchmark.h"
#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <algorithm>
#include <cstring>

static constexpr int     ROW_STRIDE    = 24;
static constexpr int     GROUP_COL     = 4;
static constexpr int     VALUE_COL     = 8;
static constexpr int     N_BUCKETS     = 1024;
#ifdef ALL_BENCHMARKS_BUILD
static constexpr int64_t N_ROWS_MAX    = 50'000'000;
#else
static constexpr int64_t N_ROWS_MAX    = 700'000'000;
#endif

// Global dataset: N_ROWS_MAX rows of ROW_STRIDE bytes.
// Benchmarks pass size-specific n_rows via state.range(0).
static std::vector<uint8_t> g_deltas = [] {
    std::vector<uint8_t> data(static_cast<size_t>(N_ROWS_MAX) * ROW_STRIDE, 0);
    std::mt19937 rng(42);
    std::uniform_int_distribution<int32_t> gk_dist(0, N_BUCKETS - 1);
    std::uniform_real_distribution<double>  val_dist(0.0, 100.0);
    std::uniform_int_distribution<uint8_t>  byte_dist(0, 255);
    for (int64_t i = 0; i < N_ROWS_MAX; ++i) {
        uint8_t* row = data.data() + static_cast<size_t>(i) * ROW_STRIDE;
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

static std::vector<double> g_buckets(N_BUCKETS, 0.0);

static void BM_UC8_unspecialized(benchmark::State& state) {
    int64_t n_rows = state.range(0);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        for (int64_t i = 0; i < n_rows; ++i) {
            apply_row_delta(g_deltas.data() + static_cast<size_t>(i) * ROW_STRIDE,
                            g_buckets.data(),
                            N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
        }
        benchmark::DoNotOptimize(g_buckets.data());
    }
}

static void BM_UC8_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_ivm_specialized(N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE));
    }
}

static void BM_UC8_specialized_exec(benchmark::State& state) {
    int64_t n_rows = state.range(0);
    auto spec = create_ivm_specialized(N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        for (int64_t i = 0; i < n_rows; ++i) {
            spec(g_deltas.data() + static_cast<size_t>(i) * ROW_STRIDE,
                 g_buckets.data());
        }
        benchmark::DoNotOptimize(g_buckets.data());
    }
}

#define UC8_BENCHMARK_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC8_unspecialized)->Name("BM_g:uc8_ivm;n:ivm_sum;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_unspecialized)->Name("BM_g:uc8_ivm;n:ivm_sum;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_unspecialized)->Name("BM_g:uc8_ivm;n:ivm_sum;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_unspecialized)->Name("BM_g:uc8_ivm;n:ivm_sum;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_jit_overhead)->Name("BM_g:uc8_ivm;n:ivm_sum;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_jit_overhead)->Name("BM_g:uc8_ivm;n:ivm_sum;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_jit_overhead)->Name("BM_g:uc8_ivm;n:ivm_sum;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_jit_overhead)->Name("BM_g:uc8_ivm;n:ivm_sum;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_specialized_exec)->Name("BM_g:uc8_ivm;n:ivm_sum;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_specialized_exec)->Name("BM_g:uc8_ivm;n:ivm_sum;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_specialized_exec)->Name("BM_g:uc8_ivm;n:ivm_sum;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_specialized_exec)->Name("BM_g:uc8_ivm;n:ivm_sum;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

#ifdef ALL_BENCHMARKS_BUILD
UC8_BENCHMARK_SPEC(
    Arg(1'000'000),
    Arg(10'000'000),
    Arg(30'000'000),
    Arg(50'000'000)
)
#else
UC8_BENCHMARK_SPEC(
    Arg(31'200'000),
    Arg(300'000'000),
    Arg(500'000'000),
    Arg(700'000'000)
)
#endif

#ifndef ALL_BENCHMARKS_BUILD
int main(int argc, char** argv) {
    validate_ivm_specialized(N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);

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
