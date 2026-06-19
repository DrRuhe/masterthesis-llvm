#include "UC1Kernels.h"
#include "ClangRuntimeSpecializerBenchmark.h"
#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <algorithm>

static constexpr int     ROW_STRIDE   = 16;
static constexpr int     COL_OFFSET   = 8;   // column A (primary)
static constexpr int     COL_OFFSET_B = 0;   // column B (multi_predicate second column)
// Buffer = 1 GB / 16 B/row = 64 M rows.  Each benchmark calls the kernel
// exactly once per iteration with min(state.range(0), N_ROWS_MAX) rows.
static constexpr int64_t N_ROWS_MAX   = 64'000'000;

// Global dataset: N_ROWS_MAX rows of ROW_STRIDE bytes.
static std::vector<uint8_t> g_rows;

// Output buffer for column_scan variants (indices written here).
static std::vector<int32_t> g_out_indices;

static void setup_uc1(const benchmark::State& state) {
    clangRuntimeSpecializer::detail::applyUCPipelineOptionsForBenchmark(state.name());
    if (!g_rows.empty()) return;
    g_rows.resize(static_cast<size_t>(N_ROWS_MAX) * ROW_STRIDE, 0);
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    for (int64_t i = 0; i < N_ROWS_MAX; ++i) {
        double vb = dist(rng);
        __builtin_memcpy(g_rows.data() + i * ROW_STRIDE + COL_OFFSET_B, &vb, sizeof(double));
        double va = dist(rng);
        __builtin_memcpy(g_rows.data() + i * ROW_STRIDE + COL_OFFSET, &va, sizeof(double));
    }
    g_out_indices.resize(N_ROWS_MAX);
}
static void teardown_uc1(const benchmark::State&) {
    clangRuntimeSpecializer::detail::resetUCPipelineOptionsAfterBenchmark();
    // Buffer stays allocated for process lifetime; freed by OS on exit.
}

// column_scan_unspecialized: same logic as column_scan in the kernel TU but defined
// here so the unspecialized benchmark does identical work to the JIT-specialized variant
// (scan + write matching indices to out).
static int64_t column_scan_unspecialized(const uint8_t* rows, int64_t n_rows,
                                          int row_stride, int col_offset,
                                          double threshold, int32_t* out) {
    int64_t count = 0;
    for (int64_t i = 0; i < n_rows; ++i) {
        double val;
        __builtin_memcpy(&val, rows + i * row_stride + col_offset, sizeof(double));
        if (val > threshold)
            out[count++] = static_cast<int32_t>(i);
    }
    return count;
}

// multi_predicate_count_unspecialized: AND of two column predicates, matching the
// semantics of multi_predicate_count in the kernel TU.
static int64_t multi_predicate_count_unspecialized(const uint8_t* rows, int64_t n_rows,
                                                    int row_stride,
                                                    int col_offset_a, int col_offset_b,
                                                    double threshold_a, double threshold_b) {
    int64_t count = 0;
    for (int64_t i = 0; i < n_rows; ++i) {
        double va, vb;
        __builtin_memcpy(&va, rows + i * row_stride + col_offset_a, sizeof(double));
        __builtin_memcpy(&vb, rows + i * row_stride + col_offset_b, sizeof(double));
        if (va > threshold_a && vb > threshold_b)
            ++count;
    }
    return count;
}

// ============================================================================
// count_matching_rows / low
// ============================================================================

static void BM_UC1_count_matching_rows_low_unspecialized(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            count_matching_rows(g_rows.data(), n_rows, ROW_STRIDE, COL_OFFSET, 0.5));
    }
}

static void BM_UC1_count_matching_rows_low_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_sql_specialized(ROW_STRIDE, COL_OFFSET, 0.5));
    }
}

static void BM_UC1_count_matching_rows_low_specialized_exec(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    auto spec = create_sql_specialized(ROW_STRIDE, COL_OFFSET, 0.5);
    for (auto _ : state) {
        benchmark::DoNotOptimize(spec(g_rows.data(), n_rows));
    }
}

// ============================================================================
// count_matching_rows / tradeoff
// ============================================================================

static void BM_UC1_count_matching_rows_tradeoff_unspecialized(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            count_matching_rows_tradeoff_unspecialized(g_rows.data(), n_rows, COL_OFFSET, ROW_STRIDE, 0.5));
    }
}

static void BM_UC1_count_matching_rows_tradeoff_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_count_matching_rows_tradeoff_specialized(COL_OFFSET, ROW_STRIDE, 0.5));
    }
}

static void BM_UC1_count_matching_rows_tradeoff_specialized_exec(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    auto spec = create_count_matching_rows_tradeoff_specialized(COL_OFFSET, ROW_STRIDE, 0.5);
    for (auto _ : state) {
        benchmark::DoNotOptimize(spec(g_rows.data(), n_rows));
    }
}

// ============================================================================
// count_matching_rows / abstract
// ============================================================================

static void BM_UC1_count_matching_rows_abstract_unspecialized(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            count_matching_rows_abstract_unspecialized(g_rows.data(), n_rows, COL_OFFSET, ROW_STRIDE, 0.5));
    }
}

static void BM_UC1_count_matching_rows_abstract_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_count_matching_rows_abstract_specialized(COL_OFFSET, ROW_STRIDE, 0.5));
    }
}

static void BM_UC1_count_matching_rows_abstract_specialized_exec(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    auto spec = create_count_matching_rows_abstract_specialized(COL_OFFSET, ROW_STRIDE, 0.5);
    for (auto _ : state) {
        benchmark::DoNotOptimize(spec(g_rows.data(), n_rows));
    }
}

// ============================================================================
// multi_predicate / low
// ============================================================================

static void BM_UC1_multi_predicate_low_unspecialized(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            multi_predicate_count_unspecialized(g_rows.data(), n_rows,
                ROW_STRIDE, COL_OFFSET, COL_OFFSET_B, 0.5, 0.5));
    }
}

static void BM_UC1_multi_predicate_low_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_multi_predicate_low_specialized(ROW_STRIDE, COL_OFFSET, COL_OFFSET_B, 0.5, 0.5));
    }
}

static void BM_UC1_multi_predicate_low_specialized_exec(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    auto spec = create_multi_predicate_low_specialized(ROW_STRIDE, COL_OFFSET, COL_OFFSET_B, 0.5, 0.5);
    for (auto _ : state) {
        benchmark::DoNotOptimize(spec(g_rows.data(), n_rows));
    }
}

// ============================================================================
// multi_predicate / tradeoff
// ============================================================================

static void BM_UC1_multi_predicate_tradeoff_unspecialized(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            multi_predicate_tradeoff_unspecialized(g_rows.data(), n_rows,
                ROW_STRIDE, COL_OFFSET, COL_OFFSET_B, 0.5, 0.5));
    }
}

static void BM_UC1_multi_predicate_tradeoff_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_multi_predicate_tradeoff_specialized(ROW_STRIDE, COL_OFFSET, COL_OFFSET_B, 0.5, 0.5));
    }
}

static void BM_UC1_multi_predicate_tradeoff_specialized_exec(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    auto spec = create_multi_predicate_tradeoff_specialized(ROW_STRIDE, COL_OFFSET, COL_OFFSET_B, 0.5, 0.5);
    for (auto _ : state) {
        benchmark::DoNotOptimize(spec(g_rows.data(), n_rows));
    }
}

// ============================================================================
// multi_predicate / abstract
// ============================================================================

static void BM_UC1_multi_predicate_abstract_unspecialized(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            multi_predicate_abstract_unspecialized(g_rows.data(), n_rows,
                ROW_STRIDE, COL_OFFSET, COL_OFFSET_B, 0.5, 0.5));
    }
}

static void BM_UC1_multi_predicate_abstract_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_multi_predicate_abstract_specialized(ROW_STRIDE, COL_OFFSET, COL_OFFSET_B, 0.5, 0.5));
    }
}

static void BM_UC1_multi_predicate_abstract_specialized_exec(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    auto spec = create_multi_predicate_abstract_specialized(ROW_STRIDE, COL_OFFSET, COL_OFFSET_B, 0.5, 0.5);
    for (auto _ : state) {
        benchmark::DoNotOptimize(spec(g_rows.data(), n_rows));
    }
}

// ============================================================================
// column_scan / low
// ============================================================================

static void BM_UC1_column_scan_low_unspecialized(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            column_scan_unspecialized(g_rows.data(), n_rows,
                ROW_STRIDE, COL_OFFSET, 0.5, g_out_indices.data()));
    }
}

static void BM_UC1_column_scan_low_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_column_scan_low_specialized(ROW_STRIDE, COL_OFFSET, 0.5));
    }
}

static void BM_UC1_column_scan_low_specialized_exec(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    auto spec = create_column_scan_low_specialized(ROW_STRIDE, COL_OFFSET, 0.5);
    for (auto _ : state) {
        benchmark::DoNotOptimize(spec(g_rows.data(), n_rows, g_out_indices.data()));
    }
}

// ============================================================================
// column_scan / tradeoff
// ============================================================================

static void BM_UC1_column_scan_tradeoff_unspecialized(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            column_scan_tradeoff_unspecialized(g_rows.data(), n_rows,
                ROW_STRIDE, COL_OFFSET, 0.5, g_out_indices.data()));
    }
}

static void BM_UC1_column_scan_tradeoff_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_column_scan_tradeoff_specialized(ROW_STRIDE, COL_OFFSET, 0.5));
    }
}

static void BM_UC1_column_scan_tradeoff_specialized_exec(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    auto spec = create_column_scan_tradeoff_specialized(ROW_STRIDE, COL_OFFSET, 0.5);
    for (auto _ : state) {
        benchmark::DoNotOptimize(spec(g_rows.data(), n_rows, g_out_indices.data()));
    }
}

// ============================================================================
// column_scan / abstract
// ============================================================================

static void BM_UC1_column_scan_abstract_unspecialized(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            column_scan_abstract_unspecialized(g_rows.data(), n_rows,
                ROW_STRIDE, COL_OFFSET, 0.5, g_out_indices.data()));
    }
}

static void BM_UC1_column_scan_abstract_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_column_scan_abstract_specialized(ROW_STRIDE, COL_OFFSET, 0.5));
    }
}

static void BM_UC1_column_scan_abstract_specialized_exec(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    auto spec = create_column_scan_abstract_specialized(ROW_STRIDE, COL_OFFSET, 0.5);
    for (auto _ : state) {
        benchmark::DoNotOptimize(spec(g_rows.data(), n_rows, g_out_indices.data()));
    }
}

// ============================================================================
// ---------------------------------------------------------------------------
// JIT analysis benchmarks
// ---------------------------------------------------------------------------

static void BM_UC1_count_matching_rows_low_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_sql_specialized(ROW_STRIDE, COL_OFFSET, 0.5);
    });
}

static void BM_UC1_count_matching_rows_tradeoff_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_count_matching_rows_tradeoff_specialized(COL_OFFSET, ROW_STRIDE, 0.5);
    });
}

static void BM_UC1_count_matching_rows_abstract_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_count_matching_rows_abstract_specialized(COL_OFFSET, ROW_STRIDE, 0.5);
    });
}

static void BM_UC1_multi_predicate_low_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_multi_predicate_low_specialized(ROW_STRIDE, COL_OFFSET, COL_OFFSET_B, 0.5, 0.5);
    });
}

static void BM_UC1_multi_predicate_tradeoff_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_multi_predicate_tradeoff_specialized(ROW_STRIDE, COL_OFFSET, COL_OFFSET_B, 0.5, 0.5);
    });
}

static void BM_UC1_multi_predicate_abstract_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_multi_predicate_abstract_specialized(ROW_STRIDE, COL_OFFSET, COL_OFFSET_B, 0.5, 0.5);
    });
}

static void BM_UC1_column_scan_low_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_column_scan_low_specialized(ROW_STRIDE, COL_OFFSET, 0.5);
    });
}

static void BM_UC1_column_scan_tradeoff_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_column_scan_tradeoff_specialized(ROW_STRIDE, COL_OFFSET, 0.5);
    });
}

static void BM_UC1_column_scan_abstract_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_column_scan_abstract_specialized(ROW_STRIDE, COL_OFFSET, 0.5);
    });
}

// BENCHMARK macros
#define UC1_JIT_ANALYSIS_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC1_count_matching_rows_low_jit_analysis)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_jit_analysis)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_jit_analysis)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_jit_analysis)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_jit_analysis)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_jit_analysis)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_jit_analysis)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_jit_analysis)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_jit_analysis)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_jit_analysis)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_jit_analysis)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_jit_analysis)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_jit_analysis)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_jit_analysis)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_jit_analysis)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_jit_analysis)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_jit_analysis)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_jit_analysis)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_jit_analysis)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_jit_analysis)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_jit_analysis)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_jit_analysis)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_jit_analysis)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_jit_analysis)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_jit_analysis)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_jit_analysis)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_jit_analysis)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_jit_analysis)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_jit_analysis)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_jit_analysis)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_jit_analysis)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_jit_analysis)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_jit_analysis)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_jit_analysis)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_jit_analysis)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_jit_analysis)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond);

// ============================================================================

// count_matching_rows / low  (original 12 macros — kept for backward compatibility)
#define UC1_BENCHMARK_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC1_count_matching_rows_low_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_low_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond);

// count_matching_rows / tradeoff
#define UC1_BENCHMARK_CMR_TRADEOFF(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_tradeoff_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond);

// count_matching_rows / abstract
#define UC1_BENCHMARK_CMR_ABSTRACT(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC1_count_matching_rows_abstract_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_unspecialized)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_jit_overhead)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_count_matching_rows_abstract_specialized_exec)->Name("BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond);

// multi_predicate / low
#define UC1_BENCHMARK_MP_LOW(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC1_multi_predicate_low_unspecialized)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_unspecialized)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_unspecialized)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_unspecialized)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_jit_overhead)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_jit_overhead)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_jit_overhead)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_jit_overhead)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_specialized_exec)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_specialized_exec)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_specialized_exec)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_low_specialized_exec)->Name("BM_g:uc1_sql;n:multi_predicate;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond);

// multi_predicate / tradeoff
#define UC1_BENCHMARK_MP_TRADEOFF(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_unspecialized)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_unspecialized)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_unspecialized)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_unspecialized)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_jit_overhead)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_jit_overhead)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_jit_overhead)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_jit_overhead)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_specialized_exec)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_specialized_exec)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_specialized_exec)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_tradeoff_specialized_exec)->Name("BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond);

// multi_predicate / abstract
#define UC1_BENCHMARK_MP_ABSTRACT(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC1_multi_predicate_abstract_unspecialized)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_unspecialized)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_unspecialized)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_unspecialized)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_jit_overhead)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_jit_overhead)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_jit_overhead)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_jit_overhead)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_specialized_exec)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_specialized_exec)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_specialized_exec)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_multi_predicate_abstract_specialized_exec)->Name("BM_g:uc1_sql;n:multi_predicate;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond);

// column_scan / low
#define UC1_BENCHMARK_CS_LOW(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC1_column_scan_low_unspecialized)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_unspecialized)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_unspecialized)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_unspecialized)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_jit_overhead)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_jit_overhead)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_jit_overhead)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_jit_overhead)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_specialized_exec)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_specialized_exec)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_specialized_exec)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_low_specialized_exec)->Name("BM_g:uc1_sql;n:column_scan;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond);

// column_scan / tradeoff
#define UC1_BENCHMARK_CS_TRADEOFF(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC1_column_scan_tradeoff_unspecialized)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_unspecialized)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_unspecialized)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_unspecialized)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_jit_overhead)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_jit_overhead)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_jit_overhead)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_jit_overhead)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_specialized_exec)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_specialized_exec)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_specialized_exec)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_tradeoff_specialized_exec)->Name("BM_g:uc1_sql;n:column_scan;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond);

// column_scan / abstract
#define UC1_BENCHMARK_CS_ABSTRACT(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC1_column_scan_abstract_unspecialized)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_unspecialized)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_unspecialized)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_unspecialized)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_jit_overhead)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_jit_overhead)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_jit_overhead)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_jit_overhead)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_specialized_exec)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_specialized_exec)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_specialized_exec)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC1_column_scan_abstract_specialized_exec)->Name("BM_g:uc1_sql;n:column_scan;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc1)->Teardown(teardown_uc1)->Unit(benchmark::kMillisecond);

// ============================================================================
// Instantiate all macros
// ============================================================================

#ifdef ALL_BENCHMARKS_BUILD
#define UC1_SIZES Arg(1'000'000), Arg(10'000'000), Arg(30'000'000), Arg(64'000'000)
UC1_BENCHMARK_SPEC(Arg(1'000'000), Arg(10'000'000), Arg(30'000'000), Arg(64'000'000))
UC1_BENCHMARK_CMR_TRADEOFF(Arg(1'000'000), Arg(10'000'000), Arg(30'000'000), Arg(64'000'000))
UC1_BENCHMARK_CMR_ABSTRACT(Arg(1'000'000), Arg(10'000'000), Arg(30'000'000), Arg(64'000'000))
UC1_BENCHMARK_MP_LOW(Arg(1'000'000), Arg(10'000'000), Arg(30'000'000), Arg(64'000'000))
UC1_BENCHMARK_MP_TRADEOFF(Arg(1'000'000), Arg(10'000'000), Arg(30'000'000), Arg(64'000'000))
UC1_BENCHMARK_MP_ABSTRACT(Arg(1'000'000), Arg(10'000'000), Arg(30'000'000), Arg(64'000'000))
UC1_BENCHMARK_CS_LOW(Arg(1'000'000), Arg(10'000'000), Arg(30'000'000), Arg(64'000'000))
UC1_BENCHMARK_CS_TRADEOFF(Arg(1'000'000), Arg(10'000'000), Arg(30'000'000), Arg(64'000'000))
UC1_BENCHMARK_CS_ABSTRACT(Arg(1'000'000), Arg(10'000'000), Arg(30'000'000), Arg(64'000'000))
UC1_JIT_ANALYSIS_SPEC(Arg(1'000'000), Arg(10'000'000), Arg(30'000'000), Arg(64'000'000))
#else
// Single-call batch: SMALL=~1ms, MEDIUM=~10ms; LARGE/EXTRALARGE cap at buffer.
UC1_BENCHMARK_SPEC(Arg(6'400'000LL), Arg(64'000'000LL), Arg(64'000'000LL), Arg(64'000'000LL))
UC1_BENCHMARK_CMR_TRADEOFF(Arg(6'400'000LL), Arg(64'000'000LL), Arg(64'000'000LL), Arg(64'000'000LL))
UC1_BENCHMARK_CMR_ABSTRACT(Arg(6'400'000LL), Arg(64'000'000LL), Arg(64'000'000LL), Arg(64'000'000LL))
UC1_BENCHMARK_MP_LOW(Arg(6'400'000LL), Arg(64'000'000LL), Arg(64'000'000LL), Arg(64'000'000LL))
UC1_BENCHMARK_MP_TRADEOFF(Arg(6'400'000LL), Arg(64'000'000LL), Arg(64'000'000LL), Arg(64'000'000LL))
UC1_BENCHMARK_MP_ABSTRACT(Arg(6'400'000LL), Arg(64'000'000LL), Arg(64'000'000LL), Arg(64'000'000LL))
UC1_BENCHMARK_CS_LOW(Arg(6'400'000LL), Arg(64'000'000LL), Arg(64'000'000LL), Arg(64'000'000LL))
UC1_BENCHMARK_CS_TRADEOFF(Arg(6'400'000LL), Arg(64'000'000LL), Arg(64'000'000LL), Arg(64'000'000LL))
UC1_BENCHMARK_CS_ABSTRACT(Arg(6'400'000LL), Arg(64'000'000LL), Arg(64'000'000LL), Arg(64'000'000LL))
UC1_JIT_ANALYSIS_SPEC(Arg(6'400'000LL), Arg(64'000'000LL), Arg(64'000'000LL), Arg(64'000'000LL))
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
