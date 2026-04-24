#!/usr/bin/env python3
"""create_db.py — Initialise a fresh DuckDB database with the full benchmark schema.

Creates the database file and all tables/views required by record_benchmark.py and
optimize_benchmarks.py.  Exits with an error if the target file already exists.

Usage:
    create_db.py [--db PATH]
"""

import argparse
import os
import sys
from pathlib import Path

import duckdb

# ---------------------------------------------------------------------------
# Schema — copied from record_benchmark.py
# ---------------------------------------------------------------------------

_SCHEMA_CONTEXT = """
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
"""

# Base columns that are always present in Google Benchmark JSON output.
# Dynamic columns (perf counters, custom counters, etc.) are added on demand.
# kv_* columns are parsed from the benchmark name KV prefix (e.g. BM_g:X;n:Y;t:Z;).
_SCHEMA_BENCHMARKS = """
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
    kv_g                       VARCHAR,
    kv_n                       VARCHAR,
    kv_t                       VARCHAR,
    kv_raw_params              VARCHAR,
    PRIMARY KEY (run_id, name)
);
"""

# Per-pass records from PassInstrumentationCallbacks (written by benchmarkJITAnalysis).
_SCHEMA_PASS_TRACES = """
CREATE TABLE IF NOT EXISTS pass_traces (
    run_id          VARCHAR NOT NULL REFERENCES context(run_id),
    benchmark_name  VARCHAR NOT NULL,
    pass_idx        INTEGER NOT NULL,
    pass_name       VARCHAR,
    pass_group      VARCHAR,
    fixpoint_iter   INTEGER,
    fns_before      BIGINT,
    fns_after       BIGINT,
    instrs_before   BIGINT,
    instrs_after    BIGINT,
    bbs_before      BIGINT,
    bbs_after       BIGINT,
    wall_time_ms    DOUBLE,
    ir_changed      BOOLEAN
);
"""

# ---------------------------------------------------------------------------
# Schema — copied from optimize_benchmarks.py
# ---------------------------------------------------------------------------

# Lean trial-params table: stores only what is not derivable from context/benchmarks.
# Per-kernel (jit_ns, exec_ns) is recovered via v_ratios JOIN optim_trial_params USING (run_id).
_SCHEMA_OPTIM_TRIAL_PARAMS = """
CREATE TABLE IF NOT EXISTS optim_trial_params (
    study_name            VARCHAR NOT NULL,
    trial_id              INTEGER NOT NULL,
    run_id                VARCHAR REFERENCES context(run_id),
    params_json           JSON    NOT NULL,
    used_timeout_fallback BOOLEAN,
    obj_jit_ns            DOUBLE,
    obj_exec_ns           DOUBLE,
    obj_combined_ns       DOUBLE,
    PRIMARY KEY (study_name, trial_id)
);
"""

_SCHEMA_UNSPEC_BASELINES = """
CREATE TABLE IF NOT EXISTS unspec_baselines (
    study_name VARCHAR NOT NULL,
    kernel     VARCHAR NOT NULL,
    unspec_ns  DOUBLE  NOT NULL,
    PRIMARY KEY (study_name, kernel)
);
"""

_SCHEMA_OPTIM_SESSIONS = """
CREATE TABLE IF NOT EXISTS optimization_sessions (
    study_name        VARCHAR PRIMARY KEY,
    binary            VARCHAR,
    n_trials          INTEGER,
    started_at        TIMESTAMP,
    completed_at      TIMESTAMP,
    status            VARCHAR,
    search_space_json JSON
);
"""

# ---------------------------------------------------------------------------
# Views — copied from record_benchmark.py
# ---------------------------------------------------------------------------

# Expose stored KV columns as readable aliases.
_SCHEMA_V_PARSED = """
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
"""

# Convert all times to nanoseconds; drop rows that didn't match the naming convention.
_SCHEMA_V_NS = """
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
"""

# Pivot phases per (run_id, kernel, raw_params, group) for ratio computation.
_SCHEMA_V_RATIOS = """
CREATE OR REPLACE VIEW v_ratios AS
SELECT
    run_id,
    kernel,
    raw_params,
    "group",
    git_sha,
    run_ts,
    host_name,
    MAX(CASE WHEN phase = 'unspecialized'    THEN real_time_ns END) AS t_unspec_ns,
    MAX(CASE WHEN phase = 'specialized_exec' THEN real_time_ns END) AS t_spec_ns,
    MAX(CASE WHEN phase = 'jit_overhead'     THEN real_time_ns END) AS t_jit_ns
FROM v_ns
GROUP BY run_id, kernel, raw_params, "group", git_sha, run_ts, host_name;
"""

# JIT stats for jit_overhead rows (columns may not exist; view creation guarded).
_SCHEMA_V_JIT_STATS = """
CREATE OR REPLACE VIEW v_jit_stats AS
SELECT run_id, kernel, raw_params, "group",
       jit_module_fns, jit_module_instrs, jit_blob_kb,
       jit_pruned_fns, jit_pruned_instrs, max_bytes_used
FROM v_parsed
WHERE phase = 'jit_overhead' AND run_type = 'iteration';
"""

# Budget sweep Pareto view: (scale_bp, kernel) → (JIT overhead, speedup vs unspecialized).
_SCHEMA_V_BUDGET_SWEEP = """
CREATE OR REPLACE VIEW v_budget_sweep AS
SELECT run_id, kernel, "group", git_sha, run_ts,
       kv_raw_params,
       CAST(SPLIT_PART(kv_raw_params, '/', 2) AS INTEGER) AS scale_bp,
       real_time_ns, phase,
       expected_call_ns, budget_scale, budget_fixpoint, budget_unroll
FROM v_ns
WHERE phase IN ('jit_budget_sweep', 'exec_budget_sweep');
"""

# ---------------------------------------------------------------------------
# Views — copied from optimize_benchmarks.py
# ---------------------------------------------------------------------------

# Per-kernel (jit, exec, unspec) per trial — basis for per-kernel best-config analysis
# and for fitting the expected_runtime → optimal_Options prediction model.
_SCHEMA_V_OPTIM_RESULTS = """
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
"""

# Break-even: minimum call count for JIT specialization to amortize its overhead.
# Derivation: unspec_ns * X > jit_ns + spec_ns * X  →  X > jit_ns / (unspec_ns - spec_ns)
_SCHEMA_V_OPTIM_BREAKEVEN = """
CREATE OR REPLACE VIEW v_optim_breakeven AS
SELECT *,
    CASE
        WHEN unspec_ns > t_spec_ns
        THEN t_jit_ns / (unspec_ns - t_spec_ns)
        ELSE NULL
    END AS break_even_calls
FROM v_optim_results;
"""

# Per-kernel best trial: the config that minimised jit + exec for each kernel individually.
# Only considers studies where status = 'complete' (excludes interrupted studies).
_SCHEMA_V_OPTIM_BEST_PER_KERNEL = """
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
"""

# ---------------------------------------------------------------------------
# DB path resolution
# ---------------------------------------------------------------------------

def resolve_db_path(flag_value: str | None) -> Path:
    """Resolve DB path: --db flag > BENCHPLOT_DB_PATH env var > CWD/benchmarks.duckdb."""
    if flag_value is not None:
        return Path(flag_value)
    env = os.environ.get("BENCHPLOT_DB_PATH")
    if env:
        return Path(env)
    return Path.cwd() / "benchmarks.duckdb"


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Initialise a fresh DuckDB database with the full benchmark schema."
    )
    parser.add_argument(
        "--db", default=None, metavar="PATH",
        help="DuckDB file path (overrides BENCHPLOT_DB_PATH env var and CWD default).",
    )
    args = parser.parse_args()

    db_path = resolve_db_path(args.db)

    if db_path.exists():
        print(f"Error: DB file already exists: {db_path}", file=sys.stderr)
        sys.exit(1)

    con = duckdb.connect(str(db_path))

    # Tables (order matters: context before benchmarks/pass_traces due to FK references)
    con.execute(_SCHEMA_CONTEXT)
    con.execute(_SCHEMA_BENCHMARKS)
    con.execute(_SCHEMA_PASS_TRACES)
    con.execute(_SCHEMA_OPTIM_TRIAL_PARAMS)
    con.execute(_SCHEMA_UNSPEC_BASELINES)
    con.execute(_SCHEMA_OPTIM_SESSIONS)

    # Core views
    con.execute(_SCHEMA_V_PARSED)
    con.execute(_SCHEMA_V_NS)
    con.execute(_SCHEMA_V_RATIOS)

    # Views that reference dynamic columns — guarded against fresh DBs without those columns
    try:
        con.execute(_SCHEMA_V_JIT_STATS)
    except Exception:
        pass  # JIT counter columns not yet present in this DB

    try:
        con.execute(_SCHEMA_V_BUDGET_SWEEP)
    except Exception:
        pass  # budget counter columns not yet present in this DB

    # Optimizer views
    con.execute(_SCHEMA_V_OPTIM_RESULTS)
    con.execute(_SCHEMA_V_OPTIM_BREAKEVEN)
    con.execute(_SCHEMA_V_OPTIM_BEST_PER_KERNEL)

    con.close()

    print(f"Database initialised: {db_path}")


if __name__ == "__main__":
    main()
