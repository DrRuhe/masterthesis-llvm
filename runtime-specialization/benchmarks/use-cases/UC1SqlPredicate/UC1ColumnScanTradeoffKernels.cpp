#include "UC1Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <vector>

// ProjectingScanner encapsulates col_offset as a specialization constant.
// scan_to() writes matching row indices to out and returns count.
// Full scan — no early exit per FR-011.
struct ProjectingScanner {
    int col_offset;

    int64_t scan_to(const uint8_t* rows, int64_t n,
                    int row_stride, double threshold, int32_t* out) const {
        int64_t count = 0;
        for (int64_t i = 0; i < n; ++i) {
            double val;
            __builtin_memcpy(&val, rows + i * row_stride + col_offset, sizeof(double));
            if (val > threshold)
                out[count++] = static_cast<int32_t>(i);
        }
        return count;
    }
};

ColumnScanTradeoffSpecialized create_column_scan_tradeoff_specialized(
        int row_stride, int col_offset, double threshold) {
    ProjectingScanner scanner{col_offset};
    auto lam = [scanner, row_stride, threshold](
                    const uint8_t* rows, int64_t n, int32_t* out) -> int64_t {
        return scanner.scan_to(rows, n, row_stride, threshold, out);
    };
    return clangRuntimeSpecializer::specializeLambda<int64_t>(lam);
}

int64_t column_scan_tradeoff_unspecialized(const uint8_t* rows, int64_t n,
                                           int row_stride, int col_offset,
                                           double threshold, int32_t* out) {
    ProjectingScanner scanner{col_offset};
    return scanner.scan_to(rows, n, row_stride, threshold, out);
}

void validate_column_scan_tradeoff_specialized(int row_stride, int col_offset, double threshold) {
    constexpr int N_TEST = 100;
    std::vector<uint8_t> test_data(N_TEST * row_stride, 0);
    std::vector<int32_t> out_ref(N_TEST), out_spec(N_TEST);

    for (int i = 0; i < N_TEST; ++i) {
        double v = (i < 50) ? 0.25 : 0.75;
        __builtin_memcpy(test_data.data() + i * row_stride + col_offset, &v, sizeof(double));
    }

    auto spec = create_column_scan_tradeoff_specialized(row_stride, col_offset, threshold);
    ProjectingScanner ref_scanner{col_offset};

    struct TestCase { const uint8_t* rows; int64_t n_rows; };
    TestCase cases[] = {
        { test_data.data(), N_TEST },
        { test_data.data(), 50 },
        { test_data.data(), 0 },
    };

    for (const auto& tc : cases) {
        int64_t expected_count = ref_scanner.scan_to(tc.rows, tc.n_rows, row_stride, threshold, out_ref.data());
        int64_t actual_count   = spec(tc.rows, tc.n_rows, out_spec.data());
        if (expected_count != actual_count)
            throw clangRuntimeSpecializer::ClangRuntimeSpecializerChangesBehaviorError(
                "validate_column_scan_tradeoff_specialized: count mismatch");
        for (int64_t j = 0; j < expected_count; ++j) {
            if (out_ref[j] != out_spec[j])
                throw clangRuntimeSpecializer::ClangRuntimeSpecializerChangesBehaviorError(
                    "validate_column_scan_tradeoff_specialized: output index mismatch");
        }
    }

    CRS_LOG(Info, "validate_column_scan_tradeoff_specialized passed");
}
