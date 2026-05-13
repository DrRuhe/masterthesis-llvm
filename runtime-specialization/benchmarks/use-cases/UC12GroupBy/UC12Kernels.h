#pragma once
#include "ClangRuntimeSpecializer.h"
#include <cstdint>

// grouped_sum: row_stride, key_offset, value_offset, n_buckets are specialization constants.
void grouped_sum(const uint8_t* rows, int64_t n_rows,
                 int row_stride, int key_offset, int value_offset,
                 double* out_buckets, int n_buckets);

using GroupBySpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, int64_t, double*>;

GroupBySpecialized create_groupby_specialized(int row_stride, int key_offset,
                                               int value_offset, int n_buckets);
void validate_groupby_specialized(int row_stride, int key_offset,
                                   int value_offset, int n_buckets);
