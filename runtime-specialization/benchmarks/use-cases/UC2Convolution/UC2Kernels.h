#pragma once
#include "ClangRuntimeSpecializer.h"
#include <cstdint>

// ---------------------------------------------------------------------------
// Separable Gaussian (existing variant — low tier)
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// Shared SpecializedLambda type aliases
// ---------------------------------------------------------------------------

using BoxFilterLowSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const float*, float*>;
using BoxFilterTradeoffSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const float*, float*>;
using BoxFilterAbstractSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const float*, float*>;

using EdgeDetectionLowSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const float*, float*>;
using EdgeDetectionTradeoffSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const float*, float*>;
using EdgeDetectionAbstractSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const float*, float*>;

using SeparableGaussianTradeoffSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const float*, float*>;
using SeparableGaussianAbstractSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const float*, float*>;

// ---------------------------------------------------------------------------
// Box filter (low tier)
// ---------------------------------------------------------------------------

void box_filter(const float* src, float* dst, int width, int height, int radius);
BoxFilterLowSpecialized create_box_filter_low_specialized(int width, int height, int radius);
void validate_box_filter_low_specialized();

// ---------------------------------------------------------------------------
// Edge detection / Sobel (low tier)
// ---------------------------------------------------------------------------

void sobel_edge_detect(const float* src, float* dst, int width, int height);
EdgeDetectionLowSpecialized create_edge_detection_low_specialized(int width, int height);
void validate_edge_detection_low_specialized();

// ---------------------------------------------------------------------------
// Separable Gaussian (tradeoff tier)
// ---------------------------------------------------------------------------

void separable_gaussian_tradeoff_unspecialized(const float* src, float* dst,
                                               int width, int height,
                                               const float* coeffs, int ksize);
SeparableGaussianTradeoffSpecialized create_separable_gaussian_tradeoff_specialized(
        int width, int height, const float* coeffs, int ksize);
void validate_separable_gaussian_tradeoff_specialized();

// ---------------------------------------------------------------------------
// Box filter (tradeoff tier)
// ---------------------------------------------------------------------------

void box_filter_tradeoff_unspecialized(const float* src, float* dst,
                                       int width, int height, int radius);
BoxFilterTradeoffSpecialized create_box_filter_tradeoff_specialized(
        int width, int height, int radius);
void validate_box_filter_tradeoff_specialized();

// ---------------------------------------------------------------------------
// Edge detection (tradeoff tier)
// ---------------------------------------------------------------------------

void edge_detection_tradeoff_unspecialized(const float* src, float* dst,
                                           int width, int height);
EdgeDetectionTradeoffSpecialized create_edge_detection_tradeoff_specialized(
        int width, int height);
void validate_edge_detection_tradeoff_specialized();

// ---------------------------------------------------------------------------
// Separable Gaussian (abstract tier)
// ---------------------------------------------------------------------------

void separable_gaussian_abstract_unspecialized(const float* src, float* dst,
                                               int width, int height,
                                               const float* coeffs, int ksize);
SeparableGaussianAbstractSpecialized create_separable_gaussian_abstract_specialized(
        int width, int height, const float* coeffs, int ksize);
void validate_separable_gaussian_abstract_specialized();

// ---------------------------------------------------------------------------
// Box filter (abstract tier)
// ---------------------------------------------------------------------------

void box_filter_abstract_unspecialized(const float* src, float* dst,
                                       int width, int height, int radius);
BoxFilterAbstractSpecialized create_box_filter_abstract_specialized(
        int width, int height, int radius);
void validate_box_filter_abstract_specialized();

// ---------------------------------------------------------------------------
// Edge detection (abstract tier)
// ---------------------------------------------------------------------------

void edge_detection_abstract_unspecialized(const float* src, float* dst,
                                           int width, int height);
EdgeDetectionAbstractSpecialized create_edge_detection_abstract_specialized(
        int width, int height);
void validate_edge_detection_abstract_specialized();
