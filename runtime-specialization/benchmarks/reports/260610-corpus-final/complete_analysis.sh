#!/bin/bash
# Post-ablation completion script for corpus_uc_final_20260610
# Run from benchmarks/ directory after the ablation study completes (all 9 configs × 5 reps)

set -e
DB="benchmarks.duckdb"
OUTDIR="reports/260610-corpus-final"
PARETO_DIR="reports/260610-pareto"

echo "=== corpus_uc_final_20260610 Post-Ablation Analysis ==="
echo ""

# 1. Per-kernel speedup table
echo "--- Per-kernel speedup table ---"
duckdb "$DB" "
SELECT config_name, kernel,
       ROUND(med_unspec_ns::DOUBLE/med_spec_ns, 3) AS speedup, n_reps
FROM v_ablation_medians
WHERE study_name='corpus_uc_final_20260610'
  AND med_spec_ns IS NOT NULL
ORDER BY config_name, speedup DESC;
" > "$OUTDIR/speedup_summary.txt" 2>&1

echo "  Saved to $OUTDIR/speedup_summary.txt"

# 2. Config aggregate summary
echo ""
echo "--- Config aggregate summary ---"
duckdb "$DB" "
SELECT config_name,
       COUNT(DISTINCT kernel) as kernels,
       ROUND(SUM(med_unspec_ns)/SUM(med_spec_ns), 3) AS sum_speedup,
       ROUND(EXP(AVG(LN(GREATEST(med_unspec_ns::DOUBLE/med_spec_ns, 0.001)))), 3) AS geomean_speedup
FROM v_ablation_medians
WHERE study_name='corpus_uc_final_20260610'
  AND med_spec_ns IS NOT NULL
GROUP BY config_name ORDER BY geomean_speedup DESC;
" | tee -a "$OUTDIR/speedup_summary.txt"

# 3. Pareto plots (regenerate with all 9 configs)
echo ""
echo "--- Generating Pareto plots ---"
python3 reporting/plot_pareto_configs.py \
  --db "$DB" \
  --study-name corpus_uc_final_20260610 \
  --output-dir "$PARETO_DIR" \
  --per-group

echo ""
echo "=== Analysis complete ==="
echo "Files:"
echo "  $OUTDIR/speedup_summary.txt"
ls "$PARETO_DIR"/pareto_corpus_uc_final_20260610_*.png 2>/dev/null | wc -l | xargs echo "  Pareto plots:"
