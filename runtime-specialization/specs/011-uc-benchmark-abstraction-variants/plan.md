# Implementation Plan: Use-Case Benchmark Abstraction Variants

**Branch**: `011-uc-benchmark-abstraction-variants` | **Date**: 2026-05-14 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `specs/011-uc-benchmark-abstraction-variants/spec.md`

## Summary

Add tradeoff and abstract abstraction-level variants alongside new variant scenarios to the 6 existing use-case benchmarks (UC1, UC2, UC7, UC8, UC12, UC14), producing 54 kernel TUs total (6 UCs × 3 variants × 3 levels). Each level represents a distinct C++ coding style: low (raw loops), tradeoff (templates/policy classes), abstract (virtual dispatch). All variants share the existing lambda factory pattern and benchmark naming convention, with a new `a:<level>` KV tag added to benchmark names. The `record_benchmark.py` dynamic-column mechanism handles `kv_a` without schema changes.

## Technical Context

**Language/Version**: C++17 (existing codebase standard)
**Primary Dependencies**: LLVM/Clang, Google Benchmark, DuckDB (record_benchmark.py)
**Storage**: `benchmarks/benchmarks.duckdb` — `kv_a` column added dynamically on first import
**Testing**: Google Benchmark (`ninja check-smoke-runtime-specializer` for regression guard; no new lit tests needed for benchmarks)
**Target Platform**: Linux x86_64 (NixOS, Nix flake)
**Project Type**: Research benchmark suite (library + benchmarks)
**Performance Goals**: Abstract-tier speedup ≥ low-tier speedup for ≥4/6 UCs (SC-002); JIT overhead within 10% of existing for low tier (SC-005)
**Constraints**: No static globals (FR-006); no benchmark infrastructure in kernel TUs; virtual dispatch not devirtualized at compile time (FR-008)
**Scale/Scope**: 54 kernel TUs, 6 unified benchmark files, ~108 BENCHMARK macros per UC (27 combinations × 4 sizes), 2 CMakeLists.txt files modified

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Principle | Status | Notes |
|-----------|--------|-------|
| I. Correctness & Safety | ✅ PASS | `validate_*` functions verify equivalence across levels (SC-004); abstract-tier captures concrete object → specialization semantically correct |
| II. LLVM Coding Standards | ✅ PASS | New kernel code uses `__builtin_memcpy`, no `std::cerr`/`printf`; `CRS_LOG` in validation; no raw owning pointers |
| III. Test-First Validation | ✅ PASS | Benchmark binaries include `validate_*` calls in `main()`; smoke suite remains unchanged; no new lit tests required for benchmark-only feature |
| IV. Performance Measurement | ✅ PASS | All benchmarks use existing `BENCHMARK()->Name(...)` pattern; recorded in DuckDB; SC-005 regression guard defined |
| V. Minimal Public API | ✅ PASS | No public API changes; only new kernel TUs and benchmark registrations |
| Specialization Scope | ✅ PASS | All benchmarks are single-threaded; specialized arguments not shared across threads |
| Backwards Compatibility | ✅ PASS | Old `kv_n` values (`predicate`, `gaussian5x5`, etc.) preserved in existing DuckDB run rows; no data deleted |
| DuckDB Schema | ✅ PASS | `kv_a` column added dynamically by `ensure_columns()` — no manual schema change needed |

**Post-Phase-1 re-check**: No violations found. Abstract-tier vtable globals covered by existing spec 004 WeakODR invariant; no new IRDumpingPass changes required.

## Project Structure

### Documentation (this feature)

```text
specs/011-uc-benchmark-abstraction-variants/
├── plan.md              # This file
├── research.md          # Phase 0 output (complete)
├── data-model.md        # Phase 1 output (complete)
└── tasks.md             # Phase 2 output (/speckit-tasks command)
```

### Source Code Layout

```text
benchmarks/use-cases/
├── CMakeLists.txt                          # updated: foreach list re-export
├── UC1SqlPredicate/
│   ├── UC1CountMatchingRowsLowKernels.cpp  # renamed from UC1Kernels.cpp
│   ├── UC1CountMatchingRowsTradeoffKernels.cpp  # NEW
│   ├── UC1CountMatchingRowsAbstractKernels.cpp  # NEW
│   ├── UC1MultiPredicateLowKernels.cpp     # NEW
│   ├── UC1MultiPredicateTradeoffKernels.cpp     # NEW
│   ├── UC1MultiPredicateAbstractKernels.cpp     # NEW
│   ├── UC1ColumnScanLowKernels.cpp         # NEW
│   ├── UC1ColumnScanTradeoffKernels.cpp         # NEW
│   ├── UC1ColumnScanAbstractKernels.cpp         # NEW
│   ├── UC1Kernels.h                        # updated: all 18 declarations + typedefs
│   ├── UC1Benchmark.cpp                    # updated: 108 BENCHMARK macros, 9 validate calls
│   └── CMakeLists.txt                      # rewritten: foreach over KERNEL_SRCS list
├── UC2Convolution/     # identical structure, 9 kernel TUs
├── UC7DfaRegex/        # identical structure, 9 kernel TUs
├── UC8IVM/             # identical structure, 9 kernel TUs
├── UC12GroupBy/        # identical structure, 9 kernel TUs
└── UC14Sort/           # identical structure, 9 kernel TUs

benchmarks/CMakeLists.txt                  # updated: nested foreach for kernel TU lists
```

**Structure Decision**: Flat files within each UC directory. Extending the existing single-directory, single-CMakeLists pattern by multiplying kernel source files and exporting a list variable. No new directory levels added — the file naming convention (`<Variant><Level>Kernels.cpp`) is sufficient to distinguish TUs.

---

## Implementation Phases

### Phase 1: CMake Infrastructure + Naming Re-tag

*Prerequisite for all kernel work. Establishes the build system contract that all new TUs follow.*

#### 1.1 — Per-UC CMakeLists.txt template

Replace the single `add_custom_command` for the kernel TU with a foreach loop. The pattern for each UC's `CMakeLists.txt`:

```cmake
# List of all kernel sources for this UC (one per variant × level)
set(UC1_KERNEL_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/UC1CountMatchingRowsLowKernels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UC1CountMatchingRowsTradeoffKernels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UC1CountMatchingRowsAbstractKernels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UC1MultiPredicateLowKernels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UC1MultiPredicateTradeoffKernels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UC1MultiPredicateAbstractKernels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UC1ColumnScanLowKernels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UC1ColumnScanTradeoffKernels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UC1ColumnScanAbstractKernels.cpp
)

set(_KERN_OBJS "")
foreach(KERN_SRC IN LISTS UC1_KERNEL_SRCS)
    get_filename_component(KERN_STEM "${KERN_SRC}" NAME_WE)
    set(_KERN_OBJ "${CMAKE_CURRENT_BINARY_DIR}/${KERN_STEM}.o")
    add_custom_command(OUTPUT ${_KERN_OBJ}
        COMMAND ${CLANG_EXE} -x c++ -g -O0 ${CXX_FLAGS_LIST}
                -I${CMAKE_CURRENT_SOURCE_DIR}
                -I${CMAKE_CURRENT_SOURCE_DIR}/../../../runtime/ClangRuntimeSpecializer
                -I${LLVM_MAIN_INCLUDE_DIR}
                -I${LLVM_BINARY_DIR}/include
                -fpass-plugin=${PLUGIN_LIB}
                -fexceptions -frtti
                -c ${KERN_SRC} -o ${_KERN_OBJ}
        DEPENDS ${KERN_SRC} ${CMAKE_CURRENT_SOURCE_DIR}/UC1Kernels.h
                LLVMRuntimeSpecializationComptimePlugin clang
        COMMENT "Compiling ${KERN_STEM} with plugin")
    list(APPEND _KERN_OBJS ${_KERN_OBJ})
endforeach()

# Export to parent (use-cases/) scope
set(UC1_KERNELS_OBJS "${_KERN_OBJS}"             PARENT_SCOPE)
set(UC1_KERNELS_SRCS "${UC1_KERNEL_SRCS}"         PARENT_SCOPE)
set(UC1_SRC_DIR      "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)
set(UC1_BENCH_SRC    "${CMAKE_CURRENT_SOURCE_DIR}/UC1Benchmark.cpp" PARENT_SCOPE)
# ... bench .o compilation unchanged ...
```

Note: The old singular `UC<N>_KERNELS_SRC` and `UC<N>_KERNELS_OBJ` variables are replaced. The `use-cases/CMakeLists.txt` and `benchmarks/CMakeLists.txt` must be updated in the same commit.

#### 1.2 — Update `use-cases/CMakeLists.txt`

Change the re-export foreach to handle the list variables:
```cmake
foreach(UC IN ITEMS UC1 UC2 UC7 UC8 UC12 UC14)
    set(${UC}_KERNELS_OBJS "${${UC}_KERNELS_OBJS}" PARENT_SCOPE)
    set(${UC}_KERNELS_SRCS "${${UC}_KERNELS_SRCS}" PARENT_SCOPE)
    set(${UC}_SRC_DIR      "${${UC}_SRC_DIR}"      PARENT_SCOPE)
    set(${UC}_BENCH_SRC    "${${UC}_BENCH_SRC}"    PARENT_SCOPE)
    set(${UC}_BENCH_OBJ    "${${UC}_BENCH_OBJ}"    PARENT_SCOPE)
endforeach()
```

#### 1.3 — Update `benchmarks/CMakeLists.txt` AllBenchmarks loop

Replace the existing foreach that references `${UC}_KERNELS_SRC` (singular) with a nested loop:
```cmake
set(_ALL_UC_KERNEL_OBJS "")
foreach(UC IN ITEMS UC1 UC2 UC7 UC8 UC12 UC14)
    foreach(KERN_SRC IN LISTS ${UC}_KERNELS_SRCS)
        get_filename_component(KERN_STEM "${KERN_SRC}" NAME_WE)
        set(_KERN_OBJ "${CMAKE_CURRENT_BINARY_DIR}/AllBenchmarks_${KERN_STEM}.o")
        add_custom_command(OUTPUT ${_KERN_OBJ}
            COMMAND ${CLANG_EXE} -x c++ -g -O0 ${CXX_FLAGS_LIST}
                    -DALL_BENCHMARKS_BUILD=1
                    -I${${UC}_SRC_DIR}
                    -I${CMAKE_CURRENT_SOURCE_DIR}/../runtime/ClangRuntimeSpecializer
                    -I${LLVM_MAIN_INCLUDE_DIR}
                    -I${LLVM_BINARY_DIR}/include
                    -fpass-plugin=${PLUGIN_LIB}
                    -fexceptions -frtti
                    -c ${KERN_SRC} -o ${_KERN_OBJ}
            DEPENDS ${KERN_SRC} LLVMRuntimeSpecializationComptimePlugin clang
            COMMENT "Compiling ${KERN_STEM} for AllBenchmarks")
        list(APPEND _ALL_UC_KERNEL_OBJS ${_KERN_OBJ})
    endforeach()
    # ... benchmark TU unchanged ...
endforeach()
```

#### 1.4 — FR-010: Re-tag existing benchmark names

In each existing `UC<N>Benchmark.cpp`, update the BENCHMARK macro name strings:
- Add `a:low;` after `n:<new_variant_name>;`
- Rename `kv_n` from old name to new variant 1 name

| File | Old `n:` | New `n:` | `a:` added |
|------|----------|----------|------------|
| `UC1Benchmark.cpp` | `predicate` | `count_matching_rows` | `a:low` |
| `UC2Benchmark.cpp` | `gaussian5x5` | `separable_gaussian` | `a:low` |
| `UC7Benchmark.cpp` | `email` | `email_match` | `a:low` |
| `UC8Benchmark.cpp` | `ivm_sum` | `apply_row_delta` | `a:low` |
| `UC12Benchmark.cpp` | `groupby_sum` | `grouped_sum` | `a:low` |
| `UC14Benchmark.cpp` | `sort_int64` | `generic_sort` | `a:low` |

Example change for UC1:
```
// Before:
->Name("BM_g:uc1_sql;n:predicate;s:SMALL;t:unspecialized;")
// After:
->Name("BM_g:uc1_sql;n:count_matching_rows;a:low;s:SMALL;t:unspecialized;")
```

---

### Phase 2: UC1 Reference Implementation

*UC1 is implemented first as the reference. Its pattern is replicated verbatim for UC2–UC14.*

#### 2.1 — Rename existing kernel TU

Rename `UC1Kernels.cpp` → `UC1CountMatchingRowsLowKernels.cpp`.
- No functional changes to the kernel or factory function.
- Function name `count_matching_rows` and factory `create_sql_specialized` stay the same (referenced from `UC1Benchmark.cpp`).
- Function names for the new variants follow `create_<variant>_<level>_specialized(...)`.

#### 2.2 — `UC1CountMatchingRowsTradeoffKernels.cpp`

Tradeoff implementation of variant 1 (`count_matching_rows`):
```cpp
// RowScanner<ColOffset, RowStride> — template parameters are specialization constants
// captured by lambda at factory call time.
template <int ColOffset, int RowStride>
struct RowScanner {
    int64_t scan(const uint8_t* rows, int64_t n, double threshold) const {
        int64_t count = 0;
        for (int64_t i = 0; i < n; ++i) {
            double val;
            __builtin_memcpy(&val, rows + i * RowStride + ColOffset, sizeof(double));
            if (val > threshold) ++count;
        }
        return count;
    }
};

CountMatchingRowsTradeoffSpecialized
create_count_matching_rows_tradeoff_specialized(int col_offset, int row_stride, double threshold) {
    // Template instantiation with runtime-as-specialization-constant values.
    // The JIT specializer propagates col_offset/row_stride as IR constants
    // through the captured lambda.
    auto lam = [col_offset, row_stride, threshold](const uint8_t* rows, int64_t n) -> int64_t {
        // Template parameters fold into constants after specialization.
        // col_offset/row_stride are captured as constants → IPSCCP propagates them.
        RowScanner<0, 0> scanner;  // dummy instantiation; runtime values override via lambda capture
        (void)scanner;
        int64_t count = 0;
        for (int64_t i = 0; i < n; ++i) {
            double val;
            __builtin_memcpy(&val, rows + i * row_stride + col_offset, sizeof(double));
            if (val > threshold) ++count;
        }
        return count;
    };
    return clangRuntimeSpecializer::specializeLambda<int64_t>(lam);
}
```

**Note on tradeoff templates**: C++ template parameters are compile-time constants; they cannot directly encode JIT specialization constants (which are runtime values). The tradeoff tier demonstrates templates as the *idiomatic C++ pattern*, but the actual specialization benefit comes from the lambda capturing `col_offset`/`row_stride` as JIT constants. The template struct is instantiated in the TU to show the pattern; the JIT propagates the lambda captures. This distinction is documented in research.md Decision 6.

#### 2.3 — `UC1CountMatchingRowsAbstractKernels.cpp`

Abstract implementation:
```cpp
struct Predicate {
    virtual bool test(const uint8_t* row) const = 0;
    virtual ~Predicate() = default;
};

struct ThresholdPredicate : Predicate {
    int col_offset, row_stride;
    double threshold;
    ThresholdPredicate(int co, int rs, double th)
        : col_offset(co), row_stride(rs), threshold(th) {}
    bool test(const uint8_t* row) const override {
        double val;
        __builtin_memcpy(&val, row + col_offset, sizeof(double));
        return val > threshold;
    }
};

static int64_t scan_with_predicate(const uint8_t* rows, int64_t n,
                                    int row_stride, const Predicate& pred) {
    int64_t count = 0;
    for (int64_t i = 0; i < n; ++i)
        if (pred.test(rows + i * row_stride)) ++count;
    return count;
}

CountMatchingRowsAbstractSpecialized
create_count_matching_rows_abstract_specialized(int col_offset, int row_stride, double threshold) {
    ThresholdPredicate pred{col_offset, row_stride, threshold};
    // Capture concrete ThresholdPredicate by value.
    // JIT: vtable ptr in pred is a constant → DevirtualizeConstantVtableCallsPass devirtualizes test().
    auto lam = [pred, row_stride](const uint8_t* rows, int64_t n) -> int64_t {
        return scan_with_predicate(rows, n, row_stride, pred);
    };
    return clangRuntimeSpecializer::specializeLambda<int64_t>(lam);
}
```

**Key**: `ThresholdPredicate` captured by **value** in the lambda. The vtable pointer in the captured object is a constant in the JIT IR — enabling `DevirtualizeConstantVtableCallsPass` to inline `test()`.

#### 2.4 — Variants 2 (`multi_predicate`) and 3 (`column_scan`) × 3 levels

Apply the same low/tradeoff/abstract pattern for the remaining two variants.

**`multi_predicate` low**: Two-field extraction, four constants (`col_offset_a`, `col_offset_b`, `threshold_a`, `threshold_b`, `row_stride`).
**`multi_predicate` tradeoff**: Two `ColumnPredicate<Offset>` instances composed via AND in the lambda.
**`multi_predicate` abstract**: `AndPredicate` holding two `ThresholdPredicate` objects (by value).

**`column_scan` low**: Loop writes matching indices to `int32_t* out`.
**`column_scan` tradeoff**: `ProjectingScanner` struct with scan method; captured by value.
**`column_scan` abstract**: `ResultCollector` interface; `BufferCollector` captures output vector; concrete type captured by value.

#### 2.5 — Update `UC1Kernels.h`

Expand with all 18 (9 × 2) declarations and `using` typedefs:
```cpp
// ─── variant 1: count_matching_rows ───────────────────────────────────────
using CountMatchingRowsLowSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const uint8_t*, int64_t>;
using CountMatchingRowsTradeoffSpecialized = CountMatchingRowsLowSpecialized;
using CountMatchingRowsAbstractSpecialized = CountMatchingRowsLowSpecialized;
// All three levels share the same runtime signature.

CountMatchingRowsLowSpecialized      create_count_matching_rows_low_specialized(...);
CountMatchingRowsTradeoffSpecialized create_count_matching_rows_tradeoff_specialized(...);
CountMatchingRowsAbstractSpecialized create_count_matching_rows_abstract_specialized(...);

void validate_count_matching_rows_low_specialized(...);
void validate_count_matching_rows_tradeoff_specialized(...);
void validate_count_matching_rows_abstract_specialized(...);

// ─── variant 2: multi_predicate ───────────────────────────────────────────
// ... analogous ...

// ─── variant 3: column_scan ───────────────────────────────────────────────
// ... analogous ...
```

Note: `SQLSpecialized` (the existing name) is retained as an alias for backward compat in `UC1Benchmark.cpp` if needed, or updated to use the new names.

#### 2.6 — Update `UC1Benchmark.cpp`

Add benchmark functions for all 9 variant × level combinations. Pattern:
```cpp
static void BM_UC1_count_matching_rows_unspecialized(benchmark::State& state) { ... }
static void BM_UC1_count_matching_rows_low_jit(benchmark::State& state) { ... }
static void BM_UC1_count_matching_rows_low_exec(benchmark::State& state) { ... }
static void BM_UC1_count_matching_rows_tradeoff_jit(benchmark::State& state) { ... }
// ... etc for all 9 variants × 3 phases (minus unspecialized duplicates per variant)
```

Benchmark name format: `BM_g:uc1_sql;n:count_matching_rows;a:low;s:SMALL;t:unspecialized;`

The `#ifdef ALL_BENCHMARKS_BUILD` guard remains: sizes are smaller for the AllBenchmarks binary.

---

### Phase 3: Remaining UCs (UC2, UC7, UC8, UC12, UC14)

Apply the Phase 2 pattern verbatim for each remaining UC. Each UC produces 9 kernel TUs and an updated benchmark file.

**UC2 — Convolution**:
- Variant 1 (`separable_gaussian`): Rename existing `UC2Kernels.cpp` → `UC2SeparableGaussianLowKernels.cpp`. Tradeoff: `SeparableFilter<KSize>` struct. Abstract: `FilterKernel` base + `GaussianKernel` captured by value.
- Variant 2 (`box_filter`): Low: radius loop. Tradeoff: `BoxFilter` struct. Abstract: `SpatialFilter` interface + `BoxFilter` captured by value.
- Variant 3 (`edge_detection`): Low: two 3×3 passes. Tradeoff: `SobelFilter` struct with static constexpr coefficients. Abstract: `GradientDetector` interface + `SobelDetector` captured by value.

**UC7 — DFA Regex**:
- Variant 1 (`email_match`): Rename existing `UC7Kernels.cpp`. Tradeoff: `DFAMatcher<NStates, NChars>` template. Abstract: `Matcher` interface + `DFAMatcher` captured by value.
- Variant 2 (`url_match`): Different DFA table (URL grammar). Same level pattern.
- Variant 3 (`multi_pattern_match`): Combined DFA table, scans entire buffer, returns total match count (no early exit per FR-011).

**UC8 — IVM**:
- Variant 1 (`apply_row_delta`): Rename existing. Tradeoff: `IVMUpdater<NBuckets, GroupOffset, ValueOffset, RowStride>`. Abstract: `Aggregator` + `SumAggregator` captured by value.
- Variant 2 (`multi_agg_delta`): Sum + count both updated per row.
- Variant 3 (`batch_delta`): Outer loop over `n_rows`.

**UC12 — GroupBy**:
- Variant 1 (`grouped_sum`): Rename existing. Tradeoff: `GroupedAggregator<RowStride, KeyOffset, ValueOffset>`. Abstract: `AggregationOperator` + `SumOperator` captured by value.
- Variant 2 (`grouped_count`): Count only, no value column.
- Variant 3 (`grouped_minmax`): Dual output arrays for min and max.

**UC14 — Sort**:
- Variant 1 (`generic_sort`): Rename existing. Tradeoff: `GenericSorter<ElementSize>`. Abstract: `Comparator` interface + `Int64AscComparator` captured by value.
- Variant 2 (`struct_sort`): `double` field at runtime offset. Tradeoff: `FieldComparator<FieldOffset>`. Abstract: `FieldExtractor` + `ByteOffsetExtractor` captured by value.
- Variant 3 (`multi_key_sort`): Two-field lexicographic comparison. Tradeoff: `MultiKeyComparator<Key1Offset, Key2Offset>`. Abstract: `CompositeComparator` with two `FieldExtractor`s captured by value.

---

### Phase 4: Validation and Success Criteria

#### 4.1 — SC-001: Verify 54 benchmark scenarios

After building `AllBenchmarks`, run:
```bash
./AllBenchmarks --benchmark_list_tests | grep -oP 'a:\K(low|tradeoff|abstract)' | sort | uniq -c
```
Expected: 18 low, 18 tradeoff, 18 abstract (per UC × variant combination, counting distinct `n:` + `a:` pairs, not size variants).

#### 4.2 — SC-004: Numerical equivalence

Validation functions are called in each UC benchmark's `main()` block (under `#ifndef ALL_BENCHMARKS_BUILD`). Building and running each individual UC binary verifies equivalence:
```bash
ninja UC1SqlPredicate && ./UC1SqlPredicate --benchmark_filter="NOTHING"
# validate_* functions run in main() before benchmarks; throw on mismatch
```

#### 4.3 — SC-003: DuckDB import

```bash
python record_benchmark.py ./AllBenchmarks --benchmark_filter="a:low|a:tradeoff|a:abstract"
# Verify kv_a column created and populated:
duckdb benchmarks.duckdb "SELECT kv_a, COUNT(*) FROM benchmarks GROUP BY kv_a"
```

#### 4.4 — SC-005: JIT overhead regression check

After recording, compare:
```sql
SELECT kv_a, kernel, AVG(t_jit_ns) AS avg_jit_ns
FROM v_ratios JOIN benchmarks USING (run_id, name)
WHERE kv_a = 'low'
GROUP BY kv_a, kernel;
```
Compare against pre-feature baseline; flag any >10% increase.

### Phase 5: Pareto Visualization Update

*Addresses FR-012: per-kernel Pareto analysis with abstraction-level coloring.*

#### 5.1 — `benchmarks/reporting/plot_pareto_configs.py`

Changes already applied (see git history):

- `fetch_rows()`: queries `kv_a` from `v_ratios` when the column exists (guarded via `information_schema.columns`); falls back to `NULL` for pre-011 data.
- `_flag_default()`: groups by `(group, kernel, kv_a)` so the default star is placed once per abstraction level per kernel.
- `render_kernel()`: new per-kernel render function. When `kv_a` is present, colors dots and per-level frontier lines by level (low=blue `#1f77b4`, tradeoff=orange `#ff7f0e`, abstract=green `#2ca02c`) and draws the global frontier as a dashed gray `#555555` line on top.
- `main()` with `--per-group`: iterates over `(group, kernel)` pairs — not just group — emitting one PNG + CSV per pair.
- CSV: `is_pareto_optimal` is now computed per-kernel; `kv_a` column added (empty string for pre-011 data).

#### 5.2 — Re-run existing pareto reports

After the benchmark data is recorded, regenerate reports with `--per-group` to replace the previous methodologically wrong per-group (cross-kernel) plots:

```bash
python benchmarks/reporting/plot_pareto_configs.py \
  --study-name uc_optim_iter2_20260521 \
  --per-group \
  --output-dir benchmarks/reports/260521-17-32-optimize-pipeline/per-kernel/
```

Expected output: one PNG + CSV per `(group, kernel)` — e.g., `uc7_dfa_email_match`, `uc7_dfa_url_match`, `uc7_dfa_multi_pattern_match` — instead of the single `uc7_dfa` plot.

---

## Key Constraints (summary)

1. **No static globals** in any kernel TU (FR-006, spec 008 rule).
2. **No benchmark headers** (`ClangRuntimeSpecializerBenchmark.h`, `<benchmark/benchmark.h>`) in kernel TUs — only in `UC<N>Benchmark.cpp`.
3. **Abstract-tier concrete objects captured by value** in lambda — not via `std::unique_ptr` or `new` — to expose vtable pointer as a JIT constant.
4. **Kernel TUs compiled at -O0** (CMake rule: prevents DAE from hiding `specializeLambda` call).
5. **Each (variant × level) is a separate .cpp TU** (FR-003) — no `#include`-based combination.
6. **Benchmark name format**: `BM_g:<group>;n:<variant>;a:<level>;s:<size>;t:<type>;` — `a:` placed after `n:`, before `s:`.
7. **Scalar capture + reconstruct pattern (abstract AND tradeoff tier)**: `serializeArgumentToIR` for class types embeds `&lambda_closure` (a stack address on the factory call frame). After the factory returns, this address is stale → segfault on virtual dispatch. Fix: capture only scalar fields (ints, stable global pointers); reconstruct the polymorphic object inside the lambda body so `this` is a local variable, never stale. This applies to ALL tiers — even tradeoff structs with a `const T* table` pointer field must use scalar capture (`[tbl = g_table]`) + reconstruct inside.
8. **Zero-capture lambda crash**: `specializeLambdaImpl` segfaults when the closure struct is zero-sized. If all needed constants are global-scope, add a dummy scalar capture (e.g., `[tbl = g_global_ptr]`) to ensure the closure is non-empty.
9. **ODR violation between tradeoff and abstract TUs**: structs with the same name but different definitions across TUs cause ODR UB. Specifically, tradeoff DFAMatcher (non-polymorphic, 24 bytes) and abstract DFAMatcher (polymorphic with vtable, 32 bytes) with the same name → linker picks one implementation → crash. Fix: rename abstract-tier structs to `AbstractFoo` (e.g., `AbstractDFAMatcher`) in UC7 abstract TUs.
10. **CMake KERNEL_SRCS must use absolute paths**: the `${UC_KERNELS_SRCS}` variable exported to `PARENT_SCOPE` must contain absolute paths (using `${CMAKE_CURRENT_SOURCE_DIR}/...`). Relative paths fail when the `AllBenchmarks` target in `benchmarks/CMakeLists.txt` tries to use them as source dependencies in `add_custom_command`.

## Complexity Tracking

> No constitution violations. Table omitted per template instructions.

---

## Artifacts Generated

- `specs/011-uc-benchmark-abstraction-variants/research.md` — 7 design decisions with rationales
- `specs/011-uc-benchmark-abstraction-variants/data-model.md` — entity definitions, variant mapping table, CMake export contract
- `specs/011-uc-benchmark-abstraction-variants/plan.md` — this file
