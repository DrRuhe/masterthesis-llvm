# Spec 018: Input Size Limits for JIT Specialization

## Overview

This spec investigates at which input scale JIT specialization breaks down, using sqlite3 as
the primary vehicle. sqlite3's `sqlite3VdbeExec` is compiled from a single, very large TU
(the canonical "amalgamation"), making it a natural stress test for the two input-size
dimensions of interest: (1) the bitcode blob size of the compiled TU, and (2) the size of
the specialization input — measured here as the number of VDBE bytecode ops (`nOp`) in the
prepared query. The research question is: **at what query bytecode size does specialization
become infeasible, and does the pipeline (P0 vs P2) change that threshold?**

The end result is a scatterplot: x-axis = query bytecode size (nOps), y-axis = pass/fail,
one series per pipeline configuration, providing a clear cliff-visualization for the thesis
discussion of specialization limits.

## User Scenarios

**Scenario A — Fix and run sqlite3 benchmarks**
Given the sqlite3 TPC-H benchmark binary is built,
When the JIT overhead benchmark runs for any query,
Then it completes (or times out cleanly) without throwing a `ClangRuntimeSpecializerDumpedIRError`.

**Scenario B — Graduated query complexity**
Given a set of synthetic SQL queries ranging from trivial to TPC-H-level complexity,
When each is run under P0-optimal and P2-optimal configurations,
Then the benchmark reports JIT overhead and nOp count for each query.

**Scenario C — Scatterplot**
Given all benchmark results collected in `benchmarks.duckdb`,
When the visualization script is run,
Then a scatterplot is produced with nOps on the x-axis, pass/fail on the y-axis, and
separate markers for P0 and P2 configurations.

## Functional Requirements

**FR-001** — sqlite3.c (the amalgamation) must be compiled with the IRDumpingPass plugin so
that `sqlite3VdbeExec` appears in a registered bitcode blob at runtime.

**FR-002** — The sqlite3 benchmark must specialize `sqlite3VdbeExec` via the lambda API:
```cpp
auto q = [vdbe]() -> int { return sqlite3VdbeExec(vdbe); };
auto spec = RS->specializeOnly<int>(q);
```
The lambda captures `vdbe` as a compile-time constant (bytecode identity); the mutable
execution state inside Vdbe flows through the same pointer across `sqlite3_reset()` calls.

**FR-003** — The `jit_overhead` benchmark phase must report two new counters per query:
- `query_nops`: `vdbe->nOp` (number of bytecode ops in the prepared statement)
- `query_bytecode_bytes`: `vdbe->nOp * sizeof(VdbeOp)` (total bytecode size in bytes)

These counters must appear in the Google Benchmark JSON output and be ingested into
`benchmarks.duckdb` by the existing pipeline.

**FR-004** — A graduated set of synthetic SQL queries must be added, covering at least the
following complexity levels, all using the existing TPC-H schema and database:

| ID | SQL template | Complexity |
|----|-------------|------------|
| simple_scan | `SELECT * FROM region` | trivial full scan, no filter |
| filter_small | `SELECT * FROM region WHERE r_regionkey < 3` | filter on small table |
| filter_large | `SELECT * FROM lineitem WHERE l_quantity > 10` | filter on large table |
| agg_simple | `SELECT COUNT(*) FROM lineitem` | single-pass aggregate |
| agg_group | `SELECT l_returnflag, COUNT(*) FROM lineitem GROUP BY l_returnflag` | grouped aggregate |
| join_simple | `SELECT o_orderkey, l_quantity FROM orders JOIN lineitem ON o_orderkey = l_orderkey LIMIT 1000` | two-table join |
| join_agg | `SELECT o_custkey, SUM(l_extendedprice) FROM orders JOIN lineitem ON o_orderkey = l_orderkey GROUP BY o_custkey` | join + aggregate |

Plus the 22 existing TPC-H queries (tpch_q01 … tpch_q22) which represent TPC-H-level complexity.

**FR-005** — JIT overhead benchmarks must run with a 300-second wall-clock timeout. A query
is classified as **failed** if it throws any exception OR if the JIT overhead exceeds 300s.
No correctness checks are performed against reference results.

**FR-006** — Two pipeline configurations must be tested:
- **P0-optimal**: `Options::FromExpectedRuntime` or the Optuna-derived optimal P0 config
  (`fixpointIter=16, unrollThreshold=62, largeModuleThreshold=3`). Label: `p0_optimal`.
- **P2-optimal**: The best P2 config from `corpus_uc_final_20260610`
  (`uc_workload_optimal`). Label: `p2_optimal`.

Both configs must be exercised for every query in FR-004.

**FR-007** — A visualization script must produce a scatterplot:
- x-axis: `query_nops` (log scale if range spans >2 orders of magnitude)
- y-axis: binary pass (1) / fail (0), with jitter to separate overlapping points
- Marker shape or color: P0-optimal vs P2-optimal
- Each query is one dot per pipeline config
- The TU bitcode blob size (KB) must appear as a text annotation on the plot (single value,
  since all sqlite3 queries share the same TU)

**FR-008** — The existing benchmark infrastructure in `sqlite3_tpch_bench.cpp` must be
extended in-place (not replaced) to host the new synthetic queries. No new binary target is
required; the existing `TPCHBenchmark` or `AllBenchmarks` build can include the new queries.

## Success Criteria

**SC-001** — `AllBenchmarks --benchmark_filter=g:db/sqlite3` completes without throwing
`ClangRuntimeSpecializerDumpedIRError` for at least the simple synthetic queries (FR-004
levels simple_scan through agg_simple).

**SC-002** — JSON output for the `jit_overhead` phase of every executed query contains
non-zero `query_nops` and `query_bytecode_bytes` counter values.

**SC-003** — `benchmarks.duckdb` contains rows for all executed queries with correct nOp
counter values, ingestible by the existing reporting pipeline.

**SC-004** — The scatterplot visualization script runs without error and produces a PNG
with at least 5 data points covering at least 3 distinct nOp magnitudes.

**SC-005** — For any query that fails (exception or timeout), the failure mode is documented
in the spec's Clarifications section as a thesis-ready finding.

## Edge Cases

- **sqlite3 amalgamation size**: The sqlite3.c amalgamation is ~230k lines. Compiling it
  with the IRDumpingPass may produce a very large bitcode blob (likely several MB). The
  existing large-module threshold logic must be verified to not silently skip
  `StaticMutabilityAnalysis` (see constitution: `JITPipelineIPSCCP.cpp` guard was already
  removed).

- **sqlite3_reset() and !invariant.load**: After specialization, calling `sqlite3_reset()`
  and then the specialized lambda is safe only if P2 does not fold loads from the mutable
  Vdbe execution-state fields. If SIGSEGV is observed during `specialized_exec`, this is
  a P2 soundness failure on the sqlite3 TU and must be documented (not necessarily fixed).

- **nOp access**: `vdbe->nOp` requires access to sqlite3's internal `Vdbe` struct layout.
  The benchmark TU must include the sqlite3 internal header (or the amalgamation) to read
  `nOp` and `sizeof(VdbeOp)` — not just the public `sqlite3.h`.

- **Query compilation failures**: Some TPC-H queries use SQL features that may not be
  prepareable on the SF=0.1 database. A `sqlite3_prepare_v2` failure must be caught and
  reported as a configuration error, not a specialization failure.

- **Timeout implementation**: Google Benchmark's `Iterations(1)` + `UseManualTime()` pattern
  requires the JIT call to be wrapped with a wall-clock timer and early-exit if 300s is
  exceeded. The existing `jit_timeout` counter pattern (from DuckDB bench) should be reused.

## Clarifications

*To be appended as findings emerge during implementation.*
