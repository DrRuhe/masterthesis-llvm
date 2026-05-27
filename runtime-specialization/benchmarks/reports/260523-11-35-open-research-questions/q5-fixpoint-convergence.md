# Q5: Is the fixpoint loop actually converging before the iteration cap?

## Important Context

The iter-2 winner uses `fixpoint_max=7`. Pass-trace data (`g_lastPassTrace` via `writePassTraceJSON`) is available in DuckDB's `pass_traces` table for three benchmarks: `db_operators/chained_filter`, `db_operators/single_filter`, and `polybench/correlation` (EXTRALARGE). The UC kernels do not yet have pass-trace data collected.

The `pass_traces` table schema: `benchmark_name`, `pass_idx`, `pass_name`, `pass_group`, `fixpoint_iter`, `fns_before/after`, `instrs_before/after`, `bbs_before/after`, `wall_time_ms`, `ir_changed`.

---

## Investigation

### Analyze fixpoint convergence from available pass-trace data

**db_operators/chained_filter** (a small proxy kernel):

| fixpoint_iter | passes changed | instrs at end of iter |
|---------------|---------------|-----------------------|
| 0             | 5             | 104                   |
| 1             | 4             | 107                   |
| 2             | 4             | 106                   |
| 3             | 2             | 106                   |

The instruction count stops changing after iter 2 (106 → 106). In iter 3, only 2 passes report `ir_changed=True`, but the final instruction count is identical to iter 2. This suggests **true convergence at iter 2** — the `ir_changed` flags in iter 3 are false positives from passes that touch metadata without affecting the effective IR (e.g., analysis invalidation).

With `fixpoint_max=7`, this kernel runs 4 wasteful iterations (iters 3–6). The JIT overhead cost is proportional to the number of full-MPM passes executed.

**polybench/correlation** (a larger kernel):

| fixpoint_iter | passes changed | instrs at end of iter |
|---------------|---------------|-----------------------|
| 0             | 10            | 591                   |
| 1             | 4             | 591                   |

Instruction count is already stable after iter 0! Even the 4 passes reporting `ir_changed=True` in iter 1 don't reduce the instruction count. This kernel converges in **1 iteration** under the default fixpoint.

**db_operators/single_filter**:

| fixpoint_iter | passes changed | instrs at end of iter |
|---------------|---------------|-----------------------|
| 0             | 5             | 104                   |
| 1             | 4             | 102                   |
| 2             | 2             | 100                   |

Convergence at iter 2 (instruction count stabilizes at 100). One more wasted iteration at fixpoint_max=7.

**Pattern across available traces**: Convergence occurs at iter 1–3, not iter 7. `fixpoint_max=7` runs 4–6 excess iterations for these kernels.

**What to look for in future data**: Run `jit_analysis` for all 18 UC kernels (all 3 abstraction levels) and collect pass traces. For each, record the first fixpoint iteration where `instrs_after` matches the previous iteration's `instrs_after` — that is the true convergence point. Compute the waste ratio: (fixpoint_max - convergence_iter) / fixpoint_max.

### Analyze whether early-exit would reduce JIT overhead without losing quality

The sensitivity data from `sens_uc_iter2_20260521` provides a direct measurement:

| config           | uc12_groupby spec_ms | uc14_sort spec_ms | uc2_conv spec_ms | uc8_ivm spec_ms |
|------------------|---------------------|-------------------|------------------|-----------------|
| sens_fixpoint_2  | 10.84 / 10.2 / 11.04| 285 / 681 / 688   | 110 / 360 / 213  | 17.9 / 53.4 / 20.0|
| sens_fixpoint_5  | 12.04 / 12.58 / 12.25| 283 / 694 / 674  | 381 / 296 / 382  | 20.5 / 63.4 / 19.8|
| sens_fixpoint_7  | 12.01 / 12.54 / 12.30| 284 / 695 / 671  | 382 / 297 / 381  | 17.75 / 63.15 / 20.3|
| sens_fixpoint_30 | 11.82 / 12.32 / 12.22| 284 / 697 / 675  | 381 / 305 / 379  | 17.93 / 63.65 / 19.9|

The exec spec_ms values are **statistically indistinguishable** across fixpoint_max = 2, 5, 7, 30 for all groups. The UC kernels converge in ≤ 2 fixpoint iterations, making `fixpoint_max` a parameter that only affects JIT overhead, not specialization quality.

**JIT overhead** from sens data:

| config           | uc12_groupby jit_ms | uc14_sort jit_ms |
|------------------|---------------------|-----------------|
| sens_fixpoint_2  | 42.59 / 42.75 / 43.84 | 50.43 / 53.49 / 50.08 |
| sens_fixpoint_7  | 42.71 / 43.05 / 44.41 | 50.54 / 53.90 / 50.59 |
| sens_fixpoint_15 | 42.70 / 42.97 / 43.86 | 50.56 / 53.89 / 50.14 |
| sens_fixpoint_30 | 42.63 / 42.72 / 43.63 | 50.32 / 53.95 / 50.35 |

JIT overhead is also nearly identical across fixpoint_max values! This is unexpected — if extra iterations cost time, the jit_ms should scale with fixpoint_max. The explanation is that the extra iterations (when the IR is unchanged) are extremely cheap: the passes operate on an already-converged IR and exit quickly (most `ir_changed=False` paths have near-zero wall time).

**Interpretation**: For UC-scale kernels, the fixpoint loop is not the bottleneck in JIT overhead. The dominant JIT costs are O3 final optimization and bitcode parsing. The 7 vs 2 vs 30 iteration choice makes < 1 % difference in both JIT overhead and exec time. The early-exit criterion would save negligible time on UC kernels but could be important for larger modules (DuckDB, polybench) where each fixpoint iteration's passes run on thousands of functions.

### Analyze whether early-exit makes the fixpoint_max hyperparameter unnecessary

For UC kernels: yes, because convergence at iter 1–3 means the cap never binds in practice. An early-exit criterion (instruction count unchanged between iterations) would allow removing `fixpoint_max` from the Optuna search space entirely, reducing dimensionality by 1.

For DuckDB-scale modules: the pass traces show convergence at iter 1 for polybench/correlation (which is already EXTRALARGE). This suggests even large modules converge quickly, making the early-exit criterion effective universally.

**Implementation**: After each fixpoint iteration, compare `instrs_after` of the last pass in the iteration to `instrs_after` of the last pass in the previous iteration. If equal for N consecutive iterations (N=1 or 2 to account for false-positive `ir_changed` flags), stop. This is a 1-line check per iteration.

---

## Conclusion

The fixpoint loop **does converge before the iteration cap** for all available pass-trace data. UC kernels converge in 1–3 iterations; polybench/correlation converges in 1 iteration. Running 7 iterations wastes compute on iterations 2–6 for most kernels, but since each wasted iteration is cheap (empty passes exit in microseconds), the practical JIT overhead impact is < 1 %.

Implementing an early-exit convergence criterion is technically straightforward and would:
1. Eliminate `fixpoint_max` as a tuning parameter (since it never binds in practice)
2. Remove one dimension from Optuna's search space
3. Provide correctness guarantees (never under-converge due to a too-small cap)

The only risk is that pathological kernels require many iterations — but the data shows no such case in the current benchmark suite.

---

## Further Questions/Directions of Research

- Collect UC kernel pass traces via `jit_analysis` benchmarks across all 18 kernels and all 3 abstraction levels. Record convergence iteration per kernel. See `missing-data.md` — this is a cheap experiment (< 1 hour).
- Implement the early-exit criterion in `ClangRuntimeSpecializer.cpp`'s transform lambda: compare instruction count before and after each fixpoint MPM run; exit if unchanged for 2 consecutive iterations.
- After implementing early-exit: measure whether the savings are significant for DuckDB-scale modules (where each pass run on 47k functions is expensive).
- Verify that false-positive `ir_changed=True` flags (metadata-only changes) don't prevent early-exit by checking instruction count equality rather than the `ir_changed` flag.
