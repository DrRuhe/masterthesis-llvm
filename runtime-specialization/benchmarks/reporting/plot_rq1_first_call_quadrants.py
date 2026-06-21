#!/usr/bin/env python3
"""Render the RQ1 first-call quadrant scatterplot for the UC thesis rerun."""

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
from matplotlib.ticker import FuncFormatter, LogLocator
import pandas as pd

from report_utils import open_db, resolve_db_path, save_csv, save_plot


CONFIGS = ("default", "per_uc_best")
CONFIG_LABELS = {
    "default": "Default pipeline",
    "per_uc_best": "max($U/S$) pipeline",
}
ABSTRACTION_ORDER = ("low", "abstract", "tradeoff")
ABSTRACTION_COLORS = {
    "low": "#355070",
    "abstract": "#b56576",
    "tradeoff": "#6d597a",
}
SIZE_ORDER = ("SMALL", "MEDIUM", "LARGE", "EXTRALARGE")
SIZE_MARKERS = {
    "SMALL": "o",
    "MEDIUM": "s",
    "LARGE": "^",
    "EXTRALARGE": "D",
}
SIZE_RANK = {name: idx for idx, name in enumerate(SIZE_ORDER)}


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Plot U/U_p vs U/(J+S) scatterplots for the UC thesis rerun across "
            "all sizes and abstraction levels."
        )
    )
    parser.add_argument("--db", default=None, metavar="PATH")
    parser.add_argument("--study-name", required=True, metavar="STR")
    parser.add_argument(
        "--output-dir",
        default="benchmarks/reports/thesis-figures/rq1",
        metavar="DIR",
        help="Directory for PNG and CSV outputs.",
    )
    return parser.parse_args()


def _load_rows(con: duckdb.DuckDBPyConnection, study_name: str) -> pd.DataFrame:
    query = """
        WITH phase_rows AS (
            SELECT
                a.study_name,
                a.config_name,
                p."group",
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
              AND a.config_name IN ('default', 'per_uc_best')
              AND p."group" IN ('uc1_sql', 'uc2_conv', 'uc7_dfa', 'uc8_ivm', 'uc12_groupby', 'uc14_sort')
              AND p.kv_a IN ('low', 'abstract', 'tradeoff')
              AND p.kv_s IN ('SMALL', 'MEDIUM', 'LARGE', 'EXTRALARGE')
              AND p.phase IN ('jit_overhead', 'specialized_exec', 'unspecialized')
        ),
        medians AS (
            SELECT
                study_name,
                config_name,
                "group",
                kernel,
                abstraction,
                size,
                MEDIAN(CASE WHEN phase = 'jit_overhead' THEN phase_ns END) / 1e6 AS jit_ms,
                MEDIAN(CASE WHEN phase = 'specialized_exec' THEN phase_ns END) / 1e6 AS spec_ms,
                MEDIAN(CASE WHEN phase = 'unspecialized' THEN phase_ns END) / 1e6 AS unspec_ms
            FROM phase_rows
            GROUP BY study_name, config_name, "group", kernel, abstraction, size
        ),
        medium_thresholds AS (
            SELECT
                config_name,
                "group",
                kernel,
                abstraction,
                unspec_ms / spec_ms AS medium_exec_speedup,
                CASE
                    WHEN unspec_ms / spec_ms > 1.0
                    THEN ((unspec_ms / spec_ms) * jit_ms) / ((unspec_ms / spec_ms) - 1.0)
                    ELSE NULL
                END AS up_ms
            FROM medians
            WHERE size = 'MEDIUM'
        ),
        enriched AS (
            SELECT
                m.study_name,
                m.config_name,
                m."group",
                m.kernel,
                m.abstraction,
                m.size,
                m.jit_ms,
                m.spec_ms,
                m.unspec_ms,
                t.medium_exec_speedup,
                t.up_ms,
                m.unspec_ms / (m.jit_ms + m.spec_ms) AS first_call_speedup,
                CASE
                    WHEN t.up_ms IS NOT NULL AND t.up_ms > 0.0
                    THEN m.unspec_ms / t.up_ms
                    ELSE NULL
                END AS u_over_up
            FROM medians m
            JOIN medium_thresholds t
              ON t.config_name = m.config_name
             AND t."group" = m."group"
             AND t.kernel = m.kernel
             AND t.abstraction = m.abstraction
        )
        SELECT
            study_name,
            config_name,
            "group",
            kernel,
            abstraction,
            size,
            jit_ms,
            spec_ms,
            unspec_ms,
            medium_exec_speedup,
            up_ms,
            first_call_speedup,
            u_over_up,
            CASE
                WHEN u_over_up IS NULL OR NOT isfinite(u_over_up) OR u_over_up <= 0.0
                THEN 'undefined'
                WHEN u_over_up < 1.0 AND first_call_speedup < 1.0
                THEN 'bottom_left'
                WHEN u_over_up >= 1.0 AND first_call_speedup >= 1.0
                THEN 'top_right'
                WHEN u_over_up < 1.0 AND first_call_speedup >= 1.0
                THEN 'top_left'
                ELSE 'bottom_right'
            END AS quadrant,
            CASE
                WHEN
                    CASE
                        WHEN u_over_up IS NULL OR NOT isfinite(u_over_up) OR u_over_up <= 0.0
                        THEN 'undefined'
                        WHEN u_over_up < 1.0 AND first_call_speedup < 1.0
                        THEN 'bottom_left'
                        WHEN u_over_up >= 1.0 AND first_call_speedup >= 1.0
                        THEN 'top_right'
                        WHEN u_over_up < 1.0 AND first_call_speedup >= 1.0
                        THEN 'top_left'
                        ELSE 'bottom_right'
                    END IN ('bottom_left', 'top_right')
                THEN TRUE
                ELSE FALSE
            END AS matches_prediction,
            kernel || ' [' || abstraction || ', ' || size || ']' AS label
        FROM enriched
        ORDER BY config_name, "group", kernel, abstraction, size
    """
    return con.execute(query, [study_name, f"{study_name}__%"]).df()


def _plot_points(ax, sub: pd.DataFrame) -> pd.DataFrame:
    defined = sub[sub["quadrant"] != "undefined"].copy()
    if defined.empty:
        raise RuntimeError("No plottable rows found.")

    for abstraction in ABSTRACTION_ORDER:
        series_rows = (
            defined[defined["abstraction"] == abstraction]
            .sort_values(["group", "kernel", "abstraction", "size"], key=lambda col: col.map(SIZE_RANK) if col.name == "size" else col)
        )
        for (_, kernel_rows) in series_rows.groupby(
            ["group", "kernel", "abstraction"], sort=False, observed=False
        ):
            if len(kernel_rows) < 2:
                continue
            ax.plot(
                kernel_rows["u_over_up"],
                kernel_rows["first_call_speedup"],
                color=ABSTRACTION_COLORS[abstraction],
                linewidth=0.8,
                alpha=0.22,
                zorder=2,
            )

    for abstraction in ABSTRACTION_ORDER:
        for size in SIZE_ORDER:
            mask = (defined["abstraction"] == abstraction) & (defined["size"] == size)
            points = defined[mask]
            if points.empty:
                continue
            ax.scatter(
                points["u_over_up"],
                points["first_call_speedup"],
                s=70,
                marker=SIZE_MARKERS[size],
                color=ABSTRACTION_COLORS[abstraction],
                edgecolors="white",
                linewidths=0.7,
                alpha=0.9,
                zorder=3,
            )

    ax.axvline(1.0, color="black", linestyle="--", linewidth=1.0)
    ax.axhline(1.0, color="black", linestyle="--", linewidth=1.0)
    ax.set_xscale("log")
    ax.xaxis.set_major_locator(LogLocator(base=10.0))
    ax.grid(True, which="both", axis="both", color="#dddddd", linewidth=0.8)
    ax.set_axisbelow(True)
    return defined


def _format_decimal_log_tick(value: float, _: float) -> str:
    if value <= 0:
        return ""
    exponent = round(math.log10(value))
    if not math.isclose(value, 10 ** exponent, rel_tol=1e-9, abs_tol=0.0):
        return ""
    if -2 <= exponent <= 3:
        if exponent < 0:
            return f"{value:.{-exponent}f}"
        return str(int(value))
    return rf"$10^{{{exponent}}}$"


def _add_legends(ax) -> None:
    abstraction_handles = [
        Line2D(
            [0],
            [0],
            marker="o",
            color="none",
            markerfacecolor=ABSTRACTION_COLORS[name],
            markeredgecolor=ABSTRACTION_COLORS[name],
            markersize=8,
            linestyle="None",
            label=name,
        )
        for name in ABSTRACTION_ORDER
    ]
    size_handles = [
        Line2D(
            [0],
            [0],
            marker=SIZE_MARKERS[name],
            color="#444444",
            markerfacecolor="#444444",
            markersize=8,
            linestyle="None",
            label=name,
        )
        for name in SIZE_ORDER
    ]
    legend1 = ax.legend(handles=abstraction_handles, title="Abstraction", loc="upper left")
    ax.add_artist(legend1)
    ax.legend(handles=size_handles, title="Size", loc="lower right")


def _plot_combined(df: pd.DataFrame, output_path: Path) -> None:
    fig, axes = plt.subplots(
        1,
        len(CONFIGS),
        figsize=(13.2, 5.8),
        sharex=True,
        sharey=True,
        constrained_layout=True,
    )

    for ax, config_name in zip(axes, CONFIGS):
        sub = df[df["config_name"] == config_name].copy()
        _plot_points(ax, sub)
        ax.set_title(CONFIG_LABELS[config_name])
        ax.xaxis.set_major_formatter(FuncFormatter(_format_decimal_log_tick))
        ax.set_xlabel(r"problem size measured by $U/U_p$")

    axes[0].set_ylabel("first-call speedup ($U/ (J+S)$))")
    _add_legends(axes[0])
    save_plot(fig, output_path, dpi=220)
    plt.close(fig)


def main() -> None:
    args = _parse_args()
    db_path = resolve_db_path(args.db)
    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    con = open_db(db_path)
    try:
        rows = _load_rows(con, args.study_name)
    finally:
        con.close()

    if rows.empty:
        raise RuntimeError(f"No data found for study '{args.study_name}'.")

    frame = rows.copy()
    frame["size"] = pd.Categorical(frame["size"], categories=SIZE_ORDER, ordered=True)
    frame["abstraction"] = pd.Categorical(
        frame["abstraction"], categories=ABSTRACTION_ORDER, ordered=True
    )
    frame = frame.sort_values(["config_name", "group", "kernel", "abstraction", "size"]).reset_index(drop=True)
    csv_out = output_dir / "rq1_first_call_quadrants.csv"
    csv_frame = frame.copy()
    csv_frame["size"] = csv_frame["size"].astype(str)
    csv_frame["abstraction"] = csv_frame["abstraction"].astype(str)
    save_csv(csv_frame, csv_out)
    combined_out = output_dir / "rq1_first_call_quadrants.png"
    _plot_combined(frame, combined_out)

    expected_configs = set(CONFIGS)
    actual_configs = set(frame["config_name"].unique())
    if actual_configs != expected_configs:
        raise RuntimeError(
            f"Expected configs {sorted(expected_configs)}, got {sorted(actual_configs)}."
        )

    print(f"Study: {args.study_name}")
    print(f"CSV: {csv_out}")
    print(f"Plot: {combined_out}")
    for config_name in CONFIGS:
        sub = frame[frame["config_name"] == config_name]
        counts = sub["quadrant"].value_counts().to_dict()
        total = len(sub)
        matches = int(sub["matches_prediction"].sum())
        undefined = int(counts.get("undefined", 0))
        print(
            f"{config_name}: {matches}/{total - undefined} predicted rows in bottom-left or top-right "
            f"(undefined={undefined}, disagreements={counts.get('top_left', 0) + counts.get('bottom_right', 0)})"
        )


if __name__ == "__main__":
    main()
