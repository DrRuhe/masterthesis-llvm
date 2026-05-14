#include "UC1Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <vector>

// Abstract predicate interface — local definition (not shared with other TUs).
struct Predicate {
    virtual bool test(const uint8_t* row) const = 0;
    virtual ~Predicate() = default;
};

struct ThresholdPredicate : Predicate {
    int col_offset;
    double threshold;

    ThresholdPredicate(int col_offset, double threshold)
        : col_offset(col_offset), threshold(threshold) {}

    bool test(const uint8_t* row) const override {
        double val;
        __builtin_memcpy(&val, row + col_offset, sizeof(double));
        return val > threshold;
    }
};

// AndPredicate holds two ThresholdPredicates BY VALUE (not pointers).
// test() calls BOTH sub-predicates — no short-circuit, no early exit per FR-011.
struct AndPredicate : Predicate {
    ThresholdPredicate pred_a;
    ThresholdPredicate pred_b;

    AndPredicate(ThresholdPredicate a, ThresholdPredicate b)
        : pred_a(a), pred_b(b) {}

    bool test(const uint8_t* row) const override {
        bool ra = pred_a.test(row);
        bool rb = pred_b.test(row);
        return ra & rb; // bitwise AND ensures both are always evaluated (no short-circuit)
    }
};

// Full scan — no early exit per FR-011.
static int64_t scan_with_predicate_and(const uint8_t* rows, int64_t n,
                                        int row_stride, const Predicate& pred) {
    int64_t count = 0;
    for (int64_t i = 0; i < n; ++i) {
        if (pred.test(rows + i * row_stride))
            ++count;
    }
    return count;
}

MultiPredicateAbstractSpecialized create_multi_predicate_abstract_specialized(
        int row_stride, int col_offset_a, int col_offset_b,
        double threshold_a, double threshold_b) {
    // Capture AndPredicate BY VALUE so vtable is a JIT constant.
    AndPredicate pred{
        ThresholdPredicate{col_offset_a, threshold_a},
        ThresholdPredicate{col_offset_b, threshold_b}
    };
    auto lam = [pred, row_stride](const uint8_t* rows, int64_t n) -> int64_t {
        return scan_with_predicate_and(rows, n, row_stride, pred);
    };
    return clangRuntimeSpecializer::specializeLambda<int64_t>(lam);
}

void validate_multi_predicate_abstract_specialized(int row_stride, int col_offset_a, int col_offset_b,
                                                    double threshold_a, double threshold_b) {
    constexpr int N_TEST = 100;
    std::vector<uint8_t> test_data(N_TEST * row_stride, 0);

    for (int i = 0; i < N_TEST; ++i) {
        double va = (i % 3 == 0) ? 0.75 : 0.25;
        double vb = (i % 2 == 0) ? 0.75 : 0.25;
        __builtin_memcpy(test_data.data() + i * row_stride + col_offset_a, &va, sizeof(double));
        __builtin_memcpy(test_data.data() + i * row_stride + col_offset_b, &vb, sizeof(double));
    }

    auto spec = create_multi_predicate_abstract_specialized(row_stride, col_offset_a, col_offset_b,
                                                             threshold_a, threshold_b);
    AndPredicate ref_pred{
        ThresholdPredicate{col_offset_a, threshold_a},
        ThresholdPredicate{col_offset_b, threshold_b}
    };

    struct TestCase { const uint8_t* rows; int64_t n_rows; };
    TestCase cases[] = {
        { test_data.data(), N_TEST },
        { test_data.data(), 50 },
        { test_data.data(), 0 },
    };

    for (const auto& tc : cases) {
        int64_t expected = scan_with_predicate_and(tc.rows, tc.n_rows, row_stride, ref_pred);
        int64_t actual   = spec(tc.rows, tc.n_rows);
        if (expected != actual)
            throw clangRuntimeSpecializer::ClangRuntimeSpecializerChangesBehaviorError(
                "validate_multi_predicate_abstract_specialized: result mismatch");
    }

    CRS_LOG(Info, "validate_multi_predicate_abstract_specialized passed");
}
