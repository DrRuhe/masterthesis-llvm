#!/usr/bin/env python3
"""Generate TPC-H databases (SQLite and DuckDB) using DuckDB's built-in tpch extension.

Usage:
    python3 generate.py [--sf=0.1] [--out=tpch.db] [--duckdb-out=tpch.duckdb]

Note: --out is the SQLite TPC-H query database (not the benchmark results database).
      --duckdb-out is the DuckDB TPC-H query database (also not the benchmark results database).
      The benchmark results database (benchmarks.duckdb) is managed by record_benchmark.py.

Requirements:
    pip install duckdb  (nixpkgs devshell provides this; version should be ≤ 1.5.2 to match the C++ DuckDB)
"""
import argparse
import os
import sqlite3
import sys

try:
    import duckdb
except ImportError:
    print("Error: duckdb not installed. Run: pip install duckdb", file=sys.stderr)
    sys.exit(1)


TABLES = [
    "region", "nation", "part", "supplier",
    "partsupp", "customer", "orders", "lineitem",
]

SCHEMA_SQL = os.path.join(os.path.dirname(__file__), "tpch_schema.sql")


def generate(sf: float, out_path: str, duckdb_out_path: str) -> None:
    print(f"Generating TPC-H data at scale factor {sf}")

    # Generate data in DuckDB (in-memory, used as source for SQLite transfer)
    duck = duckdb.connect()
    duck.execute("INSTALL tpch; LOAD tpch")
    duck.execute(f"CALL dbgen(sf={sf})")

    # ── SQLite output ──────────────────────────────────────────────────────────
    print(f"  -> SQLite TPC-H database: {out_path}")
    if os.path.exists(out_path):
        os.remove(out_path)
    sq = sqlite3.connect(out_path)
    sq.execute("PRAGMA journal_mode = OFF")
    sq.execute("PRAGMA synchronous  = OFF")
    sq.execute("PRAGMA cache_size   = -65536")

    with open(SCHEMA_SQL) as f:
        sq.executescript(f.read())

    from decimal import Decimal

    def coerce(v):
        """Convert DuckDB-specific types to SQLite-compatible Python types."""
        if isinstance(v, Decimal):
            return float(v)
        return v

    for table in TABLES:
        rows = duck.execute(f"SELECT * FROM {table}").fetchall()
        if not rows:
            continue
        ncols = len(rows[0])
        placeholders = ",".join(["?"] * ncols)
        rows = [tuple(coerce(v) for v in row) for row in rows]
        sq.executemany(f"INSERT INTO {table} VALUES ({placeholders})", rows)
        print(f"    {table}: {len(rows):,} rows")

    sq.commit()
    sq.close()
    duck.close()
    size_mb = os.path.getsize(out_path) / (1024 * 1024)
    print(f"  SQLite done. Size: {size_mb:.1f} MB")

    # ── DuckDB output ──────────────────────────────────────────────────────────
    # Use the same Python duckdb package for the DuckDB file so the file format
    # matches the C++ DuckDB library version in tpch/CMakeLists.txt.
    # The benchmark binary uses DuckDB v1.0.0 (see CMakeLists.txt comment for
    # why v1.1.0+ is not used). Python duckdb must match: pip install duckdb==1.0.0.
    print(f"  -> DuckDB TPC-H database: {duckdb_out_path}")
    if os.path.exists(duckdb_out_path):
        os.remove(duckdb_out_path)
    duck_file = duckdb.connect(duckdb_out_path)
    duck_file.execute("INSTALL tpch; LOAD tpch")
    duck_file.execute(f"CALL dbgen(sf={sf})")
    for table in TABLES:
        count = duck_file.execute(f"SELECT COUNT(*) FROM {table}").fetchone()[0]
        print(f"    {table}: {count:,} rows")
    duck_file.close()
    size_mb = os.path.getsize(duckdb_out_path) / (1024 * 1024)
    print(f"  DuckDB done. Size: {size_mb:.1f} MB")

    print("Done.")


def main() -> None:
    parser = argparse.ArgumentParser(
        description=(
            "Generate TPC-H databases for benchmarking. "
            "Produces a SQLite TPC-H query database (--out) and a DuckDB TPC-H query database "
            "(--duckdb-out). Neither file is the benchmark results database (benchmarks.duckdb)."
        )
    )
    parser.add_argument("--sf", type=float, default=0.1,
                        help="Scale factor (default: 0.1 ≈ 100 MB)")
    parser.add_argument("--out", default="tpch.db",
                        help="Output path for the SQLite TPC-H query database (default: tpch.db). "
                             "This is NOT the benchmark results database.")
    parser.add_argument("--duckdb-out", default="tpch.duckdb",
                        help="Output path for the DuckDB TPC-H query database (default: tpch.duckdb). "
                             "This is NOT the benchmark results database.")
    args = parser.parse_args()
    generate(args.sf, args.out, args.duckdb_out)


if __name__ == "__main__":
    main()
