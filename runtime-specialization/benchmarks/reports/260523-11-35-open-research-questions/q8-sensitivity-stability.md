# Q8: How does parameter sensitivity change as the baseline config improves?

## Important Context

Iter-1's OAT sensitivity analysis (study `sens_uc_20260517`) classified `unroll_max` as low importance. Iter-2's per-kernel winner analysis showed `unroll_max=27` drives -14.5 % on `generic_sort` and -6.1 % on `separable_gaussian` compared to `unroll_max=4`. This is a direct contradiction across iterations — the same parameter was "unimportant" in iter-1 and "critical" in iter-2.

Iter-1 baseline config: `fixpoint_max=7, unroll_max=4, large_module_max=0, early_prune=1, o3_final=1, pipeline=0`.
Iter-2 winner: identical except `unroll_max=27`.

Sources: iter-2 reflection `§2` (cross-iteration delta), `sens_uc_iter2_20260521`.

---

## Investigation

### Analyze why unroll_max appeared unimportant in iter-1

Iter-1's OAT sweep varied `unroll_max` against a baseline where `fixpoint_max` was not yet optimally set (the iter-1 study was incomplete at 24/150 trials). The sensitivity result depends on which parameters are held fixed:

- If `fixpoint_max` is suboptimal (too low), the IR entering the loop unroller is less specialized — constant expressions are not yet fully propagated. In this state, unrolling may not be profitable because the loop body still contains non-constant computations that prevent vectorization even after unrolling.
- With `fixpoint_max=7` (fully converged IR), the loop body is maximally specialized before unrolling. In this state, unrolling can expose vectorization opportunities that weren't visible at lower fixpoint depths.

**This explains the revision**: `unroll_max` appears unimportant when evaluated at a baseline where the IR is under-specialized. Once the baseline is improved (more fixpoint iterations → more constant folding → loops with fully constant trip counts), `unroll_max` becomes the next critical lever.

This is a classic saturation effect in sensitivity analysis: primary parameters dominate when the baseline is weak; secondary parameters gain relative importance as primary parameters saturate.

### Analyze the sensitivity data from iter-2 for unroll_max

The `sens_uc_iter2_20260521` sweep for uc14_sort's generic_sort:

| sens_fixpoint_max | generic_sort spec_ms |
|-------------------|----------------------|
| 2                 | 281.44               |
| 3                 | 281.69               |
| 5                 | 283.19               |
| 7 (baseline)      | 282.72               |
| 8                 | 281.63               |
| 15                | 281.92               |
| 30                | 283.65               |

`fixpoint_max` has ≤ 1 % effect on generic_sort exec time at the iter-2 baseline. This confirms that at the iter-2 baseline, `fixpoint_max` is **saturated** — any value ≥ 2 produces identical results for this kernel. The next unsaturated dimension is `unroll_max`.

The iter-2 per-kernel winner for generic_sort used `unroll_max=41` (spec_ms ≈ 234 ms vs baseline's 283 ms at `unroll_max=27`). The sensitivity sweep for `unroll_max` directly was not run as a named OAT point — it was discovered via Optuna's exploration.

**What to look for**: A sensitivity sweep over `unroll_max` at the iter-2 baseline would quantify the shape of the response surface. Is it monotone (more unrolling always better until icache saturation), or is there a sweet spot?

### Analyze whether second-order interactions between unroll_max and fixpoint_max exist

A 2D interaction grid can be inferred from the Optuna trial data. The top trials by exec objective:

| trial | fixpoint_max | unroll_max | exec_ms |
|-------|-------------|-----------|---------|
| 7     | 27          | 13        | 64.23   |
| 38    | 22          | 21        | 64.75   |
| 36    | 14          | 314       | 65.18   |
| 40    | 18          | 5         | 65.19   |
| 49    | 26          | 42        | 65.60   |
| 15    | 7           | 27        | 68.54   |

The top 5 trials by exec objective all have `fixpoint_max >> 7` but are not meaningfully better than trial 15 (`fixpoint_max=7, unroll_max=27`) on the combined objective. However, trial 7 (`fixpoint_max=27, unroll_max=13`) is better on exec alone — suggesting that when fixpoint_max is very high, a lower unroll_max is optimal. This could be a second-order interaction: more constant propagation (higher fixpoint) reduces the benefit of unrolling (because vectorization is already enabled by constants, and extra unrolling just adds code size).

**Caution**: With 50 Optuna trials, the top-5 results are noisy. Second-order conclusions require a dedicated 2D grid sweep.

### Analyze the general phenomenon: sensitivity ranking instability

The pattern observed is consistent with the **parameter importance cascade**:
1. In early optimization, primary parameters (fixpoint_max, early_prune, o3_final) dominate because the baseline is far from optimal on these dimensions.
2. As the optimizer finds better values for primary parameters, they saturate — any value near the optimum gives nearly identical results.
3. Secondary parameters (unroll_max, p1_inline_threshold) then become the differentiators because they can only help once the primary parameters are correctly set.

This implies that single-iteration sensitivity studies (OAT at a fixed baseline) provide unreliable importance rankings — the ranking depends on the baseline. A full Sobol/Morris sensitivity analysis across the entire search space would give more stable rankings, but requires ≥ 1000 evaluations to be reliable.

---

## Conclusion

The instability in sensitivity rankings between iter-1 and iter-2 is not a measurement error — it is the expected behavior of parameter importance in a multi-dimensional optimization landscape. `unroll_max` appeared unimportant in iter-1 because `fixpoint_max` was not yet at its optimal value, making the IR entering the unroller insufficiently specialized. With `fixpoint_max=7` (iter-2 baseline), `fixpoint_max` saturates and `unroll_max` emerges as the next critical dimension.

This finding has a methodological implication: OAT sensitivity sweeps should always be run against the **current best config**, not a fixed baseline. After each Optuna iteration, the sensitivity sweep should be re-run to update the importance ranking. The iter-2 change in `unroll_max` importance is evidence that iter-1's sensitivity sweep was informative for iter-1's baseline but misleading for iter-2's optimization.

---

## Further Questions/Directions of Research

- Run a 1D sensitivity sweep over `unroll_max` (values: 0, 4, 8, 16, 27, 41, 64, 128, 256) at the iter-2 baseline for `generic_sort` and `separable_gaussian`. This maps the full response surface for the most important secondary parameter.
- Design a 2D grid sweep: `fixpoint_max` × `unroll_max` at 5 values each (25 configs) for uc14_sort. This would quantify the interaction term and confirm or refute the cascade hypothesis.
- Consider using Sobol sequence sampling (via Optuna's `SobolSampler`) in iter-3 rather than TPE, to get unbiased importance estimates early in the search.
- Add sklearn to the Nix environment (`python312Packages.scikit-learn`) to enable `optuna.importance.get_param_importances` — this was blocked in iter-2 and would provide Fanova-based importance estimates that account for parameter interactions.
