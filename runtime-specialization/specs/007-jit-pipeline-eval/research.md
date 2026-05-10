# Research: JIT Pipeline Evaluation Methodology

## Resolved Unknowns

### RQ-1: What ablation granularity is achievable without C++ changes?

**Decision**: Coarse-grained ablation using the 6 existing `CRS_DEFAULT_*` env vars.
No individual pass-enable flags (IPSCCP, DevirtVtable, GVN) are exposed.

**Rationale**: The 8 named configs (see plan.md) cover the most meaningful ablation
axes for the thesis: the presence/absence of the fixpoint loop, early pruning, final O3,
and loop unrolling. These map directly to architecture-level choices described in the
thesis. Pass-level ablation would require new Options fields and engineering time
disproportionate to the thesis benefit.

**Alternatives considered**:
- Add `EnableIPSCCP`/`EnableDevirtVtable` bool Options fields: provides finer resolution
  but requires C++ changes and new tests. Deferred as future work.
- Use a custom LLVM pass pipeline string (bypassing CRS): too invasive and would change
  the system under study.

---

### RQ-2: Does `scipy.stats.wilcoxon` require changes to the Nix flake?

**Decision**: Verify via `python3 -c "from scipy.stats import wilcoxon"` before writing
Experiment C code. If absent from the flake, add `python312Packages.scipy` to the
benchmark Python env in `flake.nix` as part of the implementation tasks.

**Rationale**: Scipy is a standard scientific Python package; it belongs in the Nix
environment, not installed ad-hoc. The implementation task list includes a `flake.nix`
check as the first step.

---

### RQ-3: What is the best filter string for EXTRALARGE polybench benchmarks?

**Decision**:
```
g:polybench.*s:EXTRALARGE.*(jit_overhead|specialized_exec|unspecialized)
```
For smoke tests, use `s:MINI` instead.

**Rationale**: The benchmark naming convention is `BM_g:polybench;n:KERNEL;s:SIZE;t:PHASE;`.
The `--apply-default-filters` flag in `optimize_benchmarks.py` already excludes
`_t_jit_analysis_` benchmarks. No additional filtering is needed for Experiments A/B/C/E.

---

### RQ-4: How many polybench kernels are at EXTRALARGE size?

**Decision**: 30 kernels (all POLYBENCH_BENCHMARK_SPEC entries in polybench_bench.cpp),
including: correlation, covariance, 2mm, 3mm, atax, bicg, doitgen, mvt, gemm, gemver,
gesummv, symm, syr2k, syrk, trmm, cholesky, durbin, gramschmidt, lu, ludcmp, trisolv,
deriche, floyd_warshall, nussinov, and 6 additional kernels in the BLAS group.

**Rationale**: Verified by grep on `POLYBENCH_BENCHMARK_SPEC` in polybench_bench.cpp.
Total: 30 kernels × 3 phases = 90 benchmark rows per trial run.

---

### RQ-5: What is the TPC-H benchmark coverage?

**Decision**: 3 queries available: `tpch_q1`, `tpch_q6`, `tpch_q3`. Only q1 has a
`jit_analysis` variant. All 3 have `jit_overhead`, `specialized_exec`, and `unspecialized`
phases.

**Rationale**: Verified from tpch_bench.cpp grep. TPC-H q3 is the largest/most complex;
q6 is simpler filter-only. This provides meaningful diversity within the SQL domain.

---

### RQ-6: How should the "workload_optimal" ablation config be injected at runtime?

**Decision**: After Experiment A completes, extract the best `params_json` for each
workload from `v_optim_best_per_kernel`. Convert the JSON to env vars using the same
`_params_to_env()` mapping function in `optimize_benchmarks.py`. Add the resulting
env dict as a 9th config named `workload_optimal` to the ablation config list at
runtime.

**Rationale**: This connects Experiment A's output directly to Experiment B's input,
ensuring the ablation baseline is the empirically best config, not just `Options::Default()`.

---

### RQ-7: For the Optuna importance analysis (Experiment E), where is the data stored?

**Decision**: Optuna's TPE sampler stores the study internally in an in-memory study
(per the current `optimize_benchmarks.py` design — no Optuna storage backend is used).
The importance scores must be computed *during* the same Python session, immediately
after `study.optimize()` completes.

**Mitigation**: Add importance extraction to `optimize_benchmarks.py` as an output step:
compute `optuna.importance.get_param_importances(study)` and store the result as JSON
alongside the best-config output file. This does not require schema changes.

---

### RQ-8: Can `--benchmark_repetitions=3` be used for ablation, or do we run the binary 3 times?

**Decision**: Run the binary 3 separate times (3 separate `context` + `benchmarks` rows
per config), not `--benchmark_repetitions=3` which produces aggregate rows.

**Rationale**: The existing `record_benchmark.py` and `optimize_benchmarks.py` use
`run_type=iteration` rows for timing. Aggregate rows from `--benchmark_repetitions` are
harder to query from DuckDB. Three separate runs produce 3 independent `run_id` values
that can be aggregated via `PERCENTILE_CONT(0.5)` in `v_ablation_medians`.
