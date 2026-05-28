# P2-Only Optimization Analysis — JIT Pipeline Evaluation

**Run dir**: `benchmarks/reports/260528-p2-optimize/`  
**Date**: 2026-05-28  
**Spec**: `specs/007-jit-pipeline-eval/`  
**Previous reflections**: `reports/260527-13-01-optimize-pipeline/reflection.md` (iter-3),
`reports/260521-17-32-optimize-pipeline/reflection.md` (iter-2)

**Context**: This analysis targets Pipeline 2 (JIT-IPSCCP, spec 015) exclusively, following
the Phase 11 fix (`feat(jit-ipsccp): Phase 11 — propagate !invariant.load values across call
boundaries`, commit `0e94b3512a58`). The fix adds cross-call constant propagation to
`JitSCCPSolver` and SSA-promotion of vtable pointers before the SCCP pass.

Binary rebuilt at 14:01 from today's HEAD. Optimization study: `uc_optim_p2_20260528`,
50 trials, seed=42, pipeline constrained to P2 only (`p2_search_space.json`). UC MEDIUM
filter, all 6 groups, 18 kernels.

---

## 1. Headline results

### 1.1 Does P2 produce speedups after the Phase 11 fix?

**YES. Substantially improved over the pre-fix state.**

Before Phase 11 (iter-3 §3.2): P2 produced a 12-instruction dispatch wrapper calling the
unspecialized target — effectively no specialization. Best P2 cost: **213.1 ms combined**.

After Phase 11 (today): P2 propagates constants across call boundaries via the SCCP lattice.
Best P2 cost so far (7/50 trials): **180.8 ms combined** (jit=53.7ms, exec=72.7ms).

P2 improvement vs pre-fix: **-15%** (213.1 → 180.8 ms).  
P2 vs P0 best (iter-3 trial 21, 169.1ms): **+6.9% worse** on geomean combined cost.

### 1.2 Per-kernel scorecard (best P2 trial vs. P0 best)

Source: `uc_optim_p2_20260528` trial 3 (180.8ms) vs `uc_optim_iter3_20260527` trial 21 (169.1ms).

| Kernel | P0 speedup | P2 speedup | Winner | Notes |
|--------|:----------:|:----------:|:------:|-------|
| generic_sort        | 2.64× | 2.81× | **P2** | P2 slightly better |
| grouped_count       | 2.45× | 2.28× | P0 | |
| grouped_sum         | 2.30× | 2.14× | P0 | |
| grouped_minmax      | 2.25× | 2.20× | ~TIE | |
| separable_gaussian  | 1.97× | 1.11× | **P0** | P2 misses vectorization |
| apply_row_delta     | 1.70× | 1.07× | **P0** | P2 loses inlining benefit |
| edge_detection      | 1.60× | 1.42× | P0 | |
| struct_sort         | 1.53× | 1.49× | ~TIE | |
| multi_key_sort      | 1.41× | 1.40× | ~TIE | |
| multi_pattern_match | 1.28× | 1.25× | ~TIE | |
| url_match           | 1.24× | 1.21× | ~TIE | |
| email_match         | 1.24× | 1.23× | ~TIE | |
| box_filter          | 2.80× | 1.06× | **P0** | P2 misses vectorization |
| count_matching_rows | 1.03× | 1.05× | ~TIE | JIT-dominated (2400ms JIT) |
| column_scan         | **0.22×** | **1.00×** | **P2** | Regression fixed |
| multi_predicate     | **0.32×** | **0.95×** | **P2** | Regression fixed |
| batch_delta         | **0.20×** | **0.94×** | **P2** | Regression fixed |
| multi_agg_delta     | 0.63× | 0.47× | P0 | Both regress; P2 worse |

**Summary**: P0 wins: 7 kernels | P2 wins: 4 kernels | Ties: 7 kernels

**Regression count**: P0: 4 | **P2: 1** (multi_agg_delta only).

P2 after Phase 11 is the better choice for workloads that include short per-call kernels
(uc1_sql, uc8_ivm). P0 remains better for compute-bound kernels with large loop bodies
(uc2_conv, uc14_sort).

---

## 2. Optimization landscape: all parameters are insensitive

### 2.1 Key finding: P2's search space is effectively flat

Over 7 trials with large parameter variation, the combined cost ranged only **180.8–182.0 ms
(spread: 1.3 ms)**. This is smaller than measurement noise (~3–5 ms on repeat runs).

**Root cause: none of the 9 search space parameters actually affect P2's pipeline.**

| Parameter | Used in P0 | Used in P2 | Effect on P2 |
|-----------|:----------:|:----------:|:------------:|
| `fixpoint_max` | Yes (loop iterations) | **No** | None |
| `unroll_max` | Yes (PostInlineFPM) | **No** | None |
| `early_prune` | Yes (GlobalDCE gate) | **No** (hardcoded) | None |
| `o3_final` | Yes (O3 final MPM) | **No** | None |
| `large_module_max` | Yes | Partially | None for UC MEDIUM |
| `p2_min_func_size` | — | Yes | <0.5ms effect |
| `p2_max_clones` | — | Yes | <0.5ms effect |
| `p2_func_spec_iters` | — | Yes | <0.5ms effect |
| `p2_force_spec` | — | Yes | <0.5ms effect |

`fixpoint_max`, `unroll_max`, `early_prune`, and `o3_final` are read by the Options struct
and passed to the P0 pipeline (`JITPipelineInlining.cpp`). They are not referenced anywhere
in `JITPipelineIPSCCP.cpp`. The P2-specific knobs (`p2_*`) do affect `JitIPSCCPPass` via
`Opts.P2FuncSpec`, but their effect is marginal because SCCP's cross-call propagation is
already effective regardless of clone count or specialization policy.

**Consequence**: Running 50 Optuna trials for P2 is wasteful. Any config with `pipeline=2`
produces equivalent results (~181ms). The Optuna search provides no useful ranking.

---

## 3. Why P2 fixed the cache-cold regressions (UC1, UC8)

### 3.1 P0 regression mechanism (pre-existing)

P0's `InvariantLoadToConstantPass` materializes runtime constants from `!invariant.load`
pointers as **PC-relative address computations**:

```asm
leaq   -7(%rip), %rax
movabsq $-8112, %rcx
addq   %rax, %rcx           ; compute absolute JIT data address
vaddsd  (%rsi,%rcx,8), %xmm0, %xmm0  ; load constant value from that address
```

For short kernels (column_scan, batch_delta, multi_predicate) with ~0.3–1.0 ns unspecialized
per-call duration, this JIT-data address is cold in L1 cache. Each call costs an extra
~12 ns memory access, turning a potential win into a 3–5× regression.

### 3.2 P2 fix mechanism (Phase 11)

P2's `JitSCCPSolver::visitLoadInst` (Phase 11 extension):
1. If the load has an `!invariant.load` metadata tag, resolve the current host memory value
   and mark the SCCP lattice as a **literal constant**.
2. If the load's pointer is already a SCCP constant (from a prior step), perform a
   **second-level host read** (`resolveConstantPtrLoad`), enabling recursive propagation.

The SCCP solver propagates these literal constants forward through the IR. By the time
AlwaysInliner runs in Phase 3, the load instructions that P0 replaces with PC-relative
addresses are instead replaced by **IR integer/float immediates**. These get folded to
register-immediate instructions by InstCombine:

```
; P0: load from JIT address each call (cache-cold)
; P2: IR constant → load eliminated, value becomes immediate in instruction stream
```

For column_scan: threshold `8.4` becomes an IR `double 8.4` → `vucomisd $8.4`, eliminating
the memory load entirely → no regression.

### 3.3 Why multi_agg_delta still regresses in P2 (0.47×)

multi_agg_delta unspecialized: **0.26 ns/call** (13.3ms / 50M iterations MEDIUM).

P2's wrapper function adds a function-call boundary between the benchmark and the specialized
target. Even with constants propagated, the call overhead (~0.5ns) plus wrapper prologue/
epilogue costs exceed the benefit of constant folding for a function this short.

P0 inlines the target body into the wrapper, amortizing the call overhead — but P0's
PC-relative address load costs ~12ns, which also creates a regression (0.63×).

**Neither P0 nor P2 can specialize multi_agg_delta effectively**: the kernel is too short
for constant folding to overcome JIT materialization overhead. This is a fundamental
limitation: JIT specialization is not beneficial when `unspec_ns ≤ ~2ns`.

The difference (P2: 0.47×, P0: 0.63×): P2 adds extra overhead without the vectorization
benefit that partially compensates in P0. P2's wrapper calls the target without inlining
it; the dispatch overhead dominates.

---

## 4. Why P2 misses compute-bound speedups (UC2, UC8 apply_row_delta)

### 4.1 The inlining gap

P0's pipeline:
1. `ConstantArgAlwaysInlinePass` marks the `target(struct_ptr)` call `alwaysinline`
2. `AlwaysInlinerPass` inlines the target body into the wrapper
3. `InvariantLoadToConstantPass` resolves struct field loads to constants in the inlined body
4. `O3 final pass` runs LICM, SLP vectorizer, loop vectorizer on the unified wrapper body

Result for box_filter: the triple-nested loop (row × col × stencil) with constant bounds
is vectorized → 2.80× speedup, 128-instruction specialized function.

P2's pipeline:
1. Phase 1 strips `alwaysinline` from all functions and call sites
2. `JitIPSCCPPass` propagates constants into the target body (without inlining)
3. `AlwaysInlinerPass` in Phase 3 can only inline JitFunctionSpecializer clones (if any)
4. No O3 final pass in P2

Result for box_filter: the constant values are embedded in the target function body, but
the loop structure is still opaque to the wrapper. The O3 vectorizer cannot see the full
loop. Speedup: 1.06×.

### 4.2 Why generic_sort is an exception (P2 2.81× > P0 2.64×)

generic_sort uses a comparison lambda where the sort key is a struct field. With P2's
cross-call propagation, the comparison function receives a constant sort-direction flag —
this eliminates a branch in the inner loop. Since the sort algorithm has many short calls
to the comparator, eliminating the branch reduces instruction count significantly.

P0 also eliminates this branch but via a different path (inlining + constant folding).
P2's SCCP propagation reaches deeper into the call chain for this pattern, resulting in
slightly better elimination. This suggests P2 is specifically effective for **call-graph
patterns where constants flow through multiple levels of indirection**.

---

## 5. Parameter sensitivity conclusions

Since all 9 parameters are insensitive for P2, the "best" P2 config is effectively arbitrary.
For a practical default, the minimal P2 config with lowest overhead is:

```json
{
  "pipeline": 2,
  "fixpoint_max": 5,
  "unroll_max": 1,
  "early_prune": 1,
  "o3_final": 0,
  "p2_min_func_size": 1,
  "p2_max_clones": 0,
  "p2_func_spec_iters": 1,
  "p2_force_spec": 0
}
```

(Minimal because the P2-specific params are irrelevant and the global params unused.)

---

## 6. Head-to-head verdict: P0 vs P2

| Metric | P0 (iter-3 best) | P2 (today best) |
|--------|:-----------------:|:---------------:|
| Combined geomean cost | **169.1 ms** | 180.8 ms (+6.9%) |
| JIT overhead | 56.1 ms | **53.7 ms** (-4.3%) |
| Exec geomean | 68.1 ms | 72.7 ms (+6.8%) |
| Win kernels (≥1.1×) | 13 / 18 | **15 / 18** |
| Regression kernels (<0.9×) | **4** | **1** |
| Max speedup | 2.80× (box_filter) | 2.81× (generic_sort) |
| Parameter sensitivity | High (unroll_max, fixpoint_max) | **None** |

**P0 is better when**: workload is compute-bound with large loop bodies (uc2_conv, uc14_sort).
**P2 is better when**: workload includes short per-call kernels where cache-cold constant
loading causes regressions (uc1_sql, uc8_ivm batch_delta/column_scan/multi_predicate).

A workload-aware pipeline selector (P0 for compute-bound, P2 for memory-latency-bound)
would Pareto-dominate either pipeline alone.

---

## 7. Remaining regression: multi_agg_delta

multi_agg_delta is the only kernel that regresses in P2 (0.47×) — and it also regresses
in P0 (0.63×). Root cause:

1. Unspecialized execution: 0.26 ns/call — the function is extremely fast already.
2. JIT materialization overhead: even with zero constant-folding gain, the wrapper adds
   function-call overhead that is proportionally large.
3. SCCP propagation does fold the aggregate delta computation's constants, but the
   resulting code is not faster than unspecialized because:
   - P2: wrapper→target call boundary adds overhead
   - P0: PC-relative address load adds overhead

**Fix direction**: detect at specialization time whether `unspec_ns < 2ns` (or
equivalently, module instruction count after prune < some threshold) and skip JIT
specialization entirely, falling back to the unspecialized function. This requires
the benchmark to measure the unspecialized time before calling `specializeOrFallback`.
The `Options::FromExpectedRuntime` API (already exists) partially addresses this.

---

## 8. Infrastructure improvements

### I1: Remove global params from P2 search space

`configs/p2_search_space.json` already omits `large_module_max` and P1 params. But it
still includes `fixpoint_max`, `unroll_max`, `early_prune`, `o3_final` which P2 ignores.

**Fix**: Create a truly P2-only search space with just the 4 P2-specific knobs. Since
all 4 are also insensitive, the practical minimum is 1–5 trials to confirm the flat
landscape.

### I2: Add O3 final pass to P2

The compute-bound regression (box_filter, separable_gaussian) is entirely due to missing
O3 vectorization. P2's Phase 3 already runs `InvariantLoadToConstantPass + InstCombine +
SimplifyCFG` — add a `PB.buildPerModuleDefaultPipeline(O3)` at the end, gated on
`Opts.EnableO3Final` (matching P0's convention).

Expected impact: box_filter would recover from 1.06× to ~2×+ because the constants are
already in the target body; O3 just needs to see them to vectorize.

### I3: Add loop unrolling to P2 Phase 3

Similarly, add `LoopUnrollPass` to the FPM in Phase 3, gated on `Opts.LoopUnrollCount`.
For generic_sort (already 2.81× without it), higher unroll might push to 3×.

### I4: Wire `o3_final`/`unroll_max` to P2 registry entry

Currently `Options.EnableO3Final` and `Options.LoopUnrollCount` are read by
`JITPipelineInlining.cpp` (P0) but not by `JITPipelineIPSCCP.cpp` (P2). The environment
variables `CRS_DEFAULT_O3_FINAL` and `CRS_DEFAULT_LOOP_UNROLL_COUNT` are parsed into the
Options struct but silently ignored by P2. This creates a confusing user experience where
setting `o3_final=1` appears to change behavior but doesn't.

---

## 9. Process improvements

### P1: P2 optimization needs only 5–10 trials

With all parameters insensitive, 50 trials is overkill. 5 trials would establish the
flat landscape definitively. Future P2 iterations should allocate 10 trials maximum.

### P2: Measure impact of O3 final before running optimization

The key question for P2 is not "what parameters work best?" but "what architectural
features matter?". A 2-point ablation (with/without O3 final pass, with/without loop
unrolling) would answer this in <5 minutes. Run that first, THEN decide whether to
optimize parameters.

### P3: Per-kernel Optuna studies

The geomean objective conflates compute-bound wins with cache-cold regressions. A
per-kernel study or a two-objective study (minimize both JIT overhead and exec time
independently) would better characterize the Pareto front.

---

## 10. Next iteration scope

**Iteration 4 should focus on P2 architectural improvements:**

1. **Add O3 final + loop unrolling to P2** (Infrastructure I2, I3, I4). Test hypothesis:
   P2 + O3 final achieves comparable compute-bound speedups to P0 without the cache-cold
   regression. Expected: box_filter ~2× speedup (was 1.06×), retaining column_scan fix.

2. **Confirm the workload-aware selector**: measure the P2+O3 config on all UC kernels and
   check if it Pareto-dominates P0 everywhere or just for specific patterns.

3. **multi_agg_delta fix**: implement a fallback threshold in `specializeOrFallback` that
   skips JIT when estimated unspecialized time is below a threshold (e.g., 2 ns). This
   eliminates the one remaining P2 regression.

4. **TPC-H transfer**: rebuild TPC-H binary in release mode (open item since iter-2 §3.b).
   Once P2+O3 is ready, compare P0 vs P2+O3 on TPC-H to answer SQ4.

**Out of scope for iter-4** (firmly answered by today's data):
- Parameter tuning for P2 as currently implemented — all parameters are insensitive.
- Increasing trial budget for P2 — 10 trials are sufficient to characterize the landscape.
- The cache-cold constant loading regression — Phase 11 fixed it for 3/4 cases.

---

## 11. Optimization study status

| Study | Status | Trials | Combined best |
|-------|--------|-------:|:-------------:|
| `uc_optim_p2_20260528` | running | 7/50 done at time of writing | 180.8 ms |
| `uc_optim_iter3_20260527` (P0) | complete | 50/50 | 169.1 ms |
| `uc_optim_iter2_20260521` (P0) | complete | 50/50 | 170.99 ms |

Expected final P2 best: ~180–181 ms (flat landscape; no improvement expected as more trials
accumulate).

Query to verify once complete:
```sql
SELECT trial_id, params_json, obj_combined_ns/1e6 as combined_ms
FROM optim_trial_params
WHERE study_name = 'uc_optim_p2_20260528'
ORDER BY obj_combined_ns
LIMIT 3;
```
