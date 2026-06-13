#!/usr/bin/env python3
"""Render the thesis RQ3 suite-family comparison figure."""

from __future__ import annotations

import argparse
import json
import math
import re
import sys
from pathlib import Path

import duckdb
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

sys.path.insert(0, str(Path(__file__).parent.parent))
from record_benchmark import resolve_db_path


ABSTRACTION_STUDIES = {
    "Low": "abstraction_p2o3_low_20260531",
    "Tradeoff": "abstraction_p2o3_tradeoff_20260531",
    "Abstract": "abstraction_p2o3_abstract_20260531",
}


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate the thesis RQ3 family comparison figure.")
    parser.add_argument("--db", default=None, metavar="PATH")
    parser.add_argument(
        "--polybench-default",
        default="benchmarks/reports/260610-polybench/polybench_default.json",
        metavar="PATH",
    )
    parser.add_argument(
        "--polybench-p2",
        default="benchmarks/reports/260610-polybench/polybench_p2_optimal.json",
        metavar="PATH",
    )
    parser.add_argument(
        "--tpch-module-stats",
        default="benchmarks/reports/260610-tpch-scope/module_stats.txt",
        metavar="PATH",
    )
    parser.add_argument(
        "--output",
        default="benchmarks/reports/thesis-figures/rq3/rq3_suite_family_comparison.png",
        metavar="PATH",
    )
    return parser.parse_args()


def _geomean(values: list[float]) -> float:
    return math.exp(sum(math.log(v) for v in values) / len(values))


def _load_uc_panel(con: duckdb.DuckDBPyConnection) -> tuple[list[str], list[float]]:
    labels: list[str] = []
    values: list[float] = []
    for label, study in ABSTRACTION_STUDIES.items():
        rows = con.execute(
            """
            SELECT med_unspec_ns / med_spec_ns AS speedup
            FROM v_ablation_medians
            WHERE study_name = ?
            ORDER BY kernel
            """,
            [study],
        ).fetchall()
        labels.append(label)
        values.append(_geomean([float(row[0]) for row in rows]))
    return labels, values


def _load_polybench_speedups(path: Path) -> list[float]:
    payload = json.loads(path.read_text(encoding="utf-8"))
    out: list[float] = []
    for benchmark in payload["benchmarks"]:
        name = benchmark.get("run_name", "")
        if ";t:specialized_exec;" not in name:
            continue
        spec = benchmark["real_time"]
        unspec_name = name.replace(";t:specialized_exec;", ";t:unspecialized;")
        unspec = next(
            item["real_time"] for item in payload["benchmarks"] if item.get("run_name") == unspec_name
        )
        out.append(float(unspec) / float(spec))
    return out


def _load_tpch_panel(path: Path) -> tuple[list[str], list[float]]:
    text = path.read_text(encoding="utf-8")
    instrs = float(re.search(r"255,342", text).group(0).replace(",", ""))
    funcs = float(re.search(r"2,017", text).group(0).replace(",", ""))
    blob_kib = float(re.search(r"4,078 KB", text).group(0).split()[0].replace(",", ""))
    return ["Instrs / 1k", "Functions", "Blob KiB"], [instrs / 1000.0, funcs, blob_kib]


def main() -> None:
    args = _parse_args()
    db_path = resolve_db_path(args.db)
    con = duckdb.connect(str(db_path), read_only=True)
    uc_labels, uc_values = _load_uc_panel(con)
    con.close()

    poly_default = _load_polybench_speedups(Path(args.polybench_default))
    poly_p2 = _load_polybench_speedups(Path(args.polybench_p2))
    tpch_labels, tpch_values = _load_tpch_panel(Path(args.tpch_module_stats))

    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    fig, axes = plt.subplots(1, 3, figsize=(14, 4.7), constrained_layout=True)

    axes[0].bar(np.arange(len(uc_labels)), uc_values, color=["#355070", "#6d597a", "#b56576"])
    axes[0].axhline(1.0, color="black", linestyle="--", linewidth=1)
    axes[0].set_xticks(np.arange(len(uc_labels)), uc_labels, rotation=15, ha="right")
    axes[0].set_ylabel("Geomean speedup")
    axes[0].set_title("UC Abstraction Tiers")

    axes[1].boxplot([poly_default, poly_p2], tick_labels=["Default", "P2 optimal"], patch_artist=True)
    axes[1].axhline(1.0, color="black", linestyle="--", linewidth=1)
    axes[1].set_ylabel("Per-kernel speedup")
    axes[1].set_title("PolyBench")

    axes[2].bar(np.arange(len(tpch_labels)), tpch_values, color=["#457b9d", "#e76f51", "#f4a261"])
    axes[2].set_xticks(np.arange(len(tpch_labels)), tpch_labels, rotation=20, ha="right")
    axes[2].set_title("SQLite/TPC-H Boundary Case")

    fig.suptitle("Specialization Benefit Depends on Exposed Structure, Not Abstraction Alone", fontsize=13)
    fig.savefig(output, dpi=200, bbox_inches="tight")
    plt.close(fig)
    print(f"Saved: {output}")


if __name__ == "__main__":
    main()
