# Implementation Plan: Benchmark Analysis & Reporting System

**Branch**: `001-benchmark-analysis-reporting` | **Date**: 2026-04-24  
**Spec**: `specs/001-benchmark-analysis-reporting/spec.md`  
**Input**: Feature specification from `specs/001-benchmark-analysis-reporting/spec.md`

## Summary

The benchmarking infrastructure (run → record → analyze → report) is largely already implemented. Two gaps remain: (1) a dedicated `create_db.py` script that initializes the full DuckDB schema in one step (FR-016–018), and (2) `OptimizationSession` tracking in `optimize_benchmarks.py` so interrupted studies are flagged `incomplete` and excluded from best-config queries (FR-027).

## Technical Context

**Language/Version**: Python 3.11 (Nix flake)  
**Primary Dependencies**: duckdb, optuna, pandas, ultraplot, matplotlib  
**Storage**: DuckDB file at `benchmarks/benchmarks.duckdb` (local filesystem)  
**Testing**: Manual invocation; no automated test suite for Python scripts  
**Target Platform**: Linux (researcher's local machine; single-user)  
**Project Type**: CLI tooling (data recording + reporting)  
**Performance Goals**: `create_db.py` < 5 s (SC-007); reporting scripts < 30 s on 50+ runs (SC-004)  
**Constraints**: DuckDB data must never be lost; schema changes are additive only (constitution §Backwards Compatibility)  
**Scale/Scope**: Single researcher; hundreds of runs; 6 reporting scripts

## Constitution Check

| Principle | Status | Notes |
|-----------|--------|-------|
| I. Correctness & Safety | ✓ | No LLVM C++ code; Python CLI scripts only |
| II. LLVM Coding Standards | N/A | Python, not C++ library code |
| III. Test-First Validation | N/A | No lit/FileCheck tests required for Python tools |
| IV. Performance Measurement | ✓ | This feature *is* the benchmarking infrastructure |
| V. Minimal Public API | ✓ | Scripts expose only CLI; no public programmatic API |
| Backwards Compatibility | ✓ | All schema changes add tables/columns; existing data preserved via `IF NOT EXISTS` |
| System Environment | ✓ | All Python deps already in flake.nix |

**GATE: All principles pass.**

## DuckDB Schema (authoritative — DR-001)

This section is the single source of truth for the DuckDB schema. It reflects the DDL
in `record_benchmark.py`, `optimize_benchmarks.py`, and `create_db.py` (to be created).

### Tables

#### `context`

One row per benchmark run (per binary invocation recorded with `record_benchmark.py`).

```sql
CREATE TABLE IF NOT EXISTS context (
    run_id              VARCHAR PRIMARY KEY,
    run_ts              TIMESTAMP NOT NULL,
    git_sha             VARCHAR,
    date                VARCHAR,
    host_name           VARCHAR,
    executable          VARCHAR,
    num_cpus            INTEGER,
    mhz_per_cpu         INTEGER,
    cpu_scaling_enabled BOOLEAN,
    library_version     VARCHAR,
    library_build_type  VARCHAR
);
```

#### `benchmarks`

One row per benchmark result within a run. Base columns are fixed; dynamic columns
(hardware perf counters, JIT stats, etc.) are added on demand via `ALTER TABLE … ADD COLUMN`.

```sql
CREATE TABLE IF NOT EXISTS benchmarks (
    run_id                     VARCHAR NOT NULL REFERENCES context(run_id),
    name                       VARCHAR NOT NULL,
    family_index               INTEGER,
    per_family_instance_index  INTEGER,
    run_type                   VARCHAR,
    repetitions                INTEGER,
    repetition_index           INTEGER,
    threads                    INTEGER,
    iterations                 BIGINT,
    real_time                  DOUBLE,
    cpu_time                   DOUBLE,
    time_unit                  VARCHAR,
    kv_g                       VARCHAR,   -- parsed KV: group
    kv_n                       VARCHAR,   -- parsed KV: kernel name
    kv_t                       VARCHAR,   -- parsed KV: phase
    kv_raw_params              VARCHAR,   -- unparsed suffix after KV prefix
    PRIMARY KEY (run_id, name)
    -- Dynamic columns added via ALTER TABLE ADD COLUMN:
    --   instructions BIGINT, cpu_cycles BIGINT, branch_misses BIGINT,
    --   L1_icache_load_misses BIGINT, L1_icache_loads BIGINT,
    --   iTLB_load_misses BIGINT,
    --   jit_module_fns BIGINT, jit_module_instrs BIGINT, jit_blob_kb DOUBLE,
    --   jit_pruned_fns BIGINT, jit_pruned_instrs BIGINT,
    --   max_bytes_used BIGINT, expected_call_ns DOUBLE,
    --   budget_scale DOUBLE, budget_fixpoint INTEGER, budget_unroll INTEGER,
    --   ... (any further custom counters emitted by the binary)
);
```

#### `pass_traces`

One row per compiler pass per `benchmarkJITAnalysis` invocation.

```sql
CREATE TABLE IF NOT EXISTS pass_traces (
    run_id          VARCHAR NOT NULL REFERENCES context(run_id),
    benchmark_name  VARCHAR NOT NULL,
    pass_idx        INTEGER NOT NULL,
    pass_name       VARCHAR,
    pass_group      VARCHAR,   -- prune | initial | fixpoint | postfix | final
    fixpoint_iter   INTEGER,   -- NULL / -1 for non-fixpoint passes
    fns_before      BIGINT,
    fns_after       BIGINT,
    instrs_before   BIGINT,
    instrs_after    BIGINT,
    bbs_before      BIGINT,
    bbs_after       BIGINT,
    wall_time_ms    DOUBLE,
    ir_changed      BOOLEAN
);
```

#### `optimization_sessions` *(to be added — FR-027)*

One row per `optimize_benchmarks.py` invocation. Written at study start as `incomplete`;
updated to `complete` on normal exit. Interrupted runs remain `incomplete`.

```sql
CREATE TABLE IF NOT EXISTS optimization_sessions (
    study_name    VARCHAR PRIMARY KEY,
    binary        VARCHAR,
    n_trials      INTEGER,
    started_at    TIMESTAMP,
    completed_at  TIMESTAMP,   -- NULL until status = 'complete'
    status           VARCHAR,     -- 'incomplete' | 'complete'
    search_space_json VARCHAR      -- serialized SearchSpaceDescriptor JSON; NULL for pre-spec-002 rows
);
```

#### `optim_trial_params`

One row per Optuna trial. Written immediately on trial completion (FR-021).

```sql
CREATE TABLE IF NOT EXISTS optim_trial_params (
    study_name            VARCHAR NOT NULL
                              REFERENCES optimization_sessions(study_name),
    trial_id              INTEGER NOT NULL,
    run_id                VARCHAR REFERENCES context(run_id),
    params_json           VARCHAR NOT NULL,
    used_timeout_fallback BOOLEAN,
    obj_jit_ns            DOUBLE,
    obj_exec_ns           DOUBLE,
    obj_combined_ns       DOUBLE,
    PRIMARY KEY (study_name, trial_id)
);
```

#### `unspec_baselines`

Per-kernel unspecialized median execution time for a study (measured before trials begin).

```sql
CREATE TABLE IF NOT EXISTS unspec_baselines (
    study_name  VARCHAR NOT NULL
                    REFERENCES optimization_sessions(study_name),
    kernel      VARCHAR NOT NULL,
    unspec_ns   DOUBLE  NOT NULL,
    PRIMARY KEY (study_name, kernel)
);
```

### Views

#### `v_parsed`

`benchmarks` joined with `context`; adds readable aliases for the KV columns.

```sql
CREATE OR REPLACE VIEW v_parsed AS
SELECT
    b.*,
    c.git_sha,
    c.run_ts,
    c.host_name,
    b.kv_t          AS phase,
    b.kv_n          AS kernel,
    b.kv_g          AS "group",
    b.kv_raw_params AS raw_params
FROM benchmarks b
JOIN context c USING (run_id);
```

#### `v_ns`

All times converted to nanoseconds; rows without a recognised phase are dropped.

```sql
CREATE OR REPLACE VIEW v_ns AS
SELECT *,
    real_time * CASE time_unit
        WHEN 'ns' THEN 1.0
        WHEN 'us' THEN 1e3
        WHEN 'ms' THEN 1e6
        WHEN 's'  THEN 1e9
    END AS real_time_ns
FROM v_parsed
WHERE phase != '';
```

#### `v_ratios`

Pivots phases per `(run_id, kernel, raw_params, group)` for ratio computation.

```sql
CREATE OR REPLACE VIEW v_ratios AS
SELECT
    run_id, kernel, raw_params, "group", git_sha, run_ts, host_name,
    MAX(CASE WHEN phase = 'unspecialized'    THEN real_time_ns END) AS t_unspec_ns,
    MAX(CASE WHEN phase = 'specialized_exec' THEN real_time_ns END) AS t_spec_ns,
    MAX(CASE WHEN phase = 'jit_overhead'     THEN real_time_ns END) AS t_jit_ns
FROM v_ns
GROUP BY run_id, kernel, raw_params, "group", git_sha, run_ts, host_name;
```

#### `v_jit_stats`

JIT stats columns for `jit_overhead` iteration rows. Guarded at creation time because
the dynamic columns may not exist in every database instance.

```sql
CREATE OR REPLACE VIEW v_jit_stats AS
SELECT run_id, kernel, raw_params, "group",
       jit_module_fns, jit_module_instrs, jit_blob_kb,
       jit_pruned_fns, jit_pruned_instrs, max_bytes_used
FROM v_parsed
WHERE phase = 'jit_overhead' AND run_type = 'iteration';
```

#### `v_budget_sweep`

Pareto sweep rows (budget-aware JIT options experiment).

```sql
CREATE OR REPLACE VIEW v_budget_sweep AS
SELECT run_id, kernel, "group", git_sha, run_ts,
       kv_raw_params,
       CAST(SPLIT_PART(kv_raw_params, '/', 2) AS INTEGER) AS scale_bp,
       real_time_ns, phase,
       expected_call_ns, budget_scale, budget_fixpoint, budget_unroll
FROM v_ns
WHERE phase IN ('jit_budget_sweep', 'exec_budget_sweep');
```

#### `v_optim_results`

Per-kernel `(jit, exec, unspec)` per trial; joins `optim_trial_params` with `v_ratios`
and `unspec_baselines`.

```sql
CREATE OR REPLACE VIEW v_optim_results AS
SELECT
    otp.study_name, otp.trial_id,
    otp.params_json, otp.used_timeout_fallback,
    r.kernel, r."group",
    r.t_jit_ns, r.t_spec_ns,
    u.unspec_ns
FROM optim_trial_params otp
JOIN v_ratios r USING (run_id)
LEFT JOIN unspec_baselines u
    ON u.study_name = otp.study_name AND u.kernel = r.kernel;
```

#### `v_optim_breakeven`

Extends `v_optim_results` with the minimum call count for JIT to amortize its overhead.

```sql
CREATE OR REPLACE VIEW v_optim_breakeven AS
SELECT *,
    CASE
        WHEN unspec_ns > t_spec_ns
        THEN t_jit_ns / (unspec_ns - t_spec_ns)
        ELSE NULL
    END AS break_even_calls
FROM v_optim_results;
```

#### `v_optim_best_per_kernel`

Best trial per `(study_name, kernel)`. **Filters `status = 'complete'`** via join on
`optimization_sessions` so interrupted studies are excluded by default (FR-027, FR-034).

```sql
CREATE OR REPLACE VIEW v_optim_best_per_kernel AS
SELECT DISTINCT ON (ob.study_name, ob.kernel)
    ob.study_name, ob.trial_id, ob.kernel,
    ob.params_json,
    ob.t_jit_ns, ob.t_spec_ns, ob.unspec_ns,
    (ob.t_jit_ns + ob.t_spec_ns) AS total_ns,
    ob.break_even_calls
FROM (SELECT *, (t_jit_ns + t_spec_ns) AS _total FROM v_optim_breakeven) ob
JOIN optimization_sessions os USING (study_name)
WHERE os.status = 'complete'
ORDER BY ob.study_name, ob.kernel, ob._total;
```

### Schema evolution rules

- New counter columns from benchmark binaries are added via `ALTER TABLE benchmarks ADD COLUMN` at import time (FR-015).
- Structural schema changes (new tables, new fixed columns) require an explicit migration script before the DDL is altered; existing data must survive.
- Views are always `CREATE OR REPLACE`; they tolerate new columns automatically.

## Gap Analysis

### Already implemented

| Requirement(s) | Script | Notes |
|----------------|--------|-------|
| FR-001–015 | `record_benchmark.py` | Recording, best-practice isolation, pass traces, dynamic columns |
| FR-019–026 | `optimize_benchmarks.py` | Baseline, Optuna TPE, per-trial writes, `--n-parallel`, `--seed` |
| FR-028–040 | `reporting/*.py` | All 6 plot scripts + shared utilities |

### Gaps to implement

| FR | Description | File |
|----|-------------|------|
| FR-016, FR-017, FR-018 | `create_db.py` does not exist | Create `benchmarks/create_db.py` |
| FR-027 | No `optimization_sessions` table; no `incomplete`/`complete` lifecycle; views don't filter incomplete sessions | `optimize_benchmarks.py` + `create_db.py` |
| DR-001 | `plan.md` must contain authoritative schema | This document ✓ |
| Edge case | Missing-DB error message should direct user to `create_db.py` | `record_benchmark.py` (1-line patch) |

## Project Structure

### Documentation (this feature)

```text
specs/001-benchmark-analysis-reporting/
├── plan.md              # This file — includes authoritative DuckDB schema (DR-001)
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 supplemental reference (entity narrative form)
└── tasks.md             # Phase 2 output (from /speckit-tasks)
```

### Source Code (impacted files only)

```text
benchmarks/
├── create_db.py                  # NEW — standalone DB initializer (FR-016–018)
├── record_benchmark.py           # PATCH — fix missing-DB error message
├── optimize_benchmarks.py        # PATCH — add optimization_sessions + FR-027 lifecycle
└── reporting/
    └── (all scripts complete — no changes needed)
```

## Implementation Details

### 1. `create_db.py` (new)

- CLI: `create_db.py [--db PATH]` — same resolution order: `--db` → `BENCHPLOT_DB_PATH` → `./benchmarks.duckdb`
- If target **exists**: print error, exit 1, leave file untouched (FR-017)
- If target **does not exist**: create it, execute all DDL from the schema above (FR-016)
- Prints success message + path; completes in < 5 s (SC-007)

### 2. `optimization_sessions` + FR-027 lifecycle (`optimize_benchmarks.py`)

In `main()`:
1. After `open_optim_db()`, `INSERT` row with `status = 'incomplete'`, `started_at = NOW()`.
2. Wrap `study.optimize(...)` in `try/finally`.
3. On normal return: `UPDATE … SET status = 'complete', completed_at = NOW()`.
4. On `KeyboardInterrupt`: print warning, leave row as `incomplete`, re-raise (or exit).

Update view definitions: `v_optim_best_per_kernel` (see schema above) gains the `JOIN optimization_sessions … WHERE status = 'complete'` filter.

### 3. `record_benchmark.py` error message patch

Change the missing-DB message in `open_db()` from  
`"Pass --create-db to initialise a new database."`  
to  
`"Run create_db.py to initialise a new database."`
