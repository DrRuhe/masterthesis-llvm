#!/usr/bin/env python3
"""plot_breakeven.py — Break-even call count distribution plots."""
import argparse
import sys
from pathlib import Path
from collections import defaultdict
import duckdb
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

sys.path.insert(0, str(Path(__file__).parent.parent))
from record_benchmark import resolve_db_path
from report_utils import make_report_dir

Y_CAP = 1000


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--db", default=None)
    parser.add_argument("--study-name", required=True)
    parser.add_argument("--output-dir", default=None)
    args = parser.parse_args()

    db_path = resolve_db_path(args.db)
    con = duckdb.connect(str(db_path), read_only=True)
    if args.output_dir:
        output_dir = Path(args.output_dir)
        output_dir.mkdir(parents=True, exist_ok=True)
    else:
        output_dir = make_report_dir(__file__, parser, args)

    rows = con.execute(
        """SELECT "group", kernel, break_even_calls
           FROM v_optim_breakeven
           WHERE study_name=? AND NOT used_timeout_fallback AND break_even_calls IS NOT NULL
           ORDER BY "group", kernel""",
        [args.study_name],
    ).fetchall()
    con.close()

    if not rows:
        print(f"No break-even data for study '{args.study_name}'")
        return

    by_group = defaultdict(list)
    for group, _kernel, bec in rows:
        by_group[group].append(bec)

    groups_sorted = sorted(by_group.items(), key=lambda kv: np.median(kv[1]))
    group_names = [g for g, _ in groups_sorted]
    data = [v for _, v in groups_sorted]

    fig, ax = plt.subplots(figsize=(max(8, len(group_names) * 1.5), 5))

    bp = ax.boxplot(data, tick_labels=group_names, vert=True, patch_artist=True,
                    flierprops=dict(marker=".", markersize=4, alpha=0.5))

    for patch in bp["boxes"]:
        patch.set_facecolor("steelblue")
        patch.set_alpha(0.6)

    for i, (group, vals) in enumerate(groups_sorted):
        med = np.median(vals)
        if med > 100:
            ax.annotate(
                "rarely worth it",
                xy=(i + 1, min(med, Y_CAP)),
                xytext=(i + 1, min(med * 1.15, Y_CAP * 0.95)),
                ha="center",
                fontsize=7,
                color="red",
            )

    ax.axhline(y=1, color="green", linestyle="--", linewidth=1.0, label="1 call sufficient")
    ax.set_ylim(bottom=0, top=Y_CAP)
    ax.set_ylabel("Break-even call count")
    ax.set_xlabel("UC group")
    ax.set_title(f"Break-even calls per group\n(study: {args.study_name})")
    ax.legend(loc="upper left", fontsize=8)
    plt.xticks(rotation=30, ha="right")

    out_path = output_dir / f"breakeven_{args.study_name}.png"
    fig.savefig(out_path, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"  Saved: {out_path}")


if __name__ == "__main__":
    main()
