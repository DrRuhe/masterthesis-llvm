#!/usr/bin/env python3
"""Measure per-kernel object-size overhead for the UC low-level kernel TUs."""

from __future__ import annotations

import argparse
import csv
import re
import shlex
import subprocess
import tempfile
from pathlib import Path


UC_DIRS = [
    "UC1SqlPredicate",
    "UC2Convolution",
    "UC7DfaRegex",
    "UC8IVM",
    "UC12GroupBy",
    "UC14Sort",
]


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Compile baseline UC low-kernel objects and compare against plugin-built release objects."
    )
    parser.add_argument(
        "--release-build",
        default="/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/llvm/build/release",
        metavar="DIR",
    )
    parser.add_argument(
        "--repo-root",
        default=str(Path(__file__).resolve().parents[2]),
        metavar="DIR",
    )
    parser.add_argument(
        "--output-csv",
        default="benchmarks/reports/thesis-figures/rq2/rq2_binary_size_measurements.csv",
        metavar="PATH",
    )
    return parser.parse_args()


def _extract_cxx_flags(cache_path: Path) -> list[str]:
    text = cache_path.read_text(encoding="utf-8")
    match = re.search(r"CMAKE_CXX_FLAGS:STRING=(.*)", text)
    if not match:
        raise RuntimeError(f"Could not find CMAKE_CXX_FLAGS in {cache_path}")
    return shlex.split(match.group(1))


def _iter_low_sources(use_cases_root: Path) -> list[Path]:
    out = []
    for uc_dir in UC_DIRS:
        for path in sorted((use_cases_root / uc_dir).glob("*LowKernels.cpp")):
            out.append(path)
    return out


def main() -> None:
    args = _parse_args()
    repo_root = Path(args.repo_root)
    release_build = Path(args.release_build)
    output_csv = Path(args.output_csv)
    output_csv.parent.mkdir(parents=True, exist_ok=True)

    cache_path = release_build / "CMakeCache.txt"
    clangxx = release_build / "bin/clang++"
    llvm_include = repo_root.parent / "llvm/include"
    llvm_binary_include = release_build / "include"
    runtime_include = repo_root / "runtime/ClangRuntimeSpecializer"
    use_cases_root = repo_root / "benchmarks/use-cases"
    plugin_obj_root = release_build / "tools/runtime-specialization/benchmarks/use-cases"
    cxx_flags = _extract_cxx_flags(cache_path)

    with tempfile.TemporaryDirectory(prefix="rq2-size-") as tmpdir:
        tmpdir_path = Path(tmpdir)
        rows = []
        for src in _iter_low_sources(use_cases_root):
            stem = src.stem
            uc_dir = src.parent.name
            plugin_obj = plugin_obj_root / uc_dir / f"{stem}.o"
            if not plugin_obj.exists():
                raise FileNotFoundError(f"Missing plugin-built object: {plugin_obj}")

            baseline_obj = tmpdir_path / f"{stem}.baseline.o"
            cmd = [
                str(clangxx),
                "-x", "c++",
                "-g",
                "-O3",
                *cxx_flags,
                f"-I{src.parent}",
                f"-I{runtime_include}",
                f"-I{llvm_include}",
                f"-I{llvm_binary_include}",
                "-fexceptions",
                "-frtti",
                "-c",
                str(src),
                "-o",
                str(baseline_obj),
            ]
            subprocess.run(cmd, check=True, cwd=repo_root)

            baseline_bytes = baseline_obj.stat().st_size
            plugin_bytes = plugin_obj.stat().st_size
            overhead_bytes = plugin_bytes - baseline_bytes
            overhead_pct = (overhead_bytes / baseline_bytes * 100.0) if baseline_bytes else 0.0

            kernel = re.sub(r"^UC\d+", "", stem).replace("LowKernels", "")
            kernel = re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", kernel).lower()
            rows.append(
                {
                    "uc_dir": uc_dir,
                    "kernel": kernel,
                    "source": str(src.relative_to(repo_root)),
                    "plugin_obj": str(plugin_obj.relative_to(repo_root.parent)),
                    "baseline_bytes": baseline_bytes,
                    "plugin_bytes": plugin_bytes,
                    "overhead_bytes": overhead_bytes,
                    "overhead_pct": overhead_pct,
                }
            )

    with output_csv.open("w", encoding="utf-8", newline="") as fh:
        writer = csv.DictWriter(
            fh,
            fieldnames=[
                "uc_dir",
                "kernel",
                "source",
                "plugin_obj",
                "baseline_bytes",
                "plugin_bytes",
                "overhead_bytes",
                "overhead_pct",
            ],
        )
        writer.writeheader()
        writer.writerows(rows)

    print(f"Saved: {output_csv}")


if __name__ == "__main__":
    main()
