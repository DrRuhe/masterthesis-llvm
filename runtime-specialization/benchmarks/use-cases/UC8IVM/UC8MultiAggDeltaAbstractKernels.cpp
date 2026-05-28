#include "UC8Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <stdexcept>
#include <vector>
#include <string>

// Local abstract aggregator interface (independent of other TUs).
struct Aggregator {
    virtual void apply(const uint8_t* row, double* state) const = 0;

};

struct SumAgg : Aggregator {
    int group_col_offset;
    int value_col_offset;
    int n_buckets;
    int row_stride;

    SumAgg(int gco, int vco, int nb, int rs)
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

struct CountAgg : Aggregator {
    int group_col_offset;
    int n_buckets;
    int row_stride;

    CountAgg(int gco, int nb, int rs)
        : group_col_offset(gco), n_buckets(nb), row_stride(rs) {}

    void apply(const uint8_t* row, double* state) const override {
        int32_t group_key;
        __builtin_memcpy(&group_key, row + group_col_offset, sizeof(int32_t));
        int bucket = ((group_key % n_buckets) + n_buckets) % n_buckets;
        state[bucket] += 1.0;
    }
};

// MultiAggregator holds SumAgg and CountAgg BY VALUE as struct members.
// apply_both() dispatches to both sub-aggregators without virtual overhead on the
// outer call — each sub-aggregator's virtual dispatch is devirtualized by the JIT
// because the concrete types are captured by value.
struct MultiAggregator {
    SumAgg   sum_agg;
    CountAgg count_agg;

    void apply_both(const uint8_t* row, double* sum_buckets, double* count_buckets) const {
        sum_agg.apply(row, sum_buckets);
        count_agg.apply(row, count_buckets);
    }
};

MultiAggDeltaBatchAbstractSpecialized create_multi_agg_delta_batch_abstract_specialized(
        int64_t n_rows, int n_buckets, int group_col_offset, int value_col_offset, int row_stride) {
    auto lam = [n_rows, n_buckets, group_col_offset, value_col_offset, row_stride](
                   const uint8_t* rows, double* sum_buckets, double* count_buckets) {
        MultiAggregator agg{
            SumAgg{group_col_offset, value_col_offset, n_buckets, row_stride},
            CountAgg{group_col_offset, n_buckets, row_stride}
        };
        for (int64_t i = 0; i < n_rows; ++i)
            agg.apply_both(rows + static_cast<size_t>(i) * row_stride,
                           sum_buckets, count_buckets);
    };
    return clangRuntimeSpecializer::specializeLambda<void>(lam);
}

void validate_multi_agg_delta_batch_abstract_specialized(
        int64_t n_rows, int n_buckets, int group_col_offset,
        int value_col_offset, int row_stride) {
    std::vector<uint8_t> test_data(static_cast<size_t>(n_rows) * row_stride, 0);
    for (int64_t i = 0; i < n_rows; ++i) {
        int32_t gk = static_cast<int32_t>(i % n_buckets);
        double val = 1.0;
        __builtin_memcpy(test_data.data() + i * row_stride + group_col_offset, &gk, sizeof(int32_t));
        __builtin_memcpy(test_data.data() + i * row_stride + value_col_offset, &val, sizeof(double));
    }
    MultiAggregator ref_agg{
        SumAgg{group_col_offset, value_col_offset, n_buckets, row_stride},
        CountAgg{group_col_offset, n_buckets, row_stride}
    };
    std::vector<double> ref_sum(n_buckets, 0.0), ref_cnt(n_buckets, 0.0);
    for (int64_t i = 0; i < n_rows; ++i)
        ref_agg.apply_both(test_data.data() + i * row_stride,
                           ref_sum.data(), ref_cnt.data());
    auto spec = create_multi_agg_delta_batch_abstract_specialized(
        n_rows, n_buckets, group_col_offset, value_col_offset, row_stride);
    std::vector<double> spec_sum(n_buckets, 0.0), spec_cnt(n_buckets, 0.0);
    spec(test_data.data(), spec_sum.data(), spec_cnt.data());
    for (int b = 0; b < n_buckets; ++b) {
        if (ref_sum[b] != spec_sum[b] || ref_cnt[b] != spec_cnt[b]) {
            throw std::runtime_error(
                "validate_multi_agg_delta_batch_abstract_specialized: bucket mismatch at index " +
                std::to_string(b));
        }
    }
}

MultiAggDeltaAbstractSpecialized create_multi_agg_delta_abstract_specialized(
        int n_buckets, int group_col_offset, int value_col_offset, int row_stride) {
    // Capture scalars only; reconstruct objects inside the lambda so their this-pointers
    // are local variables (not stale factory-frame stack addresses).
    auto lam = [n_buckets, group_col_offset, value_col_offset, row_stride](
                   const uint8_t* row, double* sum_buckets, double* count_buckets) {
        MultiAggregator agg{
            SumAgg{group_col_offset, value_col_offset, n_buckets, row_stride},
            CountAgg{group_col_offset, n_buckets, row_stride}
        };
        agg.apply_both(row, sum_buckets, count_buckets);
    };
    return clangRuntimeSpecializer::specializeLambda<void>(lam);
}

void validate_multi_agg_delta_abstract_specialized(int n_buckets, int group_col_offset,
                                                    int value_col_offset, int row_stride) {
    constexpr int N_TEST = 100;
    std::vector<uint8_t> test_data(N_TEST * row_stride, 0);

    for (int i = 0; i < N_TEST; ++i) {
        int32_t gk  = static_cast<int32_t>(i % n_buckets);
        double  val = 1.0;
        __builtin_memcpy(test_data.data() + i * row_stride + group_col_offset, &gk, sizeof(int32_t));
        __builtin_memcpy(test_data.data() + i * row_stride + value_col_offset, &val, sizeof(double));
    }

    MultiAggregator ref_agg{
        SumAgg{group_col_offset, value_col_offset, n_buckets, row_stride},
        CountAgg{group_col_offset, n_buckets, row_stride}
    };
    std::vector<double> ref_sum(n_buckets, 0.0);
    std::vector<double> ref_cnt(n_buckets, 0.0);
    for (int i = 0; i < N_TEST; ++i)
        ref_agg.apply_both(test_data.data() + i * row_stride, ref_sum.data(), ref_cnt.data());

    auto spec = create_multi_agg_delta_abstract_specialized(
        n_buckets, group_col_offset, value_col_offset, row_stride);
    std::vector<double> spec_sum(n_buckets, 0.0);
    std::vector<double> spec_cnt(n_buckets, 0.0);
    for (int i = 0; i < N_TEST; ++i)
        spec(test_data.data() + i * row_stride, spec_sum.data(), spec_cnt.data());

    for (int b = 0; b < n_buckets; ++b) {
        if (ref_sum[b] != spec_sum[b] || ref_cnt[b] != spec_cnt[b]) {
            throw std::runtime_error(
                "validate_multi_agg_delta_abstract_specialized: bucket mismatch at index " +
                std::to_string(b));
        }
    }
}
