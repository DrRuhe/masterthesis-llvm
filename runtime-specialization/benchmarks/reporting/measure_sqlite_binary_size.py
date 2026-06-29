#!/usr/bin/env python3
"""Measure SQLite object-size overhead on the same basis as the UC RQ2 case study."""

from __future__ import annotations

import argparse
import datetime as dt
import re
import shlex
import subprocess
import tempfile
from pathlib import Path


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Rebuild sqlite3_with_accessor.c without the IR-dumping plugin, compare it "
            "against the plugin-built sqlite3_with_ir.o, and write a text report."
        )
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
        "--output",
        default=None,
        metavar="PATH",
        help="Optional output report path. Defaults to a timestamped report directory.",
    )
    return parser.parse_args()


def _extract_compile_command(build_ninja: Path) -> tuple[Path, list[str], Path]:
    text = build_ninja.read_text(encoding="utf-8")
    marker = "build tools/runtime-specialization/benchmarks/tpch/sqlite3_with_ir.o"
    idx = text.find(marker)
    if idx == -1:
        raise RuntimeError(f"Could not find sqlite3_with_ir.o rule in {build_ninja}")

    command_match = re.search(r"^  COMMAND = (.+)$", text[idx:], flags=re.MULTILINE)
    if not command_match:
        raise RuntimeError("Could not find COMMAND line for sqlite3_with_ir.o")

    command = command_match.group(1)
    if not command.startswith("cd "):
        raise RuntimeError(f"Unexpected command format: {command}")

    workdir_part, argv_part = command[3:].split(" && ", 1)
    workdir = Path(workdir_part.strip())
    argv = shlex.split(argv_part)
    plugin_obj = Path(argv[argv.index("-o") + 1])
    return workdir, argv, plugin_obj


def _strip_plugin_flag(argv: list[str], output_obj: Path) -> list[str]:
    out = []
    skip_next = False
    for i, arg in enumerate(argv):
        if skip_next:
            skip_next = False
            continue
        if arg.startswith("-fpass-plugin="):
            continue
        if arg == "-o":
            out.extend(["-o", str(output_obj)])
            skip_next = True
            continue
        out.append(arg)
    return out


def _strip_output_flag(argv: list[str]) -> list[str]:
    out = []
    skip_next = False
    for arg in argv:
        if skip_next:
            skip_next = False
            continue
        if arg == "-o":
            skip_next = True
            continue
        out.append(arg)
    return out


def _run_text(cmd: list[str]) -> str:
    return subprocess.run(cmd, check=True, text=True, capture_output=True).stdout


def _parse_size_default(path: Path) -> dict[str, int]:
    lines = _run_text(["size", str(path)]).strip().splitlines()
    if len(lines) < 2:
        raise RuntimeError(f"Unexpected size output for {path}")
    fields = lines[-1].split()
    return {
        "text": int(fields[0]),
        "data": int(fields[1]),
        "bss": int(fields[2]),
        "total": int(fields[3]),
    }


def _parse_size_a(path: Path) -> dict[str, int]:
    sections: dict[str, int] = {}
    for line in _run_text(["size", "-A", "-d", str(path)]).splitlines():
        parts = line.split()
        if len(parts) < 2:
            continue
        name, size = parts[0], parts[1]
        if not re.fullmatch(r"\d+", size):
            continue
        sections[name] = int(size)
    return sections


def _sum_sections(sections: dict[str, int], prefix: str) -> int:
    return sum(size for name, size in sections.items() if name.startswith(prefix))


def _format_bytes(n: int) -> str:
    if n >= 1024:
        return f"{n / 1024.0:,.1f} KB"
    return f"{n:,} B"


def _format_pct(delta: int, baseline: int) -> str:
    if baseline == 0:
        return "n/a"
    return f"+{(delta / baseline) * 100.0:,.1f}%"


def _default_output(repo_root: Path) -> Path:
    stamp = dt.datetime.now().strftime("%y%m%d-%H%M%S")
    report_dir = repo_root / "benchmarks" / "reports" / f"{stamp}-sqlite-binary-size"
    report_dir.mkdir(parents=True, exist_ok=False)
    return report_dir / "sqlite_binary_size_table.txt"


def main() -> None:
    args = _parse_args()
    repo_root = Path(args.repo_root)
    release_build = Path(args.release_build)
    build_ninja = release_build / "build.ninja"

    workdir, plugin_argv, plugin_obj = _extract_compile_command(build_ninja)
    if not plugin_obj.exists():
        raise FileNotFoundError(f"Plugin-built object not found: {plugin_obj}")

    output_path = Path(args.output) if args.output else _default_output(repo_root)
    output_path.parent.mkdir(parents=True, exist_ok=True)

    with tempfile.TemporaryDirectory(prefix="rq2-sqlite-size-") as tmpdir:
        baseline_obj = Path(tmpdir) / "sqlite3_without_ir.o"
        baseline_argv = _strip_plugin_flag(plugin_argv, baseline_obj)
        subprocess.run(baseline_argv, check=True, cwd=workdir)

        baseline_file_bytes = baseline_obj.stat().st_size
        plugin_file_bytes = plugin_obj.stat().st_size
        baseline_size = _parse_size_default(baseline_obj)
        plugin_size = _parse_size_default(plugin_obj)
        baseline_sections = _parse_size_a(baseline_obj)
        plugin_sections = _parse_size_a(plugin_obj)

    blob_rodata = max(_sum_sections(plugin_sections, ".rodata") - _sum_sections(baseline_sections, ".rodata"), 0)
    extra_text = max(_sum_sections(plugin_sections, ".text") - _sum_sections(baseline_sections, ".text"), 0)
    extra_data = max(_sum_sections(plugin_sections, ".data") - _sum_sections(baseline_sections, ".data"), 0)
    extra_init_array = max(plugin_sections.get(".init_array", 0) - baseline_sections.get(".init_array", 0), 0)
    total_file_overhead = plugin_file_bytes - baseline_file_bytes
    plugin_remainder = max(total_file_overhead - blob_rodata - extra_text - extra_data - extra_init_array, 0)

    text = f"""Binary-Size Overhead of SQLite IR-Dump + Runtime Infrastructure
===========================================================
Date: {dt.date.today().isoformat()}
Purpose: RQ2 follow-up — quantify per-TU binary size cost of the IRDumpingPass plugin for SQLite on the same basis as the UC case study

Subject TU
----------
File:      benchmarks/tpch/sqlite3_with_accessor.c
Content:   SQLite amalgamation TU with `sqlite3VdbeExec` IR blob and accessor hooks
Compiler:  {' '.join(_strip_output_flag([arg for arg in plugin_argv if not arg.startswith('-fpass-plugin=')]))}
Plugin:    {next(arg.split('=', 1)[1] for arg in plugin_argv if arg.startswith('-fpass-plugin='))}
Method:    Exact release-build compile command from `build.ninja`, rebuilt once with and once without `-fpass-plugin`

Object File Sizes  (from `size` and `wc -c`)
---------------------------------------------
                        Without plugin    With plugin    Overhead
                        --------------    -----------    --------
text (size tool)*       {baseline_size['text']:>12,} B  {plugin_size['text']:>13,} B  {plugin_size['text'] - baseline_size['text']:+11,} B  ({_format_pct(plugin_size['text'] - baseline_size['text'], baseline_size['text'])})
data (size tool)        {baseline_size['data']:>12,} B  {plugin_size['data']:>13,} B  {plugin_size['data'] - baseline_size['data']:+11,} B  ({_format_pct(plugin_size['data'] - baseline_size['data'], baseline_size['data'])})
bss                     {baseline_size['bss']:>12,} B  {plugin_size['bss']:>13,} B  {plugin_size['bss'] - baseline_size['bss']:+11,} B
Total sections (size)   {baseline_size['total']:>12,} B  {plugin_size['total']:>13,} B  {plugin_size['total'] - baseline_size['total']:+11,} B  ({_format_pct(plugin_size['total'] - baseline_size['total'], baseline_size['total'])})
On-disk file size       {baseline_file_bytes:>12,} B  {plugin_file_bytes:>13,} B  {total_file_overhead:+11,} B  ({_format_pct(total_file_overhead, baseline_file_bytes)})
                          {_format_bytes(baseline_file_bytes):>12}  {_format_bytes(plugin_file_bytes):>13}  {(_format_bytes(total_file_overhead)):>11}

* `size(1)` on ELF counts `.text`, `.rodata`, `.gcc_except_table`, and similar sections
  into the "text" column. See the section breakdown below for the split.

Detailed Section Breakdown  (from `size -A -d`)
-----------------------------------------------
                        Without plugin    With plugin    Overhead
                        --------------    -----------    --------
.text* (code only)      {_sum_sections(baseline_sections, '.text'):>12,} B  {_sum_sections(plugin_sections, '.text'):>13,} B  {extra_text:+11,} B
.rodata* (read-only)    {_sum_sections(baseline_sections, '.rodata'):>12,} B  {_sum_sections(plugin_sections, '.rodata'):>13,} B  {blob_rodata:+11,} B
.data*                  {_sum_sections(baseline_sections, '.data'):>12,} B  {_sum_sections(plugin_sections, '.data'):>13,} B  {extra_data:+11,} B
.init_array             {baseline_sections.get('.init_array', 0):>12,} B  {plugin_sections.get('.init_array', 0):>13,} B  {extra_init_array:+11,} B

IR Blob Analysis
----------------
The IR-dumping plugin embeds a serialized LLVM bitcode snapshot of the SQLite TU
as a read-only data blob registered at startup.

.rodata section growth:          {blob_rodata:,} bytes  ({_format_bytes(blob_rodata)})
  As % of total file overhead:    {(blob_rodata / total_file_overhead * 100.0):.1f}%
  As % of with-plugin file size:  {(blob_rodata / plugin_file_bytes * 100.0):.1f}%

Extra .text growth:              {extra_text:,} bytes  ({_format_bytes(extra_text)})
Extra .data growth:              {extra_data:,} bytes  ({_format_bytes(extra_data)})
Other plugin overhead:           {plugin_remainder:,} bytes  ({_format_bytes(plugin_remainder)})

Summary Table
-------------
Metric                         Without plugin    With plugin    Overhead
-----------------------------  --------------    -----------    --------
Object file (on disk)         {_format_bytes(baseline_file_bytes):>12}  {_format_bytes(plugin_file_bytes):>13}  {(_format_bytes(total_file_overhead)):>11}  ({_format_pct(total_file_overhead, baseline_file_bytes)})
Loadable sections (text+data) {_format_bytes(baseline_size['total']):>12}  {_format_bytes(plugin_size['total']):>13}  {(_format_bytes(plugin_size['total'] - baseline_size['total'])):>11}
  of which .rodata growth     {_format_bytes(_sum_sections(baseline_sections, '.rodata')):>12}  {_format_bytes(_sum_sections(plugin_sections, '.rodata')):>13}  {(_format_bytes(blob_rodata)):>11}
  of which extra .text        {_format_bytes(_sum_sections(baseline_sections, '.text')):>12}  {_format_bytes(_sum_sections(plugin_sections, '.text')):>13}  {(_format_bytes(extra_text)):>11}
  of which extra .data        {_format_bytes(_sum_sections(baseline_sections, '.data')):>12}  {_format_bytes(_sum_sections(plugin_sections, '.data')):>13}  {(_format_bytes(extra_data)):>11}

Notes
-----
1. This measurement uses the same object-level before/after-plugin basis as the recorded
   UC RQ2 case study, so the two TUs can be compared directly without mixing in linked-binary
   or benchmark-harness metadata.
2. The correct SQLite subject TU is `sqlite3_with_accessor.c` / `sqlite3_with_ir.o`, not
   `sqlite3_tpch_bench.cpp`, because this is the TU that actually embeds the SQLite IR blob.
3. The benchmark path's blob-selection issue affects specialization behavior, but it does not
   change this binary-size measurement because the object-level comparison is performed on the
   blob-carrying TU itself.
"""

    output_path.write_text(text, encoding="utf-8")
    print(f"Saved: {output_path}")


if __name__ == "__main__":
    main()
