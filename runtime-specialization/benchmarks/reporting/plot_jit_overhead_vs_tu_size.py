#!/usr/bin/env python3
"""Plot thesis JIT overhead against TU-size for UC, SQLite, and PolyBench."""

from __future__ import annotations

import argparse
import csv
import math
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

import duckdb
os.environ.setdefault("MPLCONFIGDIR", "/tmp/matplotlib-runtime-specialization")
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.ticker as mticker
import pandas as pd
from matplotlib.lines import Line2D

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from report_utils import open_db, resolve_db_path


DEFAULT_UC_STUDY = "corpus_uc_first_call_quadrants_20260620_160209"
DEFAULT_UC_CONFIG = "default"
DEFAULT_RELEASE_BUILD = "/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/llvm/build/release"
DEFAULT_OUTPUT_DIR = "benchmarks/reports/thesis-figures/rq1"

SQLITE_POLYBENCH_GROUPS = ("polybench", "db/sqlite3/input_size", "db/sqlite3/tpch")
UC_GROUPS = ("uc1_sql", "uc2_conv", "uc7_dfa", "uc8_ivm", "uc12_groupby", "uc14_sort")
UC_GROUP_BY_DIR = {
    "UC1SqlPredicate": "uc1_sql",
    "UC2Convolution": "uc2_conv",
    "UC7DfaRegex": "uc7_dfa",
    "UC8IVM": "uc8_ivm",
    "UC12GroupBy": "uc12_groupby",
    "UC14Sort": "uc14_sort",
}
UC_ABSTRACTION_SUFFIX = {
    "Low": "low",
    "Tradeoff": "tradeoff",
    "Abstract": "abstract",
}
SUITE_COLORS = {
    "UC": "#1d3557",
    "PolyBench": "#d62828",
    "SQLite input-size": "#2a9d8f",
    "SQLite TPC-H": "#ff9f1c",
}
SUITE_MARKERS = {
    "UC": "o",
    "PolyBench": "s",
    "SQLite input-size": "^",
    "SQLite TPC-H": "D",
}
IR_SECTION_NAME = ".rodata.RuntimeSpecializeableIR_data"
IR_SYMBOL_NAME = "RuntimeSpecializeableIR_data"
MANIFEST_CACHE_NAME = "uc_tu_size_manifest.csv"
POINTS_CSV_NAME = "jit_overhead_vs_tu_size_points.csv"
PLOT_NAMES = {
    "jit_module_instrs": "jit_overhead_vs_tu_size_instrs.png",
    "jit_blob_kb": "jit_overhead_vs_tu_size_blob_kb.png",
}
INSTRUCTION_OPCODES = {
    "addrspacecast", "add", "alloca", "and", "ashr", "atomicrmw", "bitcast",
    "br", "call", "callbr", "catchpad", "catchret", "catchswitch", "cleanupret",
    "cleanuppad", "cmpxchg", "extractelement", "extractvalue", "fadd", "fcmp",
    "fdiv", "fence", "fmul", "fneg", "fpext", "fptosi", "fptoui", "fptrunc",
    "freeze", "frem", "fsub", "getelementptr", "icmp", "indirectbr", "insertelement",
    "insertvalue", "inttoptr", "invoke", "landingpad", "load", "lshr", "mul",
    "or", "phi", "ptrtoint", "resume", "ret", "sdiv", "select", "sext", "shl",
    "shufflevector", "sitofp", "srem", "store", "sub", "switch", "trunc", "udiv",
    "uitofp", "unreachable", "urem", "va_arg", "xor", "zext",
}
INSTRUCTION_PREFIX_MODIFIERS = {
    "afn", "arcp", "contract", "disjoint", "exact", "fast", "inbounds",
    "musttail", "nnan", "ninf", "nneg", "notail", "nsw", "nuw", "nsz",
    "reassoc", "tail", "volatile",
}


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Build thesis JIT-overhead versus TU-size scatterplots with UC inclusion."
    )
    parser.add_argument("--db", default=None, metavar="PATH")
    parser.add_argument("--sqlite-polybench-study-name", required=True, metavar="NAME")
    parser.add_argument("--uc-study-name", default=DEFAULT_UC_STUDY, metavar="NAME")
    parser.add_argument("--uc-config-name", default=DEFAULT_UC_CONFIG, metavar="NAME")
    parser.add_argument("--release-build", default=DEFAULT_RELEASE_BUILD, metavar="DIR")
    parser.add_argument(
        "--x-metric",
        default="both",
        choices=("jit_module_instrs", "jit_blob_kb", "both"),
        metavar="NAME",
    )
    parser.add_argument("--output-dir", default=DEFAULT_OUTPUT_DIR, metavar="DIR")
    return parser.parse_args()


def _camel_to_snake(text: str) -> str:
    normalized = re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", text).lower()
    return normalized.replace("min_max", "minmax")


def _suite_label(group: str) -> str:
    if group == "polybench":
        return "PolyBench"
    if group == "db/sqlite3/input_size":
        return "SQLite input-size"
    if group == "db/sqlite3/tpch":
        return "SQLite TPC-H"
    if group in UC_GROUPS:
        return "UC"
    raise RuntimeError(f"Unsupported benchmark group '{group}'.")


def _ensure_output_dir(path: Path) -> Path:
    path.mkdir(parents=True, exist_ok=True)
    return path


def _check_study_exists(con: duckdb.DuckDBPyConnection, study_name: str) -> None:
    count = con.execute(
        "SELECT COUNT(*) FROM ablation_studies WHERE study_name = ?",
        [study_name],
    ).fetchone()[0]
    if not count:
        raise RuntimeError(f"Study '{study_name}' was not found in the DuckDB database.")


def _validate_sqlite_polybench_study(con: duckdb.DuckDBPyConnection, study_name: str) -> None:
    _check_study_exists(con, study_name)
    row = con.execute(
        """
        SELECT
            COUNT(DISTINCT a.run_id) AS total_runs,
            COUNT(DISTINCT CASE WHEN c.best_practice_full THEN a.run_id END) AS best_practice_runs,
            COUNT(DISTINCT a.config_name) FILTER (WHERE a.run_id IS NOT NULL) AS config_count,
            COUNT(*) FILTER (WHERE a.config_name = 'default') AS default_rows
        FROM ablation_studies a
        LEFT JOIN context c USING (run_id)
        WHERE a.study_name = ?
        """,
        [study_name],
    ).fetchone()
    total_runs = int(row[0] or 0)
    best_practice_runs = int(row[1] or 0)
    config_count = int(row[2] or 0)
    default_rows = int(row[3] or 0)
    if total_runs == 0:
        raise RuntimeError(
            f"Study '{study_name}' exists, but it has no recorded run_id rows."
        )
    if best_practice_runs != total_runs:
        raise RuntimeError(
            f"Study '{study_name}' is not fully best-practice: {best_practice_runs}/{total_runs} runs have best_practice_full=TRUE."
        )
    if config_count != 1 or default_rows == 0:
        raise RuntimeError(
            f"Study '{study_name}' must contain exactly one recorded config named 'default'."
        )


def _validate_uc_study(con: duckdb.DuckDBPyConnection, study_name: str, config_name: str) -> None:
    _check_study_exists(con, study_name)
    row = con.execute(
        """
        SELECT
            COUNT(DISTINCT a.run_id) AS total_runs,
            COUNT(DISTINCT CASE WHEN c.best_practice_full THEN a.run_id END) AS best_practice_runs
        FROM ablation_studies a
        LEFT JOIN context c USING (run_id)
        WHERE a.study_name = ?
          AND a.config_name = ?
          AND a.run_id IS NOT NULL
        """,
        [study_name, config_name],
    ).fetchone()
    total_runs = int(row[0] or 0)
    best_practice_runs = int(row[1] or 0)
    if total_runs == 0:
        raise RuntimeError(
            f"UC study '{study_name}' with config '{config_name}' was not found in DuckDB."
        )
    if best_practice_runs != total_runs:
        raise RuntimeError(
            f"UC study '{study_name}' config '{config_name}' is not fully best-practice: {best_practice_runs}/{total_runs} runs have best_practice_full=TRUE."
        )


def _load_sqlite_polybench_points(
    con: duckdb.DuckDBPyConnection,
    study_name: str,
) -> pd.DataFrame:
    _validate_sqlite_polybench_study(con, study_name)
    df = con.execute(
        """
        WITH scoped AS (
            SELECT
                p."group",
                p.kernel,
                p.kv_s AS size_label,
                p.cpu_time * CASE p.time_unit
                    WHEN 'ns' THEN 1.0
                    WHEN 'us' THEN 1e3
                    WHEN 'ms' THEN 1e6
                    WHEN 's'  THEN 1e9
                END / 1e6 AS jit_ms,
                p.jit_module_instrs,
                p.jit_blob_kb
            FROM v_parsed p
            JOIN ablation_studies a USING (run_id)
            JOIN context c USING (run_id)
            WHERE a.study_name = ?
              AND a.config_name = 'default'
              AND c.best_practice_full
              AND p.phase = 'jit_overhead'
              AND p."group" IN ('polybench', 'db/sqlite3/input_size', 'db/sqlite3/tpch')
        )
        SELECT
            "group",
            kernel,
            COALESCE(size_label, '') AS size_label,
            median(jit_ms) AS median_jit_overhead_ms,
            min(jit_module_instrs) AS min_instrs,
            max(jit_module_instrs) AS max_instrs,
            min(jit_blob_kb) AS min_blob_kb,
            max(jit_blob_kb) AS max_blob_kb,
            count(*) AS row_count
        FROM scoped
        GROUP BY "group", kernel, size_label
        ORDER BY "group", kernel, size_label
        """,
        [study_name],
    ).df()
    if df.empty:
        raise RuntimeError(
            f"Study '{study_name}' has no best-practice SQLite/PolyBench jit_overhead rows."
        )
    if df["min_instrs"].isna().any() or df["min_blob_kb"].isna().any():
        raise RuntimeError(
            f"Study '{study_name}' is missing jit_module_instrs or jit_blob_kb in one or more SQLite/PolyBench rows."
        )
    if not (df["min_instrs"] == df["max_instrs"]).all():
        raise RuntimeError(
            f"Study '{study_name}' contains inconsistent jit_module_instrs values within a SQLite/PolyBench point."
        )
    if not (df["min_blob_kb"] == df["max_blob_kb"]).all():
        raise RuntimeError(
            f"Study '{study_name}' contains inconsistent jit_blob_kb values within a SQLite/PolyBench point."
        )
    df = df.rename(
        columns={
            "min_instrs": "jit_module_instrs",
            "min_blob_kb": "jit_blob_kb",
        }
    )
    df["suite"] = df["group"].map(_suite_label)
    df["abstraction"] = ""
    df["source_study"] = study_name
    df["source_config"] = "default"
    df["point_label"] = df.apply(
        lambda row: (
            f"{row['kernel']} ({row['size_label']})"
            if row["suite"] == "PolyBench"
            else row["kernel"]
        ),
        axis=1,
    )
    return df[
        [
            "suite",
            "group",
            "kernel",
            "abstraction",
            "size_label",
            "source_study",
            "source_config",
            "median_jit_overhead_ms",
            "jit_module_instrs",
            "jit_blob_kb",
            "point_label",
        ]
    ]


def _load_uc_overhead_points(
    con: duckdb.DuckDBPyConnection,
    study_name: str,
    config_name: str,
) -> pd.DataFrame:
    _validate_uc_study(con, study_name, config_name)
    df = con.execute(
        """
        WITH scoped AS (
            SELECT
                p."group",
                p.kernel,
                p.kv_a AS abstraction,
                p.cpu_time * CASE p.time_unit
                    WHEN 'ns' THEN 1.0
                    WHEN 'us' THEN 1e3
                    WHEN 'ms' THEN 1e6
                    WHEN 's'  THEN 1e9
                END / 1e6 AS jit_ms
            FROM v_parsed p
            JOIN ablation_studies a USING (run_id)
            JOIN context c USING (run_id)
            WHERE a.study_name = ?
              AND a.config_name = ?
              AND c.best_practice_full
              AND p.phase = 'jit_overhead'
              AND p."group" IN ('uc1_sql', 'uc2_conv', 'uc7_dfa', 'uc8_ivm', 'uc12_groupby', 'uc14_sort')
              AND p.kv_a IN ('low', 'tradeoff', 'abstract')
        )
        SELECT
            "group",
            kernel,
            abstraction,
            median(jit_ms) AS median_jit_overhead_ms,
            count(*) AS row_count
        FROM scoped
        GROUP BY "group", kernel, abstraction
        ORDER BY "group", kernel, abstraction
        """
        ,
        [study_name, config_name],
    ).df()
    if df.empty:
        raise RuntimeError(
            f"UC study '{study_name}' config '{config_name}' has no best-practice jit_overhead rows."
        )
    if len(df) != 54:
        raise RuntimeError(
            f"Expected 54 UC abstraction-specific points from '{study_name}' / '{config_name}', got {len(df)}."
        )
    df["suite"] = "UC"
    df["size_label"] = ""
    df["source_study"] = study_name
    df["source_config"] = config_name
    df["point_label"] = df.apply(
        lambda row: f"{row['kernel']} ({row['abstraction']})",
        axis=1,
    )
    return df[
        [
            "suite",
            "group",
            "kernel",
            "abstraction",
            "size_label",
            "source_study",
            "source_config",
            "median_jit_overhead_ms",
            "point_label",
        ]
    ]


def _required_tool(name: str) -> str:
    path = shutil.which(name)
    if not path:
        raise RuntimeError(f"Required tool '{name}' was not found in PATH.")
    return path


def _readelf_symbol_size(readelf_bin: str, object_path: Path) -> int:
    proc = subprocess.run(
        [readelf_bin, "-sW", str(object_path)],
        check=True,
        capture_output=True,
        text=True,
    )
    pattern = re.compile(
        rf"^\s*\d+:\s+[0-9A-Fa-f]+\s+(\S+)\s+OBJECT\s+\S+\s+\S+\s+\S+\s+{re.escape(IR_SYMBOL_NAME)}$"
    )
    for line in proc.stdout.splitlines():
        match = pattern.match(line)
        if match:
            return int(match.group(1), 0)
    raise RuntimeError(
        f"Could not locate symbol '{IR_SYMBOL_NAME}' in object '{object_path}'."
    )


def _extract_bitcode_blob(objcopy_bin: str, object_path: Path, tmp_dir: Path) -> Path:
    local_object = tmp_dir / object_path.name
    shutil.copy2(object_path, local_object)
    blob_path = tmp_dir / f"{object_path.stem}.bc"
    subprocess.run(
        [objcopy_bin, f"--dump-section", f"{IR_SECTION_NAME}={blob_path}", str(local_object)],
        check=True,
        capture_output=True,
        text=True,
    )
    if not blob_path.exists():
        raise RuntimeError(
            f"Failed to extract section '{IR_SECTION_NAME}' from '{object_path}'."
        )
    return blob_path


def _normalized_instruction_opcode(line: str) -> str | None:
    candidate = line.strip()
    if not candidate or candidate.startswith(";") or candidate.endswith(":"):
        return None
    if "=" in candidate:
        candidate = candidate.split("=", 1)[1].strip()
    parts = candidate.split()
    while parts and parts[0] in INSTRUCTION_PREFIX_MODIFIERS:
        parts = parts[1:]
    if not parts:
        return None
    return parts[0].rstrip(",")


def _count_instructions_from_ir(ir_text: str) -> tuple[int, int]:
    function_count = 0
    instruction_count = 0
    inside_function = False

    for raw_line in ir_text.splitlines():
        stripped = raw_line.strip()
        if not inside_function:
            if stripped.startswith("define "):
                inside_function = True
                function_count += 1
            continue
        if stripped == "}":
            inside_function = False
            continue
        opcode = _normalized_instruction_opcode(raw_line)
        if opcode in INSTRUCTION_OPCODES:
            instruction_count += 1

    return function_count, instruction_count


def _parse_uc_object_identity(object_path: Path) -> tuple[str, str, str]:
    group = UC_GROUP_BY_DIR.get(object_path.parent.name)
    if not group:
        raise RuntimeError(f"Unsupported UC object directory '{object_path.parent.name}'.")
    match = re.match(r"^UC\d+(.+?)(Low|Tradeoff|Abstract)Kernels$", object_path.stem)
    if not match:
        raise RuntimeError(f"Unsupported UC object filename '{object_path.name}'.")
    kernel = _camel_to_snake(match.group(1))
    abstraction = UC_ABSTRACTION_SUFFIX[match.group(2)]
    return group, kernel, abstraction


def _generate_uc_manifest(release_build: Path, cache_path: Path) -> pd.DataFrame:
    obj_root = release_build / "tools/runtime-specialization/benchmarks/use-cases"
    clang_bin = release_build / "bin/clang"
    if not obj_root.exists():
        raise RuntimeError(f"UC object root does not exist: {obj_root}")
    if not clang_bin.exists():
        raise RuntimeError(f"Release clang was not found: {clang_bin}")

    readelf_bin = _required_tool("readelf")
    objcopy_bin = _required_tool("objcopy")

    rows: list[dict[str, object]] = []
    object_paths = sorted(
        path
        for path in obj_root.glob("UC*/*Kernels.o")
        if re.match(r"^UC\d+.+?(Low|Tradeoff|Abstract)Kernels\.o$", path.name)
    )
    if not object_paths:
        raise RuntimeError(f"No UC abstraction objects were found under '{obj_root}'.")

    for object_path in object_paths:
        group, kernel, abstraction = _parse_uc_object_identity(object_path)
        with tempfile.TemporaryDirectory(prefix="uc-tu-size-") as tmp_dir_name:
            tmp_dir = Path(tmp_dir_name)
            symbol_blob_size = _readelf_symbol_size(readelf_bin, object_path)
            blob_path = _extract_bitcode_blob(objcopy_bin, object_path, tmp_dir)
            actual_blob_size = blob_path.stat().st_size
            if actual_blob_size != symbol_blob_size:
                raise RuntimeError(
                    f"Extracted bitcode size mismatch for '{object_path}': symbol={symbol_blob_size}, extracted={actual_blob_size}."
                )
            ir_proc = subprocess.run(
                [str(clang_bin), "-S", "-emit-llvm", str(blob_path), "-o", "-"],
                check=True,
                capture_output=True,
                text=True,
            )
            function_count, instruction_count = _count_instructions_from_ir(ir_proc.stdout)
            if function_count == 0:
                raise RuntimeError(
                    f"Could not derive any defined functions from extracted bitcode in '{object_path}'."
                )
            rows.append(
                {
                    "group": group,
                    "kernel": kernel,
                    "abstraction": abstraction,
                    "jit_module_instrs": float(instruction_count),
                    "jit_blob_kb": actual_blob_size / 1024.0,
                    "function_count": function_count,
                    "object_path": str(object_path),
                    "release_build": str(release_build.resolve()),
                }
            )

    manifest = pd.DataFrame(rows).sort_values(["group", "kernel", "abstraction"]).reset_index(drop=True)
    if len(manifest) != 54:
        raise RuntimeError(
            f"UC object introspection produced {len(manifest)} abstraction-specific rows; expected 54."
        )
    if manifest.duplicated(["group", "kernel", "abstraction"]).any():
        raise RuntimeError("UC manifest contains duplicate (group, kernel, abstraction) rows.")

    cache_path.parent.mkdir(parents=True, exist_ok=True)
    manifest.to_csv(cache_path, index=False, quoting=csv.QUOTE_MINIMAL)
    return manifest


def _load_or_generate_uc_manifest(release_build: Path, output_dir: Path) -> pd.DataFrame:
    cache_path = output_dir / MANIFEST_CACHE_NAME
    expected_release = str(release_build.resolve())
    if cache_path.exists():
        manifest = pd.read_csv(cache_path)
        required_columns = {
            "group", "kernel", "abstraction", "jit_module_instrs", "jit_blob_kb",
            "function_count", "object_path", "release_build",
        }
        if required_columns.issubset(manifest.columns):
            if len(manifest) == 54 and set(manifest["release_build"]) == {expected_release}:
                return manifest
    return _generate_uc_manifest(release_build, cache_path)


def _build_points_dataframe(args: argparse.Namespace, con: duckdb.DuckDBPyConnection) -> tuple[pd.DataFrame, Path]:
    output_dir = _ensure_output_dir(Path(args.output_dir))
    sqlite_polybench = _load_sqlite_polybench_points(con, args.sqlite_polybench_study_name)
    uc_overhead = _load_uc_overhead_points(con, args.uc_study_name, args.uc_config_name)
    uc_manifest = _load_or_generate_uc_manifest(Path(args.release_build), output_dir)
    uc_points = uc_overhead.merge(
        uc_manifest[["group", "kernel", "abstraction", "jit_module_instrs", "jit_blob_kb"]],
        on=["group", "kernel", "abstraction"],
        how="left",
        validate="one_to_one",
    )
    if uc_points[["jit_module_instrs", "jit_blob_kb"]].isna().any().any():
        raise RuntimeError("UC object introspection did not cover every UC DuckDB median point.")
    if len(uc_points) != 54:
        raise RuntimeError(f"Expected 54 joined UC points, got {len(uc_points)}.")

    points = pd.concat([sqlite_polybench, uc_points], ignore_index=True)
    points["median_jit_overhead_ms"] = points["median_jit_overhead_ms"].astype(float)
    points["jit_module_instrs"] = points["jit_module_instrs"].astype(float)
    points["jit_blob_kb"] = points["jit_blob_kb"].astype(float)
    points["abstraction"] = points["abstraction"].fillna("")
    points["size_label"] = points["size_label"].fillna("")
    points = points.sort_values(
        ["suite", "group", "kernel", "abstraction", "size_label"]
    ).reset_index(drop=True)
    return points, output_dir


def _annotation_indices(points: pd.DataFrame, x_metric: str) -> list[int]:
    picks: list[int] = []
    if points.empty:
        return picks
    selectors = [
        points["median_jit_overhead_ms"].idxmax(),
        points[x_metric].idxmax(),
        points[x_metric].idxmin(),
    ]
    uc_points = points[points["suite"] == "UC"]
    if not uc_points.empty:
        selectors.append(uc_points["median_jit_overhead_ms"].idxmax())
        selectors.append(uc_points[x_metric].idxmax())
    sqlite_tpch = points[points["suite"] == "SQLite TPC-H"]
    if not sqlite_tpch.empty:
        selectors.append(sqlite_tpch["median_jit_overhead_ms"].idxmax())

    seen: set[int] = set()
    for idx in selectors:
        idx = int(idx)
        if idx not in seen:
            seen.add(idx)
            picks.append(idx)
    return picks[:6]


def _format_x_label(x_metric: str) -> str:
    if x_metric == "jit_module_instrs":
        return "JIT Module Size"
    if x_metric == "jit_blob_kb":
        return "IR Blob Size"
    raise RuntimeError(f"Unsupported x metric '{x_metric}'.")


def _select_tick_values(values: pd.Series, candidates: list[float]) -> list[float]:
    lower = float(values.min()) * 0.9
    upper = float(values.max()) * 1.1
    ticks = [tick for tick in candidates if lower <= tick <= upper]
    if not ticks:
        ticks = [min(candidates, key=lambda tick: abs(math.log10(tick) - math.log10(float(values.median()))))]
    return ticks


def _format_ms_tick(value: float) -> str:
    rounded = int(value) if float(value).is_integer() else value
    return f"{rounded:g}ms"


def _format_mib_tick_from_kib(value_kib: float) -> str:
    value_mib = value_kib / 1024.0
    rounded = int(value_mib) if float(value_mib).is_integer() else value_mib
    return f"{rounded:g} MiB"


def _apply_y_ticks(ax, values: pd.Series) -> None:
    candidates = [1, 2, 5, 10, 20, 50, 60, 100, 200, 500, 1000, 2000, 5000]
    ticks = _select_tick_values(values, candidates)
    ax.set_yticks(ticks)
    ax.set_yticklabels([_format_ms_tick(tick) for tick in ticks])
    ax.yaxis.set_major_formatter(mticker.FuncFormatter(lambda value, _: _format_ms_tick(value)))
    ax.yaxis.set_minor_formatter(mticker.NullFormatter())


def _apply_x_ticks(ax, values: pd.Series, x_metric: str) -> None:
    if x_metric != "jit_blob_kb":
        return
    candidates_kib = [
        1 * 1024.0,
        10 * 1024.0,
        60 * 1024.0,
        100 * 1024.0,
    ]
    ticks = _select_tick_values(values, candidates_kib)
    ax.set_xticks(ticks)
    ax.set_xticklabels([_format_mib_tick_from_kib(tick) for tick in ticks])
    ax.xaxis.set_minor_formatter(mticker.NullFormatter())


def _plot(points: pd.DataFrame, output_dir: Path, x_metric: str) -> Path:
    points = points[points["suite"] != "SQLite input-size"].copy()
    if (points[x_metric] <= 0).any():
        raise RuntimeError(f"Metric '{x_metric}' contains non-positive values and cannot be plotted on a log axis.")
    if (points["median_jit_overhead_ms"] <= 0).any():
        raise RuntimeError("Median JIT overhead contains non-positive values and cannot be plotted on a log axis.")

    fig, ax = plt.subplots(figsize=(9.5, 6.2), constrained_layout=True)

    for suite, suite_points in points.groupby("suite", sort=False):
        ax.scatter(
            suite_points[x_metric],
            suite_points["median_jit_overhead_ms"],
            s=42,
            marker=SUITE_MARKERS[suite],
            color=SUITE_COLORS[suite],
            alpha=0.86,
            linewidths=0.5,
            edgecolors="white",
            zorder=3,
        )

    for idx in _annotation_indices(points, x_metric):
        row = points.loc[idx]
        ax.annotate(
            row["point_label"],
            (row[x_metric], row["median_jit_overhead_ms"]),
            xytext=(5, 5),
            textcoords="offset points",
            fontsize=8,
            color="#222222",
        )

    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlabel(_format_x_label(x_metric))
    ax.set_ylabel("Median JIT specialization time (J)")
    _apply_x_ticks(ax, points[x_metric], x_metric)
    _apply_y_ticks(ax, points["median_jit_overhead_ms"])
    ax.grid(True, which="both", linewidth=0.5, alpha=0.28)

    suite_handles = [
        Line2D(
            [0], [0],
            marker=SUITE_MARKERS[suite],
            linestyle="none",
            markerfacecolor=SUITE_COLORS[suite],
            markeredgecolor="white",
            markeredgewidth=0.6,
            markersize=7,
            label=suite,
        )
        for suite in ("UC", "PolyBench", "SQLite input-size", "SQLite TPC-H")
        if suite in set(points["suite"])
    ]
    ax.legend(handles=suite_handles, loc="upper left", title="Suite", frameon=True)

    output_path = output_dir / PLOT_NAMES[x_metric]
    fig.savefig(output_path, dpi=200, bbox_inches="tight")
    plt.close(fig)
    return output_path


def main() -> None:
    args = _parse_args()
    db_path = resolve_db_path(args.db)
    con = open_db(db_path)
    try:
        points, output_dir = _build_points_dataframe(args, con)
    finally:
        con.close()

    csv_path = output_dir / POINTS_CSV_NAME
    points[
        [
            "suite",
            "group",
            "kernel",
            "abstraction",
            "size_label",
            "source_study",
            "source_config",
            "median_jit_overhead_ms",
            "jit_module_instrs",
            "jit_blob_kb",
        ]
    ].to_csv(csv_path, index=False, quoting=csv.QUOTE_MINIMAL)

    metrics = ["jit_module_instrs", "jit_blob_kb"] if args.x_metric == "both" else [args.x_metric]
    plot_paths = [_plot(points, output_dir, metric) for metric in metrics]

    uc_count = int((points["suite"] == "UC").sum())
    if uc_count != 54:
        raise RuntimeError(f"Expected 54 UC points in exported data, got {uc_count}.")

    print(f"Points CSV: {csv_path}")
    print(f"UC points: {uc_count}")
    for plot_path in plot_paths:
        print(f"Plot: {plot_path}")
    print(f"UC study/config: {args.uc_study_name} / {args.uc_config_name}")
    print(f"SQLite/PolyBench study: {args.sqlite_polybench_study_name}")
    print(f"UC manifest cache: {output_dir / MANIFEST_CACHE_NAME}")


if __name__ == "__main__":
    main()
