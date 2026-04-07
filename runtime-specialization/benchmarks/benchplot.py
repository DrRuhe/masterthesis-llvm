#!/usr/bin/env python3
"""benchplot.py — Run Google Benchmark binaries and plot specialization overhead."""

import argparse
import hashlib
import json
import os
import re
import subprocess
import sys
import tempfile
import uuid
from datetime import datetime
from pathlib import Path

import duckdb
import numpy as np
import pandas as pd
import ultraplot as uplt

# ---------------------------------------------------------------------------
# Schema
# ---------------------------------------------------------------------------

SCHEMA = """
CREATE TABLE IF NOT EXISTS benchmark_results (
    run_id       VARCHAR NOT NULL,
    run_ts       TIMESTAMP NOT NULL,
    binary_path  VARCHAR NOT NULL,
    binary_hash  VARCHAR NOT NULL,
    name         VARCHAR NOT NULL,
    real_time    DOUBLE NOT NULL,
    cpu_time     DOUBLE NOT NULL,
    time_unit    VARCHAR NOT NULL,
    iterations   BIGINT NOT NULL,
    PRIMARY KEY (run_id, name)
);
"""

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

TIME_UNITS = {"ns": 1e-9, "us": 1e-6, "ms": 1e-3, "s": 1.0}


def sha256_file(path: str) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def parse_name(name: str):
    """Parse a Google Benchmark name into (phase, kernel, params, group_key)."""
    m = re.match(
        r"^BM_(unspecialized|jit_overhead|specialized_exec)_(.+?)(/.*)?$",
        name,
    )
    if not m:
        return None
    phase = m.group(1)
    kernel = m.group(2)
    raw_params = m.group(3) or ""
    # Keep only purely numeric segments (drops e.g. "min_warmup_time:1.000")
    numeric_parts = [p for p in raw_params.split("/") if p.isdigit()]
    params = ",".join(numeric_parts)
    group_key = f"{kernel}/{params}" if params else kernel
    return phase, kernel, params, group_key


def convert_time(value: float, from_unit: str, to_unit: str) -> float:
    return value * TIME_UNITS[from_unit] / TIME_UNITS[to_unit]


def auto_unit(median_ns: float) -> str:
    """Pick a human-friendly time unit based on median time in nanoseconds."""
    if median_ns >= 1e9:
        return "s"
    if median_ns >= 1e6:
        return "ms"
    if median_ns >= 1e3:
        return "us"
    return "ns"


# ---------------------------------------------------------------------------
# Run / Load
# ---------------------------------------------------------------------------

def run_binary(binary: str) -> list[dict]:
    """Run the benchmark binary and return parsed JSON benchmarks list."""
    with tempfile.NamedTemporaryFile(suffix=".json", delete=False) as tmp:
        out_path = tmp.name
    cmd = [
        binary,
        "--benchmark_out_format=json",
        f"--benchmark_out={out_path}",
    ]
    print(f"Running: {' '.join(cmd)}")
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(result.stderr, file=sys.stderr)
        sys.exit(1)
    with open(out_path) as f:
        data = json.load(f)
    return data.get("benchmarks", [])


def load_json(path: str) -> list[dict]:
    with open(path) as f:
        data = json.load(f)
    return data.get("benchmarks", [])


# ---------------------------------------------------------------------------
# DuckDB
# ---------------------------------------------------------------------------

def open_db(db_path: str) -> duckdb.DuckDBPyConnection:
    con = duckdb.connect(db_path)
    con.execute(SCHEMA)
    return con


def store_results(con, benchmarks: list[dict], binary_path: str, binary_hash: str):
    run_id = str(uuid.uuid4())
    run_ts = datetime.now()
    rows = []
    for b in benchmarks:
        rows.append((
            run_id,
            run_ts,
            binary_path,
            binary_hash,
            b["name"],
            b["real_time"],
            b["cpu_time"],
            b["time_unit"],
            int(b["iterations"]),
        ))
    con.executemany(
        """
        INSERT OR REPLACE INTO benchmark_results
            (run_id, run_ts, binary_path, binary_hash, name,
             real_time, cpu_time, time_unit, iterations)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        """,
        rows,
    )
    print(f"Stored {len(rows)} rows (run_id={run_id})")


def load_from_db(con, binary_hash: str) -> pd.DataFrame:
    return con.execute(
        """
        SELECT r.*
        FROM benchmark_results r
        INNER JOIN (
            SELECT name, MAX(run_ts) AS latest_ts
            FROM benchmark_results
            WHERE binary_hash = ?
            GROUP BY name
        ) latest ON r.name = latest.name AND r.run_ts = latest.latest_ts
        WHERE r.binary_hash = ?
        """,
        [binary_hash, binary_hash],
    ).df()


def benchmarks_to_df(benchmarks: list[dict]) -> pd.DataFrame:
    rows = [
        {
            "name": b["name"],
            "real_time": b["real_time"],
            "cpu_time": b["cpu_time"],
            "time_unit": b["time_unit"],
            "iterations": int(b["iterations"]),
        }
        for b in benchmarks
    ]
    return pd.DataFrame(rows)


# ---------------------------------------------------------------------------
# Processing
# ---------------------------------------------------------------------------

def process(df: pd.DataFrame, kernel_filter: str | None, time_unit: str | None):
    """Parse names, filter, normalize times, compute bar heights."""
    # Parse names
    parsed = df["name"].apply(parse_name)
    mask = parsed.notna()
    df = df[mask].copy()
    parsed = parsed[mask]

    df["phase"] = parsed.apply(lambda x: x[0])
    df["kernel"] = parsed.apply(lambda x: x[1])
    df["params"] = parsed.apply(lambda x: x[2])
    df["group_key"] = parsed.apply(lambda x: x[3])

    # Apply filter
    if kernel_filter:
        pat = re.compile(kernel_filter, re.IGNORECASE)
        df = df[df["kernel"].apply(lambda k: bool(pat.search(k)))]

    if df.empty:
        print("No benchmarks match the filter.", file=sys.stderr)
        sys.exit(1)

    # Normalize to a common time unit
    # First convert everything to nanoseconds
    df["real_time_ns"] = df.apply(
        lambda r: convert_time(r["real_time"], r["time_unit"], "ns"), axis=1
    )

    if time_unit:
        target_unit = time_unit
    else:
        median_ns = df[df["phase"] == "unspecialized"]["real_time_ns"].median()
        target_unit = auto_unit(median_ns)

    df["real_time_norm"] = df.apply(
        lambda r: convert_time(r["real_time"], r["time_unit"], target_unit), axis=1
    )

    return df, target_unit


def compute_bars(df: pd.DataFrame):
    """Return group labels and arrays of bar heights (ratios relative to unspecialized)."""
    groups = df["group_key"].unique()
    # Preserve order by first appearance
    seen = {}
    for gk in df["group_key"]:
        if gk not in seen:
            seen[gk] = len(seen)
    groups = sorted(groups, key=lambda g: seen[g])

    h_unspec = []
    h_spec = []
    h_jit = []
    h_spec_jit = []
    valid_groups = []

    for gk in groups:
        sub = df[df["group_key"] == gk]

        def get_time(phase):
            rows = sub[sub["phase"] == phase]["real_time_norm"]
            return rows.iloc[0] if not rows.empty else None

        t_u = get_time("unspecialized")
        t_s = get_time("specialized_exec")
        t_j = get_time("jit_overhead")

        if t_u is None or t_u == 0:
            continue

        valid_groups.append(gk)
        h_unspec.append(1.0)
        h_spec.append(t_s / t_u if t_s is not None else float("nan"))
        h_jit.append(t_j / t_u if t_j is not None else float("nan"))
        h_spec_jit.append(
            (t_s + t_j) / t_u if (t_s is not None and t_j is not None) else float("nan")
        )

    return valid_groups, np.array(h_unspec), np.array(h_spec), np.array(h_jit), np.array(h_spec_jit)


# ---------------------------------------------------------------------------
# Plotting
# ---------------------------------------------------------------------------

def plot(groups, h_unspec, h_spec, h_jit, h_spec_jit, time_unit, title, output_path):
    n = len(groups)
    width = max(8, n * 1.4)
    fig, ax = uplt.subplots(figsize=(width, 4))

    x = np.arange(n)
    w = 0.18

    ax.bar(x - 1.5 * w, h_unspec,   w, label="Unspecialized",   color="gray7")
    ax.bar(x - 0.5 * w, h_spec,     w, label="Specialized",     color="green7")
    ax.bar(x + 0.5 * w, h_jit,      w, label="JIT Overhead",    color="orange7")
    ax.bar(x + 1.5 * w, h_spec_jit, w, label="Specialized + JIT", color="blue7")

    ax.axhline(1.0, color="k", ls="--", lw=0.8)

    ax.format(
        xlocator=x,
        xformatter=[str(g) for g in groups],
        ylabel=f"Normalized Time ({time_unit}, lower = faster)",
        yscale="log",
        title=title,
    )
    ax.tick_params(axis="x", labelrotation=45)
    ax.legend(loc="b")

    fig.save(output_path)
    print(f"Saved chart to {output_path}")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="Run benchmarks and plot specialization overhead.")
    src = parser.add_mutually_exclusive_group()
    src.add_argument("--binary", help="Benchmark executable to run")
    src.add_argument("--json", dest="json_path", help="Load existing benchmark JSON output")
    parser.add_argument("--db", default=None, help="DuckDB cache file")
    parser.add_argument("--no-run", action="store_true", help="Skip running; load from DB only")
    parser.add_argument("--filter", dest="kernel_filter", help="Regex filter on kernel name", default=None)
    parser.add_argument("--output", default="benchmarks.pdf", help="Output chart path")
    parser.add_argument("--time-unit", choices=["ns", "us", "ms", "s"], help="Time unit override")
    args = parser.parse_args()

    env_db = os.environ.get("BENCHPLOT_DB_PATH")
    if env_db:
        db_path = env_db
    else:
        db_path = args.db or "benchmarks.duckdb"
        if not Path(db_path).exists():
            print(f"Warning: '{db_path}' not found in CWD.", file=sys.stderr)
            if not args.binary and not args.json_path:
                print("Error: No data source available. Provide --binary or --json, "
                      "or set BENCHPLOT_DB_PATH.", file=sys.stderr)
                sys.exit(1)
    con = open_db(db_path)

    df = None
    binary_hash = None

    if args.no_run:
        if not args.binary:
            parser.error("--no-run requires --binary to identify the cache entry")
        binary_hash = sha256_file(args.binary)
        df = load_from_db(con, binary_hash)
        if df.empty:
            print("No cached results found for this binary.", file=sys.stderr)
            sys.exit(1)
        print(f"Loaded {len(df)} rows from cache.")

    elif args.json_path:
        benchmarks = load_json(args.json_path)
        df = benchmarks_to_df(benchmarks)

    elif args.binary:
        binary_hash = sha256_file(args.binary)
        # Check cache
        cached = load_from_db(con, binary_hash)
        if not cached.empty:
            print(f"Cache hit: loaded {len(cached)} rows from DB.")
            df = cached
        else:
            benchmarks = run_binary(args.binary)
            df = benchmarks_to_df(benchmarks)
            store_results(con, benchmarks, str(Path(args.binary).resolve()), binary_hash)

    else:
        parser.error("Provide --binary, --json, or --no-run with --binary")

    df, target_unit = process(df, args.kernel_filter, args.time_unit)
    groups, h_unspec, h_spec, h_jit, h_spec_jit = compute_bars(df)

    filter_label = args.kernel_filter or "All Kernels"
    title = f"Specialization Overhead — {filter_label}"
    plot(groups, h_unspec, h_spec, h_jit, h_spec_jit, target_unit, title, args.output)


if __name__ == "__main__":
    main()
