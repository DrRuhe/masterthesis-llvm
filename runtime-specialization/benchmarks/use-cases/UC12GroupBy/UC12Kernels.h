#pragma once
#include "ClangRuntimeSpecializer.h"
#include <cstdint>

// ---------------------------------------------------------------------------
// Specialization type aliases
// ---------------------------------------------------------------------------

// Grouped-sum: lambda(rows, n_rows, double* out_buckets)
using GroupedSumLowSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, int64_t, double*>;

// For backward compatibility the original alias is retained.
using GroupBySpecialized = GroupedSumLowSpecialized;

using GroupedSumTradeoffSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, int64_t, double*>;

using GroupedSumAbstractSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, int64_t, double*>;

// Grouped-count: lambda(rows, n_rows, int64_t* count_buckets)
using GroupedCountLowSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, int64_t, int64_t*>;

using GroupedCountTradeoffSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, int64_t, int64_t*>;

using GroupedCountAbstractSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, int64_t, int64_t*>;

// Grouped-minmax: lambda(rows, n_rows, double* min_out, double* max_out)
using GroupedMinMaxLowSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, int64_t, double*, double*>;

using GroupedMinMaxTradeoffSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, int64_t, double*, double*>;

using GroupedMinMaxAbstractSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, int64_t, double*, double*>;

// ---------------------------------------------------------------------------
// grouped_sum (low tier) — declared for use in UC12Benchmark.cpp
// ---------------------------------------------------------------------------
void grouped_sum(const uint8_t* rows, int64_t n_rows,
                 int row_stride, int key_offset, int value_offset,
                 double* out_buckets, int n_buckets);

GroupBySpecialized create_groupby_specialized(int row_stride, int key_offset,
                                               int value_offset, int n_buckets);
void validate_groupby_specialized(int row_stride, int key_offset,
                                   int value_offset, int n_buckets);

// ---------------------------------------------------------------------------
// grouped_count (low tier)
// ---------------------------------------------------------------------------
void grouped_count(const uint8_t* rows, int64_t n_rows,
                   int row_stride, int key_offset,
                   int n_buckets, int64_t* count_buckets);

GroupedCountLowSpecialized create_grouped_count_low_specialized(
        int row_stride, int key_offset, int n_buckets);
void validate_grouped_count_low_specialized(int row_stride, int key_offset, int n_buckets);

// ---------------------------------------------------------------------------
// grouped_minmax (low tier)
// ---------------------------------------------------------------------------
void grouped_minmax(const uint8_t* rows, int64_t n_rows,
                    int row_stride, int key_offset, int value_offset,
                    int n_buckets, double* min_buckets, double* max_buckets);

GroupedMinMaxLowSpecialized create_grouped_minmax_low_specialized(
        int row_stride, int key_offset, int value_offset, int n_buckets);
void validate_grouped_minmax_low_specialized(
        int row_stride, int key_offset, int value_offset, int n_buckets);

// ---------------------------------------------------------------------------
// grouped_sum (tradeoff tier)
// ---------------------------------------------------------------------------
void grouped_sum_tradeoff_unspecialized(const uint8_t* rows, int64_t n_rows,
                                         int row_stride, int key_offset, int value_offset,
                                         int n_buckets, double* out_buckets);
GroupedSumTradeoffSpecialized create_grouped_sum_tradeoff_specialized(
        int row_stride, int key_offset, int value_offset, int n_buckets);
void validate_grouped_sum_tradeoff_specialized(
        int row_stride, int key_offset, int value_offset, int n_buckets);

// ---------------------------------------------------------------------------
// grouped_count (tradeoff tier)
// ---------------------------------------------------------------------------
void grouped_count_tradeoff_unspecialized(const uint8_t* rows, int64_t n_rows,
                                           int row_stride, int key_offset,
                                           int n_buckets, int64_t* out);
GroupedCountTradeoffSpecialized create_grouped_count_tradeoff_specialized(
        int row_stride, int key_offset, int n_buckets);
void validate_grouped_count_tradeoff_specialized(
        int row_stride, int key_offset, int n_buckets);

// ---------------------------------------------------------------------------
// grouped_minmax (tradeoff tier)
// ---------------------------------------------------------------------------
void grouped_minmax_tradeoff_unspecialized(const uint8_t* rows, int64_t n_rows,
                                            int row_stride, int key_offset, int value_offset,
                                            int n_buckets, double* min_buckets, double* max_buckets);
GroupedMinMaxTradeoffSpecialized create_grouped_minmax_tradeoff_specialized(
        int row_stride, int key_offset, int value_offset, int n_buckets);
void validate_grouped_minmax_tradeoff_specialized(
        int row_stride, int key_offset, int value_offset, int n_buckets);

// ---------------------------------------------------------------------------
// grouped_sum (abstract tier)
// ---------------------------------------------------------------------------
void grouped_sum_abstract_unspecialized(const uint8_t* rows, int64_t n_rows,
                                         int row_stride, int key_offset, int value_offset,
                                         int n_buckets, double* out_buckets);
GroupedSumAbstractSpecialized create_grouped_sum_abstract_specialized(
        int row_stride, int key_offset, int value_offset, int n_buckets);
void validate_grouped_sum_abstract_specialized(
        int row_stride, int key_offset, int value_offset, int n_buckets);

// ---------------------------------------------------------------------------
// grouped_count (abstract tier)
// ---------------------------------------------------------------------------
void grouped_count_abstract_unspecialized(const uint8_t* rows, int64_t n_rows,
                                           int row_stride, int key_offset,
                                           int n_buckets, int64_t* out);
GroupedCountAbstractSpecialized create_grouped_count_abstract_specialized(
        int row_stride, int key_offset, int n_buckets);
void validate_grouped_count_abstract_specialized(
        int row_stride, int key_offset, int n_buckets);

// ---------------------------------------------------------------------------
// grouped_minmax (abstract tier)
// ---------------------------------------------------------------------------
void grouped_minmax_abstract_unspecialized(const uint8_t* rows, int64_t n_rows,
                                            int row_stride, int key_offset, int value_offset,
                                            int n_buckets, double* min_buckets, double* max_buckets);
GroupedMinMaxAbstractSpecialized create_grouped_minmax_abstract_specialized(
        int row_stride, int key_offset, int value_offset, int n_buckets);
void validate_grouped_minmax_abstract_specialized(
        int row_stride, int key_offset, int value_offset, int n_buckets);
