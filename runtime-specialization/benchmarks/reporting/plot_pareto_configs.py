#!/usr/bin/env python3
"""plot_pareto_configs.py — Thesis-ready Pareto plots + CSV export for JIT pipeline studies.

Reads any study (optim or ablation) from benchmarks.duckdb, computes the Pareto-optimal
set in the lower-left envelope (both axes minimized: JIT overhead and specialized exec),
and writes both a PNG and a CSV.

Pareto optimality is evaluated *per (group, kernel)* — configs from different kernels
are never compared (their absolute runtimes differ by nature). With --per-group, one
PNG + CSV is emitted per (group, kernel) pair.

When the kv_a column (abstraction level) is present in the data, each plot colors dots
by abstraction level and draws one Pareto frontier per level plus a global frontier.

CSV columns:
    config_label, fixpoint_max, unroll_max, large_module_max, early_prune, o3_final,
    group, kernel, kv_a, jit_overhead_ms, specialized_exec_ms,
    is_pareto_optimal (bool, per-kernel), is_default (bool)

The CSV is designed to be loaded directly into pgfplots/LaTeX. No post-processing needed
to filter to Pareto-optimal or default rows.
"""
import argparse
import csv
import json
import sys
from collections import defaultdict
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

# Colors per abstraction level and for the global Pareto frontier.
_LEVEL_COLORS = {
    "low":       "#1f77b4",  # blue
    "tradeoff":  "#ff7f0e",  # orange
    "abstract":  "#2ca02c",  # green
}
_GLOBAL_PARETO_COLOR = "#555555"   # neutral dark gray for global frontier
_SINGLE_LEVEL_COLOR  = "#1f77b4"   # blue when no kv_a dimension


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


def _has_kv_a_column(con: duckdb.DuckDBPyConnection) -> bool:
    """Check whether kv_a exists as a column in v_ratios (added by spec 011)."""
    try:
        cols = con.execute(
            "SELECT column_name FROM information_schema.columns "
            "WHERE table_name = 'v_ratios' AND column_name = 'kv_a'"
        ).fetchall()
        return len(cols) > 0
    except Exception:
        return False


def fetch_rows(con: duckdb.DuckDBPyConnection, study_name: str):
    """Fetch rows for a study. Returns list of 7-tuples:
    (config_label, params_json, group, kernel, jit_ms, spec_ms, kv_a_or_None).

    Tries optim_trial_params first; falls back to ablation_studies. Returns empty list
    if neither has data for the study.
    """
    has_kv_a = _has_kv_a_column(con)
    kv_a_expr = "r.kv_a" if has_kv_a else "NULL"

    out = []
    optim_rows = con.execute(
        f"""
        SELECT 'trial_' || CAST(otp.trial_id AS VARCHAR) AS config_label,
               otp.params_json,
               r."group", r.kernel,
               r.t_jit_ns / 1e6 AS jit_ms,
               r.t_spec_ns / 1e6 AS spec_ms,
               {kv_a_expr} AS kv_a
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
        f"""
        SELECT a.config_name AS config_label,
               a.params_json,
               r."group", r.kernel,
               r.t_jit_ns / 1e6 AS jit_ms,
               r.t_spec_ns / 1e6 AS spec_ms,
               {kv_a_expr} AS kv_a
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
    """Mark exactly one row per (group, kernel, kv_a) as is_default.

    Prefers ablation rows labelled 'default'; falls back to the config closest
    to Options::Default() by parameter-space distance.
    """
    is_default = [False] * len(rows)

    has_default_named = any(r[0] == "default" for r in rows)
    if has_default_named:
        # Mark one per (group, kernel, kv_a) where config_label == 'default'.
        seen = set()
        for i, r in enumerate(rows):
            if r[0] == "default":
                key = (r[2], r[3], r[6])  # group, kernel, kv_a
                if key not in seen:
                    is_default[i] = True
                    seen.add(key)
        return is_default

    # Pick the closest-to-default config per (group, kernel, kv_a).
    best_per_group = {}
    for i, r in enumerate(rows):
        params = _parse_params(r[1])
        key = (r[2], r[3], r[6])  # group, kernel, kv_a
        d = _params_distance(params)
        if key not in best_per_group or d < best_per_group[key][0]:
            best_per_group[key] = (d, i)
    for (_, idx) in best_per_group.values():
        is_default[idx] = True
    return is_default


def _plot_frontier(ax, jit_vals, spec_vals, color, linewidth=1.5, alpha=0.7,
                   linestyle="-", label=None, zorder=2):
    """Draw Pareto polyline (lower-left envelope) sorted by ascending JIT overhead."""
    pts = sorted(zip(jit_vals, spec_vals))
    if len(pts) >= 2:
        ax.plot(
            [p[0] for p in pts], [p[1] for p in pts],
            linestyle=linestyle, color=color, linewidth=linewidth,
            alpha=alpha, zorder=zorder, label=label,
        )


def _write_csv(output_path_csv: Path, rows, default_flags, pareto_per_row):
    """Write CSV with one row per (config, kernel[, kv_a]) combination."""
    output_path_csv.parent.mkdir(parents=True, exist_ok=True)
    with open(output_path_csv, "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow([
            "config_label", *_PARAM_COLS,
            "group", "kernel", "kv_a",
            "jit_overhead_ms", "specialized_exec_ms",
            "is_pareto_optimal", "is_default",
        ])
        for i, r in enumerate(rows):
            params = _parse_params(r[1])
            param_vals = [params.get(c, "") for c in _PARAM_COLS]
            w.writerow([
                r[0], *param_vals,
                r[2], r[3], r[6] if r[6] is not None else "",
                f"{r[4]:.6f}", f"{r[5]:.6f}",
                bool(pareto_per_row[i]), bool(default_flags[i]),
            ])


def render_kernel(rows, default_flags, output_path_png: Path, output_path_csv: Path,
                  title: str):
    """Render one (group, kernel) plot and CSV.

    If kv_a values are present, colors dots and Pareto frontiers by abstraction level
    and draws a global frontier in neutral gray. Otherwise renders a single-level plot.
    Pareto optimality is always computed per-kernel (within this call's row set).
    """
    if not rows:
        print(f"  (no data) skipped: {output_path_png}", file=sys.stderr)
        return

    jit_arr  = np.array([r[4] for r in rows])
    spec_arr = np.array([r[5] for r in rows])
    kv_a_vals = [r[6] for r in rows]
    has_levels = any(v is not None for v in kv_a_vals)

    # Global per-kernel Pareto mask (used for CSV is_pareto_optimal and for global line).
    global_pareto = pareto_mask(jit_arr, spec_arr)

    # --- CSV (is_pareto_optimal is per-kernel global) ---
    _write_csv(output_path_csv, rows, default_flags, global_pareto)

    # --- PNG ---
    fig, ax = plt.subplots(figsize=(9, 6))
    dflags = np.array(default_flags)

    if has_levels:
        levels = sorted(set(v for v in kv_a_vals if v is not None),
                        key=lambda x: {"low": 0, "tradeoff": 1, "abstract": 2}.get(x, 99))
        level_arr = np.array(kv_a_vals)

        for level in levels:
            lmask = np.array([v == level for v in kv_a_vals])
            color = _LEVEL_COLORS.get(level, "#888888")
            lj, ls = jit_arr[lmask], spec_arr[lmask]
            ldefault = dflags[lmask]

            # Per-level Pareto mask for frontier drawing.
            l_pareto = pareto_mask(lj, ls) if len(lj) > 1 else np.ones(len(lj), dtype=bool)

            # Non-Pareto, non-default dots.
            non_pareto_non_def = ~l_pareto & ~ldefault
            ax.scatter(lj[non_pareto_non_def], ls[non_pareto_non_def],
                       c=color, alpha=0.35, s=18, zorder=1)

            # Pareto dots (not default).
            pareto_non_def = l_pareto & ~ldefault
            ax.scatter(lj[pareto_non_def], ls[pareto_non_def],
                       c=color, alpha=0.95, s=55, marker="o",
                       edgecolors="black", linewidths=0.5, zorder=3)

            # Per-level Pareto frontier.
            _plot_frontier(ax, lj[l_pareto], ls[l_pareto],
                           color=color, linewidth=1.5, alpha=0.8, zorder=2,
                           label=f"{level} Pareto")

            # Default stars for this level.
            if ldefault.any():
                ax.scatter(lj[ldefault], ls[ldefault],
                           c=color, s=260, marker="*",
                           edgecolors="black", linewidths=0.6, zorder=5,
                           label=f"{level} default")

        # Global Pareto frontier — dashed, neutral color, drawn on top.
        _plot_frontier(ax, jit_arr[global_pareto], spec_arr[global_pareto],
                       color=_GLOBAL_PARETO_COLOR, linewidth=2.0, linestyle="--",
                       alpha=0.9, zorder=4, label="Global Pareto")

    else:
        # Single-level (no kv_a): original behavior.
        nondef_nonpareto = ~dflags & ~global_pareto
        ax.scatter(jit_arr[nondef_nonpareto], spec_arr[nondef_nonpareto],
                   c="#999999", alpha=0.5, s=20, label="Sampled configs", zorder=1)

        pareto_non_default = global_pareto & ~dflags
        ax.scatter(jit_arr[pareto_non_default], spec_arr[pareto_non_default],
                   c=_SINGLE_LEVEL_COLOR, alpha=0.95, s=55, marker="o",
                   label="Pareto-optimal", edgecolors="black", linewidths=0.5, zorder=3)

        if dflags.any():
            ax.scatter(jit_arr[dflags], spec_arr[dflags],
                       c="red", s=260, marker="*", edgecolors="black", linewidths=0.6,
                       label="Options::Default()", zorder=5)

        _plot_frontier(ax, jit_arr[global_pareto], spec_arr[global_pareto],
                       color=_SINGLE_LEVEL_COLOR, linewidth=1.5, alpha=0.7, zorder=2)

    ax.set_xlabel("JIT overhead (ms)  — lower is better")
    ax.set_ylabel("Specialized exec (ms)  — lower is better")
    ax.set_title(title)
    ax.legend(loc="best")
    ax.grid(True, alpha=0.3)

    fig.savefig(output_path_png, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"  Saved: {output_path_png}")
    print(f"  Saved: {output_path_csv}")


def render(rows, default_flags, output_path_png: Path, output_path_csv: Path, title: str):
    """Render a combined plot (all groups/kernels together). is_pareto_optimal in CSV
    is computed per-kernel via groupby, then merged back."""
    if not rows:
        print(f"  (no data) skipped: {output_path_png}", file=sys.stderr)
        return

    # Compute per-kernel Pareto masks and merge into a single array.
    by_kernel = defaultdict(list)
    for i, r in enumerate(rows):
        by_kernel[(r[2], r[3])].append(i)

    pareto_per_row = np.zeros(len(rows), dtype=bool)
    for idxs in by_kernel.values():
        sub_jit  = np.array([rows[i][4] for i in idxs])
        sub_spec = np.array([rows[i][5] for i in idxs])
        sub_mask = pareto_mask(sub_jit, sub_spec)
        for j, idx in enumerate(idxs):
            pareto_per_row[idx] = sub_mask[j]

    _write_csv(output_path_csv, rows, default_flags, pareto_per_row)

    # For the combined PNG use the global (cross-kernel) frontier — purely illustrative.
    jit_arr  = np.array([r[4] for r in rows])
    spec_arr = np.array([r[5] for r in rows])
    dflags   = np.array(default_flags)
    global_pareto = pareto_mask(jit_arr, spec_arr)

    fig, ax = plt.subplots(figsize=(9, 6))
    nondef_nonpareto = ~dflags & ~global_pareto
    ax.scatter(jit_arr[nondef_nonpareto], spec_arr[nondef_nonpareto],
               c="#999999", alpha=0.5, s=20, label="Sampled configs", zorder=1)
    pareto_non_default = global_pareto & ~dflags
    ax.scatter(jit_arr[pareto_non_default], spec_arr[pareto_non_default],
               c=_SINGLE_LEVEL_COLOR, alpha=0.95, s=55, marker="o",
               label="Pareto-optimal (cross-kernel)", edgecolors="black",
               linewidths=0.5, zorder=3)
    if dflags.any():
        ax.scatter(jit_arr[dflags], spec_arr[dflags],
                   c="red", s=260, marker="*", edgecolors="black", linewidths=0.6,
                   label="Options::Default()", zorder=5)
    _plot_frontier(ax, jit_arr[global_pareto], spec_arr[global_pareto],
                   color=_SINGLE_LEVEL_COLOR, linewidth=1.5, alpha=0.7, zorder=2)

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
                        help="Emit one PNG+CSV per (group, kernel) pair (methodologically correct).")
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
        by_group_kernel = defaultdict(list)
        for r in rows:
            by_group_kernel[(r[2], r[3])].append(r)  # key: (group, kernel)
        for (group, kernel), grows in sorted(by_group_kernel.items()):
            default_flags = _flag_default(grows)
            render_kernel(
                grows, default_flags,
                output_dir / f"pareto_{args.study_name}_{group}_{kernel}.png",
                output_dir / f"pareto_{args.study_name}_{group}_{kernel}.csv",
                title=f"Pareto — {group} / {kernel}\n(study: {args.study_name})",
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
