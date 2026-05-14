# Data Model: Use-Case Benchmark Abstraction Variants

## Core Entities

### 1. KernelTU

A translation unit containing one kernel function at one abstraction level for one variant of one use-case, plus its lambda factory and validation function.

**Identity**: `(uc_id, variant_name, level)` — unique combination.

**File path pattern**: `benchmarks/use-cases/UC<N><UCName>/<VariantPascalCase><Level>Kernels.cpp`

**Contents**:
- Kernel function (the hot loop/computation)
- `create_<variant>_<level>_specialized(const-args...) -> <Variant><Level>Specialized`
- `validate_<variant>_<level>_specialized(const-args...)` — asserts equivalence across levels

**Levels**: `Low`, `Tradeoff`, `Abstract`

**Invariant**: No `static` globals (FR-006). No benchmark infrastructure headers (`ClangRuntimeSpecializerBenchmark.h`) — see TU separation rule.

---

### 2. SpecializedLambdaType

A `using` alias defined in `UC<N>Kernels.h` for each (variant, level) pair.

**Pattern**:
```cpp
using <VariantPascalCase><Level>Specialized =
    clangRuntimeSpecializer::SpecializedLambda<RetType, RuntimeArg1, RuntimeArg2, ...>;
```

**Example** (UC1, `count_matching_rows`, tradeoff level):
```cpp
using CountMatchingRowsTradeoffSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const uint8_t*, int64_t>;
```

The runtime arguments are the same across all three levels for the same variant (same function signature). The specialization constants (captured by the lambda factory) differ.

---

### 3. BenchmarkName

The string passed to `->Name(...)` in the BENCHMARK macro.

**Format**: `BM_g:<group>;n:<variant>;a:<level>;s:<size>;t:<type>;`

**Field values**:
| Field | Key | Example values |
|-------|-----|----------------|
| Benchmark group | `g:` | `uc1_sql`, `uc2_conv`, `uc7_dfa`, `uc8_ivm`, `uc12_groupby`, `uc14_sort` |
| Variant name | `n:` | `count_matching_rows`, `multi_predicate`, `column_scan`, … |
| Abstraction level | `a:` | `low`, `tradeoff`, `abstract` |
| Size tier | `s:` | `SMALL`, `MEDIUM`, `LARGE`, `EXTRALARGE` |
| Benchmark phase | `t:` | `unspecialized`, `jit_overhead`, `specialized_exec` |

**DuckDB column**: `kv_a` — added automatically by `ensure_columns()` on first import; no schema change needed.

**Comparison query pattern** (abstraction speedup comparison):
```sql
SELECT kv_a, kernel, AVG(t_spec_ns / t_unspec_ns) AS speedup
FROM v_ratios JOIN benchmarks USING (run_id, name)
WHERE "group" = 'uc1_sql'
GROUP BY kv_a, kernel
ORDER BY kv_a, kernel;
```

---

### 4. UC Benchmark Directory

One directory per use-case containing all kernel TUs and one benchmark registration file.

**Layout** (example for UC1):
```
UC1SqlPredicate/
├── UC1CountMatchingRowsLowKernels.cpp      # variant 1 × low   (renamed from UC1Kernels.cpp)
├── UC1CountMatchingRowsTradeoffKernels.cpp # variant 1 × tradeoff
├── UC1CountMatchingRowsAbstractKernels.cpp # variant 1 × abstract
├── UC1MultiPredicateLowKernels.cpp         # variant 2 × low
├── UC1MultiPredicateTradeoffKernels.cpp    # variant 2 × tradeoff
├── UC1MultiPredicateAbstractKernels.cpp    # variant 2 × abstract
├── UC1ColumnScanLowKernels.cpp             # variant 3 × low
├── UC1ColumnScanTradeoffKernels.cpp        # variant 3 × tradeoff
├── UC1ColumnScanAbstractKernels.cpp        # variant 3 × abstract
├── UC1Kernels.h                            # all declarations + SpecializedLambda typedefs
├── UC1Benchmark.cpp                        # all benchmark registrations (108 BENCHMARK macros)
└── CMakeLists.txt                          # foreach over 9 kernel TUs
```

**Identical layout** for UC2, UC7, UC8, UC12, UC14 with their respective variant names.

---

### 5. CMake Export Variables (per UC)

Variables exported from each UC's `CMakeLists.txt` to parent scope:

| Variable | Type | Description |
|----------|------|-------------|
| `UC<N>_KERNELS_SRCS` | CMake list | Absolute paths to all 9 kernel `.cpp` files |
| `UC<N>_KERNELS_OBJS` | CMake list | Absolute paths to all 9 compiled kernel `.o` files |
| `UC<N>_BENCH_OBJ` | Path | Single compiled benchmark `.o` |
| `UC<N>_SRC_DIR` | Path | UC source directory (for `-I` includes) |
| `UC<N>_BENCH_SRC` | Path | Absolute path to `UC<N>Benchmark.cpp` |

The old `UC<N>_KERNELS_SRC` / `UC<N>_KERNELS_OBJ` (singular) variables are replaced by their plural forms. The AllBenchmarks loop in `benchmarks/CMakeLists.txt` iterates over the list with a nested `foreach`.

---

## Variant Mapping Table

Complete mapping of UC × variant × level to kernel TU filename:

| UC | Group | Variant | Low TU | Tradeoff TU | Abstract TU |
|----|-------|---------|--------|-------------|-------------|
| UC1 | `uc1_sql` | `count_matching_rows` | `UC1CountMatchingRowsLowKernels.cpp` | `…TradeoffKernels.cpp` | `…AbstractKernels.cpp` |
| UC1 | `uc1_sql` | `multi_predicate` | `UC1MultiPredicateLowKernels.cpp` | … | … |
| UC1 | `uc1_sql` | `column_scan` | `UC1ColumnScanLowKernels.cpp` | … | … |
| UC2 | `uc2_conv` | `separable_gaussian` | `UC2SeparableGaussianLowKernels.cpp` | … | … |
| UC2 | `uc2_conv` | `box_filter` | `UC2BoxFilterLowKernels.cpp` | … | … |
| UC2 | `uc2_conv` | `edge_detection` | `UC2EdgeDetectionLowKernels.cpp` | … | … |
| UC7 | `uc7_dfa` | `email_match` | `UC7EmailMatchLowKernels.cpp` | … | … |
| UC7 | `uc7_dfa` | `url_match` | `UC7UrlMatchLowKernels.cpp` | … | … |
| UC7 | `uc7_dfa` | `multi_pattern_match` | `UC7MultiPatternMatchLowKernels.cpp` | … | … |
| UC8 | `uc8_ivm` | `apply_row_delta` | `UC8ApplyRowDeltaLowKernels.cpp` | … | … |
| UC8 | `uc8_ivm` | `multi_agg_delta` | `UC8MultiAggDeltaLowKernels.cpp` | … | … |
| UC8 | `uc8_ivm` | `batch_delta` | `UC8BatchDeltaLowKernels.cpp` | … | … |
| UC12 | `uc12_groupby` | `grouped_sum` | `UC12GroupedSumLowKernels.cpp` | … | … |
| UC12 | `uc12_groupby` | `grouped_count` | `UC12GroupedCountLowKernels.cpp` | … | … |
| UC12 | `uc12_groupby` | `grouped_minmax` | `UC12GroupedMinMaxLowKernels.cpp` | … | … |
| UC14 | `uc14_sort` | `generic_sort` | `UC14GenericSortLowKernels.cpp` | … | … |
| UC14 | `uc14_sort` | `struct_sort` | `UC14StructSortLowKernels.cpp` | … | … |
| UC14 | `uc14_sort` | `multi_key_sort` | `UC14MultiKeySortLowKernels.cpp` | … | … |

**Total**: 18 variants × 3 levels = 54 kernel TUs.

---

## State Transitions

Abstraction levels are independent variants of the same algorithm — there is no "state transition" between them at runtime. The comparison is purely at the measurement level: query `v_ratios` filtered to the same `(run_id, kernel, group)` but different `kv_a` values.

---

## Validation Contract

Each kernel TU MUST expose a `validate_<variant>_<level>_specialized` function (called from `UC<N>Benchmark.cpp` main block, under `#ifndef ALL_BENCHMARKS_BUILD`). The validation function:
1. Creates a small fixed test dataset.
2. Calls `create_<variant>_<level>_specialized(constants...)` to get a `SpecializedLambda`.
3. Calls the lambda and the unspecialized kernel with identical inputs.
4. Throws `ClangRuntimeSpecializerChangesBehaviorError` on any mismatch.

Cross-level numerical equivalence (SC-004) is verified by calling the validation functions for all three levels with the same constants in `main()`.
