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

void separable_gaussian_tradeoff_unspecialized(const float* src, float* dst,
                                               int width, int height,
                                               const float* coeffs, int ksize) {
    SeparableFilter sf{};
    sf.ksize = ksize;
    for (int i = 0; i < ksize; ++i) sf.coeffs[i] = coeffs[i];
    sf.apply(src, dst, width, height);
}

SeparableGaussianTradeoffSpecialized create_separable_gaussian_tradeoff_specialized(
        int width, int height, const float* coeffs, int ksize) {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    const float c0 = ksize > 0 ? coeffs[0] : 0.0f;
    const float c1 = ksize > 1 ? coeffs[1] : 0.0f;
    const float c2 = ksize > 2 ? coeffs[2] : 0.0f;
    const float c3 = ksize > 3 ? coeffs[3] : 0.0f;
    const float c4 = ksize > 4 ? coeffs[4] : 0.0f;
    const float c5 = ksize > 5 ? coeffs[5] : 0.0f;
    const float c6 = ksize > 6 ? coeffs[6] : 0.0f;
    const float c7 = ksize > 7 ? coeffs[7] : 0.0f;
    const float c8 = ksize > 8 ? coeffs[8] : 0.0f;
    const float c9 = ksize > 9 ? coeffs[9] : 0.0f;
    const float c10 = ksize > 10 ? coeffs[10] : 0.0f;
    const float c11 = ksize > 11 ? coeffs[11] : 0.0f;
    const float c12 = ksize > 12 ? coeffs[12] : 0.0f;
    const float c13 = ksize > 13 ? coeffs[13] : 0.0f;
    const float c14 = ksize > 14 ? coeffs[14] : 0.0f;
    const float c15 = ksize > 15 ? coeffs[15] : 0.0f;
    auto lam = [width, height, ksize, c0, c1, c2, c3, c4, c5, c6, c7,
                c8, c9, c10, c11, c12, c13, c14, c15](const float* src, float* dst) {
        // Reconstruct the filter inside the lambda so the specialized code
        // does not depend on factory-frame or caller-frame coefficient storage.
        const float coeff_storage[SeparableFilter::kMaxKsize] = {
            c0, c1, c2, c3, c4, c5, c6, c7,
            c8, c9, c10, c11, c12, c13, c14, c15
        };
        SeparableFilter sf{};
        sf.ksize = ksize;
        for (int i = 0; i < ksize; ++i)
            sf.coeffs[i] = coeff_storage[i];
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
