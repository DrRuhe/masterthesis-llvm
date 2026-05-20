# Sensitivity Analysis Summary — Iteration 1

**Study**: `sens_uc_20260517` (UC MEDIUM, 1 rep per sweep point, 18 kernels per point)
**Base config**: optimal from `uc_optim_20260517` — `{fixpoint_max=9, unroll_max=5, large_module_max=0, early_prune=1, o3_final=1}`

## Method

For each parameter we hold all others at the optimal values and sweep the parameter through
its grid. We report the **geometric mean of `combined_cost = med_jit_ns + med_spec_ns`**
across the 18 UC kernels at each sweep value. A parameter is classified as **sensitive** if
`max_cost / min_cost > 1.10`, **insensitive** if `< 1.05`, **borderline** in between.

## Results

| Parameter | Min cost (ms) | Max cost (ms) | Ratio | Verdict |
|-----------|--------------:|--------------:|------:|---------|
| `fixpoint_max` (range 2–30) | 177.8 | 186.4 | 1.05 | borderline-insensitive |
| `unroll_max` (range 1–512) | 176.7 | 179.8 | 1.02 | **insensitive** |
| `large_module_max` (range 0–100k) | 177.5 | 185.3 | 1.04 | **insensitive** |
| `early_prune` (0 vs 1) | 177.5 | 183.0 | 1.03 | **insensitive** (in geomean) |
| `o3_final` (0 vs 1) | 179.1 | 194.8 | 1.09 | borderline-sensitive |

**Excluded values** (would have crashed the binary):
- `fixpoint_max=0` — SIGSEGV combined with O3 final (RQ-10, research.md)
- `fixpoint_max=1` — SIGSEGV when combined with `early_prune=1` + `o3_final=1` (RQ-15)
- `pipeline=1` — core dump on full UC filter (RQ-11)

## Interpretation

The geomean across kernels is dominated by the kernels with the largest absolute cost. For
those large kernels (e.g. uc7_dfa, uc14_sort), the JIT pipeline parameter changes shift the
combined cost by only a few percent — the optimization is **robust within the search space**.

This contradicts the smaller-kernel single-point readings (e.g. `early_prune=0` showed a
~50× JIT slowdown on one kernel) and tells us:

- The dominant cost component is the **execution phase**, not JIT overhead.
- Small kernels reap large *relative* JIT speedups from `early_prune` and `o3_final`, but
  these contribute little to the **geomean of combined cost** at MEDIUM size.
- The TPE optimizer's convergence to `early_prune=1, o3_final=1, large_module_max=0` is
  driven primarily by JIT overhead reduction on small kernels and a small but consistent
  exec improvement from `o3_final`.

## Implications for Search Space (Next Iteration)

**Parameters to prune from the search space** (insensitive in this iteration, save Optuna trials):
- `unroll_max` — fix at default (no measurable benefit anywhere in 1–512).
- `large_module_max` — fix at `0` (all UC kernels are below the threshold).

**Parameters to keep but with coarser grid**:
- `fixpoint_max` — keep, but coarse grid `{3, 5, 10, 20}` is sufficient (range 6–25 is flat).
- `early_prune` — keep as a binary flag (still has dramatic per-kernel effects).
- `o3_final` — keep as a binary flag (1.09 ratio is the strongest signal).

**Parameters that need a different evaluation lens**:
- For all five, repeat the sweep on **small kernels in isolation** (not the geomean across
  all 18). The geomean hides the parameter's role for kernels where JIT cost dominates.

## Caveats

- 1 rep per point — single-trial noise is significant; ratios within ~5% of 1.0 should not
  be treated as strict orderings.
- The base config used the trial-16 optimum (`fixpoint_max=9, unroll_max=5`), not the
  trial-17 optimum (`fixpoint_max=7, unroll_max=4`). The difference between these two base
  points is within noise, so the OAT conclusions still hold.
- `sens_fixpoint_max_1` is recorded as a `NULL` row (SIGSEGV — RQ-15). It is excluded from
  the sweep.
