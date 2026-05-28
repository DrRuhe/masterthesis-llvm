#include "UC14Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <cstdint>
#include <stdexcept>
#include <vector>

// Abstract comparator interface — virtual dispatch is the specialization target.
struct Comparator {
    virtual int compare(const void* a, const void* b) const = 0;

};

// Concrete subclass defined in the same TU so the JIT can devirtualize.
struct Int64AscComparator : Comparator {
    int compare(const void* a, const void* b) const override {
        int64_t va, vb;
        std::memcpy(&va, a, sizeof(int64_t));
        std::memcpy(&vb, b, sizeof(int64_t));
        return (va > vb) - (va < vb);
    }
};

// Sorter embeds Int64AscComparator BY VALUE (concrete type directly, not pointer).
struct Sorter {
    Int64AscComparator cmp_impl;

    void sort(void* data, int64_t n_elements, int element_size) const {
        if (n_elements <= 1) return;

        uint8_t* base = (uint8_t*)data;
        int esz = element_size;
        const Comparator& comparator = cmp_impl;

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

void generic_sort_abstract_unspecialized(void* data, int64_t n_elements, int element_size) {
    Sorter sorter{};
    sorter.sort(data, n_elements, element_size);
}

GenericSortAbstractSpecialized create_generic_sort_abstract_specialized(int element_size) {
    // Reconstruct object inside the lambda so its this-pointer is a local variable
    // (not a stale factory-frame stack address).
    auto lam = [element_size](void* data, int64_t n_elements) {
        Sorter sorter{};
        sorter.sort(data, n_elements, element_size);
    };
    return clangRuntimeSpecializer::specializeLambda<void>(lam);
}

void validate_generic_sort_abstract_specialized(int element_size) {
    const int N = 1000;
    std::vector<int64_t> ref_data(N), spec_data(N);
    for (int i = 0; i < N; ++i) ref_data[i] = spec_data[i] = (int64_t)((N - i) * 7 % 1000);

    Sorter ref_sorter{};
    ref_sorter.sort(ref_data.data(), N, element_size);

    auto spec = create_generic_sort_abstract_specialized(element_size);
    spec(spec_data.data(), N);

    for (int i = 0; i < N; ++i) {
        if (ref_data[i] != spec_data[i])
            throw std::runtime_error("validate_generic_sort_abstract_specialized: mismatch");
    }
}
