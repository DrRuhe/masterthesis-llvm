#!/usr/bin/env python3
"""Export the RQ1 perf-stats table as CSV and Typst."""

from __future__ import annotations

import argparse
import csv
import math
from dataclasses import dataclass
from pathlib import Path

import duckdb

from report_utils import open_db, resolve_db_path


EXPECTED_CONFIGS = ("default", "per_uc_best")
EXPECTED_KERNELS = (
    "apply_row_delta",
    "batch_delta",
    "box_filter",
    "column_scan",
    "count_matching_rows",
    "edge_detection",
    "email_match",
    "generic_sort",
    "grouped_count",
    "grouped_minmax",
    "grouped_sum",
    "multi_agg_delta",
    "multi_key_sort",
    "multi_pattern_match",
    "multi_predicate",
    "separable_gaussian",
    "struct_sort",
    "url_match",
)

CONFIG_LABELS = {
    "default": "Default pipeline",
    "per_uc_best": "$max(U/S)$ pipeline",
}


@dataclass(frozen=True)
class KernelStats:
    kernel: str
    config_name: str
    jit_ms: float
    spec_ms: float
    unspec_ms: float
    amortized_speedup: float
    profitable_unspec_ms: float | None


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Export the final RQ1 perf-stats table as CSV and Typst.",
    )
    parser.add_argument(
        "--db",
        default="benchmarks/benchmarks.duckdb",
        metavar="PATH",
        help="DuckDB database path.",
    )
    parser.add_argument(
        "--study-name",
        required=True,
        metavar="STR",
        help="Study prefix. Matches both the exact study name and any '<study>__*' shards.",
    )
    parser.add_argument(
        "--abstraction",
        default="low",
        metavar="STR",
        help="Benchmark abstraction level to export (default: low).",
    )
    parser.add_argument(
        "--size",
        default="MEDIUM",
        metavar="STR",
        help="Benchmark size class to export (default: MEDIUM).",
    )
    parser.add_argument(
        "--csv-out",
        required=True,
        metavar="PATH",
        help="Output CSV path.",
    )
    parser.add_argument(
        "--typst-out",
        required=True,
        metavar="PATH",
        help="Output Typst snippet path.",
    )
    return parser.parse_args()


def fetch_rows(
    con: duckdb.DuckDBPyConnection,
    study_name: str,
    abstraction: str,
    size: str,
) -> list[KernelStats]:
    rows = con.execute(
        """
        WITH phase_rows AS (
            SELECT
                a.config_name,
                p.kernel,
                p.kv_a AS abstraction,
                p.kv_s AS size,
                p.phase,
                CASE
                    WHEN p.phase = 'jit_overhead'
                    THEN p.cpu_time * CASE p.time_unit
                        WHEN 'ns' THEN 1.0
                        WHEN 'us' THEN 1e3
                        WHEN 'ms' THEN 1e6
                        WHEN 's'  THEN 1e9
                    END
                    ELSE p.real_time * CASE p.time_unit
                        WHEN 'ns' THEN 1.0
                        WHEN 'us' THEN 1e3
                        WHEN 'ms' THEN 1e6
                        WHEN 's'  THEN 1e9
                    END
                END AS phase_ns
            FROM ablation_studies a
            JOIN v_parsed p USING (run_id)
            WHERE (a.study_name = ? OR a.study_name LIKE ?)
              AND p.kv_a = ?
              AND p.kv_s = ?
              AND p.phase IN ('jit_overhead', 'specialized_exec', 'unspecialized')
        ),
        medians AS (
            SELECT
                config_name,
                kernel,
                MEDIAN(CASE WHEN phase = 'jit_overhead' THEN phase_ns END) / 1e6 AS jit_ms,
                MEDIAN(CASE WHEN phase = 'specialized_exec' THEN phase_ns END) / 1e6 AS spec_ms,
                MEDIAN(CASE WHEN phase = 'unspecialized' THEN phase_ns END) / 1e6 AS unspec_ms
            FROM phase_rows
            GROUP BY config_name, kernel
        )
        SELECT
            kernel,
            config_name,
            jit_ms,
            spec_ms,
            unspec_ms,
            unspec_ms / spec_ms AS amortized_speedup
        FROM medians
        ORDER BY kernel, config_name
        """,
        [study_name, f"{study_name}__%", abstraction, size],
    ).fetchall()

    stats = []
    for kernel, config_name, jit_ms, spec_ms, unspec_ms, amortized_speedup in rows:
        profitable_unspec_ms = None
        if amortized_speedup > 1.0:
            profitable_unspec_ms = (
                amortized_speedup * jit_ms / (amortized_speedup - 1.0)
            )
        stats.append(
            KernelStats(
                kernel=kernel,
                config_name=config_name,
                jit_ms=float(jit_ms),
                spec_ms=float(spec_ms),
                unspec_ms=float(unspec_ms),
                amortized_speedup=float(amortized_speedup),
                profitable_unspec_ms=profitable_unspec_ms,
            )
        )
    return stats


def validate_rows(rows: list[KernelStats]) -> None:
    if len(rows) != len(EXPECTED_CONFIGS) * len(EXPECTED_KERNELS):
        raise SystemExit(
            f"Expected {len(EXPECTED_CONFIGS) * len(EXPECTED_KERNELS)} rows, got {len(rows)}."
        )

    configs = sorted({row.config_name for row in rows})
    if configs != sorted(EXPECTED_CONFIGS):
        raise SystemExit(f"Expected configs {EXPECTED_CONFIGS}, got {tuple(configs)}.")

    kernels = sorted({row.kernel for row in rows})
    if kernels != sorted(EXPECTED_KERNELS):
        raise SystemExit("Kernel coverage mismatch in final study export.")

    by_kernel: dict[str, set[str]] = {kernel: set() for kernel in EXPECTED_KERNELS}
    for row in rows:
        if not math.isfinite(row.amortized_speedup) or row.amortized_speedup <= 0.0:
            raise SystemExit(
                f"Non-finite or non-positive amortized speedup for {row.config_name}/{row.kernel}."
            )
        by_kernel[row.kernel].add(row.config_name)

    missing = [kernel for kernel, configs_seen in by_kernel.items() if configs_seen != set(EXPECTED_CONFIGS)]
    if missing:
        raise SystemExit(f"Missing pipeline rows for kernels: {', '.join(missing)}")


def format_ms(value: float | None) -> str:
    if value is None:
        return "not profitable"
    return f"{value:.1f} ms"


def format_speedup(value: float) -> str:
    return f"{value:.3f}x"


def write_csv(rows: list[KernelStats], path: Path) -> None:
    grouped: dict[str, dict[str, KernelStats]] = {}
    for row in rows:
        grouped.setdefault(row.kernel, {})[row.config_name] = row

    with path.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow(
            [
                "Kernel",
                "Default pipeline @jit-overhead",
                "Default pipeline @amortized-speedup",
                "Default pipeline U_p",
                "Best UC pipeline @jit-overhead",
                "Best UC pipeline @amortized-speedup",
                "Best UC pipeline U_p",
            ]
        )
        for kernel in EXPECTED_KERNELS:
            default = grouped[kernel]["default"]
            best = grouped[kernel]["per_uc_best"]
            writer.writerow(
                [
                    kernel,
                    format_ms(default.jit_ms),
                    format_speedup(default.amortized_speedup),
                    format_ms(default.profitable_unspec_ms),
                    format_ms(best.jit_ms),
                    format_speedup(best.amortized_speedup),
                    format_ms(best.profitable_unspec_ms),
                ]
            )


def write_typst(
    rows: list[KernelStats],
    study_name: str,
    abstraction: str,
    size: str,
    path: Path,
) -> None:
    grouped: dict[str, dict[str, KernelStats]] = {}
    for row in rows:
        grouped.setdefault(row.kernel, {})[row.config_name] = row

    lines = [
        (
            "// Generated by benchmarks/reporting/export_rq1_perf_stats.py from "
            f"{study_name} (slice: abstraction={abstraction}, size={size})."
        ),
        "// @jit-overhead and U_p are median milliseconds; @amortized-speedup is unitless.",
        "#table(",
        "  columns: (1.7fr, 1fr, 1fr, 1fr, 1fr, 1fr, 1fr),",
        "  table.header(",
        "    table.cell(rowspan: 2)[Kernel],",
        f"    table.cell(colspan: 3)[{CONFIG_LABELS['default']}],",
        f"    table.cell(colspan: 3)[{CONFIG_LABELS['per_uc_best']}],",
        "    [@jit-overhead], [@amortized-speedup], [$U_p$],",
        "    [@jit-overhead], [@amortized-speedup], [$U_p$],",
        "  ),",
    ]

    for kernel in EXPECTED_KERNELS:
        default = grouped[kernel]["default"]
        best = grouped[kernel]["per_uc_best"]
        lines.append(
            "  "
            + ", ".join(
                [
                    f"[{kernel}]",
                    f"[{format_ms(default.jit_ms)}]",
                    f"[{format_speedup(default.amortized_speedup)}]",
                    f"[{format_ms(default.profitable_unspec_ms)}]",
                    f"[{format_ms(best.jit_ms)}]",
                    f"[{format_speedup(best.amortized_speedup)}]",
                    f"[{format_ms(best.profitable_unspec_ms)}]",
                ]
            )
            + ","
        )

    lines.append(")")
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> None:
    args = parse_args()
    db_path = resolve_db_path(args.db)
    csv_out = Path(args.csv_out)
    typst_out = Path(args.typst_out)
    csv_out.parent.mkdir(parents=True, exist_ok=True)
    typst_out.parent.mkdir(parents=True, exist_ok=True)

    con = open_db(db_path)
    try:
        rows = fetch_rows(con, args.study_name, args.abstraction, args.size)
    finally:
        con.close()

    validate_rows(rows)
    write_csv(rows, csv_out)
    write_typst(rows, args.study_name, args.abstraction, args.size, typst_out)

    print(f"Saved CSV: {csv_out}")
    print(f"Saved Typst: {typst_out}")


if __name__ == "__main__":
    main()
