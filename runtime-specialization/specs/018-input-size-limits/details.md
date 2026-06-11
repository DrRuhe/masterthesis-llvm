# Details 018: Input Size Limits

## Key Design Decisions

### Lambda API vs direct function pointer

The benchmark switches from `RS->specializeOnly<int>(sqlite3VdbeExec, vdbe)` to:
```cpp
auto q = [vdbe]() -> int { return sqlite3VdbeExec(vdbe); };
RS->specializeOnly<int>(q);
```
The IR-level behavior is identical — `vdbe` is a compile-time constant pointer in both
cases. The lambda form is preferred because it expresses the semantic intent clearly in the
thesis: the bytecode (captured) is constant; the mutable Vdbe execution state (inside the
pointed-to struct, reset by `sqlite3_reset()`) is runtime data.

### nOp access requires internal Vdbe header

`sqlite3.h` (the public header) exposes `sqlite3_stmt*` but not `Vdbe*` internals. To read
`vdbe->nOp` and `sizeof(VdbeOp)`, the benchmark must include `vdbe.h` or compile against
the amalgamation's internal definitions. The existing bench already defines `struct Vdbe;`
as an opaque forward declaration — this is insufficient for field access. The fix is to
include the relevant internal sqlite3 header, or to add a small accessor function compiled
in the same TU as sqlite3 itself.

### TU size is fixed across all queries

All sqlite3 queries share the same compiled TU (the amalgamation). `jit_blob_kb` will be
identical for every query row. This value should be annotated once on the scatterplot rather
than plotted as an axis. The scatterplot's x-axis is solely `query_nops`.

### Timeout implementation

Mirrors the pattern from `benchmarks/tpch/duckdb_tpch_bench.cpp`. The `jit_overhead` phase
uses `Iterations(1)->UseManualTime()`. Wrap the `specializeOnly` call:
```cpp
auto t0 = std::chrono::steady_clock::now();
auto spec = RS->specializeOnly<int>(q);
auto elapsed = std::chrono::steady_clock::now() - t0;
double elapsed_s = std::chrono::duration<double>(elapsed).count();
state.SetIterationTime(elapsed_s);
state.counters["jit_timeout"] = (elapsed_s >= 300.0 || !spec) ? 1.0 : 0.0;
```

### P0-optimal and P2-optimal configuration parameters

From `corpus_uc_final_20260610` results (see memory):
- **P0-optimal**: `fixpointIter=16, unrollThreshold=62, largeModuleThreshold=3`
  → construct via `Options::Default().withMaxFixpointIterations(16).withUnrollThreshold(62).withLargeModuleThreshold(3)`
- **P2-optimal** (`uc_workload_optimal`): the Optuna P2 config with `unrolling=5` (from spec 007 results)
  → retrieve exact parameter values from `benchmarks/reports/260610-corpus-final/`

### Benchmark naming convention

New synthetic queries use group `db/sqlite3/input_size` to separate them from the TPC-H
group `db/sqlite3/tpch`. This allows independent filtering and ingestion.

### Query file naming

Stored under `benchmarks/tpch/queries/sqlite/<name>.sql`. Using the existing queries
directory avoids CMake changes; the new `getNamedQuery()` helper loads by name rather than
by TPC-H query number.

### Scatterplot pass/fail definition

A data point is **pass** (`jit_timeout == 0` AND no exception thrown). A data point is
**fail** if `jit_timeout == 1` OR if the benchmark row is absent (exception caused early
exit). The visualization script must treat missing rows as failures.

## Gotchas

- **Amalgamation compile time**: compiling sqlite3.c with `-O2 -fplugin=IRDumpingPass.so`
  will be slow (~minutes). This is a one-time cost per build.
- **Large blob, GlobalDCE pruning**: the sqlite3 blob may be 5–10MB. `IRDumpingPass`'s
  early GlobalDCE prune is critical to reduce what gets JIT-compiled. The pruned counts
  (`jit_pruned_fns`, `jit_pruned_instrs`) should be checked to confirm pruning is effective.
- **`StaticMutabilityAnalysis` guard already removed**: constitution note: the
  `if (!IsLargeModule)` guard on `StaticMutabilityAnalysis` was removed in commit
  `6f1b06925ae7`. This means P2 will always annotate `!invariant.load` correctly even for
  the large sqlite3 TU.
- **P2 SIGSEGV risk**: sqlite3 contains complex pointer aliasing patterns. P2 IPSCCP may
  fold loads from Vdbe fields that are mutated between calls. If `specialized_exec` crashes,
  document as a P2 soundness limitation on this TU (do not attempt to fix — it is a thesis
  finding).
