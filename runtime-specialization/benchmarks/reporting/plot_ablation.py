#!/usr/bin/env python3
"""plot_ablation.py — Ablation analysis bar charts (exec speedup per config per group)."""
import argparse
import math
import sys
from collections import defaultdict
from pathlib import Path

import duckdb
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

sys.path.insert(0, str(Path(__file__).parent.parent))
from record_benchmark import resolve_db_path
from report_utils import make_report_dir

_DEFAULT_CONFIG = "default"


def _sort_configs(configs: list[str]) -> list[str]:
    """Sort config names: 'default' first, remainder alphabetically."""
    default = [c for c in configs if c.lower() == _DEFAULT_CONFIG]
    rest = sorted(c for c in configs if c.lower() != _DEFAULT_CONFIG)
    return default + rest


def main():
    parser = argparse.ArgumentParser(
        description="Grouped bar chart of exec speedup per config from v_ablation_medians."
    )
    parser.add_argument("--db", default=None, metavar="PATH")
    parser.add_argument("--study-name", required=True, metavar="STR")
    parser.add_argument("--output-dir", default=None, metavar="DIR")
    args = parser.parse_args()

    db_path = resolve_db_path(args.db)
    con = duckdb.connect(str(db_path), read_only=True)

    rows = con.execute(
        """SELECT config_name, "group", med_spec_ns, med_unspec_ns
           FROM v_ablation_medians
           WHERE study_name=? AND med_spec_ns > 0 AND med_unspec_ns IS NOT NULL
           ORDER BY "group", config_name""",
        [args.study_name],
    ).fetchall()

    if not rows:
        print(f"No data found for study '{args.study_name}'.", file=sys.stderr)
        sys.exit(1)

    by_group: dict[str, dict[str, list[float]]] = defaultdict(lambda: defaultdict(list))
    for config, group, spec_ns, unspec_ns in rows:
        if spec_ns and spec_ns > 0:
            by_group[group][config].append(unspec_ns / spec_ns)

    groups = sorted(by_group.keys())
    n_groups = len(groups)

    ncols = min(3, n_groups)
    nrows = math.ceil(n_groups / ncols)
    fig, axes = plt.subplots(nrows, ncols, figsize=(4 * ncols, 4 * nrows), squeeze=False)

    for idx, group in enumerate(groups):
        ax = axes[idx // ncols][idx % ncols]
        config_speedups = by_group[group]
        sorted_configs = _sort_configs(list(config_speedups.keys()))
        speedups = [float(np.mean(config_speedups[c])) for c in sorted_configs]
        colors = ["steelblue" if s > 1.0 else "salmon" for s in speedups]

        x = np.arange(len(sorted_configs))
        ax.bar(x, speedups, color=colors, alpha=0.8, edgecolor="white")
        ax.axhline(y=1.0, color="black", linestyle="--", linewidth=1, label="break-even")
        ax.set_xticks(x)
        ax.set_xticklabels(sorted_configs, rotation=45, ha="right", fontsize=8)
        ax.set_title(group, fontsize=10)
        ax.set_ylabel("Exec speedup (unspec / spec)")
        ax.legend(fontsize=7)

    # Hide unused subplot cells.
    for idx in range(n_groups, nrows * ncols):
        axes[idx // ncols][idx % ncols].set_visible(False)

    fig.suptitle(f"Ablation: {args.study_name}", fontsize=12, y=1.01)
    plt.tight_layout()

    if args.output_dir:
        output_dir = Path(args.output_dir)
        output_dir.mkdir(parents=True, exist_ok=True)
    else:
        output_dir = make_report_dir(__file__, parser, args)
    out_path = output_dir / f"ablation_{args.study_name}.png"
    fig.savefig(out_path, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"  Saved: {out_path}")

    con.close()


if __name__ == "__main__":
    main()
