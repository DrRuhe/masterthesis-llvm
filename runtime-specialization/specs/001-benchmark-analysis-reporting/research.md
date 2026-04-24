# Research: Benchmark Analysis & Reporting System

**Branch**: `001-benchmark-analysis-reporting` | **Date**: 2026-04-24  
**Status**: Complete — no blockers identified

## Findings

### FR-027 — OptimizationSession: `incomplete` / `complete` status

**Decision**: Add `optimization_sessions` table to the schema (in both `create_db.py` and `optimize_benchmarks.py`). Write a row at study start with `status = 'incomplete'`; update to `'complete'` in the `finally` block after `study.optimize()` returns normally. A `SIGINT` handler sets a flag that causes the `finally` block to leave the status as `'incomplete'`.

**Rationale**: DuckDB does not support multi-connection writes safely with background threads, so the session update must happen in the main thread after `study.optimize()` exits. Using `finally` is sufficient — the process either completes normally (→ `complete`) or is killed/interrupted (→ `incomplete` row persists because the main-thread `finally` does not update).

**Alternatives considered**: A context-manager around the study; rejected because Optuna already manages the trial lifecycle and we only need one update at the study level.

**Incomplete-session filtering**: `v_optim_best_per_kernel` and `v_optim_breakeven` join against `optimization_sessions` and filter `status = 'complete'`. An explicit `--include-incomplete` flag on future optimization reporting scripts will allow overriding this.

### `create_db.py` — Scope of schema

**Decision**: `create_db.py` creates all tables and views defined across both `record_benchmark.py` and `optimize_benchmarks.py`, including:
- Tables: `context`, `benchmarks`, `pass_traces`, `optim_trial_params`, `unspec_baselines`, `optimization_sessions`
- Views: `v_parsed`, `v_ns`, `v_ratios`, `v_jit_stats`, `v_budget_sweep`, `v_optim_results`, `v_optim_breakeven`, `v_optim_best_per_kernel`

**Rationale**: A researcher should be able to `create_db.py` once and then use all scripts without any further DB initialization.

**How the existing scripts relate**: `record_benchmark.py`'s `open_db()` uses `CREATE TABLE IF NOT EXISTS` and `CREATE OR REPLACE VIEW` so it still works even when some optimization-schema objects already exist from `create_db.py`. `optimize_benchmarks.py` similarly uses `IF NOT EXISTS`. No duplication risk.

### `--create-db` flag in `record_benchmark.py`

**Decision**: Keep the `--create-db` flag in `record_benchmark.py` for backwards compatibility with existing shell scripts, but update the error message when DB is missing to point users to `create_db.py` instead.

**Rationale**: The spec says `record_benchmark.py` exits with an error directing the user to `create_db.py` when the DB is missing (edge case entry in spec). The current message says "Pass --create-db to initialise a new database." — this should change to "Run create_db.py first."

### Existing reporting scripts

All 6 reporting scripts (`runtime_comparison.py`, `specialization_overhead.py`, `combined.py`, `memory_per_jit.py`, `pass_time_plot.py`, `pass_trace_plot.py`) are fully implemented and cover FR-028 through FR-040. No work needed.

### FR-034 scope

FR-034 applies to "optimization-result reporting scripts". No such dedicated script exists yet. The view `v_optim_best_per_kernel` currently does not filter by session status — it will be updated as part of the OptimizationSession work so that any future optimization reporting script that queries this view inherits the filter automatically.
