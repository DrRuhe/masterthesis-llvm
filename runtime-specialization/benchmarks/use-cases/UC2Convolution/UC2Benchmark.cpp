#include "UC2Kernels.h"
#include "ClangRuntimeSpecializerBenchmark.h"
#include <benchmark/benchmark.h>
#include <random>
#include <vector>

static constexpr int TILE_W = 3840;
static constexpr int TILE_H = 3840;

// Global buffers sized to one tile (TILE_W * TILE_H ≈ 57 MB each).
// Benchmarks stream n_tiles tiles over the fixed buffer via state.range(0).
static std::vector<float> g_src;
static std::vector<float> g_dst;

static void setup_uc2(const benchmark::State&) {
    if (!g_src.empty()) return;
    g_src.resize(TILE_W * TILE_H, 0.0f);
    g_dst.resize(TILE_W * TILE_H, 0.0f);
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    for (auto& v : g_src) v = dist(rng);
}
static void teardown_uc2(const benchmark::State&) {
    // Buffer stays allocated for process lifetime; freed by OS on exit.
}

// ---------------------------------------------------------------------------
// Variant: separable_gaussian / low
// ---------------------------------------------------------------------------

static void BM_UC2_unspecialized(benchmark::State& state) {
    int64_t n_tiles = state.range(0);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        for (int64_t t = 0; t < n_tiles; ++t)
            convolve2d(g_src.data(), g_dst.data(), TILE_W, TILE_H, g_kernel_coeffs, 5);
        benchmark::DoNotOptimize(g_dst.data());
    }
}

static void BM_UC2_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(create_conv_specialized(TILE_W, TILE_H));
    }
}

static void BM_UC2_specialized_exec(benchmark::State& state) {
    int64_t n_tiles = state.range(0);
    auto spec = create_conv_specialized(TILE_W, TILE_H);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        for (int64_t t = 0; t < n_tiles; ++t)
            spec(g_src.data(), g_dst.data());
        benchmark::DoNotOptimize(g_dst.data());
    }
}

// ---------------------------------------------------------------------------
// Variant: box_filter / low
// ---------------------------------------------------------------------------

static void BM_UC2_box_filter_low_unspecialized(benchmark::State& state) {
    int64_t n_tiles = state.range(0);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        for (int64_t t = 0; t < n_tiles; ++t)
            box_filter(g_src.data(), g_dst.data(), TILE_W, TILE_H, 2);
        benchmark::DoNotOptimize(g_dst.data());
    }
}

static void BM_UC2_box_filter_low_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(create_box_filter_low_specialized(TILE_W, TILE_H, 2));
    }
}

static void BM_UC2_box_filter_low_specialized_exec(benchmark::State& state) {
    int64_t n_tiles = state.range(0);
    auto spec = create_box_filter_low_specialized(TILE_W, TILE_H, 2);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        for (int64_t t = 0; t < n_tiles; ++t)
            spec(g_src.data(), g_dst.data());
        benchmark::DoNotOptimize(g_dst.data());
    }
}

// ---------------------------------------------------------------------------
// Variant: edge_detection / low
// ---------------------------------------------------------------------------

static void BM_UC2_edge_detection_low_unspecialized(benchmark::State& state) {
    int64_t n_tiles = state.range(0);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        for (int64_t t = 0; t < n_tiles; ++t)
            sobel_edge_detect(g_src.data(), g_dst.data(), TILE_W, TILE_H);
        benchmark::DoNotOptimize(g_dst.data());
    }
}

static void BM_UC2_edge_detection_low_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(create_edge_detection_low_specialized(TILE_W, TILE_H));
    }
}

static void BM_UC2_edge_detection_low_specialized_exec(benchmark::State& state) {
    int64_t n_tiles = state.range(0);
    auto spec = create_edge_detection_low_specialized(TILE_W, TILE_H);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        for (int64_t t = 0; t < n_tiles; ++t)
            spec(g_src.data(), g_dst.data());
        benchmark::DoNotOptimize(g_dst.data());
    }
}

// ---------------------------------------------------------------------------
// Variant: separable_gaussian / tradeoff
// ---------------------------------------------------------------------------

static void BM_UC2_sg_tradeoff_unspecialized(benchmark::State& state) {
    int64_t n_tiles = state.range(0);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        for (int64_t t = 0; t < n_tiles; ++t)
            separable_gaussian_tradeoff_unspecialized(g_src.data(), g_dst.data(), TILE_W, TILE_H, g_kernel_coeffs, 5);
        benchmark::DoNotOptimize(g_dst.data());
    }
}

static void BM_UC2_sg_tradeoff_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_separable_gaussian_tradeoff_specialized(TILE_W, TILE_H, g_kernel_coeffs, 5));
    }
}

static void BM_UC2_sg_tradeoff_specialized_exec(benchmark::State& state) {
    int64_t n_tiles = state.range(0);
    auto spec = create_separable_gaussian_tradeoff_specialized(TILE_W, TILE_H, g_kernel_coeffs, 5);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        for (int64_t t = 0; t < n_tiles; ++t)
            spec(g_src.data(), g_dst.data());
        benchmark::DoNotOptimize(g_dst.data());
    }
}

// ---------------------------------------------------------------------------
// Variant: box_filter / tradeoff
// ---------------------------------------------------------------------------

static void BM_UC2_box_filter_tradeoff_unspecialized(benchmark::State& state) {
    int64_t n_tiles = state.range(0);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        for (int64_t t = 0; t < n_tiles; ++t)
            box_filter_tradeoff_unspecialized(g_src.data(), g_dst.data(), TILE_W, TILE_H, 2);
        benchmark::DoNotOptimize(g_dst.data());
    }
}

static void BM_UC2_box_filter_tradeoff_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_box_filter_tradeoff_specialized(TILE_W, TILE_H, 2));
    }
}

static void BM_UC2_box_filter_tradeoff_specialized_exec(benchmark::State& state) {
    int64_t n_tiles = state.range(0);
    auto spec = create_box_filter_tradeoff_specialized(TILE_W, TILE_H, 2);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        for (int64_t t = 0; t < n_tiles; ++t)
            spec(g_src.data(), g_dst.data());
        benchmark::DoNotOptimize(g_dst.data());
    }
}

// ---------------------------------------------------------------------------
// Variant: edge_detection / tradeoff
// ---------------------------------------------------------------------------

static void BM_UC2_edge_detection_tradeoff_unspecialized(benchmark::State& state) {
    int64_t n_tiles = state.range(0);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        for (int64_t t = 0; t < n_tiles; ++t)
            edge_detection_tradeoff_unspecialized(g_src.data(), g_dst.data(), TILE_W, TILE_H);
        benchmark::DoNotOptimize(g_dst.data());
    }
}

static void BM_UC2_edge_detection_tradeoff_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_edge_detection_tradeoff_specialized(TILE_W, TILE_H));
    }
}

static void BM_UC2_edge_detection_tradeoff_specialized_exec(benchmark::State& state) {
    int64_t n_tiles = state.range(0);
    auto spec = create_edge_detection_tradeoff_specialized(TILE_W, TILE_H);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        for (int64_t t = 0; t < n_tiles; ++t)
            spec(g_src.data(), g_dst.data());
        benchmark::DoNotOptimize(g_dst.data());
    }
}

// ---------------------------------------------------------------------------
// Variant: separable_gaussian / abstract
// ---------------------------------------------------------------------------

static void BM_UC2_sg_abstract_unspecialized(benchmark::State& state) {
    int64_t n_tiles = state.range(0);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        for (int64_t t = 0; t < n_tiles; ++t)
            separable_gaussian_abstract_unspecialized(g_src.data(), g_dst.data(), TILE_W, TILE_H, g_kernel_coeffs, 5);
        benchmark::DoNotOptimize(g_dst.data());
    }
}

static void BM_UC2_sg_abstract_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_separable_gaussian_abstract_specialized(TILE_W, TILE_H, g_kernel_coeffs, 5));
    }
}

static void BM_UC2_sg_abstract_specialized_exec(benchmark::State& state) {
    int64_t n_tiles = state.range(0);
    auto spec = create_separable_gaussian_abstract_specialized(
        TILE_W, TILE_H, g_kernel_coeffs, 5);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        for (int64_t t = 0; t < n_tiles; ++t)
            spec(g_src.data(), g_dst.data());
        benchmark::DoNotOptimize(g_dst.data());
    }
}

// ---------------------------------------------------------------------------
// Variant: box_filter / abstract
// ---------------------------------------------------------------------------

static void BM_UC2_box_filter_abstract_unspecialized(benchmark::State& state) {
    int64_t n_tiles = state.range(0);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        for (int64_t t = 0; t < n_tiles; ++t)
            box_filter_abstract_unspecialized(g_src.data(), g_dst.data(), TILE_W, TILE_H, 2);
        benchmark::DoNotOptimize(g_dst.data());
    }
}

static void BM_UC2_box_filter_abstract_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_box_filter_abstract_specialized(TILE_W, TILE_H, 2));
    }
}

static void BM_UC2_box_filter_abstract_specialized_exec(benchmark::State& state) {
    int64_t n_tiles = state.range(0);
    auto spec = create_box_filter_abstract_specialized(TILE_W, TILE_H, 2);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        for (int64_t t = 0; t < n_tiles; ++t)
            spec(g_src.data(), g_dst.data());
        benchmark::DoNotOptimize(g_dst.data());
    }
}

// ---------------------------------------------------------------------------
// Variant: edge_detection / abstract
// ---------------------------------------------------------------------------

static void BM_UC2_edge_detection_abstract_unspecialized(benchmark::State& state) {
    int64_t n_tiles = state.range(0);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        for (int64_t t = 0; t < n_tiles; ++t)
            edge_detection_abstract_unspecialized(g_src.data(), g_dst.data(), TILE_W, TILE_H);
        benchmark::DoNotOptimize(g_dst.data());
    }
}

static void BM_UC2_edge_detection_abstract_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_edge_detection_abstract_specialized(TILE_W, TILE_H));
    }
}

static void BM_UC2_edge_detection_abstract_specialized_exec(benchmark::State& state) {
    int64_t n_tiles = state.range(0);
    auto spec = create_edge_detection_abstract_specialized(TILE_W, TILE_H);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        for (int64_t t = 0; t < n_tiles; ++t)
            spec(g_src.data(), g_dst.data());
        benchmark::DoNotOptimize(g_dst.data());
    }
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// JIT analysis benchmarks
// ---------------------------------------------------------------------------

static void BM_UC2_sg_low_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_conv_specialized(TILE_W, TILE_H);
    });
}

static void BM_UC2_box_filter_low_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_box_filter_low_specialized(TILE_W, TILE_H, 2);
    });
}

static void BM_UC2_edge_detection_low_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_edge_detection_low_specialized(TILE_W, TILE_H);
    });
}

static void BM_UC2_sg_tradeoff_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_separable_gaussian_tradeoff_specialized(TILE_W, TILE_H, g_kernel_coeffs, 5);
    });
}

static void BM_UC2_box_filter_tradeoff_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_box_filter_tradeoff_specialized(TILE_W, TILE_H, 2);
    });
}

static void BM_UC2_edge_detection_tradeoff_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_edge_detection_tradeoff_specialized(TILE_W, TILE_H);
    });
}

static void BM_UC2_sg_abstract_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_separable_gaussian_abstract_specialized(TILE_W, TILE_H, g_kernel_coeffs, 5);
    });
}

static void BM_UC2_box_filter_abstract_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_box_filter_abstract_specialized(TILE_W, TILE_H, 2);
    });
}

static void BM_UC2_edge_detection_abstract_jit_analysis(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkLambdaJITAnalysis(state, [&] {
        return create_edge_detection_abstract_specialized(TILE_W, TILE_H);
    });
}

// Benchmark registration macros
#define UC2_JIT_ANALYSIS_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC2_sg_low_jit_analysis)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_low_jit_analysis)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_low_jit_analysis)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_low_jit_analysis)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_low_jit_analysis)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_low_jit_analysis)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_low_jit_analysis)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_low_jit_analysis)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_low_jit_analysis)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_low_jit_analysis)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_low_jit_analysis)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_low_jit_analysis)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_tradeoff_jit_analysis)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_tradeoff_jit_analysis)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_tradeoff_jit_analysis)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_tradeoff_jit_analysis)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_tradeoff_jit_analysis)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_tradeoff_jit_analysis)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_tradeoff_jit_analysis)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_tradeoff_jit_analysis)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_tradeoff_jit_analysis)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_tradeoff_jit_analysis)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_tradeoff_jit_analysis)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_tradeoff_jit_analysis)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_abstract_jit_analysis)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_abstract_jit_analysis)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_abstract_jit_analysis)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_abstract_jit_analysis)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_abstract_jit_analysis)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_abstract_jit_analysis)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_abstract_jit_analysis)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_abstract_jit_analysis)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_abstract_jit_analysis)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_abstract_jit_analysis)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_abstract_jit_analysis)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_abstract_jit_analysis)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(setup_uc2)->Teardown(teardown_uc2);

// ---------------------------------------------------------------------------

#define UC2_BENCHMARK_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC2_unspecialized)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_unspecialized)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_unspecialized)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_unspecialized)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_jit_overhead)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:SMALL;t:jit_overhead;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_jit_overhead)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_jit_overhead)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:LARGE;t:jit_overhead;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_jit_overhead)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_specialized_exec)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_specialized_exec)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_specialized_exec)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_specialized_exec)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
\
BENCHMARK(BM_UC2_box_filter_low_unspecialized)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_low_unspecialized)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_low_unspecialized)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_low_unspecialized)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_low_jit_overhead)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:SMALL;t:jit_overhead;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_low_jit_overhead)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_low_jit_overhead)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:LARGE;t:jit_overhead;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_low_jit_overhead)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_low_specialized_exec)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_low_specialized_exec)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_low_specialized_exec)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_low_specialized_exec)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
\
BENCHMARK(BM_UC2_edge_detection_low_unspecialized)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_low_unspecialized)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_low_unspecialized)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_low_unspecialized)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_low_jit_overhead)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:SMALL;t:jit_overhead;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_low_jit_overhead)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_low_jit_overhead)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:LARGE;t:jit_overhead;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_low_jit_overhead)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_low_specialized_exec)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_low_specialized_exec)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_low_specialized_exec)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_low_specialized_exec)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
\
BENCHMARK(BM_UC2_sg_tradeoff_unspecialized)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_tradeoff_unspecialized)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_tradeoff_unspecialized)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_tradeoff_unspecialized)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_tradeoff_jit_overhead)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_tradeoff_jit_overhead)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_tradeoff_jit_overhead)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_tradeoff_jit_overhead)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_tradeoff_specialized_exec)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_tradeoff_specialized_exec)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_tradeoff_specialized_exec)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_tradeoff_specialized_exec)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
\
BENCHMARK(BM_UC2_box_filter_tradeoff_unspecialized)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_tradeoff_unspecialized)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_tradeoff_unspecialized)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_tradeoff_unspecialized)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_tradeoff_jit_overhead)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_tradeoff_jit_overhead)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_tradeoff_jit_overhead)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_tradeoff_jit_overhead)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_tradeoff_specialized_exec)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_tradeoff_specialized_exec)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_tradeoff_specialized_exec)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_tradeoff_specialized_exec)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
\
BENCHMARK(BM_UC2_edge_detection_tradeoff_unspecialized)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_tradeoff_unspecialized)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_tradeoff_unspecialized)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_tradeoff_unspecialized)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_tradeoff_jit_overhead)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_tradeoff_jit_overhead)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_tradeoff_jit_overhead)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_tradeoff_jit_overhead)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_tradeoff_specialized_exec)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_tradeoff_specialized_exec)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_tradeoff_specialized_exec)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_tradeoff_specialized_exec)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
\
BENCHMARK(BM_UC2_sg_abstract_unspecialized)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_abstract_unspecialized)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_abstract_unspecialized)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_abstract_unspecialized)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_abstract_jit_overhead)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:SMALL;t:jit_overhead;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_abstract_jit_overhead)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_abstract_jit_overhead)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:LARGE;t:jit_overhead;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_abstract_jit_overhead)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_abstract_specialized_exec)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_abstract_specialized_exec)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_abstract_specialized_exec)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_sg_abstract_specialized_exec)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
\
BENCHMARK(BM_UC2_box_filter_abstract_unspecialized)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_abstract_unspecialized)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_abstract_unspecialized)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_abstract_unspecialized)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_abstract_jit_overhead)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:SMALL;t:jit_overhead;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_abstract_jit_overhead)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_abstract_jit_overhead)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:LARGE;t:jit_overhead;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_abstract_jit_overhead)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_abstract_specialized_exec)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_abstract_specialized_exec)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_abstract_specialized_exec)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_box_filter_abstract_specialized_exec)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
\
BENCHMARK(BM_UC2_edge_detection_abstract_unspecialized)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_abstract_unspecialized)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_abstract_unspecialized)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_abstract_unspecialized)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_abstract_jit_overhead)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:SMALL;t:jit_overhead;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_abstract_jit_overhead)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_abstract_jit_overhead)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:LARGE;t:jit_overhead;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_abstract_jit_overhead)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_abstract_specialized_exec)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_abstract_specialized_exec)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_abstract_specialized_exec)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc2)->Teardown(teardown_uc2); \
BENCHMARK(BM_UC2_edge_detection_abstract_specialized_exec)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc2)->Teardown(teardown_uc2);

#ifdef ALL_BENCHMARKS_BUILD
UC2_BENCHMARK_SPEC(
    Arg(1),
    Arg(6),
    Arg(58),
    Arg(58)
)
UC2_JIT_ANALYSIS_SPEC(Arg(1), Arg(6), Arg(58), Arg(58))
#else
UC2_BENCHMARK_SPEC(
    Arg(1),
    Arg(6),
    Arg(58),
    Arg(345)
)
UC2_JIT_ANALYSIS_SPEC(Arg(1), Arg(6), Arg(58), Arg(345))
#endif

#ifndef ALL_BENCHMARKS_BUILD
int main(int argc, char** argv) {
    validate_conv_specialized();
    validate_box_filter_low_specialized();
    validate_edge_detection_low_specialized();
    validate_separable_gaussian_tradeoff_specialized();
    validate_box_filter_tradeoff_specialized();
    validate_edge_detection_tradeoff_specialized();
    validate_separable_gaussian_abstract_specialized();
    validate_box_filter_abstract_specialized();
    validate_edge_detection_abstract_specialized();

    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    if (benchmark::GetBenchmarkFilter() == "")
        benchmark::SetBenchmarkFilter("s:SMALL|s:MEDIUM|s:LARGE");
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
#endif // ALL_BENCHMARKS_BUILD
