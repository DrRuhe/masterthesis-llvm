#!/usr/bin/env bash
# run_evaluation.sh — Orchestration: smoke -> Exp A -> B -> C -> E -> R -> (manual) F
#
# All outputs of a single run are written to a per-run timestamped folder under
# benchmarks/reports/<YYMMDD-HH-MM>-optimize-pipeline/. The folder contains:
#   eval_run_log.txt, best_<study>.json, uc_workload_optimal.json,
#   configs/ablation_uc.json, configs/transfer_configs.json,
#   and (in Phase R) all PNG/CSV plot outputs.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

UC_BINARY=""
TPCH_BINARY=""
SMOKE_ONLY=0
DB="benchmarks.duckdb"
DATE="$(date +%Y%m%d)"
TS="$(date +%y%m%d-%H-%M)"

usage() {
    echo "Usage: $0 <uc_binary> <tpch_binary> [--smoke-only] [--db PATH]"
    exit 1
}

[[ $# -lt 2 ]] && usage
UC_BINARY="$1"; shift
TPCH_BINARY="$1"; shift
while [[ $# -gt 0 ]]; do
    case "$1" in
        --smoke-only) SMOKE_ONLY=1 ;;
        --db) shift; DB="$1" ;;
        *) echo "Unknown option: $1"; usage ;;
    esac
    shift
done

REPORT_BASE="reports/${TS}-optimize-pipeline"
REPORT_DIR="$REPORT_BASE"
suffix=0
while [[ -d "$REPORT_DIR" ]]; do
    suffix=$((suffix+1))
    REPORT_DIR="${REPORT_BASE}-${suffix}"
done
mkdir -p "$REPORT_DIR/configs"
export REPORT_DIR
LOG="$REPORT_DIR/eval_run_log.txt"

echo "=== eval run: $(date) ===" >> "$LOG"
echo "Report dir: $REPORT_DIR" >> "$LOG"
echo "UC binary: $UC_BINARY" >> "$LOG"
echo "TPCH binary: $TPCH_BINARY" >> "$LOG"
echo "Report dir: $REPORT_DIR"

log() { echo "$@" | tee -a "$LOG"; }

# Phase 0: Smoke test
log "--- Phase 0: Smoke test ---"
python3 create_db.py --db eval_smoke.duckdb 2>/dev/null || true

python3 optimize_benchmarks.py "$UC_BINARY" \
    --db eval_smoke.duckdb \
    --study-name smoke_uc_A \
    --n-trials 10 --seed 0 --n-parallel 1 \
    --benchmark-filter 'BM_g:(uc7|uc8);.*s:MINI;.*t:(jit_overhead|specialized_exec)' \
    --apply-default-filters || { log "SMOKE FAILED: Exp A smoke"; exit 1; }

python3 ablation_benchmarks.py "$UC_BINARY" \
    --db eval_smoke.duckdb \
    --study-name smoke_ablation \
    --configs configs/ablation_subset.json \
    --benchmark-filter 'BM_g:(uc7|uc8);.*s:MINI;.*t:(jit_overhead|specialized_exec|unspecialized)' \
    --reps 3 || { log "SMOKE FAILED: Exp B smoke"; exit 1; }

python3 -c "
import duckdb
c = duckdb.connect('eval_smoke.duckdb')
rows = c.execute(\"SELECT study_name, COUNT(*) FROM v_ablation_medians WHERE study_name='smoke_ablation' GROUP BY 1\").fetchall()
assert rows, 'smoke_ablation: no rows in v_ablation_medians'
print('Smoke B OK:', rows)
" || { log "SMOKE FAILED: B verification"; exit 1; }

log "Smoke test PASSED"

[[ $SMOKE_ONLY -eq 1 ]] && { log "Smoke-only mode -- done."; exit 0; }

# Phase A: UC optimization (150 trials)
log "--- Phase A: UC optimization ---"
STUDY_A="uc_optim_${DATE}"
python3 optimize_benchmarks.py "$UC_BINARY" \
    --db "$DB" \
    --study-name "$STUDY_A" \
    --n-trials 150 --seed 42 --n-parallel 1 \
    --benchmark-filter 'BM_g:(uc1|uc2|uc7|uc8|uc12|uc14);.*s:MEDIUM;.*t:(jit_overhead|specialized_exec)' \
    --apply-default-filters \
    --output-best "$REPORT_DIR/best_${STUDY_A}.json"
log "STUDY_A=$STUDY_A" >> "$LOG"
echo "STUDY_A=$STUDY_A"

# Extract best config
python3 - "$DB" "$STUDY_A" <<'EOF'
import sys, os, json, duckdb
from pathlib import Path
db, study = sys.argv[1], sys.argv[2]
report_dir = Path(os.environ["REPORT_DIR"])
con = duckdb.connect(db)
rows = con.execute(
    "SELECT params_json FROM v_optim_best_per_kernel WHERE study_name=? LIMIT 1", [study]
).fetchone()
if not rows:
    print("Warning: no best config found")
    sys.exit(0)
out_path = report_dir / "uc_workload_optimal.json"
with open(out_path, "w") as f:
    params = json.loads(rows[0])
    env = {}
    param_env_map = {
        "fixpoint_max": "CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS",
        "unroll_max": "CRS_DEFAULT_LOOP_UNROLL_COUNT",
        "large_module_max": "CRS_DEFAULT_LARGE_MODULE_INSTR_THRESHOLD",
        "early_prune": "CRS_DEFAULT_EARLY_PRUNE",
        "o3_final": "CRS_DEFAULT_O3_FINAL",
        "pipeline": "CRS_DEFAULT_PIPELINE",
    }
    for k, v in params.items():
        if k in param_env_map:
            env[param_env_map[k]] = str(v)
        elif "_off" not in k and "_val" not in k:
            env[k] = str(v)
    json.dump([{"name": "uc_workload_optimal", "env": env}], f, indent=2)
print(f"Wrote {out_path}")
EOF

# Phase B: Ablation
log "--- Phase B: UC ablation ---"
STUDY_B="ablation_uc_${DATE}"
# Build ablation config including uc_workload_optimal
python3 - <<'EOF'
import os, json
from pathlib import Path
report_dir = Path(os.environ["REPORT_DIR"])
with open(report_dir / "uc_workload_optimal.json") as f:
    opt = json.load(f)[0]
configs = [
    {'name': 'default', 'env': {}},
    {'name': 'o3_only', 'env': {'CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS': '1', 'CRS_DEFAULT_EARLY_PRUNE': '0', 'CRS_DEFAULT_O3_FINAL': '1'}},
    {'name': 'no_prune', 'env': {'CRS_DEFAULT_EARLY_PRUNE': '0'}},
    {'name': 'no_o3_final', 'env': {'CRS_DEFAULT_O3_FINAL': '0'}},
    {'name': 'no_unroll', 'env': {'CRS_DEFAULT_LOOP_UNROLL_COUNT': '1'}},
    # fixpoint_1 excluded — fixpoint_max=1 + early_prune=1 (default) + o3_final=1 (default) = SIGSEGV (RQ-15)
    {'name': 'fixpoint_2', 'env': {'CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS': '2'}},
    # pipeline_1 excluded — CRS_DEFAULT_PIPELINE=1 causes core dump on full UC filter (RQ-11)
    {'name': 'aggressive', 'env': {'CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS': '20', 'CRS_DEFAULT_LOOP_UNROLL_COUNT': '256'}},
    opt,
]
out_path = report_dir / "configs" / "ablation_uc.json"
out_path.parent.mkdir(parents=True, exist_ok=True)
with open(out_path, 'w') as f:
    json.dump(configs, f, indent=2)
print(f"Wrote {out_path}")
EOF
python3 ablation_benchmarks.py "$UC_BINARY" \
    --db "$DB" \
    --study-name "$STUDY_B" \
    --configs "$REPORT_DIR/configs/ablation_uc.json" \
    --benchmark-filter 'BM_g:(uc1|uc2|uc7|uc8|uc12|uc14);.*s:MEDIUM;.*t:(jit_overhead|specialized_exec|unspecialized)' \
    --reps 3
log "STUDY_B=$STUDY_B" >> "$LOG"

# Phase C: Transfer
log "--- Phase C: Transfer ---"
STUDY_C_UC="transfer_uc_${DATE}"
STUDY_C_TPCH="transfer_tpch_${DATE}"
python3 - <<'EOF'
import os, json
from pathlib import Path
report_dir = Path(os.environ["REPORT_DIR"])
with open(report_dir / "uc_workload_optimal.json") as f:
    opt = json.load(f)[0]
configs = [
    {'name': 'default', 'env': {}},
    {'name': 'aggressive', 'env': {'CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS': '20', 'CRS_DEFAULT_LOOP_UNROLL_COUNT': '256'}},
    {'name': 'uc_optimal', 'env': opt['env']},
    {'name': 'uc_workload_optimal', 'env': opt['env']},
]
out_path = report_dir / "configs" / "transfer_configs.json"
out_path.parent.mkdir(parents=True, exist_ok=True)
with open(out_path, 'w') as f:
    json.dump(configs, f, indent=2)
print(f"Wrote {out_path}")
EOF
python3 ablation_benchmarks.py "$UC_BINARY" \
    --db "$DB" --study-name "$STUDY_C_UC" \
    --configs "$REPORT_DIR/configs/transfer_configs.json" \
    --benchmark-filter 'BM_g:(uc1|uc2|uc7|uc8|uc12|uc14);.*s:MEDIUM;.*t:(jit_overhead|specialized_exec|unspecialized)' \
    --reps 3
python3 ablation_benchmarks.py "$TPCH_BINARY" \
    --db "$DB" --study-name "$STUDY_C_TPCH" \
    --configs "$REPORT_DIR/configs/transfer_configs.json" \
    --benchmark-filter 'g:tpch;.*t:(jit_overhead|specialized_exec|unspecialized)' \
    --reps 3
log "STUDY_C_UC=$STUDY_C_UC STUDY_C_TPCH=$STUDY_C_TPCH" >> "$LOG"

# Phase E: Sensitivity
log "--- Phase E: Sensitivity ---"
STUDY_E="sens_uc_${DATE}"
BEST_JSON="$REPORT_DIR/best_${STUDY_A}.json"
python3 sensitivity_analysis.py "$UC_BINARY" \
    --db "$DB" --study-name "$STUDY_E" \
    --optimal-config "$BEST_JSON" \
    --param fixpoint_max --sweep-values 1,2,3,5,8,15,30 \
    --benchmark-filter 'BM_g:(uc1|uc2|uc7|uc8|uc12|uc14);.*s:MEDIUM;.*t:(jit_overhead|specialized_exec|unspecialized)' \
    --reps 1
for param_sweep in "unroll_max:1,2,4,8,16,32,64,128,256,512" \
                   "large_module_max:0,1000,5000,10000,50000,100000" \
                   "early_prune:0,1" "o3_final:0,1"; do
    param="${param_sweep%%:*}"
    sweep="${param_sweep##*:}"
    python3 sensitivity_analysis.py "$UC_BINARY" \
        --db "$DB" --study-name "$STUDY_E" \
        --optimal-config "$BEST_JSON" \
        --param "$param" --sweep-values "$sweep" \
        --benchmark-filter 'BM_g:(uc1|uc2|uc7|uc8|uc12|uc14);.*s:MEDIUM;.*t:(jit_overhead|specialized_exec|unspecialized)' \
        --reps 1
done
log "STUDY_E=$STUDY_E" >> "$LOG"

# Phase R: Reporting (plots + CSVs into the per-run REPORT_DIR)
log "--- Phase R: Reporting ---"
IMPORTANCE_JSON="$REPORT_DIR/importance_${STUDY_A}.json"
python3 reporting/plot_pareto.py         --db "$DB" --study-name "$STUDY_A" --output-dir "$REPORT_DIR" \
    || log "plot_pareto failed"
python3 reporting/plot_pareto_configs.py --db "$DB" --study-name "$STUDY_A" --output-dir "$REPORT_DIR" --per-group \
    || log "plot_pareto_configs failed"
python3 reporting/plot_ablation.py       --db "$DB" --study-name "$STUDY_B" --output-dir "$REPORT_DIR" \
    || log "plot_ablation failed"
python3 reporting/plot_breakeven.py      --db "$DB" --study-name "$STUDY_A" --output-dir "$REPORT_DIR" \
    || log "plot_breakeven failed"
if [[ -f "$IMPORTANCE_JSON" ]]; then
    python3 reporting/plot_sensitivity.py --db "$DB" --study-name "$STUDY_E" \
        --importance-json "$IMPORTANCE_JSON" --output-dir "$REPORT_DIR" \
        || log "plot_sensitivity failed"
else
    python3 reporting/plot_sensitivity.py --db "$DB" --study-name "$STUDY_E" \
        --output-dir "$REPORT_DIR" \
        || log "plot_sensitivity failed"
fi

log "=== All phases complete. Study names: A=$STUDY_A B=$STUDY_B C_UC=$STUDY_C_UC C_TPCH=$STUDY_C_TPCH E=$STUDY_E ==="
log "Next: write $REPORT_DIR/reflection.md"
