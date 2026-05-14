#!/usr/bin/env python3
"""Analyze JIT compile time and specialization scaling across dataset sizes.

Works with any benchmark run (polybench, use-case benchmarks, or mixed).

Four plots stacked vertically:
  1. JIT compile time (t_jit_ns)
  2. Steady-state speedup: t_unspec_ns / t_spec_ns
  3. First-call speedup (incl. JIT): t_unspec_ns / (t_jit_ns + t_spec_ns)
  4. Break-even executions: calls needed to recoup JIT cost
     (only kernels/sizes where t_spec_ns < t_unspec_ns)

X axis = dataset size class, derived from kv_s labels (SMALL/MEDIUM/LARGE/EXTRALARGE)
when present (use-case benchmarks), otherwise from ranking the first numeric parameter
in raw_params (polybench).

Usage:
    reporting/polybench_size_scaling.py [--db PATH] [--run-id ID]
                                         [--filter REGEX] [--group GROUP]
                                         [--time-unit ns|us|ms|s] [--title TITLE]
"""

import os
import sys
from itertools import cycle

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import matplotlib.pyplot as plt
import pandas as pd
import ultraplot as uplt

from report_utils import (
    TIME_UNITS, add_common_args, auto_time_unit,
    make_report_dir, open_db, query_df,
    resolve_db_path, resolve_run_id, save_csv, save_plot,
)

# ── Visual style tables ────────────────────────────────────────────────────────

# Fallback size labels used when kv_s is absent (polybench numeric-rank style).
_FALLBACK_SIZE_LABELS = ["MINI", "SMALL", "MEDIUM", "LARGE", "XLARGE"]

_MARKERS    = ["o", "s", "^", "D", "v", "p", "*", "h", "P", "X", "<", ">", "8", "H", "1", "2"]
_LINESTYLES = ["-", "--", "-.", ":"]


def _kernel_styles(kernels: list[str]) -> dict:
    """Assign a unique (marker, linestyle, color) to each kernel."""
    cmap = plt.get_cmap("tab20")
    n = max(len(kernels), 1)
    m_it  = cycle(_MARKERS)
    ls_it = cycle(_LINESTYLES)
    return {
        k: {
            "marker":    next(m_it),
            "linestyle": next(ls_it),
            "color":     cmap(i / n),
        }
        for i, k in enumerate(kernels)
    }


# ── Data loading ───────────────────────────────────────────────────────────────

def _has_kv_s(con) -> bool:
    """Return True if the benchmarks table has a kv_s column."""
    cols = {row[0].lower() for row in con.execute("DESCRIBE benchmarks").fetchall()}
    return "kv_s" in cols


def load_data(
    con,
    run_id: str,
    kernel_filter: str | None = None,
    group_filter: str | None = None,
) -> pd.DataFrame:
    """Load benchmark size-scaling data; metrics computed in SQL.

    Works for any group (polybench, use-case, etc.).  When the benchmarks table
    contains a kv_s column (use-case benchmarks with explicit size labels), the
    size_label column is populated; otherwise it is NULL and callers fall back to
    rank-derived labels.
    """
    size_label_expr = "MAX(kv_s)" if _has_kv_s(con) else "NULL"

    clauses = []
    params: list = [run_id]
    if kernel_filter:
        clauses.append("AND regexp_matches(kernel, ?, 'i')")
        params.append(kernel_filter)
    if group_filter:
        clauses.append('AND "group" = ?')
        params.append(group_filter)
    extra = "\n          ".join(clauses)

    return query_df(
        con,
        f"""
        WITH base AS (
            SELECT
                kernel,
                raw_params,
                MAX("group")  AS bench_group,
                {size_label_expr} AS size_label,
                MAX(CASE WHEN phase = 'unspecialized'    THEN real_time_ns END) AS t_unspec_ns,
                MAX(CASE WHEN phase = 'specialized_exec' THEN real_time_ns END) AS t_spec_ns,
                MAX(CASE WHEN phase = 'jit_overhead'     THEN real_time_ns END) AS t_jit_ns
            FROM v_ns
            WHERE run_id = ?
              AND phase != ''
              AND raw_params NOT LIKE '%manual_time%'
              AND raw_params NOT LIKE '%iterations%'
              {extra}
            GROUP BY kernel, raw_params
        )
        SELECT
            kernel,
            bench_group,
            CONCAT(bench_group, '/', kernel) AS display_name,
            raw_params,
            size_label,
            t_unspec_ns,
            t_spec_ns,
            t_jit_ns,
            ROW_NUMBER() OVER (
                PARTITION BY kernel
                ORDER BY TRY_CAST(SPLIT_PART(LTRIM(raw_params, '/'), '/', 1) AS DOUBLE)
            ) AS size_rank,
            t_unspec_ns / t_spec_ns                          AS speedup,
            t_unspec_ns / (t_jit_ns + t_spec_ns)            AS speedup_with_jit,
            -- NULL when spec is not faster; avoids negative values leaking to Python
            CASE WHEN t_unspec_ns > t_spec_ns
                 THEN CEIL(t_jit_ns / (t_unspec_ns - t_spec_ns))
                 ELSE NULL
            END AS breakeven_executions
        FROM base
        WHERE t_jit_ns IS NOT NULL
        ORDER BY kernel, size_rank
        """,
        params,
    )


# ── Plotting helpers ───────────────────────────────────────────────────────────

def _build_size_tick_labels(df: pd.DataFrame) -> dict[int, str]:
    """Return {size_rank: label} mapping.

    When kv_s labels are present in the data, use them directly.
    Otherwise fall back to _FALLBACK_SIZE_LABELS indexed by rank.
    """
    if "size_label" in df.columns and df["size_label"].notna().any():
        pairs = (df[["size_rank", "size_label"]]
                 .dropna(subset=["size_label"])
                 .drop_duplicates("size_rank")
                 .sort_values("size_rank"))
        return dict(zip(pairs["size_rank"].astype(int), pairs["size_label"].astype(str)))
    max_rank = int(df["size_rank"].max())
    return {r: _FALLBACK_SIZE_LABELS[r - 1] for r in range(1, max_rank + 1)
            if r - 1 < len(_FALLBACK_SIZE_LABELS)}


def _set_size_xticks(ax, tick_labels: dict[int, str]) -> None:
    ranks = sorted(tick_labels)
    ax.set_xticks(ranks)
    ax.set_xticklabels([tick_labels[r] for r in ranks])


def _plot_lines(ax, df: pd.DataFrame, styles: dict, col: str, notna: bool = True) -> bool:
    """Draw one line per benchmark (display_name) for `col`. Returns True if any line was drawn."""
    drawn = False
    name_col = "display_name" if "display_name" in df.columns else "kernel"
    for name in sorted(df[name_col].unique()):
        kdf = df[df[name_col] == name]
        if notna:
            kdf = kdf[kdf[col].notna()]
        kdf = kdf.sort_values("size_rank")
        if kdf.empty:
            continue
        s = styles[name]
        ax.plot(
            kdf["size_rank"].to_numpy(),
            kdf[col].to_numpy(),
            marker=s["marker"], linestyle=s["linestyle"], color=s["color"],
            label=name, markersize=6, linewidth=1.5,
        )
        drawn = True
    return drawn


def plot_jit_time(ax, df: pd.DataFrame, styles: dict, time_unit: str, title: str,
                  tick_labels: dict) -> None:
    divisor = TIME_UNITS[time_unit]
    scaled  = df.assign(t_jit_scaled=df["t_jit_ns"] / divisor)
    _plot_lines(ax, scaled, styles, "t_jit_scaled", notna=False)
    ax.set_title(title, fontsize=9, pad=4)
    ax.set_ylabel(f"JIT compile time [{time_unit}]", fontsize=8)
    ax.set_xlabel("Dataset size class", fontsize=8)
    _set_size_xticks(ax, tick_labels)


def plot_speedup(ax, df: pd.DataFrame, styles: dict, title: str,
                 tick_labels: dict) -> None:
    drawn = _plot_lines(ax, df, styles, "speedup")
    ax.set_yscale("log")
    ymin, ymax = ax.get_ylim()
    ax.set_ylim(bottom=min(ymin, 0.8), top=ymax)
    if drawn:
        ax.axhline(1.0, color="gray", linestyle=":", linewidth=1.2, alpha=0.8)
    ax.set_title(title, fontsize=9, pad=4)
    ax.set_ylabel("t_unspec / t_spec  [>1 = faster, log scale]", fontsize=8)
    ax.set_xlabel("Dataset size class", fontsize=8)
    _set_size_xticks(ax, tick_labels)


def plot_speedup_with_jit(ax, df: pd.DataFrame, styles: dict, title: str,
                          tick_labels: dict) -> None:
    drawn = _plot_lines(ax, df, styles, "speedup_with_jit")
    ax.set_yscale("log")
    ymin, ymax = ax.get_ylim()
    ax.set_ylim(bottom=min(ymin, 0.8), top=ymax)
    if drawn:
        ax.axhline(1.0, color="gray", linestyle=":", linewidth=1.2, alpha=0.8)
    ax.set_title(title, fontsize=9, pad=4)
    ax.set_ylabel("t_unspec / (t_jit + t_spec)  [>1 = first call faster, log scale]", fontsize=8)
    ax.set_xlabel("Dataset size class", fontsize=8)
    _set_size_xticks(ax, tick_labels)


def plot_breakeven(ax, df: pd.DataFrame, styles: dict, title: str,
                   tick_labels: dict) -> None:
    _plot_lines(ax, df, styles, "breakeven_executions")
    ax.set_title(title, fontsize=9, pad=4)
    ax.set_ylabel("Executions to recoup JIT cost  [log scale]", fontsize=8)
    ax.set_xlabel("Dataset size class", fontsize=8)
    ax.set_yscale("log")
    ax.yaxis.set_major_formatter(plt.FuncFormatter(lambda y, _: f'{y:.0f}'))
    ax.set_ylim(bottom=1)
    _set_size_xticks(ax, tick_labels)


# ── Main ───────────────────────────────────────────────────────────────────────

def main():
    import argparse

    parser = argparse.ArgumentParser(
        description="Plot JIT scaling across dataset sizes (4-panel). Works for any benchmark group."
    )
    add_common_args(parser)
    parser.add_argument(
        "--group", metavar="GROUP", default=None,
        help="Filter to a specific benchmark group (e.g. 'polybench', 'uc1_sql').",
    )
    args = parser.parse_args()

    con    = open_db(resolve_db_path(args.db))
    run_id = resolve_run_id(con, args.run_id)

    report_dir = make_report_dir(__file__, parser, args)

    df = load_data(con, run_id, args.kernel_filter, args.group)
    if df.empty:
        sys.exit("No data found for the selected run/filter.")

    kernels = sorted(df["display_name"].unique())
    print(f"[size_scaling] {len(df)} rows, {len(kernels)} kernels")
    print(df[["display_name", "size_rank", "t_jit_ns", "speedup", "speedup_with_jit", "breakeven_executions"]]
          .head(12).to_string(index=False))

    styles      = _kernel_styles(kernels)
    time_unit   = args.time_unit or auto_time_unit(df["t_jit_ns"].median())
    tick_labels = _build_size_tick_labels(df)
    filter_note = f" — {args.kernel_filter}" if args.kernel_filter else (
                  f" — {args.group}" if args.group else "")
    default_title = args.title or "Size Scaling"

    fig, axs = uplt.subplots(nrows=4, ncols=1, figsize=(14, 22), hspace=1.4)

    plot_jit_time(
        axs[0], df, styles, time_unit,
        f"JIT Compile Time{filter_note}",
        tick_labels,
    )
    plot_speedup(
        axs[1], df, styles,
        f"Steady-State Speedup  t_unspec / t_spec{filter_note}",
        tick_labels,
    )
    plot_speedup_with_jit(
        axs[2], df, styles,
        f"First-Call Speedup  t_unspec / (t_jit + t_spec){filter_note}",
        tick_labels,
    )
    plot_breakeven(
        axs[3], df, styles,
        f"Break-Even Executions  (only where t_spec < t_unspec){filter_note}",
        tick_labels,
    )

    handles, labels = axs[0].get_legend_handles_labels()
    ncols = min(6, len(kernels))
    fig.legend(handles=handles, labels=labels, loc="b", ncols=ncols, fontsize=7)

    save_plot(fig, report_dir / "plot.pdf")
    save_csv(df, report_dir / "data.csv")


if __name__ == "__main__":
    main()
