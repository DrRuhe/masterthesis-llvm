#pragma once
#include "ClangRuntimeSpecializer.h"
#include <cstdint>

// apply_row_delta: n_buckets, group_col_offset, value_col_offset, row_stride
// are specialization constants.
void apply_row_delta(const uint8_t* row, double* agg_buckets,
                     int n_buckets, int group_col_offset,
                     int value_col_offset, int row_stride);

using IVMSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, double*>;

IVMSpecialized create_ivm_specialized(int n_buckets, int group_col_offset,
                                       int value_col_offset, int row_stride);
void validate_ivm_specialized(int n_buckets, int group_col_offset,
                               int value_col_offset, int row_stride);
