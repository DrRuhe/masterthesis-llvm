#include "UC12Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <cfloat>
#include <vector>
#include <stdexcept>

// MinMaxAggregator encapsulates row layout fields as specialization constants.
// Caller must initialise min_out to DBL_MAX and max_out to -DBL_MAX.
struct MinMaxAggregator {
    int row_stride;
    int key_offset;
    int value_offset;
    int n_buckets;

    void aggregate_minmax(const uint8_t* rows, int64_t n_rows,
                          double* min_out, double* max_out) const {
        for (int64_t i = 0; i < n_rows; ++i) {
            const uint8_t* row_ptr = rows + (int64_t)i * row_stride;

            int32_t key;
            __builtin_memcpy(&key, row_ptr + key_offset, sizeof(int32_t));
            int bucket = ((key % n_buckets) + n_buckets) % n_buckets;

            double value;
            __builtin_memcpy(&value, row_ptr + value_offset, sizeof(double));

            if (value < min_out[bucket]) min_out[bucket] = value;
            if (value > max_out[bucket]) max_out[bucket] = value;
        }
    }
};

void grouped_minmax_tradeoff_unspecialized(const uint8_t* rows, int64_t n_rows,
                                            int row_stride, int key_offset, int value_offset,
                                            int n_buckets, double* min_buckets, double* max_buckets) {
    MinMaxAggregator agg{row_stride, key_offset, value_offset, n_buckets};
    agg.aggregate_minmax(rows, n_rows, min_buckets, max_buckets);
}

GroupedMinMaxTradeoffSpecialized create_grouped_minmax_tradeoff_specialized(
        int row_stride, int key_offset, int value_offset, int n_buckets) {
    MinMaxAggregator agg{row_stride, key_offset, value_offset, n_buckets};
    auto lam = [agg](const uint8_t* rows, int64_t n_rows,
                     double* min_out, double* max_out) {
        agg.aggregate_minmax(rows, n_rows, min_out, max_out);
    };
    return clangRuntimeSpecializer::specializeLambda<void>(lam);
}

void validate_grouped_minmax_tradeoff_specialized(
        int row_stride, int key_offset, int value_offset, int n_buckets) {
    constexpr int N_TEST = 1000;
    std::vector<uint8_t> test_data(N_TEST * row_stride, 0);

    for (int i = 0; i < N_TEST; ++i) {
        int32_t key = (int32_t)(i % n_buckets);
        __builtin_memcpy(test_data.data() + i * row_stride + key_offset, &key, sizeof(int32_t));
        double value = (double)i;
        __builtin_memcpy(test_data.data() + i * row_stride + value_offset, &value, sizeof(double));
    }

    MinMaxAggregator ref_agg{row_stride, key_offset, value_offset, n_buckets};
    std::vector<double> ref_min(n_buckets, DBL_MAX), ref_max(n_buckets, -DBL_MAX);
    ref_agg.aggregate_minmax(test_data.data(), N_TEST, ref_min.data(), ref_max.data());

    auto spec = create_grouped_minmax_tradeoff_specialized(row_stride, key_offset, value_offset, n_buckets);

    std::vector<double> spec_min(n_buckets, DBL_MAX), spec_max(n_buckets, -DBL_MAX);
    spec(test_data.data(), N_TEST, spec_min.data(), spec_max.data());

    for (int b = 0; b < n_buckets; ++b) {
        if (ref_min[b] != spec_min[b] || ref_max[b] != spec_max[b]) {
            throw std::runtime_error(
                "validate_grouped_minmax_tradeoff_specialized: bucket mismatch at index " +
                std::to_string(b));
        }
    }
}
