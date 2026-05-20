#!/usr/bin/env python3
"""plot_pareto.py — Pareto frontier plots for JIT pipeline optimization results."""
import argparse
import json
import sys
from pathlib import Path

import duckdb
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

sys.path.insert(0, str(Path(__file__).parent.parent))
from record_benchmark import resolve_db_path
from report_utils import make_report_dir

_DEFAULT_PARAMS = {"fixpoint_max": 10, "unroll_max": 128, "early_prune": 1, "o3_final": 1}


def pareto_front(points):
    """Return indices of Pareto-optimal points (minimize both axes).

    A point p is Pareto-optimal iff no other point q satisfies q <= p elementwise with
    at least one strictly less. Equivalently, p is dominated iff some q has all coords
    <= p AND at least one coord strictly less.
    """
    pts = np.array(points)
    n = len(pts)
    is_pareto = np.ones(n, dtype=bool)
    for i in range(n):
        # Some other point dominates pts[i] iff all coords <= pts[i] AND at least one <.
        leq = np.all(pts <= pts[i], axis=1)
        lt_any = np.any(pts < pts[i], axis=1)
        dominators = leq & lt_any
        if np.any(dominators):
            is_pareto[i] = False
    return np.where(is_pareto)[0]


def _params_distance(p: dict) -> float:
    """Sum of squared relative deviations from Default config keys."""
    dist = 0.0
    for k, ref in _DEFAULT_PARAMS.items():
        v = p.get(k, ref)
        dist += ((v - ref) / max(abs(ref), 1)) ** 2
    return dist


def main():
    parser = argparse.ArgumentParser(
        description="Scatter/Pareto plots of JIT tuning trials from v_optim_breakeven."
    )
    parser.add_argument("--db", default=None, metavar="PATH")
    parser.add_argument("--study-name", required=True, metavar="STR")
    parser.add_argument("--output-dir", default=None, metavar="DIR")
    args = parser.parse_args()

    db_path = resolve_db_path(args.db)
    con = duckdb.connect(str(db_path), read_only=True)

    groups = con.execute(
        'SELECT DISTINCT "group" FROM v_optim_breakeven WHERE study_name=? ORDER BY 1',
        [args.study_name],
    ).fetchall()

    if not groups:
        print(f"No data found for study '{args.study_name}'.", file=sys.stderr)
        sys.exit(1)

    if args.output_dir:
        output_dir = Path(args.output_dir)
        output_dir.mkdir(parents=True, exist_ok=True)
    else:
        output_dir = make_report_dir(__file__, parser, args)
    study_dir = output_dir / f"pareto_{args.study_name}"
    study_dir.mkdir(parents=True, exist_ok=True)

    for (group,) in groups:
        rows = con.execute(
            """SELECT t_jit_ns, t_spec_ns, params_json
               FROM v_optim_breakeven
               WHERE study_name=? AND "group"=?
                 AND NOT used_timeout_fallback
                 AND t_jit_ns IS NOT NULL AND t_spec_ns IS NOT NULL""",
            [args.study_name, group],
        ).fetchall()
        if not rows:
            continue

        jit_ms = np.array([r[0] / 1e6 for r in rows])
        spec_ms = np.array([r[1] / 1e6 for r in rows])
        combined = jit_ms + spec_ms

        fig, ax = plt.subplots(figsize=(8, 6))

        sc = ax.scatter(
            jit_ms, spec_ms,
            c=combined, cmap="viridis_r",
            alpha=0.6, s=25, zorder=2,
        )
        fig.colorbar(sc, ax=ax, label="JIT + exec (ms)")

        # Mark Default reference — closest trial to the canonical Default params.
        best_idx, best_dist = None, float("inf")
        for i, r in enumerate(rows):
            try:
                p = json.loads(r[2]) if r[2] else {}
            except Exception:
                p = {}
            d = _params_distance(p)
            if d < best_dist:
                best_dist, best_idx = d, i

        if best_idx is not None:
            ax.scatter(
                [jit_ms[best_idx]], [spec_ms[best_idx]],
                marker="*", s=250, color="red", zorder=5, label="Default config",
            )

        # Pareto frontier — non-dominated (minimize both axes).
        pts = list(zip(jit_ms, spec_ms))
        pareto_idx = pareto_front(pts)
        pareto_pts = sorted([pts[i] for i in pareto_idx], key=lambda x: x[0])
        if pareto_pts:
            px = [p[0] for p in pareto_pts]
            py = [p[1] for p in pareto_pts]
            ax.plot(px, py, "g-o", linewidth=2, markersize=6, zorder=4, label="Pareto front")

        ax.set_xlabel("JIT overhead (ms)")
        ax.set_ylabel("Specialized exec (ms)")
        ax.set_title(f"Pareto frontier: {group}\n(study: {args.study_name})")
        ax.legend()

        out_path = study_dir / f"{group}.png"
        fig.savefig(out_path, dpi=150, bbox_inches="tight")
        plt.close(fig)
        print(f"  Saved: {out_path}")

    con.close()


if __name__ == "__main__":
    main()
