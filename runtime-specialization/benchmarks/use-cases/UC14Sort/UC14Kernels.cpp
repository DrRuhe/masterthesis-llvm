#include "UC14Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <stdexcept>
#include <vector>

// CRITICAL: int64_asc_cmp must be non-static and non-inline so AlwaysInlinerPass
// can inline it from the same JIT module clone into the sort body.
int int64_asc_cmp(const void* a, const void* b) {
    int64_t va, vb;
    std::memcpy(&va, a, sizeof(int64_t));
    std::memcpy(&vb, b, sizeof(int64_t));
    return (va > vb) - (va < vb);
}

// generic_sort: iterative median-of-three quicksort using an explicit stack.
// comparator and element_size are specialization constants baked by the JIT.
void generic_sort(void* data, int64_t n_elements, int element_size,
                  int (*comparator)(const void*, const void*)) {
    if (n_elements <= 1) return;

    uint8_t* base = (uint8_t*)data;

    auto elem = [base, element_size](int64_t i) -> uint8_t* {
        return base + i * element_size;
    };
    auto swap_elems = [element_size](uint8_t* a, uint8_t* b) {
        // tmp[16] works since element_size = sizeof(int64_t) = 8 <= 16
        uint8_t tmp[16];
        std::memcpy(tmp, a, element_size);
        std::memcpy(a, b, element_size);
        std::memcpy(b, tmp, element_size);
    };

    // Iterative quicksort using explicit stack (avoids stack overflow on 1M elements)
    struct Range { int64_t lo, hi; };
    Range stack[64];
    int top = 0;
    stack[top++] = {0, n_elements - 1};

    while (top > 0) {
        auto [lo, hi] = stack[--top];
        if (lo >= hi) continue;

        // Insertion sort for small ranges
        if (hi - lo < 16) {
            for (int64_t i = lo + 1; i <= hi; ++i) {
                for (int64_t j = i; j > lo && comparator(elem(j-1), elem(j)) > 0; --j)
                    swap_elems(elem(j-1), elem(j));
            }
            continue;
        }

        // Median-of-three pivot selection
        int64_t mid = lo + (hi - lo) / 2;
        if (comparator(elem(lo), elem(mid)) > 0) swap_elems(elem(lo), elem(mid));
        if (comparator(elem(lo), elem(hi)) > 0) swap_elems(elem(lo), elem(hi));
        if (comparator(elem(mid), elem(hi)) > 0) swap_elems(elem(mid), elem(hi));
        // elem(mid) is now the median; place pivot at hi-1
        swap_elems(elem(mid), elem(hi-1));
        uint8_t* pivot_ptr = elem(hi-1);

        // Partition [lo+1 .. hi-2]
        int64_t left = lo, right = hi - 1;
        while (true) {
            while (comparator(elem(++left), pivot_ptr) < 0) {}
            while (comparator(elem(--right), pivot_ptr) > 0) {}
            if (left >= right) break;
            swap_elems(elem(left), elem(right));
        }
        // Place pivot at left
        swap_elems(elem(left), elem(hi-1));

        // Push larger partition first so smaller is processed first (bounded stack depth)
        if (left - 1 - lo > hi - (left + 1)) {
            if (lo < left - 1)  stack[top++] = {lo, left - 1};
            if (left + 1 < hi)  stack[top++] = {left + 1, hi};
        } else {
            if (left + 1 < hi)  stack[top++] = {left + 1, hi};
            if (lo < left - 1)  stack[top++] = {lo, left - 1};
        }
    }
}

// Lambda factory — both this and generic_sort live in the same TU so the JIT
// module clone contains both, enabling full inlining of the comparator call.
SortSpecialized create_sort_specialized(int (*comparator)(const void*, const void*),
                                         int element_size) {
    auto lam = [comparator, element_size](void* data, int64_t n_elements) {
        generic_sort(data, n_elements, element_size, comparator);
    };
    return clangRuntimeSpecializer::specializeLambda<void>(lam);
}

void validate_sort_specialized(int (*comparator)(const void*, const void*),
                                int element_size) {
    const int N = 1000;
    std::vector<int64_t> ref_data(N), spec_data(N);
    for (int i = 0; i < N; ++i) ref_data[i] = spec_data[i] = (int64_t)((N - i) * 7 % 1000);

    generic_sort(ref_data.data(), N, sizeof(int64_t), comparator);

    auto spec = create_sort_specialized(comparator, element_size);
    spec(spec_data.data(), N);

    for (int i = 0; i < N; ++i) {
        if (ref_data[i] != spec_data[i])
            throw std::runtime_error("validate_sort_specialized: mismatch");
    }
}
