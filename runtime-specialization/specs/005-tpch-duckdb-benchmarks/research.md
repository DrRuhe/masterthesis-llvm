# Research: TPC-H Benchmark Groups (SQLite + DuckDB)

**Phase 0 output for**: `specs/005-tpch-duckdb-benchmarks/plan.md`
**Date**: 2026-04-29

---

## R-01: DuckDB Distribution Format

**Decision**: Use the DuckDB amalgamation source (`libduckdb-src.zip`) downloaded via CMake `FetchContent` from GitHub Releases.

**Rationale**: The amalgamation (`duckdb.cpp` + `duckdb.hpp`) is a single C++ translation unit, exactly analogous to SQLite's `sqlite3.c`. It can be compiled directly with the IR-dumping plugin, embedding the entire DuckDB execution path as a bitcode blob that the JIT can inline and specialize. Prebuilt `.so` binaries cannot be compiled with the plugin and would leave DuckDB internals opaque to the IR optimizer.

**Release asset**:
- URL: `https://github.com/duckdb/duckdb/releases/download/v1.5.2/libduckdb-src.zip`
- SHA256: `36388f54d4e73c7148895f9b075c063189d47df8687db237f765f74a7ff5d8f6`
- Size: ~4.7 MB
- Contains: `duckdb.cpp`, `duckdb.hpp`

**Alternatives considered**:
- *FetchContent from git source*: DuckDB's CMake build is complex and slow (~20 min for a full build); the amalgamation is the officially supported single-file distribution for embedding.
- *System DuckDB package via flake.nix*: Possible, but doesn't solve IR visibility. The `pkgs.duckdb` package in nixpkgs provides a prebuilt `.so`, not source. Adding it would add a system dependency without enabling IR dumping.
- *Prebuilt `libduckdb-linux-amd64.zip`*: Provides `.so` + `duckdb.h` (C API header only). Not suitable for IR-dumping compilation.

**Compilation flags for `duckdb.cpp`**:
- Compile as C++17 (`-std=c++17`) — DuckDB requires it
- `-O2` (optimised IR before plugin runs, matching SQLite approach)
- `-fpass-plugin=${PLUGIN_LIB}` — embeds bitcode blob
- `-fexceptions -frtti` — DuckDB uses exceptions and RTTI internally
- `-DDUCKDB_NO_THREADS=1` — optional; disables multi-threaded execution to match single-threaded benchmark constraint from the constitution
- `-fvisibility=default` — ensures symbols are visible for JIT symbol resolution

---

## R-02: Specialization Target Function

**Decision**: Specialize `duckdb_execute_prepared` (C API function, defined in `duckdb.cpp`).

**Signature** (from `duckdb.h`):
```c
duckdb_state duckdb_execute_prepared(
    duckdb_prepared_statement prepared_statement,
    duckdb_result *out_result);
```

**Rationale**: This is the natural analog to `sqlite3VdbeExec(Vdbe* p)`:
- It is a C function (no name mangling, stable symbol name)
- `prepared_statement` is an opaque pointer to the compiled query plan — specializing on it bakes the query-specific execution context as a constant into the JIT-compiled IR
- The function is defined in the amalgamation `duckdb.cpp`, so the full DuckDB execution chain is visible to the IR optimizer in the blob
- IPSCCP can propagate `prepared_statement` as a constant pointer into the pipeline executor, physical operator tree, and expression evaluators, enabling constant folding and devirtualization (via IRDumpingPass vtable BFS)

**Result storage strategy** (see R-03).

**Alternatives considered**:
- *`PipelineExecutor::Execute`*: Deeper in the execution stack, would yield more optimization leverage, but is a C++ virtual method whose name is mangled and whose API is internal/unstable. Requires identifying the correct specialization point per query type (scan vs. join vs. aggregation), which varies per query.
- *Thin C wrapper in a separate kernels TU*: Define `int tpch_duckdb_exec(duckdb_prepared_statement)` that calls `duckdb_execute_prepared(stmt, &g_result)`. Clean interface but the wrapper and `duckdb_execute_prepared` body live in different blobs; the JIT cannot inline across blobs during the current single-blob-clone specialization path. Eliminates the IR visibility advantage.
- *`duckdb_query` (non-prepared)*: Re-parses and re-plans the query every call; not useful for measuring execution speedup of a fixed plan.

---

## R-03: Fixed Result Storage for Specialization

**Decision**: Use a module-level static `duckdb_result g_duckdb_exec_result` variable whose address is baked into the specialization alongside the prepared statement pointer.

**Rationale**: `duckdb_execute_prepared` writes results via an output pointer. Since `specializeOnly` bakes ALL arguments as constants, both `prepared_statement` and `out_result` must have fixed addresses. For benchmark purposes this is correct:
- `g_duckdb_exec_result` is a global; its address is stable for the entire process lifetime
- Between loop iterations, call `duckdb_destroy_result(&g_duckdb_exec_result)` to free column data (the struct itself stays at the same address)
- Each `duckdb_execute_prepared` call replaces the column data pointers inside the struct at the fixed address

**Correctness note**: DuckDB re-executes the full query plan on each call to `duckdb_execute_prepared` — there is no "reset" step. This must be verified against the DuckDB v1.5.2 C API semantics during implementation.

---

## R-04: SQL Queries Q1–Q22 (Standard TPC-H)

**Decision**: Add all 22 standard TPC-H queries as separate `.sql` files under `benchmarks/tpch/queries/` and embed them as C++ raw string literals in the benchmark source files.

**SQLite compatibility notes**: SQLite does not support all SQL features used in the standard TPC-H specification:
- Queries using `INTERSECT` / `EXCEPT` (Q3 alternative form, Q22) may need rewriting as NOT IN / NOT EXISTS
- `CASE WHEN` is supported
- Correlated subqueries are supported
- `INTERVAL` arithmetic is not supported; use `DATE(col, '+N months')` form
- Q11, Q16, Q22 use `NOT IN (SELECT ...)` which is supported

Queries that need DuckDB-only syntax will use standard SQL (DuckDB supports the full TPC-H benchmark query set natively).

**Currently existing queries**: Q1 (inline string in `tpch_bench.cpp`), Q3 (inline string), Q6 (inline string). Files `queries/q01.sql`, `queries/q03.sql`, `queries/q06.sql` also exist.

**Action required during implementation**: Add `q02.sql`, `q04.sql`, `q05.sql`, `q07.sql`–`q22.sql` with standard TPC-H SQL, adapted to SQLite where necessary for the SQLite group. DuckDB group uses standard SQL unchanged.

---

## R-05: TPC-H Database Format for DuckDB Group

**Decision**: Use a native DuckDB database file `tpch.duckdb` (distinct from the SQLite `tpch.db`) for the DuckDB benchmark group. Both are distinct from the benchmark results database (`benchmarks.duckdb`).

**Generation**: Extend the existing `data/generate.py` script to also write a DuckDB-format database by saving the in-memory DuckDB instance used for `dbgen()` to a file path. The Python `duckdb` package is already in `flake.nix`.

**Path default**: `"tpch/data/tpch.duckdb"` (relative to benchmark binary). Overridden at runtime via `--duckdb-db=<path>` flag.

**Three distinct files**:
| File | Format | Purpose |
|------|--------|---------|
| `tpch/data/tpch.db` | SQLite | TPC-H query target for `db/sqlite3/tpch` |
| `tpch/data/tpch.duckdb` | DuckDB | TPC-H query target for `db/duckdb/tpch` |
| `benchmarks/benchmarks.duckdb` | DuckDB | Benchmark results (never touched by tpch queries) |

---

## R-06: TU Separation Strategy for DuckDB

**Decision**: Follow the existing SQLite TU separation pattern:
- `duckdb.cpp` (amalgamation, compiled with plugin) = the blob TU containing `duckdb_execute_prepared`
- `duckdb_tpch_bench.cpp` = benchmark registration TU (also compiled with plugin for the IR embedding trigger, but does not define the specialization target)

**Rationale**: `duckdb.cpp` does not include any benchmark infrastructure headers (`ClangRuntimeSpecializerBenchmark.h`, `benchmark/benchmark.h`). Its blob is free of benchmark library vtables, avoiding the "Symbols not found" JIT crash documented in the constitution's TU separation pattern. This mirrors exactly how `sqlite3.c` is kept separate from `tpch_bench.cpp`.

---

## R-07: flake.nix — No Changes Required

**Decision**: No `flake.nix` changes needed for DuckDB C++ library.

**Rationale**:
- DuckDB C++ is fetched via CMake `FetchContent` at build time (amalgamation source) — no system dependency
- Python `duckdb` is already in `flake.nix` (line 199: `python-pkgs.duckdb`) — needed for `generate.py`

---

## R-08: AllBenchmarks Command-Line Interface

**Decision**: Add a separate `--duckdb-db=<path>` flag to `AllBenchmarks_main.cpp` alongside the existing `--db=<path>` flag (which routes to SQLite).

**Rationale**: The SQLite and DuckDB databases are different file formats at different paths. A single shared flag would require implicit path derivation rules (error-prone). Two explicit flags are clearer and follow the established `--db=` precedent.

**Interface after this feature**:
```
./AllBenchmarks --db=path/to/tpch.db --duckdb-db=path/to/tpch.duckdb
```

Defaults (if flags omitted):
- SQLite: `"tpch/data/tpch.db"`
- DuckDB: `"tpch/data/tpch.duckdb"`
