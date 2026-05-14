#include "UC1Kernels.h"
#include "ClangRuntimeSpecializerBenchmark.h"
#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <algorithm>

static constexpr int     ROW_STRIDE  = 16;
static constexpr int     COL_OFFSET  = 8;
// Fixed buffer: 50M rows × 16 B = 800 MB.  For sizes larger than N_ROWS_MAX
// the benchmarks loop through this buffer multiple times (streaming pattern).
static constexpr int64_t N_ROWS_MAX  = 50'000'000;

// Global dataset: N_ROWS_MAX rows of ROW_STRIDE bytes.
// Benchmarks pass a size-specific n_rows via state.range(0).
static std::vector<uint8_t> g_rows = [] {
    std::vector<uint8_t> data(N_ROWS_MAX * ROW_STRIDE, 0);
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    std::uniform_int_distribution<uint8_t> byte_dist(0, 255);
    for (int64_t i = 0; i < N_ROWS_MAX; ++i) {
        for (int b = 0; b < COL_OFFSET; ++b)
            data[i * ROW_STRIDE + b] = byte_dist(rng);
        double v = dist(rng);
        __builtin_memcpy(data.data() + i * ROW_STRIDE + COL_OFFSET, &v, sizeof(double));
        for (int b = COL_OFFSET + (int)sizeof(double); b < ROW_STRIDE; ++b)
            data[i * ROW_STRIDE + b] = byte_dist(rng);
    }
    return data;
}();

static void BM_UC1_unspecialized(benchmark::State& state) {
    int64_t n_total = state.range(0);
    for (auto _ : state) {
        int64_t total = 0;
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX)
            total += count_matching_rows(g_rows.data(),
                                         std::min(rem, (int64_t)N_ROWS_MAX),
                                         ROW_STRIDE, COL_OFFSET, 0.5);
        benchmark::DoNotOptimize(total);
    }
}

static void BM_UC1_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_sql_specialized(ROW_STRIDE, COL_OFFSET, 0.5));
    }
}

static void BM_UC1_specialized_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_sql_specialized(ROW_STRIDE, COL_OFFSET, 0.5);
    for (auto _ : state) {
        int64_t total = 0;
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX)
            total += spec(g_rows.data(), std::min(rem, (int64_t)N_ROWS_MAX));
        benchmark::DoNotOptimize(total);
    }
}

#define UC1_BENCHMARK_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC1_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

#ifdef ALL_BENCHMARKS_BUILD
UC1_BENCHMARK_SPEC(
    Arg(1'000'000),
    Arg(10'000'000),
    Arg(30'000'000),
    Arg(50'000'000)
)
#else
UC1_BENCHMARK_SPEC(
    Arg(16'000'000),
    Arg(150'000'000),
    Arg(800'000'000),
    Arg(4'550'000'000LL)
)
#endif

#ifndef ALL_BENCHMARKS_BUILD
int main(int argc, char** argv) {
    validate_sql_specialized(ROW_STRIDE, COL_OFFSET, 0.5);

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
