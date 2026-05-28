#pragma once
#include "ClangRuntimeSpecializer.h"
#include <cstdint>

// ---------------------------------------------------------------------------
// Type aliases — one per variant × level (void return, (void*, int64_t) args)
// ---------------------------------------------------------------------------

// generic_sort (low / original)
using SortSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, void*, int64_t>;

// struct_sort
using StructSortLowSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, void*, int64_t>;
using StructSortTradeoffSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, void*, int64_t>;
using StructSortAbstractSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, void*, int64_t>;

// multi_key_sort
using MultiKeySortLowSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, void*, int64_t>;
using MultiKeySortTradeoffSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, void*, int64_t>;
using MultiKeySortAbstractSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, void*, int64_t>;

// generic_sort tradeoff + abstract
using GenericSortTradeoffSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, void*, int64_t>;
using GenericSortAbstractSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, void*, int64_t>;

// ---------------------------------------------------------------------------
// Kernel declarations
// ---------------------------------------------------------------------------

// Comparator: non-static, non-inline external function (required for JIT inlining)
int int64_asc_cmp(const void* a, const void* b);

// generic_sort: comparator and element_size are specialization constants.
// Implements iterative median-of-three quicksort.
void generic_sort(void* data, int64_t n_elements, int element_size,
                  int (*comparator)(const void*, const void*));

// struct_sort: sorts fixed-size structs by a double field at field_offset.
void struct_sort(void* data, int64_t n_elements, int element_size, int field_offset);

// struct_sort unspecialized baselines (tradeoff: FieldSorter; abstract: FieldComparatorAdapter).
void struct_sort_tradeoff_unspecialized(void* data, int64_t n_elements,
                                        int element_size, int field_offset);
void struct_sort_abstract_unspecialized(void* data, int64_t n_elements,
                                        int element_size, int field_offset);

// multi_key_sort: sorts by two double fields (key1 asc, key2 desc on tie).
void multi_key_sort(void* data, int64_t n_elements, int element_size,
                    int key1_offset, int key2_offset);

// multi_key_sort unspecialized baselines (tradeoff: MultiKeySorter; abstract: CompositeComparator).
void multi_key_sort_tradeoff_unspecialized(void* data, int64_t n_elements,
                                           int element_size, int key1_offset, int key2_offset);
void multi_key_sort_abstract_unspecialized(void* data, int64_t n_elements,
                                           int element_size, int key1_offset, int key2_offset);

// generic_sort unspecialized baselines (tradeoff: GenericSorterT; abstract: Sorter+vtable).
void generic_sort_tradeoff_unspecialized(void* data, int64_t n_elements, int element_size);
void generic_sort_abstract_unspecialized(void* data, int64_t n_elements, int element_size);

// ---------------------------------------------------------------------------
// Factory + validation — generic_sort (low)
// ---------------------------------------------------------------------------

SortSpecialized create_sort_specialized(int (*comparator)(const void*, const void*),
                                         int element_size);
void validate_sort_specialized(int (*comparator)(const void*, const void*),
                                int element_size);

// ---------------------------------------------------------------------------
// Factory + validation — struct_sort
// ---------------------------------------------------------------------------

StructSortLowSpecialized create_struct_sort_low_specialized(int element_size, int field_offset);
void validate_struct_sort_low_specialized(int element_size, int field_offset);

StructSortTradeoffSpecialized create_struct_sort_tradeoff_specialized(int element_size,
                                                                       int field_offset);
void validate_struct_sort_tradeoff_specialized(int element_size, int field_offset);

StructSortAbstractSpecialized create_struct_sort_abstract_specialized(int element_size,
                                                                       int field_offset);
void validate_struct_sort_abstract_specialized(int element_size, int field_offset);

// ---------------------------------------------------------------------------
// Factory + validation — multi_key_sort
// ---------------------------------------------------------------------------

MultiKeySortLowSpecialized create_multi_key_sort_low_specialized(int element_size,
                                                                  int key1_offset,
                                                                  int key2_offset);
void validate_multi_key_sort_low_specialized(int element_size, int key1_offset, int key2_offset);

MultiKeySortTradeoffSpecialized create_multi_key_sort_tradeoff_specialized(int element_size,
                                                                            int key1_offset,
                                                                            int key2_offset);
void validate_multi_key_sort_tradeoff_specialized(int element_size,
                                                   int key1_offset, int key2_offset);

MultiKeySortAbstractSpecialized create_multi_key_sort_abstract_specialized(int element_size,
                                                                            int key1_offset,
                                                                            int key2_offset);
void validate_multi_key_sort_abstract_specialized(int element_size,
                                                   int key1_offset, int key2_offset);

// ---------------------------------------------------------------------------
// Factory + validation — generic_sort tradeoff + abstract
// ---------------------------------------------------------------------------

GenericSortTradeoffSpecialized create_generic_sort_tradeoff_specialized(int element_size);
void validate_generic_sort_tradeoff_specialized(int element_size);

GenericSortAbstractSpecialized create_generic_sort_abstract_specialized(int element_size);
void validate_generic_sort_abstract_specialized(int element_size);
