#!/usr/bin/env python3
"""pass_trace_plot_json.py — Render a pass trace plot from a standalone JSON file.

The JSON format is the array produced by writePassTraceJSON() in
ClangRuntimeSpecializerBenchmark.h (one object per pass, fields:
name, group, fixpoint_iter, fns_before/after, instrs_before/after,
bbs_before/after, wall_time_ms, ir_changed).

Primary entry point for programmatic use:
    from pass_trace_plot_json import plot_from_json
    plot_from_json(Path("trace.json"), Path("trace.pdf"))

Can also be run as a script:
    python reporting/pass_trace_plot_json.py trace.json [output.pdf]
"""

import json
import os
import sys
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd
import ultraplot as uplt

# Add this file's directory to sys.path so sibling modules are importable.
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pass_trace_plot as _ptp  # noqa: E402  (import after sys.path fixup)

# ---------------------------------------------------------------------------
# DataFrame helpers
# ---------------------------------------------------------------------------

def _load_json(json_path: Path) -> pd.DataFrame:
    """Load pass trace JSON into a DataFrame with derived pass_idx and typed columns."""
    with open(json_path) as f:
        records = json.load(f)
    df = pd.DataFrame(records)
    df.insert(0, "pass_idx", range(len(df)))
    df["ir_changed_int"] = df["ir_changed"].astype(int)
    df["fixpoint_iter"] = df["fixpoint_iter"].fillna(-1).astype(int)
    # Rename JSON field names to match the column names used by the plot helpers.
    df = df.rename(columns={"name": "pass_name", "group": "pass_group"})
    return df


def _compute_spans(df: pd.DataFrame) -> pd.DataFrame:
    """Island algorithm: find consecutive runs of the same (pass_group, fixpoint_iter)."""
    df2 = df.copy()
    df2["_island"] = (
        (df2["pass_group"] != df2["pass_group"].shift()) |
        (df2["fixpoint_iter"] != df2["fixpoint_iter"].shift())
    ).cumsum()
    spans = (
        df2.groupby(["pass_group", "fixpoint_iter", "_island"])
        .agg(span_start=("pass_idx", "min"), span_end=("pass_idx", "max"))
        .reset_index()
        .drop(columns=["_island"])
        .sort_values("span_start")
    )
    return spans


def _compute_fixpoints(df: pd.DataFrame) -> pd.DataFrame:
    """First pass_idx of each fixpoint iteration boundary."""
    filt = df[df["fixpoint_iter"] >= 0]
    if filt.empty:
        return pd.DataFrame(columns=["fixpoint_iter", "first_idx"])
    return (
        filt.groupby("fixpoint_iter")
        .agg(first_idx=("pass_idx", "min"))
        .reset_index()
        .sort_values("fixpoint_iter")
    )

# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------

def plot_from_json(json_path: Path, output_path: Path, title: str | None = None) -> None:
    """Render a pass trace PDF from a standalone JSON trace file.

    Args:
        json_path:   Path to the *_pass_trace.json file produced by writePassTraceJSON.
        output_path: Destination path for the rendered PDF.
        title:       Optional figure suptitle (defaults to a cleaned version of the stem).
    """
    df = _load_json(json_path)
    if df.empty:
        raise ValueError(f"Pass trace JSON is empty: {json_path}")

    spans_df  = _compute_spans(df)
    fixpts_df = _compute_fixpoints(df)

    n     = len(df)
    width = max(14, n * 0.22)
    fig, axes = uplt.subplots(
        nrows=5, ncols=1,
        figsize=(width, 20),
        sharex=False,
        hratios=[4, 4, 4, 4, 1.2],
        hspace=1.2,
    )

    title_str = title or ("JIT Pipeline Trace — "
                          + json_path.stem.replace("_pass_trace", "").replace("_", " ").strip())
    fig.suptitle(title_str, fontsize=12, fontweight="bold", y=1.002)

    _ptp._plot_count(axes[0], df, "instrs_after", "instrs_before",
                     "Instructions", "Instruction count",
                     spans_df, fixpts_df, label_fixpoints=True)
    _ptp._plot_count(axes[1], df, "fns_after", "fns_before",
                     "Functions", "Function count", spans_df, fixpts_df)
    _ptp._plot_count(axes[2], df, "bbs_after", "bbs_before",
                     "Basic blocks", "Basic block count", spans_df, fixpts_df)
    _ptp._plot_wall_time(axes[3], df, spans_df, fixpts_df)
    _ptp._plot_ir_changed(axes[4], df, spans_df, fixpts_df)

    for i, ax in enumerate(axes):
        _ptp._setup_xticks(ax, df, show_labels=(i == 4))

    _ptp._make_legend(fig, spans_df, fixpts_df)

    kw = {"dpi": 150, "bbox_inches": "tight"}
    if hasattr(fig, "save"):
        fig.save(str(output_path), **kw)
    else:
        fig.savefig(str(output_path), **kw)
    plt.close(fig)

# ---------------------------------------------------------------------------
# CLI entry point
# ---------------------------------------------------------------------------

def main() -> None:
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <trace.json> [output.pdf]", file=sys.stderr)
        sys.exit(1)
    json_path   = Path(sys.argv[1])
    output_path = Path(sys.argv[2]) if len(sys.argv) > 2 else json_path.with_suffix(".pdf")
    plot_from_json(json_path, output_path)
    print(f"Plot written to {output_path}")


if __name__ == "__main__":
    main()
