#!/usr/bin/env python3
"""record_benchmark.py — Record Google Benchmark runs into DuckDB.

Usage:
    record_benchmark.py <binary> [--benchmark-filter=PATTERN] [--db=PATH]
"""

import argparse
import contextlib
import json
import os
import re
import shutil
import signal
import subprocess
import sys
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
    library_build_type  VARCHAR,
    best_practice_full  BOOLEAN
);
"""

# Base columns that are always present in Google Benchmark JSON output.
# Dynamic columns (perf counters, custom counters, etc.) are added on demand.
# kv_* columns are parsed from the benchmark name KV prefix (e.g. BM_g:X;n:Y;t:Z;).
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
    kv_g                       VARCHAR,
    kv_n                       VARCHAR,
    kv_t                       VARCHAR,
    kv_raw_params              VARCHAR,
    PRIMARY KEY (run_id, name)
);
"""

# Expose stored KV columns as readable aliases.
_SCHEMA_V_PARSED = """
CREATE OR REPLACE VIEW v_parsed AS
SELECT
    b.*,
    c.git_sha,
    c.run_ts,
    c.host_name,
    b.kv_t          AS phase,
    b.kv_n          AS kernel,
    b.kv_g          AS "group",
    b.kv_raw_params AS raw_params
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

# Pivot phases per (run_id, kernel, raw_params, group) for ratio computation.
_SCHEMA_V_RATIOS = """
CREATE OR REPLACE VIEW v_ratios AS
SELECT
    run_id,
    kernel,
    raw_params,
    "group",
    git_sha,
    run_ts,
    host_name,
    MAX(CASE WHEN phase = 'unspecialized'    THEN real_time_ns END) AS t_unspec_ns,
    MAX(CASE WHEN phase = 'specialized_exec' THEN real_time_ns END) AS t_spec_ns,
    MAX(CASE WHEN phase = 'jit_overhead'     THEN real_time_ns END) AS t_jit_ns
FROM v_ns
GROUP BY run_id, kernel, raw_params, "group", git_sha, run_ts, host_name;
"""

# JIT stats for jit_overhead rows (columns may not exist; view creation guarded).
_SCHEMA_V_JIT_STATS = """
CREATE OR REPLACE VIEW v_jit_stats AS
SELECT run_id, kernel, raw_params, "group",
       jit_module_fns, jit_module_instrs, jit_blob_kb,
       jit_pruned_fns, jit_pruned_instrs, max_bytes_used
FROM v_parsed
WHERE phase = 'jit_overhead' AND run_type = 'iteration';
"""

# Budget sweep Pareto view: (scale_bp, kernel) → (JIT overhead, speedup vs unspecialized).
_SCHEMA_V_BUDGET_SWEEP = """
CREATE OR REPLACE VIEW v_budget_sweep AS
SELECT run_id, kernel, "group", git_sha, run_ts,
       kv_raw_params,
       CAST(SPLIT_PART(kv_raw_params, '/', 2) AS INTEGER) AS scale_bp,
       real_time_ns, phase,
       expected_call_ns, budget_scale, budget_fixpoint, budget_unroll
FROM v_ns
WHERE phase IN ('jit_budget_sweep', 'exec_budget_sweep');
"""

# Per-pass records from PassInstrumentationCallbacks (written by benchmarkJITAnalysis).
_SCHEMA_PASS_TRACES = """
CREATE TABLE IF NOT EXISTS pass_traces (
    run_id          VARCHAR NOT NULL REFERENCES context(run_id),
    benchmark_name  VARCHAR NOT NULL,
    pass_idx        INTEGER NOT NULL,
    pass_name       VARCHAR,
    pass_group      VARCHAR,
    fixpoint_iter   INTEGER,
    fns_before      BIGINT,
    fns_after       BIGINT,
    instrs_before   BIGINT,
    instrs_after    BIGINT,
    bbs_before      BIGINT,
    bbs_after       BIGINT,
    wall_time_ms    DOUBLE,
    ir_changed      BOOLEAN
);
"""

_SCHEMA_ABLATION_STUDIES = """
CREATE TABLE IF NOT EXISTS ablation_studies (
    study_name  VARCHAR NOT NULL,
    config_name VARCHAR NOT NULL,
    rep         INTEGER NOT NULL,
    params_json JSON,
    run_id      VARCHAR REFERENCES context(run_id),
    PRIMARY KEY (study_name, config_name, rep)
);
"""

_SCHEMA_V_ABLATION_RESULTS = """
CREATE OR REPLACE VIEW v_ablation_results AS
SELECT
    a.study_name, a.config_name, a.rep, a.params_json,
    r.kernel, r."group",
    r.t_jit_ns, r.t_spec_ns, r.t_unspec_ns
FROM ablation_studies a
JOIN v_ratios r USING (run_id);
"""

_SCHEMA_V_ABLATION_MEDIANS = """
CREATE OR REPLACE VIEW v_ablation_medians AS
SELECT
    study_name, config_name, kernel, "group",
    PERCENTILE_CONT(0.5) WITHIN GROUP (ORDER BY t_jit_ns)    AS med_jit_ns,
    PERCENTILE_CONT(0.5) WITHIN GROUP (ORDER BY t_spec_ns)   AS med_spec_ns,
    PERCENTILE_CONT(0.5) WITHIN GROUP (ORDER BY t_unspec_ns) AS med_unspec_ns,
    COUNT(*) AS n_reps
FROM v_ablation_results
GROUP BY study_name, config_name, kernel, "group";
"""

# Fixed column names (lowercase) that are part of the base schema.
_BASE_COLUMNS = {
    "run_id", "name", "family_index", "per_family_instance_index",
    "run_type", "repetitions", "repetition_index", "threads",
    "iterations", "real_time", "cpu_time", "time_unit",
    "kv_g", "kv_n", "kv_t", "kv_raw_params",
}

# ---------------------------------------------------------------------------
# KV name parsing
# ---------------------------------------------------------------------------

_KV_PREFIX_RE = re.compile(r'^BM_(?P<kvs>[^/]+?)(?:/(?P<params>.*))?$')
_KV_PAIR_RE = re.compile(r'(\w+):([^;/]+);')


def _parse_bm_name(name: str) -> dict:
    """Parse 'BM_g:X;n:Y;t:Z;/1/2' → {'kv_g': 'X', 'kv_n': 'Y', 'kv_t': 'Z', 'kv_raw_params': '/1/2'}.

    Returns empty dict if the name does not match the KV format.
    """
    m = _KV_PREFIX_RE.match(name)
    if not m:
        return {}
    result = {}
    for key, val in _KV_PAIR_RE.findall(m.group('kvs') or ''):
        result[f'kv_{key}'] = val
    if 'kv_g' not in result:
        return {}  # not a KV-format name (no 'g:' key)
    params = m.group('params')
    result['kv_raw_params'] = f'/{params}' if params else ''
    return result


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
    """Add any JSON keys or KV name keys missing from the benchmarks table as new columns."""
    existing = {
        row[0].lower()
        for row in con.execute("DESCRIBE benchmarks").fetchall()
    }
    for b in benchmarks:
        # JSON fields from benchmark output
        for key, val in b.items():
            col = _col_name(key)
            if col.lower() in existing:
                continue
            if val is None:
                continue  # defer until we see an actual value for type inference
            dtype = _infer_sql_type(val)
            con.execute(f'ALTER TABLE benchmarks ADD COLUMN "{col}" {dtype}')
            existing.add(col.lower())
        # KV columns parsed from the benchmark name (always VARCHAR)
        for col in _parse_bm_name(b.get('name', '')):
            if col.lower() not in existing:
                con.execute(f'ALTER TABLE benchmarks ADD COLUMN "{col}" VARCHAR')
                existing.add(col.lower())


def insert_benchmarks(con: duckdb.DuckDBPyConnection, run_id: str, benchmarks: list) -> None:
    """Insert all benchmark rows, mapping JSON keys and KV name keys to columns dynamically."""
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
        # Merge KV columns parsed from the benchmark name
        for col, val in _parse_bm_name(b.get('name', '')).items():
            if col.lower() in cols_in_db:
                row_data[col] = val

        col_list = ", ".join(f'"{c}"' for c in row_data)
        placeholders = ", ".join(["?"] * len(row_data))
        con.execute(
            f"INSERT INTO benchmarks ({col_list}) VALUES ({placeholders})",
            list(row_data.values()),
        )


# ---------------------------------------------------------------------------
# Pass-trace import
# ---------------------------------------------------------------------------

def import_pass_traces(con: duckdb.DuckDBPyConnection, run_id: str, trace_dir: Path,
                       delete_after_import: bool = True) -> int:
    """Import *_pass_trace.json files written by benchmarkJITAnalysis into pass_traces table."""
    total = 0
    for trace_file in sorted(trace_dir.glob("*_pass_trace.json")):
        try:
            with open(trace_file) as f:
                records = json.load(f)
            bm_name = trace_file.stem[:-len("_pass_trace")]
            for idx, r in enumerate(records):
                con.execute(
                    "INSERT INTO pass_traces VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
                    [
                        run_id, bm_name, idx,
                        r.get("name"), r.get("group"), r.get("fixpoint_iter"),
                        r.get("fns_before"), r.get("fns_after"),
                        r.get("instrs_before"), r.get("instrs_after"),
                        r.get("bbs_before"), r.get("bbs_after"),
                        r.get("wall_time_ms"), r.get("ir_changed"),
                    ],
                )
                total += 1
            print(f"  Imported {len(records)} pass records from {trace_file.name}")
            if delete_after_import:
                trace_file.unlink()
        except Exception as e:
            print(f"Warning: could not import pass trace {trace_file}: {e}", file=sys.stderr)
    return total


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


def refresh_views(con: duckdb.DuckDBPyConnection) -> None:
    """Recreate all views so their cached column lists reflect the current table schema."""
    for stmt in [_SCHEMA_V_PARSED, _SCHEMA_V_NS, _SCHEMA_V_RATIOS]:
        con.execute(stmt)
    try:
        con.execute(_SCHEMA_V_JIT_STATS)
    except Exception:
        pass
    try:
        con.execute(_SCHEMA_V_BUDGET_SWEEP)
    except Exception:
        pass
    try:
        con.execute(_SCHEMA_V_ABLATION_RESULTS)
        con.execute(_SCHEMA_V_ABLATION_MEDIANS)
    except Exception:
        pass


def open_db(db_path: Path) -> duckdb.DuckDBPyConnection:
    if not db_path.exists():
        print(f"Error: DB file not found: {db_path}", file=sys.stderr)
        print("Run create_db.py to initialise a new database.", file=sys.stderr)
        sys.exit(1)
    con = duckdb.connect(str(db_path))
    for stmt in [_SCHEMA_CONTEXT, _SCHEMA_BENCHMARKS, _SCHEMA_PASS_TRACES]:
        con.execute(stmt)
    # Migrate existing DBs that predate the best_practice_full column.
    con.execute("ALTER TABLE context ADD COLUMN IF NOT EXISTS best_practice_full BOOLEAN")
    con.execute(_SCHEMA_ABLATION_STUDIES)
    refresh_views(con)
    return con


# ---------------------------------------------------------------------------
# CPU management helpers (best-practice mode)
# ---------------------------------------------------------------------------

_GOV_DIR = Path("/sys/devices/system/cpu/cpu0/cpufreq")
_GOV_FILE = _GOV_DIR / "scaling_governor"


def _parse_cpu_list(s: str) -> list[int]:
    """Parse a kernel cpulist string like '0,4-7,10' into a sorted list of ints."""
    cpus = []
    for part in s.strip().split(","):
        if "-" in part:
            lo, hi = part.split("-", 1)
            cpus.extend(range(int(lo), int(hi) + 1))
        else:
            cpus.append(int(part))
    return sorted(set(cpus))


def _online_cpus() -> list[int]:
    """Return sorted list of currently online CPU indices."""
    result = []
    for cpu_dir in sorted(
        Path("/sys/devices/system/cpu").glob("cpu[0-9]*"),
        key=lambda p: int(p.name[3:]),
    ):
        idx = int(cpu_dir.name[3:])
        online_file = cpu_dir / "online"
        if not online_file.exists():
            result.append(idx)  # cpu0 has no 'online' file, always on
        elif online_file.read_text().strip() == "1":
            result.append(idx)
    return result


def _get_smt_siblings(cpu: int) -> list[int]:
    """Return all sibling logical CPUs for cpu (including cpu itself)."""
    siblings_file = Path(
        f"/sys/devices/system/cpu/cpu{cpu}/topology/thread_siblings_list"
    )
    if not siblings_file.exists():
        return [cpu]
    return _parse_cpu_list(siblings_file.read_text())


def _sudo_cmd() -> list[str]:
    """Return the sudo invocation to use.

    Uses ``sudo -A`` (askpass) when SUDO_ASKPASS is set in the environment or
    when there is no interactive terminal, so the script works in non-terminal
    environments (CI, remote sessions, IDE runners).
    """
    if os.environ.get("SUDO_ASKPASS") or not sys.stdin.isatty():
        return ["sudo", "-A"]
    return ["sudo"]


def _sudo_write(path: str, value: str) -> None:
    subprocess.run(
        _sudo_cmd() + ["tee", path], input=value, text=True, check=True, capture_output=True
    )


def _set_governor(gov: str) -> None:
    if shutil.which("cpupower"):
        subprocess.run(
            _sudo_cmd() + ["cpupower", "frequency-set", "-g", gov],
            check=True, capture_output=True,
        )
    else:
        for f in Path("/sys/devices/system/cpu").glob("cpu*/cpufreq/scaling_governor"):
            _sudo_write(str(f), gov)


def _classify_cpus() -> tuple[list[int], list[int]]:
    """
    Classify online CPUs into P-cores and E-cores.

    P-cores (Performance): physical cores with more than one logical CPU,
    i.e. hyperthreaded cores.  Each physical core is represented by its
    lowest-numbered logical CPU.

    E-cores (Efficiency): physical cores with exactly one logical CPU
    (no hyperthreading sibling).

    Returns (p_core_representatives, e_core_representatives), each sorted.
    """
    online = set(_online_cpus())
    seen: set[frozenset] = set()
    p_cores: list[int] = []
    e_cores: list[int] = []
    for cpu in sorted(online):
        siblings = frozenset(_get_smt_siblings(cpu))
        if siblings in seen:
            continue
        seen.add(siblings)
        if len(siblings) > 1:
            p_cores.append(min(siblings))
        else:
            e_cores.append(cpu)
    return sorted(p_cores), sorted(e_cores)


def _pick_benchmark_cpus(requested: list[int] | None) -> list[int]:
    """
    Return CPUs to dedicate to the benchmark.
    If requested is given, validate and use those.
    Otherwise auto-pick the 2 highest-indexed P-cores (one logical CPU per
    physical core, so their HT siblings can be disabled).  Falls back to
    E-cores if no P-cores are available.
    """
    online = set(_online_cpus())
    if requested is not None:
        valid = [c for c in requested if c in online]
        invalid = [c for c in requested if c not in online]
        if invalid:
            print(f"Warning: CPU(s) {invalid} not online; skipping.", file=sys.stderr)
        if valid:
            return valid
        print("Warning: no valid requested CPUs; falling back to auto-selection.", file=sys.stderr)

    p_cores, e_cores = _classify_cpus()
    if p_cores:
        # Exclude CPU 0 (handles boot/IRQ traffic) unless it's the only option.
        candidates = [c for c in p_cores if c != 0] or p_cores
        core_type = "P-core"
    else:
        candidates = [c for c in e_cores if c != 0] or e_cores
        core_type = "E-core"

    chosen = candidates[-2:] if len(candidates) >= 2 else list(candidates)
    print(f"Auto-selected {core_type}(s) {chosen} for benchmarking.", file=sys.stderr)
    return chosen


@contextlib.contextmanager
def best_practice_env(benchmark_cpus: list[int]):
    """
    Context manager: applies LLVM Linux benchmarking best practices then
    restores the original state on exit.

    Yields True if all setup steps succeeded (caller should prefix the
    benchmark command with `taskset -c <cpus>`), False on any failure.

    Steps applied (in order):
      1. Disable ASLR
      2. Disable Intel Turbo Boost (if available)
      3. Set CPU governor to performance
      4. Disable SMT siblings of benchmark CPUs
    CPU affinity is enforced by the caller via taskset (no cpuset filesystem needed).

    Teardown is guaranteed to run for any Python-catchable termination: normal
    exit, exceptions (including DB write failures), KeyboardInterrupt (SIGINT),
    and SIGTERM.  SIGKILL cannot be intercepted and is out of scope.
    """
    disabled_cpus: list[int] = []
    original_gov: str | None = None
    original_aslr: str | None = None
    original_turbo: str | None = None
    setup_failed = False

    cpu_list_str = ",".join(str(c) for c in sorted(benchmark_cpus))
    aslr_path = "/proc/sys/kernel/randomize_va_space"
    turbo_path = "/sys/devices/system/cpu/intel_pstate/no_turbo"

    # Convert SIGTERM to SystemExit before any setup so the finally block below
    # always runs teardown.  SIGINT already raises KeyboardInterrupt natively.
    def _sigterm_handler(signum, frame):
        raise SystemExit(signum)

    original_sigterm = signal.signal(signal.SIGTERM, _sigterm_handler)

    try:
        # 1. Disable ASLR
        try:
            original_aslr = Path(aslr_path).read_text().strip()
            if original_aslr != "0":
                print("Disabling ASLR...", file=sys.stderr)
                _sudo_write(aslr_path, "0\n")
        except Exception as e:
            print(f"Warning: could not configure ASLR: {e}", file=sys.stderr)
            setup_failed = True

        # 2. Disable Intel Turbo Boost (Intel pstate only)
        if Path(turbo_path).exists():
            try:
                original_turbo = Path(turbo_path).read_text().strip()
                if original_turbo != "1":
                    print("Disabling Intel Turbo Boost...", file=sys.stderr)
                    _sudo_write(turbo_path, "1\n")
            except Exception as e:
                print(f"Warning: could not disable Turbo Boost: {e}", file=sys.stderr)
                setup_failed = True

        # 3. Set CPU governor to performance
        if _GOV_FILE.exists():
            try:
                original_gov = _GOV_FILE.read_text().strip()
                if original_gov != "performance":
                    print(
                        f"Setting CPU governor to performance (was: {original_gov})...",
                        file=sys.stderr,
                    )
                    _set_governor("performance")
            except Exception as e:
                print(f"Warning: could not set CPU governor: {e}", file=sys.stderr)
                setup_failed = True
        else:
            print("Warning: CPU frequency scaling not available.", file=sys.stderr)
            setup_failed = True

        # 4. Disable SMT siblings of benchmark CPUs
        benchmark_cpu_set = set(benchmark_cpus)
        siblings_to_disable: set[int] = set()
        for cpu in benchmark_cpus:
            for sib in _get_smt_siblings(cpu):
                if sib not in benchmark_cpu_set:
                    siblings_to_disable.add(sib)

        for sib in sorted(siblings_to_disable):
            online_file = f"/sys/devices/system/cpu/cpu{sib}/online"
            if Path(online_file).exists():
                try:
                    print(f"Disabling SMT sibling CPU {sib}...", file=sys.stderr)
                    _sudo_write(online_file, "0\n")
                    disabled_cpus.append(sib)
                except Exception as e:
                    print(f"Warning: could not disable CPU {sib}: {e}", file=sys.stderr)
                    setup_failed = True

        print(f"CPU affinity will be set via taskset on CPU(s) {cpu_list_str}.", file=sys.stderr)

        yield not setup_failed

    finally:
        # Restore SIGTERM before any sudo calls in teardown.
        signal.signal(signal.SIGTERM, original_sigterm)

        # Teardown in reverse order; each step is independent.
        for sib in sorted(disabled_cpus):
            online_file = f"/sys/devices/system/cpu/cpu{sib}/online"
            try:
                print(f"Re-enabling SMT sibling CPU {sib}...", file=sys.stderr)
                _sudo_write(online_file, "1\n")
            except Exception as e:
                print(f"Warning: could not re-enable CPU {sib}: {e}", file=sys.stderr)

        if original_gov is not None and original_gov != "performance":
            try:
                print(f"Restoring CPU governor to {original_gov}...", file=sys.stderr)
                _set_governor(original_gov)
            except Exception as e:
                print(f"Warning: could not restore CPU governor: {e}", file=sys.stderr)

        if original_turbo is not None and original_turbo != "1" and Path(turbo_path).exists():
            try:
                _sudo_write(turbo_path, f"{original_turbo}\n")
            except Exception as e:
                print(f"Warning: could not restore Turbo Boost: {e}", file=sys.stderr)

        if original_aslr is not None and original_aslr != "0":
            try:
                print("Restoring ASLR...", file=sys.stderr)
                _sudo_write(aslr_path, f"{original_aslr}\n")
            except Exception as e:
                print(f"Warning: could not restore ASLR: {e}", file=sys.stderr)


# ---------------------------------------------------------------------------
# Memory limit helpers (FR-047)
# ---------------------------------------------------------------------------

def _wrap_with_mem_limits(cmd: list[str], mem_max: str, mem_high: str) -> list[str]:
    """Prepend systemd-run --scope memory limits to cmd; warn and return cmd unchanged if unavailable."""
    systemd_run = shutil.which("systemd-run")
    if not systemd_run:
        print(
            "Warning: systemd-run not found; memory limits will not be applied.",
            file=sys.stderr,
        )
        return cmd
    return [systemd_run, "--scope",
            "-p", f"MemoryMax={mem_max}",
            "-p", f"MemoryHigh={mem_high}"] + cmd


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

def check_dependencies(args) -> None:
    missing = []
    if getattr(args, 'binary', None) and not Path(args.binary).exists():
        missing.append(f"benchmark binary not found: {args.binary}")
    if getattr(args, 'benchmarking_best_practice', False):
        if not shutil.which("sudo"):
            missing.append("sudo (required for --benchmarking-best-practice)")
    # Validate DB path before running the benchmark so we fail fast rather than
    # running a potentially long benchmark and then losing its results.
    db_path = resolve_db_path(getattr(args, 'db', None))
    if not db_path.exists():
        missing.append(
            f"DB file not found: {db_path} — run create_db.py to initialise a new database"
        )
    if missing:
        print("Error: missing dependencies:", file=sys.stderr)
        for m in missing:
            print(f"  - {m}", file=sys.stderr)
        sys.exit(1)


def _load_json_safe(path: str) -> dict:
    try:
        with open(path) as f:
            return json.load(f)
    except Exception:
        import traceback
        traceback.print_exc()
        print(file=sys.stderr)
        print(f"ERROR: could not parse benchmark JSON output at {path}", file=sys.stderr)
        try:
            lines = Path(path).read_text().splitlines()
            if lines:
                print(f"First {min(10, len(lines))} lines of the file:", file=sys.stderr)
                for ln in lines[:10]:
                    print(f"  {ln}", file=sys.stderr)
            else:
                print("  (file is empty — benchmark may have crashed before writing output)", file=sys.stderr)
        except Exception:
            print("  (could not read file)", file=sys.stderr)
        print(file=sys.stderr)
        print(f"The raw output file is preserved at: {path}", file=sys.stderr)
        print(f"If the data is recoverable, run:", file=sys.stderr)
        print(f"  record_benchmark.py --record-json {path}", file=sys.stderr)
        sys.exit(1)


def store_to_db(args, data: dict, json_path: str, delete_on_success: bool,
                trace_dir: Path | None = None,
                best_practice_full: bool | None = None,
                keep_pass_traces: bool = False) -> None:
    ctx = data.get("context", {})
    benchmarks = data.get("benchmarks", [])
    db_path = resolve_db_path(args.db)
    con = None
    try:
        con = open_db(db_path)
        run_id = str(uuid.uuid4())
        run_ts = datetime.now()

        con.begin()
        con.execute(
            "INSERT INTO context VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
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
                best_practice_full,
            ],
        )
        ensure_columns(con, benchmarks)
        refresh_views(con)
        insert_benchmarks(con, run_id, benchmarks)
        if trace_dir is not None:
            n_traces = import_pass_traces(con, run_id, trace_dir,
                                          delete_after_import=not keep_pass_traces)
            if n_traces:
                print(f"Imported {n_traces} pass-trace records.")
        con.commit()

        print(f"run_id: {run_id}")
        print(f"Stored context + {len(benchmarks)} benchmark rows.")
        if delete_on_success:
            os.unlink(json_path)
    except Exception:
        if con is not None:
            try:
                con.rollback()
            except Exception:
                pass
        import traceback
        traceback.print_exc()
        print(file=sys.stderr)
        print(f"ERROR while adding data to DB. The output from this benchmark run is available at {json_path}", file=sys.stderr)
        print(file=sys.stderr)
        print(f"Please fix the importing issues and run:", file=sys.stderr)
        print(f"  record_benchmark.py --record-json {json_path}", file=sys.stderr)
        sys.exit(1)


def _make_run_dir() -> tuple[Path, Path, Path, Path]:
    """Create a timestamped run directory under benchmarks/benchmarks_raw_data/.

    Returns (run_dir, out_path, pass_traces_dir, chrome_traces_dir).
    If the base timestamp dir already exists, appends -2, -3, … until a free name is found.
    On failure or crash the directory persists for manual inspection.
    """
    script_dir = Path(__file__).parent
    base = script_dir / "benchmarks_raw_data" / datetime.now().strftime("%Y%m%d-%H%M%S")
    run_dir = base
    counter = 2
    while True:
        try:
            run_dir.mkdir(parents=True)
            break
        except FileExistsError:
            run_dir = Path(f"{base}-{counter}")
            counter += 1
    pass_traces_dir = run_dir / "pass_traces"
    chrome_traces_dir = run_dir / "chrome_traces"
    pass_traces_dir.mkdir()
    chrome_traces_dir.mkdir()
    out_path = run_dir / "raw.json"
    return run_dir, out_path, pass_traces_dir, chrome_traces_dir


def _cleanup_after_store(pass_traces_dir: Path, chrome_traces_dir: Path,
                         run_dir: Path, keep_chrome_traces: bool) -> None:
    """Remove trace files and empty dirs created by this run.

    pass_traces_dir: individual JSON files already deleted by import_pass_traces;
      this tries to rmdir the now-empty directory.
    chrome_traces_dir: rmtree'd unless --keep-chrome-traces; then rmdir is attempted.
    run_dir: rmdir attempted last; succeeds only when both subdirs were removed.
    """
    if not keep_chrome_traces:
        shutil.rmtree(chrome_traces_dir, ignore_errors=True)
    for d in (pass_traces_dir, chrome_traces_dir, run_dir):
        try:
            d.rmdir()
        except OSError:
            pass


def cmd_record(args):
    check_dependencies(args)

    if args.sudo_askpass:
        os.environ["SUDO_ASKPASS"] = str(Path(args.sudo_askpass).resolve())

    sha = get_git_sha()

    run_dir, out_path, pass_traces_dir, chrome_traces_dir = _make_run_dir()
    print(f"Run directory: {run_dir}", flush=True)

    sub_env = os.environ.copy()
    sub_env["CRS_PASS_TRACE_DIR"] = str(pass_traces_dir)
    sub_env["CRS_CHROME_TRACE_DIR"] = str(chrome_traces_dir)

    flags = [
        "--benchmark_out_format=json",
        f"--benchmark_out={out_path}",
        f"--benchmark_context=git_sha={sha}",
        f"--benchmark_context=benchmark_best_practice={args.benchmarking_best_practice}",
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

    if args.benchmarking_best_practice:
        requested_cpus = _parse_cpu_list(args.benchmark_cpus) if args.benchmark_cpus else None
        benchmark_cpus = _pick_benchmark_cpus(requested_cpus)
        cpu_list_str = ",".join(str(c) for c in sorted(benchmark_cpus))
        with best_practice_env(benchmark_cpus) as setup_ok:
            if not setup_ok:
                print(
                    "Warning: not all benchmarking best-practice settings could be applied; "
                    "results may be less reliable.",
                    file=sys.stderr,
                )
            taskset_bin = shutil.which("taskset") or "taskset"
            full_cmd = [taskset_bin, "-c", cpu_list_str] + cmd
            if not args.no_mem_limits:
                full_cmd = _wrap_with_mem_limits(full_cmd, args.mem_max, args.mem_high)
            print(f"Running: {' '.join(full_cmd)}", flush=True)
            result = subprocess.run(full_cmd, env=sub_env)
            if result.returncode != 0:
                print(
                    f"Warning: benchmark exited with code {result.returncode}; "
                    "storing available results before restoring system settings.",
                    file=sys.stderr,
                )
            # Store BEFORE the with-block exits so teardown (ASLR/governor restore)
            # happens after the DB write, not before.
            data = _load_json_safe(str(out_path))
            store_to_db(args, data, str(out_path), delete_on_success=True,
                        trace_dir=pass_traces_dir, best_practice_full=bool(setup_ok),
                        keep_pass_traces=args.keep_pass_traces)
            _cleanup_after_store(pass_traces_dir, chrome_traces_dir, run_dir,
                                 args.keep_chrome_traces)
        # System settings restored here; propagate non-zero exit code after the fact.
        if result.returncode != 0:
            sys.exit(result.returncode)
    else:
        full_cmd = cmd
        if not args.no_mem_limits:
            full_cmd = _wrap_with_mem_limits(full_cmd, args.mem_max, args.mem_high)
        print(f"Running: {' '.join(full_cmd)}", flush=True)
        result = subprocess.run(full_cmd, env=sub_env)
        if result.returncode != 0:
            # run_dir persists with whatever was written for manual inspection.
            print(f"Raw data preserved at: {run_dir}", file=sys.stderr)
            sys.exit(result.returncode)
        data = _load_json_safe(str(out_path))
        store_to_db(args, data, str(out_path), delete_on_success=True,
                    trace_dir=pass_traces_dir,
                    keep_pass_traces=args.keep_pass_traces)
        _cleanup_after_store(pass_traces_dir, chrome_traces_dir, run_dir,
                             args.keep_chrome_traces)


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
        help="Apply LLVM Linux benchmarking best practices: performance CPU governor, "
             "disable ASLR, disable Turbo Boost, disable SMT siblings, isolate CPUs "
             "with cpuset shield (requires sudo).",
    )
    parser.add_argument(
        "--benchmark-cpus", metavar="LIST", default=None,
        help="Comma-separated CPU indices to dedicate for benchmarking (e.g. '2,3'). "
             "Used with --benchmarking-best-practice. Default: auto-select the 2 "
             "highest-indexed non-boot CPUs.",
    )
    parser.add_argument(
        "--sudo-askpass", metavar="PATH", default=None,
        help="Path to an askpass helper program (sets SUDO_ASKPASS and uses 'sudo -A'). "
             "Required when running without a terminal. Example: /usr/lib/ssh/x11-ssh-askpass",
    )
    parser.add_argument(
        "--db", default=None, metavar="PATH",
        help="DuckDB file path (overrides BENCHPLOT_DB_PATH env var and CWD default).",
    )
    parser.add_argument(
        "--pass-trace-dir", metavar="DIR", default=None,
        help="Directory to scan for *_pass_trace.json files (used with --record-json). "
             "Defaults to CRS_PASS_TRACE_DIR env var if set, otherwise no trace import.",
    )
    parser.add_argument(
        "--keep-pass-traces", action="store_true",
        help="Do not delete *_pass_trace.json files after importing them into the DB.",
    )
    parser.add_argument(
        "--keep-chrome-traces", action="store_true",
        help="Do not delete *_chrome_trace.json files after the benchmark run.",
    )
    parser.add_argument(
        "--no-mem-limits", action="store_true",
        help="Do not wrap the benchmark with systemd-run memory limits.",
    )
    parser.add_argument(
        "--mem-max", metavar="SIZE", default="32G",
        help="Hard memory ceiling passed as MemoryMax to systemd-run (default: 32G).",
    )
    parser.add_argument(
        "--mem-high", metavar="SIZE", default="26G",
        help="Soft memory high-water mark passed as MemoryHigh to systemd-run (default: 26G).",
    )

    args = parser.parse_args()

    if args.record_json and args.binary:
        parser.error("--record-json and binary are mutually exclusive.")
    if not args.record_json and not args.binary:
        parser.error("Provide a binary to run, or use --record-json to import existing results.")

    if args.record_json:
        data = _load_json_safe(args.record_json)
        if args.pass_trace_dir:
            trace_dir: Path | None = Path(args.pass_trace_dir)
        else:
            sibling = Path(args.record_json).parent / "pass_traces"
            trace_dir = sibling if sibling.is_dir() else None
        store_to_db(args, data, args.record_json, delete_on_success=False, trace_dir=trace_dir,
                    keep_pass_traces=args.keep_pass_traces)
    else:
        cmd_record(args)


if __name__ == "__main__":
    main()
