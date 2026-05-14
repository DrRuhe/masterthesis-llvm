#include "UC12Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <vector>
#include <stdexcept>

// Abstract aggregation interface. Concrete subclass defined in this TU so the
// vtable is a JIT constant when the lambda captures a SumOperator by value.
struct AggregationOperator {
    virtual void aggregate(const uint8_t* rows, int64_t n_rows,
                           double* out_buckets, int n_buckets) = 0;

};

struct SumOperator : AggregationOperator {
    int row_stride;
    int key_offset;
    int value_offset;

    SumOperator(int row_stride, int key_offset, int value_offset)
        : row_stride(row_stride), key_offset(key_offset), value_offset(value_offset) {}

    void aggregate(const uint8_t* rows, int64_t n_rows,
                   double* out_buckets, int n_buckets) override {
        for (int64_t i = 0; i < n_rows; ++i) {
            const uint8_t* row_ptr = rows + (int64_t)i * row_stride;

            int32_t key;
            __builtin_memcpy(&key, row_ptr + key_offset, sizeof(int32_t));
            int bucket = ((key % n_buckets) + n_buckets) % n_buckets;

            double value;
            __builtin_memcpy(&value, row_ptr + value_offset, sizeof(double));
            out_buckets[bucket] += value;
        }
    }
};

GroupedSumAbstractSpecialized create_grouped_sum_abstract_specialized(
        int row_stride, int key_offset, int value_offset, int n_buckets) {
    // Capture scalars only; reconstruct object inside the lambda so its this-pointer
    // is a local variable (not a stale factory-frame stack address).
    auto lam = [row_stride, key_offset, value_offset, n_buckets](
                   const uint8_t* rows, int64_t n_rows, double* out) {
        SumOperator op{row_stride, key_offset, value_offset};
        op.aggregate(rows, n_rows, out, n_buckets);
    };
    return clangRuntimeSpecializer::specializeLambda<void>(lam);
}

void validate_grouped_sum_abstract_specialized(
        int row_stride, int key_offset, int value_offset, int n_buckets) {
    constexpr int N_TEST = 1000;
    std::vector<uint8_t> test_data(N_TEST * row_stride, 0);

    for (int i = 0; i < N_TEST; ++i) {
        int32_t key = (int32_t)(i % n_buckets);
        __builtin_memcpy(test_data.data() + i * row_stride + key_offset, &key, sizeof(int32_t));
        double value = 1.0;
        __builtin_memcpy(test_data.data() + i * row_stride + value_offset, &value, sizeof(double));
    }

    SumOperator ref_op{row_stride, key_offset, value_offset};
    std::vector<double> ref_buckets(n_buckets, 0.0);
    ref_op.aggregate(test_data.data(), N_TEST, ref_buckets.data(), n_buckets);

    auto spec = create_grouped_sum_abstract_specialized(row_stride, key_offset, value_offset, n_buckets);

    std::vector<double> spec_buckets(n_buckets, 0.0);
    spec(test_data.data(), N_TEST, spec_buckets.data());

    for (int b = 0; b < n_buckets; ++b) {
        if (ref_buckets[b] != spec_buckets[b]) {
            throw std::runtime_error(
                "validate_grouped_sum_abstract_specialized: bucket mismatch at index " +
                std::to_string(b));
        }
    }
}
