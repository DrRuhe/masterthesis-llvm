#!/usr/bin/env python3
"""Two-panel thesis figure for UC size scaling."""

from __future__ import annotations

import argparse
import os
import sys
from pathlib import Path

import pandas as pd
import ultraplot as uplt

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import size_scaling as ss
from report_utils import open_db, resolve_db_path, resolve_run_id, save_plot


DEFAULT_FILTER = "box_filter|generic_sort|multi_pattern_match"


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate the thesis-specific 2-panel size scaling figure."
    )
    parser.add_argument("--db", default=None, metavar="PATH")
    parser.add_argument("--run-id", default=None, metavar="ID")
    parser.add_argument("--csv", default=None, metavar="PATH")
    parser.add_argument("--filter", default=DEFAULT_FILTER, metavar="REGEX")
    parser.add_argument(
        "--output",
        default="benchmarks/reports/thesis-figures/rq1/rq1_size_scaling.png",
        metavar="PATH",
    )
    return parser.parse_args()


def main() -> None:
    args = _parse_args()
    if args.csv:
        df = pd.read_csv(args.csv)
        run_id = Path(args.csv).stem
        if args.filter:
            df = df[df["kernel"].str.contains(args.filter, case=False, regex=True)]
    else:
        db_path = resolve_db_path(args.db)
        con = open_db(db_path)
        run_id = resolve_run_id(con, args.run_id)
        df = ss.load_data(con, run_id, kernel_filter=args.filter, group_filter=None)
        con.close()
    if df.empty:
        raise RuntimeError(f"No size-scaling data found for run '{run_id}'.")

    kernels = sorted(df["display_name"].unique())
    styles = ss._kernel_styles(kernels)
    tick_labels = ss._build_size_tick_labels(df)
    fig, axes = uplt.subplots(ncols=2, figsize=(12, 4.6))
    ss.plot_speedup(axes[0], df, styles, "Raw execution speedup", tick_labels)
    ss.plot_speedup_with_jit(axes[1], df, styles, "First-call speedup", tick_labels)
    for ax in axes:
        ax.legend(fontsize=7)
    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    save_plot(fig, output)
    print(f"Saved: {output}")


if __name__ == "__main__":
    main()
