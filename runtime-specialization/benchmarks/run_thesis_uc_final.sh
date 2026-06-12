#!/usr/bin/env bash
# Run the thesis-cited UC corpus comparison under the best-practice benchmark setup.
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

mkdir -p "$REPORT_DIR"

cat > "$CONFIG_JSON" <<'JSON'
[
  {"name": "default", "env": {}},
  {"name": "no_o3_final", "env": {"CRS_DEFAULT_O3_FINAL": "0"}},
  {
    "name": "p0_o3_optimal",
    "env": {
      "CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS": "16",
      "CRS_DEFAULT_LOOP_UNROLL_COUNT": "62",
      "CRS_DEFAULT_P0_LARGE_MODULE_INSTR_THRESHOLD": "3",
      "CRS_DEFAULT_EARLY_PRUNE": "1",
      "CRS_DEFAULT_O3_FINAL": "1",
      "CRS_DEFAULT_PIPELINE": "0"
    }
  },
  {
    "name": "uc_workload_optimal",
    "env": {
      "CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS": "22",
      "CRS_DEFAULT_LOOP_UNROLL_COUNT": "5",
      "CRS_DEFAULT_P2_LARGE_MODULE_INSTR_THRESHOLD": "1",
      "CRS_DEFAULT_EARLY_PRUNE": "1",
      "CRS_DEFAULT_O3_FINAL": "1",
      "CRS_DEFAULT_PIPELINE": "2",
      "CRS_P2_MIN_FUNC_SIZE": "34",
      "CRS_P2_MAX_CLONES": "16",
      "CRS_P2_FUNC_SPEC_ITERS": "4",
      "CRS_P2_FORCE_SPEC": "1",
      "CRS_P2_SPEC_ON_ADDR": "1",
      "CRS_P2_SPEC_LITERAL": "1"
    }
  }
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
