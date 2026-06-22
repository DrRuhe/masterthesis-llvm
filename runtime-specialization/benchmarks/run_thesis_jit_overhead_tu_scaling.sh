#!/usr/bin/env bash
# Run the thesis JIT-overhead vs TU-size collection for fresh SQLite + PolyBench
# data under the best-practice benchmark controls.
#
# This script intentionally reruns only the suites that already record TU-size
# metadata in DuckDB:
#   - db/sqlite3/tpch
#   - db/sqlite3/input_size
#   - polybench
#
# UC overheads are not recollected here; the corresponding plot joins fresh
# SQLite/PolyBench rows with an existing UC best-practice study and an offline
# UC TU-size manifest extracted from built objects.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

if [[ $# -eq 1 && ( "$1" == "-h" || "$1" == "--help" ) ]]; then
    echo "Usage: $0 <all_benchmarks_binary> [duckdb_path]"
    exit 0
fi

if [[ $# -lt 1 || $# -gt 2 ]]; then
    echo "Usage: $0 <all_benchmarks_binary> [duckdb_path]" >&2
    exit 1
fi

BINARY="$1"
DB="${2:-benchmarks.duckdb}"
STAMP="$(date +%Y%m%d_%H%M%S)"
STUDY="thesis_jit_overhead_tu_scaling_${STAMP}"
REPORT_DIR="reports/${STAMP}-thesis-jit-overhead-tu-scaling"
CONFIG_JSON="$REPORT_DIR/configs.json"
FILTER='^(BM_g:(db/sqlite3/tpch|db/sqlite3/input_size|polybench);.*t:jit_overhead;.*)$'
TIMEOUT_SECONDS=21600

if [[ ! -f "$BINARY" ]]; then
    echo "Error: benchmark binary not found: $BINARY" >&2
    exit 1
fi

if [[ ! -f "$DB" ]]; then
    echo "Error: DuckDB file not found: $DB" >&2
    echo "Create it first via benchmarks/create_db.py or pass an existing DB path." >&2
    exit 1
fi

if [[ -e "$REPORT_DIR" && ! -w "$REPORT_DIR" ]]; then
    SUFFIX=1
    while [[ -e "${REPORT_DIR}-${SUFFIX}" ]]; do
        ((SUFFIX++))
    done
    REPORT_DIR="${REPORT_DIR}-${SUFFIX}"
fi

mkdir -p "$REPORT_DIR"

if [[ -e "$CONFIG_JSON" && ! -w "$CONFIG_JSON" ]]; then
    echo "Error: config path is not writable: $CONFIG_JSON" >&2
    echo "An older run likely created it via sudo. Remove or chown that file, then rerun." >&2
    exit 1
fi

cat > "$CONFIG_JSON" <<'JSON'
[
  {"name": "default", "env": {}}
]
JSON

python3 ablation_benchmarks.py "$BINARY" \
  --db "$DB" \
  --study-name "$STUDY" \
  --configs "$CONFIG_JSON" \
  --benchmark-filter "$FILTER" \
  --reps 5 \
  --timeout "$TIMEOUT_SECONDS" \
  --benchmarking-best-practice

python3 - "$DB" "$STUDY" <<'PY'
import sys
import duckdb

db = sys.argv[1]
study = sys.argv[2]
con = duckdb.connect(db, read_only=True)

run_summary = con.execute(
    """
    SELECT
        COUNT(DISTINCT a.run_id) AS total_runs,
        COUNT(DISTINCT CASE WHEN c.best_practice_full THEN a.run_id END) AS best_practice_runs,
        COUNT(DISTINCT a.config_name) AS config_count
    FROM ablation_studies a
    LEFT JOIN context c USING (run_id)
    WHERE a.study_name = ?
      AND a.run_id IS NOT NULL
    """,
    [study],
).fetchone()

suite_rows = con.execute(
    """
    WITH scoped AS (
        SELECT
            p."group" AS suite_group,
            p.kernel,
            p.jit_module_instrs,
            p.jit_blob_kb
        FROM v_parsed p
        JOIN ablation_studies a USING (run_id)
        JOIN context c USING (run_id)
        WHERE a.study_name = ?
          AND a.config_name = 'default'
          AND c.best_practice_full
          AND p.phase = 'jit_overhead'
          AND p."group" IN ('db/sqlite3/tpch', 'db/sqlite3/input_size', 'polybench')
    )
    SELECT
        suite_group,
        COUNT(*) AS jit_rows,
        COUNT(DISTINCT kernel) AS kernels,
        COUNT(*) FILTER (WHERE jit_module_instrs IS NOT NULL AND jit_blob_kb IS NOT NULL) AS rows_with_size
    FROM scoped
    GROUP BY suite_group
    ORDER BY suite_group
    """
    ,
    [study],
).fetchall()

total_runs = int(run_summary[0] or 0)
best_practice_runs = int(run_summary[1] or 0)
config_count = int(run_summary[2] or 0)

print(f"Study: {study}")
print(f"Runs recorded: {total_runs}")
print(f"Runs with best_practice_full=TRUE: {best_practice_runs}")
print(f"Configs covered: {config_count}")

if total_runs == 0:
    raise SystemExit("No runs were recorded for the study.")
if best_practice_runs != total_runs:
    raise SystemExit(
        f"Best-practice validation failed: {best_practice_runs}/{total_runs} runs have best_practice_full=TRUE."
    )
if config_count != 1:
    raise SystemExit(f"Expected exactly one config ('default'), got {config_count}.")
if not suite_rows:
    raise SystemExit("No SQLite/PolyBench jit_overhead rows were recorded for the study.")

missing_size = []
for suite_group, jit_rows, kernels, rows_with_size in suite_rows:
    print(f"{suite_group}: jit_rows={jit_rows} kernels={kernels} rows_with_size={rows_with_size}")
    if jit_rows != rows_with_size:
        missing_size.append((suite_group, jit_rows, rows_with_size))

con.close()

if missing_size:
    details = ", ".join(
        f"{suite_group} ({rows_with_size}/{jit_rows} rows with size metadata)"
        for suite_group, jit_rows, rows_with_size in missing_size
    )
    raise SystemExit(f"Size-metadata validation failed: {details}.")
PY

echo
echo "JIT-overhead TU-scaling study stored as '$STUDY' in '$DB'."
echo "Report directory: $REPORT_DIR"
echo "Use this study name with:"
echo "  benchmarks/reporting/plot_jit_overhead_vs_tu_size.py --sqlite-polybench-study-name '$STUDY'"
