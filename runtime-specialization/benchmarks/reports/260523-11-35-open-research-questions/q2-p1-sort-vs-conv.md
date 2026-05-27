# Q2: Why does Pipeline-1 help `uc14_sort` but hurt others?

## Important Context

Pipeline-1 (P1) differs from Pipeline-0 (P0) by adding a budget-aware inlining pass that promotes calls into the specialization module if they fall within a size-growth budget (`p1_max_module_growth` × original module size and per-call inline threshold `p1_inline_threshold`). The hypothesis is that P1 enables profitable inlining of runtime-resolved callees that P0's `ConstantArgAlwaysInlinePass` misses.

Key numbers from `ablation_uc_iter2_20260521`:

| Group       | P0 best spec_ms | P1 best spec_ms | P1 vs P0 |
|-------------|-----------------|-----------------|----------|
| uc14_sort   | 281.27          | 233.67          | −17 %    |
| uc2_conv    | 110.19          | 150.96          | +37 %    |
| uc8_ivm     | 17.84           | 26.03           | +46 %    |
| uc12_groupby| 10.04           | 11.84           | +18 %    |
| uc1_sql     | 7.66            | 8.19            | +7 %     |
| uc7_dfa     | 89.12           | 90.50           | +2 % (≈tie)|

P1 wins on exactly one group (`uc14_sort/generic_sort` kernel). The `p1_max_module_growth` sensitivity data for uc14_sort (from `sens_uc_iter2_20260521`) shows:

| growth cap | generic_sort spec_ms |
|------------|----------------------|
| 1.0        | 285.35               |
| 1.5        | 283.73               |
| 2.0        | **274.51**           |
| 3.0        | 275.37               |
| 4.0        | 272.32               |
| 5.0        | 273.28               |

A growth cap ≥ 2.0 yields meaningful improvement (−4 % vs cap=1.0), suggesting the win requires moderate module expansion.

---

## Investigation

### Analyze the structural property that makes sort profitable for P1

The `generic_sort` kernel (`UC14GenericSortLowKernels.cpp`) takes a comparator as a function pointer:

```cpp
void generic_sort(void* data, int64_t n_elements, int element_size,
                  int (*comparator)(const void*, const void*)) {
```

When the JIT specializes the call with `int64_asc_cmp` as the comparator constant, it must inline `int64_asc_cmp` into the sort body to eliminate the indirect call overhead. Under P0, `ConstantArgAlwaysInlinePass` marks the call site `alwaysinline` only if the argument is a `Constant`. A function pointer passed as a runtime constant should qualify, but only if IPSCCP can prove the pointer is constant through the call chain.

Under P1, the budget inliner promotes the comparator regardless of whether IPSCCP has already proven it constant — it inlines based on size budget alone. This inlines `int64_asc_cmp` (a small function: two memcpy + comparison ≈ 10 instructions) into every call site in the hot sort inner loop, enabling:
1. Elimination of the indirect branch (direct comparison instruction)
2. Constant-propagation of `sizeof(int64_t)=8` into `memcpy` calls, enabling the compiler to lower them to scalar loads
3. Visibility of the comparison result to enable loop vectorization

This is the textbook case where inlining a small, frequently-called function is highly profitable.

**What to look for**: `jit_analysis` pass-trace data for `generic_sort` would show instruction-count changes at the P1 vs P0 inlining step. The pruned module instruction count should be similar, but post-fixpoint should diverge.

### Analyze why P1 hurts convolution kernels

`box_filter` (`UC2BoxFilterLowKernels.cpp`) specializes `width`, `height`, `radius`:

```cpp
void box_filter(const float* src, float* dst, int width, int height, int radius) {
    const int diam = 2 * radius + 1;
    const float inv_area = 1.0f / (float)(diam * diam);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            for (int ky = -radius; ky <= radius; ++ky) {
                for (int kx = -radius; kx <= radius; ++kx) { ...
```

There are **no indirect calls in the hot path**. All function calls are direct: `std::max`, `std::min`, and array indexing. These are already inlined by the AOT compiler. P1's budget inlining cannot find profitable indirect calls to eliminate because there are none.

What P1 *can* do for conv kernels is inline any helper functions that access `this` (e.g., the lambda closure or any abstraction-layer wrappers). At the `low` abstraction level, the lambda wraps the kernel call directly — there may be one level of function-pointer or vtable dispatch through the specializer's wrapper. P1 might inline this wrapper, adding code from the calling context into the module. This increases the module size without shortening the critical compute path, causing L1 icache pressure in the tight nested loop.

**What to look for**: module instruction count before/after P1 application for `box_filter`. If P1 doubles the instruction count (by pulling in benchmark infrastructure or wrapper code), that explains the regression.

### Analyze whether a minimal P1 configuration captures sort win without regression

The `p1_inline_threshold` sensitivity data for uc14_sort (from `sens_uc_iter2_20260521`) shows:

| threshold | generic_sort spec_ms |
|-----------|----------------------|
| 50        | 286.53               |
| 100       | 281.71               |
| 225       | 281.61               |
| 500       | 281.34               |
| 1000      | 285.02               |
| 2000      | 284.47               |

The threshold variations (50–2000) do **not** reproduce the 233 ms achieved by the `pipeline_1_aggressive_budget` config in Phase B. This suggests the threshold alone isn't the lever — the gain requires the full P1 inlining path, not just a threshold change. The Phase B winner used threshold=1000, growth=4.0.

**Interpretation**: The comparator is inlined in all P1 configurations once the growth budget permits it (growth ≥ 2.0). The threshold primarily gates *which* other call sites are also inlined; for generic_sort, the comparator is the dominant call and any threshold ≥ comparator size will inline it.

### Analyze automatic pipeline selection from IR properties

Structural IR features that predict P1 profitability:
- **Indirect call count in the JIT module's hot functions**: if ≥ 1 indirect call exists (function pointer or vtable) that is not yet resolved to a constant, P1 is potentially profitable.
- **Ratio of indirect calls to total instructions**: high ratio → P1 wins; near zero → P1 will regress.
- **Module instruction count post-prune**: if the pruned module is large (many functions survived GlobalDCE), P1's growth budget is consumed by the existing functions, leaving less room for profitable inlining.

For `generic_sort`, the module post-prune is small (the comparator + sort body), and there is exactly one unresolved indirect call (the comparator pointer). For `box_filter`, the pruned module has no unresolved indirect calls — P1 has nothing useful to inline.

This suggests a cheap pre-JIT heuristic: *count indirect calls in the pruned IR; if count ≥ 1, apply P1; otherwise apply P0.* This would require exposing the post-prune IR to the pipeline selector, which is feasible since the prune step runs first.

---

## Conclusion

P1's benefit for `uc14_sort` is driven by a single structural feature: the sort comparator is passed as a function pointer and constitutes the dominant per-element operation (called O(n log n) times). P1's module-growth budget is spent profitably on inlining a small, frequent callee. For convolution, DFA matching, and groupby operations, there are no unresolved indirect calls in the hot loop — P1 adds code size without eliminating any branch overhead, causing regression.

The fix is structural: P1 should only be applied to kernels where the JIT IR (post-prune) contains at least one unresolved indirect call in a hot function. This is a one-pass IR analysis (count `call i64* %fp` or `call @vtable_slot` instructions in functions with high use counts).

---

## Further Questions/Directions of Research

- Generate `jit_analysis` pass traces for both `generic_sort` (P1) and `box_filter` (P1) to measure the instruction count after P1's inlining step vs P0's inlining step. The delta quantifies the code size impact.
- Implement the IR heuristic (indirect call count) as a pre-JIT check and validate on the 6 UC groups: does it correctly predict P0 vs P1 for all groups?
- Can `p1_max_module_growth=1.0` (no growth allowed) be used as a safe fallback that avoids P1 regressions, while `p1_max_module_growth=2.0+` selectively enables the sort win? The sens data shows growth=1.0 still gives 285 ms for generic_sort — not as good as growth=4.0 (272 ms) but still within 5 % of P0. Could this be a safe universal P1 setting?
- Why does uc8_ivm regress so severely under P1 (46 %)? uc8_ivm/batch_delta is already slower than unspecialized under P0 — investigate whether P1 inlines the IVM update function into the wrapper, pulling in more computation rather than eliminating it.
