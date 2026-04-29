# Feature Specification: TPC-H Benchmark Groups (SQLite + DuckDB)

**Feature Branch**: `005-tpch-duckdb-benchmarks`
**Created**: 2026-04-29
**Status**: Draft

## User Scenarios & Testing *(mandatory)*

### User Story 1 — SQLite TPC-H benchmarks accessible under a namespaced group (Priority: P1)

A researcher running the benchmark suite needs all existing SQLite-backed TPC-H benchmarks to appear under the group `db/sqlite3/tpch` so that benchmark results can be filtered and compared by backend.

**Why this priority**: This is a prerequisite for adding the DuckDB group and for consistent naming across the `db/` benchmark namespace. Without it, results from the two backends cannot be meaningfully compared in the results database.

**Independent Test**: Can be fully tested by building and running the benchmark binary, filtering to `.*db/sqlite3/tpch.*`, and verifying that all TPC-H benchmark variants (all 22 queries across all phases) appear under the new group name.

**Acceptance Scenarios**:

1. **Given** the benchmark binary is built, **When** filtering to `.*db/sqlite3/tpch.*`, **Then** benchmark entries for all 22 TPC-H queries (Q1–Q22) across all phases (unspecialized, jit_overhead, specialized_exec, jit_analysis) appear in the output, with no `p1_specialized_exec` entries remaining.
2. **Given** the benchmark binary is built, **When** filtering to the old group name (plain `tpch`), **Then** no results match, confirming the rename is complete.
3. **Given** a benchmark run is recorded, **When** querying the results database for `kv_g = 'db/sqlite3/tpch'`, **Then** the run's benchmarks are found; the `db/sqlite3/tpch` group is distinct from `db/duckdb/tpch` in the results database.

---

### User Story 2 — DuckDB TPC-H benchmarks registered and runnable (Priority: P2)

A researcher wants to run all 22 TPC-H queries (Q1–Q22) against a DuckDB in-process database and collect the same benchmark phases (unspecialized, jit_overhead, specialized_exec, jit_analysis) as the SQLite group, so that the two backends can be compared side-by-side across the full query set.

**Why this priority**: The core deliverable of this feature. The DuckDB group must mirror the SQLite group's benchmark structure so that results are comparable across backends.

**Independent Test**: Can be fully tested by building and running the benchmark binary, filtering to `.*db/duckdb/tpch.*`, and verifying that all 22 queries each produce timing measurements for all phases without crashes.

**Acceptance Scenarios**:

1. **Given** a TPC-H sample database file exists on disk, **When** the `db/duckdb/tpch` benchmarks run, **Then** all 22 queries each produce valid timing results for the unspecialized phase (baseline execution via DuckDB's native query engine).
2. **Given** a TPC-H sample database file exists on disk, **When** the `db/duckdb/tpch` benchmarks run, **Then** the jit_overhead phase reports specialization cost and JIT module statistics (function count, instruction count, bitcode size) for each of the 22 queries.
3. **Given** a TPC-H sample database file exists on disk, **When** the `db/duckdb/tpch` benchmarks run, **Then** the specialized_exec phase reports execution time for the runtime-specialized query plan executor for each query, and results are correct (match unspecialized output for the same query and database).
4. **Given** a TPC-H sample database file exists on disk, **When** the jit_analysis phase runs, **Then** a pass-trace JSON file is produced per query, recording per-pass LLVM transformation statistics.

---

### User Story 3 — TPC-H sample database can be generated independently of the benchmark results database (Priority: P3)

A researcher setting up the benchmarking environment needs to generate a TPC-H sample database at a configurable scale factor. The TPC-H sample database is a separate file from the database used to store benchmark results, so that clearing or regenerating TPC-H data never affects benchmark history.

**Why this priority**: Operational correctness. Conflating the two databases would risk data loss; making generation self-contained lets users vary scale factors and regenerate without side effects.

**Independent Test**: Can be fully tested by running the generation script, verifying the TPC-H database file is created with the expected tables and row counts, and confirming the benchmark results database path is different.

**Acceptance Scenarios**:

1. **Given** DuckDB is available as a Python library, **When** the TPC-H data generation script runs with a given scale factor, **Then** a TPC-H database file is created at a configurable output path containing all required TPC-H tables (region, nation, part, supplier, partsupp, customer, orders, lineitem).
2. **Given** the TPC-H database file exists, **When** either the SQLite or DuckDB benchmark suite opens it, **Then** queries execute without errors and return non-empty results.
3. **Given** a benchmark run completes and results are recorded, **When** inspecting file paths, **Then** the TPC-H database file and the benchmark results database are distinct files that do not share a path.

---

### User Story 4 — Specialization targets the DuckDB query executor at the right point (Priority: P2)

A researcher wants to verify that runtime specialization is applied to the DuckDB query plan executor function at the point just before the prepared query is executed, so that the specialization captures the fully-prepared, query-specific execution state.

**Why this priority**: This is the key research question for the DuckDB group — confirming that specialization of a query engine's execution entry point yields measurable speedups comparable to the SQLite case.

**Independent Test**: Can be tested by inspecting the JIT module statistics from the jit_overhead phase: if specialization occurs after plan preparation, the IR blob should reflect query-specific structure (low function count after pruning relative to full library size).

**Acceptance Scenarios**:

1. **Given** a query has been prepared (plan compiled), **When** the jit_overhead benchmark phase runs, **Then** specialization is invoked with the prepared execution state as argument, and the JIT module function-count-after-prune metric is reported.
2. **Given** specialization has completed, **When** the specialized_exec phase runs, **Then** the specialized function is called in place of the unspecialized executor for that query.
3. **Given** any two distinct queries are benchmarked, **When** comparing their jit_analysis pass-traces, **Then** the IR after specialization differs between queries, confirming that query-specific constants are being propagated.

---

### Edge Cases

- What happens when the TPC-H database file is missing at benchmark startup? The benchmark binary must emit a clear error and skip the affected benchmarks rather than crashing.
- What happens if the DuckDB query preparation fails (e.g., schema mismatch)? The benchmark must report the error and not attempt specialization on a null or invalid execution state.
- What happens when scale factor 0 or a negative value is passed to the data generation script? The script must reject the input with a descriptive error.
- What happens if the TPC-H database is opened simultaneously by the SQLite benchmarks and the DuckDB benchmarks within the same AllBenchmarks binary run? They must not conflict (read-only access or separate open handles are acceptable).
- What happens if the JIT specialization step exceeds the configured timeout? The `jit_overhead` phase must report the timeout condition and complete without blocking indefinitely. The `specialized_exec` and `jit_analysis` phases for that query must be skipped, since they internally require specialization to complete first.

## Requirements *(mandatory)*

### Functional Requirements

**SQLite group rename**

- **FR-001**: The SQLite TPC-H benchmark group name MUST be `db/sqlite3/tpch` (parsed as `kv_g = 'db/sqlite3/tpch'` in the results database).
- **FR-002**: Benchmark phases (unspecialized, jit_overhead, specialized_exec, jit_analysis) MUST be present for all 22 TPC-H queries (Q1–Q22) under `db/sqlite3/tpch`. Any existing `p1_specialized_exec` phase benchmarks MUST be removed; pipeline configuration is controlled via environment variables (spec 003) and does not warrant a separate benchmark phase type.

**DuckDB benchmark group**

- **FR-003**: A benchmark group named `db/duckdb/tpch` MUST exist and MUST include the same benchmark phases as the SQLite group (unspecialized, jit_overhead, specialized_exec, jit_analysis) for all 22 TPC-H queries (Q1–Q22).
- **FR-004**: The DuckDB unspecialized phase MUST measure the wall-clock time for executing a fully-prepared query through DuckDB's native execution path, without any JIT specialization.
- **FR-005**: The DuckDB jit_overhead phase MUST invoke runtime specialization on the DuckDB query executor function, passing the fully-prepared query execution state as the specialization argument, and MUST record JIT module statistics (function count, instruction count, bitcode size, pruned function/instruction counts).
- **FR-006**: The DuckDB specialized_exec phase MUST invoke the runtime-specialized function in place of the unspecialized executor and MUST measure only the execution time of the specialized call. It MUST also compare the specialized result against the unspecialized result and record the maximum per-column absolute difference as a benchmark counter (`result_max_diff`), enabling divergence analysis in the results database.
- **FR-007**: The DuckDB jit_analysis phase MUST produce a pass-trace JSON file recording per-pass LLVM transformation statistics for a single specialization invocation.
- **FR-008**: The DuckDB benchmark TU (translation unit) containing the specialization target MUST be compiled with the IR-dumping compiler plugin so that a bitcode blob is embedded in the binary.
- **FR-009**: The DuckDB benchmark binary MUST link DuckDB as a shared or static library and MUST export symbols required for JIT symbol resolution.

**TPC-H sample database**

- **FR-010**: A data generation script MUST be provided that produces a TPC-H sample database at a user-specified scale factor.
- **FR-011**: The TPC-H sample database MUST contain all eight standard TPC-H tables: region, nation, part, supplier, partsupp, customer, orders, lineitem.
- **FR-012**: The TPC-H sample database path MUST be configurable at runtime (e.g., via a command-line flag or environment variable) and MUST NOT be hard-coded to the benchmark results database path.
- **FR-013**: The TPC-H sample database and the benchmark results database MUST be separate files. The data generation script MUST NOT write to the benchmark results database.
- **FR-014**: If the TPC-H sample database file is absent when a benchmark starts, the benchmark MUST emit an informative message and skip affected benchmarks cleanly (no crash, no silent wrong results).

**JIT timeout**

- **FR-017**: The JIT specialization step MUST enforce a configurable timeout (default: 60 seconds). If the timeout is exceeded, the `jit_overhead` phase MUST report the timeout condition gracefully (no hang, no crash) rather than blocking indefinitely.
- **FR-018**: If the `jit_overhead` phase for a given query times out, the `specialized_exec` and `jit_analysis` phases for that same query MUST be skipped. They MUST NOT attempt to execute, since doing so would require re-running the JIT (which already exceeded the timeout) and would block the benchmark run.

**AllBenchmarks integration**

- **FR-015**: Both `db/sqlite3/tpch` and `db/duckdb/tpch` benchmark groups MUST be included in the AllBenchmarks multi-benchmark binary.
- **FR-016**: The AllBenchmarks binary MUST accept a command-line flag to specify the TPC-H database path, forwarding it to both the SQLite and DuckDB benchmark modules.

### Key Entities

- **TPC-H Sample Database**: A database file containing the eight standard TPC-H tables at a configurable scale factor. Used exclusively as the query target by both benchmark groups. Distinct from the benchmark results database.
- **Benchmark Results Database**: The persistent store for benchmark run metadata and timing results. Never modified by TPC-H data generation.
- **Query**: One of the 22 standard TPC-H queries (Q1–Q22) expressed in SQL. Compiled into a prepared execution state before benchmarking.
- **Prepared Execution State**: The query-specific, fully-compiled execution artifact produced by the database engine after parsing and planning a query. The subject of runtime specialization.
- **Benchmark Phase**: One measurement variant within a benchmark group (unspecialized, jit_overhead, specialized_exec, jit_analysis). Each phase isolates a distinct aspect of performance.
- **Pass-Trace**: A per-pass record of LLVM IR transformation statistics collected during a single specialization invocation. Written to a JSON file for offline analysis.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: All 22 TPC-H benchmark phases are accessible under `db/sqlite3/tpch` and produce valid results.
- **SC-002**: The `db/duckdb/tpch` group produces valid timing measurements for all phases (unspecialized, jit_overhead, specialized_exec, jit_analysis) for all 22 TPC-H queries (Q1–Q22) without crashes or error exits.
- **SC-003**: The jit_overhead phase for each DuckDB query reports a non-zero bitcode size and a pruned function count smaller than the total function count, confirming that specialization and IR pruning are both active.
- **SC-004**: The specialized_exec phase for each DuckDB query produces a result that agrees with the unspecialized phase result within a relative floating-point tolerance (e.g., 1e-6). The maximum observed per-column difference MUST be recorded as a benchmark counter so divergence can be quantified in later analysis.
- **SC-005**: The TPC-H sample database generation script completes in under 60 seconds for scale factor 0.1 and produces a database with the correct schema.
- **SC-006**: The benchmark results database and the TPC-H sample database are distinct files; running the generation script does not alter the benchmark results database.
- **SC-007**: The AllBenchmarks binary, when run with `--benchmark_filter=.*db/(sqlite3|duckdb)/tpch.*`, completes all matched benchmarks without a non-zero exit code.

## Clarifications

### Session 2026-04-29

- Q: Should there be an upper bound on benchmark phase execution time, or on JIT compilation time? → A: No cap on query execution time (all 22 queries run to MinTime regardless of duration). The JIT specialization step must enforce a 60-second timeout. If `jit_overhead` hits the timeout, the corresponding `specialized_exec` and `jit_analysis` phases for that query must be skipped.
- Q: How should "numerically identical" in SC-004 be interpreted for floating-point aggregate results? → A: Within relative floating-point epsilon (e.g., 1e-6). The maximum per-column absolute difference between specialized and unspecialized results must be recorded as a `result_max_diff` benchmark counter for later divergence analysis.
- Q: Should the DuckDB group include a `p1_specialized_exec` phase to match the existing SQLite phase? → A: No — `p1_specialized_exec` is a leftover from testing an alternative pipeline configuration. Pipeline variants are now controlled via environment variables (spec 003). The phase must be removed from the SQLite group as part of this feature, and must not be added to the DuckDB group.

## Assumptions

- The DuckDB C++ API provides a stable entry point function for executing a fully-prepared query that can be targeted by runtime specialization (analogous to `sqlite3VdbeExec` for SQLite). Identifying the exact function is a planning-stage concern.
- The TPC-H data generation script uses DuckDB's built-in TPC-H extension (`dbgen()`) as the data source, consistent with the existing generation infrastructure.
- The same scale factor (0.1 by default) is used for both the SQLite and DuckDB TPC-H sample databases to keep results comparable, unless the user explicitly changes the scale.
- DuckDB can open a DuckDB-format database file (produced by the generation script) directly; no SQLite-to-DuckDB conversion step is required for the DuckDB benchmark group.
- The DuckDB benchmark TU follows the same TU-separation pattern as the SQLite group: the specialization target lives in a separate translation unit from benchmark registration code.
- Read-only access to the TPC-H sample database is sufficient; benchmarks do not write to it.
- All 22 standard TPC-H queries (Q1–Q22) are in scope for both benchmark groups. SQL definitions for queries not yet present in the repository must be added as part of this feature.
