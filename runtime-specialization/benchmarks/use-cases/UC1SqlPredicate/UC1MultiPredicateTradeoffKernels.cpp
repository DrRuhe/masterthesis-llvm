#include "UC1Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <vector>

// BinaryPredicateScanner encapsulates both column offsets and thresholds as
// specialization constants via the struct fields.
struct BinaryPredicateScanner {
    int col_offset_a;
    int col_offset_b;
    double threshold_a;
    double threshold_b;

    // Full scan — no early exit per FR-011.
    int64_t scan(const uint8_t* rows, int64_t n, int row_stride) const {
        int64_t count = 0;
        for (int64_t i = 0; i < n; ++i) {
            double val_a, val_b;
            __builtin_memcpy(&val_a, rows + i * row_stride + col_offset_a, sizeof(double));
            __builtin_memcpy(&val_b, rows + i * row_stride + col_offset_b, sizeof(double));
            if (val_a > threshold_a && val_b > threshold_b)
                ++count;
        }
        return count;
    }
};

MultiPredicateTradeoffSpecialized create_multi_predicate_tradeoff_specialized(
        int row_stride, int col_offset_a, int col_offset_b,
        double threshold_a, double threshold_b) {
    auto lam = [row_stride, col_offset_a, col_offset_b, threshold_a, threshold_b](
                       const uint8_t* rows, int64_t n) -> int64_t {
        // Reconstruct the scanner inside the lambda so specialized code does
        // not depend on a factory-frame closure object lifetime.
        BinaryPredicateScanner scanner{col_offset_a, col_offset_b, threshold_a, threshold_b};
        return scanner.scan(rows, n, row_stride);
    };
    return clangRuntimeSpecializer::specializeLambda<int64_t>(lam);
}

int64_t multi_predicate_tradeoff_unspecialized(const uint8_t* rows, int64_t n,
                                               int row_stride,
                                               int col_offset_a, int col_offset_b,
                                               double threshold_a, double threshold_b) {
    BinaryPredicateScanner scanner{col_offset_a, col_offset_b, threshold_a, threshold_b};
    return scanner.scan(rows, n, row_stride);
}

void validate_multi_predicate_tradeoff_specialized(int row_stride, int col_offset_a, int col_offset_b,
                                                    double threshold_a, double threshold_b) {
    constexpr int N_TEST = 100;
    std::vector<uint8_t> test_data(N_TEST * row_stride, 0);

    for (int i = 0; i < N_TEST; ++i) {
        double va = (i % 3 == 0) ? 0.75 : 0.25;
        double vb = (i % 2 == 0) ? 0.75 : 0.25;
        __builtin_memcpy(test_data.data() + i * row_stride + col_offset_a, &va, sizeof(double));
        __builtin_memcpy(test_data.data() + i * row_stride + col_offset_b, &vb, sizeof(double));
    }

    auto spec = create_multi_predicate_tradeoff_specialized(row_stride, col_offset_a, col_offset_b,
                                                             threshold_a, threshold_b);
    BinaryPredicateScanner ref_scanner{col_offset_a, col_offset_b, threshold_a, threshold_b};

    struct TestCase { const uint8_t* rows; int64_t n_rows; };
    TestCase cases[] = {
        { test_data.data(), N_TEST },
        { test_data.data(), 50 },
        { test_data.data(), 0 },
    };

    for (const auto& tc : cases) {
        int64_t expected = ref_scanner.scan(tc.rows, tc.n_rows, row_stride);
        int64_t actual   = spec(tc.rows, tc.n_rows);
        if (expected != actual)
            throw clangRuntimeSpecializer::ClangRuntimeSpecializerChangesBehaviorError(
                "validate_multi_predicate_tradeoff_specialized: result mismatch");
    }

    CRS_LOG(Info, "validate_multi_predicate_tradeoff_specialized passed");
}
