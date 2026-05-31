#!/usr/bin/env bash
# run_evaluation.sh — Orchestration: smoke -> Exp A -> B -> C -> E -> R -> (manual) F
#
# All outputs of a single run are written to a per-run timestamped folder under
# benchmarks/reports/<YYMMDD-HH-MM>-optimize-pipeline/. The folder contains:
#   eval_run_log.txt, best_<study>.json, uc_workload_optimal.json,
#   configs/ablation_uc.json, configs/transfer_configs.json,
#   and (in Phase R) all PNG/CSV plot outputs.
#
# Iter-3 (spec 007, spec 015):
#   * Phase A: 50 trials, pipeline search space now includes P2 (choices 0,1,2).
#     P2-specific knobs (p2_min_func_size, p2_max_clones, p2_func_spec_iters,
#     p2_force_spec) sampled conditionally when pipeline=2.
#   * Phase B: ablation adds three P2 configs (default, force_spec, aggressive).
#   * Phase E: pipeline sweep extended to 0,1,2. P2-specific param sweeps added
#     (p2_min_func_size, p2_max_clones, p2_func_spec_iters, p2_force_spec) run
#     from a fixed P2 baseline so they are informative regardless of the Phase A
#     winner.
#   * Study names suffixed with iter3.
#   * Phase B/C: 5 reps (unchanged from iter-2).
#   * Filter regex unchanged (post-spec-011 group names).
#
# Iter-2 reference: benchmarks/reports/260521-17-32-optimize-pipeline/reflection.md

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

UC_BINARY=""
TPCH_BINARY=""
SMOKE_ONLY=0
PHASE_ONLY=""
DB="benchmarks.duckdb"
DATE="$(date +%Y%m%d)"
TS="$(date +%y%m%d-%H-%M)"

# Shared filter regex constants (iter-3; spec 007 T041).
# Post-spec-011 group names look like "uc1_sql", "uc2_conv", ... so we anchor
# with `_[a-z]+` between the numeric group ID and `;`. The `a:low` constraint
# selects the lowest abstraction variant per group (mirrors iter-1 coverage).
UC_FILTER='BM_g:(uc1|uc2|uc7|uc8|uc12|uc14)_[a-z]+;.*a:low;.*s:MEDIUM;.*t:(jit_overhead|specialized_exec)'
UC_FILTER_WITH_UNSPEC='BM_g:(uc1|uc2|uc7|uc8|uc12|uc14)_[a-z]+;.*a:low;.*s:MEDIUM;.*t:(jit_overhead|specialized_exec|unspecialized)'
TPCH_FILTER='g:tpch;.*t:(jit_overhead|specialized_exec|unspecialized)'
SMOKE_UC_FILTER='BM_g:(uc7|uc8)_[a-z]+;.*a:low;.*s:SMALL;.*t:(jit_overhead|specialized_exec)'
SMOKE_UC_FILTER_WITH_UNSPEC='BM_g:(uc7|uc8)_[a-z]+;.*a:low;.*s:SMALL;.*t:(jit_overhead|specialized_exec|unspecialized)'

usage() {
    cat <<USAGE
Usage: $0 <uc_binary> <tpch_binary> [--smoke-only] [--phase-only=A|B|C|E|R] [--db PATH]

Options:
  --smoke-only             Run Phase 0 (smoke) only and exit.
  --phase-only=PHASE       Run a single phase (A, B, C, E, or R) plus Phase 0
                           smoke (Phase R skips smoke). Phases B/C/E/R discover
                           the most recent prior iter-3 study artifacts; exits
                           non-zero if those prerequisites cannot be located.
  --db PATH                DuckDB file (default: benchmarks.duckdb).
USAGE
    exit 1
}

[[ $# -lt 2 ]] && usage
UC_BINARY="$1"; shift
TPCH_BINARY="$1"; shift
while [[ $# -gt 0 ]]; do
    case "$1" in
        --smoke-only) SMOKE_ONLY=1 ;;
        --phase-only=*) PHASE_ONLY="${1#*=}" ;;
        --phase-only) shift; PHASE_ONLY="$1" ;;
        --db) shift; DB="$1" ;;
        -h|--help) usage ;;
        *) echo "Unknown option: $1"; usage ;;
    esac
    shift
done

if [[ -n "$PHASE_ONLY" ]]; then
    case "$PHASE_ONLY" in
        A|B|C|E|R) ;;
        *) echo "Invalid --phase-only value: $PHASE_ONLY (expected A|B|C|E|R)"; exit 1 ;;
    esac
fi

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
echo "Phase-only: ${PHASE_ONLY:-<full chain>}" >> "$LOG"
echo "Report dir: $REPORT_DIR"

log() { echo "$@" | tee -a "$LOG"; }

# Helpers: discover prior iter-3 study artifacts when running a single phase.

# discover_latest_study_a: queries $DB for the most recent (by completed_at,
# then started_at) study whose name matches `uc_optim_iter3_%`. Echoes the
# study name to stdout; returns non-zero if none found.
discover_latest_study_a() {
    python3 - "$DB" <<'PY' || return 1
import sys
import duckdb
db = sys.argv[1]
try:
    con = duckdb.connect(db, read_only=True)
except Exception as exc:
    print(f"ERROR: cannot open DB {db}: {exc}", file=sys.stderr)
    sys.exit(2)
row = con.execute(
    """
    SELECT study_name
    FROM optimization_sessions
    WHERE study_name LIKE 'uc_optim_iter3_%'
      AND status = 'complete'
    ORDER BY COALESCE(completed_at, started_at) DESC NULLS LAST
    LIMIT 1
    """
).fetchone()
if not row:
    print("ERROR: no completed iter-3 Phase A study (uc_optim_iter3_%) found in DB",
          file=sys.stderr)
    sys.exit(3)
print(row[0])
PY
}

# discover_latest_best_json: locates the most recent best_uc_optim_iter3_<DATE>.json
# under benchmarks/reports/*-optimize-pipeline/. Echoes the file path on success.
discover_latest_best_json() {
    # shellcheck disable=SC2012
    local found
    found=$(ls -1t reports/*-optimize-pipeline/best_uc_optim_iter3_*.json 2>/dev/null | head -n 1 || true)
    if [[ -z "$found" ]]; then
        return 1
    fi
    echo "$found"
}

# write_uc_workload_optimal_from_db <study> <out_path>
# Reuses the same env-var extraction logic that the full chain runs after
# Phase A. Used by --phase-only=B and --phase-only=C.
write_uc_workload_optimal_from_db() {
    local study="$1"
    local out_path="$2"
    OUT_PATH="$out_path" python3 - "$DB" "$study" <<'PY'
import sys, os, json, duckdb
from pathlib import Path
db, study = sys.argv[1], sys.argv[2]
out_path = Path(os.environ["OUT_PATH"])
con = duckdb.connect(db, read_only=True)
rows = con.execute(
    "SELECT params_json FROM v_optim_best_per_kernel WHERE study_name=? LIMIT 1",
    [study],
).fetchone()
if not rows:
    print(f"ERROR: no best config found for study {study}", file=sys.stderr)
    sys.exit(3)
params = json.loads(rows[0])
env = {}
param_env_map = {
    "fixpoint_max":         "CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS",
    "unroll_max":           "CRS_DEFAULT_LOOP_UNROLL_COUNT",
    "large_module_max":     "CRS_DEFAULT_LARGE_MODULE_INSTR_THRESHOLD",
    "early_prune":          "CRS_DEFAULT_EARLY_PRUNE",
    "o3_final":             "CRS_DEFAULT_O3_FINAL",
    "pipeline":             "CRS_DEFAULT_PIPELINE",
    "p1_inline_threshold":  "CRS_DEFAULT_P1_INLINE_THRESHOLD",
    "p1_max_module_growth": "CRS_DEFAULT_P1_MAX_MODULE_GROWTH",
    "p2_min_func_size":     "CRS_P2_MIN_FUNC_SIZE",
    "p2_max_clones":        "CRS_P2_MAX_CLONES",
    "p2_func_spec_iters":   "CRS_P2_FUNC_SPEC_ITERS",
    "p2_force_spec":        "CRS_P2_FORCE_SPEC",
}
for k, v in params.items():
    if k in param_env_map:
        env[param_env_map[k]] = str(v)
    elif "_off" not in k and "_val" not in k:
        env[k] = str(v)
out_path.parent.mkdir(parents=True, exist_ok=True)
with open(out_path, "w") as f:
    json.dump([{"name": "uc_workload_optimal", "env": env}], f, indent=2)
print(f"Wrote {out_path}")
PY
}

# Should we run a given phase? Returns true if running the full chain or if the
# requested --phase-only matches.
should_run_phase() {
    local phase="$1"
    if [[ -z "$PHASE_ONLY" ]]; then
        return 0
    fi
    [[ "$PHASE_ONLY" == "$phase" ]]
}

# Phase 0: Smoke test (skipped for --phase-only=R)
if [[ "$PHASE_ONLY" != "R" ]]; then
    log "--- Phase 0: Smoke test ---"
    python3 create_db.py --db eval_smoke.duckdb 2>/dev/null || true

    python3 optimize_benchmarks.py "$UC_BINARY" \
        --db eval_smoke.duckdb \
        --study-name smoke_uc_A \
        --n-trials 10 --seed 0 --n-parallel 1 \
        --benchmark-filter "$SMOKE_UC_FILTER" \
        --apply-default-filters || { log "SMOKE FAILED: Exp A smoke"; exit 1; }

    python3 ablation_benchmarks.py "$UC_BINARY" \
        --db eval_smoke.duckdb \
        --study-name smoke_ablation \
        --configs configs/ablation_subset.json \
        --benchmark-filter "$SMOKE_UC_FILTER_WITH_UNSPEC" \
        --reps 3 || { log "SMOKE FAILED: Exp B smoke"; exit 1; }

    python3 -c "
import duckdb
c = duckdb.connect('eval_smoke.duckdb')
rows = c.execute(\"SELECT study_name, COUNT(*) FROM v_ablation_medians WHERE study_name='smoke_ablation' GROUP BY 1\").fetchall()
assert rows, 'smoke_ablation: no rows in v_ablation_medians'
print('Smoke B OK:', rows)
" || { log "SMOKE FAILED: B verification"; exit 1; }

    log "Smoke test PASSED"
else
    log "--phase-only=R: skipping Phase 0 smoke."
fi

[[ $SMOKE_ONLY -eq 1 ]] && { log "Smoke-only mode -- done."; exit 0; }

# ---------------------------------------------------------------------------
# Phase A: UC optimization (iter-3: 50 trials; pipeline search space 0,1,2)
# ---------------------------------------------------------------------------
STUDY_A="uc_optim_iter3_${DATE}"
if should_run_phase A; then
    log "--- Phase A: UC optimization (iter-3: 50 trials, P0+P1+P2) ---"
    python3 optimize_benchmarks.py "$UC_BINARY" \
        --db "$DB" \
        --study-name "$STUDY_A" \
        --n-trials 50 --seed 42 --n-parallel 1 \
        --benchmark-filter "$UC_FILTER" \
        --apply-default-filters \
        --output-best "$REPORT_DIR/best_${STUDY_A}.json"
    log "STUDY_A=$STUDY_A" >> "$LOG"
    echo "STUDY_A=$STUDY_A"

    # Extract best config from the just-completed study.
    write_uc_workload_optimal_from_db "$STUDY_A" "$REPORT_DIR/uc_workload_optimal.json"
fi

# ---------------------------------------------------------------------------
# Phase B: Ablation
# ---------------------------------------------------------------------------
STUDY_B="ablation_uc_iter3_${DATE}"
if should_run_phase B; then
    log "--- Phase B: UC ablation (iter-3: 5 reps, P0+P1+P2) ---"

    # If running B in isolation, discover the most recent iter-3 Phase A study
    # and rebuild uc_workload_optimal.json into the current REPORT_DIR.
    if [[ "$PHASE_ONLY" == "B" ]]; then
        log "--phase-only=B: discovering most recent uc_optim_iter3_% study..."
        if ! prior_a=$(discover_latest_study_a); then
            log "FATAL: --phase-only=B needs a prior completed iter-3 Phase A study (uc_optim_iter3_%); none found in $DB"
            exit 2
        fi
        log "Using prior STUDY_A=$prior_a"
        if ! write_uc_workload_optimal_from_db "$prior_a" "$REPORT_DIR/uc_workload_optimal.json"; then
            log "FATAL: could not extract uc_workload_optimal.json from study $prior_a"
            exit 2
        fi
    fi

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
    {'name': 'aggressive', 'env': {'CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS': '20', 'CRS_DEFAULT_LOOP_UNROLL_COUNT': '256'}},
    opt,
    # Pipeline 1 (budget-aware inlining, spec 014).
    {'name': 'pipeline_1_default', 'env': {'CRS_DEFAULT_PIPELINE': '1'}},
    {'name': 'pipeline_1_aggressive_budget', 'env': {'CRS_DEFAULT_PIPELINE': '1', 'CRS_DEFAULT_P1_INLINE_THRESHOLD': '1000', 'CRS_DEFAULT_P1_MAX_MODULE_GROWTH': '4.0'}},
    {'name': 'pipeline_1_tight_budget', 'env': {'CRS_DEFAULT_PIPELINE': '1', 'CRS_DEFAULT_P1_INLINE_THRESHOLD': '100', 'CRS_DEFAULT_P1_MAX_MODULE_GROWTH': '1.5'}},
    # Pipeline 2 (JIT-IPSCCP + FunctionSpecialization, spec 015).
    {'name': 'pipeline_2_default', 'env': {'CRS_DEFAULT_PIPELINE': '2'}},
    {'name': 'pipeline_2_force_spec', 'env': {'CRS_DEFAULT_PIPELINE': '2', 'CRS_P2_FORCE_SPEC': '1'}},
    {'name': 'pipeline_2_aggressive', 'env': {
        'CRS_DEFAULT_PIPELINE': '2',
        'CRS_P2_MIN_FUNC_SIZE': '1',
        'CRS_P2_MAX_CLONES': '10',
        'CRS_P2_FUNC_SPEC_ITERS': '5',
        'CRS_P2_FORCE_SPEC': '1',
    }},
    # P2 + O3 final: the confirmed optimal config from uc_optim_iter3_20260531.
    {'name': 'pipeline_2_o3_final', 'env': {
        'CRS_DEFAULT_PIPELINE': '2',
        'CRS_DEFAULT_O3_FINAL': '1',
        'CRS_P2_FORCE_SPEC': '1',
        'CRS_P2_SPEC_ON_ADDR': '1',
        'CRS_P2_SPEC_LITERAL': '1',
    }},
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
        --benchmark-filter "$UC_FILTER_WITH_UNSPEC" \
        --reps 5
    log "STUDY_B=$STUDY_B" >> "$LOG"
fi

# ---------------------------------------------------------------------------
# Phase C: Transfer
# ---------------------------------------------------------------------------
STUDY_C_UC="transfer_uc_iter3_${DATE}"
STUDY_C_TPCH="transfer_tpch_iter3_${DATE}"
if should_run_phase C; then
    log "--- Phase C: Transfer (iter-3: 5 reps) ---"

    if [[ "$PHASE_ONLY" == "C" ]]; then
        log "--phase-only=C: discovering most recent uc_optim_iter3_% study..."
        if ! prior_a=$(discover_latest_study_a); then
            log "FATAL: --phase-only=C needs a prior completed iter-3 Phase A study (uc_optim_iter3_%); none found in $DB"
            exit 2
        fi
        log "Using prior STUDY_A=$prior_a"
        if ! write_uc_workload_optimal_from_db "$prior_a" "$REPORT_DIR/uc_workload_optimal.json"; then
            log "FATAL: could not extract uc_workload_optimal.json from study $prior_a"
            exit 2
        fi
    fi

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
        --benchmark-filter "$UC_FILTER_WITH_UNSPEC" \
        --reps 5
    python3 ablation_benchmarks.py "$TPCH_BINARY" \
        --db "$DB" --study-name "$STUDY_C_TPCH" \
        --configs "$REPORT_DIR/configs/transfer_configs.json" \
        --benchmark-filter "$TPCH_FILTER" \
        --reps 5
    log "STUDY_C_UC=$STUDY_C_UC STUDY_C_TPCH=$STUDY_C_TPCH" >> "$LOG"
fi

# ---------------------------------------------------------------------------
# Phase E: Sensitivity
#
# Iter-3: extend iter-2 sweeps with pipeline=2 and P2-specific knobs.
# P2 knobs are swept from a fixed P2 baseline config (pipeline=2, all others
# at default) so they are informative regardless of the Phase A winner.
# fixpoint_max lower bound stays 2 per RQ-15.
# ---------------------------------------------------------------------------
STUDY_E="sens_uc_iter3_${DATE}"
if should_run_phase E; then
    log "--- Phase E: Sensitivity (iter-3: P0+P1 params + P2 params, 5 reps) ---"

    BEST_JSON="$REPORT_DIR/best_${STUDY_A}.json"
    if [[ "$PHASE_ONLY" == "E" ]]; then
        log "--phase-only=E: locating most recent best_uc_optim_iter3_*.json..."
        if ! BEST_JSON=$(discover_latest_best_json); then
            log "FATAL: --phase-only=E needs a prior best_uc_optim_iter3_*.json under benchmarks/reports/*-optimize-pipeline/; none found"
            exit 2
        fi
        log "Using BEST_JSON=$BEST_JSON"
    fi

    if [[ ! -f "$BEST_JSON" ]]; then
        log "FATAL: Phase E best-config JSON not found at $BEST_JSON"
        exit 2
    fi

    # Write a fixed P2 baseline JSON for P2-specific sweeps (pipeline=2, all
    # other P2 knobs at their defaults: min_func_size=1, max_clones=0,
    # func_spec_iters=10, force_spec=0).
    # Format matches optimize_benchmarks.py --output-best: {"trial_id":..., "params":{...}}.
    P2_BASELINE_JSON="$REPORT_DIR/p2_baseline.json"
    cat > "$P2_BASELINE_JSON" <<'JSONEOF'
{"trial_id": "p2_baseline", "params": {"fixpoint_max": 5, "unroll_max": 4, "large_module_max": 0, "early_prune": 1, "o3_final": 1, "pipeline": 2, "p2_min_func_size": 1, "p2_max_clones": 0, "p2_func_spec_iters": 10, "p2_force_spec": 0}}
JSONEOF

    # Sweep list: (param_name, comma_separated_values) from the Phase A best-config baseline.
    # Order: scalars first, then booleans, then P1 budget params, then pipeline choice.
    for param_sweep in \
        "fixpoint_max:2,3,5,8,15,30" \
        "early_prune:0,1" \
        "o3_final:0,1" \
        "pipeline:0,1,2" \
        "p1_inline_threshold:50,100,225,500,1000,2000" \
        "p1_max_module_growth:1.0,1.5,2.0,3.0,4.0,5.0"; do
        param="${param_sweep%%:*}"
        sweep="${param_sweep##*:}"
        log "Phase E: sweeping $param over [$sweep] (from Phase A best-config baseline)"
        python3 sensitivity_analysis.py "$UC_BINARY" \
            --db "$DB" --study-name "$STUDY_E" \
            --optimal-config "$BEST_JSON" \
            --param "$param" --sweep-values "$sweep" \
            --benchmark-filter "$UC_FILTER_WITH_UNSPEC" \
            --reps 5
    done

    # P2-specific sweeps from the fixed P2 baseline.
    for param_sweep in \
        "p2_min_func_size:1,5,10,20,50" \
        "p2_max_clones:0,2,5,10,20" \
        "p2_func_spec_iters:1,3,5,10" \
        "p2_force_spec:0,1"; do
        param="${param_sweep%%:*}"
        sweep="${param_sweep##*:}"
        log "Phase E: sweeping $param over [$sweep] (from P2 baseline)"
        python3 sensitivity_analysis.py "$UC_BINARY" \
            --db "$DB" --study-name "$STUDY_E" \
            --optimal-config "$P2_BASELINE_JSON" \
            --param "$param" --sweep-values "$sweep" \
            --benchmark-filter "$UC_FILTER_WITH_UNSPEC" \
            --reps 5
    done

    log "STUDY_E=$STUDY_E" >> "$LOG"
fi

# ---------------------------------------------------------------------------
# Phase R: Reporting (plots + CSVs into the per-run REPORT_DIR)
# ---------------------------------------------------------------------------
if should_run_phase R; then
    log "--- Phase R: Reporting ---"

    # In --phase-only=R mode, discover the most recent iter-3 study names so
    # we can produce a coherent set of plots. STUDY_B/STUDY_C_* aren't strictly
    # required by every reporting script, but use the same iter-3 suffix
    # pattern, derived from the same date as STUDY_A.
    if [[ "$PHASE_ONLY" == "R" ]]; then
        log "--phase-only=R: discovering most recent uc_optim_iter3_% study..."
        if ! STUDY_A=$(discover_latest_study_a); then
            log "FATAL: --phase-only=R needs a prior completed iter-3 Phase A study; none found in $DB"
            exit 2
        fi
        # STUDY_A looks like uc_optim_iter3_YYYYMMDD; extract date suffix.
        iter3_date="${STUDY_A##*_}"
        STUDY_B="ablation_uc_iter3_${iter3_date}"
        STUDY_C_UC="transfer_uc_iter3_${iter3_date}"
        STUDY_C_TPCH="transfer_tpch_iter3_${iter3_date}"
        STUDY_E="sens_uc_iter3_${iter3_date}"
        log "Using STUDY_A=$STUDY_A STUDY_B=$STUDY_B STUDY_C_UC=$STUDY_C_UC STUDY_C_TPCH=$STUDY_C_TPCH STUDY_E=$STUDY_E"
    fi

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
fi

if [[ -z "$PHASE_ONLY" ]]; then
    log "=== All phases complete. Study names: A=$STUDY_A B=$STUDY_B C_UC=$STUDY_C_UC C_TPCH=$STUDY_C_TPCH E=$STUDY_E ==="
    log "Next: write $REPORT_DIR/reflection.md"
else
    log "=== Phase $PHASE_ONLY complete (single-phase mode). ==="
fi
