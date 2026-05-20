#include "UC12Kernels.h"
#include "ClangRuntimeSpecializerBenchmark.h"
#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <algorithm>
#include <cstring>
#include <cfloat>

static constexpr int     ROW_STRIDE12  = 24;
static constexpr int     KEY_OFFSET12  = 0;
static constexpr int     VALUE_OFFSET12 = 8;
static constexpr int     N_BUCKETS12   = 1024;
// Fixed buffer: 50M rows × 24 B = 1200 MB.  For sizes larger than N_ROWS_MAX12
// the benchmarks loop through this buffer multiple times (streaming pattern).
static constexpr int64_t N_ROWS_MAX12  = 50'000'000;

// Global dataset: N_ROWS_MAX12 rows of ROW_STRIDE12 bytes.
// Benchmarks pass size-specific n_rows via state.range(0).
static std::vector<uint8_t> g_rows12;

// Output buffers for all variants (reset before each benchmark iteration).
static std::vector<double>  g_buckets12(N_BUCKETS12, 0.0);
static std::vector<int64_t> g_count_buckets(N_BUCKETS12, 0);
static std::vector<double>  g_min_buckets(N_BUCKETS12, 0.0);
static std::vector<double>  g_max_buckets(N_BUCKETS12, 0.0);

static void setup_uc12(const benchmark::State&) {
    if (!g_rows12.empty()) return;
    g_rows12.resize(N_ROWS_MAX12 * ROW_STRIDE12, 0);
    std::mt19937_64 rng(42);
    std::uniform_int_distribution<int32_t> key_dist(0, N_BUCKETS12 - 1);
    std::uniform_real_distribution<double> val_dist(0.0, 1.0);
    for (int64_t i = 0; i < N_ROWS_MAX12; ++i) {
        uint8_t* row = g_rows12.data() + i * ROW_STRIDE12;
        int32_t key = key_dist(rng);
        std::memcpy(row + KEY_OFFSET12, &key, sizeof(int32_t));
        double val = val_dist(rng);
        std::memcpy(row + VALUE_OFFSET12, &val, sizeof(double));
    }
}
static void teardown_uc12(const benchmark::State&) {
    // Buffer stays allocated for process lifetime; freed by OS on exit.
}

// ---------------------------------------------------------------------------
// grouped_sum — low tier  (original)
// ---------------------------------------------------------------------------

static void BM_UC12_unspecialized(benchmark::State& state) {
    int64_t n_total = state.range(0);
    for (auto _ : state) {
        std::fill(g_buckets12.begin(), g_buckets12.end(), 0.0);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX12)
            grouped_sum(g_rows12.data(), std::min(rem, (int64_t)N_ROWS_MAX12),
                        ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12,
                        g_buckets12.data(), N_BUCKETS12);
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
    int64_t n_total = state.range(0);
    auto spec = create_groupby_specialized(ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12);
    for (auto _ : state) {
        std::fill(g_buckets12.begin(), g_buckets12.end(), 0.0);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX12)
            spec(g_rows12.data(), std::min(rem, (int64_t)N_ROWS_MAX12), g_buckets12.data());
        benchmark::DoNotOptimize(g_buckets12.data());
    }
}

// ---------------------------------------------------------------------------
// grouped_sum — tradeoff tier
// ---------------------------------------------------------------------------

static void BM_UC12_grouped_sum_tradeoff_unspecialized(benchmark::State& state) {
    int64_t n_total = state.range(0);
    for (auto _ : state) {
        std::fill(g_buckets12.begin(), g_buckets12.end(), 0.0);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX12)
            grouped_sum(g_rows12.data(), std::min(rem, (int64_t)N_ROWS_MAX12),
                        ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12,
                        g_buckets12.data(), N_BUCKETS12);
        benchmark::DoNotOptimize(g_buckets12.data());
    }
}

static void BM_UC12_grouped_sum_tradeoff_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_grouped_sum_tradeoff_specialized(
                ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12));
    }
}

static void BM_UC12_grouped_sum_tradeoff_specialized_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_grouped_sum_tradeoff_specialized(
        ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12);
    for (auto _ : state) {
        std::fill(g_buckets12.begin(), g_buckets12.end(), 0.0);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX12)
            spec(g_rows12.data(), std::min(rem, (int64_t)N_ROWS_MAX12), g_buckets12.data());
        benchmark::DoNotOptimize(g_buckets12.data());
    }
}

// ---------------------------------------------------------------------------
// grouped_sum — abstract tier
// ---------------------------------------------------------------------------

static void BM_UC12_grouped_sum_abstract_unspecialized(benchmark::State& state) {
    int64_t n_total = state.range(0);
    for (auto _ : state) {
        std::fill(g_buckets12.begin(), g_buckets12.end(), 0.0);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX12)
            grouped_sum(g_rows12.data(), std::min(rem, (int64_t)N_ROWS_MAX12),
                        ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12,
                        g_buckets12.data(), N_BUCKETS12);
        benchmark::DoNotOptimize(g_buckets12.data());
    }
}

static void BM_UC12_grouped_sum_abstract_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_grouped_sum_abstract_specialized(
                ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12));
    }
}

static void BM_UC12_grouped_sum_abstract_specialized_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_grouped_sum_abstract_specialized(
        ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12);
    for (auto _ : state) {
        std::fill(g_buckets12.begin(), g_buckets12.end(), 0.0);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX12)
            spec(g_rows12.data(), std::min(rem, (int64_t)N_ROWS_MAX12), g_buckets12.data());
        benchmark::DoNotOptimize(g_buckets12.data());
    }
}

// ---------------------------------------------------------------------------
// grouped_count — low tier
// ---------------------------------------------------------------------------

static void BM_UC12_grouped_count_low_unspecialized(benchmark::State& state) {
    int64_t n_total = state.range(0);
    for (auto _ : state) {
        std::fill(g_count_buckets.begin(), g_count_buckets.end(), 0);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX12)
            grouped_count(g_rows12.data(), std::min(rem, (int64_t)N_ROWS_MAX12),
                          ROW_STRIDE12, KEY_OFFSET12, N_BUCKETS12,
                          g_count_buckets.data());
        benchmark::DoNotOptimize(g_count_buckets.data());
    }
}

static void BM_UC12_grouped_count_low_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_grouped_count_low_specialized(ROW_STRIDE12, KEY_OFFSET12, N_BUCKETS12));
    }
}

static void BM_UC12_grouped_count_low_specialized_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_grouped_count_low_specialized(ROW_STRIDE12, KEY_OFFSET12, N_BUCKETS12);
    for (auto _ : state) {
        std::fill(g_count_buckets.begin(), g_count_buckets.end(), 0);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX12)
            spec(g_rows12.data(), std::min(rem, (int64_t)N_ROWS_MAX12), g_count_buckets.data());
        benchmark::DoNotOptimize(g_count_buckets.data());
    }
}

// ---------------------------------------------------------------------------
// grouped_count — tradeoff tier
// ---------------------------------------------------------------------------

static void BM_UC12_grouped_count_tradeoff_unspecialized(benchmark::State& state) {
    int64_t n_total = state.range(0);
    for (auto _ : state) {
        std::fill(g_count_buckets.begin(), g_count_buckets.end(), 0);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX12)
            grouped_count(g_rows12.data(), std::min(rem, (int64_t)N_ROWS_MAX12),
                          ROW_STRIDE12, KEY_OFFSET12, N_BUCKETS12,
                          g_count_buckets.data());
        benchmark::DoNotOptimize(g_count_buckets.data());
    }
}

static void BM_UC12_grouped_count_tradeoff_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_grouped_count_tradeoff_specialized(ROW_STRIDE12, KEY_OFFSET12, N_BUCKETS12));
    }
}

static void BM_UC12_grouped_count_tradeoff_specialized_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_grouped_count_tradeoff_specialized(
        ROW_STRIDE12, KEY_OFFSET12, N_BUCKETS12);
    for (auto _ : state) {
        std::fill(g_count_buckets.begin(), g_count_buckets.end(), 0);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX12)
            spec(g_rows12.data(), std::min(rem, (int64_t)N_ROWS_MAX12), g_count_buckets.data());
        benchmark::DoNotOptimize(g_count_buckets.data());
    }
}

// ---------------------------------------------------------------------------
// grouped_count — abstract tier
// ---------------------------------------------------------------------------

static void BM_UC12_grouped_count_abstract_unspecialized(benchmark::State& state) {
    int64_t n_total = state.range(0);
    for (auto _ : state) {
        std::fill(g_count_buckets.begin(), g_count_buckets.end(), 0);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX12)
            grouped_count(g_rows12.data(), std::min(rem, (int64_t)N_ROWS_MAX12),
                          ROW_STRIDE12, KEY_OFFSET12, N_BUCKETS12,
                          g_count_buckets.data());
        benchmark::DoNotOptimize(g_count_buckets.data());
    }
}

static void BM_UC12_grouped_count_abstract_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_grouped_count_abstract_specialized(ROW_STRIDE12, KEY_OFFSET12, N_BUCKETS12));
    }
}

static void BM_UC12_grouped_count_abstract_specialized_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_grouped_count_abstract_specialized(
        ROW_STRIDE12, KEY_OFFSET12, N_BUCKETS12);
    for (auto _ : state) {
        std::fill(g_count_buckets.begin(), g_count_buckets.end(), 0);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX12)
            spec(g_rows12.data(), std::min(rem, (int64_t)N_ROWS_MAX12), g_count_buckets.data());
        benchmark::DoNotOptimize(g_count_buckets.data());
    }
}

// ---------------------------------------------------------------------------
// grouped_minmax — low tier
// ---------------------------------------------------------------------------

static void BM_UC12_grouped_minmax_low_unspecialized(benchmark::State& state) {
    int64_t n_total = state.range(0);
    for (auto _ : state) {
        std::fill(g_min_buckets.begin(), g_min_buckets.end(), DBL_MAX);
        std::fill(g_max_buckets.begin(), g_max_buckets.end(), -DBL_MAX);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX12)
            grouped_minmax(g_rows12.data(), std::min(rem, (int64_t)N_ROWS_MAX12),
                           ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12,
                           g_min_buckets.data(), g_max_buckets.data());
        benchmark::DoNotOptimize(g_min_buckets.data());
        benchmark::DoNotOptimize(g_max_buckets.data());
    }
}

static void BM_UC12_grouped_minmax_low_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_grouped_minmax_low_specialized(
                ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12));
    }
}

static void BM_UC12_grouped_minmax_low_specialized_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_grouped_minmax_low_specialized(
        ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12);
    for (auto _ : state) {
        std::fill(g_min_buckets.begin(), g_min_buckets.end(), DBL_MAX);
        std::fill(g_max_buckets.begin(), g_max_buckets.end(), -DBL_MAX);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX12)
            spec(g_rows12.data(), std::min(rem, (int64_t)N_ROWS_MAX12),
                 g_min_buckets.data(), g_max_buckets.data());
        benchmark::DoNotOptimize(g_min_buckets.data());
        benchmark::DoNotOptimize(g_max_buckets.data());
    }
}

// ---------------------------------------------------------------------------
// grouped_minmax — tradeoff tier
// ---------------------------------------------------------------------------

static void BM_UC12_grouped_minmax_tradeoff_unspecialized(benchmark::State& state) {
    int64_t n_total = state.range(0);
    for (auto _ : state) {
        std::fill(g_min_buckets.begin(), g_min_buckets.end(), DBL_MAX);
        std::fill(g_max_buckets.begin(), g_max_buckets.end(), -DBL_MAX);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX12)
            grouped_minmax(g_rows12.data(), std::min(rem, (int64_t)N_ROWS_MAX12),
                           ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12,
                           g_min_buckets.data(), g_max_buckets.data());
        benchmark::DoNotOptimize(g_min_buckets.data());
        benchmark::DoNotOptimize(g_max_buckets.data());
    }
}

static void BM_UC12_grouped_minmax_tradeoff_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_grouped_minmax_tradeoff_specialized(
                ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12));
    }
}

static void BM_UC12_grouped_minmax_tradeoff_specialized_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_grouped_minmax_tradeoff_specialized(
        ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12);
    for (auto _ : state) {
        std::fill(g_min_buckets.begin(), g_min_buckets.end(), DBL_MAX);
        std::fill(g_max_buckets.begin(), g_max_buckets.end(), -DBL_MAX);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX12)
            spec(g_rows12.data(), std::min(rem, (int64_t)N_ROWS_MAX12),
                 g_min_buckets.data(), g_max_buckets.data());
        benchmark::DoNotOptimize(g_min_buckets.data());
        benchmark::DoNotOptimize(g_max_buckets.data());
    }
}

// ---------------------------------------------------------------------------
// grouped_minmax — abstract tier
// ---------------------------------------------------------------------------

static void BM_UC12_grouped_minmax_abstract_unspecialized(benchmark::State& state) {
    int64_t n_total = state.range(0);
    for (auto _ : state) {
        std::fill(g_min_buckets.begin(), g_min_buckets.end(), DBL_MAX);
        std::fill(g_max_buckets.begin(), g_max_buckets.end(), -DBL_MAX);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX12)
            grouped_minmax(g_rows12.data(), std::min(rem, (int64_t)N_ROWS_MAX12),
                           ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12,
                           g_min_buckets.data(), g_max_buckets.data());
        benchmark::DoNotOptimize(g_min_buckets.data());
        benchmark::DoNotOptimize(g_max_buckets.data());
    }
}

static void BM_UC12_grouped_minmax_abstract_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_grouped_minmax_abstract_specialized(
                ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12));
    }
}

static void BM_UC12_grouped_minmax_abstract_specialized_exec(benchmark::State& state) {
    int64_t n_total = state.range(0);
    auto spec = create_grouped_minmax_abstract_specialized(
        ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12);
    for (auto _ : state) {
        std::fill(g_min_buckets.begin(), g_min_buckets.end(), DBL_MAX);
        std::fill(g_max_buckets.begin(), g_max_buckets.end(), -DBL_MAX);
        for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX12)
            spec(g_rows12.data(), std::min(rem, (int64_t)N_ROWS_MAX12),
                 g_min_buckets.data(), g_max_buckets.data());
        benchmark::DoNotOptimize(g_min_buckets.data());
        benchmark::DoNotOptimize(g_max_buckets.data());
    }
}

// ---------------------------------------------------------------------------
// Benchmark registration macros
// ---------------------------------------------------------------------------

#define UC12_BENCHMARK_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC12_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_sum;a:low;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_sum;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_sum;a:low;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_sum;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_sum;a:low;s:SMALL;t:jit_overhead;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_sum;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_sum;a:low;s:LARGE;t:jit_overhead;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_sum;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_sum;a:low;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_sum;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_sum;a:low;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_sum;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_sum_tradeoff_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_sum;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_sum_tradeoff_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_sum;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_sum_tradeoff_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_sum;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_sum_tradeoff_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_sum;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_sum_tradeoff_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_sum;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_sum_tradeoff_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_sum;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_sum_tradeoff_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_sum;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_sum_tradeoff_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_sum;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_sum_tradeoff_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_sum;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_sum_tradeoff_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_sum;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_sum_tradeoff_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_sum;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_sum_tradeoff_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_sum;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_sum_abstract_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_sum;a:abstract;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_sum_abstract_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_sum;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_sum_abstract_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_sum;a:abstract;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_sum_abstract_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_sum;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_sum_abstract_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_sum;a:abstract;s:SMALL;t:jit_overhead;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_sum_abstract_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_sum;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_sum_abstract_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_sum;a:abstract;s:LARGE;t:jit_overhead;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_sum_abstract_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_sum;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_sum_abstract_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_sum;a:abstract;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_sum_abstract_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_sum;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_sum_abstract_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_sum;a:abstract;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_sum_abstract_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_sum;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_low_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_count;a:low;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_low_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_count;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_low_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_count;a:low;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_low_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_count;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_low_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_count;a:low;s:SMALL;t:jit_overhead;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_low_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_count;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_low_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_count;a:low;s:LARGE;t:jit_overhead;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_low_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_count;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_low_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_count;a:low;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_low_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_count;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_low_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_count;a:low;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_low_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_count;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_tradeoff_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_count;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_tradeoff_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_count;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_tradeoff_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_count;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_tradeoff_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_count;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_tradeoff_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_count;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_tradeoff_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_count;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_tradeoff_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_count;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_tradeoff_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_count;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_tradeoff_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_count;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_tradeoff_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_count;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_tradeoff_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_count;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_tradeoff_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_count;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_abstract_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_count;a:abstract;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_abstract_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_count;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_abstract_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_count;a:abstract;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_abstract_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_count;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_abstract_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_count;a:abstract;s:SMALL;t:jit_overhead;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_abstract_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_count;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_abstract_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_count;a:abstract;s:LARGE;t:jit_overhead;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_abstract_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_count;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_abstract_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_count;a:abstract;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_abstract_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_count;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_abstract_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_count;a:abstract;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_count_abstract_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_count;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_low_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:low;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_low_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_low_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:low;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_low_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_low_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:low;s:SMALL;t:jit_overhead;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_low_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_low_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:low;s:LARGE;t:jit_overhead;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_low_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_low_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:low;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_low_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_low_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:low;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_low_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_tradeoff_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_tradeoff_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_tradeoff_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_tradeoff_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_tradeoff_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_tradeoff_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_tradeoff_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_tradeoff_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_tradeoff_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_tradeoff_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_tradeoff_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_tradeoff_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_abstract_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:abstract;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_abstract_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_abstract_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:abstract;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_abstract_unspecialized)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_abstract_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:abstract;s:SMALL;t:jit_overhead;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_abstract_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_abstract_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:abstract;s:LARGE;t:jit_overhead;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_abstract_jit_overhead)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_abstract_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:abstract;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_abstract_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_abstract_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:abstract;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC12_grouped_minmax_abstract_specialized_exec)->Name("BM_g:uc12_groupby;n:grouped_minmax;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc12)->Teardown(teardown_uc12)->Unit(benchmark::kMillisecond);

#ifdef ALL_BENCHMARKS_BUILD
UC12_BENCHMARK_SPEC(
    Arg(1'000'000),
    Arg(10'000'000),
    Arg(30'000'000),
    Arg(50'000'000)
)
#else
UC12_BENCHMARK_SPEC(
    Arg(36'000'000LL),
    Arg(360'000'000LL),
    Arg(3'600'000'000LL),
    Arg(21'700'000'000LL)
)
#endif

#ifndef ALL_BENCHMARKS_BUILD
int main(int argc, char** argv) {
    validate_groupby_specialized(ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12);
    validate_grouped_count_low_specialized(ROW_STRIDE12, KEY_OFFSET12, N_BUCKETS12);
    validate_grouped_minmax_low_specialized(ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12);
    validate_grouped_sum_tradeoff_specialized(ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12);
    validate_grouped_count_tradeoff_specialized(ROW_STRIDE12, KEY_OFFSET12, N_BUCKETS12);
    validate_grouped_minmax_tradeoff_specialized(ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12);
    validate_grouped_sum_abstract_specialized(ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12);
    validate_grouped_count_abstract_specialized(ROW_STRIDE12, KEY_OFFSET12, N_BUCKETS12);
    validate_grouped_minmax_abstract_specialized(ROW_STRIDE12, KEY_OFFSET12, VALUE_OFFSET12, N_BUCKETS12);

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
