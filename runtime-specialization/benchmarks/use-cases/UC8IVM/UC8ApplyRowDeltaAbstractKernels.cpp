#include "UC8Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <stdexcept>
#include <vector>
#include <string>

// Abstract aggregator interface.  Concrete subclass defined in this TU so the
// vtable pointer is a JIT constant when the lambda captures a SumAggregator by value.
struct Aggregator {
    virtual void apply(const uint8_t* row, double* state) const = 0;

};

struct SumAggregator : Aggregator {
    int group_col_offset;
    int value_col_offset;
    int n_buckets;
    int row_stride;

    SumAggregator(int gco, int vco, int nb, int rs)
        : group_col_offset(gco), value_col_offset(vco), n_buckets(nb), row_stride(rs) {}

    void apply(const uint8_t* row, double* state) const override {
        int32_t group_key;
        __builtin_memcpy(&group_key, row + group_col_offset, sizeof(int32_t));
        int bucket = ((group_key % n_buckets) + n_buckets) % n_buckets;
        double value;
        __builtin_memcpy(&value, row + value_col_offset, sizeof(double));
        state[bucket] += value;
    }
};

ApplyRowDeltaAbstractSpecialized create_apply_row_delta_abstract_specialized(
        int n_buckets, int group_col_offset, int value_col_offset, int row_stride) {
    // Capture scalars only; reconstruct object inside the lambda so its this-pointer
    // is a local variable (not a stale factory-frame stack address).
    auto lam = [group_col_offset, value_col_offset, n_buckets, row_stride](
                   const uint8_t* row, double* state) {
        SumAggregator agg{group_col_offset, value_col_offset, n_buckets, row_stride};
        agg.apply(row, state);
    };
    return clangRuntimeSpecializer::specializeLambda<void>(lam);
}

void validate_apply_row_delta_abstract_specialized(int n_buckets, int group_col_offset,
                                                    int value_col_offset, int row_stride) {
    constexpr int N_TEST = 100;
    std::vector<uint8_t> test_data(N_TEST * row_stride, 0);

    for (int i = 0; i < N_TEST; ++i) {
        int32_t gk  = static_cast<int32_t>(i % n_buckets);
        double  val = 1.0;
        __builtin_memcpy(test_data.data() + i * row_stride + group_col_offset, &gk, sizeof(int32_t));
        __builtin_memcpy(test_data.data() + i * row_stride + value_col_offset, &val, sizeof(double));
    }

    SumAggregator ref_agg{group_col_offset, value_col_offset, n_buckets, row_stride};
    std::vector<double> ref_buckets(n_buckets, 0.0);
    for (int i = 0; i < N_TEST; ++i)
        ref_agg.apply(test_data.data() + i * row_stride, ref_buckets.data());

    auto spec = create_apply_row_delta_abstract_specialized(
        n_buckets, group_col_offset, value_col_offset, row_stride);
    std::vector<double> spec_buckets(n_buckets, 0.0);
    for (int i = 0; i < N_TEST; ++i)
        spec(test_data.data() + i * row_stride, spec_buckets.data());

    for (int b = 0; b < n_buckets; ++b) {
        if (ref_buckets[b] != spec_buckets[b]) {
            throw std::runtime_error(
                "validate_apply_row_delta_abstract_specialized: bucket mismatch at index " +
                std::to_string(b));
        }
    }
}
