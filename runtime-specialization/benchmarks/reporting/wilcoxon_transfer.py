#!/usr/bin/env python3
"""wilcoxon_transfer.py — Cross-workload config transfer statistical test."""
import argparse
import sys
import json
from pathlib import Path
import duckdb

sys.path.insert(0, str(Path(__file__).parent.parent))
from record_benchmark import resolve_db_path
from scipy.stats import wilcoxon


def get_combined_costs(con, study_name):
    """Returns {config_name: [combined_cost_per_group, ...]}"""
    rows = con.execute(
        """SELECT config_name, "group",
                  AVG(med_jit_ns) + AVG(med_spec_ns) AS combined_ns
           FROM v_ablation_medians
           WHERE study_name=? AND med_jit_ns IS NOT NULL AND med_spec_ns IS NOT NULL
           GROUP BY config_name, "group"
           ORDER BY config_name, "group" """,
        [study_name]
    ).fetchall()
    from collections import defaultdict
    d = defaultdict(list)
    for config, group, cost in rows:
        d[config].append(cost)
    return dict(d)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--db", default=None)
    parser.add_argument("--uc-study", required=True)
    parser.add_argument("--tpch-study", required=True)
    args = parser.parse_args()

    db_path = resolve_db_path(args.db)
    con = duckdb.connect(str(db_path), read_only=True)

    uc_costs = get_combined_costs(con, args.uc_study)
    tpch_costs = get_combined_costs(con, args.tpch_study)
    con.close()

    print(f"\nCross-workload Config Transfer Analysis")
    print(f"UC study:   {args.uc_study}")
    print(f"TPC-H study: {args.tpch_study}")
    print()

    # Find uc_optimal vs default comparison
    optimal_key = "uc_workload_optimal" if "uc_workload_optimal" in uc_costs else "uc_optimal"
    default_key = "default"

    results = {}
    for workload_name, costs in [("UC", uc_costs), ("TPC-H", tpch_costs)]:
        if optimal_key not in costs or default_key not in costs:
            print(f"  Warning: {workload_name} missing {optimal_key!r} or {default_key!r}")
            continue
        optimal = costs[optimal_key]
        default = costs[default_key]
        if len(optimal) < 2:
            print(f"  Warning: {workload_name} has < 2 groups, skipping Wilcoxon test")
            continue
        try:
            stat, pval = wilcoxon(optimal, default)
            significant = pval < 0.05
            degradation_pct = [(o-d)/d*100 for o, d in zip(optimal, default)]
            avg_degradation = sum(degradation_pct) / len(degradation_pct)
            results[workload_name] = (pval, significant, avg_degradation)
            print(f"  {workload_name}: p={pval:.4f}, {'significant' if significant else 'not significant'} (alpha=0.05), avg degradation={avg_degradation:+.1f}%")
        except Exception as e:
            print(f"  {workload_name}: Wilcoxon test failed: {e}")

    # Summary table
    print()
    print(f"{'Workload':<12} {'Config':<22} {'Median combined cost (ms)':>25}")
    print("-" * 62)
    for workload_name, costs in [("UC", uc_costs), ("TPC-H", tpch_costs)]:
        for cfg, vals in sorted(costs.items()):
            median = sorted(vals)[len(vals)//2] / 1e6
            print(f"  {workload_name:<10} {cfg:<22} {median:>25.2f}")

    print()
    for wl, (pval, sig, deg) in results.items():
        conclusion = "config IS workload-specific" if sig else "config generalizes across workloads"
        print(f"Conclusion ({wl}): {conclusion} (p={pval:.4f}, avg_degradation={deg:+.1f}%)")


if __name__ == "__main__":
    main()
