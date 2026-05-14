#include "UC2Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <algorithm>
#include <vector>

// Abstract spatial filter interface. Both base and concrete subclass live in
// this TU so the vtable pointer is a JIT constant when captured by value.
struct SpatialFilter {
    virtual void apply(const float* src, float* dst, int width, int height) = 0;
    virtual ~SpatialFilter() = default;
};

// BoxSpatialFilter: subclass name differs from T034's BoxFilter to avoid ODR issues
// across the linked binary. Full image processed — no early exit per FR-011.
struct BoxSpatialFilter : SpatialFilter {
    int radius;

    explicit BoxSpatialFilter(int r) : radius(r) {}

    void apply(const float* src, float* dst, int width, int height) override {
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

BoxFilterAbstractSpecialized create_box_filter_abstract_specialized(
        int width, int height, int radius) {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    BoxSpatialFilter bsf(radius);

    // Capture BoxSpatialFilter BY VALUE so vtable is a JIT constant.
    auto lam = [bsf, width, height](const float* src, float* dst) mutable {
        bsf.apply(src, dst, width, height);
    };
    return RS->specializeLambda<void>(lam);
}

void validate_box_filter_abstract_specialized() {
    constexpr int W = 64, H = 64, R = 2;
    std::vector<float> src(W * H, 1.0f);
    std::vector<float> ref_dst(W * H, 0.0f);
    std::vector<float> spec_dst(W * H, 0.0f);

    BoxSpatialFilter ref_bsf(R);
    ref_bsf.apply(src.data(), ref_dst.data(), W, H);

    auto spec = create_box_filter_abstract_specialized(W, H, R);
    spec(src.data(), spec_dst.data());

    constexpr float kTol = 1e-4f;
    for (int i = 0; i < W * H; ++i) {
        float diff = ref_dst[i] - spec_dst[i];
        if (diff < -kTol || diff > kTol)
            throw clangRuntimeSpecializer::ClangRuntimeSpecializerError(
                "validate_box_filter_abstract_specialized: element-wise mismatch");
    }
    clangRuntimeSpecializer::ClangRuntimeSpecializer::log(
        clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Info,
        "validate_box_filter_abstract_specialized passed");
}
