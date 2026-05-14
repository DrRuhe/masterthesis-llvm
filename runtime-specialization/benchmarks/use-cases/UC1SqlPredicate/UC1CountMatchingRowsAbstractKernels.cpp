#include "UC1Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <vector>

// Abstract predicate interface. Concrete subclass defined in this TU so the
// vtable is a JIT constant when the lambda captures a ThresholdPredicate by value.
struct Predicate {
    virtual bool test(const uint8_t* row) const = 0;
    virtual ~Predicate() = default;
};

struct ThresholdPredicate : Predicate {
    int col_offset;
    int row_stride;
    double threshold;

    ThresholdPredicate(int col_offset, int row_stride, double threshold)
        : col_offset(col_offset), row_stride(row_stride), threshold(threshold) {}

    bool test(const uint8_t* row) const override {
        double val;
        __builtin_memcpy(&val, row + col_offset, sizeof(double));
        return val > threshold;
    }
};

// Full scan — no early exit per FR-011.
static int64_t scan_with_predicate(const uint8_t* rows, int64_t n,
                                    int row_stride, const Predicate& pred) {
    int64_t count = 0;
    for (int64_t i = 0; i < n; ++i) {
        if (pred.test(rows + i * row_stride))
            ++count;
    }
    return count;
}

CountMatchingRowsAbstractSpecialized create_count_matching_rows_abstract_specialized(
        int col_offset, int row_stride, double threshold) {
    ThresholdPredicate pred{col_offset, row_stride, threshold};
    // Capture ThresholdPredicate BY VALUE so the vtable pointer is a JIT constant.
    auto lam = [pred, row_stride](const uint8_t* rows, int64_t n) -> int64_t {
        return scan_with_predicate(rows, n, row_stride, pred);
    };
    return clangRuntimeSpecializer::specializeLambda<int64_t>(lam);
}

void validate_count_matching_rows_abstract_specialized(int col_offset, int row_stride, double threshold) {
    constexpr int N_TEST = 100;
    std::vector<uint8_t> test_data(N_TEST * row_stride, 0);

    for (int i = 0; i < N_TEST; ++i) {
        double v = (i < 50) ? 0.25 : 0.75;
        __builtin_memcpy(test_data.data() + i * row_stride + col_offset, &v, sizeof(double));
    }

    auto spec = create_count_matching_rows_abstract_specialized(col_offset, row_stride, threshold);
    ThresholdPredicate ref_pred{col_offset, row_stride, threshold};

    struct TestCase { const uint8_t* rows; int64_t n_rows; };
    TestCase cases[] = {
        { test_data.data(), N_TEST },
        { test_data.data(), 50 },
        { test_data.data(), 0 },
    };

    for (const auto& tc : cases) {
        int64_t expected = scan_with_predicate(tc.rows, tc.n_rows, row_stride, ref_pred);
        int64_t actual   = spec(tc.rows, tc.n_rows);
        if (expected != actual)
            throw clangRuntimeSpecializer::ClangRuntimeSpecializerChangesBehaviorError(
                "validate_count_matching_rows_abstract_specialized: result mismatch");
    }

    CRS_LOG(Info, "validate_count_matching_rows_abstract_specialized passed");
}
