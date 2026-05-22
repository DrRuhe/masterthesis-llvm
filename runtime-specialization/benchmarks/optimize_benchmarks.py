#!/usr/bin/env python3
"""optimize_benchmarks.py — Find optimal JIT pipeline Options for a given workload.

Minimizes jit_overhead_ns + specialized_exec_ns (per-kernel, then geomean across kernels)
by tuning MaxFixpointIterations, LoopUnrollCount, LargeModuleInstrThreshold,
EnableEarlyPrune, and EnableO3Final via ENV var overrides on Options::Default().

All benchmarks for a trial are run in a single subprocess invocation to amortize expensive
CRS initialization (e.g. AllBenchmarks loads 65 blobs / 63MB bitcode). If the entire
subprocess times out or crashes, all kernels receive fallback costs
(timeout_ns for jit, unspecialized_ns for exec).

Usage:
    optimize_benchmarks.py BINARY [options]
"""
import argparse
import json
import math
import os
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

# ---------------------------------------------------------------------------
# Built-in default search-space descriptor (version 2)
# ---------------------------------------------------------------------------

DEFAULT_SEARCH_SPACE = {
    "version": 2,
    "parameters": [
        {"name": "fixpoint_max",          "env_var": "CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS",     "type": "int",        "min": 2,    "max": 30     },
        {"name": "unroll_max",            "env_var": "CRS_DEFAULT_LOOP_UNROLL_COUNT",            "type": "log_int",    "min": 1,    "max": 512    },
        {"name": "large_module_max",      "env_var": "CRS_DEFAULT_LARGE_MODULE_INSTR_THRESHOLD", "type": "int_or_zero","min": 1,    "max": 100000 },
        {"name": "early_prune",           "env_var": "CRS_DEFAULT_EARLY_PRUNE",                 "type": "bool"                                   },
        {"name": "o3_final",              "env_var": "CRS_DEFAULT_O3_FINAL",                    "type": "bool"                                   },
        {"name": "pipeline",              "env_var": "CRS_DEFAULT_PIPELINE",                    "type": "bool"                                   },
        {"name": "p1_inline_threshold",   "env_var": "CRS_DEFAULT_P1_INLINE_THRESHOLD",         "type": "log_int",    "min": 50,   "max": 2000   },
        {"name": "p1_max_module_growth",  "env_var": "CRS_DEFAULT_P1_MAX_MODULE_GROWTH",        "type": "float",      "min": 1.0,  "max": 5.0    },
    ],
}

from record_benchmark import (
    open_db as _rb_open_db,
    ensure_columns,
    insert_benchmarks,
    resolve_db_path,
    _parse_bm_name,
    get_git_sha,
    refresh_views,
)


# ---------------------------------------------------------------------------
# Descriptor helpers
# ---------------------------------------------------------------------------

def _load_descriptor(path: "Path | None") -> dict:
    """Load and validate a search-space descriptor JSON file, or return the built-in default."""
    if path is None:
        return DEFAULT_SEARCH_SPACE
    with open(path) as f:
        d = json.load(f)
    if d.get("version") not in (1, 2):
        raise SystemExit(f"Unsupported search-space descriptor version: {d.get('version')!r}")
    seen_names, seen_envvars = set(), set()
    valid_types = {"int", "log_int", "float", "log_float", "bool", "categorical", "int_or_zero"}
    for p in d.get("parameters", []):
        if p["name"] in seen_names:
            raise SystemExit(f"Duplicate parameter name in descriptor: {p['name']!r}")
        if p["env_var"] in seen_envvars:
            raise SystemExit(f"Duplicate env_var in descriptor: {p['env_var']!r}")
        if p["type"] not in valid_types:
            raise SystemExit(f"Unknown parameter type {p['type']!r} for parameter {p['name']!r}")
        seen_names.add(p["name"])
        seen_envvars.add(p["env_var"])
    return d


def _sample_params(trial: "optuna.Trial", descriptor: dict) -> dict:
    """Sample one set of parameters from an Optuna trial using the descriptor."""
    params = {}
    for p in descriptor["parameters"]:
        name, typ = p["name"], p["type"]
        if typ == "int":
            params[name] = trial.suggest_int(name, p["min"], p["max"])
        elif typ == "log_int":
            params[name] = trial.suggest_int(name, p["min"], p["max"], log=True)
        elif typ == "float":
            params[name] = trial.suggest_float(name, p["min"], p["max"])
        elif typ == "log_float":
            params[name] = trial.suggest_float(name, p["min"], p["max"], log=True)
        elif typ == "bool":
            params[name] = trial.suggest_categorical(name, [0, 1])
        elif typ == "categorical":
            params[name] = trial.suggest_categorical(name, p["choices"])
        elif typ == "int_or_zero":
            off = trial.suggest_categorical(f"{name}_off", [True, False])
            params[name] = 0 if off else trial.suggest_int(f"{name}_val", p["min"], p["max"], log=True)
    return params


def _params_to_env(params: dict, descriptor: dict) -> dict:
    """Build {env_var: str(value)} for subprocess injection from sampled params."""
    env_map = {p["name"]: p["env_var"] for p in descriptor["parameters"]}
    return {env_map[name]: str(value) for name, value in params.items() if name in env_map}

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
    "binary"          VARCHAR,
    n_trials          INTEGER,
    started_at        TIMESTAMP,
    completed_at      TIMESTAMP,
    status            VARCHAR,
    search_space_json JSON
);
"""

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
# DB helpers
# ---------------------------------------------------------------------------

def open_optim_db(db_path: Path) -> duckdb.DuckDBPyConnection:
    """Open an existing benchmark DB and ensure optimizer tables/views exist."""
    con = _rb_open_db(db_path)
    refresh_views(con)  # refresh base views first so optimizer views see current schema
    con.execute(_SCHEMA_OPTIM_TRIAL_PARAMS)
    con.execute(_SCHEMA_UNSPEC_BASELINES)
    con.execute(_SCHEMA_OPTIM_SESSIONS)
    con.execute(_SCHEMA_V_OPTIM_RESULTS)
    con.execute(_SCHEMA_V_OPTIM_BREAKEVEN)
    con.execute(_SCHEMA_V_OPTIM_BEST_PER_KERNEL)
    return con


def store_raw_benchmarks(con: duckdb.DuckDBPyConnection, data: dict, git_sha: str) -> str:
    """Insert context + benchmark rows from raw JSON; returns run_id."""
    ctx = data.get("context", {})
    benchmarks = data.get("benchmarks", [])
    run_id = str(uuid.uuid4())
    run_ts = datetime.now()
    con.execute(
        "INSERT INTO context VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
        [
            run_id, run_ts, git_sha,
            ctx.get("date", ""), ctx.get("host_name", ""), ctx.get("executable", ""),
            ctx.get("num_cpus"), ctx.get("mhz_per_cpu"), ctx.get("cpu_scaling_enabled"),
            ctx.get("library_version", ""), ctx.get("library_build_type", ""),
            False,  # best_practice_full — optimization trials are not best-practice runs
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
        "(study_name, trial_id, run_id, params_json, used_timeout_fallback, "
        " obj_jit_ns, obj_exec_ns, obj_combined_ns) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
        [
            study_name, trial_id, run_id,
            json.dumps(magic),
            used_timeout_fallback,
            obj_jit, obj_exec, obj_combined,
        ],
    )


def store_unspec_baselines(
    con: duckdb.DuckDBPyConnection,
    study_name: str,
    unspec_by_kernel: dict[str, float],
) -> None:
    # DuckDB has a known index limitation where in-transaction DELETEs are not
    # observed by subsequent INSERTs in the same transaction's PK index, causing
    # spurious "Duplicate key" constraint errors if rows for this study already
    # existed before this transaction began. Use UPSERT semantics instead so the
    # operation is idempotent and robust to stale rows from previous failed runs.
    for kernel, ns in unspec_by_kernel.items():
        con.execute(
            "INSERT INTO unspec_baselines (study_name, kernel, unspec_ns) VALUES (?, ?, ?) "
            "ON CONFLICT (study_name, kernel) DO UPDATE SET unspec_ns = EXCLUDED.unspec_ns",
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


def _derive_unspec_filter(filter_pattern: str) -> str:
    """Derive an unspecialized filter from the optimization filter by replacing the phase."""
    import re as _re
    # Replace t:(jit_overhead|specialized_exec) variants with t:unspecialized
    derived = _re.sub(r"t:\([^)]+\)", "t:unspecialized", filter_pattern)
    derived = _re.sub(r"t:(jit_overhead|specialized_exec)", "t:unspecialized", derived)
    if derived != filter_pattern:
        return derived
    return "unspecialized"


def measure_unspecialized(binary: str, timeout: float,
                          unspec_filter: str = "unspecialized") -> dict[str, float]:
    """Run unspecialized benchmarks once; return {kernel_name: real_time_ns}. Fatal on failure."""
    fd, out_json = tempfile.mkstemp(suffix="_unspec.json")
    os.close(fd)
    try:
        subprocess.run(
            [
                binary,
                "--benchmark_out_format=json",
                f"--benchmark_out={out_json}",
                f"--benchmark_filter={unspec_filter}",
                "--benchmark_repetitions=1",
            ],
            capture_output=True, timeout=timeout * 6, check=True,
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
        if b.get("run_type") != "iteration":
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
# Per-trial subprocess execution (single invocation for all benchmarks)
# ---------------------------------------------------------------------------

def run_all_benchmarks(
    binary: str, filter_pattern: str, env: dict, trial_timeout: float
) -> dict | None:
    """Run all matched benchmarks in one subprocess; return parsed JSON or None on failure."""
    fd, out_json = tempfile.mkstemp(suffix=".json")
    os.close(fd)
    try:
        subprocess.run(
            [
                binary,
                "--benchmark_out_format=json",
                f"--benchmark_out={out_json}",
                f"--benchmark_filter={filter_pattern}",
                "--benchmark_repetitions=1",
            ],
            env=env, timeout=trial_timeout, check=True, capture_output=True,
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
    descriptor: dict,
    benchmark_names: list[str],
    unspec_by_kernel: dict[str, float],
    timeout: float,
    filter_pattern: str,
) -> tuple[dict[str, float], dict[str, float], dict | None, bool]:
    """Run all benchmarks in a single subprocess per trial.

    Total trial timeout = timeout * num_benchmarks + 30s to amortize initialization.
    Returns (jit_by_kernel, exc_by_kernel, merged_json, used_fallback).
    On total failure, applies fallback: jit → timeout_ns, exec → unspec_ns[kernel].
    """
    env = {**os.environ, **_params_to_env(magic, descriptor)}
    mult_map = {"ns": 1.0, "us": 1e3, "ms": 1e6, "s": 1e9}
    timeout_ns = timeout * 1e9
    trial_timeout = timeout * max(len(benchmark_names), 1) + 30.0

    jit_by_kernel: dict[str, float] = {}
    exc_by_kernel: dict[str, float] = {}
    used_fallback = False

    json_data = run_all_benchmarks(binary, filter_pattern, env, trial_timeout)

    if json_data is None:
        # Entire trial failed — apply fallback to all expected kernels
        used_fallback = True
        for name in benchmark_names:
            kv = _parse_bm_name(name)
            kernel = kv.get("kv_n", "")
            phase = kv.get("kv_t", "")
            if not kernel:
                continue
            if phase == "jit_overhead":
                jit_by_kernel[kernel] = timeout_ns
            elif phase == "specialized_exec":
                exc_by_kernel[kernel] = unspec_by_kernel.get(kernel, timeout_ns)
        return jit_by_kernel, exc_by_kernel, None, used_fallback

    all_benchmarks = json_data.get("benchmarks", [])
    first_context = json_data.get("context", {})

    for b in all_benchmarks:
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

    # Apply fallback for any kernels that didn't appear in output
    expected_jit = {_parse_bm_name(n).get("kv_n", "") for n in benchmark_names
                    if _parse_bm_name(n).get("kv_t", "") == "jit_overhead"}
    expected_exec = {_parse_bm_name(n).get("kv_n", "") for n in benchmark_names
                     if _parse_bm_name(n).get("kv_t", "") == "specialized_exec"}
    for kernel in expected_jit - set(jit_by_kernel):
        if kernel:
            jit_by_kernel[kernel] = timeout_ns
            used_fallback = True
    for kernel in expected_exec - set(exc_by_kernel):
        if kernel:
            exc_by_kernel[kernel] = unspec_by_kernel.get(kernel, timeout_ns)
            used_fallback = True

    merged = {"context": first_context, "benchmarks": all_benchmarks} if all_benchmarks else None
    return jit_by_kernel, exc_by_kernel, merged, used_fallback


# ---------------------------------------------------------------------------
# Objective closure for optuna
# ---------------------------------------------------------------------------

def make_objective(
    binary: str,
    db_path: Path,
    study_name: str,
    descriptor: dict,
    args,
    benchmark_names: list[str],
    unspec_by_kernel: dict[str, float],
    lock: threading.Lock,
    git_sha: str,
    filter_pattern: str,
):
    def objective(trial: optuna.Trial) -> float:
        magic = _sample_params(trial, descriptor)

        jit_by_kernel, exc_by_kernel, merged_json, used_fallback = run_trial(
            binary, magic, descriptor, benchmark_names, unspec_by_kernel, args.timeout,
            filter_pattern,
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
        param_summary = "  ".join(f"{k}={v}" for k, v in magic.items())
        print(
            f"  Trial {trial.number:3d}: {status} | {param_summary}",
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
                        help="Per-benchmark time budget in seconds; total trial timeout = "
                             "N × num_benchmarks + 30s (default: 60).")
    parser.add_argument("--benchmark-filter", default="jit_overhead|specialized_exec",
                        metavar="PATTERN",
                        help="--benchmark_filter passed to binary "
                             "(default: 'jit_overhead|specialized_exec').")
    parser.add_argument("--output-best", default=None, metavar="PATH",
                        help="Write best config to JSON (default: best_<study_name>.json).")
    parser.add_argument("--search-space", type=Path, default=None, metavar="PATH",
                        help="JSON search-space descriptor; defaults to built-in.")
    parser.add_argument("--seed", type=int, default=None, metavar="INT",
                        help="Random seed for TPE sampler (default: no seed — non-deterministic).")
    parser.add_argument("--apply-default-filters", action="store_true", default=False,
                        help="Prepend a filter that excludes benchmarkJITAnalysis benchmarks "
                             "(names containing '_t_jit_analysis_') from the active set.")
    parser.add_argument("--reuse-unspec-baselines-from", default=None, metavar="STUDY_NAME",
                        help="Load unspecialized baselines from an existing study in the DB "
                             "instead of re-measuring them. Useful when baseline measurement "
                             "is slow (e.g., large datasets).")
    return parser.parse_args()


def validate_args(args) -> None:
    pass


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
    if not db_path.exists():
        print(f"Error: DB file not found: {db_path}", file=sys.stderr)
        print("Run create_db.py to initialise a new database.", file=sys.stderr)
        sys.exit(1)
    study_name = args.study_name or f"direct_opts_{datetime.now():%Y%m%d_%H%M%S}"
    output_best = args.output_best or f"best_{study_name}.json"
    git_sha = get_git_sha()
    descriptor = _load_descriptor(args.search_space)

    # Resolve benchmark filter. The Google Benchmark regex engine does not
    # support PCRE lookahead, so we apply default exclusions in Python after
    # listing rather than via a (?!...) regex wrapper.
    filter_pattern = args.benchmark_filter

    # Enumerate benchmarks to run (once per study)
    print("Listing benchmarks...", flush=True)
    try:
        benchmark_names = list_benchmarks(binary, filter_pattern)
    except Exception as e:
        print(f"Error: failed to list benchmarks: {e}", file=sys.stderr)
        sys.exit(1)
    if not benchmark_names:
        print(f"Error: no benchmarks matched filter '{filter_pattern}'", file=sys.stderr)
        sys.exit(1)

    if args.apply_default_filters:
        # Strip jit-analysis benchmarks (require CRS_PASS_TRACE_DIR / CRS_CHROME_TRACE_DIR
        # and are incompatible with optimization trials).
        benchmark_names = [n for n in benchmark_names if "_t_jit_analysis_" not in n]
        if not benchmark_names:
            print(f"Error: no benchmarks remain after stripping jit_analysis names "
                  f"(filter '{filter_pattern}')", file=sys.stderr)
            sys.exit(1)

    jit_analysis_names = [n for n in benchmark_names if "_t_jit_analysis_" in n]
    if jit_analysis_names:
        print("Error: the following benchmarkJITAnalysis benchmarks are in the active set:",
              file=sys.stderr)
        for name in jit_analysis_names:
            print(f"  {name}", file=sys.stderr)
        print("These benchmarks require CRS_PASS_TRACE_DIR and CRS_CHROME_TRACE_DIR to be set "
              "and are not compatible with optimization trials.", file=sys.stderr)
        print("Use --benchmark_filter to exclude them, or pass --apply-default-filters.",
              file=sys.stderr)
        sys.exit(1)

    print(f"  {len(benchmark_names)} benchmarks matched.", flush=True)

    # Measure or load unspecialized baseline
    if args.reuse_unspec_baselines_from is not None:
        print(f"Loading unspecialized baselines from study '{args.reuse_unspec_baselines_from}'...",
              flush=True)
        _src_con = duckdb.connect(str(db_path))
        try:
            rows = _src_con.execute(
                "SELECT kernel, unspec_ns FROM unspec_baselines WHERE study_name = ?",
                [args.reuse_unspec_baselines_from],
            ).fetchall()
        finally:
            _src_con.close()
        if not rows:
            print(f"Error: no baselines found for study '{args.reuse_unspec_baselines_from}'",
                  file=sys.stderr)
            sys.exit(1)
        unspec_by_kernel = {row[0]: row[1] for row in rows}
        print(f"  Loaded {len(unspec_by_kernel)} kernels: {sorted(unspec_by_kernel)}", flush=True)
    else:
        print("Measuring unspecialized baseline...", flush=True)
        unspec_filter = _derive_unspec_filter(filter_pattern)
        print(f"  Unspecialized filter: {unspec_filter}", flush=True)
        try:
            unspec_by_kernel = measure_unspecialized(binary, args.timeout, unspec_filter)
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
                '(study_name, "binary", n_trials, started_at, status, search_space_json) '
                "VALUES (?, ?, ?, ?, 'incomplete', ?)",
                [study_name, binary, args.n_trials, datetime.now(), json.dumps(descriptor)],
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
    trial_timeout_estimate = args.timeout * max(len(benchmark_names), 1) + 30.0
    print(f"Trials:  {args.n_trials}  Parallel: {args.n_parallel}  "
          f"Timeout/benchmark: {args.timeout}s  Total/trial: {trial_timeout_estimate:.0f}s")
    print(f"Filter:  {args.benchmark_filter}")
    desc_source = str(args.search_space) if args.search_space else "built-in"
    print(f"Search space: {desc_source} ({len(descriptor['parameters'])} parameters)")
    print()

    objective = make_objective(
        binary=binary,
        db_path=db_path,
        study_name=study_name,
        descriptor=descriptor,
        args=args,
        benchmark_names=benchmark_names,
        unspec_by_kernel=unspec_by_kernel,
        lock=lock,
        git_sha=git_sha,
        filter_pattern=filter_pattern,
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

    # Extract and save parameter importance (only if not interrupted and enough trials)
    if not _interrupted:
        try:
            from optuna.importance import get_param_importances
            importance = get_param_importances(study)
            importance_path = Path(output_best).parent / f"importance_{study_name}.json"
            with open(importance_path, "w") as f:
                json.dump(dict(importance), f, indent=2)
            print(f"Parameter importance written to: {importance_path}")
        except Exception as e:
            print(f"Warning: could not compute parameter importance: {e}", file=sys.stderr)

    # Report best trial (geomean recomputed from per-kernel rows via v_optim_results)
    con = duckdb.connect(str(db_path))
    try:
        best_row = con.execute(
            """
            SELECT trial_id, params_json,
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

    cols = ["trial_id", "params_json", "obj_jit_ns", "obj_exec_ns", "obj_combined_ns"]
    best = dict(zip(cols, best_row))
    params = json.loads(best["params_json"])

    with open(output_best, "w") as f:
        json.dump({"trial_id": best["trial_id"], "params": params,
                   "obj_jit_ns": best["obj_jit_ns"], "obj_exec_ns": best["obj_exec_ns"],
                   "obj_combined_ns": best["obj_combined_ns"]}, f, indent=2)

    print(f"\nStudy '{study_name}' complete: {args.n_trials} trials.")
    print(f"Best config written to: {output_best}")
    print()
    print(f"  trial: {best['trial_id']}")
    for k, v in params.items():
        print(f"  {k}: {v}")
    print(f"  jit:      {best['obj_jit_ns']/1e6:.2f} ms  (geomean)")
    print(f"  exec:     {best['obj_exec_ns']/1e6:.2f} ms  (geomean)")
    print(f"  combined: {best['obj_combined_ns']/1e6:.2f} ms  (geomean)")

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
