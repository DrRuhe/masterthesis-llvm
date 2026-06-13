#!/usr/bin/env python3
"""Join per-kernel binary-size overhead with execution speedup and render a scatter plot."""

from __future__ import annotations

import argparse
import csv
import sys
from pathlib import Path

import duckdb
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

sys.path.insert(0, str(Path(__file__).parent.parent))
from record_benchmark import resolve_db_path


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Plot binary-size overhead versus execution speedup.")
    parser.add_argument("--db", default=None, metavar="PATH")
    parser.add_argument("--study-name", required=True, metavar="STR")
    parser.add_argument(
        "--measurements",
        default="benchmarks/reports/thesis-figures/rq2/rq2_binary_size_measurements.csv",
        metavar="PATH",
    )
    parser.add_argument(
        "--output",
        default="benchmarks/reports/thesis-figures/rq2/rq2_binary_size_vs_speedup.png",
        metavar="PATH",
    )
    return parser.parse_args()


def main() -> None:
    args = _parse_args()
    measurement_rows = {}
    with Path(args.measurements).open("r", encoding="utf-8") as fh:
        for row in csv.DictReader(fh):
            measurement_rows[row["kernel"]] = row

    db_path = resolve_db_path(args.db)
    con = duckdb.connect(str(db_path), read_only=True)
    rows = con.execute(
        """
        SELECT kernel, med_unspec_ns / med_spec_ns AS exec_speedup
        FROM v_ablation_medians
        WHERE study_name = ?
        ORDER BY kernel
        """,
        [args.study_name],
    ).fetchall()
    con.close()

    xs = []
    ys = []
    labels = []
    for kernel, speedup in rows:
        if kernel not in measurement_rows:
            continue
        xs.append(float(measurement_rows[kernel]["overhead_pct"]))
        ys.append(float(speedup))
        labels.append(kernel)

    if not xs:
        raise RuntimeError("No overlapping kernels between size measurements and speedup data.")

    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)

    fig, ax = plt.subplots(figsize=(7.5, 5), constrained_layout=True)
    ax.scatter(xs, ys, color="#355070")
    for x, y, label in zip(xs, ys, labels):
        ax.annotate(label, (x, y), fontsize=8, xytext=(4, 4), textcoords="offset points")
    ax.axhline(1.0, color="black", linestyle="--", linewidth=1)
    ax.set_xlabel("Object-size overhead [%]")
    ax.set_ylabel("Execution speedup")
    ax.set_title(f"Binary-Size Overhead vs Execution Speedup ({args.study_name})")
    fig.savefig(output, dpi=200, bbox_inches="tight")
    plt.close(fig)
    print(f"Saved: {output}")


if __name__ == "__main__":
    main()
