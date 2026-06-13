#!/usr/bin/env python3
"""Render a stacked binary-size breakdown figure from the RQ2 report text."""

from __future__ import annotations

import argparse
import re
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Plot the binary-size breakdown from the report text.")
    parser.add_argument(
        "--report",
        default="benchmarks/reports/260610-binary-size/binary_size_table.txt",
        metavar="PATH",
    )
    parser.add_argument(
        "--output",
        default="benchmarks/reports/thesis-figures/rq2/rq2_binary_size_breakdown.png",
        metavar="PATH",
    )
    return parser.parse_args()


def _extract_bytes(text: str, label: str) -> float:
    match = re.search(rf"{re.escape(label)}\s*([0-9,]+)\s+bytes", text)
    if not match:
        raise ValueError(f"Could not extract '{label}' from report.")
    return float(match.group(1).replace(",", ""))


def main() -> None:
    args = _parse_args()
    text = Path(args.report).read_text(encoding="utf-8")
    on_disk_matches = re.findall(r"On-disk file size\s+([0-9,]+)\s+B\s+([0-9,]+)\s+B", text)
    if not on_disk_matches:
        raise ValueError("Could not extract on-disk size pair from report.")
    without_plugin, with_plugin = [float(x.replace(",", "")) for x in on_disk_matches[0]]
    extra_text = _extract_bytes(text, "Extra .text from LLVM/CRS API header inlines pulled in by the plugin:")
    blob = _extract_bytes(text, ".rodata section (IR blob):")
    extra_data = 168.0
    baseline_rest = without_plugin
    plugin_remainder = max(with_plugin - blob - extra_text - extra_data - baseline_rest, 0.0)

    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)

    fig, ax = plt.subplots(figsize=(7.5, 5))
    labels = ["Baseline object", "Plugin-enabled object"]
    xpos = np.arange(2)

    ax.bar(xpos[0], baseline_rest / 1024.0, color="#6c757d", label="Baseline object")

    bottom = 0.0
    stacked = [
        ("Baseline-equivalent", baseline_rest / 1024.0, "#6c757d"),
        ("IR blob (.rodata)", blob / 1024.0, "#457b9d"),
        ("Extra .text", extra_text / 1024.0, "#e76f51"),
        ("Extra .data", extra_data / 1024.0, "#f4a261"),
        ("Other plugin overhead", plugin_remainder / 1024.0, "#2a9d8f"),
    ]
    for name, value, color in stacked:
        ax.bar(xpos[1], value, bottom=bottom, color=color, label=name)
        bottom += value

    ax.set_xticks(xpos, labels)
    ax.set_ylabel("Object file size [KiB]")
    ax.set_title("Binary-Size Breakdown for the Measured UC1 Translation Unit")
    handles, labels = ax.get_legend_handles_labels()
    by_label = dict(zip(labels, handles))
    ax.legend(by_label.values(), by_label.keys(), fontsize=8, loc="upper left")
    fig.savefig(output, dpi=200, bbox_inches="tight")
    plt.close(fig)
    print(f"Saved: {output}")


if __name__ == "__main__":
    main()
