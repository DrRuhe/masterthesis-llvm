#include "UC2Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <algorithm>
#include <vector>

// SeparableFilter wraps the two-pass separable convolution. Coefficients are
// copied into a fixed-size array by value so they are JIT constants.
struct SeparableFilter {
    static constexpr int kMaxKsize = 16;
    float coeffs[kMaxKsize];
    int ksize;

    void apply(const float* src, float* dst, int width, int height) const {
        std::vector<float> tmp(width * height);
        const int half = ksize / 2;

        // Horizontal pass.
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float sum = 0.0f;
                for (int k = 0; k < ksize; ++k) {
                    int sx = std::max(0, std::min(width - 1, x + k - half));
                    sum += src[y * width + sx] * coeffs[k];
                }
                tmp[y * width + x] = sum;
            }
        }

        // Vertical pass.
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float sum = 0.0f;
                for (int k = 0; k < ksize; ++k) {
                    int sy = std::max(0, std::min(height - 1, y + k - half));
                    sum += tmp[sy * width + x] * coeffs[k];
                }
                dst[y * width + x] = sum;
            }
        }
    }
};

SeparableGaussianTradeoffSpecialized create_separable_gaussian_tradeoff_specialized(
        int width, int height, const float* coeffs, int ksize) {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    SeparableFilter sf{};
    sf.ksize = ksize;
    for (int i = 0; i < ksize; ++i)
        sf.coeffs[i] = coeffs[i];

    auto lam = [sf, width, height](const float* src, float* dst) {
        sf.apply(src, dst, width, height);
    };
    return RS->specializeLambda<void>(lam);
}

void validate_separable_gaussian_tradeoff_specialized() {
    constexpr int W = 64, H = 64, K = 5;
    float kcoeffs[K] = {0.0625f, 0.25f, 0.375f, 0.25f, 0.0625f};

    std::vector<float> src(W * H, 1.0f);
    std::vector<float> ref_dst(W * H, 0.0f);
    std::vector<float> spec_dst(W * H, 0.0f);

    SeparableFilter ref_sf{};
    ref_sf.ksize = K;
    for (int i = 0; i < K; ++i) ref_sf.coeffs[i] = kcoeffs[i];
    ref_sf.apply(src.data(), ref_dst.data(), W, H);

    auto spec = create_separable_gaussian_tradeoff_specialized(W, H, kcoeffs, K);
    spec(src.data(), spec_dst.data());

    constexpr float kTol = 1e-4f;
    for (int i = 0; i < W * H; ++i) {
        float diff = ref_dst[i] - spec_dst[i];
        if (diff < -kTol || diff > kTol)
            throw clangRuntimeSpecializer::ClangRuntimeSpecializerError(
                "validate_separable_gaussian_tradeoff_specialized: element-wise mismatch");
    }
    clangRuntimeSpecializer::ClangRuntimeSpecializer::log(
        clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Info,
        "validate_separable_gaussian_tradeoff_specialized passed");
}
