#!/usr/bin/env bash
# Run the thesis-cited UC first-call quadrant study under best-practice controls.
#
# This rerun covers:
#   - all UC benchmark families used in the thesis
#   - all four input sizes
#   - all three abstraction levels (low, abstract, tradeoff)
#   - two configurations:
#       1. default pipeline
#       2. per-kernel max(U/S) pipeline mapping from the RQ4 export

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
STAMP="$(date +%Y%m%d_%H%M%S)"
STUDY="corpus_uc_first_call_quadrants_${STAMP}"
REPORT_DIR="reports/${STAMP}-thesis-uc-first-call-quadrants"
BEST_UC_PIPELINES_JSON="$SCRIPT_DIR/reports/thesis-figures/rq4/best_uc_pipelines.json"
FILTER='^(BM_g:(uc2|uc7|uc8|uc12|uc14)_[a-z]+;.*a:(low|abstract|tradeoff);.*s:(SMALL|MEDIUM|LARGE|EXTRALARGE);.*t:(jit_overhead|specialized_exec|unspecialized).*$|BM_g:uc1_sql;n:(column_scan|multi_predicate);.*a:(low|abstract|tradeoff);.*s:(SMALL|MEDIUM|LARGE|EXTRALARGE);.*t:(jit_overhead|specialized_exec|unspecialized).*$|BM_g:uc1_sql;n:count_matching_rows;.*a:(low|tradeoff);.*s:(SMALL|MEDIUM|LARGE|EXTRALARGE);.*t:(jit_overhead|specialized_exec|unspecialized).*$|BM_g:uc1_sql;n:count_matching_rows;.*a:abstract;.*s:(SMALL|MEDIUM|LARGE);.*t:(jit_overhead|specialized_exec|unspecialized).*$|BM_g:uc1_sql;n:count_matching_rows;.*a:abstract;.*s:EXTRALARGE;.*t:(jit_overhead|unspecialized).*$)'
TIMEOUT_SECONDS=21600
RQ1_OUTPUT_DIR="$SCRIPT_DIR/reports/thesis-figures/rq1"
RQ1_CSV="$RQ1_OUTPUT_DIR/rq1_perf_stats.csv"
RQ1_TYPST="$RQ1_OUTPUT_DIR/rq1_perf_stats_table.typ"
RQ1_BOXPLOT="$RQ1_OUTPUT_DIR/rq1_perf_stats_speedup_boxplots.png"
RQ1_QUADRANTS_PNG="$RQ1_OUTPUT_DIR/rq1_first_call_quadrants.png"
RQ1_QUADRANTS_CSV="$RQ1_OUTPUT_DIR/rq1_first_call_quadrants.csv"
RQ1_QUADRANTS_TYPST="$RQ1_OUTPUT_DIR/rq1_first_call_quadrants.typ"
EXPECTED_PHASE_ROWS=6470
EXPECTED_CONFIGS=2
EXPECTED_KERNELS=18
EXPECTED_ABSTRACTIONS=3
EXPECTED_SIZES=4
EXPECTED_PHASE_TUPLES=1296

if [[ ! -f "$BEST_UC_PIPELINES_JSON" ]]; then
    echo "Error: missing UC pipeline config JSON: $BEST_UC_PIPELINES_JSON" >&2
    echo "Generate it first via benchmarks/reporting/export_best_uc_pipelines.py." >&2
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
CONFIG_JSON="$REPORT_DIR/configs.json"

if [[ -e "$CONFIG_JSON" && ! -w "$CONFIG_JSON" ]]; then
    echo "Error: config path is not writable: $CONFIG_JSON" >&2
    echo "An older run likely created it via sudo. Remove or chown that file, then rerun." >&2
    exit 1
fi

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
  --benchmark-filter "$FILTER" \
  --reps 5 \
  --timeout "$TIMEOUT_SECONDS" \
  --benchmarking-best-practice

python3 - "$DB" "$STUDY" "$EXPECTED_PHASE_ROWS" "$EXPECTED_CONFIGS" "$EXPECTED_KERNELS" "$EXPECTED_ABSTRACTIONS" "$EXPECTED_SIZES" "$EXPECTED_PHASE_TUPLES" <<'PY'
import sys
import duckdb

db = sys.argv[1]
study = sys.argv[2]
expected_phase_rows = int(sys.argv[3])
expected_configs = int(sys.argv[4])
expected_kernels = int(sys.argv[5])
expected_abstractions = int(sys.argv[6])
expected_sizes = int(sys.argv[7])
expected_phase_tuples = int(sys.argv[8])
con = duckdb.connect(db, read_only=True)
rows = con.execute(
    """
    WITH scoped_runs AS (
        SELECT DISTINCT run_id, config_name
        FROM ablation_studies
        WHERE (study_name = ? OR study_name LIKE ?)
          AND run_id IS NOT NULL
    ),
    scoped_phase_rows AS (
        SELECT
            r.run_id,
            r.config_name,
            p.kernel,
            p.kv_a AS abstraction,
            p.kv_s AS size,
            p.phase,
            c.best_practice_full
        FROM scoped_runs r
        JOIN v_parsed p USING (run_id)
        JOIN context c USING (run_id)
        WHERE p."group" IN ('uc1_sql', 'uc2_conv', 'uc7_dfa', 'uc8_ivm', 'uc12_groupby', 'uc14_sort')
          AND p.kv_a IN ('low', 'abstract', 'tradeoff')
          AND p.kv_s IN ('SMALL', 'MEDIUM', 'LARGE', 'EXTRALARGE')
          AND p.phase IN ('jit_overhead', 'specialized_exec', 'unspecialized')
    )
    SELECT
        COUNT(*) AS phase_rows,
        COUNT(DISTINCT run_id) AS total_runs,
        COUNT(DISTINCT CASE WHEN best_practice_full THEN run_id END) AS best_practice_runs,
        COUNT(DISTINCT config_name) AS config_count,
        COUNT(DISTINCT kernel) AS kernel_count,
        COUNT(DISTINCT abstraction) AS abstraction_count,
        COUNT(DISTINCT size) AS size_count,
        COUNT(DISTINCT config_name || '|' || kernel || '|' || abstraction || '|' || size || '|' || phase) AS phase_tuple_count
    FROM scoped_phase_rows
    """,
    [study, f"{study}__%"],
).fetchone()
con.close()
phase_rows = int(rows[0] or 0)
total_runs = int(rows[1] or 0)
best_practice_runs = int(rows[2] or 0)
config_count = int(rows[3] or 0)
kernel_count = int(rows[4] or 0)
abstraction_count = int(rows[5] or 0)
size_count = int(rows[6] or 0)
phase_tuple_count = int(rows[7] or 0)
print(f"Study: {study}")
print(f"Phase rows recorded: {phase_rows}")
print(f"Runs recorded: {total_runs}")
print(f"Runs with best_practice_full=TRUE: {best_practice_runs}")
print(f"Configs covered: {config_count}")
print(f"Kernels covered: {kernel_count}")
print(f"Abstractions covered: {abstraction_count}")
print(f"Sizes covered: {size_count}")
print(f"Distinct config/kernel/abstraction/size/phase tuples: {phase_tuple_count}")
if total_runs == 0 or phase_rows == 0:
    raise SystemExit("No runs were recorded for the study.")
if phase_rows != expected_phase_rows:
    raise SystemExit(
        f"Coverage validation failed: expected {expected_phase_rows} phase rows, got {phase_rows}."
    )
if best_practice_runs != total_runs:
    raise SystemExit(
        f"Best-practice validation failed: {best_practice_runs}/{total_runs} runs have best_practice_full=TRUE."
    )
if config_count != expected_configs:
    raise SystemExit(f"Expected {expected_configs} configs, got {config_count}.")
if kernel_count != expected_kernels:
    raise SystemExit(f"Expected {expected_kernels} kernels, got {kernel_count}.")
if abstraction_count != expected_abstractions:
    raise SystemExit(f"Expected {expected_abstractions} abstractions, got {abstraction_count}.")
if size_count != expected_sizes:
    raise SystemExit(f"Expected {expected_sizes} sizes, got {size_count}.")
if phase_tuple_count != expected_phase_tuples:
    raise SystemExit(
        f"Expected {expected_phase_tuples} distinct config/kernel/abstraction/size/phase tuples, got {phase_tuple_count}."
    )
PY

mkdir -p "$RQ1_OUTPUT_DIR"

python3 benchmarks/reporting/export_rq1_perf_stats.py \
  --db "$DB" \
  --study-name "$STUDY" \
  --abstraction low \
  --size MEDIUM \
  --csv-out "$RQ1_CSV" \
  --typst-out "$RQ1_TYPST"

MPLCONFIGDIR="${MPLCONFIGDIR:-/tmp/matplotlib-runtime-specialization}" \
python3 benchmarks/reporting/plot_rq1_perf_stats_boxplots.py \
  --csv "$RQ1_CSV" \
  --output "$RQ1_BOXPLOT"

python3 benchmarks/reporting/plot_rq1_first_call_quadrants.py \
  --db "$DB" \
  --study-name "$STUDY" \
  --output-dir "$RQ1_OUTPUT_DIR"

cat > "$RQ1_QUADRANTS_TYPST" <<'TYPST'
#figure(
  image("../llvm/runtime-specialization/benchmarks/reports/thesis-figures/rq1/rq1_first_call_quadrants.png"),
  caption: [
    First-call profitability for the canonical UC rerun across all size classes and abstraction levels.
    The x-axis is the ratio $U / U_p$, where $U_p$ is derived from the MEDIUM-sized execution-speedup and @jit-overhead medians for the same kernel, abstraction, and pipeline.
    The y-axis is the measured first-call speedup $U / (J + S)$.
    Points in the top-right and bottom-left quadrants match the prediction that $U > U_p$ implies a profitable first specialized call.
  ],
) <rq1-first-call-quadrants>
TYPST

echo
echo "Quadrant study stored as '$STUDY' in '$DB'."
echo "Generated RQ1 artifacts:"
echo "  $RQ1_CSV"
echo "  $RQ1_TYPST"
echo "  $RQ1_BOXPLOT"
echo "  $RQ1_QUADRANTS_PNG"
echo "  $RQ1_QUADRANTS_CSV"
echo "  $RQ1_QUADRANTS_TYPST"
