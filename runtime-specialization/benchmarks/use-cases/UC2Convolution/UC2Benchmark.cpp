#include "UC2Kernels.h"
#include "ClangRuntimeSpecializerBenchmark.h"
#include <benchmark/benchmark.h>
#include <random>
#include <vector>

static constexpr int IMG_WIDTH_MAX  = 25920;
static constexpr int IMG_HEIGHT_MAX = 25920;
static constexpr int N_PIXELS_MAX   = IMG_WIDTH_MAX * IMG_HEIGHT_MAX;

// Global buffers allocated to EXTRALARGE size.
// Benchmarks pass size-specific (width, height) via state.range(0/1).
static std::vector<float> g_src(N_PIXELS_MAX, 0.0f);
static std::vector<float> g_dst(N_PIXELS_MAX, 0.0f);

static struct UC2DataInit {
    UC2DataInit() {
        std::mt19937 rng(42);
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        for (auto& v : g_src) v = dist(rng);
    }
} g_uc2_init;

// ---------------------------------------------------------------------------
// Variant: separable_gaussian / low
// ---------------------------------------------------------------------------

static void BM_UC2_unspecialized(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        convolve2d(g_src.data(), g_dst.data(), width, height, g_kernel_coeffs, 5);
        benchmark::DoNotOptimize(g_dst.data());
    }
}

static void BM_UC2_jit_overhead(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    for (auto _ : state) {
        benchmark::DoNotOptimize(create_conv_specialized(width, height));
    }
}

static void BM_UC2_specialized_exec(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    auto spec = create_conv_specialized(width, height);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        spec(g_src.data(), g_dst.data());
        benchmark::DoNotOptimize(g_dst.data());
    }
}

// ---------------------------------------------------------------------------
// Variant: box_filter / low
// ---------------------------------------------------------------------------

static void BM_UC2_box_filter_low_unspecialized(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        box_filter(g_src.data(), g_dst.data(), width, height, 2);
        benchmark::DoNotOptimize(g_dst.data());
    }
}

static void BM_UC2_box_filter_low_jit_overhead(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    for (auto _ : state) {
        benchmark::DoNotOptimize(create_box_filter_low_specialized(width, height, 2));
    }
}

static void BM_UC2_box_filter_low_specialized_exec(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    auto spec = create_box_filter_low_specialized(width, height, 2);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        spec(g_src.data(), g_dst.data());
        benchmark::DoNotOptimize(g_dst.data());
    }
}

// ---------------------------------------------------------------------------
// Variant: edge_detection / low
// ---------------------------------------------------------------------------

static void BM_UC2_edge_detection_low_unspecialized(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        sobel_edge_detect(g_src.data(), g_dst.data(), width, height);
        benchmark::DoNotOptimize(g_dst.data());
    }
}

static void BM_UC2_edge_detection_low_jit_overhead(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    for (auto _ : state) {
        benchmark::DoNotOptimize(create_edge_detection_low_specialized(width, height));
    }
}

static void BM_UC2_edge_detection_low_specialized_exec(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    auto spec = create_edge_detection_low_specialized(width, height);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        spec(g_src.data(), g_dst.data());
        benchmark::DoNotOptimize(g_dst.data());
    }
}

// ---------------------------------------------------------------------------
// Variant: separable_gaussian / tradeoff
// ---------------------------------------------------------------------------

static void BM_UC2_sg_tradeoff_unspecialized(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        convolve2d(g_src.data(), g_dst.data(), width, height, g_kernel_coeffs, 5);
        benchmark::DoNotOptimize(g_dst.data());
    }
}

static void BM_UC2_sg_tradeoff_jit_overhead(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_separable_gaussian_tradeoff_specialized(width, height, g_kernel_coeffs, 5));
    }
}

static void BM_UC2_sg_tradeoff_specialized_exec(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    auto spec = create_separable_gaussian_tradeoff_specialized(width, height, g_kernel_coeffs, 5);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        spec(g_src.data(), g_dst.data());
        benchmark::DoNotOptimize(g_dst.data());
    }
}

// ---------------------------------------------------------------------------
// Variant: box_filter / tradeoff
// ---------------------------------------------------------------------------

static void BM_UC2_box_filter_tradeoff_unspecialized(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        box_filter(g_src.data(), g_dst.data(), width, height, 2);
        benchmark::DoNotOptimize(g_dst.data());
    }
}

static void BM_UC2_box_filter_tradeoff_jit_overhead(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_box_filter_tradeoff_specialized(width, height, 2));
    }
}

static void BM_UC2_box_filter_tradeoff_specialized_exec(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    auto spec = create_box_filter_tradeoff_specialized(width, height, 2);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        spec(g_src.data(), g_dst.data());
        benchmark::DoNotOptimize(g_dst.data());
    }
}

// ---------------------------------------------------------------------------
// Variant: edge_detection / tradeoff
// ---------------------------------------------------------------------------

static void BM_UC2_edge_detection_tradeoff_unspecialized(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        sobel_edge_detect(g_src.data(), g_dst.data(), width, height);
        benchmark::DoNotOptimize(g_dst.data());
    }
}

static void BM_UC2_edge_detection_tradeoff_jit_overhead(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_edge_detection_tradeoff_specialized(width, height));
    }
}

static void BM_UC2_edge_detection_tradeoff_specialized_exec(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    auto spec = create_edge_detection_tradeoff_specialized(width, height);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        spec(g_src.data(), g_dst.data());
        benchmark::DoNotOptimize(g_dst.data());
    }
}

// ---------------------------------------------------------------------------
// Variant: separable_gaussian / abstract
// ---------------------------------------------------------------------------

static void BM_UC2_sg_abstract_unspecialized(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        convolve2d(g_src.data(), g_dst.data(), width, height, g_kernel_coeffs, 5);
        benchmark::DoNotOptimize(g_dst.data());
    }
}

static void BM_UC2_sg_abstract_jit_overhead(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_separable_gaussian_abstract_specialized(width, height, g_kernel_coeffs, 5));
    }
}

static void BM_UC2_sg_abstract_specialized_exec(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    auto spec = create_separable_gaussian_abstract_specialized(
        width, height, g_kernel_coeffs, 5);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        spec(g_src.data(), g_dst.data());
        benchmark::DoNotOptimize(g_dst.data());
    }
}

// ---------------------------------------------------------------------------
// Variant: box_filter / abstract
// ---------------------------------------------------------------------------

static void BM_UC2_box_filter_abstract_unspecialized(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        box_filter(g_src.data(), g_dst.data(), width, height, 2);
        benchmark::DoNotOptimize(g_dst.data());
    }
}

static void BM_UC2_box_filter_abstract_jit_overhead(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_box_filter_abstract_specialized(width, height, 2));
    }
}

static void BM_UC2_box_filter_abstract_specialized_exec(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    auto spec = create_box_filter_abstract_specialized(width, height, 2);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        spec(g_src.data(), g_dst.data());
        benchmark::DoNotOptimize(g_dst.data());
    }
}

// ---------------------------------------------------------------------------
// Variant: edge_detection / abstract
// ---------------------------------------------------------------------------

static void BM_UC2_edge_detection_abstract_unspecialized(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        sobel_edge_detect(g_src.data(), g_dst.data(), width, height);
        benchmark::DoNotOptimize(g_dst.data());
    }
}

static void BM_UC2_edge_detection_abstract_jit_overhead(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            create_edge_detection_abstract_specialized(width, height));
    }
}

static void BM_UC2_edge_detection_abstract_specialized_exec(benchmark::State& state) {
    int width  = (int)state.range(0);
    int height = (int)state.range(1);
    auto spec = create_edge_detection_abstract_specialized(width, height);
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        spec(g_src.data(), g_dst.data());
        benchmark::DoNotOptimize(g_dst.data());
    }
}

// ---------------------------------------------------------------------------
// Benchmark registration macros
// ---------------------------------------------------------------------------

#define UC2_BENCHMARK_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC2_unspecialized)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:SMALL;t:unspecialized;")->SMALL; \
BENCHMARK(BM_UC2_unspecialized)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM; \
BENCHMARK(BM_UC2_unspecialized)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:LARGE;t:unspecialized;")->LARGE; \
BENCHMARK(BM_UC2_unspecialized)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE; \
BENCHMARK(BM_UC2_jit_overhead)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:SMALL;t:jit_overhead;")->SMALL; \
BENCHMARK(BM_UC2_jit_overhead)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM; \
BENCHMARK(BM_UC2_jit_overhead)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:LARGE;t:jit_overhead;")->LARGE; \
BENCHMARK(BM_UC2_jit_overhead)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE; \
BENCHMARK(BM_UC2_specialized_exec)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:SMALL;t:specialized_exec;")->SMALL; \
BENCHMARK(BM_UC2_specialized_exec)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM; \
BENCHMARK(BM_UC2_specialized_exec)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:LARGE;t:specialized_exec;")->LARGE; \
BENCHMARK(BM_UC2_specialized_exec)->Name("BM_g:uc2_conv;n:separable_gaussian;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE; \
\
BENCHMARK(BM_UC2_box_filter_low_unspecialized)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:SMALL;t:unspecialized;")->SMALL; \
BENCHMARK(BM_UC2_box_filter_low_unspecialized)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM; \
BENCHMARK(BM_UC2_box_filter_low_unspecialized)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:LARGE;t:unspecialized;")->LARGE; \
BENCHMARK(BM_UC2_box_filter_low_unspecialized)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE; \
BENCHMARK(BM_UC2_box_filter_low_jit_overhead)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:SMALL;t:jit_overhead;")->SMALL; \
BENCHMARK(BM_UC2_box_filter_low_jit_overhead)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM; \
BENCHMARK(BM_UC2_box_filter_low_jit_overhead)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:LARGE;t:jit_overhead;")->LARGE; \
BENCHMARK(BM_UC2_box_filter_low_jit_overhead)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE; \
BENCHMARK(BM_UC2_box_filter_low_specialized_exec)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:SMALL;t:specialized_exec;")->SMALL; \
BENCHMARK(BM_UC2_box_filter_low_specialized_exec)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM; \
BENCHMARK(BM_UC2_box_filter_low_specialized_exec)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:LARGE;t:specialized_exec;")->LARGE; \
BENCHMARK(BM_UC2_box_filter_low_specialized_exec)->Name("BM_g:uc2_conv;n:box_filter;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE; \
\
BENCHMARK(BM_UC2_edge_detection_low_unspecialized)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:SMALL;t:unspecialized;")->SMALL; \
BENCHMARK(BM_UC2_edge_detection_low_unspecialized)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM; \
BENCHMARK(BM_UC2_edge_detection_low_unspecialized)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:LARGE;t:unspecialized;")->LARGE; \
BENCHMARK(BM_UC2_edge_detection_low_unspecialized)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE; \
BENCHMARK(BM_UC2_edge_detection_low_jit_overhead)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:SMALL;t:jit_overhead;")->SMALL; \
BENCHMARK(BM_UC2_edge_detection_low_jit_overhead)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM; \
BENCHMARK(BM_UC2_edge_detection_low_jit_overhead)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:LARGE;t:jit_overhead;")->LARGE; \
BENCHMARK(BM_UC2_edge_detection_low_jit_overhead)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE; \
BENCHMARK(BM_UC2_edge_detection_low_specialized_exec)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:SMALL;t:specialized_exec;")->SMALL; \
BENCHMARK(BM_UC2_edge_detection_low_specialized_exec)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM; \
BENCHMARK(BM_UC2_edge_detection_low_specialized_exec)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:LARGE;t:specialized_exec;")->LARGE; \
BENCHMARK(BM_UC2_edge_detection_low_specialized_exec)->Name("BM_g:uc2_conv;n:edge_detection;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE; \
\
BENCHMARK(BM_UC2_sg_tradeoff_unspecialized)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL; \
BENCHMARK(BM_UC2_sg_tradeoff_unspecialized)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM; \
BENCHMARK(BM_UC2_sg_tradeoff_unspecialized)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE; \
BENCHMARK(BM_UC2_sg_tradeoff_unspecialized)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE; \
BENCHMARK(BM_UC2_sg_tradeoff_jit_overhead)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL; \
BENCHMARK(BM_UC2_sg_tradeoff_jit_overhead)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM; \
BENCHMARK(BM_UC2_sg_tradeoff_jit_overhead)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE; \
BENCHMARK(BM_UC2_sg_tradeoff_jit_overhead)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE; \
BENCHMARK(BM_UC2_sg_tradeoff_specialized_exec)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL; \
BENCHMARK(BM_UC2_sg_tradeoff_specialized_exec)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM; \
BENCHMARK(BM_UC2_sg_tradeoff_specialized_exec)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE; \
BENCHMARK(BM_UC2_sg_tradeoff_specialized_exec)->Name("BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE; \
\
BENCHMARK(BM_UC2_box_filter_tradeoff_unspecialized)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL; \
BENCHMARK(BM_UC2_box_filter_tradeoff_unspecialized)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM; \
BENCHMARK(BM_UC2_box_filter_tradeoff_unspecialized)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE; \
BENCHMARK(BM_UC2_box_filter_tradeoff_unspecialized)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE; \
BENCHMARK(BM_UC2_box_filter_tradeoff_jit_overhead)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL; \
BENCHMARK(BM_UC2_box_filter_tradeoff_jit_overhead)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM; \
BENCHMARK(BM_UC2_box_filter_tradeoff_jit_overhead)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE; \
BENCHMARK(BM_UC2_box_filter_tradeoff_jit_overhead)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE; \
BENCHMARK(BM_UC2_box_filter_tradeoff_specialized_exec)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL; \
BENCHMARK(BM_UC2_box_filter_tradeoff_specialized_exec)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM; \
BENCHMARK(BM_UC2_box_filter_tradeoff_specialized_exec)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE; \
BENCHMARK(BM_UC2_box_filter_tradeoff_specialized_exec)->Name("BM_g:uc2_conv;n:box_filter;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE; \
\
BENCHMARK(BM_UC2_edge_detection_tradeoff_unspecialized)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:SMALL;t:unspecialized;")->SMALL; \
BENCHMARK(BM_UC2_edge_detection_tradeoff_unspecialized)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:MEDIUM;t:unspecialized;")->MEDIUM; \
BENCHMARK(BM_UC2_edge_detection_tradeoff_unspecialized)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:LARGE;t:unspecialized;")->LARGE; \
BENCHMARK(BM_UC2_edge_detection_tradeoff_unspecialized)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE; \
BENCHMARK(BM_UC2_edge_detection_tradeoff_jit_overhead)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:SMALL;t:jit_overhead;")->SMALL; \
BENCHMARK(BM_UC2_edge_detection_tradeoff_jit_overhead)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:MEDIUM;t:jit_overhead;")->MEDIUM; \
BENCHMARK(BM_UC2_edge_detection_tradeoff_jit_overhead)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:LARGE;t:jit_overhead;")->LARGE; \
BENCHMARK(BM_UC2_edge_detection_tradeoff_jit_overhead)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE; \
BENCHMARK(BM_UC2_edge_detection_tradeoff_specialized_exec)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:SMALL;t:specialized_exec;")->SMALL; \
BENCHMARK(BM_UC2_edge_detection_tradeoff_specialized_exec)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:MEDIUM;t:specialized_exec;")->MEDIUM; \
BENCHMARK(BM_UC2_edge_detection_tradeoff_specialized_exec)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:LARGE;t:specialized_exec;")->LARGE; \
BENCHMARK(BM_UC2_edge_detection_tradeoff_specialized_exec)->Name("BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE; \
\
BENCHMARK(BM_UC2_sg_abstract_unspecialized)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:SMALL;t:unspecialized;")->SMALL; \
BENCHMARK(BM_UC2_sg_abstract_unspecialized)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM; \
BENCHMARK(BM_UC2_sg_abstract_unspecialized)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:LARGE;t:unspecialized;")->LARGE; \
BENCHMARK(BM_UC2_sg_abstract_unspecialized)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE; \
BENCHMARK(BM_UC2_sg_abstract_jit_overhead)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:SMALL;t:jit_overhead;")->SMALL; \
BENCHMARK(BM_UC2_sg_abstract_jit_overhead)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM; \
BENCHMARK(BM_UC2_sg_abstract_jit_overhead)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:LARGE;t:jit_overhead;")->LARGE; \
BENCHMARK(BM_UC2_sg_abstract_jit_overhead)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE; \
BENCHMARK(BM_UC2_sg_abstract_specialized_exec)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:SMALL;t:specialized_exec;")->SMALL; \
BENCHMARK(BM_UC2_sg_abstract_specialized_exec)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM; \
BENCHMARK(BM_UC2_sg_abstract_specialized_exec)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:LARGE;t:specialized_exec;")->LARGE; \
BENCHMARK(BM_UC2_sg_abstract_specialized_exec)->Name("BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE; \
\
BENCHMARK(BM_UC2_box_filter_abstract_unspecialized)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:SMALL;t:unspecialized;")->SMALL; \
BENCHMARK(BM_UC2_box_filter_abstract_unspecialized)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM; \
BENCHMARK(BM_UC2_box_filter_abstract_unspecialized)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:LARGE;t:unspecialized;")->LARGE; \
BENCHMARK(BM_UC2_box_filter_abstract_unspecialized)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE; \
BENCHMARK(BM_UC2_box_filter_abstract_jit_overhead)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:SMALL;t:jit_overhead;")->SMALL; \
BENCHMARK(BM_UC2_box_filter_abstract_jit_overhead)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM; \
BENCHMARK(BM_UC2_box_filter_abstract_jit_overhead)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:LARGE;t:jit_overhead;")->LARGE; \
BENCHMARK(BM_UC2_box_filter_abstract_jit_overhead)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE; \
BENCHMARK(BM_UC2_box_filter_abstract_specialized_exec)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:SMALL;t:specialized_exec;")->SMALL; \
BENCHMARK(BM_UC2_box_filter_abstract_specialized_exec)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM; \
BENCHMARK(BM_UC2_box_filter_abstract_specialized_exec)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:LARGE;t:specialized_exec;")->LARGE; \
BENCHMARK(BM_UC2_box_filter_abstract_specialized_exec)->Name("BM_g:uc2_conv;n:box_filter;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE; \
\
BENCHMARK(BM_UC2_edge_detection_abstract_unspecialized)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:SMALL;t:unspecialized;")->SMALL; \
BENCHMARK(BM_UC2_edge_detection_abstract_unspecialized)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:MEDIUM;t:unspecialized;")->MEDIUM; \
BENCHMARK(BM_UC2_edge_detection_abstract_unspecialized)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:LARGE;t:unspecialized;")->LARGE; \
BENCHMARK(BM_UC2_edge_detection_abstract_unspecialized)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE; \
BENCHMARK(BM_UC2_edge_detection_abstract_jit_overhead)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:SMALL;t:jit_overhead;")->SMALL; \
BENCHMARK(BM_UC2_edge_detection_abstract_jit_overhead)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:MEDIUM;t:jit_overhead;")->MEDIUM; \
BENCHMARK(BM_UC2_edge_detection_abstract_jit_overhead)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:LARGE;t:jit_overhead;")->LARGE; \
BENCHMARK(BM_UC2_edge_detection_abstract_jit_overhead)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE; \
BENCHMARK(BM_UC2_edge_detection_abstract_specialized_exec)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:SMALL;t:specialized_exec;")->SMALL; \
BENCHMARK(BM_UC2_edge_detection_abstract_specialized_exec)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:MEDIUM;t:specialized_exec;")->MEDIUM; \
BENCHMARK(BM_UC2_edge_detection_abstract_specialized_exec)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:LARGE;t:specialized_exec;")->LARGE; \
BENCHMARK(BM_UC2_edge_detection_abstract_specialized_exec)->Name("BM_g:uc2_conv;n:edge_detection;a:abstract;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE;

UC2_BENCHMARK_SPEC(
    Args({1440, 800}),
    Args({3840, 2880}),
    Args({10560, 10560}),
    Args({25920, 25920})
)

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
