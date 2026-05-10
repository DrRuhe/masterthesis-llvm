#!/usr/bin/env python3
"""analyze.py — Unified JIT analysis launcher.

Discovers jit_analysis benchmarks matching a filter, runs them in a single
invocation with CRS_* env vars, extracts original ASM from the ELF binary,
generates normalized diffs, renders pass trace plots, and writes all artifacts
into a timestamped reports/<ts>-analysis/ directory.

Usage:
    ./analyze.py <binary> [--benchmarks_filter FILTER] [--timeout N]
                          [--no-asm-dump] [--no-pass-trace] [--no-chrome-trace]
"""

import argparse
import dataclasses
import os
import re
import shutil
import subprocess
import sys
import time
from datetime import datetime
from pathlib import Path
from typing import Optional

# Add benchmarks/ and benchmarks/reporting/ to sys.path so that reporting
# modules (pass_trace_plot_json etc.) are importable from analyse.py.
_BENCH_DIR   = Path(__file__).parent
_REPORT_DIR  = _BENCH_DIR / "reporting"
sys.path.insert(0, str(_BENCH_DIR))
sys.path.insert(0, str(_REPORT_DIR))

# ---------------------------------------------------------------------------
# Data structures
# ---------------------------------------------------------------------------

@dataclasses.dataclass
class AnalysisConfig:
    binary:       Path
    filter:       str
    timeout_s:    int
    asm:          bool
    pass_trace:   bool
    chrome_trace: bool


@dataclasses.dataclass
class BenchmarkRun:
    name:           str
    sanitized_name: str
    subdir:         Path
    status:         str                = "pending"   # pending|pass|fail|timeout
    elapsed_s:      float              = 0.0
    artifacts:      list               = dataclasses.field(default_factory=list)
    error_msg:      Optional[str]      = None

# ---------------------------------------------------------------------------
# Helper utilities
# ---------------------------------------------------------------------------

def _parse_bm_name(name: str) -> dict[str, str]:
    """Parse 'BM_key:val;key:val;...' benchmark name tags into a dict."""
    result: dict[str, str] = {}
    for part in name.split(";"):
        if ":" in part:
            k, _, v = part.partition(":")
            result[k.strip().lstrip("BM_")] = v.strip()
    return result


def _sanitize_name(name: str) -> str:
    """Replace all non-alphanumeric characters with '_'."""
    return re.sub(r"[^A-Za-z0-9]", "_", name)


def _find_objdump() -> Optional[str]:
    """Return path to llvm-objdump or objdump, or None if neither is on PATH."""
    return shutil.which("llvm-objdump") or shutil.which("objdump")


def _trace_name(name: str) -> str:
    """Return the sanitized name used by the C++ runtime for trace file prefixes.

    state.name() in Google Benchmark omits runtime config suffixes like
    /iterations:N and /manual_time that --benchmark_list_tests appends.
    """
    stripped = re.sub(r"(/iterations:\d+|/manual_time|/real_time|/\d+)+$", "", name)
    return _sanitize_name(stripped)

# ---------------------------------------------------------------------------
# Benchmark discovery
# ---------------------------------------------------------------------------

def list_jit_analysis_benchmarks(binary: Path, filter_str: str, timeout: int) -> list[str]:
    """Return benchmark names matching filter_str that have t:jit_analysis tag."""
    try:
        result = subprocess.run(
            [str(binary), "--benchmark_list_tests", f"--benchmark_filter={filter_str}"],
            capture_output=True, text=True, timeout=timeout, check=True,
        )
    except subprocess.TimeoutExpired:
        sys.exit(f"ERROR: Benchmark listing timed out after {timeout}s")
    except subprocess.CalledProcessError as exc:
        sys.exit(f"ERROR: Failed to list benchmarks (exit {exc.returncode}):\n{exc.stderr.strip()}")

    names: list[str] = []
    for line in result.stdout.splitlines():
        stripped = line.strip()
        if not stripped or stripped == "Benchmark":
            continue
        kv = _parse_bm_name(stripped)
        if kv.get("t") == "jit_analysis":
            names.append(stripped)
    return names

# ---------------------------------------------------------------------------
# Report directory setup
# ---------------------------------------------------------------------------

def create_report_dir(cwd: Path) -> tuple[Path, dict[str, Path]]:
    """Create timestamped report dir + staging subdirs.

    Returns (report_dir, staging_paths) where staging_paths maps
    'asm' | 'pass_trace' | 'chrome_trace' to their Path.
    """
    ts = datetime.now().strftime("%Y%m%d-%H%M%S")
    report_dir = cwd / "reports" / f"{ts}-analysis"
    staging = {
        "asm":          report_dir / "_staging" / "asm",
        "pass_trace":   report_dir / "_staging" / "pass_trace",
        "chrome_trace": report_dir / "_staging" / "chrome_trace",
    }
    for p in staging.values():
        p.mkdir(parents=True, exist_ok=True)
    return report_dir, staging

# ---------------------------------------------------------------------------
# Benchmark execution
# ---------------------------------------------------------------------------

def run_benchmarks(
    binary: Path,
    names: list[str],
    staging: dict[str, Path],
    config: AnalysisConfig,
) -> tuple[int, float, str]:
    """Run all jit_analysis benchmarks in one invocation.

    CRS_PASS_TRACE_DIR and CRS_CHROME_TRACE_DIR are always set because
    benchmarkJITAnalysis() throws if they are absent.

    Returns (returncode, elapsed_s, stderr_or_"timeout").
    """
    # Build |OR| filter from exact benchmark names.
    combined_filter = "|".join(re.escape(n) for n in names)

    env = os.environ.copy()
    env["CRS_PASS_TRACE_DIR"]   = str(staging["pass_trace"])
    env["CRS_CHROME_TRACE_DIR"] = str(staging["chrome_trace"])
    if config.asm:
        env["CRS_ASM_DUMP_DIR"] = str(staging["asm"])

    cmd = [
        str(binary),
        f"--benchmark_filter={combined_filter}",
        "--benchmark_min_time=1x",
    ]
    # Allow time for all N benchmarks plus startup overhead.
    total_timeout = config.timeout_s * len(names) + 30

    t0 = time.monotonic()
    try:
        proc = subprocess.run(
            cmd, capture_output=True, text=True, timeout=total_timeout, env=env,
        )
    except subprocess.TimeoutExpired:
        return -1, time.monotonic() - t0, "timeout"

    return proc.returncode, time.monotonic() - t0, proc.stderr

# ---------------------------------------------------------------------------
# ASM extraction and diffing
# ---------------------------------------------------------------------------

def normalize_asm(text: str) -> str:
    """Strip addresses, collapse whitespace, drop blank lines."""
    lines = []
    for line in text.splitlines():
        # Strip address from instruction lines: "   1234:  pushq" → "pushq"
        line = re.sub(r"^\s*[0-9a-f]+:\s*", "", line)
        # Strip leading hex address from function header lines: "000012345 <funcA>:" → "<funcA>:"
        line = re.sub(r"^[0-9a-f]{4,}\s+(<)", r"\1", line)
        line = re.sub(r"\s+", " ", line).strip()
        if line:
            lines.append(line)
    return "\n".join(lines)


def _parse_all_functions(objdump_output: str) -> dict[str, list[str]]:
    """Parse objdump -d output into {func_name: raw_lines} dict."""
    funcs: dict[str, list[str]] = {}
    current_name: Optional[str] = None
    current_lines: list[str] = []
    header_re = re.compile(r"^\S.*<([^>]+)>:")

    for line in objdump_output.splitlines():
        m = header_re.match(line)
        if m:
            if current_name is not None:
                funcs[current_name] = current_lines
            current_name = m.group(1)
            current_lines = [line]
        elif current_name is not None:
            current_lines.append(line)

    if current_name is not None and current_lines:
        funcs[current_name] = current_lines
    return funcs


def _find_callees(lines: list[str]) -> set[str]:
    """Return function names referenced by call/jmp instructions (no offset, no PLT)."""
    callees: set[str] = set()
    target_re = re.compile(r"\b(?:call[ql]?|jmp[ql]?)\b.*<([^>+@]+)>")
    for line in lines:
        m = target_re.search(line)
        if m:
            callees.add(m.group(1))
    return callees


def extract_orig_asm(binary: Path, func_name: str, out_path: Path, objdump: str) -> bool:
    """Extract func_name and all transitively called functions from binary ELF.

    Writes normalized text to out_path. Returns True if func_name was found.
    """
    try:
        result = subprocess.run(
            [objdump, "-d", "--no-show-raw-insn", str(binary)],
            capture_output=True, text=True, timeout=60,
        )
    except subprocess.TimeoutExpired:
        print(f"  WARNING: objdump timed out extracting '{func_name}'", file=sys.stderr)
        out_path.write_text("")
        return False

    all_funcs = _parse_all_functions(result.stdout)

    if func_name not in all_funcs:
        out_path.write_text("")
        print(f"  WARNING: symbol '{func_name}' not found in ELF (inline or stripped). "
              "Empty placeholder written.", file=sys.stderr)
        return False

    # BFS: collect the target function and all transitively called functions (cap at 50)
    visited: list[str] = []
    visited_set: set[str] = set()
    queue = [func_name]
    while queue and len(visited) < 50:
        fn = queue.pop(0)
        if fn in visited_set:
            continue
        visited_set.add(fn)
        if fn not in all_funcs:
            continue
        visited.append(fn)
        for callee in sorted(_find_callees(all_funcs[fn])):
            if callee not in visited_set:
                queue.append(callee)

    sections = "\n\n".join("\n".join(all_funcs[fn]) for fn in visited if fn in all_funcs)
    normalized = normalize_asm(sections)
    out_path.write_text(normalized)
    return True

# ---------------------------------------------------------------------------
# Artifact routing
# ---------------------------------------------------------------------------

def _find_asm_for_kernel(kernel_tag: str, staging_asm: Path) -> list[Path]:
    """Return specialized ASM files whose stem contains kernel_tag."""
    if not kernel_tag:
        return list(staging_asm.glob("*__specialized.asm"))
    return [p for p in staging_asm.glob("*__specialized.asm") if kernel_tag in p.stem]


def route_artifacts(
    staging:   dict[str, Path],
    runs:      list[BenchmarkRun],
    config:    AnalysisConfig,
    objdump:   Optional[str],
) -> None:
    """Route artifacts from staging dirs into per-benchmark subdirs."""
    for run in runs:
        run.subdir.mkdir(parents=True, exist_ok=True)
        kv          = _parse_bm_name(run.name)
        kernel_tag  = kv.get("n", "")

        # ── Pass trace JSON + plot ─────────────────────────────────────
        if config.pass_trace:
            src = staging["pass_trace"] / (_trace_name(run.name) + "_pass_trace.json")
            if src.exists():
                dst = run.subdir / src.name
                shutil.move(str(src), dst)
                run.artifacts.append(dst)
                try:
                    from pass_trace_plot_json import plot_from_json
                    plot_dst = run.subdir / "pass_trace_plot.pdf"
                    plot_from_json(dst, plot_dst)
                    run.artifacts.append(plot_dst)
                except Exception as exc:
                    print(f"  WARNING: pass trace plot failed for {run.name}: {exc}",
                          file=sys.stderr)
            else:
                print(f"  WARNING: pass trace JSON not found: {src.name}", file=sys.stderr)

        # ── Chrome trace JSON ─────────────────────────────────────────
        if config.chrome_trace:
            src = staging["chrome_trace"] / (_trace_name(run.name) + "_chrome_trace.json")
            if src.exists():
                dst = run.subdir / src.name
                shutil.move(str(src), dst)
                run.artifacts.append(dst)
            else:
                print(f"  WARNING: chrome trace JSON not found: {src.name}", file=sys.stderr)

        # ── Specialized ASM + original ASM (with callees) ────────────
        if config.asm and objdump:
            spec_files = _find_asm_for_kernel(kernel_tag, staging["asm"])
            if not spec_files and not kernel_tag:
                spec_files = list(staging["asm"].glob("*__specialized.asm"))
            for spec_src in spec_files:
                func_name = spec_src.stem.replace("__specialized", "")
                spec_dst  = run.subdir / spec_src.name
                shutil.copy2(str(spec_src), spec_dst)
                run.artifacts.append(spec_dst)

                orig_dst = run.subdir / f"orig_{func_name}.asm"
                ok = extract_orig_asm(config.binary, func_name, orig_dst, objdump)
                if ok:
                    run.artifacts.append(orig_dst)

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------

def write_summary(report_dir: Path, runs: list[BenchmarkRun]) -> None:
    """Write summary.md to report_dir listing all benchmarks with status and artifact counts."""
    ts = report_dir.name
    lines = [
        f"# Analysis Report — {ts}",
        "",
        "| # | Benchmark | Status | Elapsed (s) | Artifacts |",
        "|---|-----------|--------|-------------|-----------|",
    ]
    for i, run in enumerate(runs, 1):
        if run.status == "pass":
            icon = "✅ pass"
        elif run.status == "timeout":
            icon = "⏱ timeout"
        else:
            icon = "❌ fail"
        lines.append(
            f"| {i} | `{run.name}` | {icon} | {run.elapsed_s:.1f} | {len(run.artifacts)} |"
        )

    failures = [r for r in runs if r.error_msg]
    if failures:
        lines += ["", "## Failures", ""]
        for run in failures:
            lines.append(f"- **{run.name}**: {run.error_msg}")

    (report_dir / "summary.md").write_text("\n".join(lines) + "\n")

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main() -> None:
    parser = argparse.ArgumentParser(
        prog="analyze.py",
        description=(
            "Discover, run, and report jit_analysis benchmarks.\n"
            "Produces a timestamped reports/<ts>-analysis/ directory containing\n"
            "per-benchmark subdirectories with ASM diffs, pass trace plots,\n"
            "chrome traces, and a summary.md."
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("binary", type=Path,
                        help="Path to the benchmark executable.")
    parser.add_argument("--benchmarks_filter", default="",
                        metavar="FILTER",
                        help="Forwarded to --benchmark_filter (Google Benchmark key:value tags).")
    parser.add_argument("--timeout", type=int, default=60, metavar="N",
                        help="Per-benchmark wall-clock timeout in seconds (default: 60).")
    parser.add_argument("--no-asm-dump",     action="store_true",
                        help="Disable ASM extraction, diff generation, and objdump calls.")
    parser.add_argument("--no-pass-trace",   action="store_true",
                        help="Disable pass trace JSON collection and plot rendering.")
    parser.add_argument("--no-chrome-trace", action="store_true",
                        help="Disable chrome trace JSON collection.")
    args = parser.parse_args()

    config = AnalysisConfig(
        binary       = args.binary.resolve(),
        filter       = args.benchmarks_filter,
        timeout_s    = args.timeout,
        asm          = not args.no_asm_dump,
        pass_trace   = not args.no_pass_trace,
        chrome_trace = not args.no_chrome_trace,
    )

    # ── Validate binary ──────────────────────────────────────────────
    if not config.binary.exists():
        sys.exit(f"ERROR: Binary not found: {config.binary}")
    if not os.access(config.binary, os.X_OK):
        sys.exit(f"ERROR: Binary is not executable: {config.binary}")

    # ── Check objdump ────────────────────────────────────────────────
    objdump: Optional[str] = None
    if config.asm:
        objdump = _find_objdump()
        if not objdump:
            sys.exit(
                "ERROR: llvm-objdump / objdump not found on PATH.\n"
                "       Install it or pass --no-asm-dump to skip ASM extraction."
            )

    # ── Discover jit_analysis benchmarks ────────────────────────────
    filter_str = config.filter or "."
    print(f"Discovering jit_analysis benchmarks (filter: '{config.filter or ''}')...")
    t_disc = time.monotonic()
    names  = list_jit_analysis_benchmarks(config.binary, filter_str, config.timeout_s)
    disc_s = time.monotonic() - t_disc

    if not names:
        print(f"No jit_analysis benchmarks matched filter '{config.filter}'. Nothing to do.")
        return

    print(f"Found {len(names)} jit_analysis benchmark(s) in {disc_s:.1f}s:")
    for i, n in enumerate(names, 1):
        print(f"  {i}. {n}")

    # ── Create report directory ──────────────────────────────────────
    cwd = Path.cwd()
    report_dir, staging = create_report_dir(cwd)
    print(f"\nReport directory: {report_dir}")

    # ── Build run objects ────────────────────────────────────────────
    runs = [
        BenchmarkRun(
            name           = n,
            sanitized_name = _sanitize_name(n),
            subdir         = report_dir / _sanitize_name(n),
        )
        for n in names
    ]
    total = len(runs)

    # ── Run all benchmarks (single invocation) ───────────────────────
    print(f"\n[1/{total}] Running {total} jit_analysis benchmark(s)...")
    rc, elapsed, stderr_text = run_benchmarks(config.binary, names, staging, config)

    if rc == -1 and stderr_text == "timeout":
        print(f"  ⏱  Overall invocation timed out after {elapsed:.1f}s")
        for run in runs:
            run.status    = "timeout"
            run.elapsed_s = elapsed / total
            run.error_msg = "benchmark invocation timed out"
    elif rc != 0:
        print(f"  ⚠  Benchmark invocation exited with code {rc}")
        for line in stderr_text.splitlines()[:15]:
            print(f"     {line}")
        for run in runs:
            run.status    = "fail"
            run.elapsed_s = elapsed / total
            run.error_msg = f"benchmark process exited with code {rc}"
    else:
        per_s = elapsed / total
        for run in runs:
            run.elapsed_s = per_s
            run.status    = "pass"
        print(f"  ✅ Done ({elapsed:.1f}s total, ~{per_s:.1f}s/benchmark).")

    # ── Route artifacts ──────────────────────────────────────────────
    print("\nRouting artifacts to benchmark subdirectories...")
    route_artifacts(staging, runs, config, objdump)

    # Mark benchmarks with no artifacts as failed (unless already timed-out/failed).
    for run in runs:
        if run.status == "pass" and not run.artifacts:
            run.status    = "fail"
            run.error_msg = "no artifacts produced"

    # Remove staging dir.
    shutil.rmtree(report_dir / "_staging", ignore_errors=True)

    # ── Per-benchmark progress lines ─────────────────────────────────
    for i, run in enumerate(runs, 1):
        icon = "✅" if run.status == "pass" else ("⏱" if run.status == "timeout" else "❌")
        print(
            f"  [{i}/{total}] {icon} {run.name} "
            f"({run.elapsed_s:.1f}s, {len(run.artifacts)} artifact(s))"
        )

    # ── Write summary ────────────────────────────────────────────────
    write_summary(report_dir, runs)

    passed = sum(1 for r in runs if r.status == "pass")
    print(f"\nDone: {passed}/{total} benchmark(s) passed.")
    print(f"Report: {report_dir}")


if __name__ == "__main__":
    main()
