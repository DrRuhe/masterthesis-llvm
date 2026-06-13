#!/usr/bin/env python3
"""Export the RQ3 abstraction-detail table as CSV and Typst."""

from __future__ import annotations

import argparse
import csv
import sys
from pathlib import Path

import duckdb

sys.path.insert(0, str(Path(__file__).parent.parent))
from record_benchmark import resolve_db_path


STUDIES = [
    ("Low", "abstraction_p2o3_low_20260531"),
    ("Tradeoff", "abstraction_p2o3_tradeoff_20260531"),
    ("Abstract", "abstraction_p2o3_abstract_20260531"),
]


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Export the per-kernel RQ3 abstraction table.")
    parser.add_argument("--db", default=None, metavar="PATH")
    parser.add_argument(
        "--output-dir",
        default="benchmarks/reports/thesis-figures/rq3",
        metavar="DIR",
    )
    return parser.parse_args()


def main() -> None:
    args = _parse_args()
    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    db_path = resolve_db_path(args.db)
    con = duckdb.connect(str(db_path), read_only=True)
    rows = {}
    for label, study in STUDIES:
        data = con.execute(
            """
            SELECT kernel, med_unspec_ns / med_spec_ns AS speedup
            FROM v_ablation_medians
            WHERE study_name = ?
            ORDER BY kernel
            """,
            [study],
        ).fetchall()
        for kernel, speedup in data:
            rows.setdefault(kernel, {})[label] = float(speedup)
    con.close()

    csv_path = output_dir / "rq3_abstraction_detail.csv"
    typ_path = output_dir / "rq3_abstraction_detail.typ"

    with csv_path.open("w", encoding="utf-8", newline="") as fh:
        writer = csv.writer(fh)
        writer.writerow(["kernel", "low", "tradeoff", "abstract"])
        for kernel in sorted(rows):
            writer.writerow([kernel, rows[kernel]["Low"], rows[kernel]["Tradeoff"], rows[kernel]["Abstract"]])

    lines = [
        "#table(",
        "  columns: (1.5fr, 1fr, 1fr, 1fr),",
        "  table.header([Kernel], [Low], [Tradeoff], [Abstract]),",
    ]
    for kernel in sorted(rows):
        values = rows[kernel]
        max_label = max(values, key=values.get)
        formatted = []
        for label in ["Low", "Tradeoff", "Abstract"]:
            value = f"{values[label]:.3f}x"
            if label == max_label:
                value = f"*{value}*"
            formatted.append(f"[{value}]")
        lines.append(f"  [{kernel}], {formatted[0]}, {formatted[1]}, {formatted[2]},")
    lines.append(")")
    typ_path.write_text("\n".join(lines) + "\n", encoding="utf-8")

    print(f"Saved: {csv_path}")
    print(f"Saved: {typ_path}")


if __name__ == "__main__":
    main()
