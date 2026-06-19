#include "UC2Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <algorithm>
#include <vector>

// Abstract filter kernel interface. Both base and subclass live in this TU
// so the vtable is a JIT constant when captured by value.
struct FilterKernel {
    virtual float weight(int offset) const = 0;
    virtual int radius() const = 0;

};

// GaussianKernel stores up to 5 coefficients by value in a fixed-size array.
struct GaussianKernel : FilterKernel {
    static constexpr int kMaxR = 2; // radius = 2 → ksize = 5
    float coeffs[2 * kMaxR + 1];
    int r;

    GaussianKernel(const float* c, int radius) : r(radius) {
        int ksz = 2 * radius + 1;
        for (int i = 0; i < ksz; ++i)
            coeffs[i] = c[i];
    }

    float weight(int offset) const override {
        // offset in [-r, r]; index 0 = -r
        return coeffs[offset + r];
    }
    int radius() const override { return r; }
};

// Convolver holds a GaussianKernel by value and applies the two-pass
// separable filter. Full image processed — no early exit per FR-011.
struct Convolver {
    GaussianKernel kern;

    void convolve(const float* src, float* dst, int width, int height) const {
        std::vector<float> tmp(width * height);
        int rad = kern.radius();

        // Horizontal pass.
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float sum = 0.0f;
                for (int k = -rad; k <= rad; ++k) {
                    int sx = std::max(0, std::min(width - 1, x + k));
                    sum += src[y * width + sx] * kern.weight(k);
                }
                tmp[y * width + x] = sum;
            }
        }

        // Vertical pass.
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float sum = 0.0f;
                for (int k = -rad; k <= rad; ++k) {
                    int sy = std::max(0, std::min(height - 1, y + k));
                    sum += tmp[sy * width + x] * kern.weight(k);
                }
                dst[y * width + x] = sum;
            }
        }
    }
};

void separable_gaussian_abstract_unspecialized(const float* src, float* dst,
                                               int width, int height,
                                               const float* coeffs, int ksize) {
    GaussianKernel gk(coeffs, ksize / 2);
    Convolver conv{gk};
    conv.convolve(src, dst, width, height);
}

SeparableGaussianAbstractSpecialized create_separable_gaussian_abstract_specialized(
        int width, int height, const float* coeffs, int ksize) {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    int radius = ksize / 2;
    const int ksz = 2 * radius + 1;
    const float c0 = ksz > 0 ? coeffs[0] : 0.0f;
    const float c1 = ksz > 1 ? coeffs[1] : 0.0f;
    const float c2 = ksz > 2 ? coeffs[2] : 0.0f;
    const float c3 = ksz > 3 ? coeffs[3] : 0.0f;
    const float c4 = ksz > 4 ? coeffs[4] : 0.0f;
    auto lam = [width, height, radius, c0, c1, c2, c3, c4](const float* src, float* dst) {
        // Reconstruct polymorphic objects inside the lambda so their
        // this-pointers and coefficient storage are local to execution.
        const float coeff_storage[2 * GaussianKernel::kMaxR + 1] = {c0, c1, c2, c3, c4};
        GaussianKernel gk(coeff_storage, radius);
        Convolver conv{gk};
        conv.convolve(src, dst, width, height);
    };
    return RS->specializeLambda<void>(lam);
}

void validate_separable_gaussian_abstract_specialized() {
    constexpr int W = 64, H = 64, K = 5;
    float kcoeffs[K] = {0.0625f, 0.25f, 0.375f, 0.25f, 0.0625f};

    std::vector<float> src(W * H, 1.0f);
    std::vector<float> ref_dst(W * H, 0.0f);
    std::vector<float> spec_dst(W * H, 0.0f);

    GaussianKernel ref_gk(kcoeffs, K / 2);
    Convolver ref_conv{ref_gk};
    ref_conv.convolve(src.data(), ref_dst.data(), W, H);

    auto spec = create_separable_gaussian_abstract_specialized(W, H, kcoeffs, K);
    spec(src.data(), spec_dst.data());

    constexpr float kTol = 1e-4f;
    for (int i = 0; i < W * H; ++i) {
        float diff = ref_dst[i] - spec_dst[i];
        if (diff < -kTol || diff > kTol)
            throw clangRuntimeSpecializer::ClangRuntimeSpecializerError(
                "validate_separable_gaussian_abstract_specialized: element-wise mismatch");
    }
    clangRuntimeSpecializer::ClangRuntimeSpecializer::log(
        clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Info,
        "validate_separable_gaussian_abstract_specialized passed");
}
