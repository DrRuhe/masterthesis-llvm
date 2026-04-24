#!/usr/bin/env python3
"""optimize_benchmarks.py — Find optimal JIT pipeline Options for a given workload.

Minimizes jit_overhead_ns + specialized_exec_ns (per-kernel, then geomean across kernels)
by tuning MaxFixpointIterations, LoopUnrollCount, LargeModuleInstrThreshold,
EnableEarlyPrune, and EnableO3Final via ENV var overrides on Options::Default().

Each benchmark is run in its own subprocess for timeout isolation: a JIT hang in one
benchmark does not kill data from other kernels. Timed-out benchmarks use a fallback of
(timeout_ns for jit, unspecialized_ns for exec) so the optimizer always receives a finite
cost signal.

Usage:
    optimize_benchmarks.py BINARY [options]
"""
import argparse
import json
import math
import os
import re
import subprocess
import sys
import tempfile
import threading
import uuid
from datetime import datetime
from pathlib import Path

import duckdb

try:
    import optuna
    optuna.logging.set_verbosity(optuna.logging.WARNING)
except ImportError:
    print("Error: optuna is required. Install with: pip install optuna", file=sys.stderr)
    sys.exit(1)

sys.path.insert(0, str(Path(__file__).parent))
from record_benchmark import (
    open_db as _rb_open_db,
    ensure_columns,
    insert_benchmarks,
    resolve_db_path,
    _parse_bm_name,
    get_git_sha,
)

# ---------------------------------------------------------------------------
# Schema
# ---------------------------------------------------------------------------

# Lean trial-params table: stores only what is not derivable from context/benchmarks.
# Per-kernel (jit_ns, exec_ns) is recovered via v_ratios JOIN optim_trial_params USING (run_id).
_SCHEMA_OPTIM_TRIAL_PARAMS = """
CREATE TABLE IF NOT EXISTS optim_trial_params (
    study_name            VARCHAR NOT NULL,
    trial_id              INTEGER NOT NULL,
    run_id                VARCHAR REFERENCES context(run_id),
    fixpoint_max          INTEGER,
    unroll_max            INTEGER,
    large_module_max      INTEGER,
    early_prune           BOOLEAN,
    o3_final              BOOLEAN,
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
    study_name    VARCHAR PRIMARY KEY,
    binary        VARCHAR,
    n_trials      INTEGER,
    started_at    TIMESTAMP,
    completed_at  TIMESTAMP,
    status        VARCHAR
);
"""

# Per-kernel (jit, exec, unspec) per trial — basis for per-kernel best-config analysis
# and for fitting the expected_runtime → optimal_Options prediction model.
_SCHEMA_V_OPTIM_RESULTS = """
CREATE OR REPLACE VIEW v_optim_results AS
SELECT
    otp.study_name, otp.trial_id,
    otp.fixpoint_max, otp.unroll_max, otp.large_module_max,
    otp.early_prune, otp.o3_final, otp.used_timeout_fallback,
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
_SCHEMA_V_OPTIM_BEST_PER_KERNEL = """
CREATE OR REPLACE VIEW v_optim_best_per_kernel AS
SELECT DISTINCT ON (ob.study_name, ob.kernel)
    ob.study_name, ob.trial_id, ob.kernel,
    ob.fixpoint_max, ob.unroll_max, ob.large_module_max,
    ob.early_prune, ob.o3_final,
    ob.t_jit_ns, ob.t_spec_ns, ob.unspec_ns,
    (ob.t_jit_ns + ob.t_spec_ns) AS total_ns,
    ob.break_even_calls
FROM (SELECT *, (t_jit_ns + t_spec_ns) AS _total FROM v_optim_breakeven) ob
JOIN optimization_sessions os USING (study_name)
WHERE os.status = 'complete'
ORDER BY ob.study_name, ob.kernel, ob._total;
"""

# ---------------------------------------------------------------------------
# DB helpers
# ---------------------------------------------------------------------------

def open_optim_db(db_path: Path) -> duckdb.DuckDBPyConnection:
    """Open benchmark DB (create if absent), ensure optimizer tables and views exist."""
    con = _rb_open_db(db_path, create=not db_path.exists())
    con.execute(_SCHEMA_OPTIM_TRIAL_PARAMS)
    con.execute(_SCHEMA_UNSPEC_BASELINES)
    con.execute(_SCHEMA_V_OPTIM_RESULTS)
    con.execute(_SCHEMA_V_OPTIM_BREAKEVEN)
    con.execute(_SCHEMA_V_OPTIM_BEST_PER_KERNEL)
    con.execute(_SCHEMA_OPTIM_SESSIONS)
    return con


def store_raw_benchmarks(con: duckdb.DuckDBPyConnection, data: dict, git_sha: str) -> str:
    """Insert context + benchmark rows from raw JSON; returns run_id."""
    ctx = data.get("context", {})
    benchmarks = data.get("benchmarks", [])
    run_id = str(uuid.uuid4())
    run_ts = datetime.now()
    con.execute(
        "INSERT INTO context VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
        [
            run_id, run_ts, git_sha,
            ctx.get("date", ""), ctx.get("host_name", ""), ctx.get("executable", ""),
            ctx.get("num_cpus"), ctx.get("mhz_per_cpu"), ctx.get("cpu_scaling_enabled"),
            ctx.get("library_version", ""), ctx.get("library_build_type", ""),
        ],
    )
    ensure_columns(con, benchmarks)
    insert_benchmarks(con, run_id, benchmarks)
    return run_id


def store_trial_params(
    con: duckdb.DuckDBPyConnection,
    study_name: str,
    trial_id: int,
    run_id: str | None,
    magic: dict,
    used_timeout_fallback: bool,
    obj_jit: float | None,
    obj_exec: float | None,
    obj_combined: float | None,
) -> None:
    con.execute(
        "INSERT INTO optim_trial_params "
        "(study_name, trial_id, run_id, fixpoint_max, unroll_max, large_module_max, "
        " early_prune, o3_final, used_timeout_fallback, obj_jit_ns, obj_exec_ns, obj_combined_ns) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
        [
            study_name, trial_id, run_id,
            int(magic["fixpoint"]), int(magic["unroll"]), int(magic["large_mod"]),
            bool(magic["early_prune"]), bool(magic["o3_final"]),
            used_timeout_fallback,
            obj_jit, obj_exec, obj_combined,
        ],
    )


def store_unspec_baselines(
    con: duckdb.DuckDBPyConnection,
    study_name: str,
    unspec_by_kernel: dict[str, float],
) -> None:
    con.execute("DELETE FROM unspec_baselines WHERE study_name = ?", [study_name])
    for kernel, ns in unspec_by_kernel.items():
        con.execute(
            "INSERT INTO unspec_baselines (study_name, kernel, unspec_ns) VALUES (?, ?, ?)",
            [study_name, kernel, ns],
        )


# ---------------------------------------------------------------------------
# Benchmark enumeration and baseline measurement
# ---------------------------------------------------------------------------

def list_benchmarks(binary: str, filter_pattern: str) -> list[str]:
    """Return benchmark names matching filter_pattern via --benchmark_list_tests."""
    result = subprocess.run(
        [binary, "--benchmark_list_tests", f"--benchmark_filter={filter_pattern}"],
        capture_output=True, text=True, timeout=30, check=True,
    )
    names = []
    for line in result.stdout.splitlines():
        stripped = line.strip()
        # Skip blank lines and the single-word "Benchmark" header line
        if stripped and stripped != "Benchmark":
            names.append(stripped)
    return names


def measure_unspecialized(binary: str, timeout: float) -> dict[str, float]:
    """Run unspecialized benchmarks 3 times; return {kernel_name: median_ns}. Fatal on failure."""
    fd, out_json = tempfile.mkstemp(suffix="_unspec.json")
    os.close(fd)
    try:
        subprocess.run(
            [
                binary,
                "--benchmark_out_format=json",
                f"--benchmark_out={out_json}",
                "--benchmark_filter=unspecialized",
                "--benchmark_repetitions=3",
            ],
            capture_output=True, timeout=timeout * 4, check=True,
        )
        with open(out_json) as f:
            data = json.load(f)
    except Exception as e:
        raise RuntimeError(f"Failed to measure unspecialized baseline: {e}") from e
    finally:
        Path(out_json).unlink(missing_ok=True)

    mult_map = {"ns": 1.0, "us": 1e3, "ms": 1e6, "s": 1e9}
    result: dict[str, float] = {}
    for b in data.get("benchmarks", []):
        if b.get("run_type") != "aggregate" or b.get("aggregate_name") != "median":
            continue
        kv = _parse_bm_name(b.get("name", ""))
        kernel = kv.get("kv_n", "")
        if kernel and kv.get("kv_t", "") == "unspecialized":
            ns = b.get("real_time", 0.0) * mult_map.get(b.get("time_unit", "ns"), 1.0)
            result[kernel] = ns
    if not result:
        raise RuntimeError("No unspecialized benchmark rows found — check binary and filter.")
    return result


# ---------------------------------------------------------------------------
# Per-benchmark subprocess execution
# ---------------------------------------------------------------------------

def _make_env(magic: dict) -> dict:
    return {
        **os.environ,
        "CRS_DEFAULT_FIXPOINT":    str(magic["fixpoint"]),
        "CRS_DEFAULT_UNROLL":      str(magic["unroll"]),
        "CRS_DEFAULT_LARGE_MOD":   str(magic["large_mod"]),
        "CRS_DEFAULT_EARLY_PRUNE": str(magic["early_prune"]),
        "CRS_DEFAULT_O3_FINAL":    str(magic["o3_final"]),
    }


def run_single_benchmark(
    binary: str, name: str, env: dict, timeout: float
) -> dict | None:
    """Run one named benchmark; return parsed JSON or None on timeout/failure."""
    fd, out_json = tempfile.mkstemp(suffix=".json")
    os.close(fd)
    try:
        subprocess.run(
            [
                binary,
                "--benchmark_out_format=json",
                f"--benchmark_out={out_json}",
                f"--benchmark_filter=^{re.escape(name)}$",
                "--benchmark_repetitions=1",
            ],
            env=env, timeout=timeout, check=True, capture_output=True,
        )
        with open(out_json) as f:
            return json.load(f)
    except (subprocess.TimeoutExpired, subprocess.CalledProcessError,
            json.JSONDecodeError, OSError):
        return None
    finally:
        Path(out_json).unlink(missing_ok=True)


# ---------------------------------------------------------------------------
# Objective computation
# ---------------------------------------------------------------------------

def _geomean_from_dicts(
    jit_by_kernel: dict[str, float],
    exc_by_kernel: dict[str, float],
) -> tuple[float | None, float | None, float | None]:
    """(geomean_jit, geomean_exec, geomean_combined) for kernels present in both dicts."""
    paired = sorted(set(jit_by_kernel) & set(exc_by_kernel))
    if not paired:
        return None, None, None

    def geomean(vals: list[float]) -> float:
        return math.exp(sum(math.log(max(v, 1.0)) for v in vals) / len(vals))

    return (
        geomean([jit_by_kernel[k]                    for k in paired]),
        geomean([exc_by_kernel[k]                    for k in paired]),
        geomean([jit_by_kernel[k] + exc_by_kernel[k] for k in paired]),
    )


# ---------------------------------------------------------------------------
# Trial execution
# ---------------------------------------------------------------------------

def run_trial(
    binary: str,
    magic: dict,
    benchmark_names: list[str],
    unspec_by_kernel: dict[str, float],
    timeout: float,
) -> tuple[dict[str, float], dict[str, float], dict | None, bool]:
    """Run each benchmark individually with independent timeout isolation.

    Returns (jit_by_kernel, exc_by_kernel, merged_json, used_fallback).
    Timed-out benchmarks fall back to: jit → timeout_ns, exec → unspec_ns[kernel].
    """
    env = _make_env(magic)
    mult_map = {"ns": 1.0, "us": 1e3, "ms": 1e6, "s": 1e9}
    timeout_ns = timeout * 1e9

    jit_by_kernel: dict[str, float] = {}
    exc_by_kernel: dict[str, float] = {}
    used_fallback = False
    all_benchmarks: list[dict] = []
    first_context: dict | None = None

    for name in benchmark_names:
        kv = _parse_bm_name(name)
        kernel = kv.get("kv_n", "")
        phase = kv.get("kv_t", "")

        json_data = run_single_benchmark(binary, name, env, timeout)

        if json_data is not None:
            if first_context is None:
                first_context = json_data.get("context", {})
            for b in json_data.get("benchmarks", []):
                all_benchmarks.append(b)
                if b.get("run_type") != "iteration":
                    continue
                bkv = _parse_bm_name(b.get("name", ""))
                bkernel = bkv.get("kv_n", "")
                bphase = bkv.get("kv_t", "")
                if not bkernel or not bphase:
                    continue
                ns = b.get("real_time", 0.0) * mult_map.get(b.get("time_unit", "ns"), 1.0)
                if bphase == "jit_overhead":
                    jit_by_kernel[bkernel] = ns
                elif bphase == "specialized_exec":
                    exc_by_kernel[bkernel] = ns
        elif kernel:
            used_fallback = True
            if phase == "jit_overhead":
                jit_by_kernel[kernel] = timeout_ns
            elif phase == "specialized_exec":
                exc_by_kernel[kernel] = unspec_by_kernel.get(kernel, timeout_ns)

    merged = (
        {"context": first_context or {}, "benchmarks": all_benchmarks}
        if all_benchmarks else None
    )
    return jit_by_kernel, exc_by_kernel, merged, used_fallback


# ---------------------------------------------------------------------------
# Objective closure for optuna
# ---------------------------------------------------------------------------

def make_objective(
    binary: str,
    db_path: Path,
    study_name: str,
    args,
    benchmark_names: list[str],
    unspec_by_kernel: dict[str, float],
    lock: threading.Lock,
    git_sha: str,
):
    def objective(trial: optuna.Trial) -> float:
        # large_mod=0 is a meaningful special value (treat all modules as large →
        # conservative unroll always). Handle it separately from the log-scale range.
        large_mod_off = trial.suggest_categorical("large_mod_off", [True, False])
        large_mod_val = (
            0 if large_mod_off
            else trial.suggest_int("large_mod_val", 1, args.large_mod_hi, log=True)
        )
        magic = {
            "fixpoint":    trial.suggest_int(        "fixpoint",    0,  args.fixpoint_hi),
            "unroll":      trial.suggest_int(        "unroll",      1,  args.unroll_hi, log=True),
            "large_mod":   large_mod_val,
            "early_prune": trial.suggest_categorical("early_prune", [0, 1]),
            "o3_final":    trial.suggest_categorical("o3_final",    [0, 1]),
        }

        jit_by_kernel, exc_by_kernel, merged_json, used_fallback = run_trial(
            binary, magic, benchmark_names, unspec_by_kernel, args.timeout,
        )
        obj_jit, obj_exec, obj_combined = _geomean_from_dicts(jit_by_kernel, exc_by_kernel)

        if obj_combined is None:
            # No parseable kernel data at all — should not happen in normal use
            print(f"  Trial {trial.number}: warning: no paired kernel data", flush=True)
            obj_combined = 1e12

        run_id = None
        with lock:
            try:
                con = duckdb.connect(str(db_path))
                try:
                    con.begin()
                    if merged_json is not None:
                        run_id = store_raw_benchmarks(con, merged_json, git_sha)
                    store_trial_params(con, study_name, trial.number, run_id, magic,
                                       used_fallback, obj_jit, obj_exec, obj_combined)
                    con.commit()
                finally:
                    con.close()
            except Exception as e:
                print(f"  Trial {trial.number}: DB write error: {e}", flush=True)

        if obj_jit is not None:
            fallback_marker = " [FALLBACK]" if used_fallback else ""
            status = (f"combined={obj_combined/1e6:.1f}ms  "
                      f"jit={obj_jit/1e6:.1f}ms  exec={obj_exec/1e6:.1f}ms{fallback_marker}")
        else:
            status = "NO_DATA"
        print(
            f"  Trial {trial.number:3d}: {status} | "
            f"fixpoint={magic['fixpoint']:2d} unroll={magic['unroll']:4d} "
            f"large_mod={magic['large_mod']:6d} "
            f"early_prune={magic['early_prune']} o3_final={magic['o3_final']}",
            flush=True,
        )
        return float(obj_combined)

    return objective


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def parse_args():
    parser = argparse.ArgumentParser(
        description="Find optimal CRS JIT pipeline options by minimizing jit+exec time."
    )
    parser.add_argument("binary", help="Path to the compiled SpecializerBenchmark binary.")
    parser.add_argument("--db", default=None, metavar="PATH",
                        help="DuckDB path (overrides BENCHPLOT_DB_PATH; default: CWD/benchmarks.duckdb).")
    parser.add_argument("--study-name", default=None, metavar="STR",
                        help="Study name (default: direct_opts_YYYYMMDD_HHMMSS).")
    parser.add_argument("--n-trials", type=int, default=80, metavar="N",
                        help="Total number of trials (default: 80).")
    parser.add_argument("--n-parallel", type=int, default=4, metavar="N",
                        help="Concurrent trials (default: 4).")
    parser.add_argument("--timeout", type=float, default=60.0, metavar="SEC",
                        help="Per-benchmark subprocess timeout in seconds (default: 60).")
    parser.add_argument("--benchmark-filter", default="jit_overhead|specialized_exec",
                        metavar="PATTERN",
                        help="--benchmark_filter passed to binary "
                             "(default: 'jit_overhead|specialized_exec').")
    parser.add_argument("--output-best", default=None, metavar="PATH",
                        help="Write best config to JSON (default: best_<study_name>.json).")
    parser.add_argument("--fixpoint-hi",  type=int, default=30,     metavar="N",
                        help="Upper bound for MaxFixpointIterations (default: 30).")
    parser.add_argument("--unroll-hi",    type=int, default=512,    metavar="N",
                        help="Upper bound for LoopUnrollCount (default: 512).")
    parser.add_argument("--large-mod-hi", type=int, default=100000, metavar="N",
                        help="Upper bound for LargeModuleInstrThreshold (default: 100000).")
    parser.add_argument("--seed", type=int, default=None, metavar="INT",
                        help="Random seed for TPE sampler (default: no seed — non-deterministic).")
    return parser.parse_args()


def validate_args(args) -> None:
    errors = []
    if args.fixpoint_hi < 0:
        errors.append("--fixpoint-hi must be >= 0")
    if args.unroll_hi < 1:
        errors.append("--unroll-hi must be >= 1")
    if args.large_mod_hi < 1:
        errors.append("--large-mod-hi must be >= 1")
    if errors:
        for e in errors:
            print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    args = parse_args()
    validate_args(args)

    if not Path(args.binary).exists():
        print(f"Error: binary not found: {args.binary}", file=sys.stderr)
        sys.exit(1)

    binary = str(Path(args.binary).resolve())
    db_path = resolve_db_path(args.db)
    study_name = args.study_name or f"direct_opts_{datetime.now():%Y%m%d_%H%M%S}"
    output_best = args.output_best or f"best_{study_name}.json"
    git_sha = get_git_sha()

    # Enumerate benchmarks to run (once per study)
    print("Listing benchmarks...", flush=True)
    try:
        benchmark_names = list_benchmarks(binary, args.benchmark_filter)
    except Exception as e:
        print(f"Error: failed to list benchmarks: {e}", file=sys.stderr)
        sys.exit(1)
    if not benchmark_names:
        print(f"Error: no benchmarks matched filter '{args.benchmark_filter}'", file=sys.stderr)
        sys.exit(1)
    print(f"  {len(benchmark_names)} benchmarks matched.", flush=True)

    # Measure unspecialized baseline (3 reps, median; used as exec fallback on timeout)
    print("Measuring unspecialized baseline (3 repetitions)...", flush=True)
    try:
        unspec_by_kernel = measure_unspecialized(binary, args.timeout)
    except RuntimeError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)
    print(f"  {len(unspec_by_kernel)} kernels measured.", flush=True)

    # Initialize DB and persist baselines
    con = open_optim_db(db_path)
    try:
        con.begin()
        store_unspec_baselines(con, study_name, unspec_by_kernel)
        con.commit()
    finally:
        con.close()

    # Write session start (FR-027); updated to 'complete' on normal exit
    try:
        with duckdb.connect(str(db_path)) as _sess:
            _sess.execute(
                "INSERT INTO optimization_sessions "
                "(study_name, binary, n_trials, started_at, status) "
                "VALUES (?, ?, ?, ?, 'incomplete')",
                [study_name, binary, args.n_trials, datetime.now()],
            )
    except Exception as e:
        print(f"Warning: could not write session record: {e}", file=sys.stderr)

    lock = threading.Lock()

    sampler = optuna.samplers.TPESampler(seed=args.seed)
    study = optuna.create_study(
        direction="minimize",
        sampler=sampler,
        study_name=study_name,
    )

    print(f"\nStudy:   {study_name}")
    print(f"DB:      {db_path}")
    print(f"Binary:  {binary}")
    print(f"Trials:  {args.n_trials}  Parallel: {args.n_parallel}  "
          f"Timeout/benchmark: {args.timeout}s")
    print(f"Filter:  {args.benchmark_filter}")
    print(f"Bounds:  fixpoint=[0,{args.fixpoint_hi}]  unroll=[1,{args.unroll_hi}] (log)  "
          f"large_mod=[0,{args.large_mod_hi}] (log+zero)")
    print()

    objective = make_objective(
        binary=binary,
        db_path=db_path,
        study_name=study_name,
        args=args,
        benchmark_names=benchmark_names,
        unspec_by_kernel=unspec_by_kernel,
        lock=lock,
        git_sha=git_sha,
    )

    _interrupted = False
    try:
        study.optimize(objective, n_trials=args.n_trials, n_jobs=args.n_parallel)
    except KeyboardInterrupt:
        _interrupted = True
        print("\nOptimization interrupted; completed trials have been saved.", flush=True)
        print(f"Study '{study_name}' is marked 'incomplete' in the data store.", flush=True)

    if not _interrupted:
        try:
            with duckdb.connect(str(db_path)) as _sess:
                _sess.execute(
                    "UPDATE optimization_sessions "
                    "SET status = 'complete', completed_at = ? WHERE study_name = ?",
                    [datetime.now(), study_name],
                )
        except Exception as e:
            print(f"Warning: could not finalise session status: {e}", file=sys.stderr)

    # Report best trial (geomean recomputed from per-kernel rows via v_optim_results)
    con = duckdb.connect(str(db_path))
    try:
        best_row = con.execute(
            """
            SELECT trial_id, fixpoint_max, unroll_max, large_module_max,
                   early_prune, o3_final,
                   obj_jit_ns, obj_exec_ns, obj_combined_ns
            FROM optim_trial_params
            WHERE study_name = ?
              AND obj_combined_ns IS NOT NULL
            ORDER BY obj_combined_ns
            LIMIT 1
            """,
            [study_name],
        ).fetchone()

        breakeven_rows = []
        if best_row is not None:
            best_trial_id = best_row[0]
            breakeven_rows = con.execute(
                """
                SELECT kernel, t_jit_ns, t_spec_ns, unspec_ns, break_even_calls
                FROM v_optim_breakeven
                WHERE study_name = ? AND trial_id = ?
                ORDER BY kernel
                """,
                [study_name, best_trial_id],
            ).fetchall()
    finally:
        con.close()

    if best_row is None:
        print("\nNo successful trials recorded.")
        return

    cols = ["trial_id", "fixpoint_max", "unroll_max", "large_module_max",
            "early_prune", "o3_final", "obj_jit_ns", "obj_exec_ns", "obj_combined_ns"]
    best = dict(zip(cols, best_row))

    with open(output_best, "w") as f:
        json.dump(best, f, indent=2)

    print(f"\nStudy '{study_name}' complete: {args.n_trials} trials.")
    print(f"Best config written to: {output_best}")
    print()
    print(f"  trial:       {best['trial_id']}")
    print(f"  fixpoint:    {best['fixpoint_max']}")
    print(f"  unroll:      {best['unroll_max']}")
    print(f"  large_mod:   {best['large_module_max']}")
    print(f"  early_prune: {best['early_prune']}")
    print(f"  o3_final:    {best['o3_final']}")
    print(f"  jit:         {best['obj_jit_ns']/1e6:.2f} ms  (geomean)")
    print(f"  exec:        {best['obj_exec_ns']/1e6:.2f} ms  (geomean)")
    print(f"  combined:    {best['obj_combined_ns']/1e6:.2f} ms  (geomean)")

    if breakeven_rows:
        print()
        print("Break-even per kernel (calls needed for specialization to pay off):")
        for kernel, jit_ns, spec_ns, unspec_ns, bec in breakeven_rows:
            speedup = (unspec_ns / spec_ns) if spec_ns and spec_ns > 0 else None
            speedup_str = f"  ({speedup:.2f}x speedup)" if speedup else ""
            bec_str = f"{bec:.0f}" if bec is not None else "never (no exec speedup)"
            print(f"  {kernel:<30} {bec_str:>12} calls{speedup_str}")

    print()
    print("SQL to explore results:")
    print(f"  SELECT * FROM v_optim_best_per_kernel WHERE study_name='{study_name}' ORDER BY kernel;")
    print(f"  SELECT * FROM v_optim_breakeven      WHERE study_name='{study_name}' ORDER BY kernel;")


if __name__ == "__main__":
    main()
