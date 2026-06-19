#!/usr/bin/env python3
"""Render horizontal boxplots for the RQ1 perf-stats amortized speedups."""

from __future__ import annotations

import argparse
import csv
import math
import os
from pathlib import Path

if "MPLCONFIGDIR" not in os.environ:
    os.environ["MPLCONFIGDIR"] = "/tmp/matplotlib-runtime-specialization"

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Plot horizontal amortized-speedup boxplots from the RQ1 perf-stats CSV."
    )
    parser.add_argument(
        "--csv",
        default="benchmarks/reports/thesis-figures/rq1/rq1_perf_stats.csv",
        metavar="PATH",
        help="RQ1 perf-stats CSV exported by export_rq1_perf_stats.py.",
    )
    parser.add_argument(
        "--output",
        default="benchmarks/reports/thesis-figures/rq1/rq1_perf_stats_speedup_boxplots.png",
        metavar="PATH",
        help="Output PNG path.",
    )
    return parser.parse_args()


def _parse_speedup(cell: str) -> float:
    return float(cell.removesuffix("x"))


def _geomean(values: list[float]) -> float:
    return math.exp(sum(math.log(v) for v in values) / len(values))


def _load_speedups(csv_path: Path) -> tuple[list[float], list[float]]:
    default_speedups: list[float] = []
    best_speedups: list[float] = []
    with csv_path.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        for row in reader:
            default_speedups.append(
                _parse_speedup(row["Default pipeline @amortized-speedup"])
            )
            best_speedups.append(
                _parse_speedup(row["Best UC pipeline @amortized-speedup"])
            )
    if len(default_speedups) != 18 or len(best_speedups) != 18:
        raise RuntimeError(
            "Expected 18 kernels per pipeline in the RQ1 perf-stats CSV."
        )
    return default_speedups, best_speedups


def main() -> None:
    args = _parse_args()
    csv_path = Path(args.csv)
    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)

    default_speedups, best_speedups = _load_speedups(csv_path)
    geomeans = [_geomean(default_speedups), _geomean(best_speedups)]
    labels = ["Default pipeline", "max(U/S) pipeline"]
    colors = ["#355070", "#b56576"]
    data = [default_speedups, best_speedups]
    ypos = np.array([2, 1], dtype=float)

    fig, ax = plt.subplots(figsize=(9.2, 2), constrained_layout=True)
    bp = ax.boxplot(
        data,
        positions=ypos,
        vert=False,
        widths=0.3,
        patch_artist=True,
        tick_labels=labels,
        medianprops={"color": "white", "linewidth": 1.8},
        whiskerprops={"color": "#444444", "linewidth": 1.2},
        capprops={"color": "#444444", "linewidth": 1.2},
        boxprops={"edgecolor": "#222222", "linewidth": 1.2},
        flierprops={
            "marker": "o",
            "markersize": 4.5,
            "markerfacecolor": "#222222",
            "markeredgecolor": "#222222",
            "alpha": 0.7,
        },
    )

    for patch, color in zip(bp["boxes"], colors):
        patch.set_facecolor(color)
        patch.set_alpha(0.9)

    # Show every kernel as a slightly jittered point on top of its boxplot.
    rng = np.random.default_rng(0)
    for values, y, color in zip(data, ypos, colors):
        jitter = rng.uniform(-0.06, 0.06, size=len(values))
        ax.scatter(
            values,
            np.full(len(values), y) + jitter,
            s=24,
            color=color,
            edgecolors="white",
            linewidths=0.5,
            alpha=0.9,
            zorder=3,
        )

    ax.scatter(
        geomeans,
        ypos,
        marker="D",
        s=58,
        color="#f4a261",
        edgecolors="#222222",
        linewidths=0.9,
        zorder=4,
        label="Geomean",
    )

    for x, y in zip(geomeans, ypos):
        ax.annotate(
            f"{x:.3f}x",
            (x, y),
            xytext=(0, -13),
            textcoords="offset points",
            va="top",
            ha="center",
            fontsize=9,
            color="#222222",
            zorder=5,
        )

    ax.axvline(1.0, color="black", linestyle="--", linewidth=1.0)
    ax.set_xlabel("Amortized speedup")
    ax.grid(axis="x", color="#d9d9d9", linewidth=0.8)
    ax.set_axisbelow(True)
    ax.legend(loc="lower right")

    fig.savefig(output, dpi=220, bbox_inches="tight")
    plt.close(fig)
    print(f"Saved: {output}")


if __name__ == "__main__":
    main()
