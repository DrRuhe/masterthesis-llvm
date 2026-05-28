#include "UC2Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <algorithm>
#include <vector>

// BoxFilter struct captures the radius as a specialization constant.
struct BoxFilter {
    int radius;

    void apply(const float* src, float* dst, int width, int height) const {
        const int diam = 2 * radius + 1;
        const float inv_area = 1.0f / (float)(diam * diam);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float sum = 0.0f;
                for (int ky = -radius; ky <= radius; ++ky) {
                    int sy = std::max(0, std::min(height - 1, y + ky));
                    for (int kx = -radius; kx <= radius; ++kx) {
                        int sx = std::max(0, std::min(width - 1, x + kx));
                        sum += src[sy * width + sx];
                    }
                }
                dst[y * width + x] = sum * inv_area;
            }
        }
    }
};

void box_filter_tradeoff_unspecialized(const float* src, float* dst,
                                       int width, int height, int radius) {
    BoxFilter bf{radius};
    bf.apply(src, dst, width, height);
}

BoxFilterTradeoffSpecialized create_box_filter_tradeoff_specialized(
        int width, int height, int radius) {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    BoxFilter bf{radius};
    auto lam = [bf, width, height](const float* src, float* dst) {
        bf.apply(src, dst, width, height);
    };
    return RS->specializeLambda<void>(lam);
}

void validate_box_filter_tradeoff_specialized() {
    constexpr int W = 64, H = 64, R = 2;
    std::vector<float> src(W * H, 1.0f);
    std::vector<float> ref_dst(W * H, 0.0f);
    std::vector<float> spec_dst(W * H, 0.0f);

    BoxFilter ref_bf{R};
    ref_bf.apply(src.data(), ref_dst.data(), W, H);

    auto spec = create_box_filter_tradeoff_specialized(W, H, R);
    spec(src.data(), spec_dst.data());

    constexpr float kTol = 1e-4f;
    for (int i = 0; i < W * H; ++i) {
        float diff = ref_dst[i] - spec_dst[i];
        if (diff < -kTol || diff > kTol)
            throw clangRuntimeSpecializer::ClangRuntimeSpecializerError(
                "validate_box_filter_tradeoff_specialized: element-wise mismatch");
    }
    clangRuntimeSpecializer::ClangRuntimeSpecializer::log(
        clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Info,
        "validate_box_filter_tradeoff_specialized passed");
}
