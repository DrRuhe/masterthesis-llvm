#!/usr/bin/env python3
"""Shared utilities for benchmark reporting scripts."""

import argparse
import os
import re
import stat
import sys
import textwrap
from datetime import datetime
from pathlib import Path

import duckdb
import pandas as pd

# ── DB resolution ──────────────────────────────────────────────────────────────

def resolve_db_path(cli_path: str | None) -> Path:
    """Resolve DuckDB path: --db flag → BENCHPLOT_DB_PATH env var → ./benchmarks.duckdb"""
    if cli_path:
        return Path(cli_path)
    env = os.environ.get("BENCHPLOT_DB_PATH")
    if env:
        return Path(env)
    return Path.cwd() / "benchmarks.duckdb"


def open_db(path: Path, read_only: bool = True) -> duckdb.DuckDBPyConnection:
    if not path.exists():
        sys.exit(f"Database not found: {path}")
    con = duckdb.connect(str(path), read_only=read_only)
    if not read_only:
        return con
    # Probe for stale views (b.* expansion changes when columns are added dynamically).
    # If detected, reopen read-write, refresh, then return a fresh read-only handle.
    try:
        con.execute("SELECT * FROM v_parsed LIMIT 0")
    except duckdb.BinderException:
        con.close()
        _refresh_db_views(path)
        con = duckdb.connect(str(path), read_only=True)
    return con


def _refresh_db_views(path: Path) -> None:
    """Reopen the DB read-write and recreate all views to fix stale column caches."""
    import sys as _sys
    import os as _os
    _sys.path.insert(0, _os.path.join(_os.path.dirname(_os.path.abspath(__file__)), ".."))
    try:
        import record_benchmark as rb
        con = duckdb.connect(str(path))
        rb.refresh_views(con)
        con.commit()
        con.close()
    except Exception as e:
        _sys.exit(f"Failed to auto-repair stale views in {path}: {e}")


def get_latest_run_id(con: duckdb.DuckDBPyConnection) -> str:
    row = con.execute("SELECT run_id FROM context ORDER BY run_ts DESC LIMIT 1").fetchone()
    if row is None:
        sys.exit("No runs found in database.")
    return row[0]


def resolve_run_id(con: duckdb.DuckDBPyConnection, cli_run_id: str | None) -> str:
    return cli_run_id if cli_run_id else get_latest_run_id(con)


def query_df(con: duckdb.DuckDBPyConnection, sql: str, params=None) -> pd.DataFrame:
    if params:
        return con.execute(sql, params).df()
    return con.execute(sql).df()


# ── Common CLI args ────────────────────────────────────────────────────────────

def add_common_args(parser: argparse.ArgumentParser) -> None:
    """Add standard reporting flags to a parser (all default=None)."""
    parser.add_argument("--db", metavar="PATH", default=None,
                        help="DuckDB file (overrides BENCHPLOT_DB_PATH env var).")
    parser.add_argument("--run-id", metavar="ID", default=None,
                        help="run_id to report on (default: most recent).")
    parser.add_argument("--filter", dest="kernel_filter", metavar="REGEX", default=None,
                        help="Regex filter on kernel name.")
    parser.add_argument("--time-unit", choices=["ns", "us", "ms", "s"], default=None,
                        help="Time unit for axis labels (default: auto).")
    parser.add_argument("--mem-unit", choices=["B", "KB", "MB"], default=None,
                        help="Memory unit for axis labels (default: auto).")
    parser.add_argument("--title", metavar="TITLE", default=None,
                        help="Figure title/suptitle.")


# ── Unit helpers ───────────────────────────────────────────────────────────────

# Divide nanosecond values by this to get the target unit.
TIME_UNITS: dict[str, float] = {"ns": 1.0, "us": 1e3, "ms": 1e6, "s": 1e9}

def auto_time_unit(median_ns: float) -> str:
    if median_ns >= 1e9: return "s"
    if median_ns >= 1e6: return "ms"
    if median_ns >= 1e3: return "us"
    return "ns"


MEM_UNITS: dict[str, int] = {"B": 1, "KB": 1024, "MB": 1024 ** 2}

def auto_mem_unit(median_bytes: float) -> str:
    if median_bytes >= 1024 ** 2: return "MB"
    if median_bytes >= 1024: return "KB"
    return "B"


def apply_kernel_filter(df: pd.DataFrame, pattern: str | None) -> pd.DataFrame:
    if not pattern:
        return df
    pat = re.compile(pattern, re.IGNORECASE)
    return df[df["kernel"].apply(lambda k: bool(pat.search(k)))]


# ── Report directory & output helpers ─────────────────────────────────────────

def make_report_dir(
    script_path: str,
    parser: argparse.ArgumentParser,
    args: argparse.Namespace,
) -> Path:
    """Create a timestamped report dir and immediately write regenerate.py into it.

    Directory layout:
        benchmarks/reports/YYMMDD-HH-MM-<script_stem>/
            regenerate.py   ← written here, before any plotting work
            plot.pdf        ← written later by save_plot()
            data.csv        ← written later by save_csv()
    """
    script_name = Path(script_path).stem
    timestamp = datetime.now().strftime("%y%m%d-%H-%M")
    benchmarks_dir = Path(script_path).resolve().parent.parent
    base = benchmarks_dir / "reports" / f"{timestamp}-{script_name}"

    report_dir = base
    suffix = 0
    while report_dir.exists():
        suffix += 1
        report_dir = Path(f"{base}-{suffix}")

    report_dir.mkdir(parents=True)
    _write_regenerate(report_dir, script_path, parser, args)
    print(f"Report → {report_dir}")
    return report_dir


def save_plot(fig, path: Path, **kwargs) -> None:
    """Save an ultraplot or matplotlib figure."""
    kw = {"dpi": 150, "bbox_inches": "tight", **kwargs}
    if hasattr(fig, "save"):
        fig.save(str(path), **kw)
    else:
        fig.savefig(str(path), **kw)
    print(f"  plot  → {path.name}")


def save_csv(df: pd.DataFrame, path: Path) -> None:
    df.to_csv(path, index=False)
    print(f"  data  → {path.name}")


# ── Internals ─────────────────────────────────────────────────────────────────

def _write_regenerate(
    report_dir: Path,
    script_path: str,
    parser: argparse.ArgumentParser,
    args: argparse.Namespace,
) -> None:
    script_abs = str(Path(script_path).resolve())
    argv = _reconstruct_argv(parser, args)
    args_repr = ", ".join(repr(a) for a in argv)

    content = textwrap.dedent(f"""\
        #!/usr/bin/env python3
        \"\"\"Re-runs the report with identical arguments.
        Output goes to a NEW timestamped directory — this report is not overwritten.\"\"\"
        import subprocess, sys

        SCRIPT = {repr(script_abs)}
        ARGS   = [{args_repr}]

        if __name__ == "__main__":
            sys.exit(subprocess.call([sys.executable, SCRIPT, *ARGS]))
    """)

    regen_path = report_dir / "regenerate.py"
    regen_path.write_text(content)
    regen_path.chmod(regen_path.stat().st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)


def _reconstruct_argv(
    parser: argparse.ArgumentParser,
    args: argparse.Namespace,
) -> list[str]:
    """Reconstruct the effective CLI argv from parser actions and parsed namespace.

    Only flags that were explicitly set (non-None, non-default) are emitted,
    so regenerate.py stays minimal and readable.
    """
    argv: list[str] = []
    for action in parser._actions:
        if isinstance(action, argparse._HelpAction):
            continue
        val = getattr(args, action.dest, None)

        if not action.option_strings:
            # Positional argument
            if val is not None:
                argv.append(str(val))
            continue

        opt = max(action.option_strings, key=len)

        if isinstance(action, (argparse._StoreTrueAction, argparse._StoreFalseAction)):
            if val != action.default:
                argv.append(opt)
        elif isinstance(action, argparse._StoreConstAction):
            if val == action.const:
                argv.append(opt)
        elif isinstance(action, argparse._AppendAction):
            for item in (val or []):
                argv.extend([opt, str(item)])
        elif isinstance(action, argparse._CountAction):
            argv.extend([opt] * (val or 0))
        else:
            # _StoreAction (most common)
            if val is not None and val != action.default:
                argv.extend([opt, str(val)])
    return argv
