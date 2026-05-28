#include "UC14Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <cstdint>
#include <stdexcept>
#include <vector>

// Local definition of int64_asc_cmp (keeps this TU self-contained for the JIT blob).
// CRITICAL: non-static, non-inline so the JIT AlwaysInlinerPass can inline it.
int int64_asc_cmp_tradeoff(const void* a, const void* b) {
    int64_t va, vb;
    std::memcpy(&va, a, sizeof(int64_t));
    std::memcpy(&vb, b, sizeof(int64_t));
    return (va > vb) - (va < vb);
}

// GenericSorterT: encapsulates element_size as a specialization constant.
// The comparator function pointer is also baked in as a constant.
struct GenericSorterT {
    int element_size;

    void sort(void* data, int64_t n_elements,
              int (*comparator)(const void*, const void*)) const {
        if (n_elements <= 1) return;

        uint8_t* base = (uint8_t*)data;
        int esz = element_size;

        auto elem = [base, esz](int64_t i) -> uint8_t* {
            return base + i * esz;
        };
        auto swap_elems = [esz](uint8_t* a, uint8_t* b) {
            uint8_t tmp[16];
            std::memcpy(tmp, a, esz);
            std::memcpy(a, b, esz);
            std::memcpy(b, tmp, esz);
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
                    for (int64_t j = i; j > lo && comparator(elem(j-1), elem(j)) > 0; --j)
                        swap_elems(elem(j-1), elem(j));
                }
                continue;
            }

            int64_t mid = lo + (hi - lo) / 2;
            if (comparator(elem(lo), elem(mid)) > 0) swap_elems(elem(lo), elem(mid));
            if (comparator(elem(lo), elem(hi)) > 0) swap_elems(elem(lo), elem(hi));
            if (comparator(elem(mid), elem(hi)) > 0) swap_elems(elem(mid), elem(hi));
            swap_elems(elem(mid), elem(hi-1));
            uint8_t* pivot_ptr = elem(hi-1);

            int64_t left = lo, right = hi - 1;
            while (true) {
                while (comparator(elem(++left), pivot_ptr) < 0) {}
                while (comparator(elem(--right), pivot_ptr) > 0) {}
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

void generic_sort_tradeoff_unspecialized(void* data, int64_t n_elements, int element_size) {
    GenericSorterT sorter{element_size};
    sorter.sort(data, n_elements, &int64_asc_cmp_tradeoff);
}

GenericSortTradeoffSpecialized create_generic_sort_tradeoff_specialized(int element_size) {
    GenericSorterT sorter{element_size};
    int (*cmp)(const void*, const void*) = &int64_asc_cmp_tradeoff;
    auto lam = [sorter, cmp](void* data, int64_t n_elements) {
        sorter.sort(data, n_elements, cmp);
    };
    return clangRuntimeSpecializer::specializeLambda<void>(lam);
}

void validate_generic_sort_tradeoff_specialized(int element_size) {
    const int N = 1000;
    std::vector<int64_t> ref_data(N), spec_data(N);
    for (int i = 0; i < N; ++i) ref_data[i] = spec_data[i] = (int64_t)((N - i) * 7 % 1000);

    GenericSorterT ref_sorter{element_size};
    ref_sorter.sort(ref_data.data(), N, &int64_asc_cmp_tradeoff);

    auto spec = create_generic_sort_tradeoff_specialized(element_size);
    spec(spec_data.data(), N);

    for (int i = 0; i < N; ++i) {
        if (ref_data[i] != spec_data[i])
            throw std::runtime_error("validate_generic_sort_tradeoff_specialized: mismatch");
    }
}
