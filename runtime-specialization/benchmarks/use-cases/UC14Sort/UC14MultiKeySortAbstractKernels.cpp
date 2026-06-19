#include "UC14Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <cstdint>
#include <stdexcept>
#include <vector>

// Reuse the ByteOffsetExtractor from the StructSort abstract TU — but since
// each TU is self-contained in the JIT, we define it locally here.
struct ByteOffsetExtractor2 {
    int field_offset;

    double extract(const void* record) const {
        double v;
        __builtin_memcpy(&v, (const uint8_t*)record + field_offset, sizeof(double));
        return v;
    }
};

// CompositeComparator: two ByteOffsetExtractor2 BY VALUE + two ascending flags.
struct CompositeComparator {
    ByteOffsetExtractor2 extractor1;
    ByteOffsetExtractor2 extractor2;
    bool ascending1;  // true = ascending for key1
    bool ascending2;  // true = ascending for key2
    int element_size;

    int compare(const void* a, const void* b) const {
        double va1 = extractor1.extract(a);
        double vb1 = extractor1.extract(b);
        if (va1 != vb1) {
            int cmp = (va1 > vb1) - (va1 < vb1);
            return ascending1 ? cmp : -cmp;
        }
        double va2 = extractor2.extract(a);
        double vb2 = extractor2.extract(b);
        int cmp = (va2 > vb2) - (va2 < vb2);
        return ascending2 ? cmp : -cmp;
    }

    void sort(void* data, int64_t n_elements) const {
        if (n_elements <= 1) return;

        uint8_t* base = (uint8_t*)data;
        int esz = element_size;

        auto elem = [base, esz](int64_t i) -> uint8_t* {
            return base + i * esz;
        };
        auto swap_elems = [esz](uint8_t* a, uint8_t* b) {
            uint8_t tmp[256];
            __builtin_memcpy(tmp, a, esz);
            __builtin_memcpy(a, b, esz);
            __builtin_memcpy(b, tmp, esz);
        };

        struct Range { int64_t lo, hi; };
        Range stack[64];
        int top = 0;
        stack[top++] = {0, n_elements - 1};

        while (top > 0) {
            auto [lo, hi] = stack[--top];
            if (lo >= hi) continue;

            if (hi - lo < 16) {
                for (int64_t i = lo + 1; i <= hi; ++i) {
                    for (int64_t j = i; j > lo && compare(elem(j-1), elem(j)) > 0; --j)
                        swap_elems(elem(j-1), elem(j));
                }
                continue;
            }

            int64_t mid = lo + (hi - lo) / 2;
            if (compare(elem(lo), elem(mid)) > 0) swap_elems(elem(lo), elem(mid));
            if (compare(elem(lo), elem(hi)) > 0) swap_elems(elem(lo), elem(hi));
            if (compare(elem(mid), elem(hi)) > 0) swap_elems(elem(mid), elem(hi));
            swap_elems(elem(mid), elem(hi-1));
            uint8_t* pivot_ptr = elem(hi-1);

            int64_t left = lo, right = hi - 1;
            while (true) {
                while (compare(elem(++left), pivot_ptr) < 0) {}
                while (compare(elem(--right), pivot_ptr) > 0) {}
                if (left >= right) break;
                swap_elems(elem(left), elem(right));
            }
            swap_elems(elem(left), elem(hi-1));

            if (left - 1 - lo > hi - (left + 1)) {
                if (lo < left - 1)  stack[top++] = {lo, left - 1};
                if (left + 1 < hi)  stack[top++] = {left + 1, hi};
            } else {
                if (left + 1 < hi)  stack[top++] = {left + 1, hi};
                if (lo < left - 1)  stack[top++] = {lo, left - 1};
            }
        }
    }
};

void multi_key_sort_abstract_unspecialized(void* data, int64_t n_elements,
                                           int element_size, int key1_offset, int key2_offset) {
    // key1 ascending, key2 descending (matching multi_key_sort convention)
    CompositeComparator cc{
        ByteOffsetExtractor2{key1_offset},
        ByteOffsetExtractor2{key2_offset},
        /*ascending1=*/true,
        /*ascending2=*/false,
        element_size
    };
    cc.sort(data, n_elements);
}

MultiKeySortAbstractSpecialized create_multi_key_sort_abstract_specialized(int element_size,
                                                                            int key1_offset,
                                                                            int key2_offset) {
    auto lam = [=](void* data, int64_t n_elements) {
        CompositeComparator cc{
            ByteOffsetExtractor2{key1_offset},
            ByteOffsetExtractor2{key2_offset},
            /*ascending1=*/true,
            /*ascending2=*/false,
            element_size
        };
        cc.sort(data, n_elements);
    };
    return clangRuntimeSpecializer::specializeLambda<void>(lam);
}

void validate_multi_key_sort_abstract_specialized(int element_size,
                                                   int key1_offset, int key2_offset) {
    struct TestRecord {
        double key1;
        double key2;
    };
    const int N = 200;
    std::vector<TestRecord> ref_data(N), spec_data(N);
    for (int i = 0; i < N; ++i) {
        ref_data[i].key1 = (double)(i % 10);
        ref_data[i].key2 = (double)(i % 7);
        spec_data[i] = ref_data[i];
    }

    CompositeComparator ref_cc{
        ByteOffsetExtractor2{key1_offset},
        ByteOffsetExtractor2{key2_offset},
        true, false, element_size
    };
    ref_cc.sort(ref_data.data(), N);

    auto spec = create_multi_key_sort_abstract_specialized(element_size, key1_offset, key2_offset);
    spec(spec_data.data(), N);

    for (int i = 0; i < N; ++i) {
        if (ref_data[i].key1 != spec_data[i].key1 || ref_data[i].key2 != spec_data[i].key2)
            throw std::runtime_error("validate_multi_key_sort_abstract_specialized: mismatch");
    }
    for (int i = 1; i < N; ++i) {
        if (ref_data[i-1].key1 > ref_data[i].key1)
            throw std::runtime_error("validate_multi_key_sort_abstract_specialized: key1 not sorted asc");
        if (ref_data[i-1].key1 == ref_data[i].key1 && ref_data[i-1].key2 < ref_data[i].key2)
            throw std::runtime_error("validate_multi_key_sort_abstract_specialized: key2 not sorted desc on tie");
    }
}
