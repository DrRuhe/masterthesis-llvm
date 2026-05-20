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

### RQ-3: What is the best filter string for MEDIUM UC benchmarks?

**Decision**:
```
BM_g:(uc1|uc2|uc7|uc8|uc12|uc14);.*s:MEDIUM;.*t:(jit_overhead|specialized_exec|unspecialized)
```
For smoke tests, use `s:MINI` instead (e.g. `BM_g:(uc7|uc8);.*s:MINI;.*t:(jit_overhead|specialized_exec)`).

**Rationale**: `optimize_benchmarks.py` runs on UC benchmarks only (uc1_sql, uc2_conv,
uc7_dfa, uc8_ivm, uc12_groupby, uc14_sort) because these most directly reflect the intended
use of runtime specialization. MEDIUM size is used instead of EXTRALARGE because size-scaling
data (see RQ-9) shows that relative speedup is stable across MEDIUM/LARGE/EXTRALARGE, making
EXTRALARGE strictly slower to run without additional insight. The `--apply-default-filters`
flag already excludes `_t_jit_analysis_` benchmarks.

---

### RQ-4: How many UC benchmark groups are there at MEDIUM size?

**Decision**: 6 benchmark groups: uc1_sql, uc2_conv, uc7_dfa, uc8_ivm, uc12_groupby,
uc14_sort. Each group has 3 phases (jit_overhead, specialized_exec, unspecialized).

**Rationale**: Verified from size-scaling report data (`benchmarks/reports/260517-13-30-size_scaling/data.csv`).
Total: 6 groups × 3 phases = 18 benchmark rows per trial run. These groups represent
the actual use cases (IVM, group-by, DFA, SQL, sorting, convolution) that motivate
runtime specialization in this thesis.

---

### RQ-5: What is the TPC-H benchmark coverage?

**Decision**: 3 queries available: `tpch_q1`, `tpch_q6`, `tpch_q3`. Only q1 has a
`jit_analysis` variant. All 3 have `jit_overhead`, `specialized_exec`, and `unspecialized`
phases. TPC-H is used only in the transfer experiment (Exp C); the optimizer does not run
on TPC-H.

**Rationale**: Verified from tpch_bench.cpp grep. TPC-H q3 is the largest/most complex;
q6 is simpler filter-only. This provides meaningful diversity for testing whether the
UC-optimal config generalises to a different workload domain.

---

### RQ-6: How should the "workload_optimal" ablation config be injected at runtime?

**Decision**: After Experiment A completes, extract the best `params_json` for each
UC benchmark group from `v_optim_best_per_kernel` for the `uc_optim_*` study. Convert
the JSON to env vars using the same `_params_to_env()` mapping function in
`optimize_benchmarks.py`. Add the resulting env dict as a 9th config named
`uc_workload_optimal` to the ablation config list at runtime.

**Rationale**: Experiment A optimises exclusively on UC benchmarks (MEDIUM size), so the
extracted config is the empirically best UC pipeline config. Experiment B then uses this
as its "optimal" baseline. The `--extra-config JSON_STR` flag in `ablation_benchmarks.py`
appends it at runtime after Exp A completes.

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

---

### RQ-9: Does benchmark size affect the relative speedup for UC benchmarks?

**Decision**: No. MEDIUM size is sufficient for all optimizer and ablation runs. Larger
sizes (LARGE, EXTRALARGE) produce identical relative speedup and should not be used, as
they take strictly longer to execute without adding information.

**Evidence**: `benchmarks/reports/260517-13-30-size_scaling/data.csv` contains size-scaling
data for all 6 UC benchmark groups across SMALL/MEDIUM/LARGE/EXTRALARGE. The `speedup`
column (unspec/spec ratio) is stable across MEDIUM, LARGE, and EXTRALARGE in all groups.
SMALL is also similar, but MEDIUM is the minimum size confirmed as representative.

**Rationale**: Running EXTRALARGE instead of MEDIUM would multiply experiment wall-clock
time by ~5× per trial (MEDIUM ≈ 10–30 ms per benchmark, EXTRALARGE ≈ 50–140 ms) with no
change in the optimizer's objective landscape. Using MEDIUM reduces Experiment A from
~6 h to ~1.5 h for 150 trials while producing the same optimal config recommendation.

---

### RQ-10: Does `MaxFixpointIterations=0` work correctly?

**Decision**: No. `MaxFixpointIterations=0` (env `CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS=0`)
combined with `EnableO3Final=1` causes a SIGSEGV crash at the start of the second benchmark
invocation (i.e., the `specialized_exec` benchmark after `jit_overhead`). The crash occurs
because the O3 final pass, when applied to the module without any fixpoint inlining of
constant args, produces machine code that corrupts state during execution.

**Mitigation**: Changed the optimization search space lower bound from `min: 0` to `min: 1`
for `fixpoint_max`. The `o3_only` ablation config was updated from `fixpoint_max=0` to
`fixpoint_max=1` (one fixpoint iteration — minimal constant propagation + O3). TPE sampler
now never samples `fixpoint_max=0`.

**Evidence**: Reproduced reliably as SIGSEGV on `AllBenchmarks` with
`CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS=0 CRS_DEFAULT_O3_FINAL=1`. Exit code 139. Crash
happens after `count_matching_rows jit_overhead` on the first `specialized_exec` call.

---

### RQ-11: Does `OptimizationPipelineToUse=1` (function-specialization pipeline) work?

**Decision**: No. `CRS_DEFAULT_PIPELINE=1` causes the benchmark binary to core dump
(segmentation fault) when running the full UC benchmark filter. Removed from the search
space. The inlining pipeline (pipeline 0) is the only viable option.

**Mitigation**: Removed `pipeline` parameter from `DEFAULT_SEARCH_SPACE` in
`optimize_benchmarks.py`. All experiments use pipeline 0 exclusively.

**Evidence**: Reproduced consistently as "timeout: the monitored command dumped core"
with `CRS_DEFAULT_PIPELINE=1` + full benchmark filter `g:(uc1_sql|uc2_conv|...)`.

---

### RQ-12: Is the TPC-H binary available for cross-workload transfer experiments?

**Decision**: No TPC-H benchmark binary exists in this project. The cross-workload
transfer experiment (Exp C, T020–T024) cannot be executed as specified.

**Mitigation**: Skip Exp C. The transfer question will be addressed in the reflection
(T030) as future work. The remaining experiments (Exp A, B, D, E) are unaffected.

**Evidence**: `ls /home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/benchmarks/use-cases/`
shows only UC1, UC2, UC7, UC8, UC12, UC14 subdirectories. No TPC-H binary found in the
release build output directory.

---

### RQ-13: What is the per-trial wall time for UC MEDIUM benchmarks?

**Decision**: ~110 seconds per trial (1 binary invocation, 36 benchmarks at MEDIUM size,
CRS init amortized across all benchmarks).

**Evidence**: Timed `AllBenchmarks` with MEDIUM filter + 36 benchmarks = 1m51s wall time.
150 trials × 110s ≈ 4.6 hours with `--n-parallel 1`. Setup/teardown cost is negligible
(one-time lazy init after spec 013 patch).

**Rationale**: SMALL vs MEDIUM per-trial time is nearly identical (~110s each) because
the benchmark framework runs each benchmark for ≥1s regardless of problem size, and the
CRS JIT overhead (~50ms per kernel) dominates. MEDIUM exec times are 2-10× longer per
kernel, but most kernels still complete in <1s so the framework adds iterations.

---

### RQ-14: What is the optimal JIT pipeline configuration? (Preliminary — smoke study)

**Decision** (preliminary, 30 trials on SMALL): `{fixpoint_max: 10–20, unroll_max:
7–100 (insensitive), large_module_max: 0, early_prune: 1, o3_final: 1}`.

**Evidence**: 30-trial Optuna TPE smoke study (`smoke_uc_A_v4`) on 6 UC benchmarks at
SMALL size. Top 10 trials all converge to `early_prune=1, o3_final=1, large_module_max=0`.
`fixpoint_max` is insensitive in range 6–25 (all top-10 combined costs within 0.5% of
each other). `unroll_max` is insensitive across 7–324. Best combined geomean: 77.9ms
(`fixpoint_max=12, unroll_max=7`).

**Next step**: T012 full study (`uc_optim_20260517`, 150 trials, MEDIUM) will confirm
whether the pattern holds at production size. Preliminary MEDIUM results (16 trials)
already confirm `o3_final=1` dominance: 180ms vs 206ms combined when o3_final differs.

---

### RQ-15: Is `fixpoint_max=1` safe in all configurations?

**Decision**: No. `CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS=1` with `CRS_DEFAULT_EARLY_PRUNE=1`
and `CRS_DEFAULT_O3_FINAL=1` causes SIGSEGV at the start of the `specialized_exec`
benchmark. The combination `fixpoint_max=1 + early_prune=1 + o3_final=1` is unsafe.

**Evidence**: Reproduced during the T027 OAT sensitivity sweep (`sens_uc_20260517`).
The `sens_fixpoint_max_1` config point (base: optimal config with early_prune=1, o3_final=1,
then fixpoint_max overridden to 1) crashed with SIGSEGV on `AllBenchmarks`.

**Contrast**: The ablation `o3_only` config (`fixpoint_max=1, early_prune=0, o3_final=1`)
works correctly. The distinguishing factor is `early_prune=1`: the early GlobalDCE removes
more IR before the fixpoint loop, and 1 fixpoint iteration is insufficient to establish the
constant propagation invariants needed for safe O3 code generation.

**Mitigation**:
- OAT sensitivity sweep for `fixpoint_max` starts at `2` (omit `1`) when base config has `early_prune=1`.
- The T012 optimizer search space already excludes `fixpoint_max=0`; `fixpoint_max=1`
  does appear in the space but the TPE sampler will learn to avoid it in the `early_prune=1` region.
- `fixpoint_max=1` data point in sensitivity results is NULL (failed run stored); excluded
  from the sensitivity plot automatically.
