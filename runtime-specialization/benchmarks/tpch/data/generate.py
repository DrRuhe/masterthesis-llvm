#!/usr/bin/env python3
"""Generate a TPC-H SQLite database using DuckDB's built-in tpch extension.

Usage:
    python3 generate.py [--sf=0.1] [--out=tpch.db]

Requirements:
    pip install duckdb
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


def generate(sf: float, out_path: str) -> None:
    print(f"Generating TPC-H data at scale factor {sf} -> {out_path}")

    # Generate data in DuckDB
    duck = duckdb.connect()
    duck.execute("INSTALL tpch; LOAD tpch")
    duck.execute(f"CALL dbgen(sf={sf})")

    # Create / reset SQLite database
    if os.path.exists(out_path):
        os.remove(out_path)
    sq = sqlite3.connect(out_path)
    sq.execute("PRAGMA journal_mode = OFF")
    sq.execute("PRAGMA synchronous  = OFF")
    sq.execute("PRAGMA cache_size   = -65536")

    # Apply schema
    with open(SCHEMA_SQL) as f:
        sq.executescript(f.read())

    from decimal import Decimal

    def coerce(v):
        """Convert DuckDB-specific types to SQLite-compatible Python types."""
        if isinstance(v, Decimal):
            return float(v)
        return v

    # Transfer data table by table
    for table in TABLES:
        rows = duck.execute(f"SELECT * FROM {table}").fetchall()
        if not rows:
            continue
        ncols = len(rows[0])
        placeholders = ",".join(["?"] * ncols)
        rows = [tuple(coerce(v) for v in row) for row in rows]
        sq.executemany(f"INSERT INTO {table} VALUES ({placeholders})", rows)
        print(f"  {table}: {len(rows):,} rows")

    sq.commit()
    sq.close()
    duck.close()
    size_mb = os.path.getsize(out_path) / (1024 * 1024)
    print(f"Done. Database size: {size_mb:.1f} MB")


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate TPC-H SQLite database")
    parser.add_argument("--sf",  type=float, default=0.1,
                        help="Scale factor (default: 0.1 ≈ 100 MB)")
    parser.add_argument("--out", default="tpch.db",
                        help="Output SQLite file path (default: tpch.db)")
    args = parser.parse_args()
    generate(args.sf, args.out)


if __name__ == "__main__":
    main()
