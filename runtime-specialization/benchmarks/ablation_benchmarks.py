#!/usr/bin/env python3
"""ablation_benchmarks.py — Run named pipeline configs × N reps and store results.

Each (config, rep) is one binary invocation (all matched benchmarks in one run).
Results are stored to the ablation_studies table for later analysis via v_ablation_medians.

Usage:
    ablation_benchmarks.py BINARY [options]
"""

import argparse
import json
import os
import shutil
import subprocess
import sys
import tempfile
from datetime import datetime
from pathlib import Path

import duckdb

sys.path.insert(0, str(Path(__file__).parent))

from record_benchmark import (
    _pick_benchmark_cpus,
    best_practice_env,
    open_db as _rb_open_db,
    resolve_db_path,
    get_git_sha,
)
from optimize_benchmarks import store_raw_benchmarks

# ---------------------------------------------------------------------------
# Built-in configs
# ---------------------------------------------------------------------------

BUILTIN_CONFIGS = [
    {"name": "default",     "env": {}},
    {"name": "o3_only",     "env": {"CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS": "1",
                                    "CRS_DEFAULT_EARLY_PRUNE": "0",
                                    "CRS_DEFAULT_O3_FINAL": "1"}},
    {"name": "no_prune",    "env": {"CRS_DEFAULT_EARLY_PRUNE": "0"}},
    {"name": "no_o3_final", "env": {"CRS_DEFAULT_O3_FINAL": "0"}},
    {"name": "no_unroll",   "env": {"CRS_DEFAULT_LOOP_UNROLL_COUNT": "1"}},
    # fixpoint_1 excluded: fixpoint_max=1 + early_prune=1 (default) + o3_final=1 (default) → SIGSEGV (RQ-15)
    {"name": "fixpoint_2",  "env": {"CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS": "2"}},
    # pipeline_1 excluded: CRS_DEFAULT_PIPELINE=1 causes core dump on full UC filter (RQ-11)

    {"name": "aggressive",  "env": {"CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS": "20",
                                    "CRS_DEFAULT_LOOP_UNROLL_COUNT": "256"}},
]

# ---------------------------------------------------------------------------
# DB helpers
# ---------------------------------------------------------------------------

def open_ablation_db(db_path: Path) -> duckdb.DuckDBPyConnection:
    """Open existing DB and ensure ablation schema exists."""
    return _rb_open_db(db_path)


def store_ablation_row(
    con: duckdb.DuckDBPyConnection,
    study_name: str,
    config_name: str,
    rep: int,
    params_json: str,
    run_id: "str | None",
) -> None:
    """Insert one row into ablation_studies."""
    con.execute(
        "INSERT INTO ablation_studies (study_name, config_name, rep, params_json, run_id) "
        "VALUES (?, ?, ?, ?, ?)",
        [study_name, config_name, rep, params_json, run_id],
    )


# ---------------------------------------------------------------------------
# Run logic
# ---------------------------------------------------------------------------

def run_config_rep(
    binary: str,
    config: dict,
    benchmark_filter: str,
    timeout: float,
    env_base: dict,
    benchmark_cpus: "list[int] | None" = None,
) -> "dict | None":
    """Run binary with config env vars injected; return parsed JSON or None."""
    env = {**env_base, **config["env"]}
    fd, out_json = tempfile.mkstemp(suffix=".json")
    os.close(fd)
    cmd = [
        binary,
        "--benchmark_out_format=json",
        f"--benchmark_out={out_json}",
        f"--benchmark_filter={benchmark_filter}",
        "--benchmark_repetitions=1",
    ]
    if benchmark_cpus:
        taskset_bin = shutil.which("taskset") or "taskset"
        cpu_list_str = ",".join(str(c) for c in benchmark_cpus)
        cmd = [taskset_bin, "-c", cpu_list_str] + cmd
    try:
        subprocess.run(cmd, env=env, timeout=timeout, check=True, capture_output=True)
        with open(out_json) as f:
            return json.load(f)
    except Exception as e:
        print(f"  Warning: run failed: {e}")
        return None
    finally:
        Path(out_json).unlink(missing_ok=True)


# ---------------------------------------------------------------------------
# Summary helpers
# ---------------------------------------------------------------------------

def _query_config_summary(
    con: duckdb.DuckDBPyConnection,
    study_name: str,
    config_name: str,
) -> "tuple[float|None, float|None, float|None]":
    """Return (median_jit_ms, median_spec_ms, median_unspec_ms) for a config across its reps."""
    row = con.execute(
        """
        SELECT med_jit_ns, med_spec_ns, med_unspec_ns
        FROM v_ablation_medians
        WHERE study_name = ? AND config_name = ?
        LIMIT 1
        """,
        [study_name, config_name],
    ).fetchone()
    if row is None:
        return None, None, None
    jit_ms = row[0] / 1e6 if row[0] is not None else None
    spec_ms = row[1] / 1e6 if row[1] is not None else None
    unspec_ms = row[2] / 1e6 if row[2] is not None else None
    return jit_ms, spec_ms, unspec_ms


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def parse_args():
    parser = argparse.ArgumentParser(
        description="Run named pipeline configs × N reps and store ablation results."
    )
    parser.add_argument("binary", help="Path to the compiled SpecializerBenchmark binary.")
    parser.add_argument(
        "--db", default=None, metavar="PATH",
        help="DuckDB path (overrides BENCHPLOT_DB_PATH; default: CWD/benchmarks.duckdb).",
    )
    parser.add_argument(
        "--study-name", default=None, metavar="STR",
        help="Study label (default: ablation_YYYYMMDD_HHMMSS).",
    )
    parser.add_argument(
        "--configs", default=None, metavar="PATH", type=Path,
        help="JSON file listing named configs (default: built-in 8).",
    )
    parser.add_argument(
        "--extra-config", default=None, metavar="JSON_STR",
        help="Append one additional config object at runtime (JSON string).",
    )
    parser.add_argument(
        "--benchmark-filter", default="jit_overhead|specialized_exec", metavar="PATTERN",
        help="Forwarded to benchmark binary as --benchmark_filter "
             "(default: 'jit_overhead|specialized_exec').",
    )
    parser.add_argument(
        "--reps", type=int, default=3, metavar="N",
        help="Repetitions per config (default: 3).",
    )
    parser.add_argument(
        "--timeout", type=float, default=300.0, metavar="SEC",
        help="Per-run timeout in seconds (default: 300).",
    )
    parser.add_argument(
        "--benchmarking-best-practice", action="store_true",
        help="Apply thesis-grade best-practice benchmarking controls "
             "(ASLR disable, Turbo disable, performance governor, SMT sibling off, taskset affinity).",
    )
    parser.add_argument(
        "--benchmark-cpus", default=None, metavar="LIST",
        help="CPU list for --benchmarking-best-practice, e.g. '2,4'. "
             "Default: auto-pick dedicated benchmark CPUs.",
    )
    return parser.parse_args()


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    args = parse_args()

    if not Path(args.binary).exists():
        print(f"Error: binary not found: {args.binary}", file=sys.stderr)
        sys.exit(1)

    binary = str(Path(args.binary).resolve())
    db_path = resolve_db_path(args.db)
    if not db_path.exists():
        print(f"Error: DB file not found: {db_path}", file=sys.stderr)
        print("Run create_db.py to initialise a new database.", file=sys.stderr)
        sys.exit(1)

    study_name = args.study_name or f"ablation_{datetime.now():%Y%m%d_%H%M%S}"
    git_sha = get_git_sha()

    # Load configs
    if args.configs is not None:
        with open(args.configs) as f:
            configs = json.load(f)
        if not isinstance(configs, list):
            print("Error: --configs JSON must be a list of config objects.", file=sys.stderr)
            sys.exit(1)
    else:
        configs = list(BUILTIN_CONFIGS)

    if args.extra_config is not None:
        try:
            extra = json.loads(args.extra_config)
        except json.JSONDecodeError as e:
            print(f"Error: --extra-config is not valid JSON: {e}", file=sys.stderr)
            sys.exit(1)
        configs.append(extra)

    print(f"\nStudy:   {study_name}")
    print(f"DB:      {db_path}")
    print(f"Binary:  {binary}")
    print(f"Configs: {len(configs)}  Reps: {args.reps}  Timeout: {args.timeout}s")
    print(f"Filter:  {args.benchmark_filter}")
    print()

    env_base = os.environ.copy()
    benchmark_cpus = None
    if args.benchmarking_best_practice:
        requested_cpus = None
        if args.benchmark_cpus is not None:
            requested_cpus = [
                int(part.strip()) for part in args.benchmark_cpus.split(",") if part.strip()
            ]
        benchmark_cpus = _pick_benchmark_cpus(requested_cpus)
    total_runs = len(configs) * args.reps
    completed = 0
    failed = 0

    def run_all_configs(best_practice_full: bool) -> None:
        nonlocal completed, failed
        for config in configs:
            config_name = config["name"]
            config_failures = 0
            print(f"Config [{config_name}]  (env: {config['env'] or 'default'})")

            for rep in range(args.reps):
                print(f"  [{config_name}] rep {rep + 1}/{args.reps}...", end=" ", flush=True)

                data = run_config_rep(
                    binary=binary,
                    config=config,
                    benchmark_filter=args.benchmark_filter,
                    timeout=args.timeout,
                    env_base=env_base,
                    benchmark_cpus=benchmark_cpus if best_practice_full else None,
                )

                run_id = None
                if data is not None:
                    try:
                        con = duckdb.connect(str(db_path))
                        try:
                            con.begin()
                            run_id = store_raw_benchmarks(
                                con,
                                data,
                                git_sha,
                                best_practice_full=best_practice_full,
                            )
                            store_ablation_row(
                                con, study_name, config_name, rep,
                                json.dumps(config["env"]), run_id,
                            )
                            con.commit()
                        finally:
                            con.close()
                        print("OK")
                    except Exception as e:
                        print(f"DB error: {e}")
                        config_failures += 1
                        failed += 1
                        run_id = None
                else:
                    # Store NULL run_id row so the study record is complete
                    try:
                        con = duckdb.connect(str(db_path))
                        try:
                            con.begin()
                            store_ablation_row(
                                con, study_name, config_name, rep,
                                json.dumps(config["env"]), None,
                            )
                            con.commit()
                        finally:
                            con.close()
                    except Exception as e:
                        print(f"DB error (storing NULL row): {e}")
                    config_failures += 1
                    failed += 1

                completed += 1

            ok_count = args.reps - config_failures
            try:
                con = duckdb.connect(str(db_path))
                try:
                    jit_ms, spec_ms, unspec_ms = _query_config_summary(
                        con, study_name, config_name
                    )
                finally:
                    con.close()
            except Exception:
                jit_ms = spec_ms = unspec_ms = None

            summary_parts = [f"  [{config_name:<14}]  {ok_count}/{args.reps} reps OK"]
            if jit_ms is not None:
                summary_parts.append(f"jit={jit_ms:.1f}ms")
            if spec_ms is not None:
                summary_parts.append(f"spec={spec_ms:.1f}ms")
            if unspec_ms is not None:
                summary_parts.append(f"unspec={unspec_ms:.1f}ms")
            if spec_ms is not None and unspec_ms is not None and spec_ms > 0:
                speedup = unspec_ms / spec_ms
                summary_parts.append(f"speedup={speedup:.2f}x")
            print("  ".join(summary_parts))
            print()

    if args.benchmarking_best_practice:
        with best_practice_env(benchmark_cpus) as setup_ok:
            run_all_configs(best_practice_full=bool(setup_ok))
    else:
        run_all_configs(best_practice_full=False)

    print(f"Done. {completed} runs ({completed - failed} OK, {failed} failed).")
    print(f"Study '{study_name}' stored in {db_path}")
    print()
    print("SQL to explore results:")
    print(f"  SELECT * FROM v_ablation_medians WHERE study_name='{study_name}' ORDER BY config_name, kernel;")


if __name__ == "__main__":
    main()
