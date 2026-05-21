#!/usr/bin/env python3
"""sensitivity_analysis.py — OAT (one-at-a-time) parameter sensitivity sweep.

Takes an optimal config JSON (from optimize_benchmarks.py), sweeps one parameter
over a list of values, runs each point with N reps, and stores results to
ablation_studies with config names following sens_{param}_{value}.

Usage:
    sensitivity_analysis.py BINARY [options]
"""

import argparse
import json
import os
import sys
from datetime import datetime
from pathlib import Path

import duckdb

sys.path.insert(0, str(Path(__file__).parent))

from record_benchmark import (
    resolve_db_path,
    get_git_sha,
)
from optimize_benchmarks import store_raw_benchmarks
from ablation_benchmarks import (
    open_ablation_db,
    run_config_rep,
    store_ablation_row,
    _query_config_summary,
)

# ---------------------------------------------------------------------------
# Parameter → env var mapping (mirrors optimize_benchmarks.py DEFAULT_SEARCH_SPACE)
# ---------------------------------------------------------------------------

PARAM_ENV_MAP = {
    "fixpoint_max":          "CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS",
    "unroll_max":            "CRS_DEFAULT_LOOP_UNROLL_COUNT",
    "large_module_max":      "CRS_DEFAULT_LARGE_MODULE_INSTR_THRESHOLD",
    "early_prune":           "CRS_DEFAULT_EARLY_PRUNE",
    "o3_final":              "CRS_DEFAULT_O3_FINAL",
    "pipeline":              "CRS_DEFAULT_PIPELINE",
    "p1_inline_threshold":   "CRS_DEFAULT_P1_INLINE_THRESHOLD",
    "p1_max_module_growth":  "CRS_DEFAULT_P1_MAX_MODULE_GROWTH",
}


def params_to_env(params: dict) -> dict:
    """Convert an optimal params dict to an env var dict."""
    return {PARAM_ENV_MAP[k]: str(v) for k, v in params.items() if k in PARAM_ENV_MAP}


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def parse_args():
    parser = argparse.ArgumentParser(
        description="OAT parameter sweep: fix optimal config, vary one parameter."
    )
    parser.add_argument("binary", help="Path to the compiled SpecializerBenchmark binary.")
    parser.add_argument(
        "--db", default=None, metavar="PATH",
        help="DuckDB path (overrides BENCHPLOT_DB_PATH; default: CWD/benchmarks.duckdb).",
    )
    parser.add_argument(
        "--study-name", default=None, metavar="STR",
        help="Study label (default: sens_PARAM_YYYYMMDD_HHMMSS).",
    )
    parser.add_argument(
        "--optimal-config", required=True, metavar="PATH", type=Path,
        help="JSON file from optimize_benchmarks.py (best_<study>.json).",
    )
    parser.add_argument(
        "--param", required=True, metavar="NAME",
        help=f"Parameter to sweep. Known params: {', '.join(sorted(PARAM_ENV_MAP))}.",
    )
    parser.add_argument(
        "--sweep-values", required=True, metavar="V1,V2,...",
        help="Comma-separated values to sweep over.",
    )
    parser.add_argument(
        "--benchmark-filter", default="jit_overhead|specialized_exec", metavar="PATTERN",
        help="Forwarded to benchmark binary as --benchmark_filter "
             "(default: 'jit_overhead|specialized_exec').",
    )
    parser.add_argument(
        "--reps", type=int, default=1, metavar="N",
        help="Repetitions per sweep point (default: 1).",
    )
    parser.add_argument(
        "--timeout", type=float, default=300.0, metavar="SEC",
        help="Per-run timeout in seconds (default: 300).",
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

    if args.param not in PARAM_ENV_MAP:
        known = ", ".join(sorted(PARAM_ENV_MAP))
        print(f"Error: unknown parameter {args.param!r}. Known params: {known}", file=sys.stderr)
        sys.exit(1)

    if not args.optimal_config.exists():
        print(f"Error: optimal config not found: {args.optimal_config}", file=sys.stderr)
        sys.exit(1)

    binary = str(Path(args.binary).resolve())
    db_path = resolve_db_path(args.db)
    if not db_path.exists():
        print(f"Error: DB file not found: {db_path}", file=sys.stderr)
        print("Run create_db.py to initialise a new database.", file=sys.stderr)
        sys.exit(1)

    # Load optimal config
    with open(args.optimal_config) as f:
        optimal = json.load(f)

    optimal_params = optimal.get("params", {})
    base_env = params_to_env(optimal_params)

    # Parse sweep values (kept as strings; the env vars are always strings)
    sweep_values = [v.strip() for v in args.sweep_values.split(",") if v.strip()]
    if not sweep_values:
        print("Error: --sweep-values produced an empty list.", file=sys.stderr)
        sys.exit(1)

    study_name = args.study_name or f"sens_{args.param}_{datetime.now():%Y%m%d_%H%M%S}"
    git_sha = get_git_sha()

    # Build configs: one per sweep value
    configs = []
    for value in sweep_values:
        config_name = f"sens_{args.param}_{value}"
        config_env = {**base_env, PARAM_ENV_MAP[args.param]: value}
        configs.append({"name": config_name, "env": config_env, "_sweep_value": value})

    print(f"\nStudy:   {study_name}")
    print(f"DB:      {db_path}")
    print(f"Binary:  {binary}")
    print(f"Param:   {args.param}  ({PARAM_ENV_MAP[args.param]})")
    print(f"Values:  {sweep_values}")
    print(f"Points:  {len(configs)}  Reps: {args.reps}  Timeout: {args.timeout}s")
    print(f"Filter:  {args.benchmark_filter}")
    print(f"Optimal: {args.optimal_config}")
    print()

    env_base = os.environ.copy()
    total_runs = len(configs) * args.reps
    completed = 0
    failed = 0

    for config in configs:
        config_name = config["name"]
        sweep_value = config["_sweep_value"]
        config_env = config["env"]
        config_failures = 0

        print(f"Point [{config_name}]  ({args.param}={sweep_value})")

        for rep in range(args.reps):
            print(f"  [{config_name}] rep {rep + 1}/{args.reps}...", end=" ", flush=True)

            data = run_config_rep(
                binary=binary,
                config={"name": config_name, "env": config_env},
                benchmark_filter=args.benchmark_filter,
                timeout=args.timeout,
                env_base=env_base,
            )

            run_id = None
            if data is not None:
                try:
                    con = duckdb.connect(str(db_path))
                    try:
                        con.begin()
                        run_id = store_raw_benchmarks(con, data, git_sha)
                        store_ablation_row(
                            con, study_name, config_name, rep,
                            json.dumps(config_env), run_id,
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
                            json.dumps(config_env), None,
                        )
                        con.commit()
                    finally:
                        con.close()
                except Exception as e:
                    print(f"DB error (storing NULL row): {e}")
                config_failures += 1
                failed += 1

            completed += 1

        # Per-point summary
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

        summary_parts = [f"  [{config_name:<30}]  {ok_count}/{args.reps} reps OK"]
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

    print(f"Done. {completed} runs ({completed - failed} OK, {failed} failed).")
    print(f"Study '{study_name}' stored in {db_path}")
    print()
    print("SQL to explore results:")
    print(f"  SELECT * FROM v_ablation_medians WHERE study_name='{study_name}' ORDER BY config_name, kernel;")


if __name__ == "__main__":
    main()
