#include "UC8Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <stdexcept>
#include <vector>
#include <string>

// Abstract batch-processor interface.  Concrete subclass defined in this TU so
// the vtable pointer is a JIT constant when the lambda captures a
// SumBatchProcessor by value.
struct BatchProcessor {
    virtual void process_batch(const uint8_t* rows, int64_t n_rows,
                                double* buckets) const = 0;

};

struct SumBatchProcessor : BatchProcessor {
    int n_buckets;
    int group_col_offset;
    int value_col_offset;
    int row_stride;

    SumBatchProcessor(int nb, int gco, int vco, int rs)
        : n_buckets(nb), group_col_offset(gco), value_col_offset(vco), row_stride(rs) {}

    // Full scan — no early exit per FR-011.
    void process_batch(const uint8_t* rows, int64_t n_rows,
                        double* buckets) const override {
        for (int64_t i = 0; i < n_rows; ++i) {
            const uint8_t* row = rows + static_cast<size_t>(i) * row_stride;
            int32_t group_key;
            __builtin_memcpy(&group_key, row + group_col_offset, sizeof(int32_t));
            int bucket = ((group_key % n_buckets) + n_buckets) % n_buckets;
            double value;
            __builtin_memcpy(&value, row + value_col_offset, sizeof(double));
            buckets[bucket] += value;
        }
    }
};

void batch_delta_abstract_unspecialized(const uint8_t* rows, int64_t n_rows,
                                        double* buckets, int n_buckets,
                                        int group_col_offset, int value_col_offset,
                                        int row_stride) {
    SumBatchProcessor proc{n_buckets, group_col_offset, value_col_offset, row_stride};
    proc.process_batch(rows, n_rows, buckets);
}

BatchDeltaAbstractSpecialized create_batch_delta_abstract_specialized(
        int64_t n_rows, int n_buckets, int group_col_offset,
        int value_col_offset, int row_stride) {
    // Capture scalars only; reconstruct object inside the lambda so its this-pointer
    // is a local variable (not a stale factory-frame stack address).
    auto lam = [n_buckets, group_col_offset, value_col_offset, row_stride, n_rows](
                   const uint8_t* rows, double* buckets) {
        SumBatchProcessor proc{n_buckets, group_col_offset, value_col_offset, row_stride};
        proc.process_batch(rows, n_rows, buckets);
    };
    return clangRuntimeSpecializer::specializeLambda<void>(lam);
}

void validate_batch_delta_abstract_specialized(int64_t n_rows, int n_buckets,
                                                int group_col_offset, int value_col_offset,
                                                int row_stride) {
    std::vector<uint8_t> test_data(static_cast<size_t>(n_rows) * row_stride, 0);

    for (int64_t i = 0; i < n_rows; ++i) {
        int32_t gk  = static_cast<int32_t>(i % n_buckets);
        double  val = 1.0;
        __builtin_memcpy(test_data.data() + i * row_stride + group_col_offset, &gk, sizeof(int32_t));
        __builtin_memcpy(test_data.data() + i * row_stride + value_col_offset, &val, sizeof(double));
    }

    SumBatchProcessor ref_proc{n_buckets, group_col_offset, value_col_offset, row_stride};
    std::vector<double> ref_buckets(n_buckets, 0.0);
    ref_proc.process_batch(test_data.data(), n_rows, ref_buckets.data());

    auto spec = create_batch_delta_abstract_specialized(
        n_rows, n_buckets, group_col_offset, value_col_offset, row_stride);
    std::vector<double> spec_buckets(n_buckets, 0.0);
    spec(test_data.data(), spec_buckets.data());

    for (int b = 0; b < n_buckets; ++b) {
        if (ref_buckets[b] != spec_buckets[b]) {
            throw std::runtime_error(
                "validate_batch_delta_abstract_specialized: bucket mismatch at index " +
                std::to_string(b));
        }
    }
}
