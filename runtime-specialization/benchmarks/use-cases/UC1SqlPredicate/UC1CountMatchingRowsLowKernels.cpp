#include "UC1Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <vector>
#include <tuple>

// CRITICAL: count_matching_rows must live in the same TU as the lambda factory
// so the JIT module clone contains both for full inlining.
int64_t count_matching_rows(const uint8_t* rows, int64_t n_rows,
                             int row_stride, int col_offset, double threshold) {
    int64_t count = 0;
    for (int64_t i = 0; i < n_rows; ++i) {
        double val;
        __builtin_memcpy(&val, rows + i * row_stride + col_offset, sizeof(double));
        if (val > threshold)
            ++count;
    }
    return count;
}

SQLSpecialized create_sql_specialized(int row_stride, int col_offset, double threshold) {
    auto lam = [row_stride, col_offset, threshold](const uint8_t* rows, int64_t n_rows) -> int64_t {
        return count_matching_rows(rows, n_rows, row_stride, col_offset, threshold);
    };
    return clangRuntimeSpecializer::specializeLambda<int64_t>(lam);
}

void validate_sql_specialized(int row_stride, int col_offset, double threshold) {
    // Allocate a small test dataset: 100 rows of row_stride bytes each.
    constexpr int N_TEST = 100;
    std::vector<uint8_t> test_data(N_TEST * row_stride, 0);

    // Write known double values at col_offset for each row.
    // Rows 0..49 get value 0.25 (below 0.5), rows 50..99 get value 0.75 (above 0.5).
    for (int i = 0; i < N_TEST; ++i) {
        double v = (i < 50) ? 0.25 : 0.75;
        __builtin_memcpy(test_data.data() + i * row_stride + col_offset, &v, sizeof(double));
    }

    // Manual validation: call create_sql_specialized and compare against
    // direct count_matching_rows calls on a few test inputs.
    auto spec = create_sql_specialized(row_stride, col_offset, threshold);

    struct TestCase { const uint8_t* rows; int64_t n_rows; };
    TestCase cases[] = {
        { test_data.data(), N_TEST },
        { test_data.data(), 50 },
        { test_data.data(), 0 },
    };

    for (const auto& tc : cases) {
        int64_t expected = count_matching_rows(tc.rows, tc.n_rows, row_stride, col_offset, threshold);
        int64_t actual   = spec(tc.rows, tc.n_rows);
        if (expected != actual)
            throw clangRuntimeSpecializer::ClangRuntimeSpecializerChangesBehaviorError(
                "validate_sql_specialized: result mismatch");
    }

    CRS_LOG(Info, "assertSpecializedLambdaIsEquivalent passed");
}
