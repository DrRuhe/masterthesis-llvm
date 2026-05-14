#include "UC2Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <algorithm>
#include <cmath>
#include <vector>

// Sobel kernels (row-major, 3x3):
//   Gx = [[-1,0,1],[-2,0,2],[-1,0,1]]
//   Gy = [[-1,-2,-1],[0,0,0],[1,2,1]]
// sobel_edge_detect: full image processed — no early exit per FR-011.
void sobel_edge_detect(const float* src, float* dst, int width, int height) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Sample the 3x3 neighbourhood with border clamping.
            float p[3][3];
            for (int ky = -1; ky <= 1; ++ky) {
                int sy = std::max(0, std::min(height - 1, y + ky));
                for (int kx = -1; kx <= 1; ++kx) {
                    int sx = std::max(0, std::min(width - 1, x + kx));
                    p[ky + 1][kx + 1] = src[sy * width + sx];
                }
            }
            float gx = -p[0][0] + p[0][2]
                       - 2.0f * p[1][0] + 2.0f * p[1][2]
                       - p[2][0] + p[2][2];
            float gy = -p[0][0] - 2.0f * p[0][1] - p[0][2]
                       + p[2][0] + 2.0f * p[2][1] + p[2][2];
            dst[y * width + x] = std::sqrt(gx * gx + gy * gy);
        }
    }
}

EdgeDetectionLowSpecialized create_edge_detection_low_specialized(int width, int height) {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    auto lam = [width, height](const float* src, float* dst) {
        sobel_edge_detect(src, dst, width, height);
    };
    return RS->specializeLambda<void>(lam);
}

void validate_edge_detection_low_specialized() {
    constexpr int W = 64, H = 64;
    std::vector<float> src(W * H);
    for (int i = 0; i < W * H; ++i)
        src[i] = (float)(i % 256) / 255.0f;

    std::vector<float> ref_dst(W * H, 0.0f);
    std::vector<float> spec_dst(W * H, 0.0f);

    sobel_edge_detect(src.data(), ref_dst.data(), W, H);

    auto spec = create_edge_detection_low_specialized(W, H);
    spec(src.data(), spec_dst.data());

    constexpr float kTol = 1e-4f;
    for (int i = 0; i < W * H; ++i) {
        float diff = ref_dst[i] - spec_dst[i];
        if (diff < -kTol || diff > kTol)
            throw clangRuntimeSpecializer::ClangRuntimeSpecializerError(
                "validate_edge_detection_low_specialized: element-wise mismatch");
    }
    clangRuntimeSpecializer::ClangRuntimeSpecializer::log(
        clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Info,
        "validate_edge_detection_low_specialized passed");
}
