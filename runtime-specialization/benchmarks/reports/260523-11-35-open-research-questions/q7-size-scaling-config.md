# Q7: Does the optimal pipeline change with workload size (SMALL / MEDIUM / LARGE)?

## Important Context

Iter-1's size-scaling study confirmed that **relative exec speedup** is stable across MEDIUM / LARGE / EXTRALARGE sizes (the ratio unspec/spec is approximately constant as problem size scales). However, iter-1 and iter-2 did not validate that the **optimal pipeline configuration** is stable across sizes — only that the default config's speedup transfers.

The key concern: `unroll_max=27` was identified as important in iter-2 based on UC/MEDIUM benchmarks. At LARGE or EXTRALARGE sizes, loop trip counts are proportionally larger — unrolling a loop 27× may either (a) become more profitable (amortizing loop overhead across more iterations) or (b) become harmful (code-size pressure degrades icache at large sizes).

Source data: `ablation_uc_20260517` (iter-1 ablation, may have different sizes), size-scaling reports in `260506-12-37-polybench_size_scaling`, `260514-17-20-polybench_size_scaling`.

---

## Investigation

### Analyze available size-scaling data

The iter-1 and iter-2 ablation studies both used MEDIUM-size benchmarks exclusively. The size-scaling reports in `260506-*` and `260514-*` measured polybench kernels at MEDIUM/LARGE/EXTRALARGE but did not vary pipeline parameters — they measured the default config at different sizes.

**What is known**: For polybench (proxy for large-loop kernels), the size-scaling study showed that the speedup ratio is stable. This is reassuring but does not address config sensitivity.

**What is not known**: Whether `unroll_max=27` remains optimal at EXTRALARGE sizes for UC kernels.

### Analyze the theoretical effect of problem size on unroll profitability

For a loop that executes N iterations:
- Unrolling by factor U produces a loop with N/U iterations and a U-instruction body
- Benefit: reduces loop overhead (branch, counter update) by factor U
- Harm: increases code size by factor U; if U × body_size > L1 icache capacity, icache misses appear

At MEDIUM sizes (e.g., 1000 × 1000 image for box_filter), the inner kernel loop iterates `diam²` times (e.g., 11² = 121 iterations for radius=5). Unrolling 27× on a 121-iteration loop partially unrolls — the compiler produces a 27-iteration body with a cleanup loop.

At EXTRALARGE sizes, the outer loops (height × width = 4000 × 4000) still have the same inner diam² loop. The optimal unroll factor for the inner loop doesn't change with image size. Thus `unroll_max` should be *size-independent* for conv-style kernels.

For sort kernels (generic_sort, multi_key_sort), the loop trip count at EXTRALARGE scales as O(N log N). Unrolling the inner loop of a merge step by 27× at N=10M may exceed icache, whereas at N=100K it does not. This suggests `unroll_max=27` might regress for sort at very large inputs.

### Analyze what a size-stability experiment would require

A minimal config-stability study at multiple sizes would:
1. Take the iter-2 winner config (`fixpoint_max=7, unroll_max=27, early_prune=1, o3_final=1`)
2. Take two alternative unroll_max values: 4 (iter-1 optimum) and 54 (2× iter-2 optimum)
3. Run all 3 unroll configs at MEDIUM, LARGE, EXTRALARGE for 2 UC groups (uc14_sort and uc2_conv)
4. Compare relative exec speedup across sizes and configs

This experiment requires ~18 runs × 5 reps = 90 total runs — a ~2 hour experiment.

**What to look for**: If the optimal unroll_max shifts between MEDIUM and EXTRALARGE for uc14_sort, the config is NOT size-stable and per-size tuning is required. If the ratios are within 5 % of each other, the config generalizes.

### Analyze whether the size-scaling stability claim holds for the iter-2 winner

From iter-1's size-scaling study: relative speedup was stable across sizes for the *default* config. The iter-2 winner differs primarily in `unroll_max=27`. If the size-scaling stability holds for this new config:
- The exec speedup of iter-2 winner at EXTRALARGE should be approximately 14.5 % better than iter-1 winner (the gain seen at MEDIUM for generic_sort), not more or less.
- If the gain is smaller at EXTRALARGE, unrolling becomes less beneficial with larger inputs.
- If the gain is larger at EXTRALARGE, unrolling is even more profitable with larger trip counts (unlikely for fixed inner loops but possible for outer loops).

---

## Conclusion

The size-scaling stability of the optimal config is **theoretically likely** for conv-style kernels (fixed inner loop trip count independent of image size) but **uncertain for sort kernels** where the loop structure changes with N. The iter-2 sensitivity data shows `unroll_max` matters most for `generic_sort` — this is the kernel most likely to show size-dependent optimal config.

Without running the 3-config × 3-size experiment, the current answer is: "We know relative speedup scales, but we cannot confirm optimal config scales." This is a gap that is relatively cheap to fill (90 runs, ~2 hours).

---

## Further Questions/Directions of Research

- Run the 3-config × 3-size experiment for uc14_sort and uc2_conv (see missing-data.md). This directly answers SQ1 and SQ4 with respect to size scaling.
- Check if the iter-1 size-scaling report data is queryable from DuckDB — if so, the experiment is partially done already.
- For the thesis, does size-stability need to be statistically proven or is the theoretical argument sufficient? If iter-2's size-scaling stability holds (exec speedup scales uniformly), a theoretical argument may be adequate given the investment required for a full per-size Optuna run.
- Consider adding `kv_s` (size tag) to the sensitivity study filter in future iterations so that per-size sensitivity data is automatically collected.
