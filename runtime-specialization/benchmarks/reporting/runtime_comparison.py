#!/usr/bin/env python3
"""Plot absolute cpu_time bar chart from DuckDB.

Usage:
    reporting/runtime_comparison.py [--db=PATH] [--run-id=ID]
                                    [--filter=REGEX] [--time-unit=ns|us|ms|s]
"""

import argparse
import os
import sys

import duckdb
import numpy as np
import pandas as pd
import ultraplot as uplt

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from report_utils import (
    TIME_UNITS, add_common_args, apply_kernel_filter, auto_time_unit,
    make_report_dir, open_db, query_df, resolve_db_path, resolve_run_id,
    save_csv, save_plot,
)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def fmt_time(val_ns: float) -> str:
    if np.isnan(val_ns) or val_ns <= 0:
        return ""
    if val_ns >= 1e9:
        return f"{val_ns / 1e9:.0f}s"
    if val_ns >= 1e6:
        return f"{val_ns / 1e6:.0f}ms"
    if val_ns >= 1e3:
        return f"{val_ns / 1e3:.0f}µs"
    return f"{val_ns:.0f}ns"


def make_group_key(row) -> str:
    params = (row["raw_params"] or "").lstrip("/")
    numeric_parts = [p for p in params.split("/") if p.isdigit()]
    params_str = ",".join(numeric_parts)
    return f"{row['kernel']}/{params_str}" if params_str else row["kernel"]


def compute_bars(df: pd.DataFrame):
    """Return (groups, t_unspec, t_spec, t_jit, t_spec_jit) all in nanoseconds."""
    df = df.copy()
    df["group_key"] = df.apply(make_group_key, axis=1)

    groups, t_u, t_s, t_j, t_sj = [], [], [], [], []
    for _, row in df.iterrows():
        u = row["t_unspec_ns"]
        s = row["t_spec_ns"]
        j = row["t_jit_ns"]

        if pd.isna(u) or u == 0:
            continue

        groups.append(row["group_key"])
        t_u.append(u)
        t_s.append(s if not pd.isna(s) else float("nan"))
        t_j.append(j if not pd.isna(j) else float("nan"))
        t_sj.append((s + j) if (not pd.isna(s) and not pd.isna(j)) else float("nan"))

    return (
        groups,
        np.array(t_u),
        np.array(t_s),
        np.array(t_j),
        np.array(t_sj),
    )


# ---------------------------------------------------------------------------
# Plotting
# ---------------------------------------------------------------------------

def plot_on_ax(ax, groups, t_unspec, t_spec, t_jit, t_spec_jit, time_unit, title):
    """Plot absolute cpu_time bars on an existing axis."""
    n = len(groups)
    x = np.arange(n)
    w = 0.18
    div = TIME_UNITS[time_unit]

    def sc(arr):
        return arr / div

    ax.bar(x - 1.5 * w, sc(t_unspec),   w, label="Unspecialized",    color="gray7")
    ax.bar(x - 0.5 * w, sc(t_spec),     w, label="Specialized",       color="green7")
    ax.bar(x + 0.5 * w, sc(t_jit),      w, label="JIT Overhead",      color="orange7")
    ax.bar(x + 1.5 * w, sc(t_spec_jit), w, label="Specialized + JIT", color="blue7")

    for i, (u, s, j, sj) in enumerate(zip(t_unspec, t_spec, t_jit, t_spec_jit)):
        for x_off, h_ns in [(-1.5 * w, u), (-0.5 * w, s), (0.5 * w, j), (1.5 * w, sj)]:
            if not (np.isnan(h_ns) or h_ns <= 0):
                ax.text(
                    i + x_off, h_ns / div,
                    fmt_time(h_ns),
                    ha="center", va="bottom", fontsize=6, rotation=90,
                )

    ax.format(
        xlocator=x,
        xformatter=[str(g) for g in groups],
        ylabel=f"CPU Time [{time_unit}] (log scale)",
        yscale="log",
        title=title,
    )
    ax.tick_params(axis="x", labelrotation=45)
    ax.legend(loc="b")


def plot(groups, t_unspec, t_spec, t_jit, t_spec_jit, time_unit, title):
    n = len(groups)
    width = max(8, n * 1.4)
    fig, ax = uplt.subplots(figsize=(width, 4))
    plot_on_ax(ax, groups, t_unspec, t_spec, t_jit, t_spec_jit, time_unit, title)
    return fig


# ---------------------------------------------------------------------------
# Data loading
# ---------------------------------------------------------------------------

def load_data(con: duckdb.DuckDBPyConnection, run_id: str,
              kernel_filter: str | None = None) -> pd.DataFrame:
    """Pivot cpu_time per phase into t_unspec_ns / t_spec_ns / t_jit_ns columns."""
    df = query_df(con, """
        SELECT
            kernel,
            raw_params,
            MAX(CASE WHEN phase = 'unspecialized'
                THEN cpu_time * CASE time_unit
                    WHEN 'ns' THEN 1.0 WHEN 'us' THEN 1e3
                    WHEN 'ms' THEN 1e6 WHEN 's'  THEN 1e9
                END
            END) AS t_unspec_ns,
            MAX(CASE WHEN phase = 'specialized_exec'
                THEN cpu_time * CASE time_unit
                    WHEN 'ns' THEN 1.0 WHEN 'us' THEN 1e3
                    WHEN 'ms' THEN 1e6 WHEN 's'  THEN 1e9
                END
            END) AS t_spec_ns,
            MAX(CASE WHEN phase = 'jit_overhead'
                THEN cpu_time * CASE time_unit
                    WHEN 'ns' THEN 1.0 WHEN 'us' THEN 1e3
                    WHEN 'ms' THEN 1e6 WHEN 's'  THEN 1e9
                END
            END) AS t_jit_ns
        FROM v_parsed
        WHERE run_id = ? AND phase != ''
        GROUP BY kernel, raw_params
        ORDER BY (t_spec_ns + t_jit_ns) / t_unspec_ns ASC NULLS LAST
        """,
        [run_id],
    )
    return apply_kernel_filter(df, kernel_filter)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Plot absolute cpu_time comparison from DuckDB."
    )
    add_common_args(parser)
    args = parser.parse_args()

    con    = open_db(resolve_db_path(args.db))
    run_id = resolve_run_id(con, args.run_id)

    report_dir = make_report_dir(__file__, parser, args)

    df = load_data(con, run_id, args.kernel_filter)
    if df.empty:
        sys.exit("No benchmarks match the filter.")

    target_unit = args.time_unit or auto_time_unit(df["t_unspec_ns"].median())
    groups, t_unspec, t_spec, t_jit, t_spec_jit = compute_bars(df)

    if not groups:
        sys.exit("No valid benchmark groups (missing unspecialized baseline).")

    filter_label = args.kernel_filter or "All Kernels"
    title = args.title or f"Runtime Comparison — {filter_label}"
    fig = plot(groups, t_unspec, t_spec, t_jit, t_spec_jit, target_unit, title)

    save_plot(fig, report_dir / "plot.pdf")
    save_csv(df, report_dir / "data.csv")


if __name__ == "__main__":
    main()
