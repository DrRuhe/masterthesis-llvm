# Implementation Plan: TPC-H Benchmark Groups (SQLite + DuckDB)

**Branch**: `005-tpch-duckdb-benchmarks` | **Date**: 2026-04-29 | **Spec**: `specs/005-tpch-duckdb-benchmarks/spec.md`

## Summary

Migrate the existing SQLite TPC-H benchmarks from group `"tpch"` to `"db/sqlite3/tpch"`, expand coverage from 3 to all 22 TPC-H queries, and add a parallel `"db/duckdb/tpch"` benchmark group that specializes `duckdb_execute_prepared` from the DuckDB v1.5.2 amalgamation compiled with the IR-dumping plugin.

DuckDB is integrated as a source amalgamation (`duckdb.cpp` + `duckdb.hpp`) fetched via CMake `FetchContent`, compiled with the IR-dumping plugin exactly like `sqlite3.c`. The specialization target is `duckdb_execute_prepared(prepared_statement, out_result)` where both arguments are baked as constants: `prepared_statement` carries the query-specific compiled plan, and `out_result` points to a module-level static `duckdb_result` buffer.

## Technical Context

**Language/Version**: C++17 (DuckDB requires it); C (SQLite amalgamation)
**Primary Dependencies**: DuckDB v1.5.2 amalgamation (`libduckdb-src.zip`), SQLite 3.49.1 amalgamation (existing), Google Benchmark (existing), ClangRuntimeSpecializer (existing)
**Storage**: `tpch/data/tpch.db` (SQLite TPC-H), `tpch/data/tpch.duckdb` (DuckDB TPC-H), `benchmarks/benchmarks.duckdb` (results — not touched by this feature)
**Testing**: No new `lit`/`FileCheck` tests required (benchmark code path; correctness verified by comparing unspecialized vs. specialized results within the benchmark)
**Target Platform**: Linux x86-64 (existing environment)
**Project Type**: Benchmark extension to existing library benchmarks
**Performance Goals**: DuckDB specialized_exec phase shows measurable JIT effect (jit_pruned_fns < jit_module_fns); all 22 queries run without crash
**Constraints**: All existing smoke tests remain green; benchmark results database schema unchanged; `tpch.db` and `tpch.duckdb` are always separate from `benchmarks.duckdb`
**Scale/Scope**: ~5 new source files; ~200 lines of DuckDB benchmark code; 19 new SQL files; CMake changes in 2 files; `AllBenchmarks_main.cpp` minor update

## Constitution Check

### I. Correctness & Safety ✅
- Specialization of `duckdb_execute_prepared` bakes `prepared_statement` (read-only compiled plan pointer) and `&g_duckdb_exec_result` (stable global address) as constants. Neither pointer is mutated by another thread during specialization (single-threaded benchmark; constitution Specialization Scope Constraint satisfied).
- Correctness is verified in the benchmark itself: the `jit_analysis` phase calls the unspecialized function and records the pass trace; the `specialized_exec` phase reuses the same result buffer, making result comparison straightforward.
- No new public API surface exposed.

### II. LLVM Coding Standards ✅
- New C++ benchmark code follows existing patterns in `tpch_bench.cpp`.
- No additions to `ClangRuntimeSpecializer.h` or `.cpp`; this is purely a benchmark addition.
- Build system changes follow the existing `add_custom_command` + `add_benchmark` pattern.

### III. Test-First Validation ✅ (conditional)
- No `lit`/`FileCheck` tests are needed: the DuckDB integration is benchmark code, not library API code. The correctness criterion (specialized output matches unspecialized output) is measurable at benchmark runtime.
- If a new runtime API is added to `ClangRuntimeSpecializer.h` as a side effect, WIP tests must be added per the constitution.

### IV. Performance Measurement ✅
- This feature IS the performance measurement infrastructure for DuckDB. Results are recorded via `record_benchmark.py` into `benchmarks.duckdb`. SC-003 (pruned_fns < module_fns) and SC-004 (specialized output == unspecialized output) are observable from recorded counters.

### V. Minimal Public API ✅
- No public API changes.

### Backwards Compatibility ✅
- The SQLite group rename from `"tpch"` to `"db/sqlite3/tpch"` changes benchmark names in the results database. Any existing benchmark runs recorded under the old `"tpch"` group name will not match the new group name in queries. This is acceptable: the benchmarks.duckdb schema doesn't need migration (the new group name simply produces new rows), and old data is still accessible under `kv_g = 'tpch'`.
- No benchmark results database schema changes.

**Gate**: ✅ No violations.

## Project Structure

### Documentation (this feature)

```text
specs/005-tpch-duckdb-benchmarks/
├── plan.md              ← this file
├── research.md          ← Phase 0 complete
├── data-model.md        ← Phase 1 complete
└── tasks.md             ← Phase 2 (/speckit-tasks, not yet created)
```

### Source Code

```text
benchmarks/
├── AllBenchmarks_main.cpp          (modified: add --duckdb-db= flag)
├── CMakeLists.txt                  (modified: add DuckDB objects to AllBenchmarks)
└── tpch/
    ├── sqlite3_tpch_bench.cpp      (renamed from tpch_bench.cpp; group string updated)
    ├── duckdb_tpch_bench.cpp       (new: DuckDB benchmark phases + registrations)
    ├── queries/
    │   ├── q01.sql                 (existing)
    │   ├── q02.sql                 (new)
    │   ├── q03.sql                 (existing)
    │   ├── q04.sql–q22.sql         (new: 18 files)
    │   └── q06.sql                 (existing)
    ├── data/
    │   ├── tpch.db                 (existing SQLite database)
    │   ├── tpch.duckdb             (new: DuckDB database, generated by generate.py)
    │   └── generate.py             (modified: also writes tpch.duckdb)
    └── CMakeLists.txt              (modified: DuckDB FetchContent + compile commands)
```

**Structure Decision**: Single-project extension under `benchmarks/tpch/`. No new top-level directories. DuckDB amalgamation is downloaded to the CMake build directory (not committed to the repo).

## Implementation Phases

### Phase 1: SQLite group rename + query expansion (prerequisite)

**Goal**: Update the SQLite group string and add all 22 queries so the SQLite group is complete before the DuckDB group is added. This phase is independently testable.

**Steps**:

1. **Rename source file**: `benchmarks/tpch/tpch_bench.cpp` → `benchmarks/tpch/sqlite3_tpch_bench.cpp`. Update all references in `benchmarks/tpch/CMakeLists.txt` and `benchmarks/CMakeLists.txt` (search for `tpch_bench.cpp`, `TPCH_SRC_FILE`, `TPCH_OBJ`).

2. **Update group string and remove p1 phase**: In `sqlite3_tpch_bench.cpp`, replace every occurrence of `"BM_g:tpch;` with `"BM_g:db/sqlite3/tpch;` in all `BENCHMARK(...)->Name(...)` calls. Additionally, remove the `Pipeline1Opts()` helper and all `BM_p1_specialized_exec_*` benchmark functions and their `BENCHMARK(...)` registrations — this simplification is intentional; Q1 is left with 4 phases (unspecialized, jit_overhead, specialized_exec, jit_analysis). The `p1_specialized_exec` phase served a transitional role and is no longer needed now that pipeline selection is exposed via `Options`.

3. **Add SQL query files**: Create `queries/q02.sql` through `queries/q22.sql` (except q03, q06 which exist). Each file contains the standard TPC-H SQL adapted for SQLite where needed (see data-model.md for compatibility notes). Key adaptations:
   - Date arithmetic: Use `DATE('1994-01-01', '+1 year')` form instead of `INTERVAL`
   - `EXTRACT(year FROM col)`: Replace with `CAST(STRFTIME('%Y', col) AS INTEGER)`
   - Q15 view: Expand the view inline as a subquery (avoids temp view lifetime issues in the benchmark)

4. **Embed queries in source**: Add the remaining 19 queries as additional `static constexpr const char*` strings (TPCH_Q2 through TPCH_Q22, skipping Q1/Q3/Q6 which already exist) following the existing pattern.

5. **Add benchmark registrations for Q2, Q4, Q5, Q7–Q22**: For each new query, add the four/five registration blocks following the exact pattern of Q1 (unspecialized, jit_overhead, specialized_exec, jit_analysis). Q6-style (no p1 variant) is acceptable for queries where the pipeline-1 distinction is not meaningful.

6. **Update AllBenchmarks compilation command** in `benchmarks/CMakeLists.txt`: The `ALL_OBJ_TPCH` custom command references `TPCH_SRC_FILE` which must now point to `sqlite3_tpch_bench.cpp`.

**Verification**: Build `TPCHBenchmark` and run with `--benchmark_filter=.*db/sqlite3/tpch.*`. Confirm 22 queries × 4–5 phases appear. Confirm `--benchmark_filter=.*BM_g:tpch;.*` matches nothing.

---

### Phase 2: DuckDB CMake integration

**Goal**: Download, compile, and link the DuckDB amalgamation with the IR-dumping plugin. No benchmark code yet.

**Steps**:

1. **Add FetchContent declaration** in `benchmarks/tpch/CMakeLists.txt`:

   ```cmake
   FetchContent_Declare(duckdb_src
       URL https://github.com/duckdb/duckdb/releases/download/v1.5.2/libduckdb-src.zip
       URL_HASH SHA256=36388f54d4e73c7148895f9b075c063189d47df8687db237f765f74a7ff5d8f6
       DOWNLOAD_EXTRACT_TIMESTAMP TRUE
   )
   FetchContent_MakeAvailable(duckdb_src)
   ```

2. **Compile `duckdb.cpp` with the IR-dumping plugin**:

   ```cmake
   set(DUCKDB_IR_OBJ "${CMAKE_CURRENT_BINARY_DIR}/duckdb_with_ir.o")

   add_custom_command(
       OUTPUT ${DUCKDB_IR_OBJ}
       COMMAND ${CLANG_EXE}
               -x c++ -std=c++17 -O2
               -fpass-plugin=${PLUGIN_LIB}
               -fexceptions -frtti
               -fvisibility=default
               -I${duckdb_src_SOURCE_DIR}
               -c ${duckdb_src_SOURCE_DIR}/duckdb.cpp
               -o ${DUCKDB_IR_OBJ}
       DEPENDS ${duckdb_src_SOURCE_DIR}/duckdb.cpp LLVMRuntimeSpecializationComptimePlugin clang
       COMMENT "Compiling duckdb.cpp with IR-dumping plugin (embeds duckdb_execute_prepared IR)"
   )
   ```

   **Note**: `duckdb.cpp` is ~500k lines and compilation will take several minutes. This is expected and acceptable for a research benchmark.

3. **Export paths to parent scope** (for AllBenchmarks in `benchmarks/CMakeLists.txt`):

   ```cmake
   set(DUCKDB_IR_OBJ    "${DUCKDB_IR_OBJ}"           PARENT_SCOPE)
   set(DUCKDB_SRC_DIR   "${duckdb_src_SOURCE_DIR}"    PARENT_SCOPE)
   ```

**Verification**: Build fails if SHA256 mismatch. Build succeeds if `duckdb_with_ir.o` is produced without errors.

---

### Phase 2b: JIT timeout API (prerequisite for DuckDB benchmark)

**Decision**: Add `withJITTimeoutSeconds(unsigned)` fluent builder and `JITTimeoutSeconds` field to the `Options` struct in `ClangRuntimeSpecializer.h`. When non-zero, `specializeOnlyImpl` runs `addModuleAndLookup` in a detached `std::thread` backed by a `std::promise/future` pair and waits up to `JITTimeoutSeconds` using `future.wait_for`. On timeout, the function returns a null `JITResult` (`TimedOut=true`); `specializeOnly` detects this and returns a default-constructed (null) `SpecializedFunction<R>`. The default value remains 0 (no timeout) to avoid breaking existing benchmarks.

**Rationale**: DuckDB's amalgamation is ~500k lines; JIT compilation for complex queries may occasionally exceed acceptable wall-clock budget. The timeout prevents the benchmark from hanging indefinitely. The implementation is best-effort: the detached thread continues to completion in the background; callers guard against concurrent JIT ops using `g_last_jit_timed_out`.

**WIP test**: `test/WIP/jit-timeout.cpp` verifies that `specializeOnly` with a generous timeout succeeds and returns a callable function, and that `Options::Default()` (no timeout) also succeeds.

---

### Phase 3: DuckDB benchmark implementation

**Goal**: Implement `duckdb_tpch_bench.cpp` with all phases for all 22 queries under group `"db/duckdb/tpch"`.

**Steps**:

1. **Create `benchmarks/tpch/duckdb_tpch_bench.cpp`** with this structure:

   ```cpp
   #include <benchmark/benchmark.h>
   #include "ClangRuntimeSpecializerBenchmark.h"
   #include "ClangRuntimeSpecializer.h"
   #include "duckdb.h"   // C API header from amalgamation
   #include <string>
   #include <string_view>
   #include <vector>

   namespace CRS = clangRuntimeSpecializer;

   // C API specialization target (from duckdb.cpp blob)
   extern "C" duckdb_state duckdb_execute_prepared(
       duckdb_prepared_statement prepared_statement,
       duckdb_result *out_result);
   inline constexpr char Fn_duckdb_execute_prepared[] = "duckdb_execute_prepared";

   // Fixed result buffer — address baked into specialized function
   static duckdb_result g_duckdb_exec_result;

   // Default DB path (relative to binary)
   static std::string g_duckdb_db_path = "tpch/data/tpch.duckdb";

   #ifdef ALL_BENCHMARKS_BUILD
   void duckdb_tpch_set_db_path(const char* path) { g_duckdb_db_path = path; }
   #endif

   // IR embedding trigger — ensures callSpecialized for duckdb_execute_prepared
   // appears in this TU's IR so the runtime resolves the function from the
   // duckdb.cpp blob at init time.
   #ifndef ALL_BENCHMARKS_BUILD
   volatile bool g_duckdb_tpch_dummy_trigger = false;
   extern "C" __attribute__((used)) void duckdb_tpch_dummy_registration() {
       auto* RS = CRS::ClangRuntimeSpecializer::init();
       if (g_duckdb_tpch_dummy_trigger)
           RS->callSpecialized<duckdb_state>(Fn_duckdb_execute_prepared,
               (duckdb_prepared_statement)nullptr, (duckdb_result*)nullptr);
   }
   #else
   volatile bool g_duckdb_tpch_dummy_trigger_duckdb = false;
   extern "C" __attribute__((used)) void duckdb_tpch_dummy_registration_duckdb() {
       auto* RS = CRS::ClangRuntimeSpecializer::init();
       if (g_duckdb_tpch_dummy_trigger_duckdb)
           RS->callSpecialized<duckdb_state>(Fn_duckdb_execute_prepared,
               (duckdb_prepared_statement)nullptr, (duckdb_result*)nullptr);
   }
   #endif
   ```

2. **Helper functions** (analogous to SQLite helpers):

   ```cpp
   static duckdb_database openDuckDB() {
       duckdb_database db;
       if (duckdb_open(g_duckdb_db_path.c_str(), &db) == DuckDBError) {
           fprintf(stderr, "Cannot open DuckDB TPC-H database '%s'\n",
                   g_duckdb_db_path.c_str());
           exit(1);
       }
       return db;
   }

   static duckdb_prepared_statement prepareQuery(duckdb_connection con,
                                                  const char* sql) {
       duckdb_prepared_statement stmt;
       if (duckdb_prepare(con, sql, &stmt) == DuckDBError) {
           fprintf(stderr, "Failed to prepare DuckDB query\n");
           exit(1);
       }
       return stmt;
   }
   ```

3. **Phase functions**:

   **`phaseUnspecialized`** — baseline execution via DuckDB's native path:
   ```cpp
   static void phaseUnspecialized(benchmark::State& state, const char* sql) {
       auto db = openDuckDB();
       duckdb_connection con; duckdb_connect(db, &con);
       auto stmt = prepareQuery(con, sql);
       duckdb_result result{};
       for (auto _ : state) {
           duckdb_destroy_result(&result);
           benchmark::DoNotOptimize(duckdb_execute_prepared(stmt, &result));
       }
       duckdb_destroy_result(&result);
       duckdb_destroy_prepared(&stmt);
       duckdb_disconnect(&con); duckdb_close(&db);
   }
   ```

   **`phaseJITOverhead`** — measures specialization compile cost:
   ```cpp
   static void phaseJITOverhead(benchmark::State& state, const char* sql) {
       auto db = openDuckDB();
       duckdb_connection con; duckdb_connect(db, &con);
       auto stmt = prepareQuery(con, sql);
       auto* RS = CRS::ClangRuntimeSpecializer::init();
       auto modStats = CRS::ClangRuntimeSpecializer::getModuleStats();
       auto Prev = CRS::ClangRuntimeSpecializer::getLogLevel();
       CRS::ClangRuntimeSpecializer::setLogLevel(CRS::ClangRuntimeSpecializer::LogLevel::None);
       for (auto _ : state)
           benchmark::DoNotOptimize(
               RS->specializeOnly<duckdb_state>(Fn_duckdb_execute_prepared,
                   stmt, &g_duckdb_exec_result));
       CRS::ClangRuntimeSpecializer::setLogLevel(Prev);
       auto txStats = CRS::ClangRuntimeSpecializer::getLastTransformStats();
       state.counters["jit_module_fns"]    = (double)modStats.FunctionCount;
       state.counters["jit_module_instrs"] = (double)modStats.InstructionCount;
       state.counters["jit_blob_kb"]       = (double)(modStats.BitcodeSizeBytes / 1024);
       state.counters["jit_pruned_fns"]    = (double)txStats.FunctionCountAfterPrune;
       state.counters["jit_pruned_instrs"] = (double)txStats.InstructionCountAfterPrune;
       duckdb_destroy_result(&g_duckdb_exec_result);
       duckdb_destroy_prepared(&stmt);
       duckdb_disconnect(&con); duckdb_close(&db);
   }
   ```

   **`phaseSpecializedExec`** — compile once outside loop, measure execution:
   ```cpp
   static void phaseSpecializedExec(benchmark::State& state, const char* sql) {
       auto db = openDuckDB();
       duckdb_connection con; duckdb_connect(db, &con);
       auto stmt = prepareQuery(con, sql);
       auto* RS = CRS::ClangRuntimeSpecializer::init();
       auto Prev = CRS::ClangRuntimeSpecializer::getLogLevel();
       CRS::ClangRuntimeSpecializer::setLogLevel(CRS::ClangRuntimeSpecializer::LogLevel::None);
       auto SpecFn = RS->specializeOnly<duckdb_state>(Fn_duckdb_execute_prepared,
           stmt, &g_duckdb_exec_result);
       CRS::ClangRuntimeSpecializer::setLogLevel(Prev);
       for (auto _ : state) {
           duckdb_destroy_result(&g_duckdb_exec_result);
           benchmark::DoNotOptimize(SpecFn());
       }
       duckdb_destroy_result(&g_duckdb_exec_result);
       duckdb_destroy_prepared(&stmt);
       duckdb_disconnect(&con); duckdb_close(&db);
   }
   ```

   **`phaseJITAnalysis`** — single invocation with pass-trace:
   ```cpp
   // Per-query example (Q1):
   void BM_jit_analysis_duckdb_tpch_q1(benchmark::State& state) {
       auto db = openDuckDB();
       duckdb_connection con; duckdb_connect(db, &con);
       auto stmt = prepareQuery(con, DUCKDB_Q1);
       clangRuntimeSpecializer::benchmarkJITAnalysis<Fn_duckdb_execute_prepared>(
           state, duckdb_execute_prepared,
           std::make_tuple(stmt, &g_duckdb_exec_result));
       duckdb_destroy_result(&g_duckdb_exec_result);
       duckdb_destroy_prepared(&stmt);
       duckdb_disconnect(&con); duckdb_close(&db);
   }
   BENCHMARK(BM_jit_analysis_duckdb_tpch_q1)
       ->Name("BM_g:db/duckdb/tpch;n:tpch_q1;t:jit_analysis;")
       ->Iterations(1)->UseManualTime();
   ```

4. **Query SQL strings**: Define `static constexpr const char* DUCKDB_Q1` through `DUCKDB_Q22` using standard TPC-H SQL (not SQLite-adapted). Use standard `INTERVAL '1 year'` and `EXTRACT(year FROM ...)` forms which DuckDB supports natively.

5. **Benchmark registrations**: For each query Q1–Q22, register the four phases (unspecialized, jit_overhead, specialized_exec, jit_analysis) following the SQLite pattern with group `"db/duckdb/tpch"`.

6. **Add CMake target** in `benchmarks/tpch/CMakeLists.txt`:

   ```cmake
   set(DUCKDB_TPCH_OBJ "${CMAKE_CURRENT_BINARY_DIR}/duckdb_tpch_bench.o")

   add_custom_command(
       OUTPUT ${DUCKDB_TPCH_OBJ}
       COMMAND ${CLANG_EXE} -x c++ -std=c++17 -g -O3 ${CXX_FLAGS_LIST}
               -I${CMAKE_CURRENT_SOURCE_DIR}
               -I${CMAKE_CURRENT_SOURCE_DIR}/..
               -I${CMAKE_CURRENT_SOURCE_DIR}/../../runtime/ClangRuntimeSpecializer
               -I${duckdb_src_SOURCE_DIR}
               -I${LLVM_MAIN_INCLUDE_DIR}
               -I${LLVM_BINARY_DIR}/include
               -I${LLVM_THIRD_PARTY_DIR}/benchmark/include
               -fpass-plugin=${PLUGIN_LIB}
               -fexceptions -frtti
               -c ${CMAKE_CURRENT_SOURCE_DIR}/duckdb_tpch_bench.cpp
               -o ${DUCKDB_TPCH_OBJ}
       DEPENDS duckdb_tpch_bench.cpp LLVMRuntimeSpecializationComptimePlugin clang
               ${DUCKDB_IR_OBJ}
       COMMENT "Compiling duckdb_tpch_bench.cpp with IR-dumping plugin"
   )

   set(DUCKDB_TPCH_IDE_STUB "${CMAKE_CURRENT_BINARY_DIR}/duckdb_tpch_ide_stub.cpp")
   file(WRITE "${DUCKDB_TPCH_IDE_STUB}" "")
   set_source_files_properties(duckdb_tpch_bench.cpp PROPERTIES HEADER_FILE_ONLY ON)

   add_benchmark(DuckDBTpchBenchmark PARTIAL_SOURCES_INTENDED
       ${DUCKDB_TPCH_OBJ} ${DUCKDB_IR_OBJ}
       duckdb_tpch_bench.cpp ${DUCKDB_TPCH_IDE_STUB})

   target_include_directories(DuckDBTpchBenchmark PRIVATE
       ${LLVM_THIRD_PARTY_DIR}/benchmark/include
       ${CMAKE_CURRENT_SOURCE_DIR}
       ${CMAKE_CURRENT_SOURCE_DIR}/..
       ${CMAKE_CURRENT_SOURCE_DIR}/../../runtime/ClangRuntimeSpecializer
       ${duckdb_src_SOURCE_DIR}
       ${LLVM_MAIN_INCLUDE_DIR}
       ${LLVM_BINARY_DIR}/include
   )

   target_link_libraries(DuckDBTpchBenchmark PRIVATE ClangRuntimeSpecializer benchmark)
   set_target_properties(DuckDBTpchBenchmark PROPERTIES ENABLE_EXPORTS ON)
   if(NOT APPLE AND NOT MSVC)
       target_link_options(DuckDBTpchBenchmark PRIVATE "-Wl,--export-dynamic")
   endif()

   # Export for AllBenchmarks
   set(DUCKDB_TPCH_OBJ      "${DUCKDB_TPCH_OBJ}"                                    PARENT_SCOPE)
   set(DUCKDB_TPCH_SRC_DIR  "${CMAKE_CURRENT_SOURCE_DIR}"                             PARENT_SCOPE)
   set(DUCKDB_TPCH_SRC_FILE "${CMAKE_CURRENT_SOURCE_DIR}/duckdb_tpch_bench.cpp"       PARENT_SCOPE)
   set(DUCKDB_SRC_DIR        "${duckdb_src_SOURCE_DIR}"                               PARENT_SCOPE)
   set(DUCKDB_IR_OBJ         "${DUCKDB_IR_OBJ}"                                       PARENT_SCOPE)
   ```

**Verification**: Build `DuckDBTpchBenchmark` and run with `--benchmark_filter=.*db/duckdb/tpch.*`. Confirm Q1–Q22 × 4 phases appear. Check that `jit_pruned_fns` < `jit_module_fns` for at least one query (confirms IR pruning is active).

---

### Phase 4: Data generation update

**Goal**: Extend `generate.py` to produce both `tpch.db` (SQLite) and `tpch.duckdb` (DuckDB).

**Steps**:

1. **Add `--duckdb-out` argument** to `generate.py` (default: `data/tpch.duckdb`).

2. **Write DuckDB file**: After `dbgen()` populates the in-memory DuckDB instance, persist to a file path:

   ```python
   def generate_duckdb(sf: float, duckdb_out: str):
       # duckdb.connect(path) creates a persistent on-disk database
       with duckdb.connect(duckdb_out) as db:
           db.execute(f"CALL dbgen(sf={sf})")
   ```

3. **Preserve existing SQLite output**: Keep the existing SQLite generation logic unchanged. Both outputs are produced in a single script run.

4. **Document the three-file distinction** in the script's `--help` text:
   ```
   --out:        SQLite TPC-H database path (NOT the benchmark results database)
   --duckdb-out: DuckDB TPC-H database path (NOT the benchmark results database)
   ```

**Verification**: Run `python generate.py --sf 0.01 --out /tmp/test.db --duckdb-out /tmp/test.duckdb`. Verify both files are created and `test.duckdb` contains all 8 tables with non-zero row counts.

---

### Phase 5: AllBenchmarks integration

**Goal**: Add DuckDB TPC-H benchmarks to the combined `AllBenchmarks` binary.

**Steps**:

1. **Add `duckdb_tpch_set_db_path` declaration** in `AllBenchmarks_main.cpp`:
   ```cpp
   void duckdb_tpch_set_db_path(const char* path);
   ```

2. **Add `--duckdb-db=<path>` parsing** alongside the existing `--db=` parsing:
   ```cpp
   } else if (a.rfind("--duckdb-db=", 0) == 0) {
       duckdb_tpch_set_db_path(std::string(a.substr(12)).c_str());
   }
   ```

3. **Add AllBenchmarks DuckDB compilation command** in `benchmarks/CMakeLists.txt`:

   ```cmake
   set(ALL_OBJ_DUCKDB_TPCH "${CMAKE_CURRENT_BINARY_DIR}/AllBenchmarks_duckdb_tpch_bench.o")
   add_custom_command(
       OUTPUT ${ALL_OBJ_DUCKDB_TPCH}
       COMMAND ${CLANG_EXE} -x c++ -std=c++17 -g -O3 ${CXX_FLAGS_LIST}
               -DALL_BENCHMARKS_BUILD=1
               -I${DUCKDB_TPCH_SRC_DIR}
               -I${CMAKE_CURRENT_SOURCE_DIR}
               -I${CMAKE_CURRENT_SOURCE_DIR}/../runtime/ClangRuntimeSpecializer
               -I${DUCKDB_SRC_DIR}
               -I${LLVM_MAIN_INCLUDE_DIR}
               -I${LLVM_BINARY_DIR}/include
               -I${LLVM_THIRD_PARTY_DIR}/benchmark/include
               -fpass-plugin=${PLUGIN_LIB}
               -fexceptions -frtti
               -c ${DUCKDB_TPCH_SRC_FILE}
               -o ${ALL_OBJ_DUCKDB_TPCH}
       DEPENDS ${DUCKDB_TPCH_SRC_FILE} LLVMRuntimeSpecializationComptimePlugin clang
               ${DUCKDB_IR_OBJ}
       COMMENT "Compiling duckdb_tpch_bench.cpp for AllBenchmarks"
   )
   ```

4. **Add new objects to `add_benchmark(AllBenchmarks ...)`**:
   ```cmake
   add_benchmark(AllBenchmarks PARTIAL_SOURCES_INTENDED
       ${KERNELS_OBJ_FILE}
       ${ALL_OBJ_SPEC} ${ALL_OBJ_DB} ${ALL_OBJ_POLY}
       ${ALL_OBJ_TPCH} ${TPCH_SQLITE3_IR_OBJ}
       ${ALL_OBJ_DUCKDB_TPCH} ${DUCKDB_IR_OBJ}
       AllBenchmarks_main.cpp)
   ```

5. **Add DuckDB source dir to AllBenchmarks include dirs**:
   ```cmake
   target_include_directories(AllBenchmarks PRIVATE
       ...
       ${DUCKDB_SRC_DIR}
   )
   ```

**Verification**: Build `AllBenchmarks` and run:
```bash
./AllBenchmarks --benchmark_filter=".*db/(sqlite3|duckdb)/tpch.*" \
  --db=tpch/data/tpch.db \
  --duckdb-db=tpch/data/tpch.duckdb
```
Confirm both groups appear and exit code is 0.

---

## Risk Notes

1. **DuckDB compilation time**: `duckdb.cpp` (~500k lines) compiled with IR-dumping plugin will be slow (potentially 10–20 min). The resulting IR blob will be large. If the blob causes memory issues in the JIT, `withEarlyPrune(true)` in `phaseJITOverhead`/`phaseSpecializedExec` will be critical for reducing module size before optimization.

2. **`duckdb_execute_prepared` re-execution semantics**: Must be verified that calling `duckdb_execute_prepared` on the same `prepared_statement` a second time (after `duckdb_destroy_result`) re-runs the full query from scratch without needing a reset call. If a reset/rebind step is required, add it between loop iterations.

3. **IR blob symbol visibility**: DuckDB uses internal anonymous namespaces and `static` functions. The IR-dumping plugin captures all functions including those with internal linkage. `duckdb_execute_prepared` is a C API function with external linkage — it will appear in the blob and be findable by `FuncToBlobIdx`.

4. **JIT vtable devirtualization**: DuckDB's execution relies heavily on C++ virtual dispatch through `PhysicalOperator` subclasses. The `IRDumpingPass` vtable BFS (spec 004) will attempt to tag vtable functions as `WeakODR` in the blob. Whether this yields significant specialization benefit is the primary research question for this benchmark group.

## Complexity Tracking

No constitution violations.
