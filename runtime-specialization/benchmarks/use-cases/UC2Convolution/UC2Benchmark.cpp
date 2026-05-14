#include "UC2Kernels.h"
#include "ClangRuntimeSpecializerBenchmark.h"
#include <benchmark/benchmark.h>
#include <random>
#include <vector>

static constexpr int IMG_WIDTH_MAX  = 7680;
static constexpr int IMG_HEIGHT_MAX = 4320;
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

#define UC2_BENCHMARK_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC2_unspecialized)->Name("BM_g:uc2_conv;n:gaussian5x5;s:SMALL;t:unspecialized;")->SMALL; \
BENCHMARK(BM_UC2_unspecialized)->Name("BM_g:uc2_conv;n:gaussian5x5;s:MEDIUM;t:unspecialized;")->MEDIUM; \
BENCHMARK(BM_UC2_unspecialized)->Name("BM_g:uc2_conv;n:gaussian5x5;s:LARGE;t:unspecialized;")->LARGE; \
BENCHMARK(BM_UC2_unspecialized)->Name("BM_g:uc2_conv;n:gaussian5x5;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE; \
BENCHMARK(BM_UC2_jit_overhead)->Name("BM_g:uc2_conv;n:gaussian5x5;s:SMALL;t:jit_overhead;")->SMALL; \
BENCHMARK(BM_UC2_jit_overhead)->Name("BM_g:uc2_conv;n:gaussian5x5;s:MEDIUM;t:jit_overhead;")->MEDIUM; \
BENCHMARK(BM_UC2_jit_overhead)->Name("BM_g:uc2_conv;n:gaussian5x5;s:LARGE;t:jit_overhead;")->LARGE; \
BENCHMARK(BM_UC2_jit_overhead)->Name("BM_g:uc2_conv;n:gaussian5x5;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE; \
BENCHMARK(BM_UC2_specialized_exec)->Name("BM_g:uc2_conv;n:gaussian5x5;s:SMALL;t:specialized_exec;")->SMALL; \
BENCHMARK(BM_UC2_specialized_exec)->Name("BM_g:uc2_conv;n:gaussian5x5;s:MEDIUM;t:specialized_exec;")->MEDIUM; \
BENCHMARK(BM_UC2_specialized_exec)->Name("BM_g:uc2_conv;n:gaussian5x5;s:LARGE;t:specialized_exec;")->LARGE; \
BENCHMARK(BM_UC2_specialized_exec)->Name("BM_g:uc2_conv;n:gaussian5x5;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE;

UC2_BENCHMARK_SPEC(
    Args({640, 360}),
    Args({1920, 1080}),
    Args({3840, 2160}),
    Args({7680, 4320})
)

#ifndef ALL_BENCHMARKS_BUILD
int main(int argc, char** argv) {
    validate_conv_specialized();

    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    if (benchmark::GetBenchmarkFilter() == "")
        benchmark::SetBenchmarkFilter("s:SMALL|s:MEDIUM|s:LARGE");
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
#endif // ALL_BENCHMARKS_BUILD
