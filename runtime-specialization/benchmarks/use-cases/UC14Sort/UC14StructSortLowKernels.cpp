#include "UC14Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <cstdint>
#include <stdexcept>
#include <vector>

// struct_sort: sorts fixed-size structs by a double field at field_offset.
// element_size and field_offset are specialization constants.
void struct_sort(void* data, int64_t n_elements, int element_size, int field_offset) {
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
    auto cmp = [field_offset](const uint8_t* a, const uint8_t* b) -> int {
        double va, vb;
        __builtin_memcpy(&va, a + field_offset, sizeof(double));
        __builtin_memcpy(&vb, b + field_offset, sizeof(double));
        return (va > vb) - (va < vb);
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

StructSortLowSpecialized create_struct_sort_low_specialized(int element_size, int field_offset) {
    auto lam = [element_size, field_offset](void* data, int64_t n_elements) {
        struct_sort(data, n_elements, element_size, field_offset);
    };
    return clangRuntimeSpecializer::specializeLambda<void>(lam);
}

void validate_struct_sort_low_specialized(int element_size, int field_offset) {
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

    struct_sort(ref_data.data(), N, element_size, field_offset);

    auto spec = create_struct_sort_low_specialized(element_size, field_offset);
    spec(spec_data.data(), N);

    for (int i = 0; i < N; ++i) {
        if (ref_data[i].key != spec_data[i].key)
            throw std::runtime_error("validate_struct_sort_low_specialized: key mismatch");
    }
    // Verify ascending order
    for (int i = 1; i < N; ++i) {
        if (ref_data[i-1].key > ref_data[i].key)
            throw std::runtime_error("validate_struct_sort_low_specialized: not sorted");
    }
}
