#!/usr/bin/env bash
# Run the thesis-cited UC corpus comparison under the best-practice benchmark
# setup.
#
# Per thesis.typ §RQ1, this final absolute UC run compares exactly two pipeline
# modes on the thesis corpus filter:
#   1. the library default pipeline
#   2. the best amortized-speedup pipeline per UC kernel from the RQ4 export
#
# This keeps the exploratory optimization workflow untouched and gives the thesis
# writeup a single, explicit collection path for final absolute numbers.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

if [[ $# -lt 1 || $# -gt 2 ]]; then
    echo "Usage: $0 <all_benchmarks_binary> [duckdb_path]" >&2
    exit 1
fi

BINARY="$1"
DB="${2:-benchmarks.duckdb}"
DATE="$(date +%Y%m%d)"
STUDY="corpus_uc_final_thesis_${DATE}"
REPORT_DIR="reports/${DATE}-thesis-uc-final"
CONFIG_JSON="$REPORT_DIR/configs.json"
BEST_UC_PIPELINES_JSON="$SCRIPT_DIR/reports/thesis-figures/rq4/best_uc_pipelines.json"

if [[ ! -f "$BEST_UC_PIPELINES_JSON" ]]; then
    echo "Error: missing UC pipeline config JSON: $BEST_UC_PIPELINES_JSON" >&2
    echo "Generate it first, e.g. via benchmarks/reporting/export_best_uc_pipelines.py." >&2
    exit 1
fi

mkdir -p "$REPORT_DIR"

cat > "$CONFIG_JSON" <<JSON
[
  {"name": "default", "env": {}},
  {"name": "per_uc_best", "env": {"CRS_UC_PIPELINE_CONFIG_JSON": "$BEST_UC_PIPELINES_JSON"}}
]
JSON

python3 ablation_benchmarks.py "$BINARY" \
  --db "$DB" \
  --study-name "$STUDY" \
  --configs "$CONFIG_JSON" \
  --benchmark-filter 'BM_g:(uc1|uc2|uc7|uc8|uc12|uc14)_[a-z]+;.*a:low;.*s:MEDIUM;.*t:(jit_overhead|specialized_exec|unspecialized)' \
  --reps 5 \
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

echo "Final UC thesis study stored as '$STUDY' in '$DB'."
