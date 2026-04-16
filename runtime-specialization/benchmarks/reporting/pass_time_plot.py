#!/usr/bin/env python3
"""pass_time_plot.py — Horizontal bar chart of JIT pipeline pass wall time.

Y-axis = passes in pipeline order.
  • Non-fixpoint passes: one bar per pass.
  • Fixpoint group: one row per pass-slot (relative position within an
    iteration); bar is stacked — each segment represents one fixpoint
    iteration, coloured green (IR changed) or red (IR unchanged).
X-axis = wall time in ms.  Linear scale.

Usage:
    reporting/pass_time_plot.py [--db PATH] [--run-id ID]
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
import matplotlib.pyplot as plt
import pandas as pd

# ---------------------------------------------------------------------------
# Styling
# ---------------------------------------------------------------------------

CHANGED_COLOR   = "#27ae60"   # green  — IR was modified by this pass
UNCHANGED_COLOR = "#e74c3c"   # red    — IR was NOT modified

GROUP_BG_COLORS = {
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

# ---------------------------------------------------------------------------
# SQL
# ---------------------------------------------------------------------------

_SQL_MAIN = """
SELECT
    pass_idx,
    pass_name,
    pass_group,
    COALESCE(fixpoint_iter, -1)  AS fixpoint_iter,
    wall_time_ms,
    ir_changed::INTEGER          AS ir_changed_int
FROM pass_traces
WHERE run_id = ?
  AND benchmark_name LIKE ?
ORDER BY pass_idx
"""

_SQL_LATEST_RUN = "SELECT run_id FROM context ORDER BY run_ts DESC LIMIT 1"

# ---------------------------------------------------------------------------
# Pass-name abbreviation (mirrors pass_trace_plot.py)
# ---------------------------------------------------------------------------

_ADAPTOR_RE = re.compile(
    r'(?:ModuleTo(?:Function|CGSCC|Loop)PassAdaptor|'
    r'CGSCCToFunctionPassAdaptor)<(.+)>$'
)
_TMPL_RE = re.compile(r'<[^<>]*>')


def _abbrev(name: str, maxlen: int = 32) -> str:
    m = _ADAPTOR_RE.match(name)
    if m:
        inner = m.group(1).split(",")[0].strip()
        name = "→" + _TMPL_RE.sub("", inner)
    name = re.sub(r'Pass(<[^<>]*>)?$', '', name)
    name = _TMPL_RE.sub('', name)
    return name[:maxlen]


# ---------------------------------------------------------------------------
# DB helpers
# ---------------------------------------------------------------------------

def _resolve_db(flag) -> Path:
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
# Build row data
# ---------------------------------------------------------------------------

def _build_rows(df: pd.DataFrame) -> list[dict]:
    """Return an ordered list of row-dicts for the horizontal bar chart.

    Each dict:
        label    : str  — abbreviated pass name
        group    : str  — pipeline group
        segments : list of {'time': float, 'changed': bool}
                   Non-fixpoint rows have exactly one segment.
                   Fixpoint rows have one segment per fixpoint iteration
                   (stacked left-to-right, ordered by iteration number).
    """
    fp_df = df[df['pass_group'] == 'fixpoint'].copy()

    # ---- Fixpoint aggregation ----
    fixpoint_rows: list[dict] = []
    if not fp_df.empty:
        # Relative index = position of each pass within its iteration
        fp_df['rel_idx'] = fp_df.groupby('fixpoint_iter').cumcount()
        n_slots = int(fp_df['rel_idx'].max()) + 1

        # Pass labels from the first fixpoint iteration
        first_iter = fp_df['fixpoint_iter'].min()
        label_map = (
            fp_df[fp_df['fixpoint_iter'] == first_iter]
            .sort_values('rel_idx')
            .set_index('rel_idx')['pass_name']
            .to_dict()
        )
        n_iters = int(fp_df['fixpoint_iter'].max()) + 1

        for slot in range(n_slots):
            slot_data = (
                fp_df[fp_df['rel_idx'] == slot]
                .sort_values('fixpoint_iter')
            )
            segments = [
                {'time': float(r['wall_time_ms']),
                 'changed': bool(r['ir_changed_int'])}
                for _, r in slot_data.iterrows()
            ]
            raw_label = label_map.get(slot, f"slot_{slot}")
            label = _abbrev(raw_label)
            if n_iters > 1:
                label = f"{label}  (×{n_iters})"
            fixpoint_rows.append({
                'label':    label,
                'group':    'fixpoint',
                'segments': segments,
            })

    # ---- Walk full df in order; insert fixpoint block once ----
    rows: list[dict] = []
    fixpoint_inserted = False

    for _, row in df.iterrows():
        g = row['pass_group']
        if g == 'fixpoint':
            if not fixpoint_inserted:
                rows.extend(fixpoint_rows)
                fixpoint_inserted = True
        else:
            rows.append({
                'label': _abbrev(row['pass_name']),
                'group': g,
                'segments': [{'time': float(row['wall_time_ms']),
                               'changed': bool(row['ir_changed_int'])}],
            })

    return rows


# ---------------------------------------------------------------------------
# Plot
# ---------------------------------------------------------------------------

def _plot(rows: list[dict], title: str, output: str) -> None:
    n          = len(rows)
    bar_height = 0.65
    fig_height = max(5, n * 0.38 + 2.5)

    fig, ax = plt.subplots(figsize=(13, fig_height))

    # ---- Background bands per group ----
    group_spans: dict[str, list[int]] = {}
    for i, r in enumerate(rows):
        g = r['group']
        if g not in group_spans:
            group_spans[g] = [i, i]
        else:
            group_spans[g][1] = i

    for g, (y0, y1) in group_spans.items():
        ax.axhspan(y0 - 0.5, y1 + 0.5,
                   color=GROUP_BG_COLORS.get(g, "#f0f0f0"),
                   alpha=0.45, zorder=0, linewidth=0)

    # ---- Horizontal bars ----
    for i, row_data in enumerate(rows):
        left = 0.0
        for seg in row_data['segments']:
            color = CHANGED_COLOR if seg['changed'] else UNCHANGED_COLOR
            ax.barh(i, seg['time'], left=left,
                    color=color, alpha=0.85, height=bar_height,
                    linewidth=0, zorder=3)
            left += seg['time']

    # ---- Y-axis labels ----
    ax.set_yticks(range(n))
    ax.set_yticklabels([r['label'] for r in rows], fontsize=7)
    ax.invert_yaxis()  # pipeline order: first pass at top

    # ---- X-axis ----
    ax.set_xlabel("Wall time (ms)", fontsize=9)
    ax.set_xlim(left=0)
    ax.tick_params(axis='x', labelsize=8)

    ax.set_title(title, fontsize=11, fontweight="bold", pad=8)

    # ---- Group labels on the right ----
    # get_yaxis_transform(): x in axes fraction, y in data coords
    trans = ax.get_yaxis_transform()
    for g, (y0, y1) in group_spans.items():
        ymid = (y0 + y1) / 2.0
        ax.text(1.005, ymid, GROUP_LABELS.get(g, g),
                transform=trans, fontsize=7,
                va='center', ha='left', color='#555555', clip_on=False)

    # ---- Legend ----
    handles = [
        mpatches.Patch(color=CHANGED_COLOR,   alpha=0.85, label="IR changed"),
        mpatches.Patch(color=UNCHANGED_COLOR, alpha=0.85, label="IR unchanged"),
    ]
    seen_groups = {r['group'] for r in rows}
    for g in ("prune", "initial", "fixpoint", "postfix", "final"):
        if g in seen_groups:
            handles.append(
                mpatches.Patch(color=GROUP_BG_COLORS[g], alpha=0.55,
                               label=f"Group: {GROUP_LABELS.get(g, g)}")
            )
    ax.legend(handles=handles, loc='lower right', fontsize=7, framealpha=0.85)

    # ---- Fixpoint note ----
    fp_rows = [r for r in rows if r['group'] == 'fixpoint']
    if fp_rows:
        n_iters = max(len(r['segments']) for r in fp_rows)
        if n_iters > 1:
            ax.text(
                0.5, 1.01,
                f"Fixpoint bars are stacked: each segment = one fixpoint "
                f"iteration ({n_iters} total). Green = IR changed, Red = unchanged.",
                transform=ax.transAxes, fontsize=7,
                ha='center', va='bottom', color='#666666',
            )

    plt.tight_layout()
    fig.savefig(output, bbox_inches='tight', dpi=150)
    print(f"Saved: {output}")
    plt.close(fig)


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def main() -> None:
    parser = argparse.ArgumentParser(
        description="Horizontal bar chart of JIT pipeline pass wall time."
    )
    parser.add_argument("--db",        default=None, metavar="PATH",
                        help="DuckDB file (overrides BENCHPLOT_DB_PATH).")
    parser.add_argument("--run-id",    default=None, metavar="ID",
                        help="run_id to plot. Defaults to most recent run.")
    parser.add_argument("--benchmark", default="%", metavar="PATTERN",
                        help="SQL LIKE pattern for benchmark_name (default '%%').")
    parser.add_argument("--output",    default=None, metavar="FILE",
                        help="Output file (PDF/PNG/SVG).")
    parser.add_argument("--title",     default=None,
                        help="Figure title.")
    args = parser.parse_args()

    db_path = _resolve_db(args.db)
    if not db_path.exists():
        print(f"Error: DB not found: {db_path}", file=sys.stderr)
        sys.exit(1)

    con     = duckdb.connect(str(db_path), read_only=True)
    run_id  = args.run_id or _latest_run_id(con)
    pattern = args.benchmark if "%" in args.benchmark else f"%{args.benchmark}%"

    df = con.execute(_SQL_MAIN, [run_id, pattern]).df()
    con.close()

    if df.empty:
        print(f"Error: no rows for run_id={run_id} LIKE '{pattern}'.",
              file=sys.stderr)
        sys.exit(1)

    rows  = _build_rows(df)
    title = (args.title or
             f"JIT Pipeline Wall Time — {pattern.strip('%').replace('_', ' ')}")
    safe  = re.sub(r'[^a-zA-Z0-9_-]', '_', pattern.strip('%'))
    out   = args.output or f"pass_time_{safe}.pdf"

    _plot(rows, title, out)


if __name__ == "__main__":
    main()
