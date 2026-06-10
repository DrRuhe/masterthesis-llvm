#!/bin/bash
# Phase 6: OAT sensitivity analysis for all 6 pipeline parameters
# Run this after the Phase 3 corpus ablation study completes.
# Must be run from: benchmarks/ directory

set -e
BINARY="/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/llvm/build/release/tools/runtime-specialization/benchmarks/AllBenchmarks"
BASE_CONFIG="reports/260601-15-16-optimize-pipeline/best_uc_optim_iter3_20260601.json"
FILTER='BM_g:uc.*a:low.*s:MEDIUM.*t:(specialized_exec|jit_overhead)'
OUTDIR="reports/260610-sensitivity"

echo "=== Phase 6: Sensitivity Analysis OAT Sweeps ==="
echo "Binary: $BINARY"
echo "Base config: $BASE_CONFIG"
echo "Filter: $FILTER"
echo ""

python3 sensitivity_analysis.py "$BINARY" \
  --study-name sens_uc_fixpoint_20260610 --param fixpoint_max \
  --sweep-values 1,2,4,8,16,24 --reps 3 \
  --optimal-config "$BASE_CONFIG" \
  --benchmark-filter "$FILTER" \
  --db benchmarks.duckdb \
  2>&1 | tee "$OUTDIR/sens_fixpoint.log"

python3 sensitivity_analysis.py "$BINARY" \
  --study-name sens_uc_unroll_20260610 --param unroll_max \
  --sweep-values 1,4,16,32,64,128 --reps 3 \
  --optimal-config "$BASE_CONFIG" \
  --benchmark-filter "$FILTER" \
  --db benchmarks.duckdb \
  2>&1 | tee "$OUTDIR/sens_unroll.log"

python3 sensitivity_analysis.py "$BINARY" \
  --study-name sens_uc_largemod_20260610 --param p0_large_module_max \
  --sweep-values 0,1,2,4,8 --reps 3 \
  --optimal-config "$BASE_CONFIG" \
  --benchmark-filter "$FILTER" \
  --db benchmarks.duckdb \
  2>&1 | tee "$OUTDIR/sens_largemod.log"

python3 sensitivity_analysis.py "$BINARY" \
  --study-name sens_uc_earlyprune_20260610 --param early_prune \
  --sweep-values 0,1 --reps 3 \
  --optimal-config "$BASE_CONFIG" \
  --benchmark-filter "$FILTER" \
  --db benchmarks.duckdb \
  2>&1 | tee "$OUTDIR/sens_earlyprune.log"

python3 sensitivity_analysis.py "$BINARY" \
  --study-name sens_uc_o3final_20260610 --param o3_final \
  --sweep-values 0,1 --reps 3 \
  --optimal-config "$BASE_CONFIG" \
  --benchmark-filter "$FILTER" \
  --db benchmarks.duckdb \
  2>&1 | tee "$OUTDIR/sens_o3final.log"

python3 sensitivity_analysis.py "$BINARY" \
  --study-name sens_uc_pipeline_20260610 --param pipeline \
  --sweep-values 0,2 --reps 3 \
  --optimal-config "$BASE_CONFIG" \
  --benchmark-filter "$FILTER" \
  --db benchmarks.duckdb \
  2>&1 | tee "$OUTDIR/sens_pipeline.log"

echo ""
echo "=== All sensitivity sweeps complete ==="
