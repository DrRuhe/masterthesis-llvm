#include "UC2Kernels.h"
#include "ClangRuntimeSpecializerBenchmark.h"
#include <benchmark/benchmark.h>
#include <random>
#include <vector>

static constexpr int IMG_WIDTH  = 1920;
static constexpr int IMG_HEIGHT = 1080;
static constexpr int N_PIXELS   = IMG_WIDTH * IMG_HEIGHT;

// Global datasets — initialized once before benchmarks run.
static std::vector<float> g_src(N_PIXELS, 0.0f);
static std::vector<float> g_dst(N_PIXELS, 0.0f);

// BM_UC2_unspecialized — baseline: direct convolve2d call each iteration.
static void BM_UC2_unspecialized(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        convolve2d(g_src.data(), g_dst.data(), IMG_WIDTH, IMG_HEIGHT, g_kernel_coeffs, 5);
        benchmark::DoNotOptimize(g_dst.data());
    }
}
BENCHMARK(BM_UC2_unspecialized)
    ->Name("BM_g:uc2_conv;n:gaussian5x5;t:unspecialized;");

// BM_UC2_jit_overhead — measures JIT compilation cost (single iteration).
static void BM_UC2_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(create_conv_specialized());
    }
}
BENCHMARK(BM_UC2_jit_overhead)
    ->Name("BM_g:uc2_conv;n:gaussian5x5;t:jit_overhead;")
    ->Iterations(1);

// BM_UC2_specialized_exec — compile once before loop, measure execution only.
static void BM_UC2_specialized_exec(benchmark::State& state) {
    auto spec = create_conv_specialized();
    for (auto _ : state) {
        benchmark::DoNotOptimize(g_src.data());
        spec(g_src.data(), g_dst.data());
        benchmark::DoNotOptimize(g_dst.data());
    }
}
BENCHMARK(BM_UC2_specialized_exec)
    ->Name("BM_g:uc2_conv;n:gaussian5x5;t:specialized_exec;");

int main(int argc, char** argv) {
    // Initialize source image with random floats in [0, 1].
    {
        std::mt19937 rng(42);
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        for (auto& v : g_src)
            v = dist(rng);
    }

    // Validate correctness before running benchmarks.
    validate_conv_specialized();

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
