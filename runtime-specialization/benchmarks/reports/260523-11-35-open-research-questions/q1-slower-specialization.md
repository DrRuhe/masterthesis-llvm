# Q1: Why are some use cases slower when specialized?

## Important Context

Two distinct slowdown patterns exist and must be distinguished:

1. **P1 regression relative to P0**: `uc2_conv` and `uc8_ivm` execute 37–46 % slower under Pipeline-1 than under Pipeline-0. Under P0 both groups still produce a net exec speedup over unspecialized.
2. **Genuine slower-than-unspecialized**: `uc8_ivm/batch_delta` and `uc8_ivm/multi_agg_delta` produce spec_ms > unspec_ms under *every* pipeline (P0 spec_ratio ≈ 4.4 and 1.55 respectively). `uc7_dfa` is only modestly faster than unspecialized (spec_ratio ≈ 0.81).

Sources: `ablation_uc_iter2_20260521`, `sens_uc_iter2_20260521`.

---

## Investigation

### Analyze the invariance of uc2_conv slowdown to pipeline parameters

From the sensitivity study (`sens_uc_iter2_20260521`), `uc2_conv`'s P0 spec_ratio is stable across all parameter sweeps:

| config              | box_filter spec_ratio | edge_det spec_ratio | sep_gauss spec_ratio |
|---------------------|-----------------------|----------------------|----------------------|
| sens_fixpoint_max_2  | 0.352                 | 0.647                | 0.521                |
| sens_fixpoint_max_7  | 0.352                 | 0.665                | 0.509                |
| sens_fixpoint_max_15 | 0.351                 | 0.645                | 0.512                |
| sens_fixpoint_max_30 | 0.349                 | 0.643                | 0.520                |
| sens_early_prune_0   | 0.351                 | 0.646                | 0.518                |
| sens_early_prune_1   | 0.351                 | 0.655                | 0.515                |

**What to look for**: any parameter that breaks the pattern (changes ratio by > 5 %).

**Interpretation**: All ratios are in the range 0.35–0.52 regardless of fixpoint depth (2–30 iterations). This rules out insufficient constant propagation as the cause of slowdown — the constants ARE being baked in, and extra fixpoint iterations don't help. The slowdown is therefore either in the backend (vectorization loss, icache pressure) or an intrinsic property of the kernel shape.

One exception: `sens_o3_final_0|box_filter` shows spec_ms = 1250 ms vs unspec_ms = 1082 ms (ratio = 1.16, i.e. 16 % **slower** than unspecialized), while `sens_o3_final_1|box_filter` gives ratio = 0.352 (2.84× faster). This dramatic flip means the fixpoint IR is **not directly executable at competitive speed** — it requires the O3 final-optimization pass to lower it to efficient machine code. Without O3, the over-inlined/expanded fixpoint IR performs worse than the unspecialized version.

### Analyze uc8_ivm/batch_delta genuine slowdown

Ablation medians (all P0 configs grouped):

| kernel        | jit_ms | spec_ms | unspec_ms | spec_ratio |
|---------------|--------|---------|-----------|------------|
| batch_delta   | ~42    | ~62     | ~12       | ~5.2       |
| multi_agg_delta | ~41  | ~20     | ~13       | ~1.55      |
| apply_row_delta | ~42  | ~18     | ~30       | ~0.59      |

`batch_delta` is the pathological case: unspecialized runs in 12 ms, specialized in 62 ms. The JIT overhead (42 ms) adds insult to injury — the combined cost is 104 ms vs 12 ms unspecialized.

**What to look for**: the JIT module size (instruction count before/after specialization) for batch_delta. A kernel that is naturally very fast (12 ms, possibly a tight loop over a small array) may be fully vectorizable by the standard compiler but the JIT specialization process may destroy the vectorization hints (e.g., loop bounds not constant after specialization if the array size isn't a specialization argument).

**Interpretation**: The unspecialized batch_delta is likely already well-optimized by the AOT compiler (auto-vectorized SIMD). Specialization changes the code in ways that break vectorization or increase code size, making the specialized version slower. Unlike sort (which has an indirect call) or SQL (which has a predicate to fold), batch_delta may not have a "hot operation" that constants can eliminate — constants baked in by the JIT don't create qualitatively different control flow.

### Analyze uc7_dfa's modest speedup

`uc7_dfa` consistently shows spec_ratio ≈ 0.81 (about 23 % faster), which is close to 1.0. DFA matching (email/URL pattern recognition) is typically dominated by a state-machine dispatch loop. Specializing the DFA for a specific pattern compiles the state table into constants, removing the table-lookup indirection. The 23 % gain suggests the devirtualization/constant-folding works, but the DFA loop itself is still memory-bound on the state transition table — the speedup is limited by cache behavior rather than computation.

### Analyze P1's regression mechanism on uc2_conv

Under P1 (`pipeline_1_default`), `uc2_conv` spec_ratio moves from ~0.41 (P0) to ~0.62. P1 inlines functions whose call cost exceeds a size-based budget threshold. For convolution kernels, the hot inner loop already runs efficiently; P1's budget inlining pulls in helper functions or expands the loop body, increasing code size beyond L1 icache capacity without eliminating any hot branch or indirect call.

Evidence: `sens_p1_max_module_growth_*` for uc14_sort shows that increasing growth cap (2.0–5.0) gives small improvements for sort (-3 % spec_ms), while for uc2_conv the same growth cap variations have negligible effect on spec_ratio under P0. P1's mechanism inherently differs in how it grows the module relative to what the conv kernels need.

---

## Conclusion

The uc2_conv slowdown under P1 (vs P0) is a **pipeline regression**, not a fundamental property: P0 already specializes conv correctly (2.4–2.8× faster than unspecialized), and P1 degrades that by inlining code that increases code size without reducing the critical-path computation. The fix requires not applying P1 budget-inlining to kernels without indirect calls in the hot path.

The uc8_ivm/batch_delta genuine slowdown is a **specialization-futility case**: the kernel is already AOT-optimal for its shape; JIT specialization of the loop body adds code rather than removing computation. This is a fundamental limitation — JIT specialization yields no benefit when there is no runtime-constant control flow to fold away.

The data strongly suggests these are two distinct failure modes and should be tracked separately in the thesis.

---

## Further Questions/Directions of Research

- Run `jit_analysis` benchmarks specifically for `batch_delta` and `multi_agg_delta` to capture pass-trace data: measure instruction count before/after prune, fixpoint, and final — quantify how many instructions remain vs. original.
- Profile `batch_delta` with `perf stat --event=L1-icache-misses,instructions` to distinguish icache pressure from compute slowdown.
- Check whether batch_delta's unspecialized fast path benefits from auto-vectorization (look for AVX2/SSE instructions via `objdump -d`). If AOT uses SIMD and JIT does not, that explains the gap.
- What happens when `unroll_max=0` is used on uc2_conv? If the slowdown persists, unrolling is not the cause. If it improves, the expanded unrolled loop is hurting icache. (Data partially available from `no_unroll` config — spec_ratio barely changed, suggesting unrolling is not the primary cause.)
- Can a "specialization-worthiness" heuristic be derived from the IR before JIT: if there are no indirect calls and no constant-foldable branches, skip specialization?
