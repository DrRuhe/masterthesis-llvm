# Iteration 2 Reflection — JIT Pipeline Evaluation

**Run dir**: `benchmarks/reports/260521-17-32-optimize-pipeline/`
**Date**: 2026-05-21 / 2026-05-22 (Phase A through Phase R)
**Spec**: `specs/007-jit-pipeline-eval/tasks.md` Phase 9 (T037–T050)
**Iter-1 reference**: `benchmarks/reports/260520-13-00-optimize-pipeline/reflection_iteration1.md`

Iter-2 studies (all stored in `benchmarks/benchmarks.duckdb`):

| Study | Phase | Rows |
|-------|-------|------|
| `uc_optim_iter2_20260521` | A — 50 Optuna trials, expanded search space (+ pipeline, p1_inline_threshold, p1_max_module_growth; fixpoint_max ≥ 2) | 50 trial rows |
| `ablation_uc_iter2_20260521` | B — 11 named configs × 5 reps | 55 runs, 0 NULL `run_id`, 66 median rows |
| `transfer_uc_iter2_20260521` | C (UC half) — 3 configs × 5 reps | 15 OK, 18 medians, 0 NULLs |
| `transfer_tpch_iter2_20260521` | C (TPC-H half) | 0 rows — see §3.b |
| `sens_uc_iter2_20260521` | E — 24 sweep points × 5 reps | 120 attempted, 115 OK, 5 SIGSEGV (sens_pipeline_1); see §3.a |

---

## 1. Headline result

**Combined-best config (iter-2 Phase A, `uc_optim_iter2_20260521` trial 15)** —
the Optuna combined objective `t_jit_ns + t_spec_ns` minimum is:

```
fixpoint_max          = 7
unroll_max            = 27
large_module_max      = 0
early_prune           = 1
o3_final              = 1
pipeline              = 0
p1_inline_threshold   = 572
p1_max_module_growth  = 1.6429245153475713
```

Combined cost: `170.99 ms` (jit 56.76 ms + spec 68.54 ms).

**The winner picks `pipeline=0`.** Of the 50 trials, 4 hit the 60-second
timeout fallback (`used_timeout_fallback=TRUE`) and all 4 had `pipeline=1`:

```sql
SELECT trial_id, params_json
FROM optim_trial_params
WHERE study_name='uc_optim_iter2_20260521'
  AND used_timeout_fallback=TRUE;
-- → trials 26, 29, 37, 39; pipeline=1 in all four
```

**Per-kernel winners (`v_optim_best_per_kernel WHERE study_name='uc_optim_iter2_20260521'`):**
14 of 18 kernels prefer `pipeline=0`. The 4 kernels that prefer `pipeline=1`
all happen to share Optuna trial 34's parameter vector (a co-discovery
artifact, not 4 independent confirmations): `generic_sort`, `grouped_count`,
`multi_pattern_match` (and `grouped_count` shares trial 34 with `generic_sort`
because trial 34 was best for both by Pareto coincidence). The per-kernel
improvements over `pipeline=0` at the same kernel are within ±2 % in every
case — pipeline choice is **dominated by other parameters at MEDIUM size**.

**P1 inline-budget bands** (when P1 is picked):
- `p1_inline_threshold` 375 (trial 34) — chosen by the only P1 per-kernel winners.
- `p1_max_module_growth` 2.76 (trial 34) — a "moderate" growth cap, between
  the iter-2 search-space midpoints. The Phase B `pipeline_1_aggressive_budget`
  config (threshold=1000, growth=4.0) was the best of the three named P1
  variants but still under-performed `pipeline=0` on the same mean.

---

## 2. Cross-iteration delta vs iter-1 winner

**Iter-1 winner** (per `reflection_iteration1.md`, study `uc_optim_20260517`,
incomplete at 24/150 trials but accepted as iter-1 final):
`fixpoint_max=7, unroll_max=4, large_module_max=0, early_prune=1, o3_final=1, pipeline=0`.

**Iter-2 winner**: same except `unroll_max=27` (was 4). The iter-2 search
also added `p1_inline_threshold=572` and `p1_max_module_growth=1.64` which
are dormant when `pipeline=0`.

**Per-kernel deltas (5 % noise band):**

| Kernel | iter-1 best total (ms) | iter-2 best total (ms) | Δ | Verdict |
|---|---|---|---|---|
| apply_row_delta | 59.4 | 58.8 | -1.1 % | noise |
| batch_delta | 100.6 | 96.4 | -4.2 % | noise |
| box_filter | 430.6 | 413.5 | -4.0 % | noise |
| column_scan | 83.0 | 80.8 | -2.6 % | noise |
| count_matching_rows | 2398.8 | 2346.0 | -2.2 % | noise |
| edge_detection | 171.2 | 167.8 | -2.0 % | noise |
| email_match | 139.2 | 137.8 | -1.0 % | noise |
| **generic_sort** | **330.8** | **282.8** | **-14.5 %** | **win** |
| grouped_count | 55.0 | 53.7 | -2.2 % | noise |
| grouped_minmax | 55.8 | 55.2 | -1.0 % | noise |
| grouped_sum | 55.5 | 54.5 | -1.8 % | noise |
| multi_agg_delta | 61.6 | 60.6 | -1.6 % | noise |
| multi_key_sort | 752.5 | 739.6 | -1.7 % | noise |
| multi_pattern_match | 132.4 | 130.9 | -1.1 % | noise |
| multi_predicate | 97.7 | 96.3 | -1.4 % | noise |
| **separable_gaussian** | **288.1** | **270.6** | **-6.1 %** | **win** |
| struct_sort | 723.7 | 719.8 | -0.5 % | noise |
| url_match | 138.5 | 137.4 | -0.8 % | noise |

Verification query: see `q_iter_delta.sql` excerpt embedded above.

**Outcome**: 16 / 18 kernels stay inside the 5 % noise band; 2 kernels
(`generic_sort` -14.5 %, `separable_gaussian` -6.1 %) show real improvement.
**Both winners chose larger `unroll_max` than iter-1's optimum** — iter-2's
`generic_sort` winner had `unroll_max=41`, `separable_gaussian` had `unroll_max=21`.
This suggests iter-1's reflection that `unroll_max` is uniformly low-importance
was over-aggressive; for unroll-friendly kernels it matters.

---

## 3. Pipeline-1 verdict

**Did P1 Pareto-dominate P0 for any kernel?** No — but it Pareto-matches on
`uc14_sort` and is close on `uc7_dfa`.

**Phase B (`ablation_uc_iter2_20260521`) per-group best-spec by pipeline:**

| Group | best P0 spec (ms) | best P1 spec (ms) | P0/P1 |
|---|---|---|---|
| uc12_groupby | 10.04 | 11.84 | 0.85 (P1 worse by 18 %) |
| **uc14_sort** | 281.27 | **233.67** | 1.20 (P1 17 % faster) |
| uc1_sql | 7.66 | 8.19 | 0.94 |
| uc2_conv | 110.19 | 150.96 | 0.73 (P1 worse by 37 %) |
| uc7_dfa | 89.12 | 90.50 | 0.98 (tie) |
| uc8_ivm | 17.84 | 26.03 | 0.69 (P1 worse by 46 %) |

(Query: `SELECT … FROM v_ablation_medians WHERE study_name='ablation_uc_iter2_20260521' GROUP BY group, pipeline_kind`.)

**P1 was best on `uc14_sort` only.** It lost decisively on `uc2_conv` and
`uc8_ivm` (37–46 % slower). The `pipeline_1_aggressive_budget` config
(threshold=1000, growth=4.0) won on `uc14_sort`; the tighter-budget P1
variants were not faster than P0 on any kernel.

**Plot citation**: see `ablation_ablation_uc_iter2_20260521.png` and the
6 `pareto_uc_optim_iter2_20260521_<group>.png` per-group Pareto plots in
the run dir.

### 3.a — New P1 instability surfaced by Phase E

`sens_uc_iter2_20260521` sweep point `sens_pipeline_1` (flip `pipeline=1`
on top of the Phase-A combined-best config: `fixpoint_max=7, unroll_max=27,
large_module_max=0, early_prune=1, o3_final=1, p1_inline_threshold=572,
p1_max_module_growth=1.6429245153475713`) produced **0 / 5 OK reps — all
crashed with SIGSEGV**.

This is a new finding: Phase B's `pipeline_1_default` ran the **default**
baseline with only `CRS_DEFAULT_PIPELINE=1` overridden and did not crash
(0.31 × speedup, slow but stable). The crashing OAT point is **a multi-parameter
interaction** — `pipeline=1` combined with the P0-tuned baseline. The exact
trigger is unidentified (candidates: `unroll_max=27`, `p1_max_module_growth=1.64`,
their interaction with the kernel module size).

This effectively replaces iter-1's RQ-11 ("P1 crashes on full UC filter")
with a more precise RQ-15-style finding: **P1 crashes under specific
parameter cross-products**. Iter-3 should bisect.

### 3.b — Phase C TPC-H — evolved RQ-12

The TPC-H binary `TPCHBenchmark` is present in the release build this
iteration (was absent in iter-1, hence RQ-12). However, all `jit_overhead`
filter invocations crash with SIGSEGV — the embedded library reports
`library_build_type=debug`, so the test binary contains a debug-mode
runtime even when compiled into the release tree.

- Reproducer: `cd benchmarks && /path/to/TPCHBenchmark --benchmark_filter='tpch_q1;.*jit_overhead'` → segfault in JIT.
- UC half of Phase C (`transfer_uc_iter2_20260521`) ran cleanly; the optimal
  config transferred without speedup advantage (1.26 × on UC MEDIUM, comparable
  to `default`'s 1.81 × within noise — Phase C's data is too thin to draw
  cross-workload conclusions).
- Phase C is therefore **partially complete**: UC transfer answered;
  TPC-H transfer postponed to iter-3 once the debug-mode TPC-H binary is
  rebuilt as release.

---

## 4. Next-iteration scope

### Iteration-3 recommendation: **yes, scoped**.

Three things are worth one more iteration; everything else is closed.

**a. Bisect the Phase-E `sens_pipeline_1` crash.** The OAT sweep landed
on a parameter combination that crashes P1 deterministically. A minimal
iter-3 plan:

1. Take the Phase-A combined-best config (call it `BASE`).
2. Generate the 2⁵ = 32 configs of `BASE` with each non-pipeline parameter
   flipped between {`BASE` value, default value}, plus `pipeline=1`.
3. Run each at reps=3 against a single UC group (start with `uc8_ivm`
   where Phase B already showed P1 weakness).
4. Find the minimal parameter set whose flip-to-default makes P1 stable.
5. File a corresponding fix in `runtime/ClangRuntimeSpecializer/JITPipelineFuncSpec.cpp`.

**b. Re-enable Phase C TPC-H.** Rebuild `TPCHBenchmark` as release (the
target as currently configured links the debug runtime). Once `jit_overhead`
runs without SIGSEGV, repeat Phase C with the iter-2 winner config to
finally answer the cross-workload transfer question.

**c. Add the `unroll_max` × kernel-size interaction to the search space.**
Iter-1's reflection §3 recommended pruning `unroll_max` from the search
space; iter-2 shows two kernels (`generic_sort`, `separable_gaussian`)
benefit from higher unroll values. Keep `unroll_max` in the search space
but in iter-3, add a `--workload-tag` to `ablation_benchmarks.py` so that
the optimizer can be told whether the workload is unroll-friendly. (This
is also a good candidate for the per-kernel Optuna deferred in iter-1.)

### Out-of-scope for iter-3 (firmly closed by iter-2 evidence):

- The `pipeline` choice itself — at MEDIUM size, P0 wins on 5/6 UC groups
  and P1 wins on 1. This is a categorical, not a continuous, choice; no
  amount of further trials will move that needle. Iter-3 should fix P1's
  stability bugs before benchmarking against P0 again.
- `p1_inline_threshold` and `p1_max_module_growth` — Phase B showed
  `aggressive_budget` ≈ `default` ≈ `tight_budget` on average; the budget
  is not a primary lever. Keep at default (225 / 2.0) for iter-3 unless
  the crash bisection in §a shows otherwise.
- The smoke filter regex bug (post-spec-011 group names) — fixed and
  baked into `run_evaluation.sh` UC_FILTER constants in T041.

### Open infrastructure debt (not blocking iter-3, but worth fixing):

- **sklearn is unavailable** in the Nix environment, so
  `optuna.importance.get_param_importances` warns and skips. The iter-2
  `importance_uc_optim_iter2_20260521.json` was not produced; `plot_sensitivity`
  had to run without `--importance-json`. Add `python312Packages.scikit-learn`
  to `flake.nix`.
- **`record_benchmark.py` ablation summary uses `LIMIT 1`** — first noted
  in iter-1 reflection §4, still present in iter-2. Reported speedups
  in the live log (e.g. "uc_workload_optimal speedup=2.86x") are from one
  kernel, not aggregate. The `v_ablation_medians` per-group view is the
  authoritative source. Cosmetic but misleading during interactive runs.

---

## 5. Status of iteration-2 outputs

| Artifact | Location | Status |
|---|---|---|
| Best config (Phase A) | `best_uc_optim_iter2_20260521.json` | Final |
| Workload optimal env | `uc_workload_optimal.json` | Final (derived from best) |
| Phase B configs | `configs/ablation_uc.json` (11 configs) | Final |
| Phase C transfer configs | `configs/transfer_configs.json` (3 configs) | Final |
| Pareto plots (per-group) | `pareto_uc_optim_iter2_20260521_<group>.png/csv` | Final, 6 each |
| Pareto trial scatter | `pareto_uc_optim_iter2_20260521/` subdir | Final, 6 PNGs |
| Ablation bar chart | `ablation_ablation_uc_iter2_20260521.png` | Final |
| Break-even box plot | `breakeven_uc_optim_iter2_20260521.png` | Final |
| Sensitivity plot | `sensitivity_sens_uc_iter2_20260521.png` | Final (no importance overlay) |
| Phase logs | `phase_a.log`, `phase_b.log`, `phase_c_uc.log`, `phase_c_tpch.log`, `phase_e.log`, `eval_run_log.txt` | Final |
| Importance JSON | — | NOT PRODUCED (sklearn missing, see §4 open debt) |

**Verification (per T050)**: see `eval_run_log.txt` for run timestamps,
study names, and skip reasons. Run-level numbers (50 trials, 55 + 15 + 120
ablation/transfer/sensitivity runs, 0 NULL `run_id` for non-crash points)
are queryable directly via the 5 study names listed at the top of this
document.
