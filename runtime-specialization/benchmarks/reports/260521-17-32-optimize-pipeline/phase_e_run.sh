#!/usr/bin/env bash
set -e
cd /home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization

for spec in \
    "fixpoint_max:2,3,5,8,15,30" \
    "early_prune:0,1" \
    "o3_final:0,1" \
    "pipeline:0,1" \
    "p1_inline_threshold:50,100,225,500,1000,2000" \
    "p1_max_module_growth:1.0,1.5,2.0,3.0,4.0,5.0"; do
    param="${spec%%:*}"
    sweep="${spec##*:}"
    echo "=== Sweeping $param values=$sweep ==="
    python3 benchmarks/sensitivity_analysis.py "/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/llvm/build/release/tools/runtime-specialization/benchmarks/AllBenchmarks" \
        --db benchmarks/benchmarks.duckdb --study-name "sens_uc_iter2_20260521" \
        --optimal-config "benchmarks/reports/260521-17-32-optimize-pipeline/best_uc_optim_iter2_20260521.json" \
        --param "$param" --sweep-values "$sweep" \
        --benchmark-filter 'BM_g:(uc1|uc2|uc7|uc8|uc12|uc14)_[a-z]+;.*a:low;.*s:MEDIUM;.*t:(jit_overhead|specialized_exec|unspecialized)' \
        --reps 5
done
echo "Phase E done."
