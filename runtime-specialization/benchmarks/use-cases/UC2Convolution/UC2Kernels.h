#pragma once
#include "ClangRuntimeSpecializer.h"
#include <cstdint>

// 5-tap 1D Gaussian coefficients (specialization constant via pointer)
extern float g_kernel_coeffs[5];

// convolve2d: separable two-pass 2D convolution.
// kernel_coeffs, ksize, width, height are specialization constants.
void convolve2d(const float* src, float* dst, int width, int height,
                const float* kernel_coeffs, int ksize);

using ConvSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const float*, float*>;

ConvSpecialized create_conv_specialized(int width, int height);
void validate_conv_specialized();
