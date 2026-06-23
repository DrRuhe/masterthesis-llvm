#!/usr/bin/env python3
"""Export PolyBench partial-specialization thesis artifacts."""

from __future__ import annotations

import argparse
import csv
import math
from pathlib import Path

import duckdb
import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt

from report_utils import open_db, resolve_db_path


SIZE_ORDER = ["SMALL", "MEDIUM", "LARGE", "EXTRALARGE"]
REPO_ROOT = Path(__file__).resolve().parents[2]
APPENDIX_TEMPLATE_PATH = REPO_ROOT / "benchmarks/reports/thesis-figures/rq3/polybench_partial_specialization_appendix.typ"
EXPECTED_KERNEL_COUNT = 30


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate thesis artifacts for the PolyBench partial-specialization study."
    )
    parser.add_argument("--db", default="benchmarks/benchmarks.duckdb", metavar="PATH")
    parser.add_argument("--study-name", required=True, metavar="STR")
    parser.add_argument(
        "--output-dir",
        default="benchmarks/reports/thesis-figures/rq3",
        metavar="PATH",
    )
    parser.add_argument(
        "--stability-threshold",
        default=0.20,
        type=float,
        metavar="FLOAT",
        help="Automation heuristic for deciding whether size scaling is stable enough "
        "to emit the summary table. The final thesis claim remains qualitative.",
    )
    return parser.parse_args()


def load_rows(con: duckdb.DuckDBPyConnection, study_name: str) -> list[dict[str, object]]:
    rows = con.execute(
        """
        WITH phase_rows AS (
            SELECT
                a.config_name,
                p.kernel,
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
              AND a.config_name = 'default'
              AND p."group" = 'polybench'
              AND p.kv_s IN ('SMALL', 'MEDIUM', 'LARGE', 'EXTRALARGE')
              AND p.phase IN ('jit_overhead', 'specialized_exec', 'unspecialized')
        ),
        medians AS (
            SELECT
                kernel,
                size,
                MEDIAN(CASE WHEN phase = 'jit_overhead' THEN phase_ns END) / 1e6 AS jit_ms,
                MEDIAN(CASE WHEN phase = 'specialized_exec' THEN phase_ns END) / 1e6 AS spec_ms,
                MEDIAN(CASE WHEN phase = 'unspecialized' THEN phase_ns END) / 1e6 AS unspec_ms
            FROM phase_rows
            GROUP BY kernel, size
        )
        SELECT
            kernel,
            size,
            jit_ms,
            spec_ms,
            unspec_ms,
            unspec_ms / spec_ms AS amortized_speedup
        FROM medians
        ORDER BY kernel, size
        """,
        [study_name, f"{study_name}__%"],
    ).fetchall()
    out: list[dict[str, object]] = []
    for kernel, size, jit_ms, spec_ms, unspec_ms, speedup in rows:
        up_ms = None
        if speedup and float(speedup) > 1.0:
            up_ms = float(speedup) * float(jit_ms) / (float(speedup) - 1.0)
        out.append(
            {
                "kernel": str(kernel),
                "size": str(size),
                "jit_ms": float(jit_ms),
                "spec_ms": float(spec_ms),
                "unspec_ms": float(unspec_ms),
                "amortized_speedup": float(speedup),
                "u_p_ms": up_ms,
            }
        )
    return out


def validate_rows(rows: list[dict[str, object]]) -> list[str]:
    kernels = sorted({row["kernel"] for row in rows})
    if len(kernels) != EXPECTED_KERNEL_COUNT:
        raise SystemExit(f"Expected {EXPECTED_KERNEL_COUNT} PolyBench kernels, got {len(kernels)}.")
    expected_points = len(kernels) * len(SIZE_ORDER)
    if len(rows) != expected_points:
        raise SystemExit(f"Expected {expected_points} PolyBench size points, got {len(rows)}.")
    for kernel in kernels:
        seen = {row["size"] for row in rows if row["kernel"] == kernel}
        if seen != set(SIZE_ORDER):
            raise SystemExit(f"Kernel {kernel} does not cover the expected size tiers.")
    return kernels


def plot_metric(
    rows: list[dict[str, object]],
    kernels: list[str],
    metric: str,
    ylabel: str,
    output: Path,
    log_scale: bool,
) -> None:
    fig, axes = plt.subplots(5, 6, figsize=(18, 12), constrained_layout=True, sharex=True)
    axes = axes.flatten()
    x_positions = list(range(len(SIZE_ORDER)))

    for index, kernel in enumerate(kernels):
        ax = axes[index]
        kernel_rows = [row for row in rows if row["kernel"] == kernel]
        kernel_rows.sort(key=lambda row: SIZE_ORDER.index(str(row["size"])))
        y_values = [float(row[metric]) for row in kernel_rows]
        ax.plot(x_positions, y_values, marker="o", linewidth=1.5, color="#355070")
        ax.set_title(kernel, fontsize=9)
        ax.set_xticks(x_positions)
        ax.set_xticklabels(SIZE_ORDER, rotation=35, ha="right", fontsize=8)
        if log_scale:
            ax.set_yscale("log")
            if metric == "amortized_speedup":
                ax.axhline(1.0, color="black", linestyle=":", linewidth=0.8)
        ax.tick_params(axis="y", labelsize=8)

    for ax in axes[len(kernels):]:
        ax.axis("off")

    fig.supylabel(ylabel, fontsize=12)
    fig.savefig(output, dpi=200, bbox_inches="tight")
    plt.close(fig)


def relative_spread(values: list[float]) -> float:
    ordered = sorted(values)
    center = ordered[len(ordered) // 2]
    if center <= 0:
        return math.inf
    return (max(values) - min(values)) / center


def compute_summary(
    rows: list[dict[str, object]], threshold: float
) -> tuple[bool, list[dict[str, object]], list[str]]:
    kernels = sorted({row["kernel"] for row in rows})
    summary_rows: list[dict[str, object]] = []
    unstable: list[str] = []
    for kernel in kernels:
        kernel_rows = [row for row in rows if row["kernel"] == kernel]
        kernel_rows.sort(key=lambda row: SIZE_ORDER.index(str(row["size"])))
        speedups = [float(row["amortized_speedup"]) for row in kernel_rows]
        jit_ms = [float(row["jit_ms"]) for row in kernel_rows]
        if relative_spread(speedups) > threshold or relative_spread(jit_ms) > threshold:
            unstable.append(kernel)
        medium = next(row for row in kernel_rows if row["size"] == "MEDIUM")
        summary_rows.append(medium)
    return (len(unstable) == 0, summary_rows, unstable)


def write_summary_csv(rows: list[dict[str, object]], path: Path) -> None:
    with path.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow(["Kernel", "Amortized speedup", "U_p"])
        for row in rows:
            writer.writerow(
                [
                    row["kernel"],
                    f"{float(row['amortized_speedup']):.3f}x",
                    "not profitable" if row["u_p_ms"] is None else f"{float(row['u_p_ms']):.1f} ms",
                ]
            )


def write_summary_typst(rows: list[dict[str, object]], study_name: str, path: Path) -> None:
    lines = [
        "// Source: generated by benchmarks/reporting/export_polybench_partial_specialization.py",
        f"// Study: {study_name}",
        "#table(",
        "  columns: 3,",
        "  align: (left, right, right),",
        "  [Kernel], [@amortized-speedup], [$U_p$],",
    ]
    for row in rows:
        up_text = "not profitable" if row["u_p_ms"] is None else f"{float(row['u_p_ms']):.1f} ms"
        lines.append(
            f"  [`{row['kernel']}`], [{float(row['amortized_speedup']):.3f}x], [{up_text}],"
        )
    lines.append(")")
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def write_appendix_typst(path: Path) -> None:
    path.write_text(APPENDIX_TEMPLATE_PATH.read_text(encoding="utf-8"), encoding="utf-8")


def write_provenance(
    study_name: str,
    threshold: float,
    emitted_summary: bool,
    unstable: list[str],
    path: Path,
) -> None:
    lines = [
        f"Study: {study_name}",
        "Config: default",
        "Sizes: SMALL, MEDIUM, LARGE, EXTRALARGE",
        "Metrics: amortized speedup = U / S, jit time = J, U_p = ((U / S) J) / ((U / S) - 1)",
        f"Summary table heuristic threshold: relative spread <= {threshold:.2f}",
        f"Summary table emitted: {'yes' if emitted_summary else 'no'}",
    ]
    if unstable:
        lines.append("Kernels exceeding heuristic threshold: " + ", ".join(unstable))
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> None:
    args = parse_args()
    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    con = open_db(resolve_db_path(args.db))
    try:
        rows = load_rows(con, args.study_name)
    finally:
        con.close()

    kernels = validate_rows(rows)

    plot_metric(
        rows,
        kernels,
        "amortized_speedup",
        "amortized speedup (U / S)",
        output_dir / "polybench_partial_amortized_speedup.png",
        log_scale=True,
    )
    plot_metric(
        rows,
        kernels,
        "jit_ms",
        "jit time [ms]",
        output_dir / "polybench_partial_jit_time.png",
        log_scale=True,
    )

    emitted_summary, summary_rows, unstable = compute_summary(rows, args.stability_threshold)
    if emitted_summary:
        write_summary_csv(summary_rows, output_dir / "polybench_partial_summary.csv")
        write_summary_typst(
            summary_rows,
            args.study_name,
            output_dir / "polybench_partial_summary.typ",
        )

    write_appendix_typst(output_dir / "polybench_partial_specialization_appendix.typ")
    write_provenance(
        args.study_name,
        args.stability_threshold,
        emitted_summary,
        unstable,
        output_dir / "polybench_partial_provenance.txt",
    )

    print(f"Study: {args.study_name}")
    print(f"Saved: {output_dir / 'polybench_partial_amortized_speedup.png'}")
    print(f"Saved: {output_dir / 'polybench_partial_jit_time.png'}")
    print(f"Saved: {output_dir / 'polybench_partial_specialization_appendix.typ'}")
    if emitted_summary:
        print(f"Saved: {output_dir / 'polybench_partial_summary.csv'}")
        print(f"Saved: {output_dir / 'polybench_partial_summary.typ'}")
    else:
        print("Summary table skipped by the automation heuristic; see provenance note.")
    print(f"Saved: {output_dir / 'polybench_partial_provenance.txt'}")


if __name__ == "__main__":
    main()
