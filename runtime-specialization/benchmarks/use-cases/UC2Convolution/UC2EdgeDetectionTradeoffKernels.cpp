#include "UC2Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <algorithm>
#include <cmath>
#include <vector>

// SobelFilter with hardcoded coefficient arrays as static constexpr members.
struct SobelFilter {
    static constexpr float GX[9] = {
        -1.0f, 0.0f, 1.0f,
        -2.0f, 0.0f, 2.0f,
        -1.0f, 0.0f, 1.0f
    };
    static constexpr float GY[9] = {
        -1.0f, -2.0f, -1.0f,
         0.0f,  0.0f,  0.0f,
         1.0f,  2.0f,  1.0f
    };

    // Full image processed — no early exit per FR-011.
    void detect(const float* src, float* dst, int width, int height) const {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float p[9];
                int idx = 0;
                for (int ky = -1; ky <= 1; ++ky) {
                    int sy = std::max(0, std::min(height - 1, y + ky));
                    for (int kx = -1; kx <= 1; ++kx) {
                        int sx = std::max(0, std::min(width - 1, x + kx));
                        p[idx++] = src[sy * width + sx];
                    }
                }
                float gx = 0.0f, gy = 0.0f;
                for (int k = 0; k < 9; ++k) {
                    gx += GX[k] * p[k];
                    gy += GY[k] * p[k];
                }
                dst[y * width + x] = std::sqrt(gx * gx + gy * gy);
            }
        }
    }
};

EdgeDetectionTradeoffSpecialized create_edge_detection_tradeoff_specialized(
        int width, int height) {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    SobelFilter sf{};
    auto lam = [sf, width, height](const float* src, float* dst) {
        sf.detect(src, dst, width, height);
    };
    return RS->specializeLambda<void>(lam);
}

void validate_edge_detection_tradeoff_specialized() {
    constexpr int W = 64, H = 64;
    std::vector<float> src(W * H);
    for (int i = 0; i < W * H; ++i)
        src[i] = (float)(i % 256) / 255.0f;

    std::vector<float> ref_dst(W * H, 0.0f);
    std::vector<float> spec_dst(W * H, 0.0f);

    SobelFilter ref_sf{};
    ref_sf.detect(src.data(), ref_dst.data(), W, H);

    auto spec = create_edge_detection_tradeoff_specialized(W, H);
    spec(src.data(), spec_dst.data());

    constexpr float kTol = 1e-4f;
    for (int i = 0; i < W * H; ++i) {
        float diff = ref_dst[i] - spec_dst[i];
        if (diff < -kTol || diff > kTol)
            throw clangRuntimeSpecializer::ClangRuntimeSpecializerError(
                "validate_edge_detection_tradeoff_specialized: element-wise mismatch");
    }
    clangRuntimeSpecializer::ClangRuntimeSpecializer::log(
        clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Info,
        "validate_edge_detection_tradeoff_specialized passed");
}
