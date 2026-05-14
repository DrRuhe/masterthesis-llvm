# Research: Use-Case Benchmark Abstraction Variants

## Decision 1 — `a:` tag and DuckDB schema (FR-004, SC-003)

**Decision**: Add `a:<level>` tag to benchmark names (e.g., `BM_g:uc1_sql;n:count_matching_rows;a:low;s:SMALL;t:unspecialized;`). No manual schema modification needed.

**Rationale**: `record_benchmark.py`'s `ensure_columns()` (lines 212–233) already calls `_parse_bm_name()` for every benchmark row and runs `ALTER TABLE benchmarks ADD COLUMN "kv_a" VARCHAR` the first time it encounters a name containing `a:`. This is the existing dynamic-column mechanism used for all KV tags. SC-003 ("without schema modifications") is satisfied because the infrastructure already handles unknown KV keys.

**Alternatives considered**: Hard-coding `kv_a` in `_SCHEMA_BENCHMARKS` — rejected because SC-003 explicitly prohibits schema changes and the dynamic mechanism is sufficient.

---

## Decision 2 — CMake pattern for N kernel TUs per UC

**Decision**: Replace the single `UC<N>_KERNELS_SRC` export variable with `UC<N>_KERNELS_SRCS` (a CMake list). Each UC's `CMakeLists.txt` compiles each kernel source in a foreach loop, appends the `.o` to a list, and exports both the source list and object list. `use-cases/CMakeLists.txt` re-exports the list variables. `benchmarks/CMakeLists.txt`'s AllBenchmarks loop iterates over the list with a nested foreach.

**Rationale**: The existing `add_custom_command` pattern scales directly: one command per `.cpp`. A foreach loop keeps the per-UC `CMakeLists.txt` short even with 9 kernel TUs. Exporting lists rather than single paths is a backward-compatible CMake pattern.

**Alternatives considered**:
- Subdirectory per level (e.g., `UC1SqlPredicate/low/`, `UC1SqlPredicate/tradeoff/`) — rejected because CMake `add_subdirectory` nesting would add two extra scoping layers and the existing flat include paths would break.
- One CMakeLists per (variant × level) — rejected as over-engineering; 9 kernel TUs per UC is manageable in one CMakeLists.

---

## Decision 3 — File naming convention for kernel TUs

**Decision**: Each kernel TU is named `UC<N><VariantPascalCase><Level>Kernels.cpp`.

Examples:
- `UC1CountMatchingRowsLowKernels.cpp`
- `UC1CountMatchingRowsTradeoffKernels.cpp`
- `UC1CountMatchingRowsAbstractKernels.cpp`
- `UC1MultiPredicateLowKernels.cpp`
- …

The existing `UC1Kernels.cpp` is renamed to `UC1CountMatchingRowsLowKernels.cpp` (the existing low-level kernel is variant 1 of UC1).

**Rationale**: The naming makes the (variant, level) pair unambiguous from the filename alone. Pascal-case variant names avoid ambiguity with multi-word names. The `Kernels` suffix preserves the existing convention.

**Alternatives considered**:
- Keeping `UC1Kernels.cpp` as a dispatcher that `#include`s sub-files — rejected because it would put all variants in a single JIT blob, violating FR-003.
- Snake-case filenames — rejected for consistency with existing CMakeLists and target names.

---

## Decision 4 — Abstract-tier TU separation and devirtualization

**Decision**: Virtual base class definition AND concrete subclass definition both live in the same kernel TU as the lambda factory. The benchmark registration TU only includes the header (declarations).

**Rationale**: The JIT blob = one TU. If the concrete subclass is defined in the kernel TU, the IRDumpingPass embeds its vtable and method bodies. When the lambda factory captures a concrete object (by value or stack pointer), the JIT specializer can devirtualize the virtual call via `DevirtualizeConstantVtableCallsPass` — the vtable pointer in the captured object is a constant known at specialization time. This is consistent with the existing `DBOperatorsBenchmark` abstract-tier pattern referenced in the spec.

The vtable global for the concrete subclass must survive early GlobalDCE. Spec 004's IRDumpingPass already marks vtable globals `WeakODR` at compile time — this invariant covers all new abstract-tier types automatically.

**Key requirement**: The lambda factory must capture a concrete object (not a pointer obtained from `new`, which would be a heap allocation unknown to the specializer). Capture by value or from a stack-allocated local is required.

**Alternatives considered**:
- Concrete subclass in separate TU — rejected because the vtable and method bodies would be absent from the JIT blob, preventing devirtualization.
- Using `std::function` for abstract tier — rejected because `std::function` uses type erasure and does not expose a vtable for `DevirtualizeConstantVtableCallsPass`.

---

## Decision 5 — FR-010 re-tagging strategy

**Decision**: Update each existing UC benchmark macro to (a) rename `kv_n` from the old name to the new variant 1 name and (b) add `a:low` after `n:`.

Mapping of old → new `kv_n`:
| UC | Old `kv_n` | New `kv_n` |
|----|-----------|-----------|
| UC1 | `predicate` | `count_matching_rows` |
| UC2 | `gaussian5x5` | `separable_gaussian` |
| UC7 | `email` | `email_match` |
| UC8 | `ivm_sum` | `apply_row_delta` |
| UC12 | `groupby_sum` | `grouped_sum` |
| UC14 | `sort_int64` | `generic_sort` |

**DuckDB data impact**: Old run rows retain the old benchmark names (no data is deleted or modified). New runs produce rows with the new names. Because the `name` column is part of the PRIMARY KEY, old and new rows coexist. Old rows have `kv_a IS NULL`; new rows have `kv_a = 'low'`. The `v_ratios` view groups by `(run_id, kernel, raw_params, group)` — since `kv_n` changes, old and new rows belong to different `kernel` groups. This is acceptable because the new runs are the research data; the old runs are superseded.

**Alternatives considered**:
- Keep old `kv_n` values, add `a:low` only — rejected because variant 1 must be named consistently with variants 2 and 3 for `v_ratios` to pivot correctly across abstraction levels.
- Write a DuckDB migration to rename old `kv_n` values — rejected as unnecessary overhead since old run data is superseded by new runs.

---

## Decision 6 — Tradeoff-tier specializability

**Decision**: Template-instantiated tradeoff-tier types (e.g., `RowScanner<8, 16>`) are defined in the kernel TU with explicit instantiation. The lambda factory creates a local instance and captures it by value (or calls a method on it). The JIT specializer sees the concrete instantiation in the blob.

**Rationale**: C++ templates are instantiated at compile time. When the lambda captures template parameters as constants and the template is instantiated in the same TU, the JIT blob contains the instantiated IR. The `IPSCCP` pass can propagate template parameter constants through the inlined method bodies.

**Alternatives considered**:
- Using `constexpr` non-type template parameters baked in at compile time — rejected because the template parameters represent the specialization constants (e.g., column offset), so they must vary per specialization call and be captured by the lambda, not fixed at compile time.

---

## Decision 7 — Single unified benchmark file per UC

**Decision**: Keep one `UC<N>Benchmark.cpp` per UC that registers all 9 kernel variants (3 variants × 3 levels × 3 phases = 27 benchmark registrations per UC, × 4 sizes = 108 BENCHMARK() macros per UC). A single nested macro `UC<N>_BENCHMARK_SPEC` expands all registrations.

**Rationale**: Following the existing pattern (single `UC1Benchmark.cpp`), the unified file avoids symbol collision between benchmark functions. All variants share the same data fixtures (rows, images, etc.) for memory efficiency.

**Alternatives considered**:
- One benchmark file per (variant × level) — rejected because it fragments data fixtures and complicates AllBenchmarks linking (18 benchmark object files per UC instead of 1).
