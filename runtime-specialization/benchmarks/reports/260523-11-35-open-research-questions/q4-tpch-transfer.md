# Q4: Does the UC-optimal pipeline configuration transfer to TPC-H?

## Important Context

Phase C of iter-2 (`transfer_uc_iter2_20260521` and `transfer_tpch_iter2_20260521`) attempted to measure whether the UC-optimal config (`fixpoint_max=7, unroll_max=27, early_prune=1, o3_final=1, pipeline=0`) outperforms `Options::Default()` on TPC-H queries. The UC half ran cleanly (15 OK runs) but showed no significant advantage over Default (1.26× on UC MEDIUM, comparable to Default's 1.81× within noise — data too thin). The TPC-H half produced **0 rows** due to a SIGSEGV: the `TPCHBenchmark` binary embedded a debug-mode runtime despite being compiled in the release tree.

The root cause: the embedded library (registered via `clang_runtime_specializer_register_blob`) was compiled with `CRS_BUILD_TYPE=debug` while the benchmark runner expected release semantics. The debug runtime asserts trigger on the first JIT call.

Until the TPC-H binary is rebuilt as pure release, this question is **structurally unanswerable**.

---

## Investigation

### Analyze the UC transfer result (available data)

The UC half of Phase C showed:
- UC MEDIUM, `uc_workload_optimal` config: 1.26× aggregate speedup
- UC MEDIUM, `default` config: ~1.81× aggregate speedup (from Phase A reference)

This is a **regression** of the optimal config when transferred from Phase A's per-kernel winners to the combined workload. However, the Phase C dataset only has 15 runs (3 configs × 5 reps), which is insufficient for statistical significance. The 1.26× vs 1.81× gap could be noise from kernel selection (Phase C likely uses a different benchmark filter than Phase A's 18-kernel full sweep).

**What to look for**: Query `transfer_uc_iter2_20260521` in DuckDB for per-kernel speedups. Compare against Phase A's per-kernel best (from `v_optim_best_per_kernel WHERE study_name='uc_optim_iter2_20260521'`) to identify if the transfer loss is uniform or concentrated in specific kernels.

### Analyze what a successful TPC-H transfer experiment would look like

TPC-H differs structurally from UC benchmarks:
- Queries span multiple operators (joins, aggregations, predicates)
- The specialization target is a single query-specific kernel, but the surrounding query execution is complex
- TPC-H has 22 queries with varying characteristics (Q1 is aggregation-heavy, Q4 is join-heavy, Q6 is filter-heavy)

A clean TPC-H transfer experiment needs:
1. Rebuild `TPCHBenchmark` with a release-mode runtime blob (verify with `strings TPCHBenchmark | grep CRS_BUILD_TYPE`)
2. Run 3 configs × 22 queries × 5 reps = 330 measurements
3. Statistical test: Wilcoxon rank-sum across all query groups at α=0.05

**Interpretation criteria**:
- If uc_optimal > default by > 5 % on ≥ 14/22 queries → config generalizes
- If uc_optimal < default on > 8/22 queries → workload-specific tuning required
- If differences are ≤ 5 % on all queries → default is already near-optimal for TPC-H

### Analyze the structural reason config transfer might fail

The UC kernels are standalone computation kernels (sort, filter, group-by) where the JIT module is small and the hot path is well-contained. TPC-H JIT modules are likely larger (more support functions survive GlobalDCE when the query runtime links against the TPC-H libraries).

`early_prune=1` removes many functions upfront. For UC kernels with small modules, this is cheap and effective. For TPC-H kernels with larger modules, the prune step costs more JIT time but the benefit is the same (or larger). So `early_prune=1` should still be beneficial.

`unroll_max=27` was the key differentiator in iter-2. TPC-H predicates and aggregations may not have inner loops that benefit from 27× unrolling — the typical TPC-H kernel is a single scan loop over millions of rows with a cheap inner operation. 27× unrolling of a single-pass scan may increase code size without reducing branch overhead.

`fixpoint_max=7` is neutral: as shown in Q5, the fixpoint converges in 2–4 iterations for most kernels, so the extra iterations are wasted but not harmful.

---

## Conclusion

The cross-workload transfer question is the **only** iter-2 spec SQ that is entirely unanswered due to infrastructure failure. The fix is mechanical: rebuild `TPCHBenchmark` linking the release runtime. Once rebuilt, the experiment is straightforward to execute.

The early evidence from the UC transfer (no advantage of optimal vs default) is a weak signal — it could indicate the config is genuinely worse on a multi-workload mix, or it could be insufficient data. TPC-H is the more informative test because it is architecturally different from UC.

The prior expectation based on structural analysis is: `early_prune=1` transfers positively, `o3_final=1` is neutral, `fixpoint_max=7` is neutral (converges early anyway), and `unroll_max=27` may not transfer — TPC-H scan kernels may not have unroll-friendly loop structures. The net effect is likely ≤ 5 % either direction, suggesting `Options::Default()` is already near-optimal for TPC-H.

---

## Further Questions/Directions of Research

- **Fix the TPC-H build** (mandatory for iter-3): identify which CMake target links the debug runtime and switch it to release. Verify with `strings TPCHBenchmark | grep library_build_type`.
- Run the 3-config transfer experiment (default, uc_workload_optimal, aggressive) on TPC-H at 5 reps per query.
- Consider a per-query-group Optuna mini-sweep (20 trials) to check whether TPC-H queries have a different optimal config family than UC kernels. This answers whether workload-specific tuning is necessary at all.
- Use Wilcoxon rank-sum (α=0.05) across all 22 query groups to produce a definitive statistical answer to spec SQ4.
- Cross-check: does the UC-optimal config's advantage on UC kernels persist when measured in the same binary run as TPC-H (i.e., is there any JIT context contamination between query types)?
