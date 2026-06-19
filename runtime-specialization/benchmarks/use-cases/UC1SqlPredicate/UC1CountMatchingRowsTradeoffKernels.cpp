#include "UC1Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <vector>

// RowScanner struct captures col_offset and row_stride as specialization constants.
// scan() extracts a double from each row via __builtin_memcpy and counts rows above threshold.
struct RowScanner {
    int col_offset;
    int row_stride;
    int64_t scan(const uint8_t* rows, int64_t n, double threshold) const {
        int64_t count = 0;
        for (int64_t i = 0; i < n; ++i) {
            double val;
            __builtin_memcpy(&val, rows + i * row_stride + col_offset, sizeof(double));
            if (val > threshold)
                ++count;
        }
        return count;
    }
};

CountMatchingRowsTradeoffSpecialized create_count_matching_rows_tradeoff_specialized(
        int col_offset, int row_stride, double threshold) {
    auto lam = [col_offset, row_stride, threshold](const uint8_t* rows, int64_t n) -> int64_t {
        // Reconstruct the scanner inside the lambda so specialized code does
        // not depend on a factory-frame closure object lifetime.
        RowScanner scanner{col_offset, row_stride};
        return scanner.scan(rows, n, threshold);
    };
    return clangRuntimeSpecializer::specializeLambda<int64_t>(lam);
}

int64_t count_matching_rows_tradeoff_unspecialized(const uint8_t* rows, int64_t n,
                                                    int col_offset, int row_stride,
                                                    double threshold) {
    RowScanner scanner{col_offset, row_stride};
    return scanner.scan(rows, n, threshold);
}

void validate_count_matching_rows_tradeoff_specialized(int col_offset, int row_stride, double threshold) {
    constexpr int N_TEST = 100;
    std::vector<uint8_t> test_data(N_TEST * row_stride, 0);

    for (int i = 0; i < N_TEST; ++i) {
        double v = (i < 50) ? 0.25 : 0.75;
        __builtin_memcpy(test_data.data() + i * row_stride + col_offset, &v, sizeof(double));
    }

    auto spec = create_count_matching_rows_tradeoff_specialized(col_offset, row_stride, threshold);
    RowScanner ref_scanner{col_offset, row_stride};

    struct TestCase { const uint8_t* rows; int64_t n_rows; };
    TestCase cases[] = {
        { test_data.data(), N_TEST },
        { test_data.data(), 50 },
        { test_data.data(), 0 },
    };

    for (const auto& tc : cases) {
        int64_t expected = ref_scanner.scan(tc.rows, tc.n_rows, threshold);
        int64_t actual   = spec(tc.rows, tc.n_rows);
        if (expected != actual)
            throw clangRuntimeSpecializer::ClangRuntimeSpecializerChangesBehaviorError(
                "validate_count_matching_rows_tradeoff_specialized: result mismatch");
    }

    CRS_LOG(Info, "validate_count_matching_rows_tradeoff_specialized passed");
}
