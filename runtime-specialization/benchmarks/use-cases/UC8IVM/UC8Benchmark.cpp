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
// Fixed buffer: 50M rows × 24 B = 1200 MB.  For sizes larger than N_ROWS_MAX
// the benchmarks loop through this buffer multiple times (streaming pattern).
static constexpr int64_t N_ROWS_MAX    = 50'000'000;

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
static std::vector<double> g_count_buckets(N_BUCKETS, 0.0);

// ============================================================================
// apply_row_delta — low
// ============================================================================

static void BM_UC8_unspecialized(benchmark::State& state) {
    int64_t n_total = state.range(0);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX) {
            int64_t batch = std::min(rem, (int64_t)N_ROWS_MAX);
            for (int64_t i = 0; i < batch; ++i) {
                apply_row_delta(g_deltas.data() + static_cast<size_t>(i) * ROW_STRIDE,
                                g_buckets.data(),
                                N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
            }
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
    int64_t n_total = state.range(0);
    auto spec = create_ivm_specialized(N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX) {
            int64_t batch = std::min(rem, (int64_t)N_ROWS_MAX);
            for (int64_t i = 0; i < batch; ++i) {
                spec(g_deltas.data() + static_cast<size_t>(i) * ROW_STRIDE,
                     g_buckets.data());
            }
        }
        benchmark::DoNotOptimize(g_buckets.data());
    }
}

// ============================================================================
// apply_row_delta — tradeoff
// ============================================================================

static void BM_UC8_apply_row_delta_tradeoff_jit(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_apply_row_delta_tradeoff_specialized(N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE));
    }
}

static void BM_UC8_apply_row_delta_tradeoff_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_apply_row_delta_tradeoff_specialized(N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX) {
            int64_t batch = std::min(rem, (int64_t)N_ROWS_MAX);
            for (int64_t i = 0; i < batch; ++i)
                spec(g_deltas.data() + static_cast<size_t>(i) * ROW_STRIDE, g_buckets.data());
        }
        benchmark::DoNotOptimize(g_buckets.data());
    }
}

// ============================================================================
// apply_row_delta — abstract
// ============================================================================

static void BM_UC8_apply_row_delta_abstract_jit(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_apply_row_delta_abstract_specialized(N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE));
    }
}

static void BM_UC8_apply_row_delta_abstract_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_apply_row_delta_abstract_specialized(N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX) {
            int64_t batch = std::min(rem, (int64_t)N_ROWS_MAX);
            for (int64_t i = 0; i < batch; ++i)
                spec(g_deltas.data() + static_cast<size_t>(i) * ROW_STRIDE, g_buckets.data());
        }
        benchmark::DoNotOptimize(g_buckets.data());
    }
}

// ============================================================================
// multi_agg_delta — unspecialized helper
// ============================================================================

static void run_multi_agg_unspecialized(int64_t n_total) {
    std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
    std::fill(g_count_buckets.begin(), g_count_buckets.end(), 0.0);
    for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX) {
        int64_t batch = std::min(rem, (int64_t)N_ROWS_MAX);
        for (int64_t i = 0; i < batch; ++i) {
            const uint8_t* row = g_deltas.data() + static_cast<size_t>(i) * ROW_STRIDE;
            int32_t gk;
            __builtin_memcpy(&gk, row + GROUP_COL, sizeof(int32_t));
            int bucket = ((gk % N_BUCKETS) + N_BUCKETS) % N_BUCKETS;
            double val;
            __builtin_memcpy(&val, row + VALUE_COL, sizeof(double));
            g_buckets[bucket]       += val;
            g_count_buckets[bucket] += 1.0;
        }
    }
}

static void BM_UC8_multi_agg_delta_unspecialized(benchmark::State& state) {
    int64_t n_total = state.range(0);
    for (auto _ : state) {
        run_multi_agg_unspecialized(n_total);
        benchmark::DoNotOptimize(g_buckets.data());
        benchmark::DoNotOptimize(g_count_buckets.data());
    }
}

// ============================================================================
// multi_agg_delta — low
// ============================================================================

static void BM_UC8_multi_agg_delta_low_jit(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_multi_agg_delta_low_specialized(N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE));
    }
}

static void BM_UC8_multi_agg_delta_low_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_multi_agg_delta_low_specialized(N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        std::fill(g_count_buckets.begin(), g_count_buckets.end(), 0.0);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX) {
            int64_t batch = std::min(rem, (int64_t)N_ROWS_MAX);
            for (int64_t i = 0; i < batch; ++i)
                spec(g_deltas.data() + static_cast<size_t>(i) * ROW_STRIDE,
                     g_buckets.data(), g_count_buckets.data());
        }
        benchmark::DoNotOptimize(g_buckets.data());
        benchmark::DoNotOptimize(g_count_buckets.data());
    }
}

// ============================================================================
// multi_agg_delta — tradeoff
// ============================================================================

static void BM_UC8_multi_agg_delta_tradeoff_jit(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_multi_agg_delta_tradeoff_specialized(N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE));
    }
}

static void BM_UC8_multi_agg_delta_tradeoff_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_multi_agg_delta_tradeoff_specialized(N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        std::fill(g_count_buckets.begin(), g_count_buckets.end(), 0.0);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX) {
            int64_t batch = std::min(rem, (int64_t)N_ROWS_MAX);
            for (int64_t i = 0; i < batch; ++i)
                spec(g_deltas.data() + static_cast<size_t>(i) * ROW_STRIDE,
                     g_buckets.data(), g_count_buckets.data());
        }
        benchmark::DoNotOptimize(g_buckets.data());
        benchmark::DoNotOptimize(g_count_buckets.data());
    }
}

// ============================================================================
// multi_agg_delta — abstract
// ============================================================================

static void BM_UC8_multi_agg_delta_abstract_jit(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_multi_agg_delta_abstract_specialized(N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE));
    }
}

static void BM_UC8_multi_agg_delta_abstract_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_multi_agg_delta_abstract_specialized(N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        std::fill(g_count_buckets.begin(), g_count_buckets.end(), 0.0);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX) {
            int64_t batch = std::min(rem, (int64_t)N_ROWS_MAX);
            for (int64_t i = 0; i < batch; ++i)
                spec(g_deltas.data() + static_cast<size_t>(i) * ROW_STRIDE,
                     g_buckets.data(), g_count_buckets.data());
        }
        benchmark::DoNotOptimize(g_buckets.data());
        benchmark::DoNotOptimize(g_count_buckets.data());
    }
}

// ============================================================================
// batch_delta — unspecialized
// ============================================================================

static void BM_UC8_batch_delta_unspecialized(benchmark::State& state) {
    int64_t n_total = state.range(0);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX) {
            int64_t batch = std::min(rem, (int64_t)N_ROWS_MAX);
            for (int64_t i = 0; i < batch; ++i) {
                const uint8_t* row = g_deltas.data() + static_cast<size_t>(i) * ROW_STRIDE;
                int32_t gk;
                __builtin_memcpy(&gk, row + GROUP_COL, sizeof(int32_t));
                int bucket = ((gk % N_BUCKETS) + N_BUCKETS) % N_BUCKETS;
                double val;
                __builtin_memcpy(&val, row + VALUE_COL, sizeof(double));
                g_buckets[bucket] += val;
            }
        }
        benchmark::DoNotOptimize(g_buckets.data());
    }
}

// ============================================================================
// batch_delta — low
// ============================================================================

static void BM_UC8_batch_delta_low_jit(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_batch_delta_low_specialized(N_ROWS_MAX, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE));
    }
}

static void BM_UC8_batch_delta_low_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_batch_delta_low_specialized(N_ROWS_MAX, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX)
            spec(g_deltas.data(), g_buckets.data());
        benchmark::DoNotOptimize(g_buckets.data());
    }
}

// ============================================================================
// batch_delta — tradeoff
// ============================================================================

static void BM_UC8_batch_delta_tradeoff_jit(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_batch_delta_tradeoff_specialized(N_ROWS_MAX, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE));
    }
}

static void BM_UC8_batch_delta_tradeoff_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_batch_delta_tradeoff_specialized(N_ROWS_MAX, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX)
            spec(g_deltas.data(), g_buckets.data());
        benchmark::DoNotOptimize(g_buckets.data());
    }
}

// ============================================================================
// batch_delta — abstract
// ============================================================================

static void BM_UC8_batch_delta_abstract_jit(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_batch_delta_abstract_specialized(N_ROWS_MAX, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE));
    }
}

static void BM_UC8_batch_delta_abstract_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_batch_delta_abstract_specialized(N_ROWS_MAX, N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    for (auto _ : state) {
        std::fill(g_buckets.begin(), g_buckets.end(), 0.0);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX)
            spec(g_deltas.data(), g_buckets.data());
        benchmark::DoNotOptimize(g_buckets.data());
    }
}

// ============================================================================
// Registration macros
// ============================================================================

#define UC8_BENCHMARK_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC8_unspecialized)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_unspecialized)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_unspecialized)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_unspecialized)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_jit_overhead)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_jit_overhead)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_jit_overhead)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_jit_overhead)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_specialized_exec)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_specialized_exec)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_specialized_exec)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_specialized_exec)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
\
BENCHMARK(BM_UC8_apply_row_delta_tradeoff_jit)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_tradeoff_jit)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_tradeoff_jit)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_tradeoff_jit)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_tradeoff_exec)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_tradeoff_exec)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_tradeoff_exec)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_tradeoff_exec)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
\
BENCHMARK(BM_UC8_apply_row_delta_abstract_jit)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_abstract_jit)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_abstract_jit)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_abstract_jit)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_abstract_exec)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_abstract_exec)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_abstract_exec)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_apply_row_delta_abstract_exec)->Name("BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
\
BENCHMARK(BM_UC8_multi_agg_delta_unspecialized)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_unspecialized)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_unspecialized)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_unspecialized)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_low_jit)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_low_jit)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_low_jit)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_low_jit)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_low_exec)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_low_exec)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_low_exec)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_low_exec)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
\
BENCHMARK(BM_UC8_multi_agg_delta_unspecialized)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_unspecialized)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_unspecialized)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_unspecialized)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_tradeoff_jit)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_tradeoff_jit)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_tradeoff_jit)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_tradeoff_jit)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_tradeoff_exec)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_tradeoff_exec)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_tradeoff_exec)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_tradeoff_exec)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
\
BENCHMARK(BM_UC8_multi_agg_delta_unspecialized)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_unspecialized)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_unspecialized)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_unspecialized)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_abstract_jit)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_abstract_jit)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_abstract_jit)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_abstract_jit)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_abstract_exec)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_abstract_exec)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_abstract_exec)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_multi_agg_delta_abstract_exec)->Name("BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
\
BENCHMARK(BM_UC8_batch_delta_unspecialized)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_unspecialized)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_unspecialized)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_unspecialized)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_low_jit)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_low_jit)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_low_jit)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_low_jit)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_low_exec)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_low_exec)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_low_exec)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_low_exec)->Name("BM_g:uc8_ivm;n:batch_delta;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
\
BENCHMARK(BM_UC8_batch_delta_unspecialized)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_unspecialized)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_unspecialized)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_unspecialized)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_tradeoff_jit)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_tradeoff_jit)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_tradeoff_jit)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_tradeoff_jit)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_tradeoff_exec)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_tradeoff_exec)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_tradeoff_exec)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_tradeoff_exec)->Name("BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
\
BENCHMARK(BM_UC8_batch_delta_unspecialized)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_unspecialized)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_unspecialized)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_unspecialized)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_abstract_jit)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_abstract_jit)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_abstract_jit)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_abstract_jit)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_abstract_exec)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_abstract_exec)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_abstract_exec)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC8_batch_delta_abstract_exec)->Name("BM_g:uc8_ivm;n:batch_delta;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

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
    Arg(1'500'000'000LL),
    Arg(8'950'000'000LL)
)
#endif

#ifndef ALL_BENCHMARKS_BUILD
int main(int argc, char** argv) {
    validate_ivm_specialized(N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    validate_apply_row_delta_tradeoff_specialized(N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    validate_apply_row_delta_abstract_specialized(N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    validate_multi_agg_delta_low_specialized(N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    validate_multi_agg_delta_tradeoff_specialized(N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
    validate_multi_agg_delta_abstract_specialized(N_BUCKETS, GROUP_COL, VALUE_COL, ROW_STRIDE);
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
