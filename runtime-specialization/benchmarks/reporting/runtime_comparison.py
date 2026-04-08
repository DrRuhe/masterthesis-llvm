#!/usr/bin/env python3
"""Plot absolute cpu_time bar chart from DuckDB.

Usage:
    reporting/runtime_comparison.py [--db=PATH] [--run-id=ID]
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

# Maps unit name -> how many nanoseconds one unit equals (used to convert ns → unit)
TIME_UNITS = {"ns": 1.0, "us": 1e3, "ms": 1e6, "s": 1e9}


def auto_unit(median_ns: float) -> str:
    """Pick a human-friendly time unit based on median time in nanoseconds."""
    if median_ns >= 1e9:
        return "s"
    if median_ns >= 1e6:
        return "ms"
    if median_ns >= 1e3:
        return "us"
    return "ns"


def fmt_time(val_ns: float) -> str:
    """Format a nanosecond value as a concise human-friendly string."""
    if np.isnan(val_ns) or val_ns <= 0:
        return ""
    if val_ns >= 1e9:
        return f"{val_ns / 1e9:.2g}s"
    if val_ns >= 1e6:
        return f"{val_ns / 1e6:.2g}ms"
    if val_ns >= 1e3:
        return f"{val_ns / 1e3:.2g}µs"
    return f"{val_ns:.2g}ns"


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
    div = TIME_UNITS[time_unit]  # divide ns by this to get values in target unit

    def sc(arr):
        return arr / div

    ax.bar(x - 1.5 * w, sc(t_unspec),   w, label="Unspecialized",    color="gray7")
    ax.bar(x - 0.5 * w, sc(t_spec),     w, label="Specialized",       color="green7")
    ax.bar(x + 0.5 * w, sc(t_jit),      w, label="JIT Overhead",      color="orange7")
    ax.bar(x + 1.5 * w, sc(t_spec_jit), w, label="Specialized + JIT", color="blue7")

    # Annotate each bar with human-friendly time (independent of y-axis unit)
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


def plot(groups, t_unspec, t_spec, t_jit, t_spec_jit, time_unit, title, output_path):
    n = len(groups)
    width = max(8, n * 1.4)
    fig, ax = uplt.subplots(figsize=(width, 4))
    plot_on_ax(ax, groups, t_unspec, t_spec, t_jit, t_spec_jit, time_unit, title)
    fig.save(output_path)
    print(f"Saved chart to {output_path}")


# ---------------------------------------------------------------------------
# Data loading
# ---------------------------------------------------------------------------

def load_data(con: duckdb.DuckDBPyConnection, run_id: str,
              kernel_filter: str | None = None) -> pd.DataFrame:
    """Pivot cpu_time per phase into t_unspec_ns / t_spec_ns / t_jit_ns columns."""
    df = con.execute(
        """
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
        ORDER BY kernel, raw_params
        """,
        [run_id],
    ).df()

    if kernel_filter:
        pat = re.compile(kernel_filter, re.IGNORECASE)
        df = df[df["kernel"].apply(lambda k: bool(pat.search(k)))]

    return df


def get_latest_run_id(con: duckdb.DuckDBPyConnection) -> str:
    row = con.execute(
        "SELECT run_id FROM context ORDER BY run_ts DESC LIMIT 1"
    ).fetchone()
    if row is None:
        print("No runs found in database.", file=sys.stderr)
        sys.exit(1)
    return row[0]


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Plot absolute cpu_time comparison from DuckDB."
    )
    parser.add_argument("--db", default="benchmarks.duckdb", help="DuckDB file path.")
    parser.add_argument("--run-id", help="Specific run_id to plot (default: most recent).")
    parser.add_argument("--filter", dest="kernel_filter", help="Regex filter on kernel name.")
    parser.add_argument("--output", default="runtime_comparison.pdf", help="Output chart path.")
    parser.add_argument(
        "--time-unit", choices=["ns", "us", "ms", "s"], help="Y-axis time unit override."
    )
    args = parser.parse_args()

    con = duckdb.connect(args.db, read_only=True)
    run_id = args.run_id or get_latest_run_id(con)

    df = load_data(con, run_id, args.kernel_filter)
    if df.empty:
        print("No benchmarks match the filter.", file=sys.stderr)
        sys.exit(1)

    target_unit = args.time_unit or auto_unit(df["t_unspec_ns"].median())
    groups, t_unspec, t_spec, t_jit, t_spec_jit = compute_bars(df)

    if not groups:
        print("No valid benchmark groups (missing unspecialized baseline).", file=sys.stderr)
        sys.exit(1)

    filter_label = args.kernel_filter or "All Kernels"
    plot(groups, t_unspec, t_spec, t_jit, t_spec_jit, target_unit,
         f"Runtime Comparison — {filter_label}", args.output)


if __name__ == "__main__":
    main()
