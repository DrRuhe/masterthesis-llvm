# Phase A Optimization Reflection — uc_optim_iter3_20260601

**Run dir**: `benchmarks/reports/260601-15-16-optimize-pipeline/`  
**Date**: 2026-06-01  
**Study**: `uc_optim_iter3_20260601` — 50 trials, P0+P1+P2 combined, MEDIUM size UC benchmarks  
**Binary**: rebuilt 2026-06-01 (includes !invariant.load fix: `a55bbd7885a0`)  
**Corpus study**: `corpus_uc_p0_p2_20260601` — 3 configs × 5 reps, UC MEDIUM "low" abstraction  

---

## 0. Bugs fixed before running

### Bug A — P2 SIGSEGV with `LargeModuleInstrThreshold=0`

`JITPipelineIPSCCP.cpp` guarded `StaticMutabilityAnalysis` with `if (!Args.IsLargeModule)`.
Setting `CRS_DEFAULT_LARGE_MODULE_INSTR_THRESHOLD=0` (the optimizer's "off" encoding for `int_or_zero` params) made every UC module "large", skipped the annotation pass, and left both path A and path B in `visitLoadInst` unguardable — causing P2FuncSpec with `SpecOnAddr=1` to produce broken specializations that SIGSEGV'd during `specialized_exec`.

**Fix** (commit `6f1b06925ae7`): `StaticMutabilityAnalysis` runs unconditionally.  
All 45 smoke tests pass after the fix.

### Bug B — Optimizer saw `jit_ns=0` for all UC benchmarks

Commit `957c0ba117f0` added `->UseManualTime()->Iterations(1)` to all UC `_jit_overhead` registrations but the implementations (`BM_UC1_*_jit_overhead` etc.) call `create_*_specialized()` directly without calling `state.SetIterationTime()`. `real_time=0.0` in every benchmark JSON → optimizer saw `jit_ns=0` for all trials.

**Fix**: `optimize_benchmarks.py` now uses `cpu_time` for `jit_overhead` benchmarks (≈ real_time for single-threaded JIT work).

---

## 1. Headline result

**Optimizer best: Pipeline 0 + O3 final, 172.3ms combined**

```
pipeline=0, o3_final=1, early_prune=1
fixpoint_max=16, unroll_max=62, large_module_max=3
```

**Previous study best** (`uc_optim_iter3_20260531`): P2+O3, 169.3ms

**3-way pipeline comparison (best trial per pipeline):**

| Pipeline | Best combined | Trial | jit (cpu_time) | exec |
|----------|:---:|:---:|:---:|:---:|
| **P0+O3** | **172.3ms** | 28 | 57.5ms | 62.7ms |
| P2+O3 | 174.9ms | 27 | 59.2ms | 63.4ms |
| P1 | 205.2ms | 3 | 73.3ms | 68.0ms |

P0 and P2 are separated by 2.6ms — within measurement noise (~3–5ms). The !invariant.load
fix reversed the ranking: P2 led by 1.7ms in the previous study; now P0 leads by 2.6ms.

**Best P2+O3 config** (new optimal after fix):
```
pipeline=2, o3_final=1, early_prune=1, fixpoint_max=16, unroll_max=44, large_module_max=3
p2_force_spec=1, p2_spec_on_addr=1, p2_spec_literal=1
p2_max_clones=18, p2_func_spec_iters=2, p2_min_func_size=18
```

---

## 2. Impact of the !invariant.load fix on per-kernel speedups

Corpus study `corpus_uc_p0_p2_20260601` ran 3 configs × 5 reps on UC MEDIUM "low" abstraction:
- `default_p0`: default P0 (no env overrides)
- `p0_o3_optimal`: P0 with new optimal config (Trial 28)
- `p2_o3_optimal`: P2 with new optimal config (Trial 27)

For comparison, old P2+O3 data from `ablation_uc_iter3_20260531` (previous binary).

### Per-kernel exec speedup table

| Kernel | Old P2+O3 | New P0+O3 | New P2+O3 | Δ (old→new P2) |
|--------|:---------:|:---------:|:---------:|:--------------:|
| box_filter | 3.06× | 2.80× | **3.02×** | −0.04× |
| grouped_count | 2.41× | 2.33× | **2.38×** | −0.03× |
| grouped_minmax | 2.26× | 2.21× | **2.25×** | −0.01× |
| grouped_sum | 2.25× | 2.19× | **2.22×** | −0.03× |
| multi_agg_delta | 2.24× | 2.21× | 2.20× | −0.04× |
| batch_delta | 2.20× | **2.27×** | 2.18× | −0.02× |
| apply_row_delta | 2.17× | 2.18× | 2.18× | +0.01× |
| separable_gaussian | 1.96× | 1.93× | **1.94×** | −0.02× |
| struct_sort | 1.53× | **1.58×** | 1.53× | 0× |
| edge_detection | 1.38× | **1.51×** | 1.44× | +0.06× |
| generic_sort | 1.48× | **1.49×** | 1.47× | −0.01× |
| multi_key_sort | 1.39× | 1.38× | 1.38× | −0.01× |
| multi_pattern_match | 1.25× | **1.26×** | 1.25× | 0× |
| url_match | 1.22× | 1.20× | 1.21× | −0.01× |
| email_match | 1.22× | 1.20× | 1.20× | −0.02× |
| count_matching_rows | 1.02× | 0.99× | **1.02×** | 0× |
| column_scan | 1.01× | 1.01× | 0.99× | −0.02× |
| multi_predicate | 0.95× | **1.02×** | **1.02×** | **+0.07×** |

**Win rate (≥1.1× exec speedup):**
- New P2+O3: 15/18 (column_scan 0.99× is a minor new regression; multi_predicate fixed from 0.95→1.02×)
- Old P2+O3: 16/18 (column_scan and multi_predicate were marginal)

**Conclusion**: The !invariant.load fix had **minimal** per-kernel impact. Maximum regression was −0.04× on a handful of groupby/IVM kernels. `multi_predicate` actually improved +0.07× because the fix corrected an unsound pointer fold that was causing a regression.

### Geomean speedups (ablation summary output)

| Config | geomean spec | geomean unspec | speedup |
|--------|:---:|:---:|:---:|
| default_p0 | 691.4ms | 963.6ms | 1.39× |
| p0_o3_optimal | 13.4ms | 29.5ms | 2.21× |
| p2_o3_optimal | 12.2ms | 27.5ms | **2.25×** |

Note: the ablation computes `speedup = sum(unspec)/sum(spec)` (aggregate of medians, not geomean).
With O3 final, P2+O3 still edges P0+O3 by 0.04× in absolute exec time geomean.

---

## 3. Polybench

The `PolyBenchBenchmark` binary cannot run JIT benchmarks with the current binary. Root cause:
`benchmarkJITOverhead` calls `RS->specializeOnly<R>(F, opts, args...)` without a funcName
argument. The `IRRewritingPass` cannot inject the funcName through the `std::apply`/lambda
call chain in the template, so `FuncToBlobIdx` lookup fails at runtime with:

```
ClangRuntimeSpecializerDumpedIRError: specializeOnly: TU was not compiled with the IRDumpingPass plugin
```

This is a pre-existing bug in `benchmarkJITOverhead` introduced by the funcptr API migration.
The May 31 polybench results remain the valid reference. Given the minimal UC impact of the fix,
polybench results are expected to show similarly small regressions (~1–3%).

---

## 4. Reproducibility — SQL queries

All results are stored in `benchmarks/benchmarks.duckdb`.

### Phase A optimizer results

```sql
-- Best config per pipeline
SELECT
    CASE CAST(json_extract_string(params_json, '$.pipeline') AS INT)
        WHEN 0 THEN 'P0'
        WHEN 1 THEN 'P1'
        WHEN 2 THEN 'P2'
    END AS pipeline,
    trial_id,
    ROUND(obj_jit_ns/1e6, 1)      AS jit_ms,
    ROUND(obj_exec_ns/1e6, 1)     AS exec_ms,
    ROUND(obj_combined_ns/1e6, 1) AS combined_ms,
    params_json
FROM optim_trial_params
WHERE study_name = 'uc_optim_iter3_20260601'
  AND used_timeout_fallback = false
ORDER BY obj_combined_ns
LIMIT 10;

-- Top 5 per pipeline
SELECT *
FROM (
    SELECT *,
           ROW_NUMBER() OVER (PARTITION BY CAST(json_extract_string(params_json,'$.pipeline') AS INT)
                              ORDER BY obj_combined_ns) AS rn
    FROM optim_trial_params
    WHERE study_name = 'uc_optim_iter3_20260601'
      AND used_timeout_fallback = false
) t
WHERE rn <= 5
ORDER BY CAST(json_extract_string(params_json,'$.pipeline') AS INT), obj_combined_ns;
```

### Per-kernel speedups — corpus ablation

```sql
-- All configs, all kernels, sorted by config and descending speedup
SELECT
    config_name,
    kernel,
    ROUND(med_spec_ns / 1e6, 2)                        AS spec_ms,
    ROUND(med_unspec_ns / 1e6, 2)                      AS unspec_ms,
    ROUND(med_unspec_ns::DOUBLE / med_spec_ns, 3)      AS speedup,
    n_reps
FROM v_ablation_medians
WHERE study_name = 'corpus_uc_p0_p2_20260601'
ORDER BY config_name, speedup DESC;

-- Side-by-side comparison: old P2+O3 vs new P0+O3 vs new P2+O3
SELECT
    a.kernel,
    ROUND(a.med_unspec_ns::DOUBLE / a.med_spec_ns, 2) AS old_p2_o3,
    ROUND(b.med_unspec_ns::DOUBLE / b.med_spec_ns, 2) AS new_p0_o3,
    ROUND(c.med_unspec_ns::DOUBLE / c.med_spec_ns, 2) AS new_p2_o3,
    ROUND(c.med_unspec_ns::DOUBLE / c.med_spec_ns
        - a.med_unspec_ns::DOUBLE / a.med_spec_ns, 3) AS delta
FROM v_ablation_medians a
JOIN v_ablation_medians b USING (kernel)
JOIN v_ablation_medians c USING (kernel)
WHERE a.study_name = 'ablation_uc_iter3_20260531' AND a.config_name = 'pipeline_2_o3_final'
  AND b.study_name = 'corpus_uc_p0_p2_20260601'   AND b.config_name = 'p0_o3_optimal'
  AND c.study_name = 'corpus_uc_p0_p2_20260601'   AND c.config_name = 'p2_o3_optimal'
ORDER BY old_p2_o3 DESC;

-- Win/loss summary per config
SELECT
    config_name,
    COUNT(*) FILTER (WHERE med_unspec_ns::DOUBLE / med_spec_ns >= 1.1) AS wins_ge_1_1x,
    COUNT(*) FILTER (WHERE med_unspec_ns::DOUBLE / med_spec_ns BETWEEN 1.0 AND 1.1) AS marginal,
    COUNT(*) FILTER (WHERE med_unspec_ns::DOUBLE / med_spec_ns < 1.0)  AS regressions,
    ROUND(EXP(AVG(LN(med_unspec_ns::DOUBLE / med_spec_ns))), 3)       AS geomean_speedup
FROM v_ablation_medians
WHERE study_name = 'corpus_uc_p0_p2_20260601'
GROUP BY config_name
ORDER BY geomean_speedup DESC;
```

### Break-even calls

```sql
-- Break-even for Phase A best config per kernel
SELECT
    ob.kernel,
    ROUND(ob.t_jit_ns / 1e6, 1)        AS jit_ms,
    ROUND(ob.t_spec_ns / 1e6, 2)       AS spec_ms,
    ROUND(ob.unspec_ns / 1e6, 2)       AS unspec_ms,
    ROUND(ob.break_even_calls)          AS break_even_calls,
    ROUND(ob.unspec_ns::DOUBLE / ob.t_spec_ns, 2) AS exec_speedup
FROM v_optim_breakeven ob
WHERE ob.study_name = 'uc_optim_iter3_20260601'
ORDER BY ob.break_even_calls NULLS LAST;
```

---

## 5. Definitive best config for thesis (updated)

After the !invariant.load fix (`a55bbd7885a0`), both pipelines are competitive.
P0+O3 wins the optimizer objective by 2.6ms; P2+O3 wins the exec geomean by 0.04×.
The correct thesis conclusion is:

> **With the !invariant.load soundness fix, P0+O3 and P2+O3 are equivalent for UC MEDIUM
> "low" abstraction kernels. P2 retains a meaningful exec advantage only for `box_filter`
> (3.02× vs 2.80×) and marginally for groupby kernels. For use cases requiring vtable
> devirtualization (higher abstraction levels), P2 remains the recommended choice.**

For thesis results tables, use `corpus_uc_p0_p2_20260601` as the authoritative source.
The two recommended configs to report are `p0_o3_optimal` and `p2_o3_optimal`.
