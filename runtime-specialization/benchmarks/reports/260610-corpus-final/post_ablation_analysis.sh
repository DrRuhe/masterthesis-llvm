#!/bin/bash
# Post-ablation analysis for corpus_uc_final_20260610
# Run after the ablation study completes.
# Must be run from: benchmarks/ directory

set -e
DB="benchmarks.duckdb"
OUTDIR="reports/260610-corpus-final"

echo "=== Phase 3: Corpus Final Ablation Analysis ==="
echo ""

echo "--- Speedup Summary ---"
duckdb "$DB" "
SELECT
    config_name,
    kernel,
    ROUND(MEDIAN(t_spec_ns)/1e6, 2) AS spec_ms,
    ROUND(MEDIAN(t_unspec_ns)/1e6, 2) AS unspec_ms,
    ROUND(MEDIAN(t_unspec_ns) / MEDIAN(t_spec_ns), 3) AS speedup,
    COUNT(*) AS n_reps
FROM v_ablation_results
WHERE study_name = 'corpus_uc_final_20260610'
  AND t_spec_ns IS NOT NULL
  AND t_unspec_ns IS NOT NULL
GROUP BY config_name, kernel
ORDER BY config_name, speedup DESC;
" > "$OUTDIR/speedup_summary.txt" 2>&1
echo "Speedup summary saved to $OUTDIR/speedup_summary.txt"

echo ""
echo "--- All configs summary ---"
duckdb "$DB" "
SELECT
    config_name,
    COUNT(DISTINCT kernel) AS kernels,
    ROUND(SUM(MEDIAN(t_spec_ns)/1e6), 1) AS total_spec_ms,
    ROUND(SUM(MEDIAN(t_unspec_ns)/1e6), 1) AS total_unspec_ms,
    ROUND(SUM(MEDIAN(t_unspec_ns)) / SUM(MEDIAN(t_spec_ns)), 3) AS geomean_approx_speedup,
    COUNT(*) AS n_reps_total
FROM v_ablation_results
WHERE study_name = 'corpus_uc_final_20260610'
  AND t_spec_ns IS NOT NULL
  AND t_unspec_ns IS NOT NULL
GROUP BY config_name
ORDER BY geomean_approx_speedup DESC;
" | tee -a "$OUTDIR/speedup_summary.txt"

echo ""
echo "Analysis complete. Files saved to $OUTDIR/"
