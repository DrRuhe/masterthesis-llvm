#include "UC8Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <stdexcept>
#include <vector>
#include <string>

// BatchIVMUpdater captures n_rows + all layout params as struct fields.
// process() iterates over n_rows and accumulates into sum_buckets.
struct BatchIVMUpdater {
    int64_t n_rows;
    int     n_buckets;
    int     group_col_offset;
    int     value_col_offset;
    int     row_stride;

    void process(const uint8_t* rows, double* sum_buckets) const {
        for (int64_t i = 0; i < n_rows; ++i) {
            const uint8_t* row = rows + static_cast<size_t>(i) * row_stride;
            int32_t group_key;
            __builtin_memcpy(&group_key, row + group_col_offset, sizeof(int32_t));
            int bucket = ((group_key % n_buckets) + n_buckets) % n_buckets;
            double value;
            __builtin_memcpy(&value, row + value_col_offset, sizeof(double));
            sum_buckets[bucket] += value;
        }
    }
};

BatchDeltaTradeoffSpecialized create_batch_delta_tradeoff_specialized(
        int64_t n_rows, int n_buckets, int group_col_offset,
        int value_col_offset, int row_stride) {
    BatchIVMUpdater updater{n_rows, n_buckets, group_col_offset, value_col_offset, row_stride};
    auto lam = [updater](const uint8_t* rows, double* sum_buckets) {
        updater.process(rows, sum_buckets);
    };
    return clangRuntimeSpecializer::specializeLambda<void>(lam);
}

void validate_batch_delta_tradeoff_specialized(int64_t n_rows, int n_buckets,
                                                int group_col_offset, int value_col_offset,
                                                int row_stride) {
    std::vector<uint8_t> test_data(static_cast<size_t>(n_rows) * row_stride, 0);

    for (int64_t i = 0; i < n_rows; ++i) {
        int32_t gk  = static_cast<int32_t>(i % n_buckets);
        double  val = 1.0;
        __builtin_memcpy(test_data.data() + i * row_stride + group_col_offset, &gk, sizeof(int32_t));
        __builtin_memcpy(test_data.data() + i * row_stride + value_col_offset, &val, sizeof(double));
    }

    BatchIVMUpdater ref_updater{n_rows, n_buckets, group_col_offset, value_col_offset, row_stride};
    std::vector<double> ref_buckets(n_buckets, 0.0);
    ref_updater.process(test_data.data(), ref_buckets.data());

    auto spec = create_batch_delta_tradeoff_specialized(
        n_rows, n_buckets, group_col_offset, value_col_offset, row_stride);
    std::vector<double> spec_buckets(n_buckets, 0.0);
    spec(test_data.data(), spec_buckets.data());

    for (int b = 0; b < n_buckets; ++b) {
        if (ref_buckets[b] != spec_buckets[b]) {
            throw std::runtime_error(
                "validate_batch_delta_tradeoff_specialized: bucket mismatch at index " +
                std::to_string(b));
        }
    }
}
