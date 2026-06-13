#!/usr/bin/env python3
"""Plot raw and best-so-far Optuna objective values from optim_trial_params."""

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


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Plot Optuna convergence from optim_trial_params.")
    parser.add_argument("--db", default=None, metavar="PATH")
    parser.add_argument("--study-name", required=True, metavar="STR")
    parser.add_argument(
        "--output",
        default="benchmarks/reports/thesis-figures/rq4/rq4_optuna_convergence.png",
        metavar="PATH",
    )
    return parser.parse_args()


def main() -> None:
    args = _parse_args()
    db_path = resolve_db_path(args.db)
    con = duckdb.connect(str(db_path), read_only=True)
    rows = con.execute(
        """
        SELECT trial_id, obj_combined_ns / 1e6 AS objective_ms
        FROM optim_trial_params
        WHERE study_name = ?
        ORDER BY trial_id
        """,
        [args.study_name],
    ).fetchall()
    con.close()
    if not rows:
        raise RuntimeError(f"No Optuna data found for study '{args.study_name}'.")

    trial_ids = np.array([row[0] for row in rows], dtype=float)
    values = np.array([row[1] for row in rows], dtype=float)
    best_so_far = np.minimum.accumulate(values)
    best_idx = int(np.argmin(best_so_far))
    best_trial = int(trial_ids[best_idx])

    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)

    fig, ax = plt.subplots(figsize=(8.5, 4.8), constrained_layout=True)
    ax.plot(trial_ids, values, "o", alpha=0.65, label="Trial objective")
    ax.plot(trial_ids, best_so_far, "-", linewidth=2, label="Best so far")
    ax.scatter([best_trial], [best_so_far[best_idx]], color="red", zorder=5)
    ax.annotate(
        f"Best trial {best_trial}\n{best_so_far[best_idx]:.3f} ms",
        xy=(best_trial, best_so_far[best_idx]),
        xytext=(best_trial + 2, best_so_far[best_idx] * 1.05),
        arrowprops={"arrowstyle": "->", "linewidth": 1.0},
        fontsize=9,
    )
    ax.set_xlabel("Trial")
    ax.set_ylabel("Combined objective [ms]")
    ax.set_title(f"Optuna Convergence ({args.study_name})")
    ax.legend()
    fig.savefig(output, dpi=200, bbox_inches="tight")
    plt.close(fig)
    print(f"Saved: {output}")


if __name__ == "__main__":
    main()
