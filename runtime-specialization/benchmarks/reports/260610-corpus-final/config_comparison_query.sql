-- All-config comparison: one row per kernel, one column per config
-- Run after corpus_uc_final_20260610 ablation completes

WITH speedups AS (
    SELECT config_name, kernel,
           ROUND(med_unspec_ns::DOUBLE/med_spec_ns, 3) AS speedup
    FROM v_ablation_medians
    WHERE study_name='corpus_uc_final_20260610'
      AND med_spec_ns IS NOT NULL
)
SELECT kernel,
       MAX(CASE WHEN config_name='default' THEN speedup END) AS default_,
       MAX(CASE WHEN config_name='o3_only' THEN speedup END) AS o3_only_,
       MAX(CASE WHEN config_name='no_prune' THEN speedup END) AS no_prune_,
       MAX(CASE WHEN config_name='no_o3_final' THEN speedup END) AS no_o3_final_,
       MAX(CASE WHEN config_name='no_unroll' THEN speedup END) AS no_unroll_,
       MAX(CASE WHEN config_name='fixpoint_2' THEN speedup END) AS fixpoint_2_,
       MAX(CASE WHEN config_name='aggressive' THEN speedup END) AS aggressive_,
       MAX(CASE WHEN config_name='p0_o3_optimal' THEN speedup END) AS p0_optimal_,
       MAX(CASE WHEN config_name='uc_workload_optimal' THEN speedup END) AS uc_p2_optimal_
FROM speedups
GROUP BY kernel
ORDER BY default_ DESC;
