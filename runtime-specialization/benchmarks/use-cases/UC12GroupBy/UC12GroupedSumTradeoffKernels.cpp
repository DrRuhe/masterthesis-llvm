#include "UC12Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <vector>
#include <stdexcept>

// GroupedAggregator encapsulates row_stride, key_offset, value_offset, n_buckets
// as specialization constants captured by value in the lambda closure.
struct GroupedAggregator {
    int row_stride;
    int key_offset;
    int value_offset;
    int n_buckets;

    void aggregate(const uint8_t* rows, int64_t n_rows, double* out) const {
        for (int64_t i = 0; i < n_rows; ++i) {
            const uint8_t* row_ptr = rows + (int64_t)i * row_stride;

            int32_t key;
            __builtin_memcpy(&key, row_ptr + key_offset, sizeof(int32_t));
            int bucket = ((key % n_buckets) + n_buckets) % n_buckets;

            double value;
            __builtin_memcpy(&value, row_ptr + value_offset, sizeof(double));
            out[bucket] += value;
        }
    }
};

GroupedSumTradeoffSpecialized create_grouped_sum_tradeoff_specialized(
        int row_stride, int key_offset, int value_offset, int n_buckets) {
    GroupedAggregator agg{row_stride, key_offset, value_offset, n_buckets};
    auto lam = [agg](const uint8_t* rows, int64_t n_rows, double* out) {
        agg.aggregate(rows, n_rows, out);
    };
    return clangRuntimeSpecializer::specializeLambda<void>(lam);
}

void validate_grouped_sum_tradeoff_specialized(
        int row_stride, int key_offset, int value_offset, int n_buckets) {
    constexpr int N_TEST = 1000;
    std::vector<uint8_t> test_data(N_TEST * row_stride, 0);

    for (int i = 0; i < N_TEST; ++i) {
        int32_t key = (int32_t)(i % n_buckets);
        __builtin_memcpy(test_data.data() + i * row_stride + key_offset, &key, sizeof(int32_t));
        double value = 1.0;
        __builtin_memcpy(test_data.data() + i * row_stride + value_offset, &value, sizeof(double));
    }

    GroupedAggregator ref_agg{row_stride, key_offset, value_offset, n_buckets};
    std::vector<double> ref_buckets(n_buckets, 0.0);
    ref_agg.aggregate(test_data.data(), N_TEST, ref_buckets.data());

    auto spec = create_grouped_sum_tradeoff_specialized(row_stride, key_offset, value_offset, n_buckets);

    std::vector<double> spec_buckets(n_buckets, 0.0);
    spec(test_data.data(), N_TEST, spec_buckets.data());

    for (int b = 0; b < n_buckets; ++b) {
        if (ref_buckets[b] != spec_buckets[b]) {
            throw std::runtime_error(
                "validate_grouped_sum_tradeoff_specialized: bucket mismatch at index " +
                std::to_string(b));
        }
    }
}
