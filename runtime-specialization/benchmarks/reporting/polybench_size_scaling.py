#!/usr/bin/env python3
"""Analyze JIT compile time and specialization scaling across polybench dataset sizes.

Four plots (2×2 grid):
  1. JIT compile time (t_jit_ns)
  2. Steady-state speedup: t_unspec_ns / t_spec_ns
  3. First-call speedup (incl. JIT): t_unspec_ns / (t_jit_ns + t_spec_ns)
  4. Break-even executions: calls needed to recoup JIT cost
     (only kernels/sizes where t_spec_ns < t_unspec_ns)

X axis = dataset size class (MINI → XLARGE), derived per-kernel by ranking
the first numeric parameter in raw_params.

Usage:
    reporting/polybench_size_scaling.py [--db PATH] [--run-id ID]
                                         [--filter REGEX] [--time-unit ns|us|ms|s]
                                         [--title TITLE]
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

SIZE_LABELS = ["MINI", "SMALL", "MEDIUM", "LARGE", "XLARGE"]

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

def load_data(con, run_id: str, kernel_filter: str | None = None) -> pd.DataFrame:
    """Load polybench data from v_ratios; all metrics computed in SQL."""
    kernel_clause = ""
    params: list = [run_id]
    if kernel_filter:
        kernel_clause = "AND regexp_matches(kernel, ?, 'i')"
        params.append(kernel_filter)

    return query_df(
        con,
        f"""
        SELECT
            kernel,
            raw_params,
            t_unspec_ns,
            t_spec_ns,
            t_jit_ns,
            ROW_NUMBER() OVER (
                PARTITION BY kernel
                ORDER BY TRY_CAST(SPLIT_PART(LTRIM(raw_params, '/'), '/', 1) AS DOUBLE)
            ) AS size_rank,
            -- Steady-state speedup (ignores JIT cost)
            t_unspec_ns / t_spec_ns AS speedup,
            -- First-call speedup (JIT cost folded into denominator)
            t_unspec_ns / (t_jit_ns + t_spec_ns) AS speedup_with_jit,
            -- Break-even: first integer call count where specialization is net-profitable
            -- NULLIF prevents division by zero; negative results (spec slower) become < 1
            CEIL(t_jit_ns / NULLIF(t_unspec_ns - t_spec_ns, 0)) AS breakeven_executions
        FROM   v_ratios
        WHERE  run_id = ?
          AND  "group" = 'polybench'
          AND  t_jit_ns IS NOT NULL
          AND  raw_params NOT LIKE '%manual_time%'
          AND  raw_params NOT LIKE '%iterations%'
          {kernel_clause}
        ORDER  BY kernel, size_rank
        """,
        params,
    )


# ── Plotting helpers ───────────────────────────────────────────────────────────

def _set_size_xticks(ax, max_rank: int) -> None:
    ax.set_xticks(range(1, max_rank + 1))
    ax.set_xticklabels(SIZE_LABELS[:max_rank])


def _plot_lines(ax, df: pd.DataFrame, styles: dict, col: str, notna: bool = True) -> bool:
    """Draw one line per kernel for `col`. Returns True if any line was drawn."""
    drawn = False
    for kernel in sorted(df["kernel"].unique()):
        kdf = df[df["kernel"] == kernel]
        if notna:
            kdf = kdf[kdf[col].notna()]
        kdf = kdf.sort_values("size_rank")
        if kdf.empty:
            continue
        s = styles[kernel]
        ax.plot(
            kdf["size_rank"].to_numpy(),
            kdf[col].to_numpy(),
            marker=s["marker"], linestyle=s["linestyle"], color=s["color"],
            label=kernel, markersize=6, linewidth=1.5,
        )
        drawn = True
    return drawn


def plot_jit_time(ax, df: pd.DataFrame, styles: dict, time_unit: str, title: str) -> None:
    divisor  = TIME_UNITS[time_unit]
    max_rank = int(df["size_rank"].max())
    # Scale t_jit_ns before plotting
    scaled = df.assign(t_jit_scaled=df["t_jit_ns"] / divisor)
    _plot_lines(ax, scaled, styles, "t_jit_scaled", notna=False)
    ax.set_title(title, fontsize=9, pad=4)
    ax.set_ylabel(f"JIT compile time [{time_unit}]", fontsize=8)
    ax.set_xlabel("Dataset size class", fontsize=8)
    _set_size_xticks(ax, max_rank)


def plot_speedup(ax, df: pd.DataFrame, styles: dict, title: str) -> None:
    max_rank = int(df["size_rank"].max())
    if _plot_lines(ax, df, styles, "speedup"):
        ax.axhline(1.0, color="gray", linestyle="--", linewidth=1.0, alpha=0.6)
    ax.set_title(title, fontsize=9, pad=4)
    ax.set_ylabel("t_unspec / t_spec  [>1 = faster]", fontsize=8)
    ax.set_xlabel("Dataset size class", fontsize=8)
    _set_size_xticks(ax, max_rank)


def plot_speedup_with_jit(ax, df: pd.DataFrame, styles: dict, title: str) -> None:
    max_rank = int(df["size_rank"].max())
    if _plot_lines(ax, df, styles, "speedup_with_jit"):
        ax.axhline(1.0, color="gray", linestyle="--", linewidth=1.0, alpha=0.6)
    ax.set_title(title, fontsize=9, pad=4)
    ax.set_ylabel("t_unspec / (t_jit + t_spec)  [>1 = first call faster]", fontsize=8)
    ax.set_xlabel("Dataset size class", fontsize=8)
    _set_size_xticks(ax, max_rank)


def plot_breakeven(ax, df: pd.DataFrame, styles: dict, title: str) -> None:
    max_rank = int(df["size_rank"].max())
    _plot_lines(ax, df[df["breakeven_executions"] >= 1], styles, "breakeven_executions")
    ax.set_title(title, fontsize=9, pad=4)
    ax.set_ylabel("Executions to recoup JIT cost  [log scale]", fontsize=8)
    ax.set_xlabel("Dataset size class", fontsize=8)
    ax.set_yscale("log")
    ax.yaxis.set_major_formatter(plt.FuncFormatter(lambda y, _: f'{y:.0f}'))
    ax.set_ylim(bottom=1)
    _set_size_xticks(ax, max_rank)


# ── Main ───────────────────────────────────────────────────────────────────────

def main():
    import argparse

    parser = argparse.ArgumentParser(
        description="Plot polybench JIT scaling across dataset sizes (4-panel)."
    )
    add_common_args(parser)
    args = parser.parse_args()

    con    = open_db(resolve_db_path(args.db))
    run_id = resolve_run_id(con, args.run_id)

    report_dir = make_report_dir(__file__, parser, args)

    df = load_data(con, run_id, args.kernel_filter)
    if df.empty:
        sys.exit("No polybench data found for the selected run/filter.")

    kernels = sorted(df["kernel"].unique())
    print(f"[polybench_size_scaling] {len(df)} rows, {len(kernels)} kernels")
    print(df[["kernel", "size_rank", "t_jit_ns", "speedup", "speedup_with_jit", "breakeven_executions"]]
          .head(12).to_string(index=False))

    styles      = _kernel_styles(kernels)
    time_unit   = args.time_unit or auto_time_unit(df["t_jit_ns"].median())
    base_title  = args.title or "Polybench Size Scaling"
    filter_note = f" — {args.kernel_filter}" if args.kernel_filter else ""

    fig, axs = uplt.subplots(nrows=4, ncols=1, figsize=(14, 22), hspace=1.4)

    plot_jit_time(
        axs[0], df, styles, time_unit,
        f"JIT Compile Time{filter_note}",
    )
    plot_speedup(
        axs[1], df, styles,
        f"Steady-State Speedup  t_unspec / t_spec{filter_note}",
    )
    plot_speedup_with_jit(
        axs[2], df, styles,
        f"First-Call Speedup  t_unspec / (t_jit + t_spec){filter_note}",
    )
    plot_breakeven(
        axs[3], df, styles,
        f"Break-Even Executions  (only where t_spec < t_unspec){filter_note}",
    )

    handles, labels = axs[0].get_legend_handles_labels()
    ncols = min(6, len(kernels))
    fig.legend(handles=handles, labels=labels, loc="b", ncols=ncols, fontsize=7)

    save_plot(fig, report_dir / "plot.pdf")
    save_csv(df, report_dir / "data.csv")


if __name__ == "__main__":
    main()
