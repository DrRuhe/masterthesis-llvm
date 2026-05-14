#include "UC14Kernels.h"
#include "ClangRuntimeSpecializerBenchmark.h"
#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <algorithm>
#include <cstdint>
#include <numeric>
#include <cstring>

// ---------------------------------------------------------------------------
// Dataset sizing
// ---------------------------------------------------------------------------

static constexpr int64_t N_SORT_MAX = 200'000'000;

// Struct layout: two double fields (key1 at offset 0, key2 at offset 8).
static constexpr int STRUCT_ELEM_SIZE  = 16;   // sizeof({double key1, double key2})
static constexpr int STRUCT_KEY1_OFF   = 0;
static constexpr int STRUCT_KEY2_OFF   = 8;

// ---------------------------------------------------------------------------
// int64 datasets — shuffled reference + working copy
// ---------------------------------------------------------------------------

// Pre-shuffled reference copy; restored into g_sort_data before each timed call.
static std::vector<int64_t> g_reference_data = [] {
    std::vector<int64_t> d(N_SORT_MAX);
    std::iota(d.begin(), d.end(), 0);
    std::shuffle(d.begin(), d.end(), std::mt19937{42});
    return d;
}();

// Working copy (overwritten each benchmark iteration).
static std::vector<int64_t> g_sort_data(N_SORT_MAX);

// Legacy alias kept for the original BM_UC14_* functions.
static std::vector<int64_t>& g_data = g_sort_data;

// ---------------------------------------------------------------------------
// Struct dataset — shuffled reference + working copy
// ---------------------------------------------------------------------------

struct SortRecord {
    double key1;
    double key2;
};

static constexpr int64_t N_STRUCT_MAX = 50'000'000;

static std::vector<SortRecord> g_struct_reference = [] {
    std::vector<SortRecord> d(N_STRUCT_MAX);
    std::mt19937_64 rng{42};
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    for (auto& r : d) { r.key1 = dist(rng); r.key2 = dist(rng); }
    std::shuffle(d.begin(), d.end(), std::mt19937{42});
    return d;
}();

static std::vector<SortRecord> g_struct_data(N_STRUCT_MAX);

// ---------------------------------------------------------------------------
// Original BM_UC14_* (generic_sort, low level)
// ---------------------------------------------------------------------------

static void BM_UC14_unspecialized(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    std::mt19937 rng{42};
    for (auto _ : state) {
        state.PauseTiming();
        std::shuffle(g_data.begin(), g_data.begin() + n_elements, rng);
        state.ResumeTiming();
        generic_sort(g_data.data(), n_elements, sizeof(int64_t), &int64_asc_cmp);
    }
}

static void BM_UC14_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_sort_specialized(&int64_asc_cmp, sizeof(int64_t)));
    }
}

static void BM_UC14_specialized_exec(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    auto spec = create_sort_specialized(&int64_asc_cmp, sizeof(int64_t));
    std::mt19937 local_rng{42};
    for (auto _ : state) {
        state.PauseTiming();
        std::shuffle(g_data.begin(), g_data.begin() + n_elements, local_rng);
        state.ResumeTiming();
        spec(g_data.data(), n_elements);
    }
}

// ---------------------------------------------------------------------------
// struct_sort — low level
// ---------------------------------------------------------------------------

static void BM_struct_sort_low_unspecialized(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        std::copy(g_struct_reference.begin(),
                  g_struct_reference.begin() + n_elements,
                  g_struct_data.begin());
        state.ResumeTiming();
        struct_sort(g_struct_data.data(), n_elements, STRUCT_ELEM_SIZE, STRUCT_KEY1_OFF);
    }
}

static void BM_struct_sort_low_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_struct_sort_low_specialized(STRUCT_ELEM_SIZE, STRUCT_KEY1_OFF));
    }
}

static void BM_struct_sort_low_specialized_exec(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    auto spec = create_struct_sort_low_specialized(STRUCT_ELEM_SIZE, STRUCT_KEY1_OFF);
    for (auto _ : state) {
        state.PauseTiming();
        std::copy(g_struct_reference.begin(),
                  g_struct_reference.begin() + n_elements,
                  g_struct_data.begin());
        state.ResumeTiming();
        spec(g_struct_data.data(), n_elements);
    }
}

// ---------------------------------------------------------------------------
// struct_sort — tradeoff level
// ---------------------------------------------------------------------------

static void BM_struct_sort_tradeoff_unspecialized(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        std::copy(g_struct_reference.begin(),
                  g_struct_reference.begin() + n_elements,
                  g_struct_data.begin());
        state.ResumeTiming();
        struct_sort(g_struct_data.data(), n_elements, STRUCT_ELEM_SIZE, STRUCT_KEY1_OFF);
    }
}

static void BM_struct_sort_tradeoff_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_struct_sort_tradeoff_specialized(STRUCT_ELEM_SIZE, STRUCT_KEY1_OFF));
    }
}

static void BM_struct_sort_tradeoff_specialized_exec(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    auto spec = create_struct_sort_tradeoff_specialized(STRUCT_ELEM_SIZE, STRUCT_KEY1_OFF);
    for (auto _ : state) {
        state.PauseTiming();
        std::copy(g_struct_reference.begin(),
                  g_struct_reference.begin() + n_elements,
                  g_struct_data.begin());
        state.ResumeTiming();
        spec(g_struct_data.data(), n_elements);
    }
}

// ---------------------------------------------------------------------------
// struct_sort — abstract level
// ---------------------------------------------------------------------------

static void BM_struct_sort_abstract_unspecialized(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        std::copy(g_struct_reference.begin(),
                  g_struct_reference.begin() + n_elements,
                  g_struct_data.begin());
        state.ResumeTiming();
        struct_sort(g_struct_data.data(), n_elements, STRUCT_ELEM_SIZE, STRUCT_KEY1_OFF);
    }
}

static void BM_struct_sort_abstract_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_struct_sort_abstract_specialized(STRUCT_ELEM_SIZE, STRUCT_KEY1_OFF));
    }
}

static void BM_struct_sort_abstract_specialized_exec(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    auto spec = create_struct_sort_abstract_specialized(STRUCT_ELEM_SIZE, STRUCT_KEY1_OFF);
    for (auto _ : state) {
        state.PauseTiming();
        std::copy(g_struct_reference.begin(),
                  g_struct_reference.begin() + n_elements,
                  g_struct_data.begin());
        state.ResumeTiming();
        spec(g_struct_data.data(), n_elements);
    }
}

// ---------------------------------------------------------------------------
// multi_key_sort — low level
// ---------------------------------------------------------------------------

static void BM_multi_key_sort_low_unspecialized(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        std::copy(g_struct_reference.begin(),
                  g_struct_reference.begin() + n_elements,
                  g_struct_data.begin());
        state.ResumeTiming();
        multi_key_sort(g_struct_data.data(), n_elements, STRUCT_ELEM_SIZE,
                       STRUCT_KEY1_OFF, STRUCT_KEY2_OFF);
    }
}

static void BM_multi_key_sort_low_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_multi_key_sort_low_specialized(STRUCT_ELEM_SIZE,
                                                  STRUCT_KEY1_OFF, STRUCT_KEY2_OFF));
    }
}

static void BM_multi_key_sort_low_specialized_exec(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    auto spec = create_multi_key_sort_low_specialized(STRUCT_ELEM_SIZE,
                                                       STRUCT_KEY1_OFF, STRUCT_KEY2_OFF);
    for (auto _ : state) {
        state.PauseTiming();
        std::copy(g_struct_reference.begin(),
                  g_struct_reference.begin() + n_elements,
                  g_struct_data.begin());
        state.ResumeTiming();
        spec(g_struct_data.data(), n_elements);
    }
}

// ---------------------------------------------------------------------------
// multi_key_sort — tradeoff level
// ---------------------------------------------------------------------------

static void BM_multi_key_sort_tradeoff_unspecialized(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        std::copy(g_struct_reference.begin(),
                  g_struct_reference.begin() + n_elements,
                  g_struct_data.begin());
        state.ResumeTiming();
        multi_key_sort(g_struct_data.data(), n_elements, STRUCT_ELEM_SIZE,
                       STRUCT_KEY1_OFF, STRUCT_KEY2_OFF);
    }
}

static void BM_multi_key_sort_tradeoff_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_multi_key_sort_tradeoff_specialized(STRUCT_ELEM_SIZE,
                                                       STRUCT_KEY1_OFF, STRUCT_KEY2_OFF));
    }
}

static void BM_multi_key_sort_tradeoff_specialized_exec(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    auto spec = create_multi_key_sort_tradeoff_specialized(STRUCT_ELEM_SIZE,
                                                            STRUCT_KEY1_OFF, STRUCT_KEY2_OFF);
    for (auto _ : state) {
        state.PauseTiming();
        std::copy(g_struct_reference.begin(),
                  g_struct_reference.begin() + n_elements,
                  g_struct_data.begin());
        state.ResumeTiming();
        spec(g_struct_data.data(), n_elements);
    }
}

// ---------------------------------------------------------------------------
// multi_key_sort — abstract level
// ---------------------------------------------------------------------------

static void BM_multi_key_sort_abstract_unspecialized(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        std::copy(g_struct_reference.begin(),
                  g_struct_reference.begin() + n_elements,
                  g_struct_data.begin());
        state.ResumeTiming();
        multi_key_sort(g_struct_data.data(), n_elements, STRUCT_ELEM_SIZE,
                       STRUCT_KEY1_OFF, STRUCT_KEY2_OFF);
    }
}

static void BM_multi_key_sort_abstract_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_multi_key_sort_abstract_specialized(STRUCT_ELEM_SIZE,
                                                       STRUCT_KEY1_OFF, STRUCT_KEY2_OFF));
    }
}

static void BM_multi_key_sort_abstract_specialized_exec(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    auto spec = create_multi_key_sort_abstract_specialized(STRUCT_ELEM_SIZE,
                                                            STRUCT_KEY1_OFF, STRUCT_KEY2_OFF);
    for (auto _ : state) {
        state.PauseTiming();
        std::copy(g_struct_reference.begin(),
                  g_struct_reference.begin() + n_elements,
                  g_struct_data.begin());
        state.ResumeTiming();
        spec(g_struct_data.data(), n_elements);
    }
}

// ---------------------------------------------------------------------------
// generic_sort — tradeoff level
// ---------------------------------------------------------------------------

static void BM_generic_sort_tradeoff_unspecialized(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        std::copy(g_reference_data.begin(),
                  g_reference_data.begin() + n_elements,
                  g_sort_data.begin());
        state.ResumeTiming();
        generic_sort(g_sort_data.data(), n_elements, sizeof(int64_t), &int64_asc_cmp);
    }
}

static void BM_generic_sort_tradeoff_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_generic_sort_tradeoff_specialized(sizeof(int64_t)));
    }
}

static void BM_generic_sort_tradeoff_specialized_exec(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    auto spec = create_generic_sort_tradeoff_specialized(sizeof(int64_t));
    for (auto _ : state) {
        state.PauseTiming();
        std::copy(g_reference_data.begin(),
                  g_reference_data.begin() + n_elements,
                  g_sort_data.begin());
        state.ResumeTiming();
        spec(g_sort_data.data(), n_elements);
    }
}

// ---------------------------------------------------------------------------
// generic_sort — abstract level
// ---------------------------------------------------------------------------

static void BM_generic_sort_abstract_unspecialized(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        std::copy(g_reference_data.begin(),
                  g_reference_data.begin() + n_elements,
                  g_sort_data.begin());
        state.ResumeTiming();
        generic_sort(g_sort_data.data(), n_elements, sizeof(int64_t), &int64_asc_cmp);
    }
}

static void BM_generic_sort_abstract_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_generic_sort_abstract_specialized(sizeof(int64_t)));
    }
}

static void BM_generic_sort_abstract_specialized_exec(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    auto spec = create_generic_sort_abstract_specialized(sizeof(int64_t));
    for (auto _ : state) {
        state.PauseTiming();
        std::copy(g_reference_data.begin(),
                  g_reference_data.begin() + n_elements,
                  g_sort_data.begin());
        state.ResumeTiming();
        spec(g_sort_data.data(), n_elements);
    }
}

// ---------------------------------------------------------------------------
// Benchmark registration macros
// ---------------------------------------------------------------------------

#define UC14_BENCHMARK_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC14_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

// generic_sort — tradeoff
#define UC14_GENERIC_TRADEOFF_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_generic_sort_tradeoff_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_tradeoff_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_tradeoff_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_tradeoff_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_tradeoff_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_tradeoff_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_tradeoff_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_tradeoff_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_tradeoff_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_tradeoff_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_tradeoff_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_tradeoff_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

// generic_sort — abstract
#define UC14_GENERIC_ABSTRACT_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_generic_sort_abstract_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:abstract;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_abstract_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_abstract_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:abstract;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_abstract_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_abstract_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:abstract;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_abstract_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_abstract_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:abstract;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_abstract_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_abstract_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:abstract;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_abstract_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_abstract_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:abstract;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_abstract_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

// struct_sort — low
#define UC14_STRUCT_LOW_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_struct_sort_low_unspecialized)->Name("BM_g:uc14_sort;n:struct_sort;a:low;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_low_unspecialized)->Name("BM_g:uc14_sort;n:struct_sort;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_low_unspecialized)->Name("BM_g:uc14_sort;n:struct_sort;a:low;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_low_unspecialized)->Name("BM_g:uc14_sort;n:struct_sort;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_low_jit_overhead)->Name("BM_g:uc14_sort;n:struct_sort;a:low;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_low_jit_overhead)->Name("BM_g:uc14_sort;n:struct_sort;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_low_jit_overhead)->Name("BM_g:uc14_sort;n:struct_sort;a:low;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_low_jit_overhead)->Name("BM_g:uc14_sort;n:struct_sort;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_low_specialized_exec)->Name("BM_g:uc14_sort;n:struct_sort;a:low;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_low_specialized_exec)->Name("BM_g:uc14_sort;n:struct_sort;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_low_specialized_exec)->Name("BM_g:uc14_sort;n:struct_sort;a:low;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_low_specialized_exec)->Name("BM_g:uc14_sort;n:struct_sort;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

// struct_sort — tradeoff
#define UC14_STRUCT_TRADEOFF_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_struct_sort_tradeoff_unspecialized)->Name("BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_tradeoff_unspecialized)->Name("BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_tradeoff_unspecialized)->Name("BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_tradeoff_unspecialized)->Name("BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_tradeoff_jit_overhead)->Name("BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_tradeoff_jit_overhead)->Name("BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_tradeoff_jit_overhead)->Name("BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_tradeoff_jit_overhead)->Name("BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_tradeoff_specialized_exec)->Name("BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_tradeoff_specialized_exec)->Name("BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_tradeoff_specialized_exec)->Name("BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_tradeoff_specialized_exec)->Name("BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

// struct_sort — abstract
#define UC14_STRUCT_ABSTRACT_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_struct_sort_abstract_unspecialized)->Name("BM_g:uc14_sort;n:struct_sort;a:abstract;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_abstract_unspecialized)->Name("BM_g:uc14_sort;n:struct_sort;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_abstract_unspecialized)->Name("BM_g:uc14_sort;n:struct_sort;a:abstract;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_abstract_unspecialized)->Name("BM_g:uc14_sort;n:struct_sort;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_abstract_jit_overhead)->Name("BM_g:uc14_sort;n:struct_sort;a:abstract;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_abstract_jit_overhead)->Name("BM_g:uc14_sort;n:struct_sort;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_abstract_jit_overhead)->Name("BM_g:uc14_sort;n:struct_sort;a:abstract;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_abstract_jit_overhead)->Name("BM_g:uc14_sort;n:struct_sort;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_abstract_specialized_exec)->Name("BM_g:uc14_sort;n:struct_sort;a:abstract;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_abstract_specialized_exec)->Name("BM_g:uc14_sort;n:struct_sort;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_abstract_specialized_exec)->Name("BM_g:uc14_sort;n:struct_sort;a:abstract;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_abstract_specialized_exec)->Name("BM_g:uc14_sort;n:struct_sort;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

// multi_key_sort — low
#define UC14_MULTI_LOW_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_multi_key_sort_low_unspecialized)->Name("BM_g:uc14_sort;n:multi_key_sort;a:low;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_low_unspecialized)->Name("BM_g:uc14_sort;n:multi_key_sort;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_low_unspecialized)->Name("BM_g:uc14_sort;n:multi_key_sort;a:low;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_low_unspecialized)->Name("BM_g:uc14_sort;n:multi_key_sort;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_low_jit_overhead)->Name("BM_g:uc14_sort;n:multi_key_sort;a:low;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_low_jit_overhead)->Name("BM_g:uc14_sort;n:multi_key_sort;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_low_jit_overhead)->Name("BM_g:uc14_sort;n:multi_key_sort;a:low;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_low_jit_overhead)->Name("BM_g:uc14_sort;n:multi_key_sort;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_low_specialized_exec)->Name("BM_g:uc14_sort;n:multi_key_sort;a:low;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_low_specialized_exec)->Name("BM_g:uc14_sort;n:multi_key_sort;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_low_specialized_exec)->Name("BM_g:uc14_sort;n:multi_key_sort;a:low;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_low_specialized_exec)->Name("BM_g:uc14_sort;n:multi_key_sort;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

// multi_key_sort — tradeoff
#define UC14_MULTI_TRADEOFF_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_multi_key_sort_tradeoff_unspecialized)->Name("BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_tradeoff_unspecialized)->Name("BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_tradeoff_unspecialized)->Name("BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_tradeoff_unspecialized)->Name("BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_tradeoff_jit_overhead)->Name("BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_tradeoff_jit_overhead)->Name("BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_tradeoff_jit_overhead)->Name("BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_tradeoff_jit_overhead)->Name("BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_tradeoff_specialized_exec)->Name("BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_tradeoff_specialized_exec)->Name("BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_tradeoff_specialized_exec)->Name("BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_tradeoff_specialized_exec)->Name("BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

// multi_key_sort — abstract
#define UC14_MULTI_ABSTRACT_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_multi_key_sort_abstract_unspecialized)->Name("BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_abstract_unspecialized)->Name("BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_abstract_unspecialized)->Name("BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_abstract_unspecialized)->Name("BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_abstract_jit_overhead)->Name("BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_abstract_jit_overhead)->Name("BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_abstract_jit_overhead)->Name("BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_abstract_jit_overhead)->Name("BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_abstract_specialized_exec)->Name("BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_abstract_specialized_exec)->Name("BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_abstract_specialized_exec)->Name("BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_abstract_specialized_exec)->Name("BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

// ---------------------------------------------------------------------------
// Instantiate all benchmark registrations
// ---------------------------------------------------------------------------

UC14_BENCHMARK_SPEC(
    Arg(550'000),
    Arg(5'000'000),
    Arg(40'000'000),
    Arg(200'000'000)
)

UC14_GENERIC_TRADEOFF_SPEC(
    Arg(550'000),
    Arg(5'000'000),
    Arg(40'000'000),
    Arg(200'000'000)
)

UC14_GENERIC_ABSTRACT_SPEC(
    Arg(550'000),
    Arg(5'000'000),
    Arg(40'000'000),
    Arg(200'000'000)
)

UC14_STRUCT_LOW_SPEC(
    Arg(300'000),
    Arg(3'000'000),
    Arg(20'000'000),
    Arg(50'000'000)
)

UC14_STRUCT_TRADEOFF_SPEC(
    Arg(300'000),
    Arg(3'000'000),
    Arg(20'000'000),
    Arg(50'000'000)
)

UC14_STRUCT_ABSTRACT_SPEC(
    Arg(300'000),
    Arg(3'000'000),
    Arg(20'000'000),
    Arg(50'000'000)
)

UC14_MULTI_LOW_SPEC(
    Arg(300'000),
    Arg(3'000'000),
    Arg(20'000'000),
    Arg(50'000'000)
)

UC14_MULTI_TRADEOFF_SPEC(
    Arg(300'000),
    Arg(3'000'000),
    Arg(20'000'000),
    Arg(50'000'000)
)

UC14_MULTI_ABSTRACT_SPEC(
    Arg(300'000),
    Arg(3'000'000),
    Arg(20'000'000),
    Arg(50'000'000)
)

#ifndef ALL_BENCHMARKS_BUILD
int main(int argc, char** argv) {
    // Validate all specializations at startup.
    validate_sort_specialized(&int64_asc_cmp, sizeof(int64_t));
    validate_generic_sort_tradeoff_specialized(sizeof(int64_t));
    validate_generic_sort_abstract_specialized(sizeof(int64_t));
    validate_struct_sort_low_specialized(STRUCT_ELEM_SIZE, STRUCT_KEY1_OFF);
    validate_struct_sort_tradeoff_specialized(STRUCT_ELEM_SIZE, STRUCT_KEY1_OFF);
    validate_struct_sort_abstract_specialized(STRUCT_ELEM_SIZE, STRUCT_KEY1_OFF);
    validate_multi_key_sort_low_specialized(STRUCT_ELEM_SIZE, STRUCT_KEY1_OFF, STRUCT_KEY2_OFF);
    validate_multi_key_sort_tradeoff_specialized(STRUCT_ELEM_SIZE, STRUCT_KEY1_OFF, STRUCT_KEY2_OFF);
    validate_multi_key_sort_abstract_specialized(STRUCT_ELEM_SIZE, STRUCT_KEY1_OFF, STRUCT_KEY2_OFF);

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
