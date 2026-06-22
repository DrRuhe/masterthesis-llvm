#!/usr/bin/env python3
"""Generate thesis size-scaling plots for amortized speedup and JIT overhead."""

from __future__ import annotations

import argparse
import math
import os
import sys
from pathlib import Path

import matplotlib.pyplot as plt
import matplotlib.ticker as mticker
import pandas as pd

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import size_scaling as ss
from report_utils import open_db, resolve_db_path, resolve_run_id, save_plot


DEFAULT_CSV = "benchmarks/reports/thesis-figures/rq1/rq1_first_call_quadrants.csv"
DEFAULT_OUTPUT_DIR = "benchmarks/reports/thesis-figures/rq1"
DEFAULT_OUTPUT_PREFIX = "rq1_size_scaling"
SIZE_ORDER = ["SMALL", "MEDIUM", "LARGE", "EXTRALARGE"]


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate thesis size-scaling figures for amortized speedup and JIT overhead."
    )
    parser.add_argument("--db", default=None, metavar="PATH")
    parser.add_argument("--run-id", default=None, metavar="ID")
    parser.add_argument("--csv", default=DEFAULT_CSV, metavar="PATH")
    parser.add_argument("--filter", default=None, metavar="REGEX")
    parser.add_argument("--config", default="default", metavar="NAME")
    parser.add_argument("--abstraction", default="low", metavar="NAME")
    parser.add_argument(
        "--output-dir",
        default=DEFAULT_OUTPUT_DIR,
        metavar="PATH",
        help="Directory for the generated PNG files.",
    )
    parser.add_argument(
        "--output-prefix",
        default=DEFAULT_OUTPUT_PREFIX,
        metavar="NAME",
        help="Prefix for the generated PNG files inside --output-dir.",
    )
    parser.add_argument(
        "--skip-legacy-combined",
        action="store_true",
        help="Do not write the legacy combined two-panel figure.",
    )
    return parser.parse_args()


def _size_rank(label: str) -> int:
    try:
        return SIZE_ORDER.index(label) + 1
    except ValueError as exc:
        raise RuntimeError(f"Unsupported size label '{label}'.") from exc


def _load_first_call_csv(path: Path, args: argparse.Namespace) -> pd.DataFrame:
    df = pd.read_csv(path)
    required = {
        "config_name",
        "kernel",
        "abstraction",
        "size",
        "jit_ms",
        "spec_ms",
        "unspec_ms",
    }
    if not required.issubset(df.columns):
        raise RuntimeError(f"{path} is not a first-call quadrants export.")

    df = df.copy()
    df = df[df["config_name"] == args.config]
    df = df[df["abstraction"] == args.abstraction]
    if args.filter:
        df = df[df["kernel"].str.contains(args.filter, case=False, regex=True)]
    if df.empty:
        return df

    df["size_label"] = df["size"].astype(str)
    df["size_rank"] = df["size_label"].map(_size_rank)
    df["display_name"] = df["kernel"]
    df["amortized_speedup"] = df["unspec_ms"] / df["spec_ms"]
    df["jit_overhead_ms"] = df["jit_ms"]
    return df[
        [
            "kernel",
            "display_name",
            "size_label",
            "size_rank",
            "amortized_speedup",
            "jit_overhead_ms",
        ]
    ].sort_values(["display_name", "size_rank"])


def _load_legacy_size_scaling_csv(path: Path, args: argparse.Namespace) -> pd.DataFrame:
    df = pd.read_csv(path)
    required = {"kernel", "display_name", "size_label", "size_rank", "speedup", "t_jit_ns"}
    if not required.issubset(df.columns):
        raise RuntimeError(f"{path} is not a legacy size-scaling export.")

    df = df.copy()
    if args.filter:
        df = df[df["kernel"].str.contains(args.filter, case=False, regex=True)]
    if df.empty:
        return df

    df["amortized_speedup"] = df["speedup"]
    df["jit_overhead_ms"] = df["t_jit_ns"] / 1e6
    return df[
        [
            "kernel",
            "display_name",
            "size_label",
            "size_rank",
            "amortized_speedup",
            "jit_overhead_ms",
        ]
    ].sort_values(["display_name", "size_rank"])


def _load_from_db(args: argparse.Namespace) -> pd.DataFrame:
    con = open_db(resolve_db_path(args.db))
    try:
        run_id = resolve_run_id(con, args.run_id)
        df = ss.load_data(con, run_id, kernel_filter=args.filter, group_filter=None)
    finally:
        con.close()
    if df.empty:
        return df

    df = df.copy()
    df["amortized_speedup"] = df["speedup"]
    df["jit_overhead_ms"] = df["t_jit_ns"] / 1e6
    return df[
        [
            "kernel",
            "display_name",
            "size_label",
            "size_rank",
            "amortized_speedup",
            "jit_overhead_ms",
        ]
    ].sort_values(["display_name", "size_rank"])


def _load_data(args: argparse.Namespace) -> pd.DataFrame:
    if args.csv:
        path = Path(args.csv)
        try:
            return _load_first_call_csv(path, args)
        except RuntimeError:
            return _load_legacy_size_scaling_csv(path, args)
    return _load_from_db(args)


def _format_tick(value: float) -> str:
    if value >= 10 or float(value).is_integer():
        return f"{value:.0f}"
    if value >= 1:
        return f"{value:.2f}".rstrip("0").rstrip(".")
    return f"{value:.3f}".rstrip("0").rstrip(".")


def _candidate_ticks(metric: str) -> list[float]:
    if metric == "jit_overhead_ms":
        return [20.0, 30.0, 50.0, 75.0, 100.0, 200.0, 500.0, 1000.0, 2000.0, 3000.0]
    return [
        0.05,
        0.1,
        0.2,
        0.3,
        0.5,
        0.75,
        1.0,
        1.25,
        1.5,
        2.0,
        2.5,
        3.0,
        4.0,
        5.0,
        7.5,
        10.0,
    ]


def _log_ticks(values: pd.Series, metric: str) -> list[float]:
    candidates = _candidate_ticks(metric)
    lower = values.min() * 0.92
    upper = values.max() * 1.08
    ticks = [tick for tick in candidates if lower <= tick <= upper]
    if 1.0 not in ticks and lower <= 1.0 <= upper:
        ticks.append(1.0)
    return sorted(set(ticks))


def _axes_fraction_from_y(ax, y: float) -> float:
    _, pixel_y = ax.transData.transform((0.0, y))
    _, axes_y = ax.transAxes.inverted().transform((0.0, pixel_y))
    return axes_y


def _data_y_from_axes_fraction(ax, y_fraction: float) -> float:
    _, pixel_y = ax.transAxes.transform((0.0, y_fraction))
    _, data_y = ax.transData.inverted().transform((0.0, pixel_y))
    return data_y


def _distribute_label_positions(ax, labels: list[dict[str, object]]) -> list[float]:
    if not labels:
        return []

    labels = sorted(labels, key=lambda item: _axes_fraction_from_y(ax, float(item["y"])))
    min_gap = 0.032
    low, high = 0.04, 0.96
    adjusted: list[float] = []

    for index, item in enumerate(labels):
        pos = max(low, min(high, _axes_fraction_from_y(ax, float(item["y"]))))
        if index:
            pos = max(pos, adjusted[-1] + min_gap)
        adjusted.append(pos)

    overflow = adjusted[-1] - high
    if overflow > 0:
        adjusted = [max(low, pos - overflow) for pos in adjusted]

    return adjusted


def _annotate_line_labels(ax, labels: list[dict[str, object]]) -> None:
    if not labels:
        return

    by_side = {
        "left": [item for item in labels if item.get("side") == "left"],
        "right": [item for item in labels if item.get("side", "right") == "right"],
    }

    for side, items in by_side.items():
        ordered = sorted(items, key=lambda item: _axes_fraction_from_y(ax, float(item["y"])))
        for item, label_pos in zip(ordered, _distribute_label_positions(ax, ordered)):
            x_end = float(item["x"])
            y_end = float(item["y"])
            y_label = _data_y_from_axes_fraction(ax, label_pos)
            color = item["color"]
            label = str(item["label"])

            if side == "left":
                ax.plot([x_end, x_end - 0.12], [y_end, y_label], color=color, linewidth=0.8, alpha=0.8)
                ax.text(
                    x_end - 0.16,
                    y_label,
                    label,
                    color=color,
                    fontsize=7,
                    va="center",
                    ha="right",
                )
                continue

            ax.plot([x_end, x_end + 0.12], [y_end, y_label], color=color, linewidth=0.8, alpha=0.8)
            ax.text(
                x_end + 0.16,
                y_label,
                label,
                color=color,
                fontsize=7,
                va="center",
                ha="left",
            )


def _label_anchor(kdf: pd.DataFrame, metric: str) -> dict[str, object]:
    if metric == "jit_overhead_ms":
        peak = kdf.loc[kdf[metric].idxmax()]
        end = kdf.iloc[-1]
        if float(peak[metric]) > 1.5 * float(end[metric]) and int(peak["size_rank"]) != int(end["size_rank"]):
            return {
                "x": float(peak["size_rank"]),
                "y": float(peak[metric]),
                "side": "left" if int(peak["size_rank"]) == 1 else "right",
            }
    end = kdf.iloc[-1]
    return {"x": float(end["size_rank"]), "y": float(end[metric]), "side": "right"}


def _plot_metric(
    ax,
    df: pd.DataFrame,
    styles: dict[str, dict[str, object]],
    metric: str,
    ylabel: str,
    *,
    log_scale: bool,
    labeled: bool,
) -> None:
    label_points: list[dict[str, object]] = []
    has_left_labels = False
    has_right_labels = False
    for kernel in sorted(df["display_name"].unique()):
        kdf = df[df["display_name"] == kernel].sort_values("size_rank")
        style = styles[kernel]
        xs = kdf["size_rank"].to_numpy()
        ys = kdf[metric].to_numpy()
        ax.plot(
            xs,
            ys,
            marker=style["marker"],
            linestyle=style["linestyle"],
            color=style["color"],
            linewidth=1.6,
            markersize=5,
            alpha=0.95,
        )
        if labeled:
            anchor = _label_anchor(kdf, metric)
            has_left_labels = has_left_labels or anchor["side"] == "left"
            has_right_labels = has_right_labels or anchor["side"] == "right"
            label_points.append(
                {
                    "x": anchor["x"],
                    "y": anchor["y"],
                    "label": kernel,
                    "color": style["color"],
                    "side": anchor["side"],
                }
            )

    if log_scale:
        ax.set_yscale("log")
        ticks = _log_ticks(df[metric], metric)
        if ticks:
            ax.yaxis.set_major_locator(mticker.FixedLocator(ticks))
            ax.yaxis.set_major_formatter(mticker.FixedFormatter([_format_tick(tick) for tick in ticks]))
        if metric == "amortized_speedup":
            ax.axhline(1.0, color="0.45", linestyle=":", linewidth=1.0, alpha=0.9)
    else:
        ax.yaxis.set_major_locator(mticker.MaxNLocator(nbins=8, min_n_ticks=6))
        ax.yaxis.set_major_formatter(mticker.FuncFormatter(lambda value, _: _format_tick(value)))

    tick_labels = {index + 1: label for index, label in enumerate(SIZE_ORDER)}
    ss._set_size_xticks(ax, tick_labels)
    ax.set_xlabel("input size")
    ax.set_ylabel(ylabel)
    ax.grid(axis="y", which="major", color="0.85", linewidth=0.8)
    ax.grid(axis="x", which="major", color="0.93", linewidth=0.5)

    left_margin = 0.35 if labeled and has_left_labels else 0.85
    right_margin = len(SIZE_ORDER) + (0.9 if labeled and has_right_labels else 0.15)
    ax.set_xlim(left_margin, right_margin)
    if labeled:
        _annotate_line_labels(ax, label_points)


def _save_metric_plot(
    df: pd.DataFrame,
    styles: dict[str, dict[str, object]],
    metric: str,
    ylabel: str,
    output: Path,
    *,
    log_scale: bool,
    labeled: bool,
) -> None:
    width = 11.5 if labeled else 8.6
    fig, ax = plt.subplots(figsize=(width, 4.8))
    _plot_metric(ax, df, styles, metric, ylabel, log_scale=log_scale, labeled=labeled)
    fig.tight_layout()
    save_plot(fig, output)
    plt.close(fig)


def _save_legacy_combined(
    df: pd.DataFrame,
    styles: dict[str, dict[str, object]],
    output: Path,
) -> None:
    fig, axes = plt.subplots(ncols=2, figsize=(13.2, 4.8))
    _plot_metric(
        axes[0],
        df,
        styles,
        "amortized_speedup",
        r"amortized speedup ($U/S$)",
        log_scale=True,
        labeled=False,
    )
    _plot_metric(
        axes[1],
        df,
        styles,
        "jit_overhead_ms",
        r"JIT overhead in ms ($J$)",
        log_scale=True,
        labeled=False,
    )
    fig.tight_layout()
    save_plot(fig, output)
    plt.close(fig)


def main() -> None:
    args = _parse_args()
    df = _load_data(args)
    if df.empty:
        source = args.csv if args.csv else args.run_id
        raise RuntimeError(f"No size-scaling data found for selection '{source}'.")

    kernels = sorted(df["display_name"].unique())
    styles = ss._kernel_styles(kernels)

    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    prefix = args.output_prefix

    outputs = {
        "speedup": output_dir / f"{prefix}_speedup.png",
        "speedup_labeled": output_dir / f"{prefix}_speedup_labeled.png",
        "jit": output_dir / f"{prefix}_jit_overhead.png",
        "jit_labeled": output_dir / f"{prefix}_jit_overhead_labeled.png",
        "legacy": output_dir / f"{prefix}.png",
    }

    _save_metric_plot(
        df,
        styles,
        "amortized_speedup",
        r"amortized speedup ($U/S$)",
        outputs["speedup"],
        log_scale=True,
        labeled=False,
    )
    _save_metric_plot(
        df,
        styles,
        "amortized_speedup",
        r"amortized speedup ($U/S$)",
        outputs["speedup_labeled"],
        log_scale=True,
        labeled=True,
    )
    _save_metric_plot(
        df,
        styles,
        "jit_overhead_ms",
        r"JIT overhead in ms ($J$)",
        outputs["jit"],
        log_scale=True,
        labeled=False,
    )
    _save_metric_plot(
        df,
        styles,
        "jit_overhead_ms",
        r"JIT overhead in ms ($J$)",
        outputs["jit_labeled"],
        log_scale=True,
        labeled=True,
    )
    if not args.skip_legacy_combined:
        _save_legacy_combined(df, styles, outputs["legacy"])

    print(
        "Generated size-scaling figures for "
        f"{len(kernels)} kernels ({args.config}, {args.abstraction})."
    )
    for name, path in outputs.items():
        if path.exists():
            print(f"{name}: {path}")


if __name__ == "__main__":
    main()
