# Feature Specification: O3 Compilation Mode for Tests and Benchmarks

**Feature Branch**: `012-o3-compilation-mode`  
**Created**: 2026-05-16  
**Status**: Draft  
**Input**: User description: "Ensure that all the tests and benchmarks work in -O0 but especially make sure they work in -O3. Since we are doing runtime specialization to increase speed it's unfair to compare with -O0 compiled baselines. To make sure that the comptime plugin works as expected, maybe we need to split the rewriting functionality from the IRDumpingPass into a separate pass to ensure the functions are correctly resolved."

## Clarifications

### Session 2026-05-16

- Q: Should FR-006/007 be made unconditional (split is a definite deliverable, not a contingency)? → A: Yes, unconditional. Research confirmed root cause: `IRDumpingPass` at `registerOptimizerLastEPCallback` runs after -O3 breaks call-site traversal. `IRRewritingPass` at pipeline start is the definite fix.
- Q: Should FR-004 be corrected from "UC1–UC7" to list the actual implemented UCs? → A: Yes — updated to UC1, UC2, UC7, UC8, UC12, UC14 (matching CMakeLists.txt).
- Q: Should SC-004 specify a minimum speedup threshold (e.g., ≥10%) for -O3 baseline vs -O0? → A: Skip quantification — SC-004 remains intentionally qualitative; threshold is left to researcher judgment and documented in the thesis narrative.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Benchmarks Run with -O3 Baselines (Priority: P1)

A researcher running benchmarks wants to compare the performance of JIT-specialized code against statically compiled -O3 baselines. Currently benchmarks are compiled with -O0, meaning the "unspecialized" baseline does not represent the best possible static compilation. This leads to unfair comparisons: JIT specialization appears to win even when a well-optimized static binary would be equally fast or faster.

**Why this priority**: The scientific validity of the thesis depends on comparing against the strongest possible baseline. -O0 benchmarks overstate the advantage of JIT specialization and cannot support a sound research conclusion.

**Independent Test**: Can be tested by verifying that benchmark executables are built with `-O3` and produce measurable baseline numbers that represent peak static performance. A comparison run should show the JIT-specialized version either competitive with or faster than the -O3 baseline.

**Acceptance Scenarios**:

1. **Given** a benchmark use-case (e.g., UC1, UC2, UC7), **When** the benchmark binary is compiled with `-O3` and run, **Then** the unspecialized baseline reports execution times representative of optimized static code.
2. **Given** an -O3 compiled benchmark, **When** the JIT-specialized variant is run for the same workload, **Then** the result is reported alongside the -O3 baseline, enabling a fair scientific comparison.
3. **Given** a benchmark that previously passed at -O0, **When** recompiled at -O3 with the comptime plugin active, **Then** the plugin-injected IR blob is still correctly generated and the specialized path executes without errors.

---

### User Story 2 - Tests Pass at Both -O0 and -O3 (Priority: P2)

A developer making changes to the runtime specializer wants confidence that the correctness tests pass under both optimization levels. Some failures that occur only under -O3 (e.g., function inlining eliminating the symbol the plugin needs to intercept, or the compiler resolving calls at compile time) would otherwise go undetected.

**Why this priority**: Correctness under -O3 is a prerequisite for any performance claim. If the system only works at -O0, the thesis results are not reproducible in realistic conditions.

**Independent Test**: Running the full smoke test suite (`ninja check-smoke-runtime-specializer`) compiled at both `-O0` and `-O3` both pass without failures.

**Acceptance Scenarios**:

1. **Given** any smoke test, **When** compiled with `-O0 -fpass-plugin=...`, **Then** the test passes as today.
2. **Given** any smoke test, **When** compiled with `-O3 -fpass-plugin=...`, **Then** the test passes with the same observable output as the -O0 version.
3. **Given** a test that exercises the comptime plugin (IR blob injection), **When** compiled at -O3, **Then** the plugin correctly instruments the translation unit and the runtime loads the correct IR blob.

---

### User Story 3 - IRDumpingPass Split into Rewriting + Dumping Passes (Priority: P3)

A developer diagnosing why the comptime plugin does not correctly resolve functions at -O3 needs a cleaner separation of concerns in the pass pipeline. Currently `IRDumpingPass` both rewrites the IR (linkage fixups, vtable BFS, ctor/dtor erasure) and serializes the result to a bitcode blob. When rewriting and serialization are interleaved, debugging which transformation caused a resolution failure is difficult, and the -O3 optimizer may reorder or eliminate constructs before the pass runs.

**Why this priority**: The split is the confirmed fix for -O3 failures (root cause: `IRDumpingPass` runs after -O3 optimisations, breaking call-site traversal). It is a definite deliverable. It is prioritized below end-to-end -O3 test correctness because the split is a prerequisite for, not a consequence of, -O3 tests passing.

**Independent Test**: Can be tested by verifying that the two-pass pipeline produces an identical bitcode blob to the single-pass pipeline on a known input, and that the comptime plugin still instruments translation units correctly.

**Acceptance Scenarios**:

1. **Given** a translation unit compiled with the refactored plugin, **When** the IR rewriting pass runs first, **Then** all linkage fixups, vtable BFS, and ctor/dtor erasures are applied to the module before serialization.
2. **Given** the rewritten module, **When** the IR dumping pass runs second, **Then** the serialized bitcode blob is identical (or semantically equivalent) to what the old single-pass pipeline produced.
3. **Given** the refactored plugin, **When** used at -O3, **Then** functions that were previously incorrectly resolved (e.g., inlined away before the pass runs) are now correctly intercepted and included in the blob.

---

### Edge Cases

- What happens when a function is fully inlined by the -O3 optimizer before the comptime plugin pass runs, leaving no call site or definition to intercept?
- How does the system handle cases where -O3 promotes local variables to constants, potentially changing the set of functions the plugin decides to include in the blob?
- What happens when a test uses `FileCheck` patterns that match -O0 IR output that differs structurally from -O3 IR output (e.g., different function attribute sets, inlined callees)?
- How does the pass ordering interact with LTO or link-time optimization steps that could further transform the IR after the plugin has run?

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: All existing smoke tests MUST pass when compiled with `-O0` (no regression from current behaviour).
- **FR-002**: All existing smoke tests MUST pass when compiled with `-O3 -fpass-plugin=...` with the comptime plugin active.
- **FR-003**: The comptime plugin MUST correctly generate the IR blob (via `clang_runtime_specializer_register_blob_v2`) at -O3, such that the runtime can locate all expected target functions at JIT time.
- **FR-004**: All implemented benchmark use-cases (UC1, UC2, UC7, UC8, UC12, UC14, and any Polybench/TPC-H variants) MUST be compilable and executable with their host binary (kernel TUs) built at `-O3`.
- **FR-005**: Benchmark output MUST report unspecialized baseline timings that reflect -O3 static performance, enabling fair comparison with JIT-specialized timings.
- **FR-006**: The IR rewriting logic MUST be extracted from `IRDumpingPass` into a new `IRRewritingPass` that is registered at pipeline start (before any -O3 optimisations). `IRDumpingPass` retains only the cloning, preprocessing, and serialisation steps.
- **FR-007**: The two-pass pipeline MUST produce a bitcode blob that is semantically equivalent to the single-pass pipeline for all existing test inputs.
- **FR-008**: The lit test configuration MUST support selecting the compilation optimization level (defaulting to -O0 for existing tests, with -O3 variants added for the new O3-mode tests).
- **FR-009**: `FileCheck` patterns in tests that are sensitive to optimization level MUST be parameterized or duplicated to accommodate structural IR differences between -O0 and -O3 output.

### Key Entities

- **Comptime Plugin** (`IRDumpingPass` / `RuntimeSpecializerPass`): The Clang pass-plugin responsible for rewriting and serializing IR at compile time. The entity that must function correctly at all optimization levels.
- **Smoke Test Suite**: The set of lit/FileCheck tests under `test/smoke/` that define correctness. The pass criterion for this feature.
- **Benchmark Use-Case**: A benchmark executable (UC1, UC2, UC7, UC8, UC12, UC14, TPC-H, Polybench) that exercises the JIT specializer. Must be compilable at -O3.
- **IR Blob**: The bitcode embedded in the host binary by the comptime plugin. Its correctness at -O3 is the central invariant.
- **Rewriting Pass** (proposed): A new pass split from `IRDumpingPass` that applies all IR transformations (linkage fixups, vtable BFS, ctor/dtor erasure) without serializing.
- **Dumping Pass** (proposed): A new pass split from `IRDumpingPass` that serializes the already-rewritten module to a bitcode blob and emits the registration call.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: 100% of existing smoke tests pass at `-O0` (zero regressions from current baseline).
- **SC-002**: 100% of existing smoke tests pass at `-O3` after implementation (or, where structural IR differences make exact FileCheck matching impossible, equivalent -O3 variants of affected tests pass).
- **SC-003**: All benchmark use-cases complete a full benchmark run at `-O3` without crashes, assertion failures, or JIT-link errors.
- **SC-004**: The unspecialized baseline reported by benchmarks at `-O3` is measurably faster than the equivalent `-O0` baseline for compute-intensive kernels (quantitative confirmation that -O3 represents a stronger baseline).
- **SC-005**: The bitcode blob produced by the two-pass (`IRRewritingPass` + `IRDumpingPass`) pipeline is semantically equivalent to the old single-pass pipeline for every existing test input (verified by `assertSpecializedIsEquivalent` or identical instruction counts).
- **SC-006**: No new public API surface is introduced (the split is internal to the comptime plugin with no observable API change for users of `ClangRuntimeSpecializer.h`).

## Assumptions

- The root cause of -O3 failures is confirmed: `IRDumpingPass` is registered at `registerOptimizerLastEPCallback`, so it runs after -O3 optimisations have already modified the IR shapes the pass relies on for call-site traversal. The two-pass split (FR-006) is the confirmed fix, not a contingency.
- The runtime JIT pipeline is unaffected by host compilation flags (it operates on the fixed bitcode blob); all -O3 failures are in the comptime plugin, not the runtime.
- Tests that currently have `-O0` hard-coded in their `RUN:` lines will need individual `-O3` `RUN:` lines added; a global lit substitution to toggle all tests to -O3 is not assumed.
- Polybench and TPC-H benchmark hosts are assumed to be compilable at -O3 without unrelated build errors (e.g., no third-party dependency that fails to compile at -O3).
- Link-time optimization (LTO) is out of scope; only per-TU `-O3` compilation is targeted.
- The feature does not change the JIT optimization pipeline (the IR transform pipeline inside the runtime); only the host compilation mode changes.
