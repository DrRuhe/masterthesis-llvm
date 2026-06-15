#!/usr/bin/env python3
"""Generate thesis-ready Pareto plots for all UC kernels."""

from __future__ import annotations

import argparse
from pathlib import Path
import shutil
import sys

import duckdb

sys.path.insert(0, str(Path(__file__).parent.parent))

from plot_pareto_configs import _flag_default, fetch_rows, render_kernel
from record_benchmark import resolve_db_path

KERNEL_ORDER = [
    ("uc1_sql", "count_matching_rows"),
    ("uc1_sql", "multi_predicate"),
    ("uc1_sql", "column_scan"),
    ("uc2_conv", "separable_gaussian"),
    ("uc2_conv", "box_filter"),
    ("uc2_conv", "edge_detection"),
    ("uc7_dfa", "email_match"),
    ("uc7_dfa", "url_match"),
    ("uc7_dfa", "multi_pattern_match"),
    ("uc8_ivm", "apply_row_delta"),
    ("uc8_ivm", "multi_agg_delta"),
    ("uc8_ivm", "batch_delta"),
    ("uc12_groupby", "grouped_sum"),
    ("uc12_groupby", "grouped_count"),
    ("uc12_groupby", "grouped_minmax"),
    ("uc14_sort", "generic_sort"),
    ("uc14_sort", "struct_sort"),
    ("uc14_sort", "multi_key_sort"),
]


def _default_thesis_output() -> Path:
    return Path(__file__).resolve().parents[4] / "docs" / "assets" / "evaluation" / "rq4" / "pareto"


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Regenerate all thesis Pareto plots for the UC corpus."
    )
    parser.add_argument(
        "--db",
        default=None,
        help="Optional path to benchmarks.duckdb.",
    )
    parser.add_argument(
        "--report-dir",
        default="benchmarks/reports/260610-pareto",
        help="Directory to regenerate `pareto_<study>_<group>_<kernel>.png/.csv` files in.",
    )
    parser.add_argument(
        "--study-name",
        default="corpus_uc_final_20260610",
        help="Study name to extract from DuckDB.",
    )
    parser.add_argument(
        "--output-dir",
        default=str(_default_thesis_output()),
        help="Thesis asset directory for copied PNGs.",
    )
    return parser.parse_args()


def main() -> None:
    args = _parse_args()
    report_dir = Path(args.report_dir)
    thesis_output_dir = Path(args.output_dir)
    report_dir.mkdir(parents=True, exist_ok=True)
    thesis_output_dir.mkdir(parents=True, exist_ok=True)

    con = duckdb.connect(str(resolve_db_path(args.db)), read_only=True)
    rows = fetch_rows(con, args.study_name)
    con.close()
    if not rows:
        raise SystemExit(f"No Pareto data found for study '{args.study_name}'.")

    grouped_rows = {(group, kernel): [] for group, kernel in KERNEL_ORDER}
    for row in rows:
        key = (row[2], row[3])
        if key in grouped_rows:
            grouped_rows[key].append(row)

    missing = [f"{group}/{kernel}" for (group, kernel), grows in grouped_rows.items() if not grows]
    if missing:
        missing_str = ", ".join(missing)
        raise SystemExit(f"Missing Pareto rows for: {missing_str}")

    for group, kernel in KERNEL_ORDER:
        grows = grouped_rows[(group, kernel)]
        default_flags = _flag_default(grows)
        filename = f"pareto_{args.study_name}_{group}_{kernel}.png"
        csv_name = f"pareto_{args.study_name}_{group}_{kernel}.csv"
        report_png = report_dir / filename
        report_csv = report_dir / csv_name
        render_kernel(
            grows,
            default_flags,
            report_png,
            report_csv,
            title=None,
        )
        shutil.copy2(report_png, thesis_output_dir / filename)
        print(f"  Copied: {thesis_output_dir / filename}")


if __name__ == "__main__":
    main()
