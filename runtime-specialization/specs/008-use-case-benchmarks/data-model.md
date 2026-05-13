# Phase 1: Data Model — Use-Case Benchmark Suite

## Entities

### SpecializedLambda (per use case)

Each factory function returns a `SpecializedLambda<R, VariableArgs...>` — a RAII,
move-only callable owning its JITDylib.

| Use case | Return type alias | Explicit arg types |
|----------|------------------|-------------------|
| UC1 | `SQLSpecialized` | `(const uint8_t*, int64_t)` |
| UC2 | `ConvSpecialized` | `(const float*, float*)` |
| UC7 | `DFASpecialized` | `(const char*, int64_t)` |
| UC8 | `IVMSpecialized` | `(const uint8_t*, double*)` |
| UC12 | `GroupBySpecialized` | `(const uint8_t*, int64_t, double*)` |
| UC14 | `SortSpecialized` | `(void*, int64_t)` |

### Globals (non-static, in kernel TU)

| Use case | Global name | Type | Purpose |
|----------|------------|------|---------|
| UC2 | `g_kernel_coeffs` | `float[25]` | 5×5 Gaussian conv kernel |
| UC7 | `g_dfa_table` | `int[N_STATES*128]` | DFA transition table |

All other fixed parameters are passed to the factory function at call time and captured
by value in the lambda.

### Benchmark Dataset (in benchmark TU, file-scope)

| Use case | Variable | Type | Size |
|----------|---------|------|------|
| UC1 | `g_rows` | `std::vector<uint8_t>` | 10M × ROW_STRIDE bytes |
| UC2 | `g_src`, `g_dst` | `std::vector<float>` | 1920×1080 each |
| UC7 | `g_corpus` | `std::vector<char>` | ≥ 50 MB |
| UC8 | `g_deltas`, `g_buckets` | `std::vector<uint8_t>`, `std::vector<double>` | 10M events, 1K buckets |
| UC12 | `g_rows12`, `g_buckets12` | `std::vector<uint8_t>`, `std::vector<double>` | 10M rows, 1K buckets |
| UC14 | `g_data` | `std::vector<int64_t>` | 1M elements |

## DuckDB Schema (existing, no changes)

New benchmark rows land in the existing `benchmarks` table. The `kernel` field is derived
from the `n:` tag in the benchmark name. Views `v_optim_best_per_kernel` and
`v_ratios` (from spec 007) work without modification.

## Factory Function Signatures

```cpp
// UC1
using SQLSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const uint8_t*, int64_t>;
SQLSpecialized create_sql_specialized(int row_stride, int col_offset, double threshold);

// UC2
using ConvSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const float*, float*>;
ConvSpecialized create_conv_specialized();  // uses g_kernel_coeffs + constexpr ksize/dims

// UC7
using DFASpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const char*, int64_t>;
DFASpecialized create_dfa_specialized();  // uses g_dfa_table + constexpr DFA params

// UC8
using IVMSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, double*>;
IVMSpecialized create_ivm_specialized(int group_col_offset, int value_col_offset,
                                      int row_stride, int n_buckets);

// UC12
using GroupBySpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, const uint8_t*, int64_t, double*>;
GroupBySpecialized create_groupby_specialized(int row_stride, int key_offset,
                                             int value_offset, int n_buckets);

// UC14
using SortSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<void, void*, int64_t>;
SortSpecialized create_sort_specialized(int (*comparator)(const void*, const void*),
                                        int element_size);
```
