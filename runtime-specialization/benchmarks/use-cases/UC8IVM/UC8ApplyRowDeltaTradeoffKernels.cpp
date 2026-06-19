#include "UC8Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <stdexcept>
#include <vector>
#include <string>

// IVMUpdater captures layout params as struct fields (specialization constants).
struct IVMUpdater {
    int n_buckets;
    int group_col_offset;
    int value_col_offset;
    int row_stride;

    void update(const uint8_t* row, double* buckets) const {
        int32_t group_key;
        __builtin_memcpy(&group_key, row + group_col_offset, sizeof(int32_t));
        int bucket = ((group_key % n_buckets) + n_buckets) % n_buckets;
        double value;
        __builtin_memcpy(&value, row + value_col_offset, sizeof(double));
        buckets[bucket] += value;
    }
};

ApplyRowDeltaBatchTradeoffSpecialized create_apply_row_delta_batch_tradeoff_specialized(
        int64_t n_rows, int n_buckets, int group_col_offset, int value_col_offset, int row_stride) {
    auto lam = [n_rows, n_buckets, group_col_offset, value_col_offset, row_stride](
                       const uint8_t* rows, double* buckets) {
        // Reconstruct the updater inside the lambda so specialized code does
        // not depend on a factory-frame closure object lifetime.
        IVMUpdater updater{n_buckets, group_col_offset, value_col_offset, row_stride};
        for (int64_t i = 0; i < n_rows; ++i)
            updater.update(rows + static_cast<size_t>(i) * row_stride, buckets);
    };
    return clangRuntimeSpecializer::specializeLambda<void>(lam);
}

void validate_apply_row_delta_batch_tradeoff_specialized(
        int64_t n_rows, int n_buckets, int group_col_offset,
        int value_col_offset, int row_stride) {
    std::vector<uint8_t> test_data(static_cast<size_t>(n_rows) * row_stride, 0);
    for (int64_t i = 0; i < n_rows; ++i) {
        int32_t gk = static_cast<int32_t>(i % n_buckets);
        double val = 1.0;
        __builtin_memcpy(test_data.data() + i * row_stride + group_col_offset, &gk, sizeof(int32_t));
        __builtin_memcpy(test_data.data() + i * row_stride + value_col_offset, &val, sizeof(double));
    }
    IVMUpdater ref_updater{n_buckets, group_col_offset, value_col_offset, row_stride};
    std::vector<double> ref_buckets(n_buckets, 0.0);
    for (int64_t i = 0; i < n_rows; ++i)
        ref_updater.update(test_data.data() + i * row_stride, ref_buckets.data());
    auto spec = create_apply_row_delta_batch_tradeoff_specialized(
        n_rows, n_buckets, group_col_offset, value_col_offset, row_stride);
    std::vector<double> spec_buckets(n_buckets, 0.0);
    spec(test_data.data(), spec_buckets.data());
    for (int b = 0; b < n_buckets; ++b) {
        if (ref_buckets[b] != spec_buckets[b]) {
            throw std::runtime_error(
                "validate_apply_row_delta_batch_tradeoff_specialized: bucket mismatch at index " +
                std::to_string(b));
        }
    }
}

void apply_row_delta_batch_tradeoff_unspecialized(const uint8_t* rows, int64_t n_rows,
                                                   double* buckets, int n_buckets,
                                                   int group_col_offset, int value_col_offset,
                                                   int row_stride) {
    IVMUpdater updater{n_buckets, group_col_offset, value_col_offset, row_stride};
    for (int64_t i = 0; i < n_rows; ++i)
        updater.update(rows + static_cast<size_t>(i) * row_stride, buckets);
}

ApplyRowDeltaTradeoffSpecialized create_apply_row_delta_tradeoff_specialized(
        int n_buckets, int group_col_offset, int value_col_offset, int row_stride) {
    auto lam = [n_buckets, group_col_offset, value_col_offset, row_stride](
                       const uint8_t* row, double* buckets) {
        // Reconstruct the updater inside the lambda so specialized code does
        // not depend on a factory-frame closure object lifetime.
        IVMUpdater updater{n_buckets, group_col_offset, value_col_offset, row_stride};
        updater.update(row, buckets);
    };
    return clangRuntimeSpecializer::specializeLambda<void>(lam);
}

void validate_apply_row_delta_tradeoff_specialized(int n_buckets, int group_col_offset,
                                                    int value_col_offset, int row_stride) {
    constexpr int N_TEST = 100;
    std::vector<uint8_t> test_data(N_TEST * row_stride, 0);

    for (int i = 0; i < N_TEST; ++i) {
        int32_t gk  = static_cast<int32_t>(i % n_buckets);
        double  val = 1.0;
        __builtin_memcpy(test_data.data() + i * row_stride + group_col_offset, &gk, sizeof(int32_t));
        __builtin_memcpy(test_data.data() + i * row_stride + value_col_offset, &val, sizeof(double));
    }

    IVMUpdater ref_updater{n_buckets, group_col_offset, value_col_offset, row_stride};
    std::vector<double> ref_buckets(n_buckets, 0.0);
    for (int i = 0; i < N_TEST; ++i)
        ref_updater.update(test_data.data() + i * row_stride, ref_buckets.data());

    auto spec = create_apply_row_delta_tradeoff_specialized(
        n_buckets, group_col_offset, value_col_offset, row_stride);
    std::vector<double> spec_buckets(n_buckets, 0.0);
    for (int i = 0; i < N_TEST; ++i)
        spec(test_data.data() + i * row_stride, spec_buckets.data());

    for (int b = 0; b < n_buckets; ++b) {
        if (ref_buckets[b] != spec_buckets[b]) {
            throw std::runtime_error(
                "validate_apply_row_delta_tradeoff_specialized: bucket mismatch at index " +
                std::to_string(b));
        }
    }
}
