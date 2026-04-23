#!/usr/bin/env python3
"""Plot specialization overhead bar chart from DuckDB.

Usage:
    reporting/specialization_overhead.py [--db=PATH] [--run-id=ID]
                                          [--filter=REGEX] [--time-unit=ns|us|ms|s]
"""

import argparse
import os
import sys

import numpy as np
import pandas as pd
import ultraplot as uplt

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from report_utils import (
    add_common_args, apply_kernel_filter, auto_time_unit, make_report_dir,
    open_db, query_df, resolve_db_path, resolve_run_id, save_csv, save_plot,
)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def compute_bars(df: pd.DataFrame):
    """Return group labels and arrays of bar heights (ratios relative to unspecialized)."""
    def make_group_key(row):
        params = (row["raw_params"] or "").lstrip("/")
        numeric_parts = [p for p in params.split("/") if p.isdigit()]
        params_str = ",".join(numeric_parts)
        return f"{row['kernel']}/{params_str}" if params_str else row["kernel"]

    df = df.copy()
    df["group_key"] = df.apply(make_group_key, axis=1)

    valid_groups = []
    h_unspec, h_spec, h_jit, h_spec_jit = [], [], [], []

    for _, row in df.iterrows():
        t_u = row["t_unspec_ns"]
        t_s = row["t_spec_ns"]
        t_j = row["t_jit_ns"]

        if pd.isna(t_u) or t_u == 0:
            continue

        valid_groups.append(row["group_key"])
        h_unspec.append(1.0)
        h_spec.append(t_s / t_u if not pd.isna(t_s) else float("nan"))
        h_jit.append(t_j / t_u if not pd.isna(t_j) else float("nan"))
        h_spec_jit.append(
            (t_s + t_j) / t_u
            if (not pd.isna(t_s) and not pd.isna(t_j))
            else float("nan")
        )

    return (
        valid_groups,
        np.array(h_unspec),
        np.array(h_spec),
        np.array(h_jit),
        np.array(h_spec_jit),
    )


# ---------------------------------------------------------------------------
# Plotting
# ---------------------------------------------------------------------------

def plot_on_ax(ax, groups, h_unspec, h_spec, h_jit, h_spec_jit, time_unit, title):
    """Plot normalized overhead bars on an existing axis."""
    n = len(groups)
    x = np.arange(n)
    w = 0.18

    ax.bar(x - 1.5 * w, h_unspec,   w, label="Unspecialized",    color="gray7")
    ax.bar(x - 0.5 * w, h_spec,     w, label="Specialized",       color="green7")
    ax.bar(x + 0.5 * w, h_jit,      w, label="JIT Overhead",      color="orange7")
    ax.bar(x + 1.5 * w, h_spec_jit, w, label="Specialized + JIT", color="blue7")

    ax.axhline(1.0, color="k", ls="--", lw=0.8)

    ax.format(
        xlocator=x,
        xformatter=[str(g) for g in groups],
        ylabel=f"Normalized Time ({time_unit}, lower = faster)",
        yscale="log",
        title=title,
    )
    ax.tick_params(axis="x", labelrotation=45)
    ax.legend(loc="b")


def plot(groups, h_unspec, h_spec, h_jit, h_spec_jit, time_unit, title):
    n = len(groups)
    width = max(8, n * 1.4)
    fig, ax = uplt.subplots(figsize=(width, 4))
    plot_on_ax(ax, groups, h_unspec, h_spec, h_jit, h_spec_jit, time_unit, title)
    return fig


# ---------------------------------------------------------------------------
# Data loading
# ---------------------------------------------------------------------------

def load_data(con, run_id: str, kernel_filter: str | None = None) -> pd.DataFrame:
    """Load real_time pivot from v_ratios; returns df with t_unspec_ns/t_spec_ns/t_jit_ns."""
    df = query_df(
        con,
        "SELECT kernel, raw_params, t_unspec_ns, t_spec_ns, t_jit_ns "
        "FROM v_ratios WHERE run_id = ? ORDER BY kernel, raw_params",
        [run_id],
    )
    return apply_kernel_filter(df, kernel_filter)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Plot specialization overhead from DuckDB."
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
    groups, h_unspec, h_spec, h_jit, h_spec_jit = compute_bars(df)

    if not groups:
        sys.exit("No valid benchmark groups (missing unspecialized baseline).")

    filter_label = args.kernel_filter or "All Kernels"
    title = args.title or f"Specialization Overhead — {filter_label}"
    fig = plot(groups, h_unspec, h_spec, h_jit, h_spec_jit, target_unit, title)

    save_plot(fig, report_dir / "plot.pdf")
    save_csv(df, report_dir / "data.csv")


if __name__ == "__main__":
    main()
