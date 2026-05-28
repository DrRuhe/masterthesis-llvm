#pragma once
#include "ClangRuntimeSpecializer.h"
#include <cstdint>

// ---------------------------------------------------------------------------
// Type aliases — one per variant × level
// ---------------------------------------------------------------------------

// apply_row_delta (single-row sum aggregation — per-row variant)
using ApplyRowDeltaLowSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, double*>;
using ApplyRowDeltaTradeoffSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, double*>;
using ApplyRowDeltaAbstractSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, double*>;

// apply_row_delta (batch: n_rows captured as constant, loop inside lambda)
using ApplyRowDeltaBatchLowSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, double*>;
using ApplyRowDeltaBatchTradeoffSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, double*>;
using ApplyRowDeltaBatchAbstractSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, double*>;

// multi_agg_delta (sum + count both updated per row — per-row variant)
using MultiAggDeltaLowSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, double*, double*>;
using MultiAggDeltaTradeoffSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, double*, double*>;
using MultiAggDeltaAbstractSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, double*, double*>;

// multi_agg_delta (batch: n_rows captured as constant, loop inside lambda)
using MultiAggDeltaBatchLowSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, double*, double*>;
using MultiAggDeltaBatchTradeoffSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, double*, double*>;
using MultiAggDeltaBatchAbstractSpecialized =
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
// Kernel declarations
// ---------------------------------------------------------------------------

void apply_row_delta(const uint8_t* row, double* agg_buckets,
                     int n_buckets, int group_col_offset,
                     int value_col_offset, int row_stride);

// Batch kernel — processes all n_rows in one call; used for unspecialized benchmarks.
void apply_row_delta_batch(const uint8_t* rows, int64_t n_rows, double* agg_buckets,
                            int n_buckets, int group_col_offset,
                            int value_col_offset, int row_stride);

void multi_agg_delta_batch(const uint8_t* rows, int64_t n_rows,
                            double* sum_buckets, double* count_buckets,
                            int n_buckets, int group_col_offset,
                            int value_col_offset, int row_stride);

// ---------------------------------------------------------------------------
// Unspecialized baseline functions — tradeoff and abstract levels
// ---------------------------------------------------------------------------

void apply_row_delta_batch_tradeoff_unspecialized(const uint8_t* rows, int64_t n_rows,
                                                   double* buckets, int n_buckets,
                                                   int group_col_offset, int value_col_offset,
                                                   int row_stride);

void apply_row_delta_batch_abstract_unspecialized(const uint8_t* rows, int64_t n_rows,
                                                   double* buckets, int n_buckets,
                                                   int group_col_offset, int value_col_offset,
                                                   int row_stride);

void multi_agg_delta_batch_tradeoff_unspecialized(const uint8_t* rows, int64_t n_rows,
                                                   double* sum_buckets, double* count_buckets,
                                                   int n_buckets, int group_col_offset,
                                                   int value_col_offset, int row_stride);

void multi_agg_delta_batch_abstract_unspecialized(const uint8_t* rows, int64_t n_rows,
                                                   double* sum_buckets, double* count_buckets,
                                                   int n_buckets, int group_col_offset,
                                                   int value_col_offset, int row_stride);

void batch_delta_tradeoff_unspecialized(const uint8_t* rows, int64_t n_rows,
                                        double* buckets, int n_buckets,
                                        int group_col_offset, int value_col_offset,
                                        int row_stride);

void batch_delta_abstract_unspecialized(const uint8_t* rows, int64_t n_rows,
                                        double* buckets, int n_buckets,
                                        int group_col_offset, int value_col_offset,
                                        int row_stride);

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
// Factory + validation — apply_row_delta batch (n_rows as constant)
// ---------------------------------------------------------------------------

ApplyRowDeltaBatchLowSpecialized create_apply_row_delta_batch_low_specialized(
    int64_t n_rows, int n_buckets, int group_col_offset, int value_col_offset, int row_stride);
void validate_apply_row_delta_batch_low_specialized(
    int64_t n_rows, int n_buckets, int group_col_offset, int value_col_offset, int row_stride);

ApplyRowDeltaBatchTradeoffSpecialized create_apply_row_delta_batch_tradeoff_specialized(
    int64_t n_rows, int n_buckets, int group_col_offset, int value_col_offset, int row_stride);
void validate_apply_row_delta_batch_tradeoff_specialized(
    int64_t n_rows, int n_buckets, int group_col_offset, int value_col_offset, int row_stride);

ApplyRowDeltaBatchAbstractSpecialized create_apply_row_delta_batch_abstract_specialized(
    int64_t n_rows, int n_buckets, int group_col_offset, int value_col_offset, int row_stride);
void validate_apply_row_delta_batch_abstract_specialized(
    int64_t n_rows, int n_buckets, int group_col_offset, int value_col_offset, int row_stride);

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
// Factory + validation — multi_agg_delta batch (n_rows as constant)
// ---------------------------------------------------------------------------

MultiAggDeltaBatchLowSpecialized create_multi_agg_delta_batch_low_specialized(
    int64_t n_rows, int n_buckets, int group_col_offset, int value_col_offset, int row_stride);
void validate_multi_agg_delta_batch_low_specialized(
    int64_t n_rows, int n_buckets, int group_col_offset, int value_col_offset, int row_stride);

MultiAggDeltaBatchTradeoffSpecialized create_multi_agg_delta_batch_tradeoff_specialized(
    int64_t n_rows, int n_buckets, int group_col_offset, int value_col_offset, int row_stride);
void validate_multi_agg_delta_batch_tradeoff_specialized(
    int64_t n_rows, int n_buckets, int group_col_offset, int value_col_offset, int row_stride);

MultiAggDeltaBatchAbstractSpecialized create_multi_agg_delta_batch_abstract_specialized(
    int64_t n_rows, int n_buckets, int group_col_offset, int value_col_offset, int row_stride);
void validate_multi_agg_delta_batch_abstract_specialized(
    int64_t n_rows, int n_buckets, int group_col_offset, int value_col_offset, int row_stride);

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
