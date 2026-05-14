#include "UC14Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <cstdint>
#include <stdexcept>
#include <vector>

// multi_key_sort: sorts fixed-size structs by two double fields.
// key1 ascending, key2 descending on tie.
// element_size, key1_offset, key2_offset are specialization constants.
void multi_key_sort(void* data, int64_t n_elements, int element_size,
                    int key1_offset, int key2_offset) {
    if (n_elements <= 1) return;

    uint8_t* base = (uint8_t*)data;

    auto elem = [base, element_size](int64_t i) -> uint8_t* {
        return base + i * element_size;
    };
    auto swap_elems = [element_size](uint8_t* a, uint8_t* b) {
        uint8_t tmp[256];
        __builtin_memcpy(tmp, a, element_size);
        __builtin_memcpy(a, b, element_size);
        __builtin_memcpy(b, tmp, element_size);
    };
    auto cmp = [key1_offset, key2_offset](const uint8_t* a, const uint8_t* b) -> int {
        double a1, b1;
        __builtin_memcpy(&a1, a + key1_offset, sizeof(double));
        __builtin_memcpy(&b1, b + key1_offset, sizeof(double));
        if (a1 != b1) return (a1 > b1) - (a1 < b1);
        double a2, b2;
        __builtin_memcpy(&a2, a + key2_offset, sizeof(double));
        __builtin_memcpy(&b2, b + key2_offset, sizeof(double));
        // key2 descending: reverse comparison
        return (b2 > a2) - (b2 < a2);
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
                for (int64_t j = i; j > lo && cmp(elem(j-1), elem(j)) > 0; --j)
                    swap_elems(elem(j-1), elem(j));
            }
            continue;
        }

        int64_t mid = lo + (hi - lo) / 2;
        if (cmp(elem(lo), elem(mid)) > 0) swap_elems(elem(lo), elem(mid));
        if (cmp(elem(lo), elem(hi)) > 0) swap_elems(elem(lo), elem(hi));
        if (cmp(elem(mid), elem(hi)) > 0) swap_elems(elem(mid), elem(hi));
        swap_elems(elem(mid), elem(hi-1));
        uint8_t* pivot_ptr = elem(hi-1);

        int64_t left = lo, right = hi - 1;
        while (true) {
            while (cmp(elem(++left), pivot_ptr) < 0) {}
            while (cmp(elem(--right), pivot_ptr) > 0) {}
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

MultiKeySortLowSpecialized create_multi_key_sort_low_specialized(int element_size,
                                                                  int key1_offset,
                                                                  int key2_offset) {
    auto lam = [element_size, key1_offset, key2_offset](void* data, int64_t n_elements) {
        multi_key_sort(data, n_elements, element_size, key1_offset, key2_offset);
    };
    return clangRuntimeSpecializer::specializeLambda<void>(lam);
}

void validate_multi_key_sort_low_specialized(int element_size, int key1_offset, int key2_offset) {
    struct TestRecord {
        double key1;
        double key2;
    };
    const int N = 200;
    std::vector<TestRecord> ref_data(N), spec_data(N);
    for (int i = 0; i < N; ++i) {
        // Create records where key1 has repeated values to test key2 ordering
        ref_data[i].key1 = (double)(i % 10);
        ref_data[i].key2 = (double)(i % 7);
        spec_data[i] = ref_data[i];
    }

    multi_key_sort(ref_data.data(), N, element_size, key1_offset, key2_offset);

    auto spec = create_multi_key_sort_low_specialized(element_size, key1_offset, key2_offset);
    spec(spec_data.data(), N);

    for (int i = 0; i < N; ++i) {
        if (ref_data[i].key1 != spec_data[i].key1 || ref_data[i].key2 != spec_data[i].key2)
            throw std::runtime_error("validate_multi_key_sort_low_specialized: mismatch");
    }
    // Verify ordering: key1 asc, then key2 desc on tie
    for (int i = 1; i < N; ++i) {
        if (ref_data[i-1].key1 > ref_data[i].key1)
            throw std::runtime_error("validate_multi_key_sort_low_specialized: key1 not sorted asc");
        if (ref_data[i-1].key1 == ref_data[i].key1 && ref_data[i-1].key2 < ref_data[i].key2)
            throw std::runtime_error("validate_multi_key_sort_low_specialized: key2 not sorted desc on tie");
    }
}
