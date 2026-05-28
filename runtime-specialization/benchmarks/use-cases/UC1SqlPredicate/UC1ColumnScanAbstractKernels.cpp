#include "UC1Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <vector>

// Abstract result-collector interface. Concrete subclass defined in this TU so
// the vtable is a JIT constant when captured by value.
struct ResultCollector {
    virtual void emit(int64_t row_idx) = 0;

};

// BufferCollector writes matching row indices as int32_t to an output array and
// tracks the count. Captures the out pointer by value in the lambda closure.
struct BufferCollector : ResultCollector {
    int32_t* out;
    int64_t  count;

    explicit BufferCollector(int32_t* out) : out(out), count(0) {}

    void emit(int64_t row_idx) override {
        out[count++] = static_cast<int32_t>(row_idx);
    }
};

// Full scan — no early exit per FR-011.
void scan_collecting(const uint8_t* rows, int64_t n,
                             int row_stride, int col_offset, double threshold,
                             ResultCollector& collector) {
    for (int64_t i = 0; i < n; ++i) {
        double val;
        __builtin_memcpy(&val, rows + i * row_stride + col_offset, sizeof(double));
        if (val > threshold)
            collector.emit(i);
    }
}

ColumnScanAbstractSpecialized create_column_scan_abstract_specialized(
        int row_stride, int col_offset, double threshold) {
    // Lambda captures BufferCollector BY VALUE (includes the out pointer field and count).
    // The out pointer is rebound each call via the lambda argument to support reuse.
    auto lam = [row_stride, col_offset, threshold](
                    const uint8_t* rows, int64_t n, int32_t* out) -> int64_t {
        BufferCollector collector{out};
        scan_collecting(rows, n, row_stride, col_offset, threshold, collector);
        return collector.count;
    };
    return clangRuntimeSpecializer::specializeLambda<int64_t>(lam);
}

int64_t column_scan_abstract_unspecialized(const uint8_t* rows, int64_t n,
                                           int row_stride, int col_offset,
                                           double threshold, int32_t* out) {
    BufferCollector collector{out};
    scan_collecting(rows, n, row_stride, col_offset, threshold, collector);
    return collector.count;
}

void validate_column_scan_abstract_specialized(int row_stride, int col_offset, double threshold) {
    constexpr int N_TEST = 100;
    std::vector<uint8_t> test_data(N_TEST * row_stride, 0);
    std::vector<int32_t> out_ref(N_TEST), out_spec(N_TEST);

    for (int i = 0; i < N_TEST; ++i) {
        double v = (i < 50) ? 0.25 : 0.75;
        __builtin_memcpy(test_data.data() + i * row_stride + col_offset, &v, sizeof(double));
    }

    auto spec = create_column_scan_abstract_specialized(row_stride, col_offset, threshold);

    struct TestCase { const uint8_t* rows; int64_t n_rows; };
    TestCase cases[] = {
        { test_data.data(), N_TEST },
        { test_data.data(), 50 },
        { test_data.data(), 0 },
    };

    for (const auto& tc : cases) {
        // Reference: collect into out_ref directly.
        BufferCollector ref_collector{out_ref.data()};
        scan_collecting(tc.rows, tc.n_rows, row_stride, col_offset, threshold, ref_collector);
        int64_t expected_count = ref_collector.count;

        int64_t actual_count = spec(tc.rows, tc.n_rows, out_spec.data());
        if (expected_count != actual_count)
            throw clangRuntimeSpecializer::ClangRuntimeSpecializerChangesBehaviorError(
                "validate_column_scan_abstract_specialized: count mismatch");
        for (int64_t j = 0; j < expected_count; ++j) {
            if (out_ref[j] != out_spec[j])
                throw clangRuntimeSpecializer::ClangRuntimeSpecializerChangesBehaviorError(
                    "validate_column_scan_abstract_specialized: output index mismatch");
        }
    }

    CRS_LOG(Info, "validate_column_scan_abstract_specialized passed");
}
