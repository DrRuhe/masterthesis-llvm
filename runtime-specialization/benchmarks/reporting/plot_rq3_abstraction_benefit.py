#!/usr/bin/env python3
"""Render the RQ3 abstraction-benefit aggregate plot and per-kernel grid."""

from __future__ import annotations

import argparse
import math
import os
from pathlib import Path

import duckdb

if "MPLCONFIGDIR" not in os.environ:
    os.environ["MPLCONFIGDIR"] = "/tmp/matplotlib-runtime-specialization"

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
from matplotlib.patches import Patch
import numpy as np
import pandas as pd

plt.rcParams.update(
    {
        "font.size": 11,
        "axes.titlesize": 11,
        "axes.labelsize": 11,
        "xtick.labelsize": 10,
        "ytick.labelsize": 10,
        "legend.fontsize": 10,
        "legend.title_fontsize": 10,
    }
)

from report_utils import open_db, resolve_db_path, save_csv, save_plot


DEFAULT_STUDY = "corpus_uc_first_call_quadrants_20260620_160209"
DEFAULT_CONFIG = "default"
DEFAULT_SIZE = "MEDIUM"

ABSTRACTION_ORDER = ("low", "tradeoff", "abstract")
PHASE_ORDER = ("unspecialized", "specialized_exec")
EXPECTED_KERNEL_COUNT = 18

ABSTRACTION_LABELS = {
    "low": "low",
    "tradeoff": "tradeoff",
    "abstract": "abstract",
}
PHASE_LABELS = {
    "unspecialized": "Unspec",
    "specialized_exec": "Spec",
}
CONFIG_LABELS = {
    "default": "Default pipeline",
    "per_uc_best": r"$\max(U/S)$ pipeline",
}
ABSTRACTION_COLORS = {
    "low": "#355070",
    "tradeoff": "#6d597a",
    "abstract": "#b56576",
}
PHASE_LINE_COLORS = {
    "unspecialized": "#3a3a3a",
    "specialized_exec": "#e56b6f",
}
GROUP_POSITIONS = {
    ("unspecialized", "low"): 0.0,
    ("unspecialized", "tradeoff"): 1.0,
    ("unspecialized", "abstract"): 2.0,
    ("specialized_exec", "low"): 4.0,
    ("specialized_exec", "tradeoff"): 5.0,
    ("specialized_exec", "abstract"): 6.0,
}

UC_DISPLAY_ORDER = (
    "uc1_sql",
    "uc2_conv",
    "uc7_dfa",
    "uc8_ivm",
    "uc12_groupby",
    "uc14_sort",
)

UC_LABELS = {
    "uc1_sql": "UC1",
    "uc2_conv": "UC2",
    "uc7_dfa": "UC3",
    "uc8_ivm": "UC4",
    "uc12_groupby": "UC5",
    "uc14_sort": "UC6",
}


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Generate the thesis RQ3 abstraction-benefit aggregate plot and the "
            "18-kernel exploration grid from the thesis-grade UC rerun."
        )
    )
    parser.add_argument("--db", default=None, metavar="PATH")
    parser.add_argument(
        "--study-name",
        default=DEFAULT_STUDY,
        metavar="STR",
        help=(
            "Study prefix. Matches both the exact study name and any '<study>__*' "
            "shards."
        ),
    )
    parser.add_argument(
        "--config-name",
        default=DEFAULT_CONFIG,
        metavar="STR",
        help="Pipeline configuration to plot (default: per_uc_best).",
    )
    parser.add_argument(
        "--size",
        default=DEFAULT_SIZE,
        metavar="STR",
        help="Benchmark size class to plot (default: MEDIUM).",
    )
    parser.add_argument(
        "--output-dir",
        default="benchmarks/reports/thesis-figures/rq3",
        metavar="DIR",
        help="Directory for PNG, CSV, and provenance outputs.",
    )
    return parser.parse_args()


def _geomean(values: list[float]) -> float:
    return math.exp(sum(math.log(value) for value in values) / len(values))


def _phase_rank(phase: str) -> int:
    return PHASE_ORDER.index(phase)


def _abstraction_rank(abstraction: str) -> int:
    return ABSTRACTION_ORDER.index(abstraction)


def _kernel_sort_key(row: tuple[str, str]) -> tuple[int, str, str]:
    group_name, kernel = row
    try:
        group_rank = UC_DISPLAY_ORDER.index(group_name)
    except ValueError as exc:
        raise RuntimeError(f"Unexpected group_name '{group_name}' in RQ3 abstraction data.") from exc
    return group_rank, kernel, group_name


def _ordered_kernel_rows(df: pd.DataFrame) -> list[tuple[str, str]]:
    unique_rows = df[["group_name", "kernel"]].drop_duplicates()
    ordered_rows = sorted(
        ((str(row.group_name), str(row.kernel)) for row in unique_rows.itertuples(index=False)),
        key=_kernel_sort_key,
    )
    return ordered_rows


def _kernel_display_label(group_name: str, kernel: str) -> str:
    uc_label = UC_LABELS.get(group_name)
    if uc_label is None:
        raise RuntimeError(f"Missing UC label mapping for group '{group_name}'.")
    return f"{uc_label}: {kernel}"


def _load_rows(
    con: duckdb.DuckDBPyConnection,
    study_name: str,
    config_name: str,
    size: str,
) -> pd.DataFrame:
    query = """
        WITH medians AS (
            SELECT
                p."group" AS group_name,
                p.kernel,
                p.kv_a AS abstraction,
                p.phase,
                MEDIAN(
                    p.real_time * CASE p.time_unit
                        WHEN 'ns' THEN 1.0
                        WHEN 'us' THEN 1e3
                        WHEN 'ms' THEN 1e6
                        WHEN 's'  THEN 1e9
                    END
                ) / 1e6 AS runtime_ms,
                COUNT(DISTINCT a.run_id) AS sample_count
            FROM ablation_studies a
            JOIN v_parsed p USING (run_id)
            WHERE (a.study_name = ? OR a.study_name LIKE ?)
              AND a.config_name = ?
              AND p.kv_s = ?
              AND p.kv_a IN ('low', 'tradeoff', 'abstract')
              AND p.phase IN ('unspecialized', 'specialized_exec')
            GROUP BY p."group", p.kernel, p.kv_a, p.phase
        ),
        baselines AS (
            SELECT
                kernel,
                runtime_ms AS baseline_ms
            FROM medians
            WHERE abstraction = 'low'
              AND phase = 'unspecialized'
        )
        SELECT
            m.group_name,
            m.kernel,
            m.abstraction,
            m.phase,
            m.runtime_ms,
            m.sample_count,
            b.baseline_ms,
            m.runtime_ms / b.baseline_ms AS relative_runtime
        FROM medians m
        JOIN baselines b USING (kernel)
        ORDER BY m.group_name, m.kernel, m.phase, m.abstraction
    """
    return con.execute(query, [study_name, f"{study_name}__%", config_name, size]).df()


def _load_provenance(
    con: duckdb.DuckDBPyConnection,
    study_name: str,
    config_name: str,
    size: str,
) -> dict[str, object]:
    study_rows = con.execute(
        """
        SELECT DISTINCT study_name
        FROM ablation_studies
        WHERE (study_name = ? OR study_name LIKE ?)
          AND config_name = ?
        ORDER BY study_name
        """,
        [study_name, f"{study_name}__%", config_name],
    ).fetchall()
    best_practice_rows = con.execute(
        """
        SELECT c.best_practice_full, COUNT(DISTINCT a.run_id) AS run_count
        FROM ablation_studies a
        JOIN context c USING (run_id)
        JOIN v_parsed p USING (run_id)
        WHERE (a.study_name = ? OR a.study_name LIKE ?)
          AND a.config_name = ?
          AND p.kv_s = ?
          AND p.kv_a IN ('low', 'tradeoff', 'abstract')
          AND p.phase IN ('unspecialized', 'specialized_exec')
        GROUP BY c.best_practice_full
        ORDER BY c.best_practice_full
        """,
        [study_name, f"{study_name}__%", config_name, size],
    ).fetchall()
    total_runs = con.execute(
        """
        SELECT COUNT(DISTINCT a.run_id)
        FROM ablation_studies a
        JOIN v_parsed p USING (run_id)
        WHERE (a.study_name = ? OR a.study_name LIKE ?)
          AND a.config_name = ?
          AND p.kv_s = ?
          AND p.kv_a IN ('low', 'tradeoff', 'abstract')
          AND p.phase IN ('unspecialized', 'specialized_exec')
        """,
        [study_name, f"{study_name}__%", config_name, size],
    ).fetchone()
    return {
        "matched_studies": [row[0] for row in study_rows],
        "best_practice_rows": [(row[0], int(row[1])) for row in best_practice_rows],
        "run_count": int(total_runs[0]) if total_runs and total_runs[0] is not None else 0,
    }


def _validate_rows(df: pd.DataFrame) -> None:
    if df.empty:
        raise RuntimeError("No RQ3 abstraction rows found for the requested slice.")

    kernels = [_kernel_display_label(group_name, kernel) for group_name, kernel in _ordered_kernel_rows(df)]
    if len(kernels) != EXPECTED_KERNEL_COUNT:
        raise RuntimeError(
            f"Expected {EXPECTED_KERNEL_COUNT} kernels, found {len(kernels)}: {kernels}"
        )

    expected_rows = EXPECTED_KERNEL_COUNT * len(ABSTRACTION_ORDER) * len(PHASE_ORDER)
    if len(df) != expected_rows:
        raise RuntimeError(f"Expected {expected_rows} rows, found {len(df)}.")

    baseline_rows = df[
        (df["abstraction"] == "low") & (df["phase"] == "unspecialized")
    ].copy()
    if len(baseline_rows) != EXPECTED_KERNEL_COUNT:
        raise RuntimeError("Missing one or more low-unspecialized baseline rows.")

    for kernel, kernel_rows in df.groupby("kernel", observed=False):
        phases = set(kernel_rows["phase"])
        abstractions = set(kernel_rows["abstraction"])
        if phases != set(PHASE_ORDER):
            raise RuntimeError(f"Phase coverage mismatch for kernel '{kernel}'.")
        if abstractions != set(ABSTRACTION_ORDER):
            raise RuntimeError(f"Abstraction coverage mismatch for kernel '{kernel}'.")
        counts = kernel_rows.groupby(["phase", "abstraction"], observed=False).size()
        if any(count != 1 for count in counts):
            raise RuntimeError(f"Duplicate or missing phase/abstraction rows for '{kernel}'.")

    if not np.allclose(baseline_rows["relative_runtime"], 1.0):
        raise RuntimeError("Baseline rows must normalize to exactly 1.0.")

    observed_groups = set(df["group_name"].unique())
    if observed_groups != set(UC_DISPLAY_ORDER):
        raise RuntimeError(
            f"Expected UC groups {UC_DISPLAY_ORDER}, found {sorted(observed_groups)}."
        )


def _add_category_labels(ax: plt.Axes) -> None:
    ax.text(1.0, -0.16, "Unspec", transform=ax.get_xaxis_transform(), ha="center", va="top")
    ax.text(5.0, -0.16, "Spec", transform=ax.get_xaxis_transform(), ha="center", va="top")
    ax.axvline(3.0, color="#c7c7c7", linewidth=1.0, linestyle=":")


def _plot_aggregate(df: pd.DataFrame, output: Path, config_name: str, size: str) -> None:
    fig, ax = plt.subplots(figsize=(11, 5.8), constrained_layout=True)
    rng = np.random.default_rng(0)

    for phase in PHASE_ORDER:
        phase_rows = df[df["phase"] == phase]
        for kernel, kernel_rows in phase_rows.groupby("kernel", sort=True, observed=False):
            ordered = kernel_rows.sort_values(
                by="abstraction",
                key=lambda col: col.map({name: idx for idx, name in enumerate(ABSTRACTION_ORDER)}),
            )
            xs = [GROUP_POSITIONS[(phase, abstraction)] for abstraction in ordered["abstraction"]]
            ax.plot(
                xs,
                ordered["relative_runtime"],
                color=PHASE_LINE_COLORS[phase],
                linewidth=1.0,
                alpha=0.25,
                zorder=1,
            )

    box_data: list[list[float]] = []
    positions: list[float] = []
    colors: list[str] = []
    for phase in PHASE_ORDER:
        for abstraction in ABSTRACTION_ORDER:
            series = df[
                (df["phase"] == phase) & (df["abstraction"] == abstraction)
            ]["relative_runtime"].tolist()
            box_data.append(series)
            positions.append(GROUP_POSITIONS[(phase, abstraction)])
            colors.append(ABSTRACTION_COLORS[abstraction])

    boxplot = ax.boxplot(
        box_data,
        positions=positions,
        widths=0.56,
        patch_artist=True,
        showfliers=False,
        medianprops={"color": "white", "linewidth": 1.8},
        whiskerprops={"color": "#444444", "linewidth": 1.1},
        capprops={"color": "#444444", "linewidth": 1.1},
        boxprops={"edgecolor": "#222222", "linewidth": 1.1},
    )
    for patch, color in zip(boxplot["boxes"], colors):
        patch.set_facecolor(color)
        patch.set_alpha(0.9)

    for phase in PHASE_ORDER:
        for abstraction in ABSTRACTION_ORDER:
            group_rows = df[(df["phase"] == phase) & (df["abstraction"] == abstraction)].copy()
            jitter = rng.uniform(-0.11, 0.11, size=len(group_rows))
            xpos = np.full(len(group_rows), GROUP_POSITIONS[(phase, abstraction)]) + jitter
            ax.scatter(
                xpos,
                group_rows["relative_runtime"],
                s=28,
                color=ABSTRACTION_COLORS[abstraction],
                edgecolors="white",
                linewidths=0.5,
                alpha=0.95,
                zorder=3,
            )

            geomean = _geomean(group_rows["relative_runtime"].tolist())
            ax.scatter(
                GROUP_POSITIONS[(phase, abstraction)],
                geomean,
                marker="D",
                s=64,
                color="#f4a261",
                edgecolors="#222222",
                linewidths=0.9,
                zorder=4,
            )

    ax.axhline(1.0, color="black", linestyle="--", linewidth=1.0)
    ax.grid(axis="y", color="#dddddd", linewidth=0.8)
    ax.set_axisbelow(True)
    ax.set_xticks(
        [GROUP_POSITIONS[(phase, abstraction)] for phase in PHASE_ORDER for abstraction in ABSTRACTION_ORDER],
        [ABSTRACTION_LABELS[abstraction] for _phase in PHASE_ORDER for abstraction in ABSTRACTION_ORDER],
    )
    _add_category_labels(ax)
    ax.set_xlim(-0.75, 6.75)
    ax.set_ylabel("Runtime relative to low-unspec")
    legend_handles = [
        Line2D([0], [0], color=PHASE_LINE_COLORS["unspecialized"], linewidth=1.2, label="Kernel path (unspec)"),
        Line2D([0], [0], color=PHASE_LINE_COLORS["specialized_exec"], linewidth=1.2, label="Kernel path (spec)"),
        Patch(facecolor=ABSTRACTION_COLORS["low"], edgecolor="#222222", label="Boxplot summary"),
        Line2D(
            [0],
            [0],
            marker="D",
            color="none",
            markerfacecolor="#f4a261",
            markeredgecolor="#222222",
            markersize=7,
            label="Geomean",
        ),
    ]
    ax.legend(handles=legend_handles, loc="upper left", ncol=2, frameon=True)

    save_plot(fig, output, dpi=220)
    plt.close(fig)


def _plot_kernel_grid(df: pd.DataFrame, output: Path, config_name: str, size: str) -> None:
    ordered_kernels = _ordered_kernel_rows(df)
    y_max = max(2.2, float(df["relative_runtime"].max()) * 1.08)
    fig, axes = plt.subplots(3, 6, figsize=(20, 10.8), sharey=True, constrained_layout=True)
    axes_flat = axes.flatten()

    for ax, (group_name, kernel) in zip(axes_flat, ordered_kernels):
        kernel_rows = df[(df["group_name"] == group_name) & (df["kernel"] == kernel)]
        for phase in PHASE_ORDER:
            phase_rows = kernel_rows[kernel_rows["phase"] == phase].sort_values(
                by="abstraction",
                key=lambda col: col.map({name: idx for idx, name in enumerate(ABSTRACTION_ORDER)}),
            )
            xs = [GROUP_POSITIONS[(phase, abstraction)] for abstraction in phase_rows["abstraction"]]
            ax.plot(
                xs,
                phase_rows["relative_runtime"],
                color=PHASE_LINE_COLORS[phase],
                linewidth=1.3,
                alpha=0.95,
                zorder=2,
            )
            ax.scatter(
                xs,
                phase_rows["relative_runtime"],
                s=38,
                color=[ABSTRACTION_COLORS[abstraction] for abstraction in phase_rows["abstraction"]],
                edgecolors="white",
                linewidths=0.6,
                zorder=3,
            )

        ax.axhline(1.0, color="black", linestyle="--", linewidth=0.9)
        ax.axvline(3.0, color="#d0d0d0", linewidth=0.9, linestyle=":")
        ax.set_xlim(-0.45, 6.45)
        ax.set_ylim(0.0, y_max)
        ax.set_xticks(
            [GROUP_POSITIONS[(phase, abstraction)] for phase in PHASE_ORDER for abstraction in ABSTRACTION_ORDER],
            ["L", "T", "A", "L", "T", "A"],
        )
        ax.tick_params(axis="x", labelsize=9)
        ax.tick_params(axis="y", labelsize=9)
        ax.set_title(_kernel_display_label(group_name, kernel), fontsize=11)
        ax.text(1.0, 0.98, "U", transform=ax.get_xaxis_transform(), ha="center", va="top", fontsize=9)
        ax.text(5.0, 0.98, "S", transform=ax.get_xaxis_transform(), ha="center", va="top", fontsize=9)

    for ax in axes[:, 0]:
        ax.set_ylabel("relative runtime")
    for ax in axes_flat[len(ordered_kernels):]:
        ax.remove()

    save_plot(fig, output, dpi=220)
    plt.close(fig)


def _write_provenance(
    df: pd.DataFrame,
    output: Path,
    study_name: str,
    config_name: str,
    size: str,
    provenance: dict[str, object],
) -> None:
    matched_studies = provenance["matched_studies"]
    best_practice_rows = provenance["best_practice_rows"]
    kernels = sorted(df["kernel"].unique())
    lines = [
        "# RQ3 Abstraction Benefit Plot Provenance",
        "",
        "- Source database: `benchmarks/benchmarks.duckdb`",
        f"- Study selector: exact `{study_name}` plus any `{study_name}__*` shards",
        f"- Matched study names: {', '.join(f'`{name}`' for name in matched_studies) if matched_studies else '(none)'}",
        f"- Config slice: `{config_name}`",
        f"- Size slice: `{size}`",
        "- Abstraction levels: `low`, `tradeoff`, `abstract`",
        "- Phases: `unspecialized`, `specialized_exec`",
        f"- Kernel count: {len(kernels)}",
        f"- Distinct run count for this slice: {provenance['run_count']}",
        "- Aggregation: median `real_time` per `(kernel, abstraction, phase)` across the matched runs",
        "- Normalization: each point is divided by the same kernel's `low` + `unspecialized` median runtime",
        "",
        "## Best-Practice Status",
        "",
    ]
    if best_practice_rows:
        for best_practice_full, run_count in best_practice_rows:
            lines.append(f"- `best_practice_full={best_practice_full}`: {run_count} runs")
    else:
        lines.append("- No matching context rows found.")
    lines.extend(
        [
            "",
            "## Files",
            "",
            "- `rq3_abstraction_benefit.csv`: normalized per-kernel plotting data",
            "- `rq3_abstraction_benefit_aggregate.png`: thesis aggregate plot with points, boxplots, and geomeans",
            "- `rq3_abstraction_benefit_per_kernel.png`: 18-kernel exploration grid",
            "",
        ]
    )
    output.write_text("\n".join(lines), encoding="utf-8")


def main() -> None:
    args = _parse_args()
    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    db_path = resolve_db_path(args.db)
    con = open_db(db_path)
    try:
        df = _load_rows(con, args.study_name, args.config_name, args.size)
        provenance = _load_provenance(con, args.study_name, args.config_name, args.size)
    finally:
        con.close()

    _validate_rows(df)

    df = df.copy()
    df["x_position"] = df.apply(
        lambda row: GROUP_POSITIONS[(row["phase"], row["abstraction"])], axis=1
    )
    group_rank_map = {name: idx for idx, name in enumerate(UC_DISPLAY_ORDER)}
    df = df.sort_values(
        by=["group_name", "kernel", "phase", "abstraction"],
        key=lambda col: col.map(group_rank_map)
        if col.name == "group_name"
        else col.map({name: idx for idx, name in enumerate(PHASE_ORDER)})
        if col.name == "phase"
        else col.map({name: idx for idx, name in enumerate(ABSTRACTION_ORDER)})
        if col.name == "abstraction"
        else col,
    )

    csv_path = output_dir / "rq3_abstraction_benefit.csv"
    aggregate_path = output_dir / "rq3_abstraction_benefit_aggregate.png"
    kernel_grid_path = output_dir / "rq3_abstraction_benefit_per_kernel.png"
    provenance_path = output_dir / "rq3_abstraction_benefit_provenance.md"

    save_csv(df, csv_path)
    _plot_aggregate(df, aggregate_path, args.config_name, args.size)
    _plot_kernel_grid(df, kernel_grid_path, args.config_name, args.size)
    _write_provenance(df, provenance_path, args.study_name, args.config_name, args.size, provenance)

    print(f"Study: {args.study_name}")
    print(f"Config: {args.config_name}")
    print(f"Size: {args.size}")
    print(f"Provenance: {provenance_path}")


if __name__ == "__main__":
    main()
