# Feature Specification: Use-Case Benchmark Suite

**Feature Branch**: `008-use-case-benchmarks`  
**Created**: 2026-05-10  
**Status**: Draft  

## Clarifications

### Session 2026-05-13

- Q: How should the UC7 DFA transition table be constructed? → A: Programmatic at startup — a small builder function fills `g_dfa_table` once (e.g., via a static initializer or `__attribute__((constructor))`); no external regex library.
- Q: Which sorting algorithm should `generic_sort` implement? → A: Quicksort with median-of-three pivot selection.
- Q: Should `convolve2d` use separable (two-pass 1D) or non-separable (direct 2D) convolution? → A: Separable two-pass (horizontal then vertical 1D pass) to maximize constant-folding payoff when `ksize` is baked in.
- Q: How should UC14 re-shuffle between iterations? → A: Call `state.PauseTiming()` before `std::shuffle` and `state.ResumeTiming()` after, so only the sort itself is timed.
- Q: Should the six benchmarks be separate binaries or a combined binary? → A: One separate `add_benchmark` target per use case (satisfying FR-006), AND all six kernel and benchmark objects are added to the existing `AllBenchmarks` target.

---

## Overview

This feature adds six self-contained C++ benchmark workloads that exercise the
runtime specialization system on the "high-iteration, simple-structure" use case
archetypes identified during autoresearch. The six archetypes are:

| ID  | Archetype | Specialization Trigger |
|-----|-----------|----------------------|
| UC1 | SQL expression evaluation | Fixed predicate (column offset, operator, literal threshold) |
| UC2 | 2D image convolution | Fixed kernel coefficients and kernel size |
| UC7 | NFA/DFA regex matching | Fixed compiled pattern (state-transition table) |
| UC8 | Incremental view maintenance | Fixed aggregate schema (column offsets, aggregate type) |
| UC12 | Columnar analytics GROUP BY | Fixed row schema (row stride, column offset, data type) |
| UC14 | Sort with fixed comparator | Fixed comparison function (column key, sort direction) |

Each benchmark constitutes a representative case where a parameter set is fixed for
the duration of a "session" (query compilation, image processing session, compiled
pattern) but cannot be determined at build time. The specialized call count easily
exceeds 10 million iterations, amortizing the ~1.2 s JIT overhead by orders of
magnitude.

The benchmarks are intended to:

1. **Validate** that the existing specialization system yields measurable speedups
   beyond the existing polybench and TPC-H workloads.
2. **Demonstrate** the breadth of domains where the LLVM-JIT specialization
   approach applies, supporting the thesis narrative.
3. **Serve as additional workloads** for the pipeline-evaluation experiments
   specified in spec 007 (JIT Pipeline Evaluation Methodology), so that
   cross-workload claims are not limited to two benchmark suites.

---

## User Scenarios & Testing *(mandatory)*

### User Story 1 — Measure Specialization Payoff Across All Six Archetypes (Priority: P1)

The thesis researcher runs each new benchmark binary and records raw results in the
DuckDB benchmarks database. For every benchmark they can compare: (a) unspecialized
execution time, (b) JIT overhead (first-call compilation cost), and (c) specialized
execution time. The resulting dataset shows which archetypes benefit most from
specialization and which are near break-even.

**Why this priority**: The benchmarks only deliver value if they produce valid,
recordable performance data. Everything else (analysis, thesis narrative) depends
on this story working end-to-end.

**Independent Test**: Each benchmark binary can be run in isolation with
`--benchmark_filter=.*` and produces valid `BENCHMARK` output. Running
`record_benchmark.py --db <path> <binary>` successfully inserts rows into the
benchmarks database with non-zero `jit_overhead_ns` and `specialized_exec_ns` fields.

**Acceptance Scenarios**:

1. **Given** the UC1 SQL predicate benchmark binary compiled in release mode, **When** it runs with `--benchmark_filter=".*predicate.*"`, **Then** it reports a `jit_overhead_ns` value, an `unspecialized_exec_ns` value, and a `specialized_exec_ns` value for at least one benchmark variant.
2. **Given** each of the six benchmark binaries (UC1–UC14), **When** each is run in release mode, **Then** none crashes and all produce Google Benchmark output with at least one benchmark row.
3. **Given** benchmark output from all six binaries, **When** recorded into DuckDB via `record_benchmark.py`, **Then** all rows are inserted without schema errors and the `kernel` field uniquely identifies each archetype.

---

### User Story 2 — Confirm Speedup ≥ 10% for the Highest-Potential Archetypes (Priority: P2)

For the archetypes where the autoresearch predicted 4–7× speedup (UC2 convolution,
UC7 regex, UC1 SQL predicate), the researcher verifies that the measured
`specialized_exec_ns / unspecialized_exec_ns` ratio is ≤ 0.90 (≥ 10% speedup) under
the default pipeline configuration. This confirms the system realises the theoretically
predicted gains.

**Why this priority**: Demonstrating concrete speedups on new workloads is the
primary thesis contribution of this feature. Without measured speedups the benchmarks
only add bulk, not evidence.

**Independent Test**: Query `SELECT kernel, unspec_ns / spec_ns AS speedup FROM v_optim_best_per_kernel` for the three archetypes and verify at least two show speedup > 1.10.

**Acceptance Scenarios**:

1. **Given** release-build benchmark results for UC2 (convolution, 5×5 kernel, 1920×1080 input), **When** specialized execution time is compared to unspecialized, **Then** the speedup ratio is ≥ 1.10 under `Options::Default()`.
2. **Given** release-build benchmark results for UC7 (regex, compiled DFA, 100 MB corpus), **When** specialized execution time is compared to unspecialized, **Then** the speedup ratio is ≥ 1.10 under `Options::Default()`.
3. **Given** release-build benchmark results for UC1 (SQL predicate, 10 M rows), **When** specialized execution time is compared to unspecialized, **Then** the speedup ratio is ≥ 1.05 under `Options::Default()`.
4. **Given** any archetype where `specialized_exec_ns ≥ unspecialized_exec_ns`, **Then** this is documented in the benchmark results and the thesis frames it as a "break-even or no-benefit" case — not treated as a defect.

---

### User Story 3 — Integration with Spec 007 Pipeline Evaluation Tooling (Priority: P3)

The researcher can point `optimize_benchmarks.py` and the ablation harness from
spec 007 at each new benchmark binary exactly as they do for polybench and TPC-H.
This enables Pareto sweeps, ablation studies, and break-even analysis on the new
workloads without any tooling changes.

**Why this priority**: The value multiplies if the benchmarks plug into existing
analysis infrastructure. This is purely an integration requirement — the benchmarks
themselves (User Story 1) already deliver value without it.

**Independent Test**: Run `python optimize_benchmarks.py --db <path> --binary <uc1_bin> --study uc1_default` and verify it completes a trial without error.

**Acceptance Scenarios**:

1. **Given** a UC1 benchmark binary, **When** `optimize_benchmarks.py` runs a 5-trial study against it, **Then** the study completes without error and a `v_optim_best_per_kernel` entry is created for the `predicate` kernel.
2. **Given** all six benchmark binaries follow the same Google Benchmark output format as polybench/TPC-H, **Then** `record_benchmark.py` requires no code changes to ingest their output.

---

### Edge Cases

- A benchmark where JIT specialization makes execution *slower* than unspecialized (e.g., code bloat from unrolling): this is recorded as-is and discussed in the thesis as a "no-benefit" case; no corrective change to the benchmark code is required.
- A kernel where IPSCCP cannot propagate all specialization constants (e.g., the comparator function pointer address is not visible as a constant through IRDumpingPass): this is documented as a "partial specialization" case; the benchmark is still valid as a data point.
- The UC14 sort benchmark passing a function pointer: the IRDumpingPass blob captures the comparator function definition in the same TU; if the address is still treated as non-constant by IPSCCP the benchmark reports lower speedup, which is a valid and interesting finding.
- Benchmarks where the "fixed" parameter is a pointer to a large constant array (UC7 DFA table, UC2 kernel coefficients): the array must be a global in the kernel TU, not a stack allocation, so IRDumpingPass serializes it into the blob.

---

## Requirements *(mandatory)*

### Functional Requirements

**Benchmark Structure (all six use cases)**

- **FR-001**: Each benchmark MUST follow the TU separation pattern: the kernel function(s) live in a dedicated `*Kernels.cpp` file (no Google Benchmark headers, no `ClangRuntimeSpecializerBenchmark.h`); the benchmark registration and harness live in a separate `*Benchmark.cpp` file.
- **FR-002**: Each benchmark binary MUST register at least three measurement phases per benchmark variant: `unspecialized` (baseline, repeated across iterations), `jit_overhead` (first-call specialization cost, `Iterations(1)`), and `specialized_exec` (execution of the specialized result, `Iterations(N)`).
- **FR-003**: Each benchmark MUST follow the phase naming convention (`t:unspecialized`, `t:jit_overhead`, `t:specialized_exec`) required by `record_benchmark.py`. Because these benchmarks use the spec 010 `specializeLambda` API (via factory functions) rather than the `funcName`-based `benchmarkJITOverhead` helper, phases are registered manually with `->Name("BM_g:<group>;n:<name>;t:<phase>;")`. The naming convention and DuckDB schema compatibility are preserved.
- **FR-004**: The "constant" parameters for each kernel MUST be clearly commented in the `*Kernels.h` header as `// specialization constant` so it is evident which arguments are fixed at JIT time.
- **FR-005**: Each benchmark MUST compile successfully under the project's release build (`ninja -C llvm/build/release`) and pass `assertSpecializedLambdaIsEquivalent` for at least one test input before the benchmark runs (called via the factory function, comparing the unspecialized kernel result against the `SpecializedLambda` result).
- **FR-006**: Each benchmark binary MUST be registered in the project's CMake as a separate `add_benchmark` target following the pattern established by `polybench_bench` and `tpch_bench`. Additionally, all six kernel and benchmark object files MUST be added to the existing `AllBenchmarks` target so a single binary covers the full benchmark suite.
- **FR-007**: Each benchmark MUST expose four dataset size variants tagged `s:SMALL`, `s:MEDIUM`, `s:LARGE`, and `s:EXTRALARGE` in the benchmark name. The dataset sizes MUST be calibrated (by measuring actual unspecialized per-call runtime and adjusting until within a factor of 2× of the target) so that the unspecialized execution time per call approximates: SMALL ≈ 0.1 s, MEDIUM ≈ 1 s, LARGE ≈ 10 s, EXTRALARGE ≈ 60 s. Calibration MUST be performed after initial implementation by querying `t_unspec_ns` per `kv_s` label from the benchmarks database (using the same query pattern as `size_scaling.py`) and updating the size constants in the respective `*Benchmark.cpp` files.

**UC1 — SQL Expression Evaluation**

- **FR-010**: The kernel TU MUST implement a function `count_matching_rows(const uint8_t* rows, int64_t n_rows, int row_stride, int col_offset, double threshold)` that iterates over `n_rows` fixed-size records and counts rows where the `double` field at `col_offset` bytes into each record exceeds `threshold`.
- **FR-011**: The specialization constants MUST be `row_stride`, `col_offset`, and `threshold`; `rows` and `n_rows` are per-call variables.
- **FR-012**: The benchmark MUST use a synthetic dataset of ≥ 10 million rows with randomized field values; the same dataset MUST be reused across iterations (no re-generation in the hot loop).

**UC2 — Image Convolution with Fixed Kernel**

- **FR-013**: The kernel TU MUST implement `convolve2d(const float* src, float* dst, int width, int height, const float* kernel_coeffs, int ksize)` performing a **separable two-pass** 2D convolution (one horizontal 1D pass followed by one vertical 1D pass), so that specializing `ksize` eliminates O(ksize²) multiplications per pixel and allows the JIT to fully unroll both inner loops.
- **FR-014**: The specialization constants MUST be `kernel_coeffs` (pointer to a global coefficient array in the kernel TU), `ksize`, `width`, and `height`; `src` and `dst` are per-call variables.
- **FR-015**: The default benchmark instance MUST use a 5×5 Gaussian kernel and a 1920×1080 image. The kernel coefficients array MUST be a non-`static` global in the kernel TU so the IRDumpingPass blob serializes it.

**UC7 — NFA/DFA Regex Matching with Fixed Pattern**

- **FR-016**: The kernel TU MUST implement `dfa_match(const char* haystack, int64_t len, const int* dfa_table, int n_states, int n_chars, int start_state, int accept_mask)` performing DFA-based substring search.
- **FR-017**: The specialization constants MUST be `dfa_table` (pointer to a global DFA transition table), `n_states`, `n_chars`, `start_state`, and `accept_mask`; `haystack` and `len` are per-call variables.
- **FR-018**: The default benchmark instance MUST use a DFA compiled from the pattern `[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}` (email address) applied to a ≥ 50 MB synthetic text corpus. The DFA table MUST be a non-`static` global `int` array in the kernel TU, filled programmatically at binary startup by a builder function (e.g., invoked via `__attribute__((constructor))` or a static initializer object) — no external regex library is permitted in the kernel TU.

**UC8 — Incremental View Maintenance**

- **FR-019**: The kernel TU MUST implement `apply_row_delta(const uint8_t* row, double* agg_buckets, int n_buckets, int group_col_offset, int value_col_offset, int row_stride)` that updates a fixed-size SUM aggregate table given one incoming row.
- **FR-020**: The specialization constants MUST be `group_col_offset`, `value_col_offset`, `row_stride`, and `n_buckets`; `row` and `agg_buckets` are per-call variables.
- **FR-021**: The benchmark MUST process ≥ 10 million row delta events in the hot loop to make per-row overhead measurable; the aggregate bucket array MUST be pre-allocated and reused across iterations.

**UC12 — Columnar Analytics GROUP BY SUM**

- **FR-022**: The kernel TU MUST implement `grouped_sum(const uint8_t* rows, int64_t n_rows, int row_stride, int key_offset, int value_offset, double* out_buckets, int n_buckets)` that computes per-group SUMs in a single pass over a flat row array.
- **FR-023**: The specialization constants MUST be `row_stride`, `key_offset`, `value_offset`, and `n_buckets`; `rows`, `n_rows`, and `out_buckets` are per-call variables.
- **FR-024**: The benchmark MUST use a synthetic dataset of ≥ 10 million rows; the output bucket array MUST be zeroed between iterations to ensure correct aggregation.

**UC14 — Sort with Fixed Comparator**

- **FR-025**: The kernel TU MUST implement `generic_sort(void* data, int64_t n_elements, int element_size, int (*comparator)(const void*, const void*))` performing an in-place **quicksort with median-of-three pivot selection**, so the comparator appears in the partition hot path and its inlining (after JIT specialization) produces a measurable speedup.
- **FR-026**: The specialization constants MUST be `comparator` (a function pointer to a comparator defined in the same kernel TU) and `element_size`; `data` and `n_elements` are per-call variables.
- **FR-027**: The default benchmark instance MUST sort arrays of ≥ 1 million `int64_t` values using an ascending comparator. The array MUST be re-shuffled between iterations (using `std::shuffle` with a fixed seed) to ensure each call performs a real sort. The shuffle MUST be excluded from benchmark timing by wrapping it with `state.PauseTiming()` / `state.ResumeTiming()`.
- **FR-028**: The kernel TU MUST define the comparator function as a non-`static` non-`inline` symbol so the IRDumpingPass blob serializes it and IPSCCP can substitute it at JIT time.

### Key Entities

- **Kernel TU**: A `.cpp` file containing only the target function(s) and any globals they access. Compiled with `-fpass-plugin=IRDumpingPass.so` so the bitcode blob is embedded in the binary. Contains no benchmark infrastructure.
- **Benchmark TU**: A `.cpp` file that includes `ClangRuntimeSpecializerBenchmark.h` and the kernel's `.h` header, registers Google Benchmark cases, and calls the specialization helpers. Not compiled with IRDumpingPass.
- **Specialization constant**: A function argument whose value is fixed for a "session" (e.g., one compiled query, one image filter session) and therefore captured by the lambda passed to `specializeLambda`, becoming an LLVM IR constant in the JIT wrapper.
- **Benchmark variant**: A single named Google Benchmark instance (e.g., `BM_UC1_predicate_10M_unspecialized`), characterizing one combination of dataset size, constant set, and phase (unspecialized / jit_overhead / specialized_exec).

---

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: All six benchmark binaries compile without error under the release build and produce valid Google Benchmark output when run.
- **SC-002**: For each benchmark, `record_benchmark.py` successfully inserts at least one row per phase (`unspecialized`, `jit_overhead`, `specialized_exec`) into the DuckDB database.
- **SC-003**: At least two of the six benchmarks achieve a measured `specialized_exec_ns / unspecialized_exec_ns` ratio ≤ 0.90 (≥ 10% speedup) under `Options::Default()` in a release build on the development machine.
- **SC-004**: Each benchmark binary runs to completion (no crash, no sanitizer error in an asan/ubsan build) when called with the default benchmark filter.
- **SC-005**: `assertSpecializedLambdaIsEquivalent` passes for at least one test input per kernel via the factory function, confirming correctness before any performance claim is made.
- **SC-006**: The benchmarks are compatible with `optimize_benchmarks.py` without code changes to that tool (verified by running at least one 5-trial Optuna study per benchmark).

---

## Assumptions

- The project's existing build infrastructure (CMake, Nix flake, IRDumpingPass plugin) is stable and can accommodate new benchmark targets without structural changes.
- The development machine has ≥ 8 GB of free RAM for holding the 10 M-row synthetic datasets across all six benchmarks during a parallel benchmark run.
- Benchmark results are reproducible on the single development machine; portability to other hardware is out of scope (consistent with the hardware-specificity framing in spec 007, W2).
- The DuckDB benchmarks database schema introduced in earlier specs is not changed by this feature; new benchmark rows are compatible with existing views and queries.
- All six kernel functions are single-threaded (no concurrent argument mutation), satisfying the Specialization Scope Constraint in the project constitution.
- For UC14 (sort), IPSCCP's ability to substitute a function pointer as a constant depends on the pointer being visible as a global symbol in the JIT blob; if not fully propagated the benchmark still runs but produces lower speedup, which is a valid data point.
- The "fixed comparator" for UC14 is not the system's `qsort`-style comparator but a project-defined function in the kernel TU, so the function body is available in the IR blob.
