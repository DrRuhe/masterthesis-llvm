#!/usr/bin/env python3
"""pass_trace_plot.py — Plot per-pass JIT pipeline metrics from DuckDB pass_traces.

Five subplots stacked vertically: instruction count, function count, basic-block
count, wall time (bar), IR-changed flag (compact rug).
X-axis = pass index.  Background bands show pipeline group (prune / initial /
fixpoint / postfix / final O3).  Vertical dashed lines mark each fixpoint-iter
start, labelled F0 … Fn.

Usage:
    reporting/pass_trace_plot.py [--db PATH] [--run-id ID]
                                  [--benchmark PATTERN] [--title TITLE]
"""

import argparse
import os
import re
import sys

import matplotlib.patches as mpatches
import matplotlib.ticker as mticker
from matplotlib.lines import Line2D
import numpy as np
import pandas as pd
import ultraplot as uplt

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from report_utils import (
    add_common_args, make_report_dir, open_db, query_df,
    resolve_db_path, resolve_run_id, save_csv, save_plot,
)

# ---------------------------------------------------------------------------
# Styling constants
# ---------------------------------------------------------------------------

GROUP_COLORS = {
    "prune":    "#d0d0d0",
    "initial":  "#c5ddf5",
    "fixpoint": "#fde0b5",
    "postfix":  "#c8edce",
    "final":    "#e8d5f0",
}
GROUP_LABELS = {
    "prune":    "Prune",
    "initial":  "Initial",
    "fixpoint": "Fixpoint",
    "postfix":  "Post-fix",
    "final":    "Final O3",
}
FIXPOINT_COLOR  = "#c0392b"
STEP_LINE_COLOR = "#2c3e50"
CHANGE_COLOR    = "#c0392b"
NOCHANGE_COLOR  = "#7f8c8d"

# ---------------------------------------------------------------------------
# SQL queries
# ---------------------------------------------------------------------------

_SQL_MAIN = """
SELECT
    pass_idx,
    pass_name,
    pass_group,
    COALESCE(fixpoint_iter, -1)          AS fixpoint_iter,
    instrs_before, instrs_after,
    fns_before,    fns_after,
    bbs_before,    bbs_after,
    wall_time_ms,
    ir_changed::INTEGER                  AS ir_changed_int
FROM pass_traces
WHERE run_id = ?
  AND benchmark_name LIKE ?
ORDER BY pass_idx
"""

_SQL_SPANS = """
SELECT pass_group, fixpoint_iter,
       MIN(pass_idx) AS span_start,
       MAX(pass_idx) AS span_end
FROM (
    SELECT *,
        ROW_NUMBER() OVER (ORDER BY pass_idx)
        - ROW_NUMBER() OVER (PARTITION BY pass_group, fixpoint_iter
                             ORDER BY pass_idx) AS island_id
    FROM pass_traces
    WHERE run_id = ?
      AND benchmark_name LIKE ?
)
GROUP BY pass_group, fixpoint_iter, island_id
ORDER BY span_start
"""

_SQL_FIXPOINTS = """
SELECT fixpoint_iter, MIN(pass_idx) AS first_idx
FROM pass_traces
WHERE run_id = ?
  AND benchmark_name LIKE ?
  AND fixpoint_iter >= 0
GROUP BY fixpoint_iter
ORDER BY fixpoint_iter
"""

# ---------------------------------------------------------------------------
# Pass-name abbreviation
# ---------------------------------------------------------------------------

_ADAPTOR_RE = re.compile(
    r'(?:ModuleTo(?:Function|CGSCC|Loop)PassAdaptor|'
    r'CGSCCToFunctionPassAdaptor)<(.+)>$'
)
_TMPL_RE = re.compile(r'<[^<>]*>')


def _abbrev(name: str, maxlen: int = 22) -> str:
    m = _ADAPTOR_RE.match(name)
    if m:
        inner = m.group(1).split(",")[0].strip()
        name = "→" + _TMPL_RE.sub("", inner)
    name = re.sub(r'Pass(<[^<>]*>)?$', '', name)
    name = _TMPL_RE.sub('', name)
    return name[:maxlen]


# ---------------------------------------------------------------------------
# Shared decorators
# ---------------------------------------------------------------------------

def _decorate(ax, spans_df: pd.DataFrame, fixpts_df: pd.DataFrame,
              label_fixpoints: bool = False):
    for _, row in spans_df.iterrows():
        g  = row["pass_group"]
        fi = int(row["fixpoint_iter"])
        alpha = 0.55 if fi < 0 else max(0.28, 0.60 - fi * 0.08)
        ax.axvspan(
            row["span_start"] - 0.5, row["span_end"] + 0.5,
            color=GROUP_COLORS.get(g, "#f0f0f0"), alpha=alpha,
            zorder=0, linewidth=0,
        )

    for _, row in fixpts_df.iterrows():
        fi   = int(row["fixpoint_iter"])
        xpos = row["first_idx"] - 0.5
        ax.axvline(xpos, color=FIXPOINT_COLOR, linestyle="--",
                   linewidth=1.1, zorder=3)
        if label_fixpoints:
            ax.text(xpos, 1.01, f" F{fi}",
                    transform=ax.get_xaxis_transform(),
                    color=FIXPOINT_COLOR, fontsize=6.5, fontweight="bold",
                    va="bottom", ha="left", clip_on=False, zorder=6)


def _set_xlim(ax, xs):
    ax.set_xlim(xs[0] - 0.5, xs[-1] + 0.5)


def _setup_xticks(ax, df: pd.DataFrame, show_labels: bool):
    n    = len(df)
    step = max(1, n // 30)
    ticks = list(range(0, n, step))
    if show_labels:
        labels = [_abbrev(df.iloc[t]["pass_name"]) for t in ticks]
        ax.set_xticks(ticks)
        ax.set_xticklabels(labels, rotation=55, ha="right", fontsize=6)
        ax.set_xlabel("Pass index", fontsize=8, labelpad=3)
    else:
        ax.set_xticks(ticks)
        ax.set_xticklabels([str(t) for t in ticks], fontsize=7)
        ax.tick_params(axis="x", labelbottom=False)


# ---------------------------------------------------------------------------
# Subplots
# ---------------------------------------------------------------------------

def _plot_count(ax, df, col_after, col_before, ylabel, title,
                spans_df, fixpts_df, label_fixpoints=False):
    xs     = df["pass_idx"].to_numpy()
    after  = df[col_after].to_numpy(dtype=float)
    before = df[col_before].to_numpy(dtype=float)
    changed = before != after

    _decorate(ax, spans_df, fixpts_df, label_fixpoints=label_fixpoints)
    _set_xlim(ax, xs)

    for i in range(len(xs)):
        color = CHANGE_COLOR if changed[i] else STEP_LINE_COLOR
        x0 = xs[i]
        x1 = xs[i + 1] if i + 1 < len(xs) else xs[i] + 1
        ax.hlines(after[i], x0, x1, color=color, linewidth=2.0, zorder=5)
        if i + 1 < len(xs):
            y0, y1 = after[i], after[i + 1]
            if y0 != y1:
                ax.vlines(x1, min(y0, y1), max(y0, y1),
                          color=STEP_LINE_COLOR, linewidth=0.9, zorder=4)

    y_min = max(0, np.nanmin(after) * 0.93)
    y_max = np.nanmax(after) * 1.08
    ax.set_ylim(y_min, y_max)
    ax.set_ylabel(ylabel, fontsize=8)
    ax.set_title(title, fontsize=9, pad=4)
    ax.tick_params(labelsize=7)


def _plot_wall_time(ax, df, spans_df, fixpts_df):
    xs = df["pass_idx"].to_numpy()
    wt = df["wall_time_ms"].to_numpy(dtype=float)

    _decorate(ax, spans_df, fixpts_df)
    _set_xlim(ax, xs)

    ax.bar(xs, wt, color="#2980b9", alpha=0.80, width=0.85, zorder=5)
    ax.set_ylim(0, wt.max() * 1.15)
    ax.set_ylabel("Wall time (ms)", fontsize=8)
    ax.set_title("Per-pass wall time", fontsize=9, pad=4)
    ax.tick_params(labelsize=7)


def _plot_ir_changed(ax, df, spans_df, fixpts_df):
    xs  = df["pass_idx"].to_numpy()
    chg = df["ir_changed_int"].to_numpy(dtype=int)

    changed_xs   = xs[chg == 1]
    unchanged_xs = xs[chg == 0]

    _decorate(ax, spans_df, fixpts_df)
    _set_xlim(ax, xs)

    if len(unchanged_xs):
        ax.vlines(unchanged_xs, 0, 0.35, color="#bdc3c7", linewidth=0.9, zorder=5)
    if len(changed_xs):
        ax.vlines(changed_xs, 0, 1.0, color=CHANGE_COLOR, linewidth=1.3, zorder=6)

    ax.set_ylim(-0.05, 1.25)
    ax.set_yticks([0, 1])
    ax.set_yticklabels(["no", "yes"], fontsize=7)
    ax.set_ylabel("IR\nchanged", fontsize=7)
    ax.set_title("IR changed per pass  (red = modified)", fontsize=9, pad=4)
    ax.tick_params(labelsize=7)


def _make_legend(fig, spans_df, fixpts_df):
    seen = set(spans_df["pass_group"].unique())
    handles = [
        mpatches.Patch(facecolor=GROUP_COLORS[g], alpha=0.65,
                       label=GROUP_LABELS.get(g, g))
        for g in ("prune", "initial", "fixpoint", "postfix", "final")
        if g in seen
    ]
    if not fixpts_df.empty:
        handles.append(Line2D([0], [0], color=FIXPOINT_COLOR,
                              linestyle="--", linewidth=1.1,
                              label="Fixpoint iter boundary"))
    handles += [
        Line2D([0], [0], color=STEP_LINE_COLOR, linewidth=2.0,
               label="Pass: metric unchanged"),
        Line2D([0], [0], color=CHANGE_COLOR, linewidth=2.0,
               label="Pass: metric changed"),
    ]
    fig.legend(handles=handles, loc="b", ncols=len(handles),
               fontsize=7, frame=True)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Plot per-pass JIT pipeline metrics from DuckDB pass_traces."
    )
    add_common_args(parser)
    parser.add_argument("--benchmark", default="%", metavar="PATTERN",
                        help="SQL LIKE pattern for benchmark_name (default: '%%').")
    args = parser.parse_args()

    con    = open_db(resolve_db_path(args.db))
    run_id = resolve_run_id(con, args.run_id)

    report_dir = make_report_dir(__file__, parser, args)

    pattern   = args.benchmark if "%" in args.benchmark else f"%{args.benchmark}%"
    df        = query_df(con, _SQL_MAIN,      [run_id, pattern])
    spans_df  = query_df(con, _SQL_SPANS,     [run_id, pattern])
    fixpts_df = query_df(con, _SQL_FIXPOINTS, [run_id, pattern])
    con.close()

    if df.empty:
        sys.exit(f"No rows for run_id={run_id} LIKE '{pattern}'.")

    n     = len(df)
    width = max(14, n * 0.22)

    fig, axes = uplt.subplots(
        nrows=5, ncols=1,
        figsize=(width, 20),
        sharex=False,
        hratios=[4, 4, 4, 4, 1.2],
        hspace=1.2,
    )

    title_str = args.title or f"JIT Pipeline Trace — {pattern.strip('%').replace('_', ' ')}"
    fig.suptitle(title_str, fontsize=12, fontweight="bold", y=1.002)

    _plot_count(axes[0], df, "instrs_after", "instrs_before",
                "Instructions", "Instruction count",
                spans_df, fixpts_df, label_fixpoints=True)
    _plot_count(axes[1], df, "fns_after", "fns_before",
                "Functions", "Function count", spans_df, fixpts_df)
    _plot_count(axes[2], df, "bbs_after", "bbs_before",
                "Basic blocks", "Basic block count", spans_df, fixpts_df)
    _plot_wall_time(axes[3], df, spans_df, fixpts_df)
    _plot_ir_changed(axes[4], df, spans_df, fixpts_df)

    for i, ax in enumerate(axes):
        _setup_xticks(ax, df, show_labels=(i == 4))

    _make_legend(fig, spans_df, fixpts_df)

    save_plot(fig, report_dir / "plot.pdf")
    save_csv(df, report_dir / "data.csv")


if __name__ == "__main__":
    main()
