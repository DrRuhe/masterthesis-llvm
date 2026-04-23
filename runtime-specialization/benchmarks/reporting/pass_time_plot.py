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
                                 [--benchmark PATTERN] [--title TITLE]
"""

import argparse
import os
import re
import sys

import matplotlib.patches as mpatches
import matplotlib.pyplot as plt
import pandas as pd

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from report_utils import (
    add_common_args, make_report_dir, open_db, query_df,
    resolve_db_path, resolve_run_id, save_csv, save_plot,
)

# ---------------------------------------------------------------------------
# Styling
# ---------------------------------------------------------------------------

CHANGED_COLOR   = "#27ae60"
UNCHANGED_COLOR = "#e74c3c"

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

# ---------------------------------------------------------------------------
# Pass-name abbreviation
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
# Build row data
# ---------------------------------------------------------------------------

def _build_rows(df: pd.DataFrame) -> list[dict]:
    """Return an ordered list of row-dicts for the horizontal bar chart."""
    fp_df = df[df['pass_group'] == 'fixpoint'].copy()

    fixpoint_rows: list[dict] = []
    if not fp_df.empty:
        fp_df['rel_idx'] = fp_df.groupby('fixpoint_iter').cumcount()
        n_slots = int(fp_df['rel_idx'].max()) + 1

        first_iter = fp_df['fixpoint_iter'].min()
        label_map = (
            fp_df[fp_df['fixpoint_iter'] == first_iter]
            .sort_values('rel_idx')
            .set_index('rel_idx')['pass_name']
            .to_dict()
        )
        n_iters = int(fp_df['fixpoint_iter'].max()) + 1

        for slot in range(n_slots):
            slot_data = fp_df[fp_df['rel_idx'] == slot].sort_values('fixpoint_iter')
            segments = [
                {'time': float(r['wall_time_ms']), 'changed': bool(r['ir_changed_int'])}
                for _, r in slot_data.iterrows()
            ]
            raw_label = label_map.get(slot, f"slot_{slot}")
            label = _abbrev(raw_label)
            if n_iters > 1:
                label = f"{label}  (×{n_iters})"
            fixpoint_rows.append({'label': label, 'group': 'fixpoint', 'segments': segments})

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
# Plot (returns figure; caller saves)
# ---------------------------------------------------------------------------

def _plot(rows: list[dict], title: str) -> plt.Figure:
    n          = len(rows)
    bar_height = 0.65
    fig_height = max(5, n * 0.38 + 2.5)

    fig, ax = plt.subplots(figsize=(13, fig_height))

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

    for i, row_data in enumerate(rows):
        left = 0.0
        for seg in row_data['segments']:
            color = CHANGED_COLOR if seg['changed'] else UNCHANGED_COLOR
            ax.barh(i, seg['time'], left=left,
                    color=color, alpha=0.85, height=bar_height,
                    linewidth=0, zorder=3)
            left += seg['time']

    ax.set_yticks(range(n))
    ax.set_yticklabels([r['label'] for r in rows], fontsize=7)
    ax.invert_yaxis()

    ax.set_xlabel("Wall time (ms)", fontsize=9)
    ax.set_xlim(left=0)
    ax.tick_params(axis='x', labelsize=8)

    ax.set_title(title, fontsize=11, fontweight="bold", pad=8)

    trans = ax.get_yaxis_transform()
    for g, (y0, y1) in group_spans.items():
        ymid = (y0 + y1) / 2.0
        ax.text(1.005, ymid, GROUP_LABELS.get(g, g),
                transform=trans, fontsize=7,
                va='center', ha='left', color='#555555', clip_on=False)

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
    return fig


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def main() -> None:
    parser = argparse.ArgumentParser(
        description="Horizontal bar chart of JIT pipeline pass wall time."
    )
    add_common_args(parser)
    parser.add_argument("--benchmark", default="%", metavar="PATTERN",
                        help="SQL LIKE pattern for benchmark_name (default '%%').")
    args = parser.parse_args()

    con    = open_db(resolve_db_path(args.db))
    run_id = resolve_run_id(con, args.run_id)

    report_dir = make_report_dir(__file__, parser, args)

    pattern = args.benchmark if "%" in args.benchmark else f"%{args.benchmark}%"
    df = query_df(con, _SQL_MAIN, [run_id, pattern])
    con.close()

    if df.empty:
        sys.exit(f"No rows for run_id={run_id} LIKE '{pattern}'.")

    rows  = _build_rows(df)
    title = args.title or f"JIT Pipeline Wall Time — {pattern.strip('%').replace('_', ' ')}"
    fig   = _plot(rows, title)

    save_plot(fig, report_dir / "plot.pdf")
    save_csv(df, report_dir / "data.csv")
    plt.close(fig)


if __name__ == "__main__":
    main()
