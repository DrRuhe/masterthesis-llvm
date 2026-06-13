#!/usr/bin/env python3
"""Combine existing Pareto PNGs into a thesis-ready representative figure."""

from __future__ import annotations

import argparse
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.image as mpimg
import matplotlib.pyplot as plt


REPRESENTATIVES = [
    ("uc1_sql", "count_matching_rows", "UC1 SQL"),
    ("uc2_conv", "box_filter", "UC2 Convolution"),
    ("uc7_dfa", "multi_pattern_match", "UC3 DFA"),
    ("uc8_ivm", "apply_row_delta", "UC4 IVM"),
    ("uc12_groupby", "grouped_count", "UC5 Group-By"),
    ("uc14_sort", "generic_sort", "UC6 Sort"),
]


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Build a 2x3 composite from per-kernel Pareto plots."
    )
    parser.add_argument(
        "--pareto-dir",
        default="benchmarks/reports/260610-pareto",
        help="Directory containing `pareto_<study>_<group>_<kernel>.png` files.",
    )
    parser.add_argument(
        "--study-name",
        default="corpus_uc_final_20260610",
        help="Study name prefix used by the Pareto PNG filenames.",
    )
    parser.add_argument(
        "--output",
        default="benchmarks/reports/thesis-figures/rq1/rq1_pareto_representatives.png",
        help="Output PNG path.",
    )
    return parser.parse_args()


def main() -> None:
    args = _parse_args()
    pareto_dir = Path(args.pareto_dir)
    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)

    fig, axes = plt.subplots(2, 3, figsize=(14, 8.5), constrained_layout=True)

    for ax, (group, kernel, title) in zip(axes.flat, REPRESENTATIVES):
        src = pareto_dir / f"pareto_{args.study_name}_{group}_{kernel}.png"
        if not src.exists():
            raise FileNotFoundError(f"Missing Pareto source image: {src}")
        ax.imshow(mpimg.imread(src))
        ax.set_title(f"{title}: `{kernel}`", fontsize=11)
        ax.axis("off")

    fig.suptitle("Representative Pareto Frontiers Across Use-Case Families", fontsize=14)
    fig.savefig(output, dpi=200, bbox_inches="tight")
    plt.close(fig)
    print(f"Saved: {output}")


if __name__ == "__main__":
    main()
