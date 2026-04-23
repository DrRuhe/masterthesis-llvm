#!/usr/bin/env python3
"""Plot peak memory usage per JIT overhead call from DuckDB.

Usage:
    reporting/memory_per_jit.py [--db=PATH] [--run-id=ID]
                                [--filter=REGEX] [--mem-unit=B|KB|MB]
"""

import argparse
import os
import re
import sys

import numpy as np
import pandas as pd
import ultraplot as uplt

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from report_utils import (
    MEM_UNITS, add_common_args, auto_mem_unit, make_report_dir, open_db,
    query_df, resolve_db_path, resolve_run_id, save_csv, save_plot,
)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def fmt_mem(val_bytes: float) -> str:
    if np.isnan(val_bytes) or val_bytes <= 0:
        return ""
    if val_bytes >= 1024 ** 2:
        return f"{val_bytes / 1024 ** 2:.1f}MB"
    if val_bytes >= 1024:
        return f"{val_bytes / 1024:.1f}KB"
    return f"{val_bytes:.0f}B"


# New KV format: BM_g:GROUP;n:KERNEL;[s:SIZE;]t:PHASE;[/params]
_KV_NAME_RE = re.compile(
    r"^BM_g:[^;]+;n:(?P<n>[^;]+);(?:[^;]+;)*?t:(?P<t>[^;]+);(?P<params>/.*)?$"
)
# Legacy format (fallback)
_NAME_RE = re.compile(
    r"^BM_(unspecialized|jit_overhead|specialized_exec)_+(.+?)(/\d.*)?$"
)


def parse_name(name: str):
    m = _KV_NAME_RE.match(name)
    if m:
        return m.group('t'), m.group('n'), m.group('params') or ""
    m = _NAME_RE.match(name)
    if not m:
        return None, None, None
    return m.group(1), m.group(2), m.group(3) or ""


def make_group_key(kernel: str, raw_params: str) -> str:
    params = raw_params.lstrip("/")
    numeric_parts = [p for p in params.split("/") if p.isdigit()]
    params_str = ",".join(numeric_parts)
    return f"{kernel}/{params_str}" if params_str else kernel


# ---------------------------------------------------------------------------
# Data loading
# ---------------------------------------------------------------------------

def load_data(con, run_id: str, kernel_filter: str | None = None) -> pd.DataFrame:
    """Load max_bytes_used and jit_blob_kb for jit_overhead rows."""
    has_blob = con.execute(
        "SELECT count(*) FROM duckdb_columns() "
        "WHERE table_name='benchmarks' AND column_name='jit_blob_kb'"
    ).fetchone()[0] > 0
    blob_col = "jit_blob_kb" if has_blob else "0.0 AS jit_blob_kb"

    df = query_df(
        con,
        f"""
        SELECT name, max_bytes_used, {blob_col}
        FROM benchmarks
        WHERE run_id = ?
          AND name LIKE 'BM_%'
          AND name LIKE '%t:jit_overhead%'
          AND run_type = 'iteration'
        """,
        [run_id],
    )

    parsed = df["name"].apply(lambda n: pd.Series(parse_name(n)[1:], index=["kernel", "raw_params"]))
    df = pd.concat([df, parsed], axis=1)
    df["group_key"] = df.apply(lambda r: make_group_key(r["kernel"], r["raw_params"]), axis=1)

    if kernel_filter:
        pat = re.compile(kernel_filter, re.IGNORECASE)
        df = df[df["kernel"].apply(lambda k: bool(pat.search(k)))]

    return df


# ---------------------------------------------------------------------------
# Plotting
# ---------------------------------------------------------------------------

def plot_on_ax(ax, df: pd.DataFrame, mem_unit: str, title: str):
    """Plot max_bytes_used and jit_blob_kb bars on an existing axis."""
    groups = df["group_key"].tolist()
    n = len(groups)
    x = np.arange(n)
    w = 0.3
    div = MEM_UNITS[mem_unit]

    mem_bytes  = df["max_bytes_used"].to_numpy(dtype=float)
    blob_bytes = (df["jit_blob_kb"].fillna(0) * 1024).to_numpy(dtype=float)

    ax.bar(x - 0.5 * w, mem_bytes / div,  w,
           label="Peak RSS delta [max_bytes_used]", color="blue7")
    ax.bar(x + 0.5 * w, blob_bytes / div, w,
           label="JIT bitcode blob size", color="orange7")

    for i, (m, b) in enumerate(zip(mem_bytes, blob_bytes)):
        if m > 0:
            ax.text(i - 0.5 * w, m / div, fmt_mem(m),
                    ha="center", va="bottom", fontsize=6, rotation=90)
        if b > 0:
            ax.text(i + 0.5 * w, b / div, fmt_mem(b),
                    ha="center", va="bottom", fontsize=6, rotation=90)

    ax.format(
        xlocator=x,
        xformatter=[str(g) for g in groups],
        ylabel=f"Memory [{mem_unit}] (log scale)",
        yscale="log",
        title=title,
    )
    ax.tick_params(axis="x", labelrotation=45)
    ax.legend(loc="b")


def plot(df: pd.DataFrame, mem_unit: str, title: str):
    n = len(df)
    width = max(8, n * 1.4)
    fig, ax = uplt.subplots(figsize=(width, 4))
    plot_on_ax(ax, df, mem_unit, title)
    return fig


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Plot peak memory per JIT overhead call from DuckDB."
    )
    add_common_args(parser)
    args = parser.parse_args()

    con    = open_db(resolve_db_path(args.db))
    run_id = resolve_run_id(con, args.run_id)

    report_dir = make_report_dir(__file__, parser, args)

    df = load_data(con, run_id, args.kernel_filter)
    if df.empty:
        sys.exit("No jit_overhead benchmarks found.")

    mem_unit     = args.mem_unit or auto_mem_unit(df["max_bytes_used"].median())
    filter_label = args.kernel_filter or "All Kernels"
    title        = args.title or f"Memory per JIT Overhead Call — {filter_label}"
    fig = plot(df, mem_unit, title)

    save_plot(fig, report_dir / "plot.pdf")
    save_csv(df, report_dir / "data.csv")


if __name__ == "__main__":
    main()
