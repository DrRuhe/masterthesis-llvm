#include "UC8Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <stdexcept>
#include <vector>
#include <string>

// IVMDualUpdater captures layout params as struct fields.
// update() increments both sum and count buckets per row.
struct IVMDualUpdater {
    int n_buckets;
    int group_col_offset;
    int value_col_offset;
    int row_stride;

    void update(const uint8_t* row, double* sum_buckets, double* count_buckets) const {
        int32_t group_key;
        __builtin_memcpy(&group_key, row + group_col_offset, sizeof(int32_t));
        int bucket = ((group_key % n_buckets) + n_buckets) % n_buckets;
        double value;
        __builtin_memcpy(&value, row + value_col_offset, sizeof(double));
        sum_buckets[bucket]   += value;
        count_buckets[bucket] += 1.0;
    }
};

MultiAggDeltaTradeoffSpecialized create_multi_agg_delta_tradeoff_specialized(
        int n_buckets, int group_col_offset, int value_col_offset, int row_stride) {
    IVMDualUpdater updater{n_buckets, group_col_offset, value_col_offset, row_stride};
    auto lam = [updater](const uint8_t* row, double* sum_buckets, double* count_buckets) {
        updater.update(row, sum_buckets, count_buckets);
    };
    return clangRuntimeSpecializer::specializeLambda<void>(lam);
}

void validate_multi_agg_delta_tradeoff_specialized(int n_buckets, int group_col_offset,
                                                    int value_col_offset, int row_stride) {
    constexpr int N_TEST = 100;
    std::vector<uint8_t> test_data(N_TEST * row_stride, 0);

    for (int i = 0; i < N_TEST; ++i) {
        int32_t gk  = static_cast<int32_t>(i % n_buckets);
        double  val = 1.0;
        __builtin_memcpy(test_data.data() + i * row_stride + group_col_offset, &gk, sizeof(int32_t));
        __builtin_memcpy(test_data.data() + i * row_stride + value_col_offset, &val, sizeof(double));
    }

    IVMDualUpdater ref_updater{n_buckets, group_col_offset, value_col_offset, row_stride};
    std::vector<double> ref_sum(n_buckets, 0.0);
    std::vector<double> ref_cnt(n_buckets, 0.0);
    for (int i = 0; i < N_TEST; ++i)
        ref_updater.update(test_data.data() + i * row_stride, ref_sum.data(), ref_cnt.data());

    auto spec = create_multi_agg_delta_tradeoff_specialized(
        n_buckets, group_col_offset, value_col_offset, row_stride);
    std::vector<double> spec_sum(n_buckets, 0.0);
    std::vector<double> spec_cnt(n_buckets, 0.0);
    for (int i = 0; i < N_TEST; ++i)
        spec(test_data.data() + i * row_stride, spec_sum.data(), spec_cnt.data());

    for (int b = 0; b < n_buckets; ++b) {
        if (ref_sum[b] != spec_sum[b] || ref_cnt[b] != spec_cnt[b]) {
            throw std::runtime_error(
                "validate_multi_agg_delta_tradeoff_specialized: bucket mismatch at index " +
                std::to_string(b));
        }
    }
}
