#include "UC2Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <algorithm>
#include <cmath>
#include <vector>

// box_filter: applies a 2D box filter by averaging a (2r+1)x(2r+1) neighbourhood.
// Borders are clamped. Full image processed — no early exit per FR-011.
void box_filter(const float* src, float* dst, int width, int height, int radius) {
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

BoxFilterLowSpecialized create_box_filter_low_specialized(int width, int height, int radius) {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    auto lam = [width, height, radius](const float* src, float* dst) {
        box_filter(src, dst, width, height, radius);
    };
    return RS->specializeLambda<void>(lam);
}

void validate_box_filter_low_specialized() {
    constexpr int W = 64, H = 64, R = 2;
    std::vector<float> src(W * H, 1.0f);
    std::vector<float> ref_dst(W * H, 0.0f);
    std::vector<float> spec_dst(W * H, 0.0f);

    box_filter(src.data(), ref_dst.data(), W, H, R);

    auto spec = create_box_filter_low_specialized(W, H, R);
    spec(src.data(), spec_dst.data());

    constexpr float kTol = 1e-4f;
    for (int i = 0; i < W * H; ++i) {
        float diff = ref_dst[i] - spec_dst[i];
        if (diff < -kTol || diff > kTol)
            throw clangRuntimeSpecializer::ClangRuntimeSpecializerError(
                "validate_box_filter_low_specialized: element-wise mismatch");
    }
    clangRuntimeSpecializer::ClangRuntimeSpecializer::log(
        clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Info,
        "validate_box_filter_low_specialized passed");
}
