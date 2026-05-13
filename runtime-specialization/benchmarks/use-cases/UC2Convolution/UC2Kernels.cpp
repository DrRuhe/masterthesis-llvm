#include "UC2Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <algorithm>
#include <vector>

// 5-tap Gaussian kernel coefficients — non-static so JIT can resolve the address.
float g_kernel_coeffs[5] = {0.0625f, 0.25f, 0.375f, 0.25f, 0.0625f};

// CRITICAL: convolve2d must live in the same TU as the lambda factory
// so the JIT module clone contains both for full inlining.
void convolve2d(const float* src, float* dst, int width, int height,
                const float* kernel_coeffs, int ksize) {
    std::vector<float> tmp(width * height);
    const int half = ksize / 2;

    // Horizontal pass: for each row, convolve along x.
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float sum = 0.0f;
            for (int k = 0; k < ksize; ++k) {
                int sx = std::max(0, std::min(width - 1, x + k - half));
                sum += src[y * width + sx] * kernel_coeffs[k];
            }
            tmp[y * width + x] = sum;
        }
    }

    // Vertical pass: for each column, convolve along y.
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float sum = 0.0f;
            for (int k = 0; k < ksize; ++k) {
                int sy = std::max(0, std::min(height - 1, y + k - half));
                sum += tmp[sy * width + x] * kernel_coeffs[k];
            }
            dst[y * width + x] = sum;
        }
    }
}

ConvSpecialized create_conv_specialized() {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    const float* kcoeffs = g_kernel_coeffs;
    const int ksize = 5, width = 1920, height = 1080;
    auto lam = [kcoeffs, ksize, width, height](const float* src, float* dst) {
        convolve2d(src, dst, width, height, kcoeffs, ksize);
    };
    return RS->specializeLambda<void>(lam);
}

void validate_conv_specialized() {
    // Use a smaller image so the specialized lambda can be validated quickly.
    // The kernel TU is compiled at -O0, so the IRDumpingPass sees all function
    // arguments and can rewrite each specializeLambda call site correctly.
    constexpr int W = 64, H = 64;
    std::vector<float> src(W * H, 1.0f);
    std::vector<float> ref_dst(W * H, 0.0f);
    std::vector<float> spec_dst(W * H, 0.0f);

    // Reference: run convolve2d directly.
    convolve2d(src.data(), ref_dst.data(), W, H, g_kernel_coeffs, 5);

    // Specialized: create a 64x64 specialized lambda and run it.
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    const float* kcoeffs = g_kernel_coeffs;
    const int ksize = 5, width = W, height = H;
    auto lam = [kcoeffs, ksize, width, height](const float* s, float* d) {
        convolve2d(s, d, width, height, kcoeffs, ksize);
    };
    auto spec = RS->specializeLambda<void>(lam);
    spec(src.data(), spec_dst.data());

    // Verify element-wise agreement between reference and specialized results.
    constexpr float kTol = 1e-4f;
    for (int i = 0; i < W * H; ++i) {
        float diff = ref_dst[i] - spec_dst[i];
        if (diff < -kTol || diff > kTol) {
            throw clangRuntimeSpecializer::ClangRuntimeSpecializerError(
                "validate_conv_specialized: element-wise mismatch");
        }
    }
    clangRuntimeSpecializer::ClangRuntimeSpecializer::log(
        clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Info,
        "validate_conv_specialized passed");
}
