# Tasks: TPC-H Benchmark Groups (SQLite + DuckDB)

**Input**: Design documents from `specs/005-tpch-duckdb-benchmarks/`
**Prerequisites**: plan.md ✅, spec.md ✅, research.md ✅, data-model.md ✅

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no shared dependencies)
- **[Story]**: User story this task belongs to (US1–US4)

---

## Phase 1: Foundational (Blocking Prerequisites)

**Purpose**: DuckDB CMake integration must be in place before any DuckDB benchmark code can be compiled or tested.

**⚠️ CRITICAL**: US2 cannot begin until T001 produces a working `duckdb_with_ir.o`.

- [X] T001 Add DuckDB v1.5.2 FetchContent declaration (URL + SHA256) and `duckdb_with_ir.o` `add_custom_command` (clang -x c++ -std=c++17 -O2 -fpass-plugin -fexceptions -frtti -fvisibility=default) to `benchmarks/tpch/CMakeLists.txt`; export `DUCKDB_IR_OBJ` and `DUCKDB_SRC_DIR` to PARENT_SCOPE

**Checkpoint**: `ninja DuckDBTpchBenchmark` (stub, pre-Phase-4) produces `duckdb_with_ir.o` without errors.

---

## Phase 2: User Story 1 — SQLite Group Rename + Query Expansion (Priority: P1) 🎯 MVP

**Goal**: All 22 TPC-H queries run under `db/sqlite3/tpch`; `p1_specialized_exec` phase removed.

**Independent Test**: `ninja TPCHBenchmark && ./TPCHBenchmark --benchmark_filter=".*db/sqlite3/tpch.*"` shows 22 × 4 phases; `--benchmark_filter=".*BM_g:tpch;.*"` matches nothing.

- [X] T002 [P] [US1] Create SQLite-compatible TPC-H SQL files `q02.sql`, `q04.sql`, `q05.sql` in `benchmarks/tpch/queries/` (Q2: min-cost supplier subquery; Q4: order priority with EXISTS; Q5: local supplier revenue multi-join; use `DATE(col, '+N months')` for date arithmetic, `STRFTIME('%Y', col)` for year extraction)
- [X] T003 [P] [US1] Create SQLite-compatible TPC-H SQL files `q07.sql`, `q08.sql`, `q09.sql`, `q10.sql`, `q13.sql` in `benchmarks/tpch/queries/` (multi-join + date + CASE queries; adapt `EXTRACT`/`INTERVAL` to SQLite equivalents)
- [X] T004 [P] [US1] Create SQLite-compatible TPC-H SQL files `q11.sql`, `q12.sql`, `q14.sql`, `q15.sql`, `q16.sql`, `q17.sql`, `q18.sql`, `q19.sql`, `q20.sql`, `q21.sql`, `q22.sql` in `benchmarks/tpch/queries/` (Q15 must inline the view as a subquery; Q22 uses `SUBSTR`; adapt date arithmetic throughout)
- [X] T005 [US1] Rename `benchmarks/tpch/tpch_bench.cpp` → `benchmarks/tpch/sqlite3_tpch_bench.cpp`; update every reference in `benchmarks/tpch/CMakeLists.txt` (`TPCH_OBJ`, `TPCH_IDE_STUB`, `TPCH_SRC_FILE`, `set_source_files_properties`, `add_benchmark`) and in `benchmarks/CMakeLists.txt` (`ALL_OBJ_TPCH` custom command `DEPENDS` and `-c` source argument)
- [X] T006 [US1] Replace every `"BM_g:tpch;` occurrence with `"BM_g:db/sqlite3/tpch;` in all `BENCHMARK(...)->Name(...)` calls in `benchmarks/tpch/sqlite3_tpch_bench.cpp` (currently 9 occurrences across Q1×4, Q6×3, Q3×3 after p1 removal)
- [X] T007 [US1] Remove `Pipeline1Opts()` helper function and all `BM_p1_specialized_exec_*` benchmark function definitions and `BENCHMARK(BM_p1_specialized_exec_*)` registration calls from `benchmarks/tpch/sqlite3_tpch_bench.cpp`
- [X] T008 [US1] Add `static constexpr const char*` SQL string constants `TPCH_Q2`, `TPCH_Q4`, `TPCH_Q5`, `TPCH_Q7`–`TPCH_Q22` (SQLite-adapted SQL from the `.sql` files created in T002–T004) to `benchmarks/tpch/sqlite3_tpch_bench.cpp`
- [X] T009 [US1] Add benchmark phase registrations (4 phases each: unspecialized `MinTime(1.0)`, jit_overhead `Iterations(1)`, specialized_exec `MinTime(1.0)`, jit_analysis `Iterations(1)->UseManualTime()`) for Q2, Q4, Q5, Q7–Q22 following the existing Q1 pattern in `benchmarks/tpch/sqlite3_tpch_bench.cpp`

**Checkpoint**: US1 fully functional and testable independently.

---

## Phase 3: User Story 2 — DuckDB TPC-H Benchmarks (Priority: P2)

**Goal**: All 22 TPC-H queries run under `db/duckdb/tpch` with 4 phases; JIT timeout enforced; result divergence recorded.

**Independent Test**: `ninja DuckDBTpchBenchmark && ./DuckDBTpchBenchmark --duckdb-db=tpch/data/tpch.duckdb --benchmark_filter=".*db/duckdb/tpch.*"` shows 22 × 4 phases; `jit_pruned_fns` < `jit_module_fns` for at least one query; no non-zero exit.

### JIT Timeout Infrastructure (prerequisite for T013–T014)

- [X] T010 [US2] Add `withJITTimeoutSeconds(unsigned)` fluent builder and `JITTimeoutSeconds` field to the `Options` struct in `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h`; implement wall-clock timeout enforcement in `specializeOnlyImpl` in `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.cpp` (run JIT in a `std::async` future; `wait_for` the timeout; if elapsed, return a null/error result and set a timeout flag); add a WIP test in `test/WIP/jit-timeout.cpp` that verifies the timeout is respected

### DuckDB Benchmark File

- [X] T011 [US2] Create `benchmarks/tpch/duckdb_tpch_bench.cpp`: add includes (`benchmark/benchmark.h`, `ClangRuntimeSpecializerBenchmark.h`, `ClangRuntimeSpecializer.h`, `duckdb.h`), `extern "C" duckdb_state duckdb_execute_prepared(...)` declaration, `inline constexpr char Fn_duckdb_execute_prepared[]`, `static duckdb_result g_duckdb_exec_result`, `static std::string g_duckdb_db_path = "tpch/data/tpch.duckdb"`, `#ifdef ALL_BENCHMARKS_BUILD` path setter, and IR embedding trigger (standalone + `_duckdb`-suffixed ALL_BENCHMARKS_BUILD variant) matching pattern in `sqlite3_tpch_bench.cpp`
- [X] T012 [P] [US2] Add `static constexpr const char*` SQL string constants `DUCKDB_Q1`–`DUCKDB_Q22` (standard TPC-H SQL — use native `INTERVAL '1 year'`, `EXTRACT(year FROM col)` forms) to `benchmarks/tpch/duckdb_tpch_bench.cpp`
- [X] T013 [P] [US2] Implement `openDuckDB()` (calls `duckdb_open`, exits with message on error) and `prepareQuery(duckdb_connection, const char*)` (calls `duckdb_prepare`, exits on error) helpers in `benchmarks/tpch/duckdb_tpch_bench.cpp`
- [X] T014 [US2] Implement `phaseUnspecialized(benchmark::State&, const char* sql)` for DuckDB: open DB + connection, prepare statement, loop calling `duckdb_execute_prepared(stmt, &result)` + `duckdb_destroy_result` between iterations, teardown on exit in `benchmarks/tpch/duckdb_tpch_bench.cpp`
- [X] T015 [US2] Implement `phaseJITOverhead(benchmark::State&, const char* sql)` for DuckDB: call `specializeOnly` with `Options::Default().withJITTimeoutSeconds(60)`, store timeout flag in a `static bool g_last_jit_timed_out`, record `jit_module_fns/instrs/blob_kb/pruned_fns/pruned_instrs` counters + `jit_timeout` counter (0 or 1), teardown in `benchmarks/tpch/duckdb_tpch_bench.cpp`
- [X] T016 [US2] Implement `phaseSpecializedExec(benchmark::State&, const char* sql)`: check `g_last_jit_timed_out` and skip (return early) per FR-018; compile specialized fn once, run in loop with `duckdb_destroy_result` between iterations; after loop run one unspecialized execution, compare numeric columns within 1e-6 relative tolerance, record `result_max_diff` counter as maximum per-column absolute difference; teardown in `benchmarks/tpch/duckdb_tpch_bench.cpp`
- [X] T017 [US2] Implement `phaseJITAnalysis(benchmark::State&, const char* sql)`: check `g_last_jit_timed_out` and skip per FR-018; call `clangRuntimeSpecializer::benchmarkJITAnalysis<Fn_duckdb_execute_prepared>(state, duckdb_execute_prepared, std::make_tuple(stmt, &g_duckdb_exec_result))`; teardown in `benchmarks/tpch/duckdb_tpch_bench.cpp`
- [X] T018 [US2] Add benchmark registrations for all 22 DuckDB queries × 4 phases (unspecialized `MinTime(1.0)`, jit_overhead `Iterations(1)`, specialized_exec `MinTime(1.0)`, jit_analysis `Iterations(1)->UseManualTime()`) with group `"db/duckdb/tpch"` in `benchmarks/tpch/duckdb_tpch_bench.cpp`
- [X] T019 [US2] Add `DuckDBTpchBenchmark` CMake target to `benchmarks/tpch/CMakeLists.txt`: `duckdb_tpch_bench.o` `add_custom_command` (clang c++17 -g -O3 -fpass-plugin -fexceptions -frtti, `DEPENDS ${DUCKDB_IR_OBJ}`), IDE stub, `add_benchmark(DuckDBTpchBenchmark ...)`, `target_include_directories` (includes `${duckdb_src_SOURCE_DIR}`), `target_link_libraries`, `ENABLE_EXPORTS`/`--export-dynamic`, and export `DUCKDB_TPCH_OBJ`, `DUCKDB_TPCH_SRC_FILE`, `DUCKDB_TPCH_SRC_DIR`, `DUCKDB_SRC_DIR`, `DUCKDB_IR_OBJ` to PARENT_SCOPE

**Checkpoint**: US2 fully functional. `jit_pruned_fns < jit_module_fns` confirmed for Q1. `result_max_diff` counter appears in benchmark output.

---

## Phase 4: User Story 3 — TPC-H Sample Database Generation (Priority: P3)

**Goal**: `generate.py` produces both `tpch.db` (SQLite) and `tpch.duckdb` (DuckDB) at configurable scale factor; neither file is the benchmark results database.

**Independent Test**: `python generate.py --sf 0.01 --out /tmp/t.db --duckdb-out /tmp/t.duckdb` creates both files; DuckDB file contains all 8 tables with non-zero row counts; existing `benchmarks.duckdb` is untouched.

- [X] T020 [US3] Add `--duckdb-out` argument (default: `data/tpch.duckdb`) to `benchmarks/tpch/data/generate.py`; implement DuckDB file generation using `duckdb.connect(duckdb_out)` + `db.execute(f"CALL dbgen(sf={sf})")` after the existing in-memory generation; preserve existing SQLite output path unchanged
- [X] T021 [P] [US3] Update `--help` / argparse descriptions in `benchmarks/tpch/data/generate.py` to explicitly state that `--out` is the SQLite TPC-H database (not the benchmark results database) and `--duckdb-out` is the DuckDB TPC-H database (also not the benchmark results database)

**Checkpoint**: US3 fully functional and independently testable.

---

## Phase 5: AllBenchmarks Integration

**Purpose**: Brings US1 and US2 together into the combined binary. Depends on Phase 2 (US1) and Phase 3 (US2) being complete.

- [X] T022 Add `void duckdb_tpch_set_db_path(const char* path);` forward declaration; add `--duckdb-db=` flag parsing (strips from argv, calls setter) alongside existing `--db=` handling in `benchmarks/AllBenchmarks_main.cpp`
- [X] T023 Add `ALL_OBJ_DUCKDB_TPCH` `add_custom_command` to `benchmarks/CMakeLists.txt` (mirrors `ALL_OBJ_TPCH` pattern: clang c++17 -g -O3 `-DALL_BENCHMARKS_BUILD=1`, includes `${DUCKDB_SRC_DIR}`, `DEPENDS ${DUCKDB_TPCH_SRC_FILE} ... ${DUCKDB_IR_OBJ}`)
- [X] T024 Add `${ALL_OBJ_DUCKDB_TPCH}` and `${DUCKDB_IR_OBJ}` to the `add_benchmark(AllBenchmarks ...)` source list; add `${DUCKDB_SRC_DIR}` to `target_include_directories(AllBenchmarks ...)` in `benchmarks/CMakeLists.txt`

**Checkpoint**: `ninja AllBenchmarks && ./AllBenchmarks --benchmark_filter=".*db/(sqlite3|duckdb)/tpch.*" --db=tpch/data/tpch.db --duckdb-db=tpch/data/tpch.duckdb` exits 0 with results for both groups.

---

## Phase 6: Polish & Verification

- [X] T025 Run `ninja check-smoke-runtime-specializer` from the debug build directory; confirm all 18 smoke tests remain green; fix any regressions before merging
- [X] T026 [P] Update `plan.md` under Phase 1 steps to record that `p1_specialized_exec` removal is part of this feature (not just a rename), and confirm the JIT timeout decision (library-level `withJITTimeoutSeconds`) in `specs/005-tpch-duckdb-benchmarks/plan.md`

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Foundational)**: No dependencies — start immediately
- **Phase 2 (US1)**: T002–T004 can start immediately in parallel (no dependency on Phase 1); T005–T009 need T002–T004 for SQL content but not T001
- **Phase 3 (US2)**: T010 needs Phase 1 (T001) complete; T011–T019 need T010 and T001
- **Phase 4 (US3)**: No dependency on Phase 1–3; can proceed in parallel with Phase 3
- **Phase 5 (AllBenchmarks)**: Needs Phase 2 (US1, for TPCH_SRC_FILE rename) and Phase 3 (US2, for DUCKDB_TPCH exports) complete
- **Phase 6 (Polish)**: Needs all phases complete

### User Story Dependencies

- **US1 (P1)**: Independent — depends only on SQL file creation (T002–T004)
- **US2 (P2)**: Depends on Phase 1 (T001 for `DUCKDB_IR_OBJ`) and T010 (JIT timeout option)
- **US3 (P3)**: Fully independent — can start at any time
- **US4 (P2)**: Acceptance criteria are verified through US2 tasks (T015 for stats, T016 for specialized fn invocation, T017 for per-query IR divergence via pass-traces) — no separate tasks needed

### Within Phase 3 (US2)

```
T010 → T011 → T012, T013 (parallel) → T014 → T015 → T016 → T017 → T018 → T019
```

### Parallel Opportunities

```
# Phase 2 (US1) — SQL files can be written simultaneously:
T002 (Q2/Q4/Q5)  ┐
T003 (Q7–Q10/Q13)├─ all parallel, different files
T004 (Q11–Q22)   ┘

# Phase 3 (US2) — after T010+T011:
T012 (SQL strings) ┐
T013 (helpers)     ┘ parallel, different functions in same file

# Phase 4 (US3):
T020, T021 — both modify generate.py sequentially; T021 is editorial only
```

---

## Implementation Strategy

### MVP (US1 only — Phase 2)

1. Complete Phase 1 (T001) — takes minutes
2. Complete Phase 2 / US1 (T002–T009) — SQLite fully renamed and expanded
3. **STOP and VALIDATE**: `ninja TPCHBenchmark` + filter test
4. Merge US1 independently if needed

### Full Delivery Order

1. Phase 1 (T001) — DuckDB CMake
2. Phase 2 (T002–T009) — US1 ← MVP delivery point
3. Phase 4 (T020–T021) — US3 (quick, independent)
4. Phase 3 (T010–T019) — US2 (main effort)
5. Phase 5 (T022–T024) — AllBenchmarks wiring
6. Phase 6 (T025–T026) — smoke tests + plan update

---

## Notes

- [P] tasks = independent files or non-conflicting sections; safe to run in parallel
- T010 adds to the public CRS API — per the constitution, a WIP test is mandatory before the feature is considered complete
- The JIT timeout (T010/T015) uses `withJITTimeoutSeconds(60)` at the call site; the default in `Options::Default()` should remain 0 (no timeout) to avoid breaking existing benchmarks
- `g_last_jit_timed_out` in T015 is a per-query static flag; reset at the start of each `phaseJITOverhead` invocation to avoid cross-query contamination
- `result_max_diff` (T016) compares the final-iteration specialized result against one unspecialized execution run after the timed loop; it is NOT in the hot path
- DuckDB `duckdb_execute_prepared` re-execution semantics must be verified during T014 implementation: confirm the same `prepared_statement` can be called repeatedly without a reset step
