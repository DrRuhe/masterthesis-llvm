#!/usr/bin/env python3
"""plot_input_size_limits.py — Scatterplot: query nOps vs JIT pass/fail (spec 018).

Produces a PNG showing:
  x-axis: query_nops (log scale)
  y-axis: jit_timeout (0=pass, 1=fail) with vertical jitter
  marker color/shape: p0_optimal vs p2_optimal pipeline
  annotation: sqlite3 TU blob size (constant across all queries)

Usage:
    python3 plot_input_size_limits.py [--db PATH] [--run-ids ID1,ID2] [--output-dir DIR]
    python3 plot_input_size_limits.py  # auto-detects the two most-recent spec-018 runs
"""
import argparse
import random
import sys
from pathlib import Path

import duckdb
import ultraplot as uplt
import numpy as np

sys.path.insert(0, str(Path(__file__).parent.parent))
from record_benchmark import resolve_db_path

# ── Deterministic jitter seed ─────────────────────────────────────────────────
_RNG = np.random.default_rng(42)


def _fetch_data(con: duckdb.DuckDBPyConnection, run_ids: list[str]) -> list[dict]:
    """Fetch jit_overhead rows for the given run_ids."""
    placeholders = ", ".join("?" for _ in run_ids)
    rows = con.execute(
        f"""
        SELECT
            run_id,
            kv_n                    AS query_name,
            kv_g                    AS query_group,
            query_nops,
            query_bytecode_bytes,
            jit_timeout,
            ROUND(real_time / 1e6, 1) AS jit_ms,
            jit_blob_kb
        FROM benchmarks
        WHERE run_id IN ({placeholders})
          AND kv_t = 'jit_overhead'
          AND query_nops IS NOT NULL
        ORDER BY query_nops, kv_n
        """,
        run_ids,
    ).fetchall()
    cols = ["run_id", "query_name", "query_group", "query_nops",
            "query_bytecode_bytes", "jit_timeout", "jit_ms", "jit_blob_kb"]
    return [dict(zip(cols, r)) for r in rows]


def _auto_detect_run_ids(con: duckdb.DuckDBPyConnection) -> list[str]:
    """Return run_ids for the two most-recent spec-018 benchmark runs."""
    rows = con.execute(
        """
        SELECT DISTINCT c.run_id
        FROM context c
        JOIN benchmarks b ON b.run_id = c.run_id
        WHERE b.kv_g LIKE 'db/sqlite3/%'
          AND b.kv_t  = 'jit_overhead'
          AND b.query_nops IS NOT NULL
        ORDER BY c.run_ts DESC
        LIMIT 2
        """
    ).fetchall()
    return [r[0] for r in rows]


def main():
    parser = argparse.ArgumentParser(
        description="Scatterplot: query nOps vs JIT pass/fail for spec-018."
    )
    parser.add_argument("--db", default=None, metavar="PATH")
    parser.add_argument("--run-ids", default=None, metavar="ID1,ID2",
                        help="Comma-separated run IDs (default: two most-recent sqlite3 runs)")
    parser.add_argument("--output-dir", default=None, metavar="DIR")
    parser.add_argument("--p0-label", default="p0_optimal")
    parser.add_argument("--p2-label", default="p2_optimal")
    args = parser.parse_args()

    db_path = resolve_db_path(args.db)
    con = duckdb.connect(str(db_path), read_only=True)

    run_ids = [r.strip() for r in args.run_ids.split(",")] if args.run_ids else _auto_detect_run_ids(con)
    if not run_ids:
        print("No matching runs found in database.", file=sys.stderr)
        sys.exit(1)

    data = _fetch_data(con, run_ids)
    if not data:
        print("No jit_overhead rows found for specified run_ids.", file=sys.stderr)
        sys.exit(1)

    # Assign labels: the run with more rows is p2 (has all 22+7 queries); the
    # other is p0 (has 18 queries). Fall back to args labels.
    from collections import Counter
    counts = Counter(r["run_id"] for r in data)
    sorted_runs = sorted(run_ids, key=lambda rid: counts.get(rid, 0))
    labels = {}
    if len(sorted_runs) == 2:
        labels[sorted_runs[0]] = args.p0_label
        labels[sorted_runs[1]] = args.p2_label
    else:
        for i, rid in enumerate(sorted_runs):
            labels[rid] = f"run_{i}"

    # Get blob size annotation (same for all sqlite3 queries)
    blob_kb = next((r["jit_blob_kb"] for r in data if r["jit_blob_kb"]), None)

    # ── Plot ─────────────────────────────────────────────────────────────────
    fig, ax = uplt.subplots(figsize=(9, 5))

    colors = {"p0_optimal": "steelblue", "p2_optimal": "tomato"}
    markers = {"p0_optimal": "o", "p2_optimal": "s"}
    default_colors = uplt.rc["axes.prop_cycle"].by_key()["color"]

    seen_labels = set()
    for row in data:
        lbl = labels.get(row["run_id"], row["run_id"][:8])
        nops = row["query_nops"]
        timeout = row["jit_timeout"]
        # Vertical jitter to separate overlapping points
        y_jitter = _RNG.uniform(-0.05, 0.05)
        y = timeout + y_jitter
        color = colors.get(lbl, "grey")
        marker = markers.get(lbl, "^")
        show_label = lbl not in seen_labels
        ax.scatter(
            [nops], [y],
            color=color, marker=marker, s=60, alpha=0.85,
            label=lbl if show_label else None,
            zorder=3,
        )
        seen_labels.add(lbl)

    # Annotate blob size
    if blob_kb is not None:
        ax.text(
            0.98, 0.95,
            f"SQLite3 TU blob: {blob_kb:.0f} KB\n(2 234 funcs → 21 after prune)",
            transform=ax.transAxes,
            ha="right", va="top",
            fontsize=8,
            bbox=dict(boxstyle="round,pad=0.3", fc="lightyellow", ec="grey", alpha=0.8),
        )

    ax.set_xscale("log")
    ax.set_xlabel("Query bytecode size (nOps)", fontsize=11)
    ax.set_ylabel("JIT outcome  (0 = pass,  1 = fail/timeout)", fontsize=11)
    ax.set_title("Input size limits for SQLite3 JIT specialization (spec 018)", fontsize=12)
    ax.set_ylim(-0.25, 1.25)
    ax.set_yticks([0, 1])
    ax.set_yticklabels(["pass (0)", "fail (1)"])
    ax.legend(loc="upper left", fontsize=9)
    ax.grid(True, alpha=0.3)

    out_dir = Path(args.output_dir) if args.output_dir else db_path.parent / "reports" / "260611-sqlite3-input-size"
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / "input_size_limits.png"
    fig.savefig(str(out_path), dpi=150, bbox_inches="tight")
    print(f"Saved: {out_path}")
    print(f"Data points: {len(data)}")
    print(f"nOps range: {min(r['query_nops'] for r in data):.0f} – {max(r['query_nops'] for r in data):.0f}")
    print(f"All pass: {all(r['jit_timeout'] == 0 for r in data)}")


if __name__ == "__main__":
    main()
