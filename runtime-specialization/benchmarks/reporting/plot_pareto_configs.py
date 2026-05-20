#!/usr/bin/env python3
"""plot_pareto_configs.py — Thesis-ready Pareto plots + CSV export for JIT pipeline studies.

Reads any study (optim or ablation) from benchmarks.duckdb, computes the Pareto-optimal
set in the lower-left envelope (both axes minimized: JIT overhead and specialized exec),
and writes both a PNG and a CSV.

CSV columns (FR-002b):
    config_label, fixpoint_max, unroll_max, large_module_max, early_prune, o3_final,
    group, kernel, jit_overhead_ms, specialized_exec_ms,
    is_pareto_optimal (bool), is_default (bool)

The CSV is designed to be loaded directly into pgfplots/LaTeX. No post-processing needed
to filter to Pareto-optimal or default rows.
"""
import argparse
import csv
import json
import sys
from pathlib import Path

import duckdb
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

sys.path.insert(0, str(Path(__file__).parent.parent))
from record_benchmark import resolve_db_path
from report_utils import make_report_dir

# Canonical Options::Default() values (matches _envOr defaults in ClangRuntimeSpecializer.h).
_DEFAULT_PARAMS = {
    "fixpoint_max": 10,
    "unroll_max": 128,
    "large_module_max": 50000,
    "early_prune": 1,
    "o3_final": 1,
}
_PARAM_COLS = ["fixpoint_max", "unroll_max", "large_module_max", "early_prune", "o3_final"]

# ablation_studies stores env-var keys directly; map them back to the short names used in
# optim_trial_params and in the CSV.
_ENV_TO_PARAM = {
    "CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS": "fixpoint_max",
    "CRS_DEFAULT_LOOP_UNROLL_COUNT": "unroll_max",
    "CRS_DEFAULT_LARGE_MODULE_INSTR_THRESHOLD": "large_module_max",
    "CRS_DEFAULT_EARLY_PRUNE": "early_prune",
    "CRS_DEFAULT_O3_FINAL": "o3_final",
}


def _normalize_params(raw_params: dict) -> dict:
    """Map env-var keys back to short names, then fill any missing keys with Default()."""
    short = {}
    for k, v in raw_params.items():
        short_key = _ENV_TO_PARAM.get(k, k)
        if isinstance(v, str) and v.lstrip("-").isdigit():
            v = int(v)
        short[short_key] = v
    for k, ref in _DEFAULT_PARAMS.items():
        short.setdefault(k, ref)
    return short


def _params_distance(p: dict) -> float:
    """Sum of squared relative deviations from Options::Default()."""
    dist = 0.0
    for k, ref in _DEFAULT_PARAMS.items():
        v = p.get(k, ref)
        if v is None:
            v = ref
        dist += ((float(v) - ref) / max(abs(ref), 1)) ** 2
    return dist


def pareto_mask(jit_arr: np.ndarray, spec_arr: np.ndarray) -> np.ndarray:
    """Return boolean mask: True iff (jit_i, spec_i) is Pareto-optimal (minimize both)."""
    pts = np.column_stack([jit_arr, spec_arr])
    n = len(pts)
    mask = np.ones(n, dtype=bool)
    for i in range(n):
        leq = np.all(pts <= pts[i], axis=1)
        lt_any = np.any(pts < pts[i], axis=1)
        if np.any(leq & lt_any):
            mask[i] = False
    return mask


def fetch_rows(con: duckdb.DuckDBPyConnection, study_name: str):
    """Fetch (config_label, params_json, group, kernel, jit_ms, spec_ms) for a study.

    Tries optim_trial_params first; falls back to ablation_studies. Returns empty list
    if neither has data for the study.
    """
    out = []
    optim_rows = con.execute(
        """
        SELECT 'trial_' || CAST(otp.trial_id AS VARCHAR) AS config_label,
               otp.params_json,
               r."group", r.kernel,
               r.t_jit_ns / 1e6 AS jit_ms,
               r.t_spec_ns / 1e6 AS spec_ms
        FROM optim_trial_params otp
        JOIN v_ratios r USING (run_id)
        WHERE otp.study_name = ?
          AND NOT otp.used_timeout_fallback
          AND r.t_jit_ns IS NOT NULL AND r.t_spec_ns IS NOT NULL
        """,
        [study_name],
    ).fetchall()
    out.extend(optim_rows)

    abl_rows = con.execute(
        """
        SELECT a.config_name AS config_label,
               a.params_json,
               r."group", r.kernel,
               r.t_jit_ns / 1e6 AS jit_ms,
               r.t_spec_ns / 1e6 AS spec_ms
        FROM ablation_studies a
        JOIN v_ratios r USING (run_id)
        WHERE a.study_name = ?
          AND r.t_jit_ns IS NOT NULL AND r.t_spec_ns IS NOT NULL
        """,
        [study_name],
    ).fetchall()
    out.extend(abl_rows)
    return out


def _parse_params(raw):
    if raw is None:
        return {}
    if isinstance(raw, dict):
        d = raw
    else:
        try:
            d = json.loads(raw)
        except Exception:
            return {}
    return _normalize_params(d)


def _flag_default(rows):
    """Mark exactly one row per group as is_default (closest to Options::Default())."""
    # Group rows by (group, kernel) — pick the config closest to default per group as the marker.
    # We mark every (config, kernel) row where config_label == 'default' (ablation case),
    # else fall back to "closest-to-default" using params distance.
    is_default = [False] * len(rows)

    # Case 1: an ablation row labelled 'default' exists.
    has_default_named = any(r[0] == "default" for r in rows)
    if has_default_named:
        for i, r in enumerate(rows):
            if r[0] == "default":
                is_default[i] = True
        return is_default

    # Case 2: pick the closest-to-default config per (group, kernel).
    from collections import defaultdict
    best_per_group = {}
    for i, r in enumerate(rows):
        params = _parse_params(r[1])
        g, k = r[2], r[3]
        d = _params_distance(params)
        key = (g, k)
        if key not in best_per_group or d < best_per_group[key][0]:
            best_per_group[key] = (d, i)
    for (_, idx) in best_per_group.values():
        is_default[idx] = True
    return is_default


def render(rows, default_flags, output_path_png: Path, output_path_csv: Path, title: str):
    """Render one PNG and one CSV from a list of rows + default-flags array."""
    if not rows:
        print(f"  (no data) skipped: {output_path_png}", file=sys.stderr)
        return

    jit_arr = np.array([r[4] for r in rows])
    spec_arr = np.array([r[5] for r in rows])
    pareto = pareto_mask(jit_arr, spec_arr)

    # --- CSV ---
    output_path_csv.parent.mkdir(parents=True, exist_ok=True)
    with open(output_path_csv, "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow([
            "config_label",
            *_PARAM_COLS,
            "group", "kernel",
            "jit_overhead_ms", "specialized_exec_ms",
            "is_pareto_optimal", "is_default",
        ])
        for i, r in enumerate(rows):
            params = _parse_params(r[1])
            param_vals = [params.get(c, "") for c in _PARAM_COLS]
            w.writerow([
                r[0],
                *param_vals,
                r[2], r[3],
                f"{r[4]:.6f}", f"{r[5]:.6f}",
                bool(pareto[i]), bool(default_flags[i]),
            ])

    # --- PNG ---
    fig, ax = plt.subplots(figsize=(9, 6))
    # Non-Pareto, non-default points.
    nondef_nonpareto = ~np.array(default_flags) & ~pareto
    ax.scatter(
        jit_arr[nondef_nonpareto], spec_arr[nondef_nonpareto],
        c="#999999", alpha=0.5, s=20, label="Sampled configs", zorder=1,
    )

    # Pareto-optimal points (but not default).
    pareto_non_default = pareto & ~np.array(default_flags)
    ax.scatter(
        jit_arr[pareto_non_default], spec_arr[pareto_non_default],
        c="#1f77b4", alpha=0.95, s=55, marker="o", label="Pareto-optimal",
        edgecolors="black", linewidths=0.5, zorder=3,
    )

    # Default config(s) — star marker.
    if any(default_flags):
        dflags = np.array(default_flags)
        ax.scatter(
            jit_arr[dflags], spec_arr[dflags],
            c="red", s=260, marker="*", edgecolors="black", linewidths=0.6,
            label="Options::Default()", zorder=5,
        )

    # Pareto polyline (lower-left envelope, sorted by ascending jit).
    pareto_pts = sorted([(jit_arr[i], spec_arr[i]) for i in np.where(pareto)[0]])
    if len(pareto_pts) >= 2:
        px = [p[0] for p in pareto_pts]
        py = [p[1] for p in pareto_pts]
        ax.plot(px, py, "-", color="#1f77b4", linewidth=1.5, alpha=0.7, zorder=2)

    ax.set_xlabel("JIT overhead (ms)  — lower is better")
    ax.set_ylabel("Specialized exec (ms)  — lower is better")
    ax.set_title(title)
    ax.legend(loc="best")
    ax.grid(True, alpha=0.3)

    fig.savefig(output_path_png, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"  Saved: {output_path_png}")
    print(f"  Saved: {output_path_csv}")


def main():
    parser = argparse.ArgumentParser(
        description="Thesis-ready Pareto plots + CSV (jit ↓, spec ↓) for a named study.",
    )
    parser.add_argument("--db", default=None)
    parser.add_argument("--study-name", required=True)
    parser.add_argument("--output-dir", default=None)
    parser.add_argument("--per-group", action="store_true",
                        help="Emit one PNG+CSV per UC group instead of one combined plot.")
    args = parser.parse_args()

    db_path = resolve_db_path(args.db)
    con = duckdb.connect(str(db_path), read_only=True)
    rows = fetch_rows(con, args.study_name)
    con.close()

    if not rows:
        print(f"No data found for study '{args.study_name}'.", file=sys.stderr)
        sys.exit(1)

    if args.output_dir:
        output_dir = Path(args.output_dir)
        output_dir.mkdir(parents=True, exist_ok=True)
    else:
        output_dir = make_report_dir(__file__, parser, args)

    if args.per_group:
        from collections import defaultdict
        by_group = defaultdict(list)
        for r in rows:
            by_group[r[2]].append(r)
        for group, grows in sorted(by_group.items()):
            default_flags = _flag_default(grows)
            render(
                grows, default_flags,
                output_dir / f"pareto_{args.study_name}_{group}.png",
                output_dir / f"pareto_{args.study_name}_{group}.csv",
                title=f"Pareto front — {group}\n(study: {args.study_name})",
            )
    else:
        default_flags = _flag_default(rows)
        render(
            rows, default_flags,
            output_dir / f"pareto_{args.study_name}.png",
            output_dir / f"pareto_{args.study_name}.csv",
            title=f"Pareto front — all groups\n(study: {args.study_name})",
        )


if __name__ == "__main__":
    main()
