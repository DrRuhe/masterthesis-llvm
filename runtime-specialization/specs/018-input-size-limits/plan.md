# Plan 018: Input Size Limits — Implementation Checklist

## Prerequisites

- [ ] Read `.specify/memory/constitution.md` before starting any build changes
- [ ] Confirm `ninja check-smoke-runtime-specializer` passes (54 tests) on current tree

## Step 1 — Fix sqlite3 TU compilation with IRDumpingPass

- [ ] Locate where sqlite3 (amalgamation) is compiled in `benchmarks/tpch/CMakeLists.txt`
- [ ] Add `-fplugin=<IRDumpingPass.so>` (or the cmake equivalent) to sqlite3's compile flags,
      mirroring how other benchmark TUs are compiled with the plugin
- [ ] Rebuild and verify that `g_registered_blobs` is non-empty at runtime by running
      `AllBenchmarks --benchmark_filter=g:db/sqlite3/tpch;n:tpch_q1;t:jit_overhead` and
      confirming no `DumpedIRError` is thrown (a timeout is acceptable at this stage)
- [ ] Record the sqlite3 TU blob size (`jit_blob_kb`) from the first successful run

## Step 2 — Switch to lambda API (FR-002)

- [ ] In `sqlite3_tpch_bench.cpp`, replace `phaseJITOverhead`'s
      `RS->specializeOnly<int>(sqlite3VdbeExec, vdbe)` with:
      ```cpp
      auto q = [vdbe]() -> int { return sqlite3VdbeExec(vdbe); };
      benchmark::DoNotOptimize(RS->specializeOnly<int>(q));
      ```
- [ ] Apply the same lambda wrapping in `phaseSpecializedExec`
- [ ] Verify the existing TPC-H benchmarks still compile and the `jit_overhead` phase
      returns a result (pass or timeout) without exception

## Step 3 — Add nOp counters (FR-003)

- [ ] Confirm that `sqlite3_tpch_bench.cpp` includes the sqlite3 internal header (or the
      amalgamation header) needed to read `Vdbe::nOp` and `sizeof(VdbeOp)`; add the include
      if missing
- [ ] In `phaseJITOverhead`, after `stmt` is prepared and before specialization, add:
      ```cpp
      auto* vdbe = reinterpret_cast<Vdbe*>(stmt);
      state.counters["query_nops"] = (double)vdbe->nOp;
      state.counters["query_bytecode_bytes"] = (double)(vdbe->nOp * sizeof(VdbeOp));
      ```
- [ ] Rebuild and confirm the counters appear in `--benchmark_out` JSON for at least one query

## Step 4 — Add synthetic query files (FR-004)

- [ ] Create `benchmarks/tpch/queries/sqlite/simple_scan.sql`:
      `SELECT * FROM region`
- [ ] Create `benchmarks/tpch/queries/sqlite/filter_small.sql`:
      `SELECT * FROM region WHERE r_regionkey < 3`
- [ ] Create `benchmarks/tpch/queries/sqlite/filter_large.sql`:
      `SELECT * FROM lineitem WHERE l_quantity > 10`
- [ ] Create `benchmarks/tpch/queries/sqlite/agg_simple.sql`:
      `SELECT COUNT(*) FROM lineitem`
- [ ] Create `benchmarks/tpch/queries/sqlite/agg_group.sql`:
      `SELECT l_returnflag, COUNT(*) FROM lineitem GROUP BY l_returnflag`
- [ ] Create `benchmarks/tpch/queries/sqlite/join_simple.sql`:
      `SELECT o_orderkey, l_quantity FROM orders JOIN lineitem ON o_orderkey = l_orderkey LIMIT 1000`
- [ ] Create `benchmarks/tpch/queries/sqlite/join_agg.sql`:
      `SELECT o_custkey, SUM(l_extendedprice) FROM orders JOIN lineitem ON o_orderkey = l_orderkey GROUP BY o_custkey`
- [ ] Verify all 7 queries can be prepared with `sqlite3_prepare_v2` against the TPC-H DB
      (run a quick one-off check or temporary test binary)

## Step 5 — Register synthetic benchmarks in sqlite3_tpch_bench.cpp (FR-008)

- [ ] Extend the query-loading infrastructure to support named queries (not just q-numbered
      TPC-H queries): add a helper `getNamedQuery(const char* name)` that loads from
      `g_sqlite_queries_dir/<name>.sql`
- [ ] For each of the 7 synthetic queries, register four benchmark phases using the existing
      `phaseUnspecialized`, `phaseJITOverhead`, `phaseSpecializedExec`, `phaseJITAnalysis`
      helpers, with BM names following the pattern:
      `BM_g:db/sqlite3/input_size;n:<query_name>;t:<phase>;`
- [ ] Rebuild and run `--benchmark_filter=g:db/sqlite3/input_size;t:jit_overhead` to confirm
      all 7 queries execute and emit `query_nops` counters

## Step 6 — Add timeout guard (FR-005)

- [ ] Implement a wall-clock timeout wrapper in `phaseJITOverhead`: measure elapsed time
      around the `specializeOnly` call; if >300s, set `state.counters["jit_timeout"] = 1.0`
      and skip `state.SetIterationTime`, mirroring the pattern from `duckdb_tpch_bench.cpp`
- [ ] Verify the timeout counter appears correctly for a known-fast query

## Step 7 — Run full evaluation (FR-006)

- [ ] [complex] Run all synthetic queries + TPC-H Q1–Q22 under P0-optimal config
      (`Options` with `fixpointIter=16, unrollThreshold=62, largeModuleThreshold=3`);
      collect JSON output to `benchmarks/reports/<date>-sqlite3-input-size/p0_optimal.json`
- [ ] [complex] Run same query set under P2-optimal config (`uc_workload_optimal` params);
      collect to `benchmarks/reports/<date>-sqlite3-input-size/p2_optimal.json`
- [ ] Ingest both JSON files into `benchmarks.duckdb` using the existing ingestion pipeline
- [ ] Verify SC-003: query rows with `query_nops` are present in `benchmarks.duckdb`

## Step 8 — Visualization (FR-007)

- [ ] [complex] Write visualization script `benchmarks/reporting/plot_input_size_limits.py`
      that queries `benchmarks.duckdb` for all `g:db/sqlite3` `jit_overhead` rows and
      produces the scatterplot: x=`query_nops`, y=pass(1)/fail(0), marker by pipeline,
      with the sqlite3 TU blob size annotated as text; save as PNG
- [ ] Verify SC-004: script runs without error and PNG contains ≥5 data points

## Step 9 — Document findings

- [ ] For each failed query (exception or timeout), append a finding to
      `specs/018-input-size-limits/spec.md` under `## Clarifications` with: query name,
      nOp count, failure mode, and pipeline
- [ ] Update `specs/TODO.md` to mark this spec as complete and cross-reference the PNG

## Step 10 — Verification

- [ ] Verify SC-001: simple_scan and filter_small complete without `DumpedIRError`
- [ ] Verify SC-002: JSON output contains non-zero `query_nops` for all executed queries
- [ ] Verify SC-005: all failure modes are documented in the Clarifications section
