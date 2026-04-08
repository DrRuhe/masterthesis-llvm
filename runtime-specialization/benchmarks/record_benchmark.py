#!/usr/bin/env python3
"""record_benchmark.py — Record Google Benchmark runs into DuckDB.

Usage:
    record_benchmark.py <binary> [--benchmark-filter=PATTERN] [--db=PATH]
"""

import argparse
import json
import os
import shutil
import subprocess
import sys
import tempfile
import uuid
from datetime import datetime
from pathlib import Path

import duckdb

# ---------------------------------------------------------------------------
# Schema
# ---------------------------------------------------------------------------

_SCHEMA_CONTEXT = """
CREATE TABLE IF NOT EXISTS context (
    run_id              VARCHAR PRIMARY KEY,
    run_ts              TIMESTAMP NOT NULL,
    git_sha             VARCHAR,
    date                VARCHAR,
    host_name           VARCHAR,
    executable          VARCHAR,
    num_cpus            INTEGER,
    mhz_per_cpu         INTEGER,
    cpu_scaling_enabled BOOLEAN,
    library_version     VARCHAR,
    library_build_type  VARCHAR
);
"""

# Base columns that are always present in Google Benchmark JSON output.
# Dynamic columns (perf counters, custom counters, etc.) are added on demand.
_SCHEMA_BENCHMARKS = """
CREATE TABLE IF NOT EXISTS benchmarks (
    run_id                     VARCHAR NOT NULL REFERENCES context(run_id),
    name                       VARCHAR NOT NULL,
    family_index               INTEGER,
    per_family_instance_index  INTEGER,
    run_type                   VARCHAR,
    repetitions                INTEGER,
    repetition_index           INTEGER,
    threads                    INTEGER,
    iterations                 BIGINT,
    real_time                  DOUBLE,
    cpu_time                   DOUBLE,
    time_unit                  VARCHAR,
    PRIMARY KEY (run_id, name)
);
"""

# Parse phase, kernel, raw_params out of the benchmark name.
_SCHEMA_V_PARSED = r"""
CREATE OR REPLACE VIEW v_parsed AS
SELECT
    b.*,
    c.git_sha,
    c.run_ts,
    c.host_name,
    regexp_extract(b.name,
        '^BM_(unspecialized|jit_overhead|specialized_exec)_+(.+?)(/\d.*)?$', 1) AS phase,
    regexp_extract(b.name,
        '^BM_(unspecialized|jit_overhead|specialized_exec)_+(.+?)(/\d.*)?$', 2) AS kernel,
    regexp_extract(b.name,
        '^BM_(unspecialized|jit_overhead|specialized_exec)_+(.+?)((/\d+)+)?$', 3) AS raw_params
FROM benchmarks b
JOIN context c USING (run_id);
"""

# Convert all times to nanoseconds; drop rows that didn't match the naming convention.
_SCHEMA_V_NS = """
CREATE OR REPLACE VIEW v_ns AS
SELECT *,
    real_time * CASE time_unit
        WHEN 'ns' THEN 1.0
        WHEN 'us' THEN 1e3
        WHEN 'ms' THEN 1e6
        WHEN 's'  THEN 1e9
    END AS real_time_ns
FROM v_parsed
WHERE phase != '';
"""

# Pivot phases per (run_id, kernel, raw_params) for ratio computation.
_SCHEMA_V_RATIOS = """
CREATE OR REPLACE VIEW v_ratios AS
SELECT
    run_id,
    kernel,
    raw_params,
    git_sha,
    run_ts,
    host_name,
    MAX(CASE WHEN phase = 'unspecialized'    THEN real_time_ns END) AS t_unspec_ns,
    MAX(CASE WHEN phase = 'specialized_exec' THEN real_time_ns END) AS t_spec_ns,
    MAX(CASE WHEN phase = 'jit_overhead'     THEN real_time_ns END) AS t_jit_ns
FROM v_ns
GROUP BY run_id, kernel, raw_params, git_sha, run_ts, host_name;
"""

# Fixed column names (lowercase) that are part of the base schema.
_BASE_COLUMNS = {
    "run_id", "name", "family_index", "per_family_instance_index",
    "run_type", "repetitions", "repetition_index", "threads",
    "iterations", "real_time", "cpu_time", "time_unit",
}


# ---------------------------------------------------------------------------
# Dynamic column helpers
# ---------------------------------------------------------------------------

def _col_name(key: str) -> str:
    """Normalize a JSON key to a valid SQL column name."""
    return key.replace("-", "_").replace(".", "_")


def _infer_sql_type(val) -> str:
    if isinstance(val, bool):
        return "BOOLEAN"
    if isinstance(val, int):
        return "BIGINT"
    if isinstance(val, float):
        return "DOUBLE"
    return "VARCHAR"


def ensure_columns(con: duckdb.DuckDBPyConnection, benchmarks: list) -> None:
    """Add any JSON keys missing from the benchmarks table as new columns (NULL default)."""
    existing = {
        row[0].lower()
        for row in con.execute("DESCRIBE benchmarks").fetchall()
    }
    for b in benchmarks:
        for key, val in b.items():
            col = _col_name(key)
            if col.lower() in existing:
                continue
            if val is None:
                continue  # defer until we see an actual value for type inference
            dtype = _infer_sql_type(val)
            con.execute(f'ALTER TABLE benchmarks ADD COLUMN "{col}" {dtype}')
            existing.add(col.lower())


def insert_benchmarks(con: duckdb.DuckDBPyConnection, run_id: str, benchmarks: list) -> None:
    """Insert all benchmark rows, mapping JSON keys to column names dynamically."""
    cols_in_db = {
        row[0].lower(): row[0]  # lower -> actual case
        for row in con.execute("DESCRIBE benchmarks").fetchall()
    }

    for b in benchmarks:
        row_data: dict = {"run_id": run_id}
        for key, val in b.items():
            col = _col_name(key)
            if col.lower() in cols_in_db:
                row_data[col] = val

        col_list = ", ".join(f'"{c}"' for c in row_data)
        placeholders = ", ".join(["?"] * len(row_data))
        con.execute(
            f"INSERT INTO benchmarks ({col_list}) VALUES ({placeholders})",
            list(row_data.values()),
        )


# ---------------------------------------------------------------------------
# DB helpers
# ---------------------------------------------------------------------------

def resolve_db_path(flag_value: str | None) -> Path:
    """Resolve DB path: --db flag > BENCHPLOT_DB_PATH env var > CWD/benchmarks.duckdb."""
    if flag_value is not None:
        return Path(flag_value)
    env = os.environ.get("BENCHPLOT_DB_PATH")
    if env:
        return Path(env)
    return Path.cwd() / "benchmarks.duckdb"


def open_db(db_path: Path, create: bool) -> duckdb.DuckDBPyConnection:
    if not create and not db_path.exists():
        print(f"Error: DB file not found: {db_path}", file=sys.stderr)
        print("Pass --create-db to initialise a new database.", file=sys.stderr)
        sys.exit(1)
    con = duckdb.connect(str(db_path))
    for stmt in [
        _SCHEMA_CONTEXT,
        _SCHEMA_BENCHMARKS,
        _SCHEMA_V_PARSED,
        _SCHEMA_V_NS,
        _SCHEMA_V_RATIOS,
    ]:
        con.execute(stmt)
    return con


# ---------------------------------------------------------------------------
# CPU scaling helpers
# ---------------------------------------------------------------------------

_GOV_DIR = Path("/sys/devices/system/cpu/cpu0/cpufreq")
_GOV_FILE = _GOV_DIR / "scaling_governor"


def _set_governor(gov: str) -> None:
    if shutil.which("cpupower"):
        subprocess.run(["sudo", "cpupower", "frequency-set", "-g", gov],
                       check=True, capture_output=True)
    else:
        for f in Path("/sys/devices/system/cpu").glob("cpu*/cpufreq/scaling_governor"):
            subprocess.run(["sudo", "tee", str(f)],
                           input=gov, text=True, check=True, capture_output=True)


def run_with_cpu_management(cmd: list, best_practice: bool) -> subprocess.CompletedProcess:
    """Run cmd, optionally pinning the CPU governor to 'performance' first."""
    if not best_practice or not _GOV_FILE.exists():
        if best_practice and not _GOV_FILE.exists():
            print("Warning: CPU frequency scaling not available — running benchmark as-is.",
                  file=sys.stderr)
        return subprocess.run(cmd)

    original_gov = _GOV_FILE.read_text().strip()
    if original_gov == "performance":
        print("CPU governor already set to: performance", file=sys.stderr)
        return subprocess.run(cmd)

    print(f"Setting CPU governor to: performance (was: {original_gov})", file=sys.stderr)
    _set_governor("performance")
    try:
        return subprocess.run(cmd)
    finally:
        print(f"Restoring CPU governor to: {original_gov}", file=sys.stderr)
        _set_governor(original_gov)


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def get_git_sha() -> str:
    try:
        result = subprocess.run(
            ["git", "rev-parse", "HEAD"],
            capture_output=True, text=True, check=True,
        )
        return result.stdout.strip()
    except Exception:
        return ""


# ---------------------------------------------------------------------------
# Commands
# ---------------------------------------------------------------------------

def store_to_db(args, data: dict, json_path: str, delete_on_success: bool) -> None:
    ctx = data.get("context", {})
    benchmarks = data.get("benchmarks", [])
    db_path = resolve_db_path(args.db)
    try:
        con = open_db(db_path, args.create_db)
        run_id = str(uuid.uuid4())
        run_ts = datetime.now()

        con.execute(
            "INSERT INTO context VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
            [
                run_id, run_ts,
                ctx.get("git_sha", ""),
                ctx.get("date", ""),
                ctx.get("host_name", ""),
                ctx.get("executable", ""),
                ctx.get("num_cpus"),
                ctx.get("mhz_per_cpu"),
                ctx.get("cpu_scaling_enabled"),
                ctx.get("library_version", ""),
                ctx.get("library_build_type", ""),
            ],
        )

        ensure_columns(con, benchmarks)
        insert_benchmarks(con, run_id, benchmarks)

        print(f"run_id: {run_id}")
        print(f"Stored context + {len(benchmarks)} benchmark rows.")
        if delete_on_success:
            os.unlink(json_path)
    except Exception:
        import traceback
        traceback.print_exc()
        print(file=sys.stderr)
        print(f"ERROR while adding data to DB. The output from this benchmark run is available at {json_path}", file=sys.stderr)
        print(file=sys.stderr)
        print(f"Please fix the importing issues and run:", file=sys.stderr)
        print(f"  record_benchmark.py --record-json {json_path}", file=sys.stderr)
        sys.exit(1)


def cmd_record(args):
    sha = get_git_sha()

    with tempfile.NamedTemporaryFile(suffix=".json", delete=False) as tmp:
        out_path = tmp.name

    flags = [
        "--benchmark_out_format=json",
        f"--benchmark_out={out_path}",
        f"--benchmark_context=git_sha={sha}",
        (
            "--benchmark_perf_counters="
            "instructions,cpu-cycles,branch-misses,"
            "L1-icache-load-misses,L1-icache-loads,iTLB-load-misses"
        ),
    ]
    if args.benchmark_filter:
        flags.append(f"--benchmark_filter={args.benchmark_filter}")

    binary = str(Path(args.binary).resolve())
    cmd = [binary] + flags
    print(f"Running: {' '.join(cmd)}", flush=True)
    result = run_with_cpu_management(cmd, args.benchmarking_best_practice)
    if result.returncode != 0:
        sys.exit(result.returncode)

    with open(out_path) as f:
        data = json.load(f)

    store_to_db(args, data, out_path, delete_on_success=True)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Record Google Benchmark runs into DuckDB."
    )
    parser.add_argument("binary", nargs="?", default=None,
                        help="Benchmark executable to run.")
    parser.add_argument(
        "--record-json", metavar="PATH",
        help="Import benchmark results from a previously saved JSON file instead of running a binary.",
    )
    parser.add_argument(
        "--benchmark_filter", metavar="PATTERN",
        help="Passed as --benchmark_filter to the binary.",
    )
    parser.add_argument(
        "--benchmarking-best-practice", action="store_true",
        help="Pin CPU governor to 'performance' for the duration of the run (requires sudo if the CPU govenor must be adjusted).",
    )
    parser.add_argument(
        "--db", default=None, metavar="PATH",
        help="DuckDB file path (overrides BENCHPLOT_DB_PATH env var and CWD default).",
    )
    parser.add_argument(
        "--create-db", action="store_true",
        help="Create the database file if it does not exist.",
    )

    args = parser.parse_args()

    if args.record_json and args.binary:
        parser.error("--record-json and binary are mutually exclusive.")
    if not args.record_json and not args.binary:
        parser.error("Provide a binary to run, or use --record-json to import existing results.")

    if args.record_json:
        with open(args.record_json) as f:
            data = json.load(f)
        store_to_db(args, data, args.record_json, delete_on_success=False)
    else:
        cmd_record(args)


if __name__ == "__main__":
    main()
