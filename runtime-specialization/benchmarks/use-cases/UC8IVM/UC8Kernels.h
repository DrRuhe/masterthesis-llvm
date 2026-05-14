#pragma once
#include "ClangRuntimeSpecializer.h"
#include <cstdint>

// ---------------------------------------------------------------------------
// Type aliases — one per variant × level
// ---------------------------------------------------------------------------

// apply_row_delta (single-row sum aggregation)
using ApplyRowDeltaLowSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, double*>;
using ApplyRowDeltaTradeoffSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, double*>;
using ApplyRowDeltaAbstractSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, double*>;

// multi_agg_delta (sum + count both updated per row)
using MultiAggDeltaLowSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, double*, double*>;
using MultiAggDeltaTradeoffSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, double*, double*>;
using MultiAggDeltaAbstractSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, double*, double*>;

// batch_delta (outer loop over n_rows, sum only)
using BatchDeltaLowSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, double*>;
using BatchDeltaTradeoffSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, double*>;
using BatchDeltaAbstractSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, double*>;

// Backward-compatibility alias (original low-tier variant).
using IVMSpecialized = ApplyRowDeltaLowSpecialized;

// ---------------------------------------------------------------------------
// Kernel declaration (low / original)
// ---------------------------------------------------------------------------

void apply_row_delta(const uint8_t* row, double* agg_buckets,
                     int n_buckets, int group_col_offset,
                     int value_col_offset, int row_stride);

// ---------------------------------------------------------------------------
// Factory + validation — apply_row_delta
// ---------------------------------------------------------------------------

IVMSpecialized create_ivm_specialized(int n_buckets, int group_col_offset,
                                       int value_col_offset, int row_stride);
void validate_ivm_specialized(int n_buckets, int group_col_offset,
                               int value_col_offset, int row_stride);

ApplyRowDeltaTradeoffSpecialized create_apply_row_delta_tradeoff_specialized(
    int n_buckets, int group_col_offset, int value_col_offset, int row_stride);
void validate_apply_row_delta_tradeoff_specialized(
    int n_buckets, int group_col_offset, int value_col_offset, int row_stride);

ApplyRowDeltaAbstractSpecialized create_apply_row_delta_abstract_specialized(
    int n_buckets, int group_col_offset, int value_col_offset, int row_stride);
void validate_apply_row_delta_abstract_specialized(
    int n_buckets, int group_col_offset, int value_col_offset, int row_stride);

// ---------------------------------------------------------------------------
// Factory + validation — multi_agg_delta
// ---------------------------------------------------------------------------

MultiAggDeltaLowSpecialized create_multi_agg_delta_low_specialized(
    int n_buckets, int group_col_offset, int value_col_offset, int row_stride);
void validate_multi_agg_delta_low_specialized(
    int n_buckets, int group_col_offset, int value_col_offset, int row_stride);

MultiAggDeltaTradeoffSpecialized create_multi_agg_delta_tradeoff_specialized(
    int n_buckets, int group_col_offset, int value_col_offset, int row_stride);
void validate_multi_agg_delta_tradeoff_specialized(
    int n_buckets, int group_col_offset, int value_col_offset, int row_stride);

MultiAggDeltaAbstractSpecialized create_multi_agg_delta_abstract_specialized(
    int n_buckets, int group_col_offset, int value_col_offset, int row_stride);
void validate_multi_agg_delta_abstract_specialized(
    int n_buckets, int group_col_offset, int value_col_offset, int row_stride);

// ---------------------------------------------------------------------------
// Factory + validation — batch_delta
// ---------------------------------------------------------------------------

BatchDeltaLowSpecialized create_batch_delta_low_specialized(
    int64_t n_rows, int n_buckets, int group_col_offset,
    int value_col_offset, int row_stride);
void validate_batch_delta_low_specialized(
    int64_t n_rows, int n_buckets, int group_col_offset,
    int value_col_offset, int row_stride);

BatchDeltaTradeoffSpecialized create_batch_delta_tradeoff_specialized(
    int64_t n_rows, int n_buckets, int group_col_offset,
    int value_col_offset, int row_stride);
void validate_batch_delta_tradeoff_specialized(
    int64_t n_rows, int n_buckets, int group_col_offset,
    int value_col_offset, int row_stride);

BatchDeltaAbstractSpecialized create_batch_delta_abstract_specialized(
    int64_t n_rows, int n_buckets, int group_col_offset,
    int value_col_offset, int row_stride);
void validate_batch_delta_abstract_specialized(
    int64_t n_rows, int n_buckets, int group_col_offset,
    int value_col_offset, int row_stride);
