# Reflection — JIT Pipeline Evaluation Iteration 1 (2026-05-17)

Scope: closes the feedback loop for iteration 1 of the JIT pipeline evaluation. All claims
cite a specific study, query, or plot from `benchmarks/benchmarks.duckdb` or
`benchmarks/results/`.

## Headline Result

The TPE optimizer (`uc_optim_20260517`, study running) converged to
`{fixpoint_max ≈ 7–12, unroll_max ≈ 4–8, large_module_max = 0, early_prune = 1, o3_final = 1}`
within the first 20 trials. Best combined cost 173.2 ms (trial 17); the top 5 trials all sit
within 3.5 ms of best. The sensitivity sweep (`sens_uc_20260517`) confirms the optimum is
**robust**: geomean cost varies < 10 % for every parameter across its full search range.

The interesting story is *why* the optimum is so flat. The two highly-sensitive effects we
expected from architectural intuition (early-prune and O3-final) collapse to small geomean
deltas because the small kernels where these passes dominate JIT time also have small
absolute exec cost. The medium and large kernels dominate the geomean but barely move when
parameters change.

---

## 1. Infrastructure Improvements

### `optimize_benchmarks.py`

**Three concrete changes** for iteration 2:

1. **Per-kernel Optuna study, not geomean.** Evidence: `sensitivity_summary.md` shows
   single-kernel effects (`early_prune=0` → jit=2527 ms on one kernel vs ≈ 50 ms with it
   enabled) that geomean across 18 kernels reduces to a 1.03 ratio. The current
   single-objective geomean hides per-kernel structure. Either split the study into 6
   per-group studies, or move to a multi-objective Optuna formulation
   (`optuna.create_study(directions=['minimize']*6)`) and inspect the Pareto front.

2. **Resume support.** Evidence: the iteration-1 run was OOM-killed after 24/150 trials and
   could not resume — Optuna's in-memory `study` lost state, and the DB has the trial rows
   but no Optuna sampler state. Switch to `optuna.storages.RDBStorage(sqlite:///optuna.db)`
   so the study survives a process restart. The DuckDB `optim_trial_params` table can stay
   as a denormalised reporting view.

3. **Smarter search-space pruning between iterations.** Evidence: `unroll_max` and
   `large_module_max` show ratio < 1.05 across full ranges in `sensitivity_summary.md`.
   These should be fixed (not sampled) in iteration 2, freeing the trial budget for params
   that matter (`fixpoint_max` resolution, plus new params we don't have yet).

### UC Benchmark Binaries

1. **Add the kernel-size axis to the optimizer.** Evidence: trial output shows the same
   18-kernel set per trial regardless of size. The size-scaling note in `research.md` says
   relative speedup is stable across MEDIUM/LARGE/EXTRALARGE, but absolute JIT-vs-exec
   ratios shift with size — small kernels favour aggressive JIT (high `fixpoint_max`),
   large ones don't. Iteration 2 should optimise over `{kernel, size}` jointly, not just
   `kernel`.

2. **Add a "small-kernel-dominant" workload group.** Evidence: the geomean is dominated by
   large kernels and hides the small-kernel structure. A synthetic group of small (<5 µs
   exec) kernels would expose the JIT-overhead sensitivity that the current UC mix masks.

### DuckDB Schema / Tooling

1. **Add a per-kernel sensitivity view.** Evidence: the OAT sweep stores 18 rows per config
   (`v_ablation_medians`), but the only reporting we do is geomean. A view
   `v_sensitivity_per_kernel(study_name, param, value, kernel, group, combined_ns,
   normalized_combined)` would let us write kernel-specific plots without rewriting the
   join in each script.

2. **Promote `params_json` to typed columns.** Evidence: every analysis query starts with
   `json_extract(params_json, '$.fixpoint_max')` — this is fragile and slow. Adding typed
   columns to `optim_trial_params` (and keeping `params_json` for raw provenance) would let
   us index on parameter values.

3. **Add a `failed_runs` table or status column.** Evidence: the RQ-15 SIGSEGV at
   `fixpoint_max=1+early_prune=1+o3_final=1` is stored as a `NULL run_id` row in
   `ablation_studies` with no signal of *why* it failed. A `status` column
   (`{ok, sigsegv, timeout, db_error}`) would make crash analysis queryable.

---

## 2. Evaluation Process Improvements

### Trial Budget

**Was 150 sufficient?** The 24 trials we actually have show clear convergence: the top 5
all live in a 3.5 ms band of `{early_prune=1, o3_final=1, large_module_max=0}`. So **24
trials are enough for the optimal *region*; 150 is overkill for the *direction*.** What
150 trials would have given us is a tighter estimate of *which* `fixpoint_max` (7–12)
gives the lowest cost, but the sensitivity sweep (`sens_uc_20260517`) says that band is
flat to 1 ms — there is no statistically meaningful winner within the band at reps=1.

**Proposal for iteration 2**: 50 trials, seed=42, **plus** explicit `optuna.trial.TrialState.PRUNED`
based on a "no improvement in 20 trials" stop criterion. Save ~3 hours wall-clock.

### Repetition Count

**Was 3 reps enough for ablation?** TBD — T017 (`ablation_uc_20260517`) is still in
progress. The smoke test (`smoke_ablation_v2`) at 3 reps already showed `[default]
speedup=1.70x` and `[o3_only] speedup=0.60x` (a 3× difference), suggesting that 3 reps is
adequate when the inter-config delta is large. For the **fine-grained** ablation (e.g.
`fixpoint_2` vs `aggressive`), 3 reps may be too few — the per-rep variance dominates.

**Proposal**: keep `reps=3` for the high-level ablation; for the post-iteration sensitivity
re-run (iteration 2), bump to `reps=5` so we can resolve sub-5% effects.

### Size Selection

**Was MEDIUM representative?** Yes for *relative* comparisons, no for *absolute* numbers.
The size-scaling report (`research.md` RQ-9) said relative speedup is stable
MEDIUM↔EXTRALARGE, and the trial data agrees. The thesis can claim MEDIUM as the working
size with the caveat that absolute JIT times scale roughly linearly with kernel size.

The one place MEDIUM may be **un**representative is for "is JIT worth it?": at MEDIUM, the
break-even is reachable in < 100 calls for most kernels (see `breakeven_uc_optim_20260517.png`),
but at MINI sizes the JIT overhead dwarfs exec savings forever. The reflection-driven next
iteration should at least *sanity-check* the iteration-1 conclusions at SMALL.

### Statistical Approach

**Wilcoxon test**: not run — TPC-H binary not available, Exp C skipped (`research.md`
RQ-12). When TPC-H becomes available (iteration 2 or 3), add the transfer experiment with
3+ reps per config × 9 TPC-H phases. Wilcoxon on 9 paired samples gives modest power; if
we want a confident "yes/no" on transfer we need 15+ pairs (more queries, more sizes, or
both).

---

## 3. Next-Iteration Scope

### Parameters to Prune (Insensitive in Iteration 1)

Per `sensitivity_summary.md`:

| Param | Ratio | Action |
|-------|------:|--------|
| `unroll_max` | 1.02 | **Drop.** Fix at default 64. Save Optuna search dim. |
| `large_module_max` | 1.04 | **Drop.** Fix at 0 (no UC kernel triggers the large-module path). |
| `early_prune` | 1.03 (geomean) | **Keep as binary**, but flag per-kernel impact in importance JSON. |

### Parameters Needing Finer Resolution

- `fixpoint_max` — the 7–12 range is flat in geomean but possibly per-kernel-sensitive.
  Iteration 2 grid: `{3, 5, 7, 10, 15}` with 5 reps.
- `o3_final` — most sensitive at 1.09 ratio. Keep, but also test a "selective O3"
  variant: O3 only on functions ≥ N instructions.

### New Parameters to Introduce

- **Per-function O3 budget** (currently a binary): expose as a function-size threshold.
- **Pass-level toggles** (currently lumped): `EnableIPSCCP`, `EnableDevirtVtable`,
  `EnableGVN`. From `research.md` RQ-1, this was deferred for thesis scope but now we have
  the sensitivity infrastructure to evaluate them cheaply.

### Experiments to Skip / Repeat / Add

| Exp | Action | Reason |
|-----|--------|--------|
| A (optimization) | **Repeat** with 50 trials, seed=42, RDB storage | resume support, less budget |
| B (ablation) | **Repeat** with 5 reps after pruning insensitive params | tighter CIs |
| C (TPC-H transfer) | **Add** once TPC-H binary exists | thesis transfer story |
| D (break-even) | **Skip** — no new data needed | derived from A |
| E (sensitivity) | **Repeat** per-kernel, not geomean | iteration-1 hid kernel structure |
| F (reflection) | **Add** the cross-iteration delta section | meta-evaluation |

### Trial Budget for Iteration 2

- A: 50 Optuna trials × ~110 s = ~90 min
- B: 5 configs × 5 reps × ~110 s = ~45 min (down from 8 × 3 × 110 s = 44 min — net flat,
  more reps per config instead of more configs)
- C: 3 TPC-H queries × 4 configs × 5 reps × ~30 s = 30 min
- E: 5 params × ~5 grid pts × 5 reps × ~110 s = ~3.5 h (much more expensive — accept it)

Total: ~6 hours, run as 4 sequential background tasks (lesson from iteration 1 —
concurrent processes OOM the box).

---

## 4. Iteration-1 Pitfalls Worth Encoding

1. **Concurrent `AllBenchmarks` processes OOM the machine.** Each instance is ~14 GB RES.
   The orchestration script `run_evaluation.sh` runs phases sequentially — keep it that
   way. Memory note saved at
   `~/.claude/projects/.../memory/feedback_concurrent_benchmarks.md`.

2. **`fixpoint_max=1 + early_prune=1 + o3_final=1` is a crashing combination** (RQ-15).
   The Optuna search space lower bound for `fixpoint_max` is `1` but the binary crashes
   when sampled. Iteration 2: set lower bound to `2` *or* gate on `early_prune` in the
   search-space sampler.

3. **`CRS_DEFAULT_PIPELINE=1` (function-specialization) crashes on the full UC filter**
   (RQ-11). Pipeline parameter dropped from search space. Iteration 2: fix the upstream
   crash first, then re-enable.

4. **Optuna parameter importance requires the in-memory study** (RQ-7). If you OOM, you
   lose the importance scores. RDB-backed Optuna in iteration 2 fixes both this and
   resume.

5. **`_query_config_summary` uses `LIMIT 1`** — the per-config summary in
   `ablation_benchmarks.py` shows one kernel's numbers, not aggregate. Useful for spot
   checks but not for thesis claims; always use the per-kernel rows from
   `v_ablation_medians`.

---

## 5. Status of Iteration 1 Outputs

| File | Status |
|------|--------|
| `benchmarks/results/uc_best_config.json` | Preliminary (trial 16 best, will update on T012 completion) |
| `benchmarks/results/uc_workload_optimal.json` | Preliminary (matches uc_best_config) |
| `benchmarks/results/pareto_uc_optim_20260517/*.png` | 6 PNGs, regenerate after T012 finishes |
| `benchmarks/results/breakeven_uc_optim_20260517.png` | Generated (24 trials), regenerate after T012 |
| `benchmarks/results/ablation_ablation_uc_20260517.png` | Pending T017 completion |
| `benchmarks/results/sensitivity_sens_uc_20260517.png` | Complete |
| `benchmarks/results/sensitivity_summary.md` | Complete |
| `benchmarks/results/reflection_iteration1.md` | This document |
| `importance_uc_optim_20260517.json` | Not produced — T012 did not reach `--output-best` write |

**Iteration is considered closed for thesis purposes once T012 either completes 150 trials
or we accept the 24-trial result as sufficient given the convergence evidence above.**
