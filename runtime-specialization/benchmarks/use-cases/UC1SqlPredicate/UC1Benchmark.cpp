#include "UC1Kernels.h"
#include "ClangRuntimeSpecializerBenchmark.h"
#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <algorithm>

static constexpr int     ROW_STRIDE   = 16;
static constexpr int     COL_OFFSET   = 8;   // column A (primary)
static constexpr int     COL_OFFSET_B = 0;   // column B (multi_predicate second column)
// Fixed buffer: 50M rows × 16 B = 800 MB. For sizes larger than N_ROWS_MAX
// the benchmarks loop through this buffer multiple times (streaming pattern).
static constexpr int64_t N_ROWS_MAX   = 50'000'000;

// Global dataset: N_ROWS_MAX rows of ROW_STRIDE bytes.
static std::vector<uint8_t> g_rows = [] {
    std::vector<uint8_t> data(N_ROWS_MAX * ROW_STRIDE, 0);
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    std::uniform_int_distribution<uint8_t> byte_dist(0, 255);
    for (int64_t i = 0; i < N_ROWS_MAX; ++i) {
        // COL_OFFSET_B (0..7): write a random double at byte 0.
        double vb = dist(rng);
        __builtin_memcpy(data.data() + i * ROW_STRIDE + COL_OFFSET_B, &vb, sizeof(double));
        // COL_OFFSET (8..15): write a random double at byte 8.
        double va = dist(rng);
        __builtin_memcpy(data.data() + i * ROW_STRIDE + COL_OFFSET, &va, sizeof(double));
    }
    return data;
}();

// Output buffer for column_scan variants (indices written here).
static std::vector<int32_t> g_out_indices(N_ROWS_MAX);

// ============================================================================
// count_matching_rows / low
// ============================================================================

static void BM_UC1_count_matching_rows_low_unspecialized(benchmark::State& state) {
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

static void BM_UC1_count_matching_rows_low_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_sql_specialized(ROW_STRIDE, COL_OFFSET, 0.5));
    }
}

static void BM_UC1_count_matching_rows_low_specialized_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_sql_specialized(ROW_STRIDE, COL_OFFSET, 0.5);
    for (auto _ : state) {
        int64_t total = 0;
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX)
            total += spec(g_rows.data(), std::min(rem, (int64_t)N_ROWS_MAX));
        benchmark::DoNotOptimize(total);
    }
}

// ============================================================================
// count_matching_rows / tradeoff
// ============================================================================

static void BM_UC1_count_matching_rows_tradeoff_unspecialized(benchmark::State& state) {
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

static void BM_UC1_count_matching_rows_tradeoff_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_count_matching_rows_tradeoff_specialized(COL_OFFSET, ROW_STRIDE, 0.5));
    }
}

static void BM_UC1_count_matching_rows_tradeoff_specialized_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_count_matching_rows_tradeoff_specialized(COL_OFFSET, ROW_STRIDE, 0.5);
    for (auto _ : state) {
        int64_t total = 0;
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX)
            total += spec(g_rows.data(), std::min(rem, (int64_t)N_ROWS_MAX));
        benchmark::DoNotOptimize(total);
    }
}

// ============================================================================
// count_matching_rows / abstract
// ============================================================================

static void BM_UC1_count_matching_rows_abstract_unspecialized(benchmark::State& state) {
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

static void BM_UC1_count_matching_rows_abstract_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_count_matching_rows_abstract_specialized(COL_OFFSET, ROW_STRIDE, 0.5));
    }
}

static void BM_UC1_count_matching_rows_abstract_specialized_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_count_matching_rows_abstract_specialized(COL_OFFSET, ROW_STRIDE, 0.5);
    for (auto _ : state) {
        int64_t total = 0;
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX)
            total += spec(g_rows.data(), std::min(rem, (int64_t)N_ROWS_MAX));
        benchmark::DoNotOptimize(total);
    }
}

// ============================================================================
// multi_predicate / low
// ============================================================================

static void BM_UC1_multi_predicate_low_unspecialized(benchmark::State& state) {
    int64_t n_total = state.range(0);
    // Unspecialized baseline: use a one-time specialized object but measure exec each iteration.
    // The "unspecialized" here represents the cost of calling count_matching_rows twice
    // (two-column predicate equivalent without specialization).
    for (auto _ : state) {
        int64_t total = 0;
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX) {
            int64_t chunk = std::min(rem, (int64_t)N_ROWS_MAX);
            total += count_matching_rows(g_rows.data(), chunk, ROW_STRIDE, COL_OFFSET, 0.5)
                   + count_matching_rows(g_rows.data(), chunk, ROW_STRIDE, COL_OFFSET_B, 0.5);
        }
        benchmark::DoNotOptimize(total);
    }
}

static void BM_UC1_multi_predicate_low_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_multi_predicate_low_specialized(ROW_STRIDE, COL_OFFSET, COL_OFFSET_B, 0.5, 0.5));
    }
}

static void BM_UC1_multi_predicate_low_specialized_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_multi_predicate_low_specialized(ROW_STRIDE, COL_OFFSET, COL_OFFSET_B, 0.5, 0.5);
    for (auto _ : state) {
        int64_t total = 0;
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX)
            total += spec(g_rows.data(), std::min(rem, (int64_t)N_ROWS_MAX));
        benchmark::DoNotOptimize(total);
    }
}

// ============================================================================
// multi_predicate / tradeoff
// ============================================================================

static void BM_UC1_multi_predicate_tradeoff_unspecialized(benchmark::State& state) {
    int64_t n_total = state.range(0);
    for (auto _ : state) {
        int64_t total = 0;
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX) {
            int64_t chunk = std::min(rem, (int64_t)N_ROWS_MAX);
            total += count_matching_rows(g_rows.data(), chunk, ROW_STRIDE, COL_OFFSET, 0.5)
                   + count_matching_rows(g_rows.data(), chunk, ROW_STRIDE, COL_OFFSET_B, 0.5);
        }
        benchmark::DoNotOptimize(total);
    }
}

static void BM_UC1_multi_predicate_tradeoff_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_multi_predicate_tradeoff_specialized(ROW_STRIDE, COL_OFFSET, COL_OFFSET_B, 0.5, 0.5));
    }
}

static void BM_UC1_multi_predicate_tradeoff_specialized_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_multi_predicate_tradeoff_specialized(ROW_STRIDE, COL_OFFSET, COL_OFFSET_B, 0.5, 0.5);
    for (auto _ : state) {
        int64_t total = 0;
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX)
            total += spec(g_rows.data(), std::min(rem, (int64_t)N_ROWS_MAX));
        benchmark::DoNotOptimize(total);
    }
}

// ============================================================================
// multi_predicate / abstract
// ============================================================================

static void BM_UC1_multi_predicate_abstract_unspecialized(benchmark::State& state) {
    int64_t n_total = state.range(0);
    for (auto _ : state) {
        int64_t total = 0;
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX) {
            int64_t chunk = std::min(rem, (int64_t)N_ROWS_MAX);
            total += count_matching_rows(g_rows.data(), chunk, ROW_STRIDE, COL_OFFSET, 0.5)
                   + count_matching_rows(g_rows.data(), chunk, ROW_STRIDE, COL_OFFSET_B, 0.5);
        }
        benchmark::DoNotOptimize(total);
    }
}

static void BM_UC1_multi_predicate_abstract_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_multi_predicate_abstract_specialized(ROW_STRIDE, COL_OFFSET, COL_OFFSET_B, 0.5, 0.5));
    }
}

static void BM_UC1_multi_predicate_abstract_specialized_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_multi_predicate_abstract_specialized(ROW_STRIDE, COL_OFFSET, COL_OFFSET_B, 0.5, 0.5);
    for (auto _ : state) {
        int64_t total = 0;
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX)
            total += spec(g_rows.data(), std::min(rem, (int64_t)N_ROWS_MAX));
        benchmark::DoNotOptimize(total);
    }
}

// ============================================================================
// column_scan / low
// ============================================================================

static void BM_UC1_column_scan_low_unspecialized(benchmark::State& state) {
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

static void BM_UC1_column_scan_low_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_column_scan_low_specialized(ROW_STRIDE, COL_OFFSET, 0.5));
    }
}

static void BM_UC1_column_scan_low_specialized_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_column_scan_low_specialized(ROW_STRIDE, COL_OFFSET, 0.5);
    for (auto _ : state) {
        int64_t total = 0;
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX)
            total += spec(g_rows.data(), std::min(rem, (int64_t)N_ROWS_MAX),
                          g_out_indices.data());
        benchmark::DoNotOptimize(total);
    }
}

// ============================================================================
// column_scan / tradeoff
// ============================================================================

static void BM_UC1_column_scan_tradeoff_unspecialized(benchmark::State& state) {
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

static void BM_UC1_column_scan_tradeoff_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_column_scan_tradeoff_specialized(ROW_STRIDE, COL_OFFSET, 0.5));
    }
}

static void BM_UC1_column_scan_tradeoff_specialized_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_column_scan_tradeoff_specialized(ROW_STRIDE, COL_OFFSET, 0.5);
    for (auto _ : state) {
        int64_t total = 0;
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX)
            total += spec(g_rows.data(), std::min(rem, (int64_t)N_ROWS_MAX),
                          g_out_indices.data());
        benchmark::DoNotOptimize(total);
    }
}

// ============================================================================
// column_scan / abstract
// ============================================================================

static void BM_UC1_column_scan_abstract_unspecialized(benchmark::State& state) {
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

static void BM_UC1_column_scan_abstract_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_column_scan_abstract_specialized(ROW_STRIDE, COL_OFFSET, 0.5));
    }
}

static void BM_UC1_column_scan_abstract_specialized_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_column_scan_abstract_specialized(ROW_STRIDE, COL_OFFSET, 0.5);
    for (auto _ : state) {
        int64_t total = 0;
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX)
            total += spec(g_rows.data(), std::min(rem, (int64_t)N_ROWS_MAX),
                          g_out_indices.data());
        benchmark::DoNotOptimize(total);
    }
}

// ============================================================================
// BENCHMARK macros
// ============================================================================

// count_matching_rows / low  (original 12 macros — kept for backward compatibility)
#define UC1_BENCHMARK_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC1_count_matching_rows_low_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

// count_matching_rows / tradeoff
#define UC1_BENCHMARK_CMR_TRADEOFF(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

// count_matching_rows / abstract
#define UC1_BENCHMARK_CMR_ABSTRACT(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC1_count_matching_rows_abstract_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

// multi_predicate / low
#define UC1_BENCHMARK_MP_LOW(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC1_multi_predicate_low_unspecialized)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_unspecialized)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_unspecialized)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_unspecialized)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_jit_overhead)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_jit_overhead)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_jit_overhead)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_jit_overhead)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_specialized_exec)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_specialized_exec)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_specialized_exec)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_specialized_exec)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

// multi_predicate / tradeoff
#define UC1_BENCHMARK_MP_TRADEOFF(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_unspecialized)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_unspecialized)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_unspecialized)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_unspecialized)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_jit_overhead)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_jit_overhead)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_jit_overhead)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_jit_overhead)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_specialized_exec)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_specialized_exec)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_specialized_exec)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_specialized_exec)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

// multi_predicate / abstract
#define UC1_BENCHMARK_MP_ABSTRACT(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC1_multi_predicate_abstract_unspecialized)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_unspecialized)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_unspecialized)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_unspecialized)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_jit_overhead)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_jit_overhead)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_jit_overhead)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_jit_overhead)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_specialized_exec)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_specialized_exec)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_specialized_exec)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_specialized_exec)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

// column_scan / low
#define UC1_BENCHMARK_CS_LOW(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC1_column_scan_low_unspecialized)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_unspecialized)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_unspecialized)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_unspecialized)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_jit_overhead)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_jit_overhead)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_jit_overhead)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_jit_overhead)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_specialized_exec)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_specialized_exec)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_specialized_exec)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_specialized_exec)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

// column_scan / tradeoff
#define UC1_BENCHMARK_CS_TRADEOFF(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC1_column_scan_tradeoff_unspecialized)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_unspecialized)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_unspecialized)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_unspecialized)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_jit_overhead)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_jit_overhead)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_jit_overhead)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_jit_overhead)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_specialized_exec)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_specialized_exec)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_specialized_exec)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_specialized_exec)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

// column_scan / abstract
#define UC1_BENCHMARK_CS_ABSTRACT(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC1_column_scan_abstract_unspecialized)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_unspecialized)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_unspecialized)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_unspecialized)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_jit_overhead)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_jit_overhead)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_jit_overhead)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_jit_overhead)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_specialized_exec)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_specialized_exec)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_specialized_exec)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_specialized_exec)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

// ============================================================================
// Instantiate all macros
// ============================================================================

#ifdef ALL_BENCHMARKS_BUILD
#define UC1_SIZES Arg(1'000'000), Arg(10'000'000), Arg(30'000'000), Arg(50'000'000)
UC1_BENCHMARK_SPEC(Arg(1'000'000), Arg(10'000'000), Arg(30'000'000), Arg(50'000'000))
UC1_BENCHMARK_CMR_TRADEOFF(Arg(1'000'000), Arg(10'000'000), Arg(30'000'000), Arg(50'000'000))
UC1_BENCHMARK_CMR_ABSTRACT(Arg(1'000'000), Arg(10'000'000), Arg(30'000'000), Arg(50'000'000))
UC1_BENCHMARK_MP_LOW(Arg(1'000'000), Arg(10'000'000), Arg(30'000'000), Arg(50'000'000))
UC1_BENCHMARK_MP_TRADEOFF(Arg(1'000'000), Arg(10'000'000), Arg(30'000'000), Arg(50'000'000))
UC1_BENCHMARK_MP_ABSTRACT(Arg(1'000'000), Arg(10'000'000), Arg(30'000'000), Arg(50'000'000))
UC1_BENCHMARK_CS_LOW(Arg(1'000'000), Arg(10'000'000), Arg(30'000'000), Arg(50'000'000))
UC1_BENCHMARK_CS_TRADEOFF(Arg(1'000'000), Arg(10'000'000), Arg(30'000'000), Arg(50'000'000))
UC1_BENCHMARK_CS_ABSTRACT(Arg(1'000'000), Arg(10'000'000), Arg(30'000'000), Arg(50'000'000))
#else
UC1_BENCHMARK_SPEC(Arg(65'000'000LL), Arg(640'000'000LL), Arg(6'400'000'000LL), Arg(38'500'000'000LL))
UC1_BENCHMARK_CMR_TRADEOFF(Arg(65'000'000LL), Arg(640'000'000LL), Arg(6'400'000'000LL), Arg(38'500'000'000LL))
UC1_BENCHMARK_CMR_ABSTRACT(Arg(65'000'000LL), Arg(640'000'000LL), Arg(6'400'000'000LL), Arg(38'500'000'000LL))
UC1_BENCHMARK_MP_LOW(Arg(65'000'000LL), Arg(640'000'000LL), Arg(6'400'000'000LL), Arg(38'500'000'000LL))
UC1_BENCHMARK_MP_TRADEOFF(Arg(65'000'000LL), Arg(640'000'000LL), Arg(6'400'000'000LL), Arg(38'500'000'000LL))
UC1_BENCHMARK_MP_ABSTRACT(Arg(65'000'000LL), Arg(640'000'000LL), Arg(6'400'000'000LL), Arg(38'500'000'000LL))
UC1_BENCHMARK_CS_LOW(Arg(65'000'000LL), Arg(640'000'000LL), Arg(6'400'000'000LL), Arg(38'500'000'000LL))
UC1_BENCHMARK_CS_TRADEOFF(Arg(65'000'000LL), Arg(640'000'000LL), Arg(6'400'000'000LL), Arg(38'500'000'000LL))
UC1_BENCHMARK_CS_ABSTRACT(Arg(65'000'000LL), Arg(640'000'000LL), Arg(6'400'000'000LL), Arg(38'500'000'000LL))
#endif

#ifndef ALL_BENCHMARKS_BUILD
int main(int argc, char** argv) {
    // Validate all 9 variants.
    validate_sql_specialized(ROW_STRIDE, COL_OFFSET, 0.5);
    validate_count_matching_rows_tradeoff_specialized(COL_OFFSET, ROW_STRIDE, 0.5);
    validate_count_matching_rows_abstract_specialized(COL_OFFSET, ROW_STRIDE, 0.5);
    validate_multi_predicate_low_specialized(ROW_STRIDE, COL_OFFSET, COL_OFFSET_B, 0.5, 0.5);
    validate_multi_predicate_tradeoff_specialized(ROW_STRIDE, COL_OFFSET, COL_OFFSET_B, 0.5, 0.5);
    validate_multi_predicate_abstract_specialized(ROW_STRIDE, COL_OFFSET, COL_OFFSET_B, 0.5, 0.5);
    validate_column_scan_low_specialized(ROW_STRIDE, COL_OFFSET, 0.5);
    validate_column_scan_tradeoff_specialized(ROW_STRIDE, COL_OFFSET, 0.5);
    validate_column_scan_abstract_specialized(ROW_STRIDE, COL_OFFSET, 0.5);

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
