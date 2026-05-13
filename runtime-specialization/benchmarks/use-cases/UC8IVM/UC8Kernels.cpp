#include "UC8Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <string>

// CRITICAL: apply_row_delta must live in the same TU as the lambda factory
// so the JIT module clone contains both for full inlining.
void apply_row_delta(const uint8_t* row, double* agg_buckets,
                     int n_buckets, int group_col_offset,
                     int value_col_offset, int row_stride) {
    int32_t group_key;
    __builtin_memcpy(&group_key, row + group_col_offset, sizeof(int32_t));
    int bucket = ((group_key % n_buckets) + n_buckets) % n_buckets;
    double value;
    __builtin_memcpy(&value, row + value_col_offset, sizeof(double));
    agg_buckets[bucket] += value;
}

IVMSpecialized create_ivm_specialized(int n_buckets, int group_col_offset,
                                       int value_col_offset, int row_stride) {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    auto lam = [n_buckets, group_col_offset, value_col_offset, row_stride]
               (const uint8_t* row, double* agg_buckets) {
        apply_row_delta(row, agg_buckets, n_buckets, group_col_offset, value_col_offset, row_stride);
    };
    return RS->specializeLambda<void>(lam);
}

void validate_ivm_specialized(int n_buckets, int group_col_offset,
                               int value_col_offset, int row_stride) {
    // Create small test: 100 rows × row_stride bytes.
    constexpr int N_TEST = 100;
    std::vector<uint8_t> test_data(N_TEST * row_stride, 0);

    // Fill with known values: group_key cycles 0..n_buckets-1, value = 1.0 for all rows.
    for (int i = 0; i < N_TEST; ++i) {
        int32_t gk = static_cast<int32_t>(i % n_buckets);
        double val = 1.0;
        __builtin_memcpy(test_data.data() + i * row_stride + group_col_offset, &gk, sizeof(int32_t));
        __builtin_memcpy(test_data.data() + i * row_stride + value_col_offset, &val, sizeof(double));
    }

    // Run reference function.
    std::vector<double> ref_buckets(n_buckets, 0.0);
    for (int i = 0; i < N_TEST; ++i)
        apply_row_delta(test_data.data() + i * row_stride, ref_buckets.data(),
                        n_buckets, group_col_offset, value_col_offset, row_stride);

    // Run specialized lambda.
    auto spec = create_ivm_specialized(n_buckets, group_col_offset, value_col_offset, row_stride);
    std::vector<double> spec_buckets(n_buckets, 0.0);
    for (int i = 0; i < N_TEST; ++i)
        spec(test_data.data() + i * row_stride, spec_buckets.data());

    // Compare results exactly (same values, same order — no floating point ambiguity).
    for (int b = 0; b < n_buckets; ++b) {
        if (ref_buckets[b] != spec_buckets[b]) {
            throw std::runtime_error(
                "validate_ivm_specialized: bucket mismatch at index " + std::to_string(b) +
                ": ref=" + std::to_string(ref_buckets[b]) +
                " spec=" + std::to_string(spec_buckets[b]));
        }
    }
}
