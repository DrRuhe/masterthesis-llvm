#pragma once
#include "ClangRuntimeSpecializer.h"
#include <cstdint>

// Comparator: non-static, non-inline external function (required for JIT inlining)
int int64_asc_cmp(const void* a, const void* b);

// generic_sort: comparator and element_size are specialization constants.
// Implements iterative median-of-three quicksort.
void generic_sort(void* data, int64_t n_elements, int element_size,
                  int (*comparator)(const void*, const void*));

using SortSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, void*, int64_t>;

SortSpecialized create_sort_specialized(int (*comparator)(const void*, const void*),
                                         int element_size);
void validate_sort_specialized(int (*comparator)(const void*, const void*),
                                int element_size);
