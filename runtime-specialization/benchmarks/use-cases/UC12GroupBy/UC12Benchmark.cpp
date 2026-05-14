#include "UC12Kernels.h"
#include "ClangRuntimeSpecializerBenchmark.h"
#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <algorithm>
#include <cstring>

static constexpr int     ROW_STRIDE12  = 24;
static constexpr int     KEY_OFFSET12  = 0;
static constexpr int     VALUE_OFFSET12 = 8;
static constexpr int     N_BUCKETS12   = 1024;
#ifdef ALL_BENCHMARKS_BUILD
static constexpr int64_t N_ROWS_MAX12  = 50'000'000;
#else
static constexpr int64_t N_ROWS_MAX12  = 700'000'000;
#endif

// Global dataset: N_ROWS_MAX12 rows of ROW_STRIDE12 bytes.
// Benchmarks pass size-specific n_rows via state.range(0).
static std::vector<uint8_t> g_rows12 = [] {
    std::vector<uint8_t> data(N_ROWS_MAX12 * ROW_STRIDE12, 0);
    std::mt19937_64 rng(42);
    std::uniform_int_distribution<int32_t> key_dist(0, N_BUCKETS12 - 1);
    std::uniform_real_distribution<double> val_dist(0.0, 1.0);
    for (int64_t i = 0; i < N_ROWS_MAX12; ++i) {
        uint8_t* row = data.data() + i * ROW_STRIDE12;
        int32_t key = key_dist(rng);
        std::memcpy(row + KEY_OFFSET12, &key, sizeof(int32_t));
        double val = val_dist(rng);
        std::memcpy(row + VALUE_OFFSET12, &val, sizeof(double));
    }
    return data;
}();

static std::vector<double> g_buckets12(N_BUCKETS12, 0.0);

static void BM_UC12_unspecialized(benchmark::State& state) {
    int64_t n_rows = state.range(0);
    for (auto _ : state) {
        std::fill(g_buckets12.begin(), g_buckets12.end(), 0.0);
        grouped_sum(g_rows12.data(), n_rows, ROW_STRIDE12, KEY_OFFSET12,
                    VALUE_OFFSET12, g_buckets12.data(), N_BUCKETS12);
        benchmark::DoNotOptimize(g_buckets12.data());
    }
}

static void BM_UC12_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_groupby_specialized(ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12));
    }
}

static void BM_UC12_specialized_exec(benchmark::State& state) {
    int64_t n_rows = state.range(0);
    auto spec = create_groupby_specialized(ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12);
    for (auto _ : state) {
        std::fill(g_buckets12.begin(), g_buckets12.end(), 0.0);
        spec(g_rows12.data(), n_rows, g_buckets12.data());
        benchmark::DoNotOptimize(g_buckets12.data());
    }
}

#define UC12_BENCHMARK_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC12_unspecialized)->Name("BM_g:uc12_groupby;n:groupby_sum;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_unspecialized)->Name("BM_g:uc12_groupby;n:groupby_sum;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_unspecialized)->Name("BM_g:uc12_groupby;n:groupby_sum;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_unspecialized)->Name("BM_g:uc12_groupby;n:groupby_sum;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_jit_overhead)->Name("BM_g:uc12_groupby;n:groupby_sum;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_jit_overhead)->Name("BM_g:uc12_groupby;n:groupby_sum;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_jit_overhead)->Name("BM_g:uc12_groupby;n:groupby_sum;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_jit_overhead)->Name("BM_g:uc12_groupby;n:groupby_sum;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_specialized_exec)->Name("BM_g:uc12_groupby;n:groupby_sum;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_specialized_exec)->Name("BM_g:uc12_groupby;n:groupby_sum;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_specialized_exec)->Name("BM_g:uc12_groupby;n:groupby_sum;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_specialized_exec)->Name("BM_g:uc12_groupby;n:groupby_sum;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

#ifdef ALL_BENCHMARKS_BUILD
UC12_BENCHMARK_SPEC(
    Arg(1'000'000),
    Arg(10'000'000),
    Arg(30'000'000),
    Arg(50'000'000)
)
#else
UC12_BENCHMARK_SPEC(
    Arg(31'000'000),
    Arg(300'000'000),
    Arg(500'000'000),
    Arg(700'000'000)
)
#endif

#ifndef ALL_BENCHMARKS_BUILD
int main(int argc, char** argv) {
    validate_groupby_specialized(ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12);

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
