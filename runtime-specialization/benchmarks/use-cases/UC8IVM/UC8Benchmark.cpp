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
// Buffer = 1 GB / 24 B/row = 42 M rows.  Each benchmark calls the kernel
// exactly once per iteration with min(state.range(0), N_ROWS_MAX) rows.
static constexpr int64_t N_ROWS_MAX    = 42'000'000;

// Global dataset: N_ROWS_MAX rows of ROW_STRIDE bytes.
// Benchmarks pass size-specific n_rows via state.range(0).
static std::vector<uint8_t> g_deltas;

static std::vector<double> g_buckets(N_BUCKETS, 0.0);
static std::vector<double> g_count_buckets(N_BUCKETS, 0.0);

static void setup_uc8(const benchmark::State& state) {
    clangRuntimeSpecializer::detail::applyUCPipelineOptionsForBenchmark(state.name());
    if (!g_deltas.empty()) return;
    g_deltas.resize(static_cast<size_t>(N_ROWS_MAX) * ROW_STRIDE, 0);
    std::mt19937 rng(42);
    std::uniform_int_distribution<int32_t> gk_dist(0, N_BUCKETS - 1);
    std::uniform_real_distribution<double>  val_dist(0.0, 100.0);
    std::uniform_int_distribution<uint8_t>  byte_dist(0, 255);
    for (int64_t i = 0; i < N_ROWS_MAX; ++i) {
        uint8_t* row = g_deltas.data() + static_cast<size_t>(i) * ROW_STRIDE;
        for (int b = 0; b < GROUP_COL; ++b)
            row[b] = byte_dist(rng);
        int32_t gk = gk_dist(rng);
        __builtin_memcpy(row + GROUP_COL, &gk, sizeof(int32_t));
        double val = val_dist(rng);
        __builtin_memcpy(row + VALUE_COL, &val, sizeof(double));
        for (int b = VALUE_COL + (int)sizeof(double); b < ROW_STRIDE; ++b)
            row[b] = byte_dist(rng);
    }
}
static void teardown_uc8(const benchmark::State&) {
    clangRuntimeSpecializer::detail::resetUCPipelineOptionsAfterBenchmark();
    // Buffer stays allocated for process lifetime; freed by OS on exit.
}

// ============================================================================
// apply_row_delta — low (batch: single call per iteration)
// ============================================================================

static void BM_UC8_unspecialized(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        apply_row_delta_batch(g_deltas.data(), n_rows, g_buckets.data(),
                              N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
        benchmark::DoNotOptimize(g_buckets.data());
    }
}

static void BM_UC8_jit_overhead(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_apply_row_delta_batch_low_specialized(
                n_rows, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE));
    }
}

static void BM_UC8_specialized_exec(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    auto spec = create_apply_row_delta_batch_low_specialized(
        n_rows, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        spec(g_deltas.data(), g_buckets.data());
        benchmark::DoNotOptimize(g_buckets.data());
    }
}

// ============================================================================
// apply_row_delta — tradeoff (batch)
// ============================================================================

static void BM_UC8_apply_row_delta_tradeoff_jit(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_apply_row_delta_batch_tradeoff_specialized(
                n_rows, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE));
    }
}

static void BM_UC8_apply_row_delta_tradeoff_exec(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    auto spec = create_apply_row_delta_batch_tradeoff_specialized(
        n_rows, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        spec(g_deltas.data(), g_buckets.data());
        benchmark::DoNotOptimize(g_buckets.data());
    }
}

// ============================================================================
// apply_row_delta — abstract (batch)
// ============================================================================

static void BM_UC8_apply_row_delta_abstract_jit(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_apply_row_delta_batch_abstract_specialized(
                n_rows, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE));
    }
}

static void BM_UC8_apply_row_delta_abstract_exec(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    auto spec = create_apply_row_delta_batch_abstract_specialized(
        n_rows, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        spec(g_deltas.data(), g_buckets.data());
        benchmark::DoNotOptimize(g_buckets.data());
    }
}

// ============================================================================
// apply_row_delta — tradeoff unspecialized (batch)
// ============================================================================

static void BM_UC8_apply_row_delta_tradeoff_unspecialized(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        apply_row_delta_batch_tradeoff_unspecialized(g_deltas.data(), n_rows,
            g_buckets.data(), N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
        benchmark::DoNotOptimize(g_buckets.data());
    }
}

// ============================================================================
// apply_row_delta — abstract unspecialized (batch)
// ============================================================================

static void BM_UC8_apply_row_delta_abstract_unspecialized(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        apply_row_delta_batch_abstract_unspecialized(g_deltas.data(), n_rows,
            g_buckets.data(), N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
        benchmark::DoNotOptimize(g_buckets.data());
    }
}

// ============================================================================
// multi_agg_delta — unspecialized (batch: single call per iteration)
// ============================================================================

static void BM_UC8_multi_agg_delta_unspecialized(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        std::fill(g_count_buckets.begin(), g_count_buckets.end(), 0.0);
        multi_agg_delta_batch(g_deltas.data(), n_rows,
                              g_buckets.data(), g_count_buckets.data(),
                              N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
        benchmark::DoNotOptimize(g_buckets.data());
        benchmark::DoNotOptimize(g_count_buckets.data());
    }
}

// ============================================================================
// multi_agg_delta — tradeoff unspecialized (batch)
// ============================================================================

static void BM_UC8_multi_agg_delta_tradeoff_unspecialized(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        std::fill(g_count_buckets.begin(), g_count_buckets.end(), 0.0);
        multi_agg_delta_batch_tradeoff_unspecialized(g_deltas.data(), n_rows,
            g_buckets.data(), g_count_buckets.data(),
            N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
        benchmark::DoNotOptimize(g_buckets.data());
        benchmark::DoNotOptimize(g_count_buckets.data());
    }
}

// ============================================================================
// multi_agg_delta — abstract unspecialized (batch)
// ============================================================================

static void BM_UC8_multi_agg_delta_abstract_unspecialized(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        std::fill(g_count_buckets.begin(), g_count_buckets.end(), 0.0);
        multi_agg_delta_batch_abstract_unspecialized(g_deltas.data(), n_rows,
            g_buckets.data(), g_count_buckets.data(),
            N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
        benchmark::DoNotOptimize(g_buckets.data());
        benchmark::DoNotOptimize(g_count_buckets.data());
    }
}

// ============================================================================
// multi_agg_delta — low (batch)
// ============================================================================

static void BM_UC8_multi_agg_delta_low_jit(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_multi_agg_delta_batch_low_specialized(
                n_rows, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE));
    }
}

static void BM_UC8_multi_agg_delta_low_exec(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    auto spec = create_multi_agg_delta_batch_low_specialized(
        n_rows, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        std::fill(g_count_buckets.begin(), g_count_buckets.end(), 0.0);
        spec(g_deltas.data(), g_buckets.data(), g_count_buckets.data());
        benchmark::DoNotOptimize(g_buckets.data());
        benchmark::DoNotOptimize(g_count_buckets.data());
    }
}

// ============================================================================
// multi_agg_delta — tradeoff (batch)
// ============================================================================

static void BM_UC8_multi_agg_delta_tradeoff_jit(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_multi_agg_delta_batch_tradeoff_specialized(
                n_rows, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE));
    }
}

static void BM_UC8_multi_agg_delta_tradeoff_exec(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    auto spec = create_multi_agg_delta_batch_tradeoff_specialized(
        n_rows, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        std::fill(g_count_buckets.begin(), g_count_buckets.end(), 0.0);
        spec(g_deltas.data(), g_buckets.data(), g_count_buckets.data());
        benchmark::DoNotOptimize(g_buckets.data());
        benchmark::DoNotOptimize(g_count_buckets.data());
    }
}

// ============================================================================
// multi_agg_delta — abstract (batch)
// ============================================================================

static void BM_UC8_multi_agg_delta_abstract_jit(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_multi_agg_delta_batch_abstract_specialized(
                n_rows, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE));
    }
}

static void BM_UC8_multi_agg_delta_abstract_exec(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    auto spec = create_multi_agg_delta_batch_abstract_specialized(
        n_rows, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        std::fill(g_count_buckets.begin(), g_count_buckets.end(), 0.0);
        spec(g_deltas.data(), g_buckets.data(), g_count_buckets.data());
        benchmark::DoNotOptimize(g_buckets.data());
        benchmark::DoNotOptimize(g_count_buckets.data());
    }
}

// ============================================================================
// batch_delta — unspecialized (single call per iteration)
// ============================================================================

static void BM_UC8_batch_delta_unspecialized(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        apply_row_delta_batch(g_deltas.data(), n_rows, g_buckets.data(),
                              N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
        benchmark::DoNotOptimize(g_buckets.data());
    }
}

// ============================================================================
// batch_delta — tradeoff unspecialized (single call per iteration)
// ============================================================================

static void BM_UC8_batch_delta_tradeoff_unspecialized(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        batch_delta_tradeoff_unspecialized(g_deltas.data(), n_rows,
            g_buckets.data(), N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
        benchmark::DoNotOptimize(g_buckets.data());
    }
}

// ============================================================================
// batch_delta — abstract unspecialized (single call per iteration)
// ============================================================================

static void BM_UC8_batch_delta_abstract_unspecialized(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        batch_delta_abstract_unspecialized(g_deltas.data(), n_rows,
            g_buckets.data(), N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
        benchmark::DoNotOptimize(g_buckets.data());
    }
}

// ============================================================================
// batch_delta — low (single call per iteration)
// ============================================================================

static void BM_UC8_batch_delta_low_jit(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_batch_delta_low_specialized(n_rows, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE));
    }
}

static void BM_UC8_batch_delta_low_exec(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    auto spec = create_batch_delta_low_specialized(n_rows, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        spec(g_deltas.data(), g_buckets.data());
        benchmark::DoNotOptimize(g_buckets.data());
    }
}

// ============================================================================
// batch_delta — tradeoff (single call per iteration)
// ============================================================================

static void BM_UC8_batch_delta_tradeoff_jit(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_batch_delta_tradeoff_specialized(n_rows, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE));
    }
}

static void BM_UC8_batch_delta_tradeoff_exec(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    auto spec = create_batch_delta_tradeoff_specialized(n_rows, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        spec(g_deltas.data(), g_buckets.data());
        benchmark::DoNotOptimize(g_buckets.data());
    }
}

// ============================================================================
// batch_delta — abstract (single call per iteration)
// ============================================================================

static void BM_UC8_batch_delta_abstract_jit(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_batch_delta_abstract_specialized(n_rows, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE));
    }
}

static void BM_UC8_batch_delta_abstract_exec(benchmark::State& state) {
    int64_t n_rows = std::min(state.range(0), (int64_t)N_ROWS_MAX);
    auto spec = create_batch_delta_abstract_specialized(n_rows, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        spec(g_deltas.data(), g_buckets.data());
        benchmark::DoNotOptimize(g_buckets.data());
    }
}

// ============================================================================
// ---------------------------------------------------------------------------
// JIT analysis benchmarks
// ---------------------------------------------------------------------------

static void BM_UC8_apply_row_delta_low_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_apply_row_delta_batch_low_specialized(
            N_ROWS_MAX, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    });
}

static void BM_UC8_apply_row_delta_tradeoff_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_apply_row_delta_batch_tradeoff_specialized(
            N_ROWS_MAX, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    });
}

static void BM_UC8_apply_row_delta_abstract_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_apply_row_delta_batch_abstract_specialized(
            N_ROWS_MAX, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    });
}

static void BM_UC8_multi_agg_delta_low_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_multi_agg_delta_batch_low_specialized(
            N_ROWS_MAX, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    });
}

static void BM_UC8_multi_agg_delta_tradeoff_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_multi_agg_delta_batch_tradeoff_specialized(
            N_ROWS_MAX, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    });
}

static void BM_UC8_multi_agg_delta_abstract_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_multi_agg_delta_batch_abstract_specialized(
            N_ROWS_MAX, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    });
}

static void BM_UC8_batch_delta_low_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_batch_delta_low_specialized(N_ROWS_MAX, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    });
}

static void BM_UC8_batch_delta_tradeoff_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_batch_delta_tradeoff_specialized(N_ROWS_MAX, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    });
}

static void BM_UC8_batch_delta_abstract_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_batch_delta_abstract_specialized(N_ROWS_MAX, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    });
}

// Registration macros
#define UC8_JIT_ANALYSIS_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC8_apply_row_delta_low_jit_analysis)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_low_jit_analysis)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_low_jit_analysis)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_low_jit_analysis)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_tradeoff_jit_analysis)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_tradeoff_jit_analysis)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_tradeoff_jit_analysis)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_tradeoff_jit_analysis)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_abstract_jit_analysis)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_abstract_jit_analysis)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_abstract_jit_analysis)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_abstract_jit_analysis)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_low_jit_analysis)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_low_jit_analysis)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_low_jit_analysis)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_low_jit_analysis)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_tradeoff_jit_analysis)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_tradeoff_jit_analysis)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_tradeoff_jit_analysis)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_tradeoff_jit_analysis)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_abstract_jit_analysis)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_abstract_jit_analysis)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_abstract_jit_analysis)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_abstract_jit_analysis)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_low_jit_analysis)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_low_jit_analysis)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_low_jit_analysis)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_low_jit_analysis)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_tradeoff_jit_analysis)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_tradeoff_jit_analysis)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_tradeoff_jit_analysis)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_tradeoff_jit_analysis)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_abstract_jit_analysis)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_abstract_jit_analysis)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_abstract_jit_analysis)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_abstract_jit_analysis)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond);

// ============================================================================

#define UC8_BENCHMARK_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC8_unspecialized)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_unspecialized)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_unspecialized)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_unspecialized)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_jit_overhead)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_jit_overhead)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_jit_overhead)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_jit_overhead)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_specialized_exec)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_specialized_exec)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_specialized_exec)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_specialized_exec)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
\
BENCHMARK(BM_UC8_apply_row_delta_tradeoff_unspecialized)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_tradeoff_unspecialized)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_tradeoff_unspecialized)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_tradeoff_unspecialized)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_tradeoff_jit)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_tradeoff_jit)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_tradeoff_jit)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_tradeoff_jit)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_tradeoff_exec)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_tradeoff_exec)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_tradeoff_exec)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_tradeoff_exec)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
\
BENCHMARK(BM_UC8_apply_row_delta_abstract_unspecialized)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_abstract_unspecialized)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_abstract_unspecialized)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_abstract_unspecialized)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_abstract_jit)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_abstract_jit)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_abstract_jit)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_abstract_jit)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_abstract_exec)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_abstract_exec)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_abstract_exec)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_abstract_exec)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
\
BENCHMARK(BM_UC8_multi_agg_delta_unspecialized)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_unspecialized)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_unspecialized)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_unspecialized)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_low_jit)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_low_jit)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_low_jit)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_low_jit)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_low_exec)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_low_exec)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_low_exec)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_low_exec)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
\
BENCHMARK(BM_UC8_multi_agg_delta_tradeoff_unspecialized)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_tradeoff_unspecialized)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_tradeoff_unspecialized)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_tradeoff_unspecialized)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_tradeoff_jit)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_tradeoff_jit)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_tradeoff_jit)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_tradeoff_jit)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_tradeoff_exec)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_tradeoff_exec)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_tradeoff_exec)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_tradeoff_exec)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
\
BENCHMARK(BM_UC8_multi_agg_delta_abstract_unspecialized)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_abstract_unspecialized)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_abstract_unspecialized)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_abstract_unspecialized)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_abstract_jit)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_abstract_jit)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_abstract_jit)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_abstract_jit)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_abstract_exec)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_abstract_exec)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_abstract_exec)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_abstract_exec)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
\
BENCHMARK(BM_UC8_batch_delta_unspecialized)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_unspecialized)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_unspecialized)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_unspecialized)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_low_jit)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_low_jit)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_low_jit)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_low_jit)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_low_exec)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_low_exec)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_low_exec)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_low_exec)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
\
BENCHMARK(BM_UC8_batch_delta_tradeoff_unspecialized)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_tradeoff_unspecialized)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_tradeoff_unspecialized)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_tradeoff_unspecialized)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_tradeoff_jit)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_tradeoff_jit)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_tradeoff_jit)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_tradeoff_jit)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_tradeoff_exec)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_tradeoff_exec)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_tradeoff_exec)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_tradeoff_exec)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
\
BENCHMARK(BM_UC8_batch_delta_abstract_unspecialized)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_abstract_unspecialized)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_abstract_unspecialized)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_abstract_unspecialized)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_abstract_jit)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_abstract_jit)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_abstract_jit)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_abstract_jit)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_abstract_exec)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_abstract_exec)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_abstract_exec)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_abstract_exec)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc8)->Teardown(teardown_uc8)->Unit(benchmark::kMillisecond);

#ifdef ALL_BENCHMARKS_BUILD
UC8_BENCHMARK_SPEC(
    Arg(1'000'000),
    Arg(10'000'000),
    Arg(30'000'000),
    Arg(42'000'000)
)
UC8_JIT_ANALYSIS_SPEC(Arg(1'000'000), Arg(10'000'000), Arg(30'000'000), Arg(42'000'000))
#else
// Single-call batch: SMALL=~1ms, MEDIUM=~10ms; LARGE/EXTRALARGE cap at buffer.
UC8_BENCHMARK_SPEC(
    Arg(4'200'000LL),
    Arg(42'000'000LL),
    Arg(42'000'000LL),
    Arg(42'000'000LL)
)
UC8_JIT_ANALYSIS_SPEC(Arg(4'200'000LL), Arg(42'000'000LL), Arg(42'000'000LL), Arg(42'000'000LL))
#endif

#ifndef ALL_BENCHMARKS_BUILD
int main(int argc, char** argv) {
    validate_apply_row_delta_batch_low_specialized(100, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    validate_apply_row_delta_batch_tradeoff_specialized(100, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    validate_apply_row_delta_batch_abstract_specialized(100, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    validate_multi_agg_delta_batch_low_specialized(100, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    validate_multi_agg_delta_batch_tradeoff_specialized(100, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    validate_multi_agg_delta_batch_abstract_specialized(100, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    validate_batch_delta_low_specialized(100, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    validate_batch_delta_tradeoff_specialized(100, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    validate_batch_delta_abstract_specialized(100, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);

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
