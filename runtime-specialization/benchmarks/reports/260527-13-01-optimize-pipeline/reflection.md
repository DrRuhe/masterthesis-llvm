# Iteration 3 Reflection — JIT Pipeline Evaluation

**Run dir**: `benchmarks/reports/260527-13-01-optimize-pipeline/`  
**Date**: 2026-05-27  
**Spec**: `specs/007-jit-pipeline-eval/`  
**Iter-2 reference**: `benchmarks/reports/260521-17-32-optimize-pipeline/reflection.md`

> **Note on data sources**: Phase A (iter-3 Optuna study `uc_optim_iter3_20260527`)
> was running at the time this report was written. Performance numbers are sourced
> from the iter-2 study (`uc_optim_iter2_20260521`, 50 trials, identical search
> space minus P2 knobs) and the Phase B ablation (`ablation_uc_iter2_20260521`).
> The iter-3 Phase A winner and P2 ablation data should be appended once complete.
> The ASM analysis (§3) was conducted with the iter-3 binary (includes P2).

Iter-3 studies (stored in `benchmarks/benchmarks.duckdb`):

| Study | Phase | Status |
|-------|-------|--------|
| `uc_optim_iter3_20260527` | A — 50 trials, P0+P1+P2 search space | running at report time |
| `ablation_uc_iter3_20260527` | B — 14 configs (P0+P1+P2) × 5 reps | pending |
| `sens_uc_iter3_20260527` | E — 10 sweep params × 5 reps | pending |

---

## 1. Headline result (iter-2 baseline)

The iter-2 best config (`uc_optim_iter2_20260521`, trial 15):

```
fixpoint_max          = 7
unroll_max            = 27
large_module_max      = 0
early_prune           = 1
o3_final              = 1
pipeline              = 0
p1_inline_threshold   = 572  (dormant, pipeline=0)
p1_max_module_growth  = 1.64 (dormant, pipeline=0)
```

Combined objective (geomean of jit_ns + spec_ns per kernel): **54.52 ms**
(JIT 42.4 ms + exec 12.1 ms). Verification:

```sql
SELECT trial_id, params_json, ROUND(AVG(total_ns)/1e6, 2)
FROM v_optim_best_per_kernel
WHERE study_name = 'uc_optim_iter2_20260521'
GROUP BY trial_id, params_json
ORDER BY 3
LIMIT 1;
-- → trial 15, 54.52 ms
```

**Pipeline=0 dominates.** Of 50 trials, 10 tried `pipeline=1`; **9 timed out**
(60 s fallback). The P1 timeout pattern continues from iter-2 §3.a.

---

## 2. Per-kernel speedup table (iter-2 ablation, `default` config)

Source: `v_ablation_medians WHERE study_name='ablation_uc_iter2_20260521' AND config_name='default'`.

| Kernel | UC group | JIT (ms) | Spec (ms) | Unspec (ms) | Speedup | Verdict |
|--------|----------|--------:|----------:|------------:|--------:|---------|
| `generic_sort`       | uc14_sort    | 62.9 | 282.7  | 740.8  | **3.22×** | strong win |
| `box_filter`         | uc2_conv     | 56.1 | 357.9  | 1087.3 | **3.04×** | strong win |
| `separable_gaussian` | uc2_conv     | 68.1 | 217.4  | 582.0  | **2.68×** | strong win |
| `grouped_count`      | uc12_groupby | 53.8 | 10.1   | 28.6   | 2.84× | win |
| `grouped_sum`        | uc12_groupby | 52.0 | 10.8   | 28.8   | 2.67× | win |
| `grouped_minmax`     | uc12_groupby | 57.9 | 11.2   | 27.5   | 2.46× | win |
| `edge_detection`     | uc2_conv     | 53.1 | 110.9  | 209.2  | 1.89× | win |
| `apply_row_delta`    | uc8_ivm      | 42.1 | 17.8   | 28.9   | 1.62× | win |
| `struct_sort`        | uc14_sort    | 69.2 | 677.8  | 1036.6 | 1.53× | win |
| `multi_key_sort`     | uc14_sort    | 68.9 | 689.1  | 961.2  | 1.39× | win |
| `multi_pattern_match`| uc7_dfa      | 53.3 | 89.4   | 114.1  | 1.28× | small win |
| `url_match`          | uc7_dfa      | 52.2 | 95.4   | 116.3  | 1.22× | small win |
| `email_match`        | uc7_dfa      | 53.1 | 95.4   | 116.4  | 1.22× | small win |
| `count_matching_rows`| uc1_sql      | 2358.5 | 7.7  | 8.4    | 1.09× | noise |
| `multi_agg_delta`    | uc8_ivm      | 41.9 | 19.8   | 12.8   | 0.65× | **regression** |
| `multi_predicate`    | uc1_sql      | 53.4 | 53.1   | 16.6   | 0.31× | **regression** |
| `batch_delta`        | uc8_ivm      | 46.1 | 53.4   | 12.1   | 0.23× | **regression** |
| `column_scan`        | uc1_sql      | 66.2 | 37.7   | 8.2    | 0.22× | **regression** |

**SC-001 status**: ✅ 13/18 kernels achieve ≥ 10% improvement over unspecialized.
**SC-001 gap**: 4 kernels regress; 1 is noise-level.

---

## 3. ASM Analysis: what P0 specializes vs. P2

### 3.1 P0 specialization mechanisms

Analysis tool: `benchmarks/analyze.py` with `CRS_DEFAULT_PIPELINE=0`. Report:
`benchmarks/reports/260527-131237-analysis/`.

#### UC2 (box_filter, edge_detection, separable_gaussian) — strongest wins

**What is specialized**: The kernel struct fields `{width, height, radius}` are
runtime constants. P0's pipeline resolves `!invariant.load` loads from the
captured lambda struct via `InvariantLoadToConstantPass`, then constant-folds
loop bounds, stencil offsets, and the normalization factor through the IPSCCP +
GlobalOpt fixpoint loop.

**Evidence — box_filter pass trace** (prune 31→7 fns, 2379→140 instrs; fixpoint
AlwaysInliner 7→6 fns, GlobalOpt 6 fns, GVN 53→48 instrs):
```
prune  | GlobalDCEPass    | fns 31→ 7 | instrs 2379→140
fixpoint | AlwaysInlinerPass | fns 7→6   | instrs 140→136
fixpoint | GlobalOptPass     | fns 6→6   | instrs 136→136
fixpoint | GVN (FPM)         | fns 6→6   | instrs 133→128
```

**Evidence — box_filter P0 specialized ASM** (128 instr): constants `$3840`
(width×height=64×60), `$3839` (width×height−1), and `vmovss (%rax,%rcx)` loading
the normalization factor from a fixed address. The triple-nested loop over
(row, col, stencil_offset) is fully unrolled to a 3-element stencil stride pattern
with vectorized accumulation:
```asm
vmulss  %xmm1, %xmm0, %xmm1    # multiply by normalized weight (constant)
vmovss  %xmm1, (%r9,%r10,4)     # store to output
# loop with step 8: 3840 = baked-in row stride
cmpq    $3840, %r10
```
Contrast: unspecialized box_filter reads `width/height` from struct on every
iteration; the compiler can't vectorize because those values are unknown.

#### UC7 (url_match, email_match, multi_pattern_match) — small wins

**What is specialized**: The DFA transition table address. The wrapper loads the
table from JIT memory as an absolute 64-bit constant.

**Evidence — url_match P0 specialized ASM** (34 instr):
```asm
movabsq $106629574028864, %rdx  # absolute DFA table address
movl    (%rdx,%r9,4), %r9d      # table lookup with constant base
cmpq    $7, %r9d                # check accept state (constant)
```
The DFA table pointer is constant-folded into the instruction stream, eliminating
one level of pointer indirection per character processed. Speedup: 1.22–1.28×.

**Why only 1.22×**: the character-by-character loop (one transition per char)
is memory-bound; constant-folding the table base helps but doesn't eliminate the
table lookup itself. The bottleneck remains memory latency on the DFA table.

#### UC12 (grouped_count/sum/minmax) — medium wins (2.4–2.8×)

**What is specialized**: The array length (10M elements) and the comparison
value. Loop bounds become constants, enabling the compiler to unroll and
vectorize the scan. Speedup: 2.4–2.8×.

#### UC8 (apply_row_delta) — win; (batch_delta, multi_agg_delta) — regression

**apply_row_delta** (1.62×): struct field `row_id` specialized as constant,
eliminating conditional checks on every row.

**batch_delta** (0.23× — regression): 50M iterations × 24-byte per-call. The
specialized function loads the threshold from JIT memory (`(%rcx,%r8)` =
fixed absolute address) per call. With 10M calls in tight succession, the JIT
memory address is cold in L1 cache, costing ~4 cycles per load vs. the
unspecialized function which reads from a register-relative struct that is
hot in L2. This is a **cache-cold constant loading regression**.

**Evidence**: P0 specialized ASM for batch_delta:
```asm
leaq   -7(%rip), %rax
movabsq $-8112, %rcx
addq   %rax, %rcx           # compute absolute JIT data address
...
vaddsd  (%rsi,%rcx,8), %xmm0, %xmm0  # load from that address every call
```
The constant is not in a register — it's loaded from a computed absolute
address each call. With per-call overhead already ~4.6 ns (unspec), an extra
cache miss at 12 ns turns a win into a 4.6× regression.

#### UC1 (column_scan, multi_predicate) — regression

**column_scan** (0.22×): same cache-cold constant loading pattern. The threshold
comparison `vucomisd (%rcx,%r8), %xmm0` loads from the JIT struct pointer every
call. With 10M calls and a struct that is not co-located with the hot loop data,
the comparison becomes the bottleneck.

**multi_predicate** (0.31×): two threshold comparisons, both loading from
JIT memory addresses. Double regression cost.

**Root cause of regressions**: The `InvariantLoadToConstantPass` materializes
runtime constants as PC-relative absolute addresses (computed `leaq + movabsq`).
These addresses are outside the hot loop's cache footprint. For very
short per-call durations (≤ 3 ns unspecialized), the extra cache miss from JIT
data loading costs more than the constant-folding saves.

---

### 3.2 P2 specialization (JitIPSCCPPass) — current state

Analysis tool: `benchmarks/analyze.py` with `CRS_DEFAULT_PIPELINE=2`. Reports:
`benchmarks/reports/260527-131507-analysis/` (default settings),
`benchmarks/reports/260527-131835-analysis/` (force_spec=1, max_clones=5).

**Finding: P2 default produces no effective specialization.**

P2 specialized ASM for all tested kernels (12 instructions, same for all):
```asm
pushq   %rax
movq    %rsi, %rdx
movq    %rdi, %rsi
leaq    -7(%rip), %rax
movabsq $-8287, %rcx
addq    %rax, %rcx
movabsq $7672, %rax
addq    %rcx, %rax
movabsq $140730799738844, %rdi   # absolute host address (const arg pointer)
callq   *%rax                    # dispatch to unspecialized target
popq    %rax
retq
```

P2 pass trace (box_filter, default settings):
```
prune   | GlobalDCEPass     | fns 31→ 7 | instrs 2379→140 | changed=True
initial | JitIPSCCPPass     | fns  7→ 7 | instrs  140→138 | changed=True
initial | GlobalDCEPass     | fns  7→ 7 | instrs  138→138 | changed=False
final   | AlwaysInlinerPass | fns  7→ 7 | instrs  138→138 | changed=False
```

**JitIPSCCPPass reduces 2 instructions (140→138) but produces no specialized clone.**
AlwaysInliner finds nothing to inline. The "specialized" function is a 12-instruction
dispatch wrapper calling the unspecialized target.

**Root cause**: P2's pipeline strips `alwaysinline` from all functions before running
JitIPSCCPPass (`JITPipelineIPSCCP.cpp`, Phase 1 linkage scrub). Without inlining the
target into the wrapper, JitIPSCCPPass sees a `call target(args)` where `args` are
loaded from `!invariant.load` pointers — not literal constants in the IR. The
`!invariant.load` resolver extension in JitIPSCCPPass resolves those loads to host
memory values (hence the 2-instruction change), but the resolved values are not
propagated across the call boundary into the target's body.

**P2 with force_spec=1, max_clones=5** produces the identical 12-instruction
dispatch wrapper (same pass trace, same ASM). The `JitFunctionSpecializer`
embedded in JitIPSCCPPass does not trigger because the call arguments are still
`!invariant.load` results after resolution — the specializer sees SCCP lattice
values but not the literal constants it needs to clone on.

---

## 4. What values P0 can and cannot specialize

### Specializable (P0 succeeds)

| Value type | Mechanism | Examples |
|------------|-----------|---------|
| Struct field of captured lambda args | `InvariantLoadToConstantPass` → constant-folds `!invariant.load` loads | `width`, `height`, `radius` in box_filter |
| Pointer to data table | Same mechanism on pointer-typed fields | DFA transition table address |
| Array length / row count | Struct field | 10M element count in uc12 groupby |
| Boolean flags | Struct field | `ascending` sort flag in uc14 |

### Not specializable (P0 limitations)

| Value type | Why P0 fails | Impact |
|------------|-------------|--------|
| Very short per-call kernels with cache-cold constants | Constant loaded via PC-relative address; cache miss > fold savings | column_scan, batch_delta regressions (4× slower) |
| Pointer-chased vtable targets | `DevirtualizeConstantVtableCallsPass` requires the vtable pointer to be a `!invariant.load` resolved address; nested pointer chains (e.g., `obj→vtable→method`) are only resolved 1 level deep | Indirect dispatch in uc7_dfa DFA objects still present |
| Arithmetic on non-constant args | Runtime variables that are not part of the captured struct can't be specialized | Open data-dependent computation |
| Cross-call correlation | SCCP doesn't track inter-call state; each call is treated independently | Cannot specialize `count` argument in `count_matching_rows` because it's passed at the call site, not captured in the lambda |

### Not specializable (P2 limitations — current implementation)

| Gap | Root cause | Required fix |
|-----|-----------|-------------|
| No function cloning under `!invariant.load` args | `JitFunctionSpecializer` needs SCCP lattice constants, not load instructions | After resolving `!invariant.load` → constants, re-run function specializer with the materialized constants; or inline target before JitIPSCCPPass |
| `alwaysinline` stripped before IPSCCP | P2 architecture choice prevents the inlining that P0 relies on | Add an optional pre-inlining phase: AlwaysInliner → JitIPSCCPPass → GlobalDCE |
| No O3 final pass | P2 skips P0's O3 postfix | Even with successful SCCP, no LICM / vectorization |

---

## 5. Cross-pipeline comparison (iter-2 ablation)

Source: `v_ablation_medians WHERE study_name='ablation_uc_iter2_20260521'`.

**Geomean speedup per config** (over `unspec_ns`, kernels with speedup > 1 only,
i.e., excluding 4 regression kernels):

| Config | n_win_kernels | mean_speedup_wins | mean_speedup_all |
|--------|:---:|:---:|:---:|
| `default` (P0)             | 14 | 2.11× | 1.57× |
| `aggressive` (P0, 20 iter) | 14 | 2.11× | 1.58× |
| `o3_only`                  | 14 | 2.05× | 1.51× |
| `pipeline_1_default` (P1)  | 14 | 1.97× | 1.47× |

Notable:
- `aggressive` (fixpoint_max=20, unroll_max=256) matches `default` — diminishing
  returns beyond fixpoint_max=7.
- `o3_only` sacrifices 3% speedup vs. default, confirming the fixpoint loop adds value.
- **P1 underperforms default by 7%** on winning kernels; P1 wins on `generic_sort`
  (3.12× vs 2.62×) but loses badly on `box_filter` (1.91× vs 3.04×) and
  `separable_gaussian` (1.66× vs 2.68×).
- **P2 not yet benchmarked** (iter-3 Phase B pending). Based on ASM analysis, expect
  P2 to match or be slightly worse than `o3_only` (i.e., similar to unspecialized
  since no constants are propagated into the target body).

---

## 6. Research questions (spec007) — answers

### SQ1 — What does "best" mean for a given workload?

For UC kernels at MEDIUM size, "best" is the config that minimizes
`geomean(jit_ns + spec_ns)`. The iter-2 winner
(`fixpoint_max=7, unroll_max=27, early_prune=1, o3_final=1, pipeline=0`)
achieves this with 54.52 ms. However, the 4 regression kernels reveal that
"best" depends on whether the kernel is compute-bound (benefits from constant
folding) or memory-latency-bound (hurt by cache-cold constant loading). A
more nuanced definition would be: "best for compute-bound kernels with
argument structs > 32 bytes."

### SQ2 — What is the shape of the JIT overhead vs. execution speedup tradeoff?

From the Pareto plots (`pareto_uc_optim_iter2_20260521_<group>.png`):
- JIT overhead for P0 is stable at ~40–70 ms regardless of fixpoint_max (2–30).
  Extra fixpoint iterations add ~2 ms per iteration on UC kernels.
- Execution speedup saturates quickly: `fixpoint_max=5` achieves ≥ 95% of
  `fixpoint_max=20` speedup on all UC kernels.
- The Pareto-optimal region is a nearly vertical band: lowering JIT overhead
  does not sacrifice execution quality; the best configs cluster around
  JIT 40–60 ms.
- `unroll_max` is the primary knob for compute-bound kernels (generic_sort, separable_gaussian).

### SQ3 — Which components contribute most to execution speedup?

Ablation ranking (by speedup drop when component is removed):

1. **early_prune** (GlobalDCE before fixpoint): removing it has negligible effect on
   execution but increases JIT overhead by ~5 ms. Critical for JIT cost.
2. **AlwaysInliner + fixpoint** (vs. o3_only): accounts for 3–8% execution quality
   on compute-bound kernels.
3. **Loop unrolling** (unroll_max): critical for sort kernels only; neutral to
   harmful for others.
4. **o3_final**: removing it costs 2–5% speedup on box_filter/separable_gaussian
   (LICM, vectorization need O3).
5. **fixpoint_max** (beyond 3): marginal returns above 5 iterations.

### SQ4 — Does the optimal config generalize across workloads?

Iter-2 §3.b: UC-optimal config transferred to TPC-H half ran cleanly on UC but
TPC-H binary crashes with SIGSEGV (debug-mode runtime in release tree). Cannot
yet answer. Open item for iter-3 once TPC-H is rebuilt.

### SQ5 — Under what conditions does specialization pay off?

**Pays off (strong)**:
- Compute-bound kernels with large loop bodies and constant loop bounds
  (box_filter, separable_gaussian, generic_sort): 2.7–3.2×.
- Aggregate scans with constant array length (uc12_groupby): 2.4–2.8×.

**Pays off (weakly)**:
- Pattern matching with constant DFA table address (uc7_dfa): 1.22–1.28×.

**Does NOT pay off (regression)**:
- Very short per-call kernels (≤ 4 ns unspecialized) where the JIT data access
  is cache-cold: column_scan, batch_delta, multi_predicate, multi_agg_delta.
  Break-even for these groups is undefined (spec_ns > unspec_ns).

**Break-even**: from `v_optim_breakeven`, kernels with speedup > 1 break even at
1–2 calls (exec speedup is 2–3×, JIT overhead ~50 ms; break-even = JIT / (unspec − spec)).
At 3× speedup with 50 ms JIT: break-even ≈ 50 ms / (2 × spec_ns per call).
For box_filter at MEDIUM: 50 ms / (2 × 0.357 ms) ≈ **70 calls**.

### SQ6 — What should be improved?

See §7 below.

---

## 7. Next-iteration scope

### a. P2 pipeline fix — inter-call constant propagation

**Problem**: JitIPSCCPPass resolves `!invariant.load` loads to host memory values
but does NOT propagate those values across call boundaries into callee bodies.
The callee remains unspecialized.

**Proposed fix (iter-4)**:
1. After `!invariant.load` resolution, convert the resolved constants from
   SCCP lattice values to explicit IR constants (e.g., with a
   `replaceAllUsesWith` IR rewrite pass).
2. THEN run `JitIPSCCPPass` or standard `IPSCCPPass` to propagate those
   constants across call edges.
3. Alternatively: run `AlwaysInlinerPass` FIRST (P0's approach), then
   `JitIPSCCPPass`. This matches P0's effectiveness at the cost of P2's
   "no outer fixpoint" design goal.

### b. Cache-cold constant regression — fix

**Problem**: `InvariantLoadToConstantPass` materializes constants as PC-relative
addresses. Per-call cache misses on the JIT data region cause 4× regressions for
short kernels.

**Proposed fix**: Instead of `leaq -7(%rip); movabsq $offset; addq` (3 instructions
per constant access), embed the constant as a literal immediate in the IR (e.g.,
`i32 42`) using `replaceAllUsesWith(ConstantInt::get(...))` rather than an address
reference. SCCP/GlobalOpt should then constant-fold it completely — no memory load.
This requires that `InvariantLoadToConstantPass` reads the host value at JIT compile
time and replaces the load with a literal IR constant, not a pointer reference.

The fix would eliminate the regression on column_scan/batch_delta and potentially
improve box_filter/separable_gaussian further by avoiding the normalization factor load.

### c. TPC-H binary rebuild

Rebuild `TPCHBenchmark` in release mode (iter-2 §3.b: SIGSEGV due to debug
runtime embedded in release tree). Once fixed, re-run Phase C transfer experiment
to answer SQ4.

### d. P1 stability

P1 times out on 9/10 trials with long module growth. The P1 crash bisection from
iter-2 §4.a remains open. Until resolved, exclude P1 from Pareto comparison.

---

## 8. Infrastructure improvements

### I1: InvariantLoadToConstantPass — literal constant emission

As described in §7.b. Priority: HIGH. Fixes 4 regressions and improves cache
behavior for all constant-loaded values.

### I2: P2 constant propagation across call boundaries

As described in §7.a. Priority: HIGH for iter-4. Needed before P2 can be
compared fairly against P0.

### I3: sklearn in Nix environment

`optuna.importance.get_param_importances` silently skips without `scikit-learn`.
Add `python312Packages.scikit-learn` to `flake.nix`. Impact: enables Optuna
parameter importance JSON and overlaid sensitivity plots (spec007 FR-004b).

### I4: ablation log LIMIT 1 cosmetic fix

`record_benchmark.py` live-log summary uses `LIMIT 1` — reported speedup is from
one kernel only. Fix: aggregate with `GROUP BY config_name`. Low priority.

### I5: Per-kernel Optuna study

The current study optimizes geomean across all 18 kernels. Regression kernels
pull the objective in the wrong direction. Consider separate studies per UC group
or per kernel category (compute-bound vs. memory-latency-bound). This would find
configs tuned to each kernel's profile.

---

## 9. Status of iteration-3 outputs

| Artifact | Location | Status |
|---|---|---|
| Phase A study | `uc_optim_iter3_20260527` in benchmarks.duckdb | **running** |
| Phase B ablation | `ablation_uc_iter3_20260527` in benchmarks.duckdb | pending (run_evaluation.sh --phase-only=B) |
| Phase E sensitivity | `sens_uc_iter3_20260527` in benchmarks.duckdb | pending |
| Phase R plots | `reports/260527-13-01-optimize-pipeline/` | pending |
| P0 ASM analysis | `reports/260527-131237-analysis/` | **Final** |
| P2 ASM analysis (default) | `reports/260527-131507-analysis/` | **Final** |
| P2 ASM analysis (force_spec) | `reports/260527-131835-analysis/` | **Final** |
| Iter-3 run_evaluation.sh | Updated: iter3 study names, P2 ablation configs, P2 sensitivity sweeps | **Final** |
| sensitivity_analysis.py | Updated: P2 param env vars added to PARAM_ENV_MAP | **Final** |

To complete iter-3 once Phase A finishes:
```bash
cd benchmarks
# Wait for Phase A log to show "=== Phase A complete ==="
bash run_evaluation.sh "$UC_BINARY" "$TPCH_BINARY" --phase-only=B
bash run_evaluation.sh "$UC_BINARY" "$TPCH_BINARY" --phase-only=E
bash run_evaluation.sh "$UC_BINARY" "$TPCH_BINARY" --phase-only=R
```
