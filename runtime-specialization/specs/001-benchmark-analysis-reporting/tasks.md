# Tasks: Benchmark Analysis & Reporting System

**Input**: Design documents from `specs/001-benchmark-analysis-reporting/`  
**Prerequisites**: plan.md ✓, spec.md ✓, research.md ✓, data-model.md ✓

> **Implementation note**: US1, US2, US5, and US6 are largely complete in the existing codebase.
> The task list below covers only the remaining gaps identified in plan.md.
> US3 and US4 require new/extended code.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to

---

## Phase 1: Setup

No setup needed — existing scripts, schema DDL, and dependencies are already in place.

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: The `create_db.py` script (US3) is foundational to the `record_benchmark.py` error-message patch (US1) because the patch must name `create_db.py`. US3 must be complete before US1 can be finalized.

**⚠️ CRITICAL**: Complete T001 before T002.

- [x] T001 [US3] Create `benchmarks/create_db.py` — CLI (`--db`/`BENCHPLOT_DB_PATH`/default), exit-1 if file exists, CREATE all 6 tables + 8 views from plan.md schema, print success + path

**Checkpoint**: `create_db.py` exists and is callable; US1 error-message patch and US4 can now proceed.

---

## Phase 3: User Story 1 & 2 — Run/Record/Import Benchmark (Priority: P1)

**Goal**: `record_benchmark.py` directs users to the now-existing `create_db.py` when the DB is missing.

**Independent Test**: Run `record_benchmark.py` against a missing DB path; verify the error message says "Run create_db.py to initialise a new database." (not "--create-db").

- [x] T002 [US1] In `benchmarks/record_benchmark.py` `open_db()` — change missing-DB error message from "Pass --create-db to initialise a new database." to "Run create_db.py to initialise a new database."

> US2 (`--record-json`, `--pass-trace-dir`) is fully implemented; no tasks required.

**Checkpoint**: US1 and US2 are fully functional.

---

## Phase 4: User Story 4 — Optimize JIT Pipeline Configuration (Priority: P2)

**Goal**: Optimization studies write an `optimization_sessions` row at start (`incomplete`) and update it to `complete` on normal exit; interrupted studies remain `incomplete` and are excluded from best-config views.

**Independent Test**: Run `optimize_benchmarks.py --n-trials=2` against a benchmark binary; verify (a) an `optimization_sessions` row exists with `status='complete'` after normal exit, and (b) interrupting with Ctrl-C leaves `status='incomplete'` and those rows do not appear in `v_optim_best_per_kernel`.

- [x] T003 [US4] In `benchmarks/optimize_benchmarks.py` — add `_SCHEMA_OPTIM_SESSIONS` DDL constant and call `con.execute(_SCHEMA_OPTIM_SESSIONS)` in `open_optim_db()`, matching the `optimization_sessions` table definition in `plan.md`

- [x] T004 [US4] In `benchmarks/optimize_benchmarks.py` `main()` — after `open_optim_db()`, INSERT a row into `optimization_sessions` with `status='incomplete'`, `started_at=NOW()`; wrap `study.optimize(...)` in `try/finally`; on normal exit UPDATE `status='complete'`, `completed_at=NOW()`; on `KeyboardInterrupt` print a warning and leave status `incomplete`

- [x] T005 [US4] In `benchmarks/optimize_benchmarks.py` — update `_SCHEMA_V_OPTIM_BEST_PER_KERNEL` to JOIN `optimization_sessions` and add `WHERE os.status = 'complete'`, matching the view DDL in `plan.md`; verify `_SCHEMA_V_OPTIM_BREAKEVEN` does not need the same filter (it exposes raw trial data; filtering happens at the best-per-kernel layer)

- [x] T006 [P] [US4] In `benchmarks/create_db.py` — add `optimization_sessions` table DDL and the updated `v_optim_best_per_kernel` view DDL (with status filter) to the schema block, keeping it in sync with `optimize_benchmarks.py`

**Checkpoint**: US4 is fully functional; optimization studies have complete lifecycle tracking.

---

## Phase 5: User Story 3 — Initialize Data Store (verification) (Priority: P1)

> Implementation was done in Phase 2 (T001). This phase records the acceptance check.

**Independent Test**: Run `create_db.py --db /tmp/test.duckdb`; verify file is created; query `duckdb /tmp/test.duckdb "SHOW TABLES"` and confirm all 6 tables are present; run again against the same path and confirm it exits with an error and the file is unchanged.

No additional tasks — T001 and T006 together satisfy FR-016, FR-017, FR-018.

---

## Phase 6: User Stories 5 & 6 — Reporting Scripts (Priority: P3/P4)

> All 6 reporting scripts (`runtime_comparison.py`, `specialization_overhead.py`, `combined.py`, `memory_per_jit.py`, `pass_time_plot.py`, `pass_trace_plot.py`) are fully implemented and satisfy FR-028 through FR-040. No tasks required.

---

## Final Phase: Polish & Cross-Cutting Concerns

- [x] T007 [P] Update `benchmarks/record_benchmark.py` — remove the `--create-db` flag and its associated `open_db(…, create=True)` path now that `create_db.py` exists as the canonical initializer (keeps CLI surface minimal per constitution §V)

- [x] T008 [US1] In `benchmarks/record_benchmark.py` `check_dependencies()` — add early DB existence check so a missing DB is reported before the benchmark runs (fail-fast; prevents results being lost when the DB path is wrong)
- [x] T009 [US1] In `benchmarks/record_benchmark.py` `cmd_record()` — delete the temp JSON file when the benchmark exits with a non-zero code in standard (non-best-practice) mode to avoid orphaned temp files
- [x] T010 [US1] In `benchmarks/create_db.py` — add `best_practice_full BOOLEAN` column to `_SCHEMA_CONTEXT` to keep the canonical schema in sync with `record_benchmark.py`

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 2 (T001)**: No dependencies — start immediately
- **Phase 3 (T002)**: Depends on T001 (error message names `create_db.py`)
- **Phase 4 (T003–T006)**: T003 must precede T004 and T005; T006 can run in parallel with T003–T005 once T001 is done
- **Final Phase (T007)**: Depends on T001 (needs `create_db.py` to exist before removing `--create-db`)

### User Story Dependencies

| Story | Depends on | Notes |
|-------|-----------|-------|
| US3 (T001) | — | First to implement |
| US1/US2 (T002) | T001 | Error message names `create_db.py` |
| US4 (T003–T006) | T001 | `optimization_sessions` schema must exist in `create_db.py` |
| US5/US6 | — | Already complete; no dependency |

### Parallel Opportunities

- T003, T004, T005, T006 can all be dispatched after T001 completes; T006 is independent of T003–T005 within Phase 4 [marked P]

---

## Parallel Example: Phase 4 (US4)

```bash
# After T001 (create_db.py) is complete, launch in parallel:
Task T003: "Add _SCHEMA_OPTIM_SESSIONS DDL in benchmarks/optimize_benchmarks.py"
Task T006: "Add optimization_sessions DDL to benchmarks/create_db.py"

# After T003, launch sequentially:
Task T004: "Insert/update optimization_sessions lifecycle in optimize_benchmarks.py main()"
Task T005: "Update v_optim_best_per_kernel view with status='complete' filter"
```

---

## Implementation Strategy

### MVP (US3 + US1, P1 stories only)

1. T001 — create `create_db.py`
2. T002 — patch error message
3. **Validate**: `create_db.py` creates schema; `record_benchmark.py` gives correct error when DB missing

### Full Delivery

4. T003 → T004 → T005 (sequential within optimize_benchmarks.py)
5. T006 (parallel with T003–T005)
6. **Validate**: full optimization lifecycle with incomplete/complete tracking
7. T007 — polish: remove `--create-db` from `record_benchmark.py`

---

## Notes

- All tasks involve Python files only; no C++ or LLVM build required
- Schema in `create_db.py` (T001, T006) must stay in sync with `plan.md §DuckDB Schema`
- The `--create-db` flag removal (T007) is polish — defer if backwards compat with existing shell scripts matters
