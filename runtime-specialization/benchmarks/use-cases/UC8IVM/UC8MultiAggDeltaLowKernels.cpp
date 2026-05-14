#include "UC8Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <string>

// multi_agg_delta: extracts group key and value from each row, updating
// both sum_buckets and count_buckets.  All layout params are specialization
// constants captured in the lambda.
static void multi_agg_delta(const uint8_t* row, int n_buckets,
                             int group_col_offset, int value_col_offset,
                             int row_stride,
                             double* sum_buckets, double* count_buckets) {
    int32_t group_key;
    __builtin_memcpy(&group_key, row + group_col_offset, sizeof(int32_t));
    int bucket = ((group_key % n_buckets) + n_buckets) % n_buckets;
    double value;
    __builtin_memcpy(&value, row + value_col_offset, sizeof(double));
    sum_buckets[bucket]   += value;
    count_buckets[bucket] += 1.0;
}

MultiAggDeltaLowSpecialized create_multi_agg_delta_low_specialized(
        int n_buckets, int group_col_offset, int value_col_offset, int row_stride) {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    auto lam = [n_buckets, group_col_offset, value_col_offset, row_stride]
               (const uint8_t* row, double* sum_buckets, double* count_buckets) {
        multi_agg_delta(row, n_buckets, group_col_offset, value_col_offset,
                        row_stride, sum_buckets, count_buckets);
    };
    return RS->specializeLambda<void>(lam);
}

void validate_multi_agg_delta_low_specialized(int n_buckets, int group_col_offset,
                                               int value_col_offset, int row_stride) {
    constexpr int N_TEST = 100;
    std::vector<uint8_t> test_data(N_TEST * row_stride, 0);

    for (int i = 0; i < N_TEST; ++i) {
        int32_t gk  = static_cast<int32_t>(i % n_buckets);
        double  val = 1.0;
        __builtin_memcpy(test_data.data() + i * row_stride + group_col_offset, &gk, sizeof(int32_t));
        __builtin_memcpy(test_data.data() + i * row_stride + value_col_offset, &val, sizeof(double));
    }

    // Reference run.
    std::vector<double> ref_sum(n_buckets, 0.0);
    std::vector<double> ref_cnt(n_buckets, 0.0);
    for (int i = 0; i < N_TEST; ++i)
        multi_agg_delta(test_data.data() + i * row_stride,
                        n_buckets, group_col_offset, value_col_offset, row_stride,
                        ref_sum.data(), ref_cnt.data());

    // Specialized run.
    auto spec = create_multi_agg_delta_low_specialized(
        n_buckets, group_col_offset, value_col_offset, row_stride);
    std::vector<double> spec_sum(n_buckets, 0.0);
    std::vector<double> spec_cnt(n_buckets, 0.0);
    for (int i = 0; i < N_TEST; ++i)
        spec(test_data.data() + i * row_stride, spec_sum.data(), spec_cnt.data());

    for (int b = 0; b < n_buckets; ++b) {
        if (ref_sum[b] != spec_sum[b] || ref_cnt[b] != spec_cnt[b]) {
            throw std::runtime_error(
                "validate_multi_agg_delta_low_specialized: bucket mismatch at index " +
                std::to_string(b));
        }
    }
}
