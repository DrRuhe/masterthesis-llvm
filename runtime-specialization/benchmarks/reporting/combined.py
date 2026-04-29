#!/usr/bin/env python3
"""Plot runtime comparison (top) and specialization overhead (bottom) in one PDF.

Usage:
    reporting/combined.py [--db=PATH] [--run-id=ID]
                          [--filter=REGEX] [--time-unit=ns|us|ms|s]
"""

import argparse
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import ultraplot as uplt

from report_utils import (
    add_common_args, auto_time_unit, make_report_dir, open_db,
    resolve_db_path, resolve_run_id, save_csv, save_plot,
)
from runtime_comparison import (
    compute_bars as compute_runtime_bars,
    load_data,
    plot_on_ax as plot_runtime_ax,
)
from specialization_overhead import (
    compute_bars as compute_overhead_bars,
    plot_on_ax as plot_overhead_ax,
)


def main():
    parser = argparse.ArgumentParser(
        description="Plot runtime comparison + overhead in a single PDF."
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
    filter_label = args.kernel_filter or "All Kernels"

    groups_r, t_u, t_s, t_j, t_sj = compute_runtime_bars(df)
    groups_o, h_u, h_s, h_j, h_sj = compute_overhead_bars(df)

    if not groups_r:
        sys.exit("No valid benchmark groups (missing unspecialized baseline).")

    n = len(groups_r)
    width = max(8, n * 1.4)
    fig, axs = uplt.subplots(nrows=2, figsize=(width, 8))

    plot_runtime_ax(
        axs[0], groups_r, t_u, t_s, t_j, t_sj, target_unit,
        args.title or f"Runtime Comparison — {filter_label}",
    )
    plot_overhead_ax(
        axs[1], groups_o, h_u, h_s, h_j, h_sj, target_unit,
        f"Specialization Overhead — {filter_label}",
    )

    save_plot(fig, report_dir / "plot.pdf")
    save_csv(df, report_dir / "data.csv")


if __name__ == "__main__":
    main()
