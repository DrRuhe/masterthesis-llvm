# Tasks: Use-Case Benchmark Abstraction Variants

**Input**: Design documents from `specs/011-uc-benchmark-abstraction-variants/`
**Prerequisites**: plan.md ✅ spec.md ✅ research.md ✅ data-model.md ✅

**Organization**: Phases 1–2 are infrastructure that blocks all user story work. Phases 3–8 implement UC1 (reference) then UC2–UC14 (parallel replication). Phase 9 validates US3 (DuckDB import). No test tasks requested.

**Subagent + commit protocol**: Each task is designed to be executed by an independent subagent. After completing a task, the subagent MUST commit all created/modified files with a message referencing the task ID (e.g., `feat(uc1): T021 add CountMatchingRowsTradeoffKernels`). Review tasks (marked `[REVIEW]`) are verification-only: the subagent builds the relevant binary, runs validation functions, and commits no code changes — only reports pass/fail.

## Format: `[ID] [P?] [REVIEW?] [Story] Description`

- **[P]**: Parallelizable (independent files, no blocking dependency)
- **[REVIEW]**: Build + correctness verification task (no new code written)
- **[USn]**: Maps to User Story n from spec.md

---

## Phase 1: CMake Infrastructure

**Purpose**: Extend the build system to compile N kernel TUs per UC. All tasks can run in parallel; must all complete before Phase 3+.

**⚠️ CRITICAL**: Until CMake supports list variables `UC<N>_KERNELS_SRCS`, no new kernel TUs can be added to the build.

- [X] T001 [P] Rewrite `benchmarks/use-cases/UC1SqlPredicate/CMakeLists.txt` to compile kernel sources from a `UC1_KERNEL_SRCS` list using a foreach loop (one `add_custom_command` per source, all at -O0 with plugin), collect objects into `_KERN_OBJS`, export `UC1_KERNELS_SRCS` and `UC1_KERNELS_OBJS` list variables to parent scope (replacing old singular `UC1_KERNELS_SRC`/`UC1_KERNELS_OBJ`). See plan.md §Phase 1.1 for the exact CMake pattern.

- [X] T002 [P] Rewrite `benchmarks/use-cases/UC2Convolution/CMakeLists.txt` identically to T001 but for UC2 (exports `UC2_KERNELS_SRCS`, `UC2_KERNELS_OBJS`).

- [X] T003 [P] Rewrite `benchmarks/use-cases/UC7DfaRegex/CMakeLists.txt` identically for UC7 (exports `UC7_KERNELS_SRCS`, `UC7_KERNELS_OBJS`).

- [X] T004 [P] Rewrite `benchmarks/use-cases/UC8IVM/CMakeLists.txt` identically for UC8 (exports `UC8_KERNELS_SRCS`, `UC8_KERNELS_OBJS`).

- [X] T005 [P] Rewrite `benchmarks/use-cases/UC12GroupBy/CMakeLists.txt` identically for UC12 (exports `UC12_KERNELS_SRCS`, `UC12_KERNELS_OBJS`).

- [X] T006 [P] Rewrite `benchmarks/use-cases/UC14Sort/CMakeLists.txt` identically for UC14 (exports `UC14_KERNELS_SRCS`, `UC14_KERNELS_OBJS`).

- [X] T007 Update `benchmarks/use-cases/CMakeLists.txt`: change the `foreach(UC IN ITEMS UC1 UC2 UC7 UC8 UC12 UC14)` re-export block to propagate the plural `UC<N>_KERNELS_SRCS` and `UC<N>_KERNELS_OBJS` list variables (replacing the old singular variables). See plan.md §1.2.

- [X] T008 Update `benchmarks/CMakeLists.txt` AllBenchmarks section (lines ~242–278): replace the single-kernel-source `foreach` loop with a nested foreach that iterates `${UC}_KERNELS_SRCS` for each UC and produces one `AllBenchmarks_<stem>.o` object per source. Update `add_benchmark(AllBenchmarks …)` to use `${_ALL_UC_KERNEL_OBJS}`. See plan.md §1.3.

- [ ] R001 [REVIEW] Verify Phase 1 CMake infrastructure: from `llvm/llvm/build/debug` run `ninja UC1SqlPredicate UC2Convolution UC7DfaRegex UC8IVM UC12GroupBy UC14Sort`. All six must build successfully — this confirms the list-variable CMake pattern compiles (Phase 2 renames will already be present since T001–T008 include the new filenames in `UC<N>_KERNEL_SRCS`). If any target fails, fix the CMakeLists error before proceeding. No code committed by this review task. **Depends on T001–T008 AND T009–T014 (renames needed so filenames match).**

---

## Phase 2: Rename Existing TUs + FR-010 Re-tag

**Purpose**: Rename the 6 existing low-level kernel source files to the new `<Variant>LowKernels` naming convention and update all 6 benchmark files to add `a:low` and the new `kv_n` values. This satisfies FR-010 and SC-003 (existing benchmarks become queryable by `kv_a`).

**Depends on**: Phase 1 complete (CMakeLists already list the new filenames).

- [X] T009 [P] Rename `benchmarks/use-cases/UC1SqlPredicate/UC1Kernels.cpp` → `UC1CountMatchingRowsLowKernels.cpp`. No functional changes; only filename changes. Update `benchmarks/use-cases/UC1SqlPredicate/CMakeLists.txt` `UC1_KERNEL_SRCS` list to reference the new filename (T001 will have added the full list; verify it includes this file).

- [X] T010 [P] Rename `benchmarks/use-cases/UC2Convolution/UC2Kernels.cpp` → `UC2SeparableGaussianLowKernels.cpp`. No functional changes.

- [X] T011 [P] Rename `benchmarks/use-cases/UC7DfaRegex/UC7Kernels.cpp` → `UC7EmailMatchLowKernels.cpp`. No functional changes.

- [X] T012 [P] Rename `benchmarks/use-cases/UC8IVM/UC8Kernels.cpp` → `UC8ApplyRowDeltaLowKernels.cpp`. No functional changes.

- [X] T013 [P] Rename `benchmarks/use-cases/UC12GroupBy/UC12Kernels.cpp` → `UC12GroupedSumLowKernels.cpp`. No functional changes.

- [X] T014 [P] Rename `benchmarks/use-cases/UC14Sort/UC14Kernels.cpp` → `UC14GenericSortLowKernels.cpp`. No functional changes.

- [X] T015 [P] Update `benchmarks/use-cases/UC1SqlPredicate/UC1Benchmark.cpp`: in the `UC1_BENCHMARK_SPEC` macro, change all `->Name(…)` strings from `n:predicate;` → `n:count_matching_rows;a:low;` (insert `a:low;` after the `n:` value). Apply to all 12 BENCHMARK lines. Example: `"BM_g:uc1_sql;n:predicate;s:SMALL;t:unspecialized;"` → `"BM_g:uc1_sql;n:count_matching_rows;a:low;s:SMALL;t:unspecialized;"`.

- [X] T016 [P] Update `benchmarks/use-cases/UC2Convolution/UC2Benchmark.cpp`: change `n:gaussian5x5;` → `n:separable_gaussian;a:low;` in all BENCHMARK Name strings.

- [X] T017 [P] Update `benchmarks/use-cases/UC7DfaRegex/UC7Benchmark.cpp`: change `n:email;` → `n:email_match;a:low;` in all BENCHMARK Name strings.

- [X] T018 [P] Update `benchmarks/use-cases/UC8IVM/UC8Benchmark.cpp`: change `n:ivm_sum;` → `n:apply_row_delta;a:low;` in all BENCHMARK Name strings.

- [X] T019 [P] Update `benchmarks/use-cases/UC12GroupBy/UC12Benchmark.cpp`: change `n:groupby_sum;` → `n:grouped_sum;a:low;` in all BENCHMARK Name strings.

- [X] T020 [P] Update `benchmarks/use-cases/UC14Sort/UC14Benchmark.cpp`: change `n:sort_int64;` → `n:generic_sort;a:low;` in all BENCHMARK Name strings.

- [ ] R002 [REVIEW] Verify Phase 2 re-tagging: build all 6 UC binaries (`ninja UC1SqlPredicate UC2Convolution UC7DfaRegex UC8IVM UC12GroupBy UC14Sort`). For each binary, run `./UCxBinary --benchmark_filter="a:low" --benchmark_list_tests` and verify (a) at least one result contains `a:low`, (b) no result still uses the old `kv_n` values (`predicate`, `gaussian5x5`, `email`, `ivm_sum`, `groupby_sum`, `sort_int64`). Run validation functions via `./UCxBinary --benchmark_filter=NOTHING`. All must pass without errors. No code committed by this review task. **Depends on T001–T020.**

---

## Phase 3: UC1 Reference Implementation (US1 + US2)

**Goal**: Implement all 3 variants × 3 abstraction levels for UC1 SQL predicate benchmarks. This is the reference implementation — its patterns are replicated verbatim in Phases 4–8.

**Independent Test**: `ninja UC1SqlPredicate && ./UC1SqlPredicate --benchmark_filter="s:SMALL" 2>&1 | grep -E "n:(count_matching_rows|multi_predicate|column_scan);a:(low|tradeoff|abstract)"` must show 9 distinct benchmark groups (3 variants × 3 levels).

**Depends on**: T001, T009, T015 complete.

### Variant 1: `count_matching_rows` — Tradeoff & Abstract tiers

- [X] T021 [P] [US1] Create `benchmarks/use-cases/UC1SqlPredicate/UC1CountMatchingRowsTradeoffKernels.cpp`. Tradeoff kernel: a `RowScanner` struct with a `scan(const uint8_t* rows, int64_t n, double threshold) const` method that uses `__builtin_memcpy` to extract a `double` at `col_offset` with `row_stride`. Lambda factory `create_count_matching_rows_tradeoff_specialized(int col_offset, int row_stride, double threshold)` captures the three constants and calls the scanner method. Returns `CountMatchingRowsTradeoffSpecialized`. No static globals; no benchmark headers. Validation function `validate_count_matching_rows_tradeoff_specialized(...)` compares output against the low-level kernel on a 100-row test dataset.

- [X] T022 [P] [US1] Create `benchmarks/use-cases/UC1SqlPredicate/UC1CountMatchingRowsAbstractKernels.cpp`. Abstract kernel: `Predicate` base class with `virtual bool test(const uint8_t* row) const = 0`, `ThresholdPredicate` subclass storing `col_offset`, `row_stride`, `threshold` as fields. `scan_with_predicate(rows, n, row_stride, pred)` free function loops over all rows (no early exit per FR-011). Lambda factory `create_count_matching_rows_abstract_specialized(int col_offset, int row_stride, double threshold)` captures a `ThresholdPredicate` **by value** in the lambda so the vtable pointer is a JIT constant. Returns `CountMatchingRowsAbstractSpecialized`. Validation function compares against low-level kernel on same test dataset.

### Variant 2: `multi_predicate` — All 3 tiers

- [X] T023 [P] [US2] Create `benchmarks/use-cases/UC1SqlPredicate/UC1MultiPredicateLowKernels.cpp`. Low-level kernel `multi_predicate_count(const uint8_t* rows, int64_t n_rows, int row_stride, int col_offset_a, int col_offset_b, double threshold_a, double threshold_b)` extracts two `double` fields via `__builtin_memcpy` and counts rows where BOTH exceed their thresholds (full scan, no early exit). Lambda factory `create_multi_predicate_low_specialized(int row_stride, int col_offset_a, int col_offset_b, double threshold_a, double threshold_b)` captures 5 constants. Returns `MultiPredicateLowSpecialized`. Validation function included.

- [X] T024 [P] [US1] [US2] Create `benchmarks/use-cases/UC1SqlPredicate/UC1MultiPredicateTradeoffKernels.cpp`. Tradeoff: a `BinaryPredicateScanner` struct taking `col_offset_a`, `col_offset_b`, `threshold_a`, `threshold_b` in its `scan(rows, n, row_stride)` method. Lambda captures the struct by value and calls scan. Returns `MultiPredicateTradeoffSpecialized`. Validation included.

- [X] T025 [P] [US1] [US2] Create `benchmarks/use-cases/UC1SqlPredicate/UC1MultiPredicateAbstractKernels.cpp`. Abstract: `AndPredicate` subclass of `Predicate` that holds two `ThresholdPredicate` member objects (by value, not pointers). `AndPredicate::test(row)` calls both `pred_a.test()` and `pred_b.test()` and ANDs the results (no early exit — both calls always happen per FR-011). Lambda captures `AndPredicate` by value. Returns `MultiPredicateAbstractSpecialized`. Validation included.

### Variant 3: `column_scan` — All 3 tiers

- [X] T026 [P] [US2] Create `benchmarks/use-cases/UC1SqlPredicate/UC1ColumnScanLowKernels.cpp`. Low-level kernel `column_scan(const uint8_t* rows, int64_t n_rows, int row_stride, int col_offset, double threshold, int32_t* out) -> int64_t` loops over all rows, writes matching row indices as `int32_t` to `out`, returns count. Specialization constants: `row_stride`, `col_offset`, `threshold`. Lambda signature: `(const uint8_t* rows, int64_t n_rows, int32_t* out) -> int64_t`. Returns `ColumnScanLowSpecialized`. Validation included.

- [X] T027 [P] [US1] [US2] Create `benchmarks/use-cases/UC1SqlPredicate/UC1ColumnScanTradeoffKernels.cpp`. Tradeoff: `ProjectingScanner` struct with `scan_to(rows, n, row_stride, threshold, out) -> int64_t`. Lambda captures scanner by value, returns `ColumnScanTradeoffSpecialized`. Validation included.

- [X] T028 [P] [US1] [US2] Create `benchmarks/use-cases/UC1SqlPredicate/UC1ColumnScanAbstractKernels.cpp`. Abstract: `ResultCollector` interface with `virtual void emit(int64_t row_idx) = 0`. `BufferCollector` subclass writes to a fixed-size `int32_t*` array and tracks a count. `scan_collecting(rows, n, row_stride, col_offset, threshold, collector)` loops over all rows. Lambda captures `BufferCollector` by value (including its `out` pointer and count field). Returns `ColumnScanAbstractSpecialized`. Validation included.

### Header + Benchmark updates for UC1

- [X] T029 [US1] [US2] Expand `benchmarks/use-cases/UC1SqlPredicate/UC1Kernels.h` to declare all 9 variant×level factory functions and validation functions, plus 9 `using` type aliases (`using CountMatchingRowsLowSpecialized = …`, `using CountMatchingRowsTradeoffSpecialized = …`, etc. — all share the same `SpecializedLambda<int64_t, const uint8_t*, int64_t>` base type). Retain `SQLSpecialized` as an alias for backward compatibility. **Depends on T021–T028.**

- [X] T030 [US1] [US2] [US3] Rewrite `benchmarks/use-cases/UC1SqlPredicate/UC1Benchmark.cpp` to add benchmark functions and BENCHMARK macro registrations for all 9 variant×level combinations (low was already done in T015; add tradeoff and abstract). Benchmark name format: `BM_g:uc1_sql;n:<variant>;a:<level>;s:<size>;t:<type>;`. Add all `validate_*` calls to `main()`. Retain existing data fixtures (`g_rows`, `ROW_STRIDE`, `COL_OFFSET`). Add a second output buffer `g_out_indices` (pre-allocated `int32_t` array of N_ROWS_MAX) for `column_scan` benchmarks. `#ifdef ALL_BENCHMARKS_BUILD` guard uses smaller sizes. **Depends on T029.**

- [ ] R003 [REVIEW] Verify UC1 complete implementation: (1) `ninja UC1SqlPredicate` builds without error. (2) `./UC1SqlPredicate --benchmark_filter=NOTHING` runs all 9 `validate_*` functions without throwing. (3) `./UC1SqlPredicate --benchmark_filter="s:SMALL" --benchmark_list_tests | grep -cE "a:(low|tradeoff|abstract)"` must return at least 27 (9 variants × 3 phases). (4) Check that no name string still contains the old `n:predicate` pattern. No code committed by this review task. **Depends on T021–T030.**

---

## Phase 4: UC2 Convolution Implementation (US1 + US2)

**Goal**: All 3 variants × 3 levels for UC2 2D Convolution. Follows the UC1 pattern exactly.

**Independent Test**: `ninja UC2Convolution && ./UC2Convolution --benchmark_filter="s:SMALL" --benchmark_list_tests` shows 9 (variant, level) combinations.

**Depends on**: T002, T010, T016 complete.

- [X] T031 [P] [US2] Create `benchmarks/use-cases/UC2Convolution/UC2BoxFilterLowKernels.cpp`. Kernel `box_filter(src, dst, width, height, radius)` applies a 2D box filter (average of `(2r+1)²` neighborhood) using two nested loops, clamped at borders. Specialization constants: `width`, `height`, `radius`. Full image processed (no early exit). Lambda factory `create_box_filter_low_specialized(int width, int height, int radius)`. Validation: compare against unspecialized on a small test image.

- [X] T032 [P] [US2] Create `benchmarks/use-cases/UC2Convolution/UC2EdgeDetectionLowKernels.cpp`. Kernel `sobel_edge_detect(src, dst, width, height)` applies Sobel Gx and Gy kernels (3×3, hardcoded coefficients), computes `sqrt(Gx²+Gy²)` per pixel, full image processed. Specialization constants: `width`, `height`. Lambda factory `create_edge_detection_low_specialized(int width, int height)`. Validation included.

- [X] T033 [P] [US1] Create `benchmarks/use-cases/UC2Convolution/UC2SeparableGaussianTradeoffKernels.cpp`. Tradeoff: `SeparableFilter` struct with `apply(src, dst, width, height, coeffs, ksize)` method. Lambda captures `width`, `height`, `ksize`, and `coeffs` array by value. Returns `SeparableGaussianTradeoffSpecialized`. Validation included.

- [X] T034 [P] [US1] Create `benchmarks/use-cases/UC2Convolution/UC2BoxFilterTradeoffKernels.cpp`. Tradeoff: `BoxFilter` struct with `radius` field. `apply(src, dst, width, height)` method. Lambda captures `BoxFilter` by value (with `width`, `height` also captured). Returns `BoxFilterTradeoffSpecialized`. Validation included.

- [X] T035 [P] [US1] [US2] Create `benchmarks/use-cases/UC2Convolution/UC2EdgeDetectionTradeoffKernels.cpp`. Tradeoff: `SobelFilter` struct with `static constexpr float GX[9]` and `GY[9]` coefficient arrays and `detect(src, dst, width, height)` method. Lambda captures struct by value. Returns `EdgeDetectionTradeoffSpecialized`. Validation included.

- [X] T036 [P] [US1] Create `benchmarks/use-cases/UC2Convolution/UC2SeparableGaussianAbstractKernels.cpp`. Abstract: `FilterKernel` base with `virtual float weight(int offset) const = 0` and `virtual int radius() const = 0`. `GaussianKernel` subclass stores coefficients in a fixed-size array (by value, not `std::vector`). `Convolver` struct holds a `GaussianKernel` by value and applies it. Lambda captures `Convolver` by value. Returns `SeparableGaussianAbstractSpecialized`. Validation included.

- [X] T037 [P] [US1] [US2] Create `benchmarks/use-cases/UC2Convolution/UC2BoxFilterAbstractKernels.cpp`. Abstract: `SpatialFilter` interface with `virtual void apply(const float* src, float* dst, int width, int height) = 0`. `BoxFilter` subclass with `radius` field. Lambda captures `BoxFilter` by value. Returns `BoxFilterAbstractSpecialized`. Validation included.

- [X] T038 [P] [US1] [US2] Create `benchmarks/use-cases/UC2Convolution/UC2EdgeDetectionAbstractKernels.cpp`. Abstract: `GradientDetector` interface with `virtual void detect(const float* src, float* dst, int width, int height) = 0`. `SobelDetector` subclass with static coefficient arrays. Lambda captures `SobelDetector` by value. Returns `EdgeDetectionAbstractSpecialized`. Validation included.

- [X] T039 [US1] [US2] Expand `benchmarks/use-cases/UC2Convolution/UC2Kernels.h` with all new factory/validation declarations and type aliases for all 9 UC2 variant×level combinations. **Depends on T031–T038.**

- [X] T040 [US1] [US2] [US3] Update `benchmarks/use-cases/UC2Convolution/UC2Benchmark.cpp` to add benchmark functions and registrations for all 9 variant×level combinations. Benchmark names: `BM_g:uc2_conv;n:<variant>;a:<level>;s:<size>;t:<type>;`. Add `validate_*` calls to `main()`. Reuse existing image data fixture. **Depends on T039.**

- [ ] R004 [REVIEW] Verify UC2: `ninja UC2Convolution` builds. `./UC2Convolution --benchmark_filter=NOTHING` runs all 9 validations without error. `./UC2Convolution --benchmark_list_tests | grep -cE "a:(tradeoff|abstract)"` ≥ 18 (6 variants × 3 phases for the two new levels). No code committed. **Depends on T031–T040.**

---

## Phase 5: UC7 DFA Regex Implementation (US1 + US2)

**Goal**: All 3 variants × 3 levels for UC7 DFA pattern matching. Note: `multi_pattern_match` counts ALL matches across the full buffer (no early exit, per FR-011).

**Independent Test**: `ninja UC7DfaRegex && ./UC7DfaRegex --benchmark_list_tests` shows 9 (variant, level) combinations.

**Depends on**: T003, T011, T017 complete.

- [X] T041 [P] [US2] Create `benchmarks/use-cases/UC7DfaRegex/UC7UrlMatchLowKernels.cpp`. Build a URL DFA table (scheme `://` host pattern; more states than email). Kernel `url_match(s, len, dfa_table, n_states, n_chars, start_state, accept_state)` loops over all characters (no early exit). Lambda factory `create_url_match_low_specialized(dfa_table, n_states, n_chars, start_state, accept_state)` where the table and state params are specialization constants. Returns `UrlMatchLowSpecialized`. Validation: test known-valid and known-invalid URL strings.

- [X] T042 [P] [US2] Create `benchmarks/use-cases/UC7DfaRegex/UC7MultiPatternMatchLowKernels.cpp`. Kernel `multi_pattern_match_count(buf, len, dfa_table, n_states, n_chars, accept_states, n_accept_states)` scans ALL bytes, increments a counter each time the automaton reaches any accept state, then resets to start state and continues (no early exit — full buffer scan). Returns total match count. Specialization constants: `dfa_table`, `n_states`, `n_chars`, `accept_states`, `n_accept_states`. Validation: test on a buffer with known number of pattern occurrences.

- [X] T043 [P] [US1] Create `benchmarks/use-cases/UC7DfaRegex/UC7EmailMatchTradeoffKernels.cpp`. Tradeoff: `DFAMatcher` struct with `n_states`, `n_chars`, `start_state`, `accept_state`, and `table` pointer fields. `match(s, len)` loops over all chars. Lambda captures `DFAMatcher` by value. Returns `EmailMatchTradeoffSpecialized`. Validation included.

- [X] T044 [P] [US1] [US2] Create `benchmarks/use-cases/UC7DfaRegex/UC7UrlMatchTradeoffKernels.cpp`. Same `DFAMatcher` struct, URL DFA table. Returns `UrlMatchTradeoffSpecialized`. Validation included.

- [X] T045 [P] [US1] [US2] Create `benchmarks/use-cases/UC7DfaRegex/UC7MultiPatternMatchTradeoffKernels.cpp`. Tradeoff: `MultiPatternMatcher` struct with `n_states`, `n_chars`, `accept_states` array (fixed-size), `n_accept_states` fields. `match_all_count(buf, len)` scans full buffer. Lambda captures `MultiPatternMatcher` by value. Returns `MultiPatternMatchTradeoffSpecialized`. Validation included.

- [X] T046 [P] [US1] Create `benchmarks/use-cases/UC7DfaRegex/UC7EmailMatchAbstractKernels.cpp`. Abstract: `Matcher` interface with `virtual bool match(const char* s, int64_t len) const = 0`. `DFAMatcher` subclass stores table and state metadata. Lambda captures concrete `DFAMatcher` by value. Returns `EmailMatchAbstractSpecialized`. Validation included.

- [X] T047 [P] [US1] [US2] Create `benchmarks/use-cases/UC7DfaRegex/UC7UrlMatchAbstractKernels.cpp`. Same `Matcher`/`DFAMatcher` hierarchy, URL table. Returns `UrlMatchAbstractSpecialized`. Validation included.

- [X] T048 [P] [US1] [US2] Create `benchmarks/use-cases/UC7DfaRegex/UC7MultiPatternMatchAbstractKernels.cpp`. Abstract: `PatternSet` interface with `virtual int64_t match_all_count(const char* buf, int64_t len) const = 0`. `DFAPatternSet` subclass scans full buffer and returns total match count. Lambda captures concrete `DFAPatternSet` by value. Returns `MultiPatternMatchAbstractSpecialized`. Validation included.

- [X] T049 [US1] [US2] Expand `benchmarks/use-cases/UC7DfaRegex/UC7Kernels.h` with all 9 declarations and type aliases. **Depends on T041–T048.**

- [X] T050 [US1] [US2] [US3] Update `benchmarks/use-cases/UC7DfaRegex/UC7Benchmark.cpp` with all 9 variant×level benchmark registrations. Names: `BM_g:uc7_dfa;n:<variant>;a:<level>;s:<size>;t:<type>;`. Add validation calls. **Depends on T049.**

- [ ] R005 [REVIEW] Verify UC7: `ninja UC7DfaRegex` builds. `./UC7DfaRegex --benchmark_filter=NOTHING` runs all validations without error. Verify `multi_pattern_match` variants produce a non-negative match count (not -1). `./UC7DfaRegex --benchmark_list_tests | grep -cE "a:(tradeoff|abstract)"` ≥ 18. No code committed. **Depends on T041–T050.**

---

## Phase 6: UC8 IVM Implementation (US1 + US2)

**Goal**: All 3 variants × 3 levels for UC8 Incremental View Materialization.

**Independent Test**: `ninja UC8IVM && ./UC8IVM --benchmark_list_tests` shows 9 (variant, level) combinations.

**Depends on**: T004, T012, T018 complete.

- [X] T051 [P] [US2] Create `benchmarks/use-cases/UC8IVM/UC8MultiAggDeltaLowKernels.cpp`. Kernel `multi_agg_delta(row, n_buckets, group_col_offset, value_col_offset, row_stride, sum_buckets, count_buckets)` extracts group key and value, updates both `sum_buckets[bucket]` and `count_buckets[bucket]`. Specialization constants: `n_buckets`, `group_col_offset`, `value_col_offset`, `row_stride`. Lambda signature: `(const uint8_t* row, double* sum_buckets, double* count_buckets) -> void`. Returns `MultiAggDeltaLowSpecialized`. Validation included.

- [X] T052 [P] [US2] Create `benchmarks/use-cases/UC8IVM/UC8BatchDeltaLowKernels.cpp`. Kernel `batch_delta(rows, n_rows, n_buckets, group_col_offset, value_col_offset, row_stride, sum_buckets)` loops over `n_rows`, extracting and accumulating. All 5 parameters are specialization constants. Lambda: `(const uint8_t* rows, double* sum_buckets) -> void`. Returns `BatchDeltaLowSpecialized`. Validation included.

- [X] T053 [P] [US1] Create `benchmarks/use-cases/UC8IVM/UC8ApplyRowDeltaTradeoffKernels.cpp`. Tradeoff: `IVMUpdater` struct with `n_buckets`, `group_col_offset`, `value_col_offset`, `row_stride` fields. `update(row, buckets)` method. Lambda captures `IVMUpdater` by value. Returns `ApplyRowDeltaTradeoffSpecialized`. Validation included.

- [X] T054 [P] [US1] [US2] Create `benchmarks/use-cases/UC8IVM/UC8MultiAggDeltaTradeoffKernels.cpp`. Tradeoff: `IVMDualUpdater` struct with fields for both output arrays' parameters. `update(row, sum_buckets, count_buckets)` method. Lambda captures by value. Returns `MultiAggDeltaTradeoffSpecialized`. Validation included.

- [X] T055 [P] [US1] [US2] Create `benchmarks/use-cases/UC8IVM/UC8BatchDeltaTradeoffKernels.cpp`. Tradeoff: `BatchIVMUpdater` struct with `n_rows` and layout fields. `process(rows, sum_buckets)` method. Lambda captures by value. Returns `BatchDeltaTradeoffSpecialized`. Validation included.

- [X] T056 [P] [US1] Create `benchmarks/use-cases/UC8IVM/UC8ApplyRowDeltaAbstractKernels.cpp`. Abstract: `Aggregator` interface with `virtual void apply(const uint8_t* row, double* state) = 0`. `SumAggregator` subclass. Lambda captures `SumAggregator` by value. Returns `ApplyRowDeltaAbstractSpecialized`. Validation included.

- [X] T057 [P] [US1] [US2] Create `benchmarks/use-cases/UC8IVM/UC8MultiAggDeltaAbstractKernels.cpp`. Abstract: `MultiAggregator` that holds two `Aggregator`-derived objects (`SumAggregator`, `CountAggregator`) by value as a struct pair (not via `unique_ptr` — use value members so vtables are JIT constants). Lambda captures `MultiAggregator` by value. Returns `MultiAggDeltaAbstractSpecialized`. Validation included.

- [X] T058 [P] [US1] [US2] Create `benchmarks/use-cases/UC8IVM/UC8BatchDeltaAbstractKernels.cpp`. Abstract: `BatchProcessor` interface with `virtual void process_batch(const uint8_t* rows, int64_t n_rows, double* buckets) = 0`. `SumBatchProcessor` subclass. Lambda captures `SumBatchProcessor` by value. Returns `BatchDeltaAbstractSpecialized`. Validation included.

- [X] T059 [US1] [US2] Expand `benchmarks/use-cases/UC8IVM/UC8Kernels.h` with all 9 declarations and type aliases. **Depends on T051–T058.**

- [X] T060 [US1] [US2] [US3] Update `benchmarks/use-cases/UC8IVM/UC8Benchmark.cpp` with all 9 variant×level registrations. Names: `BM_g:uc8_ivm;n:<variant>;a:<level>;s:<size>;t:<type>;`. Add validation calls. **Depends on T059.**

- [ ] R006 [REVIEW] Verify UC8: `ninja UC8IVM` builds. `./UC8IVM --benchmark_filter=NOTHING` runs all validations without error. `./UC8IVM --benchmark_list_tests | grep -cE "a:(tradeoff|abstract)"` ≥ 18. No code committed. **Depends on T051–T060.**

---

## Phase 7: UC12 GroupBy Implementation (US1 + US2)

**Goal**: All 3 variants × 3 levels for UC12 grouped aggregation.

**Independent Test**: `ninja UC12GroupBy && ./UC12GroupBy --benchmark_list_tests` shows 9 (variant, level) combinations.

**Depends on**: T005, T013, T019 complete.

- [X] T061 [P] [US2] Create `benchmarks/use-cases/UC12GroupBy/UC12GroupedCountLowKernels.cpp`. Kernel `grouped_count(rows, n_rows, row_stride, key_offset, n_buckets, count_buckets)` extracts `int32_t` key via `__builtin_memcpy`, increments `count_buckets[key % n_buckets]`. No value column. Specialization constants: `row_stride`, `key_offset`, `n_buckets`. Lambda: `(const uint8_t* rows, int64_t n_rows, int64_t* out, int n_buckets) -> void`. Returns `GroupedCountLowSpecialized`. Validation included.

- [X] T062 [P] [US2] Create `benchmarks/use-cases/UC12GroupBy/UC12GroupedMinMaxLowKernels.cpp`. Kernel `grouped_minmax(rows, n_rows, row_stride, key_offset, value_offset, n_buckets, min_buckets, max_buckets)` extracts key and value, updates both `min_buckets[bucket]` and `max_buckets[bucket]` in a single pass. Specialization constants: all 4 layout params. Lambda: `(rows, n_rows, min_buckets, max_buckets) -> void`. Returns `GroupedMinMaxLowSpecialized`. Validation included.

- [X] T063 [P] [US1] Create `benchmarks/use-cases/UC12GroupBy/UC12GroupedSumTradeoffKernels.cpp`. Tradeoff: `GroupedAggregator` struct with layout fields and `aggregate(rows, n_rows, out, n_buckets)` method. Lambda captures struct by value. Returns `GroupedSumTradeoffSpecialized`. Validation included.

- [X] T064 [P] [US1] [US2] Create `benchmarks/use-cases/UC12GroupBy/UC12GroupedCountTradeoffKernels.cpp`. Tradeoff: `GroupCounter` struct with `row_stride`, `key_offset`. `count(rows, n_rows, out, n_buckets)` method. Lambda captures by value. Returns `GroupedCountTradeoffSpecialized`. Validation included.

- [X] T065 [P] [US1] [US2] Create `benchmarks/use-cases/UC12GroupBy/UC12GroupedMinMaxTradeoffKernels.cpp`. Tradeoff: `MinMaxAggregator` struct with layout fields. `aggregate_minmax(rows, n_rows, min_out, max_out, n_buckets)` method. Lambda captures by value. Returns `GroupedMinMaxTradeoffSpecialized`. Validation included.

- [X] T066 [P] [US1] Create `benchmarks/use-cases/UC12GroupBy/UC12GroupedSumAbstractKernels.cpp`. Abstract: `AggregationOperator` interface with `virtual void aggregate(const uint8_t* rows, int64_t n_rows, double* out_buckets, int n_buckets) = 0`. `SumOperator` subclass with layout fields. Lambda captures `SumOperator` by value. Returns `GroupedSumAbstractSpecialized`. Validation included.

- [X] T067 [P] [US1] [US2] Create `benchmarks/use-cases/UC12GroupBy/UC12GroupedCountAbstractKernels.cpp`. Abstract: reuses `AggregationOperator`. `CountOperator` subclass writes to `int64_t*` cast from `out_buckets`. Lambda captures by value. Returns `GroupedCountAbstractSpecialized`. Validation included.

- [X] T068 [P] [US1] [US2] Create `benchmarks/use-cases/UC12GroupBy/UC12GroupedMinMaxAbstractKernels.cpp`. Abstract: `MinMaxOperator` subclass of `AggregationOperator` with dual-output signature. Lambda captures `MinMaxOperator` by value. Returns `GroupedMinMaxAbstractSpecialized`. Validation included.

- [X] T069 [US1] [US2] Expand `benchmarks/use-cases/UC12GroupBy/UC12Kernels.h` with all 9 declarations and type aliases. **Depends on T061–T068.**

- [X] T070 [US1] [US2] [US3] Update `benchmarks/use-cases/UC12GroupBy/UC12Benchmark.cpp` with all 9 variant×level registrations. Names: `BM_g:uc12_groupby;n:<variant>;a:<level>;s:<size>;t:<type>;`. Add validation calls. **Depends on T069.**

- [ ] R007 [REVIEW] Verify UC12: `ninja UC12GroupBy` builds. `./UC12GroupBy --benchmark_filter=NOTHING` runs all validations. `./UC12GroupBy --benchmark_list_tests | grep -cE "a:(tradeoff|abstract)"` ≥ 18. No code committed. **Depends on T061–T070.**

---

## Phase 8: UC14 Sort Implementation (US1 + US2)

**Goal**: All 3 variants × 3 levels for UC14 generic sort.

**Independent Test**: `ninja UC14Sort && ./UC14Sort --benchmark_list_tests` shows 9 (variant, level) combinations.

**Depends on**: T006, T014, T020 complete.

- [X] T071 [P] [US2] Create `benchmarks/use-cases/UC14Sort/UC14StructSortLowKernels.cpp`. Kernel `struct_sort(data, n, element_size, field_offset)` sorts fixed-size structs by a `double` field at `field_offset` via `__builtin_memcpy` in the comparator. Uses same iterative quicksort pattern as existing `UC14GenericSortLowKernels.cpp`. Specialization constants: `element_size`, `field_offset`. Lambda: `(void* data, int64_t n) -> void`. Returns `StructSortLowSpecialized`. Validation: sort small array, verify order.

- [X] T072 [P] [US2] Create `benchmarks/use-cases/UC14Sort/UC14MultiKeySortLowKernels.cpp`. Kernel `multi_key_sort(data, n, element_size, key1_offset, key2_offset)` sorts by two `double` fields (key1 ascending, key2 descending) using a two-level comparator. Specialization constants: `element_size`, `key1_offset`, `key2_offset`. Lambda: `(void* data, int64_t n) -> void`. Returns `MultiKeySortLowSpecialized`. Validation: sort small array, verify two-level order.

- [X] T073 [P] [US1] Create `benchmarks/use-cases/UC14Sort/UC14GenericSortTradeoffKernels.cpp`. Tradeoff: `GenericSorter` struct with `element_size` field. `sort(data, n, cmp)` method. Lambda captures `GenericSorter` by value and passes a comparator function pointer as specialization constant. Returns `GenericSortTradeoffSpecialized`. Validation included.

- [X] T074 [P] [US1] [US2] Create `benchmarks/use-cases/UC14Sort/UC14StructSortTradeoffKernels.cpp`. Tradeoff: `FieldComparator` struct with `field_offset`. Static `compare(a, b)` method extracts `double` at offset. Lambda captures `FieldComparator` + sorter by value. Returns `StructSortTradeoffSpecialized`. Validation included.

- [X] T075 [P] [US1] [US2] Create `benchmarks/use-cases/UC14Sort/UC14MultiKeySortTradeoffKernels.cpp`. Tradeoff: `MultiKeyComparator` struct with `key1_offset`, `key2_offset`. `compare(a, b)` performs two-level comparison. Lambda captures by value. Returns `MultiKeySortTradeoffSpecialized`. Validation included.

- [X] T076 [P] [US1] Create `benchmarks/use-cases/UC14Sort/UC14GenericSortAbstractKernels.cpp`. Abstract: `Comparator` interface with `virtual int compare(const void* a, const void* b) const = 0`. `Int64AscComparator` subclass. `Sorter` struct holds a `Comparator` by value (embed concrete type directly). Lambda captures `Sorter` by value. Returns `GenericSortAbstractSpecialized`. Validation included.

- [X] T077 [P] [US1] [US2] Create `benchmarks/use-cases/UC14Sort/UC14StructSortAbstractKernels.cpp`. Abstract: `FieldExtractor` interface with `virtual double extract(const void* record) const = 0`. `ByteOffsetExtractor` subclass. `FieldComparatorAdapter` struct holds a `ByteOffsetExtractor` by value and implements comparison. Lambda captures by value. Returns `StructSortAbstractSpecialized`. Validation included.

- [X] T078 [P] [US1] [US2] Create `benchmarks/use-cases/UC14Sort/UC14MultiKeySortAbstractKernels.cpp`. Abstract: `CompositeComparator` struct holding two `ByteOffsetExtractor` objects (by value, not pointers) and two `bool` ascending flags. `compare(a, b)` iterates both extractors. Lambda captures `CompositeComparator` by value. Returns `MultiKeySortAbstractSpecialized`. Validation included.

- [X] T079 [US1] [US2] Expand `benchmarks/use-cases/UC14Sort/UC14Kernels.h` with all 9 declarations and type aliases. **Depends on T071–T078.**

- [X] T080 [US1] [US2] [US3] Update `benchmarks/use-cases/UC14Sort/UC14Benchmark.cpp` with all 9 variant×level registrations. Names: `BM_g:uc14_sort;n:<variant>;a:<level>;s:<size>;t:<type>;`. Add validation calls. Sort benchmarks must re-initialize data before each timed call (use `benchmark::DoNotOptimize` after reset; keep a pre-shuffled reference copy to restore from). **Depends on T079.**

- [ ] R008 [REVIEW] Verify UC14: `ninja UC14Sort` builds. `./UC14Sort --benchmark_filter=NOTHING` runs all validations. For sort benchmarks, verify sorted arrays are in correct order in the validation function (not just that no exception was thrown). `./UC14Sort --benchmark_list_tests | grep -cE "a:(tradeoff|abstract)"` ≥ 18. No code committed. **Depends on T071–T080.**

---

## Phase 9: AllBenchmarks Integration + US3 Validation

**Goal**: Verify all 54 benchmark scenarios appear in AllBenchmarks, are importable by `record_benchmark.py`, and meet SC-001–SC-005.

**Independent Test**: Run `record_benchmark.py` on AllBenchmarks output; verify `kv_a` column populated with `low`/`tradeoff`/`abstract`.

**Depends on**: All Phases 1–8 complete.

- [X] T081 Build and smoke-test `AllBenchmarks` target: `ninja AllBenchmarks` must succeed. Run `./AllBenchmarks --benchmark_filter="s:SMALL" --benchmark_list_tests | grep -c "a:"` and verify count equals number of scenarios with `a:` tags (at minimum 54 × 3 phases × 4 sizes = 648 entries expected, verify no missing UCs).

- [X] T082 [US3] Verify `record_benchmark.py` import: run `python benchmarks/record_benchmark.py ./AllBenchmarks --benchmark_filter="a:low|a:tradeoff|a:abstract" --benchmark_format=json` (or equivalent). Verify `kv_a` column is created and populated in `benchmarks.duckdb`. Run `duckdb benchmarks/benchmarks.duckdb "SELECT kv_a, COUNT(*) FROM benchmarks GROUP BY kv_a ORDER BY kv_a"` — expect `abstract`, `low`, `tradeoff` rows.

- [X] T083 [US1] Verify SC-001 (54 benchmark scenarios): run `duckdb benchmarks/benchmarks.duckdb "SELECT kv_g, kv_n, kv_a, COUNT(*) FROM benchmarks WHERE kv_a IS NOT NULL GROUP BY kv_g, kv_n, kv_a ORDER BY kv_g, kv_n, kv_a"` and confirm 18 distinct (group, variant) pairs × 3 levels = 54 rows.

- [X] T084 [US1] Verify SC-004 (numerical equivalence): all 6 individual UC binaries must run to completion without throwing `ClangRuntimeSpecializerChangesBehaviorError`. Run each with `--benchmark_filter="NOTHING"` so only `main()` validation logic executes: `for b in UC1SqlPredicate UC2Convolution UC7DfaRegex UC8IVM UC12GroupBy UC14Sort; do ninja $b && ./$b --benchmark_filter=NOTHING; done`.

- [ ] T085 [US1] Verify SC-005 (JIT overhead no regression): after recording a full run, query `SELECT kv_n, kv_a, AVG(real_time) FROM benchmarks WHERE kv_t='jit_overhead' AND kv_a='low' GROUP BY kv_n, kv_a` and compare manually against pre-feature baseline values. Flag any variant with >10% regression.

- [ ] R009 [REVIEW] Final integration review: (1) `ninja AllBenchmarks` builds without error. (2) Run `./AllBenchmarks --benchmark_filter="s:SMALL" --benchmark_list_tests | grep -cE "a:(low|tradeoff|abstract)"` — verify result ≥ 162 (54 scenarios × 3 phases). (3) Run `./AllBenchmarks --benchmark_filter="uc1_sql.*s:SMALL"` for a quick smoke run and verify it completes without crash. (4) Confirm `kv_a` column exists in DuckDB after import. No code committed. **Depends on T081–T085.**

---

## Phase 10: Polish

- [X] T086 [P] Run `ninja check-smoke-runtime-specializer` from `llvm/llvm/build/debug` to confirm smoke suite is still green. No kernel code touches the runtime library, but CMakeLists changes could affect the build graph. Commit a note in `plan.md` if any issue found.

- [X] T087 [P] Update `specs/011-uc-benchmark-abstraction-variants/plan.md` Key Constraints section with any implementation notes discovered during coding (e.g., if abstract-tier sort required `__attribute__((noinline))` on virtual call sites, document that). Commit the update.

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1** (CMake): No dependencies — start immediately
- **R001** (CMake review): Depends on Phase 1 + T009–T014
- **Phase 2** (Rename + FR-010): Depends on Phase 1 — CMakeLists must already reference new filenames
- **R002** (Phase 2 review): Depends on Phase 2 — gate before Phase 3+
- **Phases 3–8** (UC implementations): Depend on R002 — can all start in parallel
- **R003–R008** (per-UC reviews): Each depends on the corresponding UC phase; gate before Phase 9
- **Phase 9** (AllBenchmarks + validation): Depends on R003–R008
- **R009** (final review): Depends on Phase 9
- **Phase 10** (Polish): Depends on R009

### Within Each UC Phase

- All new kernel TU tasks [P] are independent of each other — can run in parallel
- Header update depends on all kernel TUs for that UC existing
- Benchmark file update depends on header update

### User Story Dependencies

- **US1** (abstraction levels): Tradeoff + abstract kernel TU tasks across all UCs
- **US2** (variant breadth): Variant 2 + 3 low-tier kernel TU tasks + benchmark registrations
- **US3** (DuckDB import): FR-010 re-tagging (Phase 2) + `a:` tag in all new benchmark names (implicit in Phase 3–8)

All three user stories are satisfied by UC1 alone (Phases 1–3 + T081–T084). Phases 4–8 extend coverage to all 6 UCs.

---

## Parallel Execution Examples

### Phase 1 (all parallel)
```
T001, T002, T003, T004, T005, T006, T007, T008
```

### Phase 2 (all parallel)
```
T009–T014 (renames), T015–T020 (FR-010 re-tags)
```

### Phase 3: UC1 (kernel TUs parallel, header+benchmark sequential)
```
Parallel: T021, T022, T023, T024, T025, T026, T027, T028
Then sequential: T029 (header), T030 (benchmark)
```

### Phases 4–8 (all UCs in parallel with each other)
```
Phase 4 (UC2): T031–T038 parallel → T039 → T040
Phase 5 (UC7): T041–T048 parallel → T049 → T050
Phase 6 (UC8): T051–T058 parallel → T059 → T060
Phase 7 (UC12): T061–T068 parallel → T069 → T070
Phase 8 (UC14): T071–T078 parallel → T079 → T080
```
All five of the above can proceed simultaneously.

---

## Implementation Strategy

### MVP (User Stories 1+2+3 on UC1 only)

1. Phase 1: T001, T007, T008 (UC1 CMake only)
2. R001 review
3. Phase 2: T009, T015 (UC1 rename + re-tag)
4. R002 review
5. Phase 3: T021–T030 (UC1 full implementation)
6. R003 review
7. Phase 9 (partial): T082–T084 on UC1SqlPredicate only
8. **STOP and VALIDATE**: all 3 user story acceptance criteria met for UC1

### Full Feature Delivery

1. Complete all Phase 1 tasks
2. Complete all Phase 2 tasks
3. Complete Phases 3–8 in parallel (6 UCs)
4. Complete Phase 9 (AllBenchmarks integration)
5. Complete Phase 10 (Polish)

---

## Notes

- [P] = parallelizable within its phase (independent files, no blockers)
- Abstract-tier kernel objects must **always** be captured by value (not `unique_ptr`) — see research.md Decision 4
- No `static` globals anywhere in kernel TUs — see FR-006
- No benchmark headers (`ClangRuntimeSpecializerBenchmark.h`, `<benchmark/benchmark.h>`) in kernel TUs — see Benchmark TU Separation Pattern
- All kernel loops must process their full input (FR-011, no early exit)
- Sort benchmarks need data reset between timed calls — keep a pre-shuffled reference copy
