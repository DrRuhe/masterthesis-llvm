#!/usr/bin/env python3
"""pass_trace_plot.py — Plot per-pass JIT pipeline metrics from DuckDB pass_traces.

Five subplots stacked vertically: instruction count, function count, basic-block
count, wall time (bar), IR-changed flag (compact rug).
X-axis = pass index.  Background bands show pipeline group (prune / initial /
fixpoint / postfix / final O3).  Vertical dashed lines mark each fixpoint-iter
start, labelled F0 … Fn.

Usage:
    reporting/pass_trace_plot.py [--db PATH] [--run-id ID]
                                  [--benchmark PATTERN] [--output FILE]
                                  [--title TITLE]
"""

import argparse
import os
import re
import sys
from pathlib import Path

import duckdb
import matplotlib.patches as mpatches
import matplotlib.ticker as mticker
from matplotlib.lines import Line2D
import numpy as np
import pandas as pd
import ultraplot as uplt

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
FIXPOINT_COLOR   = "#c0392b"
STEP_LINE_COLOR  = "#2c3e50"
CHANGE_COLOR     = "#c0392b"
NOCHANGE_COLOR   = "#7f8c8d"

# ---------------------------------------------------------------------------
# SQL queries (all derived columns computed in SQL)
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

# Gap-and-island: consecutive runs of (group, fixpoint_iter)
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

_SQL_LATEST_RUN = "SELECT run_id FROM context ORDER BY run_ts DESC LIMIT 1"

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
# Shared decorators (bands + fixpoint lines)
# ---------------------------------------------------------------------------

def _decorate(ax, spans_df: pd.DataFrame, fixpts_df: pd.DataFrame,
              label_fixpoints: bool = False):
    """Draw background group bands and fixpoint-iteration dashed lines."""
    for _, row in spans_df.iterrows():
        g   = row["pass_group"]
        fi  = int(row["fixpoint_iter"])
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
    ticks  = list(range(0, n, step))
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
# Subplot: count metric (instructions / functions / basic blocks)
# ---------------------------------------------------------------------------

def _plot_count(ax, df: pd.DataFrame,
                col_after: str, col_before: str,
                ylabel: str, title: str,
                spans_df: pd.DataFrame, fixpts_df: pd.DataFrame,
                label_fixpoints: bool = False):
    xs     = df["pass_idx"].to_numpy()
    after  = df[col_after].to_numpy(dtype=float)
    before = df[col_before].to_numpy(dtype=float)

    changed = before != after

    _decorate(ax, spans_df, fixpts_df, label_fixpoints=label_fixpoints)
    _set_xlim(ax, xs)

    # Draw one horizontal segment per pass coloured by whether that pass
    # changed the metric: red = changed, grey = unchanged.
    # Y-limits are derived from "after" values only so post-prune detail
    # fills the full axis height (the large pre-prune drop is not shown).
    for i in range(len(xs)):
        color  = CHANGE_COLOR if changed[i] else STEP_LINE_COLOR
        x0     = xs[i]
        x1     = xs[i + 1] if i + 1 < len(xs) else xs[i] + 1
        ax.hlines(after[i], x0, x1, color=color, linewidth=2.0, zorder=5)
        # Thin vertical connector between consecutive segments when they differ
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


# ---------------------------------------------------------------------------
# Subplot: wall time per pass
# ---------------------------------------------------------------------------

def _plot_wall_time(ax, df: pd.DataFrame,
                    spans_df: pd.DataFrame, fixpts_df: pd.DataFrame):
    xs = df["pass_idx"].to_numpy()
    wt = df["wall_time_ms"].to_numpy(dtype=float)

    _decorate(ax, spans_df, fixpts_df)
    _set_xlim(ax, xs)

    ax.bar(xs, wt, color="#2980b9", alpha=0.80, width=0.85, zorder=5)
    ax.set_ylim(0, wt.max() * 1.15)
    ax.set_ylabel("Wall time (ms)", fontsize=8)
    ax.set_title("Per-pass wall time", fontsize=9, pad=4)
    ax.tick_params(labelsize=7)


# ---------------------------------------------------------------------------
# Subplot: IR-changed flag (compact rug)
# ---------------------------------------------------------------------------

def _plot_ir_changed(ax, df: pd.DataFrame,
                     spans_df: pd.DataFrame, fixpts_df: pd.DataFrame):
    xs  = df["pass_idx"].to_numpy()
    chg = df["ir_changed_int"].to_numpy(dtype=int)

    changed_xs   = xs[chg == 1]
    unchanged_xs = xs[chg == 0]

    _decorate(ax, spans_df, fixpts_df)
    _set_xlim(ax, xs)

    # Rug-style: short grey ticks where IR was NOT changed, tall red ticks
    # where IR WAS changed.  One mark per pass — purely boolean, no numeric axis.
    if len(unchanged_xs):
        ax.vlines(unchanged_xs, 0, 0.35, color="#bdc3c7",
                  linewidth=0.9, zorder=5)
    if len(changed_xs):
        ax.vlines(changed_xs, 0, 1.0, color=CHANGE_COLOR,
                  linewidth=1.3, zorder=6)

    ax.set_ylim(-0.05, 1.25)
    ax.set_yticks([0, 1])
    ax.set_yticklabels(["no", "yes"], fontsize=7)
    ax.set_ylabel("IR\nchanged", fontsize=7)
    ax.set_title("IR changed per pass  (red = modified)", fontsize=9, pad=4)
    ax.tick_params(labelsize=7)


# ---------------------------------------------------------------------------
# Legend
# ---------------------------------------------------------------------------

def _make_legend(fig, spans_df: pd.DataFrame, fixpts_df: pd.DataFrame):
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
# DB helpers
# ---------------------------------------------------------------------------

def _resolve_db(flag: str | None) -> Path:
    if flag:
        return Path(flag)
    env = os.environ.get("BENCHPLOT_DB_PATH")
    if env:
        return Path(env)
    return Path.cwd() / "benchmarks.duckdb"


def _latest_run_id(con) -> str:
    row = con.execute(_SQL_LATEST_RUN).fetchone()
    if row is None:
        print("Error: no runs found in database.", file=sys.stderr)
        sys.exit(1)
    return row[0]


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Plot per-pass JIT pipeline metrics from DuckDB pass_traces."
    )
    parser.add_argument("--db",        default=None, metavar="PATH",
                        help="DuckDB file (overrides BENCHPLOT_DB_PATH).")
    parser.add_argument("--run-id",    default=None, metavar="ID",
                        help="run_id to plot. Defaults to most recent run.")
    parser.add_argument("--benchmark", default="%", metavar="PATTERN",
                        help="SQL LIKE pattern for benchmark_name (default: '%%').")
    parser.add_argument("--output",    default=None, metavar="FILE",
                        help="Output file (PDF/PNG/SVG).")
    parser.add_argument("--title",     default=None,
                        help="Figure suptitle.")
    args = parser.parse_args()

    db_path = _resolve_db(args.db)
    if not db_path.exists():
        print(f"Error: DB not found: {db_path}", file=sys.stderr)
        sys.exit(1)

    con     = duckdb.connect(str(db_path), read_only=True)
    run_id  = args.run_id or _latest_run_id(con)
    pattern = args.benchmark if "%" in args.benchmark else f"%{args.benchmark}%"

    df        = con.execute(_SQL_MAIN,      [run_id, pattern]).df()
    spans_df  = con.execute(_SQL_SPANS,     [run_id, pattern]).df()
    fixpts_df = con.execute(_SQL_FIXPOINTS, [run_id, pattern]).df()
    con.close()

    if df.empty:
        print(f"Error: no rows for run_id={run_id} LIKE '{pattern}'.",
              file=sys.stderr)
        sys.exit(1)

    n     = len(df)
    width = max(14, n * 0.22)

    # IR-changed subplot is compact (boolean rug); the four metric subplots
    # get equal heights.  hratios in ultraplot are in inches.
    fig, axes = uplt.subplots(
        nrows=5, ncols=1,
        figsize=(width, 20),
        sharex=False,
        hratios=[4, 4, 4, 4, 1.2],
        hspace=1.2,
    )

    title_str = (args.title or
                 f"JIT Pipeline Trace — {pattern.strip('%').replace('_', ' ')}")
    fig.suptitle(title_str, fontsize=12, fontweight="bold", y=1.002)

    _plot_count(axes[0], df, "instrs_after", "instrs_before",
                "Instructions", "Instruction count",
                spans_df, fixpts_df, label_fixpoints=True)

    _plot_count(axes[1], df, "fns_after", "fns_before",
                "Functions", "Function count",
                spans_df, fixpts_df)

    _plot_count(axes[2], df, "bbs_after", "bbs_before",
                "Basic blocks", "Basic block count",
                spans_df, fixpts_df)

    _plot_wall_time(axes[3], df, spans_df, fixpts_df)
    _plot_ir_changed(axes[4], df, spans_df, fixpts_df)

    for i, ax in enumerate(axes):
        _setup_xticks(ax, df, show_labels=(i == 4))

    _make_legend(fig, spans_df, fixpts_df)

    safe = re.sub(r'[^a-zA-Z0-9_-]', '_', pattern.strip("%"))
    out  = args.output or f"pass_trace_{safe}.pdf"
    fig.save(out, bbox_inches="tight", dpi=150)
    print(f"Saved: {out}")


if __name__ == "__main__":
    main()
