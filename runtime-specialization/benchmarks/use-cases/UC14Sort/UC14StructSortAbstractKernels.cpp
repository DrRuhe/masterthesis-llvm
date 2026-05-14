#include "UC14Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <cstdint>
#include <stdexcept>
#include <vector>

// Abstract extractor interface.
struct FieldExtractor {
    virtual double extract(const void* record) const = 0;
    virtual ~FieldExtractor() = default;
};

// Concrete subclass: extracts double at a byte offset via __builtin_memcpy.
// Defined in the same TU so JIT can devirtualize.
struct ByteOffsetExtractor : FieldExtractor {
    int field_offset;

    explicit ByteOffsetExtractor(int offset) : field_offset(offset) {}

    double extract(const void* record) const override {
        double v;
        __builtin_memcpy(&v, (const uint8_t*)record + field_offset, sizeof(double));
        return v;
    }
};

// FieldComparatorAdapter holds ByteOffsetExtractor BY VALUE + element_size.
struct FieldComparatorAdapter {
    ByteOffsetExtractor extractor;
    int element_size;

    int compare(const void* a, const void* b) const {
        double va = extractor.extract(a);
        double vb = extractor.extract(b);
        return (va > vb) - (va < vb);
    }

    void sort_struct(void* data, int64_t n_elements) const {
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

StructSortAbstractSpecialized create_struct_sort_abstract_specialized(int element_size,
                                                                       int field_offset) {
    FieldComparatorAdapter adapter{ByteOffsetExtractor{field_offset}, element_size};
    auto lam = [adapter](void* data, int64_t n_elements) {
        adapter.sort_struct(data, n_elements);
    };
    return clangRuntimeSpecializer::specializeLambda<void>(lam);
}

void validate_struct_sort_abstract_specialized(int element_size, int field_offset) {
    struct TestRecord {
        double key;
        double value;
    };
    const int N = 200;
    std::vector<TestRecord> ref_data(N), spec_data(N);
    for (int i = 0; i < N; ++i) {
        double v = (double)((N - i) * 7 % N);
        ref_data[i].key = v;
        ref_data[i].value = (double)i;
        spec_data[i] = ref_data[i];
    }

    FieldComparatorAdapter ref_adapter{ByteOffsetExtractor{field_offset}, element_size};
    ref_adapter.sort_struct(ref_data.data(), N);

    auto spec = create_struct_sort_abstract_specialized(element_size, field_offset);
    spec(spec_data.data(), N);

    for (int i = 0; i < N; ++i) {
        if (ref_data[i].key != spec_data[i].key)
            throw std::runtime_error("validate_struct_sort_abstract_specialized: key mismatch");
    }
    for (int i = 1; i < N; ++i) {
        if (ref_data[i-1].key > ref_data[i].key)
            throw std::runtime_error("validate_struct_sort_abstract_specialized: not sorted");
    }
}
