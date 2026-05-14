#pragma once
#include "ClangRuntimeSpecializer.h"
#include <cstdint>

// ---------------------------------------------------------------------------
// Type aliases — one per variant × level
// ---------------------------------------------------------------------------

// count_matching_rows (single predicate)
using CountMatchingRowsLowSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const uint8_t*, int64_t>;
using CountMatchingRowsTradeoffSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const uint8_t*, int64_t>;
using CountMatchingRowsAbstractSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const uint8_t*, int64_t>;

// multi_predicate (two-column AND predicate)
using MultiPredicateLowSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const uint8_t*, int64_t>;
using MultiPredicateTradeoffSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const uint8_t*, int64_t>;
using MultiPredicateAbstractSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const uint8_t*, int64_t>;

// column_scan (predicate + index projection)
using ColumnScanLowSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const uint8_t*, int64_t, int32_t*>;
using ColumnScanTradeoffSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const uint8_t*, int64_t, int32_t*>;
using ColumnScanAbstractSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const uint8_t*, int64_t, int32_t*>;

// Backward-compatibility alias (UC1 low / original variant).
using SQLSpecialized = CountMatchingRowsLowSpecialized;

// ---------------------------------------------------------------------------
// Kernel declaration (low / original)
// ---------------------------------------------------------------------------

int64_t count_matching_rows(const uint8_t* rows, int64_t n_rows,
                             int row_stride, int col_offset, double threshold);

// ---------------------------------------------------------------------------
// Factory + validation — count_matching_rows
// ---------------------------------------------------------------------------

SQLSpecialized create_sql_specialized(int row_stride, int col_offset, double threshold);
void validate_sql_specialized(int row_stride, int col_offset, double threshold);

CountMatchingRowsTradeoffSpecialized create_count_matching_rows_tradeoff_specialized(
    int col_offset, int row_stride, double threshold);
void validate_count_matching_rows_tradeoff_specialized(
    int col_offset, int row_stride, double threshold);

CountMatchingRowsAbstractSpecialized create_count_matching_rows_abstract_specialized(
    int col_offset, int row_stride, double threshold);
void validate_count_matching_rows_abstract_specialized(
    int col_offset, int row_stride, double threshold);

// ---------------------------------------------------------------------------
// Factory + validation — multi_predicate
// ---------------------------------------------------------------------------

MultiPredicateLowSpecialized create_multi_predicate_low_specialized(
    int row_stride, int col_offset_a, int col_offset_b,
    double threshold_a, double threshold_b);
void validate_multi_predicate_low_specialized(
    int row_stride, int col_offset_a, int col_offset_b,
    double threshold_a, double threshold_b);

MultiPredicateTradeoffSpecialized create_multi_predicate_tradeoff_specialized(
    int row_stride, int col_offset_a, int col_offset_b,
    double threshold_a, double threshold_b);
void validate_multi_predicate_tradeoff_specialized(
    int row_stride, int col_offset_a, int col_offset_b,
    double threshold_a, double threshold_b);

MultiPredicateAbstractSpecialized create_multi_predicate_abstract_specialized(
    int row_stride, int col_offset_a, int col_offset_b,
    double threshold_a, double threshold_b);
void validate_multi_predicate_abstract_specialized(
    int row_stride, int col_offset_a, int col_offset_b,
    double threshold_a, double threshold_b);

// ---------------------------------------------------------------------------
// Factory + validation — column_scan
// ---------------------------------------------------------------------------

ColumnScanLowSpecialized create_column_scan_low_specialized(
    int row_stride, int col_offset, double threshold);
void validate_column_scan_low_specialized(
    int row_stride, int col_offset, double threshold);

ColumnScanTradeoffSpecialized create_column_scan_tradeoff_specialized(
    int row_stride, int col_offset, double threshold);
void validate_column_scan_tradeoff_specialized(
    int row_stride, int col_offset, double threshold);

ColumnScanAbstractSpecialized create_column_scan_abstract_specialized(
    int row_stride, int col_offset, double threshold);
void validate_column_scan_abstract_specialized(
    int row_stride, int col_offset, double threshold);
