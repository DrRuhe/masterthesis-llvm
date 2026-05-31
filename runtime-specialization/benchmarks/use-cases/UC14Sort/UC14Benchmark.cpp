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

static constexpr int64_t N_SORT_MAX = 400'000'000;

// Struct layout: two double fields (key1 at offset 0, key2 at offset 8).
static constexpr int STRUCT_ELEM_SIZE  = 16;   // sizeof({double key1, double key2})
static constexpr int STRUCT_KEY1_OFF   = 0;
static constexpr int STRUCT_KEY2_OFF   = 8;

// ---------------------------------------------------------------------------
// int64 datasets — shuffled reference + working copy
// ---------------------------------------------------------------------------

// Pre-shuffled reference copy; restored into g_sort_data before each timed call.
static std::vector<int64_t> g_reference_data;

// Working copy (overwritten each benchmark iteration).
static std::vector<int64_t> g_sort_data;

// Legacy alias kept for the original BM_UC14_* functions.
static std::vector<int64_t>& g_data = g_sort_data;

// ---------------------------------------------------------------------------
// Struct dataset — shuffled reference + working copy
// ---------------------------------------------------------------------------

struct SortRecord {
    double key1;
    double key2;
};

static constexpr int64_t N_STRUCT_MAX = 250'000'000;

static std::vector<SortRecord> g_struct_reference;

static std::vector<SortRecord> g_struct_data;

static void setup_uc14(const benchmark::State&) {
    if (!g_reference_data.empty()) return;
    g_reference_data.resize(N_SORT_MAX);
    std::iota(g_reference_data.begin(), g_reference_data.end(), int64_t{0});
    std::shuffle(g_reference_data.begin(), g_reference_data.end(), std::mt19937{42});
    g_sort_data.resize(N_SORT_MAX);
    g_sort_data = g_reference_data;
    g_struct_reference.resize(N_STRUCT_MAX);
    {
        std::mt19937_64 rng{42};
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        for (auto& r : g_struct_reference) { r.key1 = dist(rng); r.key2 = dist(rng); }
    }
    std::shuffle(g_struct_reference.begin(), g_struct_reference.end(), std::mt19937{42});
    g_struct_data.resize(N_STRUCT_MAX);
    g_struct_data = g_struct_reference;
}
static void teardown_uc14(const benchmark::State&) {
    // Buffer stays allocated for process lifetime; freed by OS on exit.
}

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
    std::mt19937 rng{42};
    for (auto _ : state) {
        state.PauseTiming();
        std::shuffle(g_struct_data.begin(), g_struct_data.begin() + n_elements, rng);
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
    std::mt19937 rng{42};
    for (auto _ : state) {
        state.PauseTiming();
        std::shuffle(g_struct_data.begin(), g_struct_data.begin() + n_elements, rng);
        state.ResumeTiming();
        spec(g_struct_data.data(), n_elements);
    }
}

// ---------------------------------------------------------------------------
// struct_sort — tradeoff level
// ---------------------------------------------------------------------------

static void BM_struct_sort_tradeoff_unspecialized(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    std::mt19937 rng{42};
    for (auto _ : state) {
        state.PauseTiming();
        std::shuffle(g_struct_data.begin(), g_struct_data.begin() + n_elements, rng);
        state.ResumeTiming();
        struct_sort_tradeoff_unspecialized(g_struct_data.data(), n_elements,
                                           STRUCT_ELEM_SIZE, STRUCT_KEY1_OFF);
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
    std::mt19937 rng{42};
    for (auto _ : state) {
        state.PauseTiming();
        std::shuffle(g_struct_data.begin(), g_struct_data.begin() + n_elements, rng);
        state.ResumeTiming();
        spec(g_struct_data.data(), n_elements);
    }
}

// ---------------------------------------------------------------------------
// struct_sort — abstract level
// ---------------------------------------------------------------------------

static void BM_struct_sort_abstract_unspecialized(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    std::mt19937 rng{42};
    for (auto _ : state) {
        state.PauseTiming();
        std::shuffle(g_struct_data.begin(), g_struct_data.begin() + n_elements, rng);
        state.ResumeTiming();
        struct_sort_abstract_unspecialized(g_struct_data.data(), n_elements,
                                           STRUCT_ELEM_SIZE, STRUCT_KEY1_OFF);
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
    std::mt19937 rng{42};
    for (auto _ : state) {
        state.PauseTiming();
        std::shuffle(g_struct_data.begin(), g_struct_data.begin() + n_elements, rng);
        state.ResumeTiming();
        spec(g_struct_data.data(), n_elements);
    }
}

// ---------------------------------------------------------------------------
// multi_key_sort — low level
// ---------------------------------------------------------------------------

static void BM_multi_key_sort_low_unspecialized(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    std::mt19937 rng{42};
    for (auto _ : state) {
        state.PauseTiming();
        std::shuffle(g_struct_data.begin(), g_struct_data.begin() + n_elements, rng);
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
    std::mt19937 rng{42};
    for (auto _ : state) {
        state.PauseTiming();
        std::shuffle(g_struct_data.begin(), g_struct_data.begin() + n_elements, rng);
        state.ResumeTiming();
        spec(g_struct_data.data(), n_elements);
    }
}

// ---------------------------------------------------------------------------
// multi_key_sort — tradeoff level
// ---------------------------------------------------------------------------

static void BM_multi_key_sort_tradeoff_unspecialized(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    std::mt19937 rng{42};
    for (auto _ : state) {
        state.PauseTiming();
        std::shuffle(g_struct_data.begin(), g_struct_data.begin() + n_elements, rng);
        state.ResumeTiming();
        multi_key_sort_tradeoff_unspecialized(g_struct_data.data(), n_elements, STRUCT_ELEM_SIZE,
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
    std::mt19937 rng{42};
    for (auto _ : state) {
        state.PauseTiming();
        std::shuffle(g_struct_data.begin(), g_struct_data.begin() + n_elements, rng);
        state.ResumeTiming();
        spec(g_struct_data.data(), n_elements);
    }
}

// ---------------------------------------------------------------------------
// multi_key_sort — abstract level
// ---------------------------------------------------------------------------

static void BM_multi_key_sort_abstract_unspecialized(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    std::mt19937 rng{42};
    for (auto _ : state) {
        state.PauseTiming();
        std::shuffle(g_struct_data.begin(), g_struct_data.begin() + n_elements, rng);
        state.ResumeTiming();
        multi_key_sort_abstract_unspecialized(g_struct_data.data(), n_elements, STRUCT_ELEM_SIZE,
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
    std::mt19937 rng{42};
    for (auto _ : state) {
        state.PauseTiming();
        std::shuffle(g_struct_data.begin(), g_struct_data.begin() + n_elements, rng);
        state.ResumeTiming();
        spec(g_struct_data.data(), n_elements);
    }
}

// ---------------------------------------------------------------------------
// generic_sort — tradeoff level
// ---------------------------------------------------------------------------

static void BM_generic_sort_tradeoff_unspecialized(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    std::mt19937 rng{42};
    for (auto _ : state) {
        state.PauseTiming();
        std::shuffle(g_sort_data.begin(), g_sort_data.begin() + n_elements, rng);
        state.ResumeTiming();
        generic_sort_tradeoff_unspecialized(g_sort_data.data(), n_elements, sizeof(int64_t));
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
    std::mt19937 rng{42};
    for (auto _ : state) {
        state.PauseTiming();
        std::shuffle(g_sort_data.begin(), g_sort_data.begin() + n_elements, rng);
        state.ResumeTiming();
        spec(g_sort_data.data(), n_elements);
    }
}

// ---------------------------------------------------------------------------
// generic_sort — abstract level
// ---------------------------------------------------------------------------

static void BM_generic_sort_abstract_unspecialized(benchmark::State& state) {
    int64_t n_elements = state.range(0);
    std::mt19937 rng{42};
    for (auto _ : state) {
        state.PauseTiming();
        std::shuffle(g_sort_data.begin(), g_sort_data.begin() + n_elements, rng);
        state.ResumeTiming();
        generic_sort_abstract_unspecialized(g_sort_data.data(), n_elements, sizeof(int64_t));
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
    std::mt19937 rng{42};
    for (auto _ : state) {
        state.PauseTiming();
        std::shuffle(g_sort_data.begin(), g_sort_data.begin() + n_elements, rng);
        state.ResumeTiming();
        spec(g_sort_data.data(), n_elements);
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// JIT analysis benchmarks
// ---------------------------------------------------------------------------

static void BM_generic_sort_low_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_sort_specialized(int64_asc_cmp, sizeof(int64_t));
    });
}

static void BM_generic_sort_tradeoff_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_generic_sort_tradeoff_specialized(sizeof(int64_t));
    });
}

static void BM_generic_sort_abstract_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_generic_sort_abstract_specialized(sizeof(int64_t));
    });
}

static void BM_struct_sort_low_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_struct_sort_low_specialized(STRUCT_ELEM_SIZE, STRUCT_KEY1_OFF);
    });
}

static void BM_struct_sort_tradeoff_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_struct_sort_tradeoff_specialized(STRUCT_ELEM_SIZE, STRUCT_KEY1_OFF);
    });
}

static void BM_struct_sort_abstract_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_struct_sort_abstract_specialized(STRUCT_ELEM_SIZE, STRUCT_KEY1_OFF);
    });
}

static void BM_multi_key_sort_low_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_multi_key_sort_low_specialized(STRUCT_ELEM_SIZE, STRUCT_KEY1_OFF, STRUCT_KEY2_OFF);
    });
}

static void BM_multi_key_sort_tradeoff_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_multi_key_sort_tradeoff_specialized(STRUCT_ELEM_SIZE, STRUCT_KEY1_OFF, STRUCT_KEY2_OFF);
    });
}

static void BM_multi_key_sort_abstract_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_multi_key_sort_abstract_specialized(STRUCT_ELEM_SIZE, STRUCT_KEY1_OFF, STRUCT_KEY2_OFF);
    });
}

// Benchmark registration macros
#define UC14_JIT_ANALYSIS_VARIANT(BM, NAME, ABSTRACTION, SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM)->Name("BM_g:uc14_sort;n:" NAME ";a:" ABSTRACTION ";s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM)->Name("BM_g:uc14_sort;n:" NAME ";a:" ABSTRACTION ";s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM)->Name("BM_g:uc14_sort;n:" NAME ";a:" ABSTRACTION ";s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM)->Name("BM_g:uc14_sort;n:" NAME ";a:" ABSTRACTION ";s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond);

// ---------------------------------------------------------------------------

#define UC14_BENCHMARK_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC14_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC14_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond);

// generic_sort — tradeoff
#define UC14_GENERIC_TRADEOFF_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_generic_sort_tradeoff_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_tradeoff_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_tradeoff_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_tradeoff_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_tradeoff_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_tradeoff_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_tradeoff_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_tradeoff_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_tradeoff_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_tradeoff_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_tradeoff_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_tradeoff_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond);

// generic_sort — abstract
#define UC14_GENERIC_ABSTRACT_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_generic_sort_abstract_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:abstract;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_abstract_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_abstract_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:abstract;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_abstract_unspecialized)->Name("BM_g:uc14_sort;n:generic_sort;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_abstract_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:abstract;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_abstract_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_abstract_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:abstract;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_abstract_jit_overhead)->Name("BM_g:uc14_sort;n:generic_sort;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_abstract_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:abstract;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_abstract_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_abstract_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:abstract;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_generic_sort_abstract_specialized_exec)->Name("BM_g:uc14_sort;n:generic_sort;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond);

// struct_sort — low
#define UC14_STRUCT_LOW_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_struct_sort_low_unspecialized)->Name("BM_g:uc14_sort;n:struct_sort;a:low;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_low_unspecialized)->Name("BM_g:uc14_sort;n:struct_sort;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_low_unspecialized)->Name("BM_g:uc14_sort;n:struct_sort;a:low;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_low_unspecialized)->Name("BM_g:uc14_sort;n:struct_sort;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_low_jit_overhead)->Name("BM_g:uc14_sort;n:struct_sort;a:low;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_low_jit_overhead)->Name("BM_g:uc14_sort;n:struct_sort;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_low_jit_overhead)->Name("BM_g:uc14_sort;n:struct_sort;a:low;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_low_jit_overhead)->Name("BM_g:uc14_sort;n:struct_sort;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_low_specialized_exec)->Name("BM_g:uc14_sort;n:struct_sort;a:low;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_low_specialized_exec)->Name("BM_g:uc14_sort;n:struct_sort;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_low_specialized_exec)->Name("BM_g:uc14_sort;n:struct_sort;a:low;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_low_specialized_exec)->Name("BM_g:uc14_sort;n:struct_sort;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond);

// struct_sort — tradeoff
#define UC14_STRUCT_TRADEOFF_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_struct_sort_tradeoff_unspecialized)->Name("BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_tradeoff_unspecialized)->Name("BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_tradeoff_unspecialized)->Name("BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_tradeoff_unspecialized)->Name("BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_tradeoff_jit_overhead)->Name("BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_tradeoff_jit_overhead)->Name("BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_tradeoff_jit_overhead)->Name("BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_tradeoff_jit_overhead)->Name("BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_tradeoff_specialized_exec)->Name("BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_tradeoff_specialized_exec)->Name("BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_tradeoff_specialized_exec)->Name("BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_tradeoff_specialized_exec)->Name("BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond);

// struct_sort — abstract
#define UC14_STRUCT_ABSTRACT_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_struct_sort_abstract_unspecialized)->Name("BM_g:uc14_sort;n:struct_sort;a:abstract;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_abstract_unspecialized)->Name("BM_g:uc14_sort;n:struct_sort;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_abstract_unspecialized)->Name("BM_g:uc14_sort;n:struct_sort;a:abstract;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_abstract_unspecialized)->Name("BM_g:uc14_sort;n:struct_sort;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_abstract_jit_overhead)->Name("BM_g:uc14_sort;n:struct_sort;a:abstract;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_abstract_jit_overhead)->Name("BM_g:uc14_sort;n:struct_sort;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_abstract_jit_overhead)->Name("BM_g:uc14_sort;n:struct_sort;a:abstract;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_abstract_jit_overhead)->Name("BM_g:uc14_sort;n:struct_sort;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_abstract_specialized_exec)->Name("BM_g:uc14_sort;n:struct_sort;a:abstract;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_abstract_specialized_exec)->Name("BM_g:uc14_sort;n:struct_sort;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_abstract_specialized_exec)->Name("BM_g:uc14_sort;n:struct_sort;a:abstract;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_struct_sort_abstract_specialized_exec)->Name("BM_g:uc14_sort;n:struct_sort;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond);

// multi_key_sort — low
#define UC14_MULTI_LOW_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_multi_key_sort_low_unspecialized)->Name("BM_g:uc14_sort;n:multi_key_sort;a:low;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_low_unspecialized)->Name("BM_g:uc14_sort;n:multi_key_sort;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_low_unspecialized)->Name("BM_g:uc14_sort;n:multi_key_sort;a:low;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_low_unspecialized)->Name("BM_g:uc14_sort;n:multi_key_sort;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_low_jit_overhead)->Name("BM_g:uc14_sort;n:multi_key_sort;a:low;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_low_jit_overhead)->Name("BM_g:uc14_sort;n:multi_key_sort;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_low_jit_overhead)->Name("BM_g:uc14_sort;n:multi_key_sort;a:low;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_low_jit_overhead)->Name("BM_g:uc14_sort;n:multi_key_sort;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_low_specialized_exec)->Name("BM_g:uc14_sort;n:multi_key_sort;a:low;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_low_specialized_exec)->Name("BM_g:uc14_sort;n:multi_key_sort;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_low_specialized_exec)->Name("BM_g:uc14_sort;n:multi_key_sort;a:low;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_low_specialized_exec)->Name("BM_g:uc14_sort;n:multi_key_sort;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond);

// multi_key_sort — tradeoff
#define UC14_MULTI_TRADEOFF_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_multi_key_sort_tradeoff_unspecialized)->Name("BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_tradeoff_unspecialized)->Name("BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_tradeoff_unspecialized)->Name("BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_tradeoff_unspecialized)->Name("BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_tradeoff_jit_overhead)->Name("BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_tradeoff_jit_overhead)->Name("BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_tradeoff_jit_overhead)->Name("BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_tradeoff_jit_overhead)->Name("BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_tradeoff_specialized_exec)->Name("BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_tradeoff_specialized_exec)->Name("BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_tradeoff_specialized_exec)->Name("BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_tradeoff_specialized_exec)->Name("BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond);

// multi_key_sort — abstract
#define UC14_MULTI_ABSTRACT_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_multi_key_sort_abstract_unspecialized)->Name("BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_abstract_unspecialized)->Name("BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_abstract_unspecialized)->Name("BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_abstract_unspecialized)->Name("BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_abstract_jit_overhead)->Name("BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:SMALL;t:jit_overhead;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_abstract_jit_overhead)->Name("BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_abstract_jit_overhead)->Name("BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:LARGE;t:jit_overhead;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_abstract_jit_overhead)->Name("BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_abstract_specialized_exec)->Name("BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_abstract_specialized_exec)->Name("BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_abstract_specialized_exec)->Name("BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_multi_key_sort_abstract_specialized_exec)->Name("BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc14)->Teardown(teardown_uc14)->Unit(benchmark::kMillisecond);

// ---------------------------------------------------------------------------
// Instantiate all benchmark registrations
// ---------------------------------------------------------------------------

UC14_BENCHMARK_SPEC(
    Arg(900'000),
    Arg(8'000'000),
    Arg(70'000'000),
    Arg(400'000'000)
)
UC14_JIT_ANALYSIS_VARIANT(BM_generic_sort_low_jit_analysis, "generic_sort", "low", Arg(900'000), Arg(8'000'000), Arg(70'000'000), Arg(400'000'000))

UC14_GENERIC_TRADEOFF_SPEC(
    Arg(900'000),
    Arg(8'000'000),
    Arg(70'000'000),
    Arg(400'000'000)
)
UC14_JIT_ANALYSIS_VARIANT(BM_generic_sort_tradeoff_jit_analysis, "generic_sort", "tradeoff", Arg(900'000), Arg(8'000'000), Arg(70'000'000), Arg(400'000'000))

UC14_GENERIC_ABSTRACT_SPEC(
    Arg(900'000),
    Arg(8'000'000),
    Arg(70'000'000),
    Arg(400'000'000)
)
UC14_JIT_ANALYSIS_VARIANT(BM_generic_sort_abstract_jit_analysis, "generic_sort", "abstract", Arg(900'000), Arg(8'000'000), Arg(70'000'000), Arg(400'000'000))

UC14_STRUCT_LOW_SPEC(
    Arg(1'000'000),
    Arg(8'500'000),
    Arg(50'000'000),
    Arg(250'000'000)
)
UC14_JIT_ANALYSIS_VARIANT(BM_struct_sort_low_jit_analysis, "struct_sort", "low", Arg(1'000'000), Arg(8'500'000), Arg(50'000'000), Arg(250'000'000))

UC14_STRUCT_TRADEOFF_SPEC(
    Arg(1'000'000),
    Arg(8'500'000),
    Arg(50'000'000),
    Arg(250'000'000)
)
UC14_JIT_ANALYSIS_VARIANT(BM_struct_sort_tradeoff_jit_analysis, "struct_sort", "tradeoff", Arg(1'000'000), Arg(8'500'000), Arg(50'000'000), Arg(250'000'000))

UC14_STRUCT_ABSTRACT_SPEC(
    Arg(1'000'000),
    Arg(8'500'000),
    Arg(50'000'000),
    Arg(250'000'000)
)
UC14_JIT_ANALYSIS_VARIANT(BM_struct_sort_abstract_jit_analysis, "struct_sort", "abstract", Arg(1'000'000), Arg(8'500'000), Arg(50'000'000), Arg(250'000'000))

UC14_MULTI_LOW_SPEC(
    Arg(750'000),
    Arg(6'500'000),
    Arg(50'000'000),
    Arg(250'000'000)
)
UC14_JIT_ANALYSIS_VARIANT(BM_multi_key_sort_low_jit_analysis, "multi_key_sort", "low", Arg(750'000), Arg(6'500'000), Arg(50'000'000), Arg(250'000'000))

UC14_MULTI_TRADEOFF_SPEC(
    Arg(750'000),
    Arg(6'500'000),
    Arg(50'000'000),
    Arg(250'000'000)
)
UC14_JIT_ANALYSIS_VARIANT(BM_multi_key_sort_tradeoff_jit_analysis, "multi_key_sort", "tradeoff", Arg(750'000), Arg(6'500'000), Arg(50'000'000), Arg(250'000'000))

UC14_MULTI_ABSTRACT_SPEC(
    Arg(750'000),
    Arg(6'500'000),
    Arg(50'000'000),
    Arg(250'000'000)
)
UC14_JIT_ANALYSIS_VARIANT(BM_multi_key_sort_abstract_jit_analysis, "multi_key_sort", "abstract", Arg(750'000), Arg(6'500'000), Arg(50'000'000), Arg(250'000'000))

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
