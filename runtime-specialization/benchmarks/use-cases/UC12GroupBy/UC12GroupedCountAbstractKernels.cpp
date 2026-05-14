#include "UC12Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <vector>
#include <stdexcept>

// Abstract aggregation interface — redefined in this TU so vtable resolution is
// local and the concrete subclass vtable becomes a JIT constant.
struct AggregationOperator {
    virtual void aggregate(const uint8_t* rows, int64_t n_rows,
                           double* out_buckets, int n_buckets) = 0;

};

// CountOperator counts rows per bucket. Writes int64_t counts reinterpreted
// through a double* output buffer (same layout, 8 bytes per slot).
struct CountOperator : AggregationOperator {
    int row_stride;
    int key_offset;

    CountOperator(int row_stride, int key_offset)
        : row_stride(row_stride), key_offset(key_offset) {}

    void aggregate(const uint8_t* rows, int64_t n_rows,
                   double* out_buckets, int n_buckets) override {
        int64_t* counts = reinterpret_cast<int64_t*>(out_buckets);
        for (int64_t i = 0; i < n_rows; ++i) {
            const uint8_t* row_ptr = rows + (int64_t)i * row_stride;

            int32_t key;
            __builtin_memcpy(&key, row_ptr + key_offset, sizeof(int32_t));
            int bucket = ((key % n_buckets) + n_buckets) % n_buckets;

            counts[bucket] += 1;
        }
    }
};

GroupedCountAbstractSpecialized create_grouped_count_abstract_specialized(
        int row_stride, int key_offset, int n_buckets) {
    // Capture scalars only; reconstruct object inside the lambda so its this-pointer
    // is a local variable (not a stale factory-frame stack address).
    auto lam = [row_stride, key_offset, n_buckets](
                   const uint8_t* rows, int64_t n_rows, int64_t* out) {
        CountOperator op{row_stride, key_offset};
        op.aggregate(rows, n_rows, reinterpret_cast<double*>(out), n_buckets);
    };
    return clangRuntimeSpecializer::specializeLambda<void>(lam);
}

void validate_grouped_count_abstract_specialized(
        int row_stride, int key_offset, int n_buckets) {
    constexpr int N_TEST = 1000;
    std::vector<uint8_t> test_data(N_TEST * row_stride, 0);

    for (int i = 0; i < N_TEST; ++i) {
        int32_t key = (int32_t)(i % n_buckets);
        __builtin_memcpy(test_data.data() + i * row_stride + key_offset, &key, sizeof(int32_t));
    }

    CountOperator ref_op{row_stride, key_offset};
    std::vector<int64_t> ref_buckets(n_buckets, 0);
    ref_op.aggregate(test_data.data(), N_TEST,
                     reinterpret_cast<double*>(ref_buckets.data()), n_buckets);

    auto spec = create_grouped_count_abstract_specialized(row_stride, key_offset, n_buckets);

    std::vector<int64_t> spec_buckets(n_buckets, 0);
    spec(test_data.data(), N_TEST, spec_buckets.data());

    for (int b = 0; b < n_buckets; ++b) {
        if (ref_buckets[b] != spec_buckets[b]) {
            throw std::runtime_error(
                "validate_grouped_count_abstract_specialized: bucket mismatch at index " +
                std::to_string(b));
        }
    }
}
