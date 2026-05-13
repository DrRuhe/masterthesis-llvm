#pragma once
#include "ClangRuntimeSpecializer.h"
#include <cstdint>

// count_matching_rows: row_stride, col_offset, threshold are specialization constants
int64_t count_matching_rows(const uint8_t* rows, int64_t n_rows,
                             int row_stride, int col_offset, double threshold);

using SQLSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const uint8_t*, int64_t>;

SQLSpecialized create_sql_specialized(int row_stride, int col_offset, double threshold);
void validate_sql_specialized(int row_stride, int col_offset, double threshold);
