#!/usr/bin/env python3
"""Plot specialization overhead bar chart from DuckDB.

Usage:
    reporting/specialization_overhead.py [--db=PATH] [--run-id=ID]
                                          [--filter=REGEX] [--output=FILE]
                                          [--time-unit=ns|us|ms|s]
"""

import argparse
import re
import sys

import duckdb
import numpy as np
import pandas as pd
import ultraplot as uplt

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

TIME_UNITS = {"ns": 1e-9, "us": 1e-6, "ms": 1e-3, "s": 1.0}


def auto_unit(median_ns: float) -> str:
    """Pick a human-friendly time unit based on median time in nanoseconds."""
    if median_ns >= 1e9:
        return "s"
    if median_ns >= 1e6:
        return "ms"
    if median_ns >= 1e3:
        return "us"
    return "ns"


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

def plot(groups, h_unspec, h_spec, h_jit, h_spec_jit, time_unit, title, output_path):
    n = len(groups)
    width = max(8, n * 1.4)
    fig, ax = uplt.subplots(figsize=(width, 4))

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

    fig.save(output_path)
    print(f"Saved chart to {output_path}")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Plot specialization overhead from DuckDB."
    )
    parser.add_argument("--db", default="benchmarks.duckdb", help="DuckDB file path.")
    parser.add_argument("--run-id", help="Specific run_id to plot (default: most recent).")
    parser.add_argument("--filter", dest="kernel_filter", help="Regex filter on kernel name.")
    parser.add_argument("--output", default="benchmarks.pdf", help="Output chart path.")
    parser.add_argument(
        "--time-unit", choices=["ns", "us", "ms", "s"], help="Time unit override."
    )
    args = parser.parse_args()

    con = duckdb.connect(args.db, read_only=True)

    if args.run_id:
        run_id = args.run_id
    else:
        row = con.execute(
            "SELECT run_id FROM context ORDER BY run_ts DESC LIMIT 1"
        ).fetchone()
        if row is None:
            print("No runs found in database.", file=sys.stderr)
            sys.exit(1)
        run_id = row[0]

    df = con.execute(
        "SELECT kernel, raw_params, t_unspec_ns, t_spec_ns, t_jit_ns "
        "FROM v_ratios WHERE run_id = ? ORDER BY kernel, raw_params",
        [run_id],
    ).df()

    if args.kernel_filter:
        pat = re.compile(args.kernel_filter, re.IGNORECASE)
        df = df[df["kernel"].apply(lambda k: bool(pat.search(k)))]

    if df.empty:
        print("No benchmarks match the filter.", file=sys.stderr)
        sys.exit(1)

    if args.time_unit:
        target_unit = args.time_unit
    else:
        median_ns = df["t_unspec_ns"].median()
        target_unit = auto_unit(median_ns)

    groups, h_unspec, h_spec, h_jit, h_spec_jit = compute_bars(df)

    if not groups:
        print("No valid benchmark groups (missing unspecialized baseline).", file=sys.stderr)
        sys.exit(1)

    filter_label = args.kernel_filter or "All Kernels"
    title = f"Specialization Overhead — {filter_label}"
    plot(groups, h_unspec, h_spec, h_jit, h_spec_jit, target_unit, title, args.output)


if __name__ == "__main__":
    main()
