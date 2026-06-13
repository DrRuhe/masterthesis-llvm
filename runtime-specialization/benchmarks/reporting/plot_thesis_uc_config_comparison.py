#!/usr/bin/env python3
"""Thesis-specific 2-panel UC final configuration comparison plot."""

from __future__ import annotations

import argparse
import math
import sys
from pathlib import Path

import duckdb
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

sys.path.insert(0, str(Path(__file__).parent.parent))
from record_benchmark import resolve_db_path


CONFIGS = ["default", "p0_o3_optimal", "uc_workload_optimal", "no_o3_final"]


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Plot geomean speedup and median JIT overhead for the final UC comparison."
    )
    parser.add_argument("--db", default=None, metavar="PATH")
    parser.add_argument("--study-name", required=True, metavar="STR")
    parser.add_argument(
        "--output",
        default="benchmarks/reports/thesis-figures/rq1/rq1_pipeline_comparison.png",
        metavar="PATH",
    )
    return parser.parse_args()


def _geomean(values: list[float]) -> float:
    return math.exp(sum(math.log(v) for v in values) / len(values))


def main() -> None:
    args = _parse_args()
    db_path = resolve_db_path(args.db)
    con = duckdb.connect(str(db_path), read_only=True)
    rows = con.execute(
        """
        SELECT config_name, kernel, med_unspec_ns / med_spec_ns AS exec_speedup, med_jit_ns / 1e6 AS jit_ms
        FROM v_ablation_medians
        WHERE study_name = ?
          AND config_name IN ('default', 'p0_o3_optimal', 'uc_workload_optimal', 'no_o3_final')
        ORDER BY config_name, kernel
        """,
        [args.study_name],
    ).fetchall()
    con.close()
    if not rows:
        raise RuntimeError(f"No ablation data found for study '{args.study_name}'.")

    speedups: dict[str, list[float]] = {cfg: [] for cfg in CONFIGS}
    jit_ms: dict[str, list[float]] = {cfg: [] for cfg in CONFIGS}
    for config_name, _kernel, exec_speedup, jit_value in rows:
        speedups[config_name].append(float(exec_speedup))
        jit_ms[config_name].append(float(jit_value))

    geo = [_geomean(speedups[cfg]) for cfg in CONFIGS]
    med = [float(np.median(jit_ms[cfg])) for cfg in CONFIGS]

    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)

    fig, axes = plt.subplots(1, 2, figsize=(11, 4.8), constrained_layout=True)
    colors = ["#355070", "#6d597a", "#b56576", "#e56b6f"]
    xpos = np.arange(len(CONFIGS))

    axes[0].bar(xpos, geo, color=colors, edgecolor="white")
    axes[0].axhline(1.0, color="black", linestyle="--", linewidth=1)
    axes[0].set_xticks(xpos, CONFIGS, rotation=20, ha="right")
    axes[0].set_ylabel("Geomean exec speedup")
    axes[0].set_title("Steady-State Execution Speedup")

    axes[1].bar(xpos, med, color=colors, edgecolor="white")
    axes[1].set_xticks(xpos, CONFIGS, rotation=20, ha="right")
    axes[1].set_ylabel("Median JIT overhead [ms]")
    axes[1].set_title("Per-Kernel JIT Overhead")

    fig.suptitle(f"Final UC Comparison ({args.study_name})", fontsize=13)
    fig.savefig(output, dpi=200, bbox_inches="tight")
    plt.close(fig)
    print(f"Saved: {output}")


if __name__ == "__main__":
    main()
