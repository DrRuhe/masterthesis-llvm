#!/usr/bin/env bash
# Run the thesis PolyBench partial-specialization study under best-practice
# benchmark controls using the default CRS pipeline only.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

if [[ $# -lt 1 || $# -gt 2 ]]; then
    echo "Usage: $0 <polybench_benchmark_binary> [duckdb_path]" >&2
    exit 1
fi

BINARY="$1"
DB="${2:-benchmarks.duckdb}"
RUN_STAMP="$(date +%Y%m%d_%H%M%S)"
STUDY="polybench_partial_default_thesis_${RUN_STAMP}"
REPORT_DIR="reports/${RUN_STAMP}-thesis-polybench-partial"
CONFIG_JSON="$REPORT_DIR/configs.json"
TIMEOUT_SECONDS=3600

mkdir -p "$REPORT_DIR"

cat > "$CONFIG_JSON" <<JSON
[
  {"name": "default", "env": {}}
]
JSON

python3 ablation_benchmarks.py "$BINARY" \
  --db "$DB" \
  --study-name "$STUDY" \
  --configs "$CONFIG_JSON" \
  --benchmark-filter 'BM_g:polybench;.*s:(SMALL|MEDIUM|LARGE|EXTRALARGE);.*t:(jit_overhead|specialized_exec|unspecialized)' \
  --reps 5 \
  --timeout "$TIMEOUT_SECONDS" \
  --benchmarking-best-practice

python3 - "$DB" "$STUDY" <<'PY'
import sys
import duckdb

db, study = sys.argv[1], sys.argv[2]
con = duckdb.connect(db, read_only=True)
rows = con.execute(
    """
    SELECT COUNT(*) AS total_runs,
           SUM(CASE WHEN c.best_practice_full THEN 1 ELSE 0 END) AS best_practice_runs
    FROM ablation_studies a
    LEFT JOIN context c USING (run_id)
    WHERE a.study_name = ?
    """,
    [study],
).fetchone()
print(f"Study: {study}")
print(f"Runs recorded: {rows[0]}")
print(f"Runs with best_practice_full=TRUE: {rows[1] or 0}")
PY

python3 reporting/export_polybench_partial_specialization.py \
  --db "$DB" \
  --study-name "$STUDY" \
  --output-dir "$SCRIPT_DIR/reports/thesis-figures/rq3"

echo "PolyBench partial-specialization study stored as '$STUDY' in '$DB'."
