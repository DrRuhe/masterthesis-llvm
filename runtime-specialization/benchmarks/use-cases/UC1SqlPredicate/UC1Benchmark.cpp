#include "UC1Kernels.h"
#include "ClangRuntimeSpecializerBenchmark.h"
#include <benchmark/benchmark.h>
#include <vector>
#include <random>

static constexpr int     ROW_STRIDE = 16;
static constexpr int     COL_OFFSET = 8;
static constexpr int64_t N_ROWS     = 10'000'000;

// Global dataset: N_ROWS rows of ROW_STRIDE bytes each.
// Random double values at col_offset=8; random byte garbage elsewhere.
static std::vector<uint8_t> g_rows = [] {
    std::vector<uint8_t> data(N_ROWS * ROW_STRIDE, 0);
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    std::uniform_int_distribution<uint8_t> byte_dist(0, 255);
    for (int64_t i = 0; i < N_ROWS; ++i) {
        // Fill non-payload bytes with garbage
        for (int b = 0; b < COL_OFFSET; ++b)
            data[i * ROW_STRIDE + b] = byte_dist(rng);
        // Write the predicate column value
        double v = dist(rng);
        __builtin_memcpy(data.data() + i * ROW_STRIDE + COL_OFFSET, &v, sizeof(double));
        // Fill remaining bytes
        for (int b = COL_OFFSET + (int)sizeof(double); b < ROW_STRIDE; ++b)
            data[i * ROW_STRIDE + b] = byte_dist(rng);
    }
    return data;
}();

// BM_UC1_unspecialized: direct call with runtime schema constants
static void BM_UC1_unspecialized(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            count_matching_rows(g_rows.data(), N_ROWS, ROW_STRIDE, COL_OFFSET, 0.5));
    }
}
BENCHMARK(BM_UC1_unspecialized)
    ->Name("BM_g:uc1_sql;n:predicate;t:unspecialized;")
    ->Unit(benchmark::kMillisecond);

// BM_UC1_jit_overhead: measure JIT compilation cost only (single iteration)
static void BM_UC1_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_sql_specialized(ROW_STRIDE, COL_OFFSET, 0.5));
    }
}
BENCHMARK(BM_UC1_jit_overhead)
    ->Name("BM_g:uc1_sql;n:predicate;t:jit_overhead;")
    ->Unit(benchmark::kMillisecond);

// BM_UC1_specialized_exec: factory called once before loop, then execute specialized fn
static void BM_UC1_specialized_exec(benchmark::State& state) {
    auto spec = create_sql_specialized(ROW_STRIDE, COL_OFFSET, 0.5);
    for (auto _ : state) {
        benchmark::DoNotOptimize(spec(g_rows.data(), N_ROWS));
    }
}
BENCHMARK(BM_UC1_specialized_exec)
    ->Name("BM_g:uc1_sql;n:predicate;t:specialized_exec;")
    ->Unit(benchmark::kMillisecond);

#ifndef ALL_BENCHMARKS_BUILD
int main(int argc, char** argv) {
    // Validate correctness before running benchmarks
    validate_sql_specialized(ROW_STRIDE, COL_OFFSET, 0.5);

    static RSSMemoryManager g_rss_mgr;
    benchmark::RegisterMemoryManager(&g_rss_mgr);
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
#endif // ALL_BENCHMARKS_BUILD
