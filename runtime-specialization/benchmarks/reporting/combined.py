#!/usr/bin/env python3
"""Plot runtime comparison (top) and specialization overhead (bottom) in one PDF.

Usage:
    reporting/combined.py [--db=PATH] [--run-id=ID]
                          [--filter=REGEX] [--output=FILE]
                          [--time-unit=ns|us|ms|s]
"""

import argparse
import os
import sys

# Allow importing sibling modules regardless of CWD
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import duckdb
import ultraplot as uplt

from runtime_comparison import (
    auto_unit,
    compute_bars as compute_runtime_bars,
    get_latest_run_id,
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
    parser.add_argument("--db", default="benchmarks.duckdb", help="DuckDB file path.")
    parser.add_argument("--run-id", help="Specific run_id to plot (default: most recent).")
    parser.add_argument("--filter", dest="kernel_filter", help="Regex filter on kernel name.")
    parser.add_argument("--output", default="combined.pdf", help="Output chart path.")
    parser.add_argument(
        "--time-unit", choices=["ns", "us", "ms", "s"], help="Y-axis time unit for upper plot."
    )
    args = parser.parse_args()

    con = duckdb.connect(args.db, read_only=True)
    run_id = args.run_id or get_latest_run_id(con)

    # Load cpu_time data (same df used for both subplots)
    df = load_data(con, run_id, args.kernel_filter)
    if df.empty:
        print("No benchmarks match the filter.", file=sys.stderr)
        sys.exit(1)

    target_unit = args.time_unit or auto_unit(df["t_unspec_ns"].median())
    filter_label = args.kernel_filter or "All Kernels"

    groups_r, t_u, t_s, t_j, t_sj = compute_runtime_bars(df)
    groups_o, h_u, h_s, h_j, h_sj = compute_overhead_bars(df)

    if not groups_r:
        print("No valid benchmark groups (missing unspecialized baseline).", file=sys.stderr)
        sys.exit(1)

    n = len(groups_r)
    width = max(8, n * 1.4)
    fig, axs = uplt.subplots(nrows=2, figsize=(width, 8))

    plot_runtime_ax(
        axs[0], groups_r, t_u, t_s, t_j, t_sj, target_unit,
        f"Runtime Comparison — {filter_label}",
    )
    plot_overhead_ax(
        axs[1], groups_o, h_u, h_s, h_j, h_sj, target_unit,
        f"Specialization Overhead — {filter_label}",
    )

    fig.save(args.output)
    print(f"Saved chart to {args.output}")


if __name__ == "__main__":
    main()
