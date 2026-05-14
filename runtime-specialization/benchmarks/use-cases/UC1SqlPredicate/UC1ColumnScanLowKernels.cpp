#include "UC1Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <vector>

// Scans all rows, writes matching row indices as int32_t to out, returns match count.
// Full scan — processes all rows, no early exit per FR-011.
static int64_t column_scan(const uint8_t* rows, int64_t n_rows,
                            int row_stride, int col_offset, double threshold,
                            int32_t* out) {
    int64_t count = 0;
    for (int64_t i = 0; i < n_rows; ++i) {
        double val;
        __builtin_memcpy(&val, rows + i * row_stride + col_offset, sizeof(double));
        if (val > threshold)
            out[count++] = static_cast<int32_t>(i);
    }
    return count;
}

ColumnScanLowSpecialized create_column_scan_low_specialized(
        int row_stride, int col_offset, double threshold) {
    auto lam = [row_stride, col_offset, threshold](
                    const uint8_t* rows, int64_t n_rows, int32_t* out) -> int64_t {
        return column_scan(rows, n_rows, row_stride, col_offset, threshold, out);
    };
    return clangRuntimeSpecializer::specializeLambda<int64_t>(lam);
}

void validate_column_scan_low_specialized(int row_stride, int col_offset, double threshold) {
    constexpr int N_TEST = 100;
    std::vector<uint8_t> test_data(N_TEST * row_stride, 0);
    std::vector<int32_t> out_ref(N_TEST), out_spec(N_TEST);

    for (int i = 0; i < N_TEST; ++i) {
        double v = (i < 50) ? 0.25 : 0.75;
        __builtin_memcpy(test_data.data() + i * row_stride + col_offset, &v, sizeof(double));
    }

    auto spec = create_column_scan_low_specialized(row_stride, col_offset, threshold);

    struct TestCase { const uint8_t* rows; int64_t n_rows; };
    TestCase cases[] = {
        { test_data.data(), N_TEST },
        { test_data.data(), 50 },
        { test_data.data(), 0 },
    };

    for (const auto& tc : cases) {
        int64_t expected_count = column_scan(tc.rows, tc.n_rows, row_stride, col_offset, threshold, out_ref.data());
        int64_t actual_count   = spec(tc.rows, tc.n_rows, out_spec.data());
        if (expected_count != actual_count)
            throw clangRuntimeSpecializer::ClangRuntimeSpecializerChangesBehaviorError(
                "validate_column_scan_low_specialized: count mismatch");
        for (int64_t j = 0; j < expected_count; ++j) {
            if (out_ref[j] != out_spec[j])
                throw clangRuntimeSpecializer::ClangRuntimeSpecializerChangesBehaviorError(
                    "validate_column_scan_low_specialized: output index mismatch");
        }
    }

    CRS_LOG(Info, "validate_column_scan_low_specialized passed");
}
