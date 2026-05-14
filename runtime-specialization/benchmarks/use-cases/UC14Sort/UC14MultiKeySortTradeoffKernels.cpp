#include "UC14Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <cstdint>
#include <stdexcept>
#include <vector>

// MultiKeyComparator: two-level comparator with key1 asc, key2 desc.
struct MultiKeyComparator {
    int key1_offset;
    int key2_offset;

    int compare(const void* a, const void* b) const {
        double a1, b1;
        __builtin_memcpy(&a1, (const uint8_t*)a + key1_offset, sizeof(double));
        __builtin_memcpy(&b1, (const uint8_t*)b + key1_offset, sizeof(double));
        if (a1 != b1) return (a1 > b1) - (a1 < b1);
        double a2, b2;
        __builtin_memcpy(&a2, (const uint8_t*)a + key2_offset, sizeof(double));
        __builtin_memcpy(&b2, (const uint8_t*)b + key2_offset, sizeof(double));
        // key2 descending: reverse comparison
        return (b2 > a2) - (b2 < a2);
    }
};

// MultiKeySorter: encapsulates element_size and MultiKeyComparator.
struct MultiKeySorter {
    int element_size;
    MultiKeyComparator comparator;

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
                    for (int64_t j = i; j > lo && comparator.compare(elem(j-1), elem(j)) > 0; --j)
                        swap_elems(elem(j-1), elem(j));
                }
                continue;
            }

            int64_t mid = lo + (hi - lo) / 2;
            if (comparator.compare(elem(lo), elem(mid)) > 0) swap_elems(elem(lo), elem(mid));
            if (comparator.compare(elem(lo), elem(hi)) > 0) swap_elems(elem(lo), elem(hi));
            if (comparator.compare(elem(mid), elem(hi)) > 0) swap_elems(elem(mid), elem(hi));
            swap_elems(elem(mid), elem(hi-1));
            uint8_t* pivot_ptr = elem(hi-1);

            int64_t left = lo, right = hi - 1;
            while (true) {
                while (comparator.compare(elem(++left), pivot_ptr) < 0) {}
                while (comparator.compare(elem(--right), pivot_ptr) > 0) {}
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

MultiKeySortTradeoffSpecialized create_multi_key_sort_tradeoff_specialized(int element_size,
                                                                            int key1_offset,
                                                                            int key2_offset) {
    MultiKeySorter sorter{element_size, MultiKeyComparator{key1_offset, key2_offset}};
    auto lam = [sorter](void* data, int64_t n_elements) {
        sorter.sort(data, n_elements);
    };
    return clangRuntimeSpecializer::specializeLambda<void>(lam);
}

void validate_multi_key_sort_tradeoff_specialized(int element_size,
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

    MultiKeySorter ref_sorter{element_size, MultiKeyComparator{key1_offset, key2_offset}};
    ref_sorter.sort(ref_data.data(), N);

    auto spec = create_multi_key_sort_tradeoff_specialized(element_size, key1_offset, key2_offset);
    spec(spec_data.data(), N);

    for (int i = 0; i < N; ++i) {
        if (ref_data[i].key1 != spec_data[i].key1 || ref_data[i].key2 != spec_data[i].key2)
            throw std::runtime_error("validate_multi_key_sort_tradeoff_specialized: mismatch");
    }
    for (int i = 1; i < N; ++i) {
        if (ref_data[i-1].key1 > ref_data[i].key1)
            throw std::runtime_error("validate_multi_key_sort_tradeoff_specialized: key1 not sorted asc");
        if (ref_data[i-1].key1 == ref_data[i].key1 && ref_data[i-1].key2 < ref_data[i].key2)
            throw std::runtime_error("validate_multi_key_sort_tradeoff_specialized: key2 not sorted desc on tie");
    }
}
