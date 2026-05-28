#include "UC12Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <cfloat>
#include <vector>
#include <stdexcept>

// Abstract aggregation interface — redefined in this TU so vtable resolution is
// local and the concrete subclass vtable becomes a JIT constant.
struct AggregationOperator {
    virtual void aggregate(const uint8_t* rows, int64_t n_rows,
                           double* out_buckets, int n_buckets) = 0;

};

// MinMaxOperator tracks per-bucket min and max in a single pass.
// min_store / max_store are pointers set at construction time; the lambda
// passes the actual output arrays via those pointers on each call.
// Because the operator is captured BY VALUE in the lambda, the pointers are
// effectively specialization constants from the JIT's point of view.
struct MinMaxOperator : AggregationOperator {
    int     row_stride;
    int     key_offset;
    int     value_offset;
    double* min_store;
    double* max_store;

    MinMaxOperator(int row_stride, int key_offset, int value_offset,
                   double* min_store, double* max_store)
        : row_stride(row_stride), key_offset(key_offset), value_offset(value_offset),
          min_store(min_store), max_store(max_store) {}

    void aggregate(const uint8_t* rows, int64_t n_rows,
                   double* /*out_buckets*/, int n_buckets) override {
        for (int64_t i = 0; i < n_rows; ++i) {
            const uint8_t* row_ptr = rows + (int64_t)i * row_stride;

            int32_t key;
            __builtin_memcpy(&key, row_ptr + key_offset, sizeof(int32_t));
            int bucket = ((key % n_buckets) + n_buckets) % n_buckets;

            double value;
            __builtin_memcpy(&value, row_ptr + value_offset, sizeof(double));

            if (value < min_store[bucket]) min_store[bucket] = value;
            if (value > max_store[bucket]) max_store[bucket] = value;
        }
    }
};

void grouped_minmax_abstract_unspecialized(const uint8_t* rows, int64_t n_rows,
                                            int row_stride, int key_offset, int value_offset,
                                            int n_buckets, double* min_buckets, double* max_buckets) {
    MinMaxOperator op{row_stride, key_offset, value_offset, min_buckets, max_buckets};
    op.aggregate(rows, n_rows, nullptr, n_buckets);
}

GroupedMinMaxAbstractSpecialized create_grouped_minmax_abstract_specialized(
        int row_stride, int key_offset, int value_offset, int n_buckets) {
    // Lambda binds min/max output arrays per call; operator pointer fields updated
    // before delegation so the vtable call operates on caller-supplied buffers.
    auto lam = [row_stride, key_offset, value_offset, n_buckets](
                    const uint8_t* rows, int64_t n_rows,
                    double* min_out, double* max_out) {
        MinMaxOperator op{row_stride, key_offset, value_offset, min_out, max_out};
        op.aggregate(rows, n_rows, nullptr, n_buckets);
    };
    return clangRuntimeSpecializer::specializeLambda<void>(lam);
}

void validate_grouped_minmax_abstract_specialized(
        int row_stride, int key_offset, int value_offset, int n_buckets) {
    constexpr int N_TEST = 1000;
    std::vector<uint8_t> test_data(N_TEST * row_stride, 0);

    for (int i = 0; i < N_TEST; ++i) {
        int32_t key = (int32_t)(i % n_buckets);
        __builtin_memcpy(test_data.data() + i * row_stride + key_offset, &key, sizeof(int32_t));
        double value = (double)i;
        __builtin_memcpy(test_data.data() + i * row_stride + value_offset, &value, sizeof(double));
    }

    std::vector<double> ref_min(n_buckets, DBL_MAX), ref_max(n_buckets, -DBL_MAX);
    {
        MinMaxOperator ref_op{row_stride, key_offset, value_offset,
                              ref_min.data(), ref_max.data()};
        ref_op.aggregate(test_data.data(), N_TEST, nullptr, n_buckets);
    }

    auto spec = create_grouped_minmax_abstract_specialized(row_stride, key_offset, value_offset, n_buckets);

    std::vector<double> spec_min(n_buckets, DBL_MAX), spec_max(n_buckets, -DBL_MAX);
    spec(test_data.data(), N_TEST, spec_min.data(), spec_max.data());

    for (int b = 0; b < n_buckets; ++b) {
        if (ref_min[b] != spec_min[b] || ref_max[b] != spec_max[b]) {
            throw std::runtime_error(
                "validate_grouped_minmax_abstract_specialized: bucket mismatch at index " +
                std::to_string(b));
        }
    }
}
