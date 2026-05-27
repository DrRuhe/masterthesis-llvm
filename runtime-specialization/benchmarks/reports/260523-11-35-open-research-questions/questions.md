# Open Research Questions — JIT Pipeline Evaluation

**Date**: 2026-05-23  
**Input**: `benchmarks/reports/260521-17-32-optimize-pipeline/reflection.md` (iter-2),
`specs/007-jit-pipeline-eval/spec.md`

These questions are grounded in the empirical gaps and anomalies that remain after iter-2.
Questions marked **⚠ pipeline-sensitive** are ones whose current answer may change if the
JIT pipeline is improved further.

---

## 1. Why are some use cases slower when specialized?

`uc2_conv` and `uc8_ivm` were **37–46 % slower** under Pipeline-1 (budget-aware inlining)
than under Pipeline-0. Even under P0 the "exec speedup" for several UC groups hovers near
1.0 or slightly below.

Sub-questions:
- Is the slowdown caused by over-specialization (e.g., code-size explosion from aggressive
  unrolling causing instruction-cache pressure), or by under-specialization (constants not
  propagated deep enough to eliminate the hot branch)?
- Does the slowdown correlate with module size, loop trip count, or the number of indirect
  calls in the unspecialized IR?
- Could a post-JIT profiling step (e.g., perf event counts) distinguish the two causes?

**⚠ pipeline-sensitive**: A better fixpoint convergence heuristic or more conservative
unroll limit for convolution-style kernels could eliminate the regression. The current
answer ("P1 hurts conv") may be a calibration artifact rather than a fundamental property.

---

## 2. Why does Pipeline-1 (budget-aware inlining) help `uc14_sort` but hurt others?

P1 gave `uc14_sort` a 17 % speedup (best ablation result), while `uc2_conv` and `uc8_ivm`
got 37–46 % slower under the same pipeline variant.

Sub-questions:
- What structural property of the sort kernel makes aggressive inlining of comparators
  profitable, while convolution loops do not benefit?
- Can this be predicted from the JIT IR before compilation (e.g., call-graph depth, ratio
  of indirect calls to total instructions)?
- Is there a minimal P1 configuration (lower `p1_inline_threshold`) that captures the sort
  win without regressing convolution?
- Can the pipeline selector be made automatic — choosing P0 vs. P1 based on IR properties
  rather than requiring a per-workload configuration?

**⚠ pipeline-sensitive**: If P1's stability issues (RQ below) are fixed, a broader
parameter sweep might reveal a P1 variant that wins on both sort and convolution.

---

## 3. What is the root cause of Pipeline-1's SIGSEGV under specific parameter cross-products?

The OAT sweep (`sens_pipeline_1`) combined `pipeline=1` with the P0-tuned baseline
(`unroll_max=27`, `p1_max_module_growth=1.64`) and produced **5/5 SIGSEGV** — 100 %
crash rate. The same P1 config with default parameters (Phase B `pipeline_1_default`)
ran stably.

Sub-questions:
- Is the crash triggered by `unroll_max=27` alone, by `p1_max_module_growth=1.64` alone,
  or only by their interaction?
- Is it a memory-safety issue in `JITPipelineFuncSpec.cpp` (e.g., dangling pointer after
  module growth) or an LLVM assertion that fires in a specific IR state?
- After the bisect proposed in the iter-2 reflection (2⁵ = 32 configs), can P1 be made
  stable across the full parameter space?

---

## 4. Does the UC-optimal pipeline configuration transfer to TPC-H?

Phase C (cross-workload transfer) was **incomplete**: TPC-H crashed with SIGSEGV due to a
debug-mode runtime embedded in the release binary. Only the UC half ran (UC transfer showed
no advantage over Default within noise).

Sub-questions:
- Once the TPC-H binary is rebuilt as a release binary, does the UC-optimal config
  (`fixpoint_max=7, unroll_max=27, early_prune=1, o3_final=1`) improve TPC-H performance,
  or does it degrade it relative to `Options::Default()`?
- Is the degradation > 20 % (suggesting workload-specific tuning is necessary) or ≤ 5 %
  (suggesting a universal config is viable)?
- Can a statistically significant conclusion be drawn via Wilcoxon rank-sum at α = 0.05
  across TPC-H query groups?

This question directly answers spec SQ4 ("Does the optimal configuration generalise across
workloads?") and is currently **unanswered**.

---

## 5. Is the fixpoint loop actually converging before the iteration cap?

The iter-2 winner uses `fixpoint_max=7`. It is unknown whether the fixpoint loop converges
earlier (e.g., IR stops changing at iteration 3–4) or keeps refining through all 7 passes.

Sub-questions:
- What is the distribution of actual fixpoint convergence iterations across UC groups?
  (Pass-trace data from `g_lastPassTrace` / `writePassTraceJSON` could answer this.)
- Is `fixpoint_max=7` always needed, or only for a subset of kernels? Could an early-exit
  criterion (e.g., instruction count unchanged between iterations) reduce JIT overhead
  without losing specialization quality?
- Would a dynamic convergence check replace the `fixpoint_max` hyperparameter entirely,
  making one of the Optuna search dimensions unnecessary?

**⚠ pipeline-sensitive**: Adding a convergence-detection early-exit could improve both JIT
overhead and the optimizer's search space — reducing the number of parameters that need
tuning.

---

## 6. How much of the speedup comes from constant folding vs. loop unrolling vs. devirtualization?

The ablation study (Phase B) tested named configurations (`no_unroll`, `no_prune`,
`O3Only`, etc.) but did not isolate the contribution of individual transformation types
within the fixpoint loop.

Sub-questions:
- For `uc14_sort`, how much speedup is attributable to comparator inlining vs. IPSCCP
  constant folding of comparison keys vs. loop unrolling?
- For `uc2_conv`, does the regression come from loop unrolling causing icache pressure,
  from IPSCCP propagating an incorrect invariant, or from something else?
- Can the `PassRecord` pass-trace infrastructure quantify per-pass instruction-count
  reductions, and does that reduction correlate with execution speedup?

---

## 7. Does the optimal pipeline change with workload size (SMALL / MEDIUM / LARGE)?

Iter-1 validated that **relative speedup** is stable across MEDIUM/LARGE/EXTRALARGE.
But it did not validate that the **optimal pipeline configuration** is stable across sizes.

Sub-questions:
- Does `unroll_max=27` remain the optimizer's choice at EXTRALARGE sizes, where loop trip
  counts are larger and unrolling may be more profitable or more harmful?
- Does `early_prune=1` become more important at LARGE sizes (larger modules → more pruning
  benefit) or less important (pruning cost amortised)?
- Can the size-scaling study be extended to include a mini-Optuna run per size tier, to
  check whether optimal configs form a consistent family?

---

## 8. How does parameter sensitivity change as the baseline config improves?

Iter-1 classified `unroll_max` as "low-importance," but iter-2 found it critical for two
kernels (−14.5 % for `generic_sort`, −6.1 % for `separable_gaussian`). This revision
suggests sensitivity rankings are **not stable** across baseline configs.

Sub-questions:
- Is the instability because iter-1's OAT was anchored at a weaker baseline, making
  `unroll_max` look unimportant relative to larger gains elsewhere?
- As the pipeline approaches a local optimum, do secondary parameters (`unroll_max`,
  `p1_inline_threshold`) gain relative importance while primary parameters
  (`fixpoint_max`, `early_prune`) saturate?
- Would second-order sensitivity analysis (interaction terms between `unroll_max` and
  `fixpoint_max`) reveal that these parameters are jointly important even if marginally
  they appear weak?

---

## 9. What is the practical break-even call count for each use case, and how does it vary with pipeline choice?

Spec SQ5 asks: "Under what conditions does specialization pay off?" Break-even data was
partially collected but not thoroughly analyzed across pipeline variants.

Sub-questions:
- For the UC groups where `spec_ns ≥ unspec_ns` (no exec speedup), is the break-even
  count infinite (specialization never pays off) or merely very high (pays off only for
  long-running batch jobs)?
- Does the break-even count change significantly between P0 and P1 variants for `uc14_sort`?
  (P1 reduces `spec_ns` by 17 % but may increase `jit_ns` — what is the net effect on
  break-even?)
- Can a simple rule-of-thumb be derived: "specialize if expected call count > N and
  argument distribution entropy < H"?

---

## 10. Does the combined-cost objective (jit_ns + spec_ns) correctly model the thesis use case?

Spec W6 flags this as a methodology weakness. The objective implicitly assumes one call
amortises the JIT overhead. For workloads with many calls this underweights execution
speedup; for one-shot workloads it is appropriate.

Sub-questions:
- What is the actual expected call-count distribution for the thesis target use cases?
  (E.g., a query engine might specialize once and call millions of times; an event-driven
  system might call tens of times.)
- Would a two-objective Optuna study (minimise JIT overhead, minimise exec time
  independently) produce a different Pareto front than the current scalar objective?
- Is there a call-count threshold N* above which the `Aggressive` config consistently
  Pareto-dominates `Default`, even if its combined cost is higher?

---

## Summary Table

| # | Question | Spec SQ | Current answer | Pipeline-sensitive? |
|---|----------|---------|----------------|---------------------|
| 1 | Why are some UCs slower when specialized? | SQ2, SQ5 | Unknown (P1 causes regression on conv/ivm) | Yes |
| 2 | Why does P1 help sort but hurt others? | SQ3, SQ2 | Structural (inlining comparators), unconfirmed | Yes |
| 3 | Root cause of P1 SIGSEGV? | SQ6 | Unknown — parameter interaction | Yes |
| 4 | Does UC-optimal transfer to TPC-H? | SQ4 | **Unanswered** (TPC-H binary broken) | No |
| 5 | Does the fixpoint converge early? | SQ3, SQ5 | Unknown — no per-iteration trace analysis | Yes |
| 6 | What drives speedup: folding vs. unroll vs. devirt? | SQ3 | Unknown — ablation granularity too coarse | Yes |
| 7 | Does optimal config change with workload size? | SQ1, SQ4 | Relative speedup stable; config stability untested | Partial |
| 8 | Are sensitivity rankings stable across baseline configs? | SQ2, SQ6 | No — iter-1 and iter-2 disagree on `unroll_max` | Yes |
| 9 | Break-even call count per UC and per pipeline? | SQ5 | Partially answered, not per-pipeline | Yes |
| 10 | Does the combined-cost objective model the use case correctly? | SQ1, SQ6 | Unknown — use-case call counts not characterised | Partial |
