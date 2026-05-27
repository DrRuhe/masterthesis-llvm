# Missing Data — Cost/Benefit Assessment

This document catalogs the experiments that would materially advance the research questions in `questions.md` but were not performed in this analysis step due to cost. Each entry includes an estimated runtime, the analysis it enables, and which RQ(s) it addresses.

---

## Priority 1: Cheap experiments (< 30 minutes each)

### M1: UC kernel pass traces (jit_analysis for all 18 UC kernels)
**What**: Run `jit_analysis` benchmarks for all 18 UC kernels × 3 abstraction levels (low / tradeoff / abstract) at MEDIUM size. These are already wired in the analysis binary (as evidenced by the 260523-102226-analysis run). Collect pass-trace JSON and import into `pass_traces` table.

**Cost**: ~54 benchmark runs × ~1.2 s = ~65 seconds total. Pass-trace file import adds ~1 minute.

**Enables**:
- **Q5**: Measure fixpoint convergence iteration per UC kernel (currently only proxy data from db_operators/polybench exists)
- **Q6**: Quantify per-pass instruction-count reductions for sort vs conv vs groupby vs IVM kernels
- **Q1**: Measure post-specialization instruction count for `batch_delta` and `multi_agg_delta` — determine whether spec IR has more instructions than AOT IR
- **Q2**: Compare P0 vs P1 module instruction counts after inlining step for sort vs conv

**How**: Run `AllBenchmarks --benchmark_filter='.*jit_analysis.*' --benchmark_out=<dir>/uc_traces.json` and use the existing `record_benchmark.py --pass-trace-dir <dir>` import.

---

### M2: P1 crash bisect (4-config run on uc8_ivm)
**What**: Run 4 configs × 3 reps on `uc8_ivm` to isolate the P1 SIGSEGV trigger (see Q3):
- Config A: `pipeline=1, unroll_max=27, p1_max_module_growth=1.643` (known to crash)
- Config B: `pipeline=1, unroll_max=27, p1_max_module_growth=2.0`
- Config C: `pipeline=1, unroll_max=4, p1_max_module_growth=1.643`
- Config D: `pipeline=1, unroll_max=4, p1_max_module_growth=2.0` (baseline P1 — known stable)

**Cost**: 4 configs × 3 reps × ~60s per run = ~12 minutes (assuming crashes are fast/near-instant).

**Enables**:
- **Q3**: Identifies the minimal crash-triggering parameter set
- Prerequisite for: the P1 stability fix that would unlock iter-3's P1 re-evaluation

**How**: Add 4 entries to a minimal `ablation_configs.json` and run `ablation_benchmarks.py --benchmark-filter=uc8_ivm`.

---

### M3: `unroll_max` 1D sensitivity sweep at iter-2 baseline
**What**: Run `unroll_max` at values {0, 4, 8, 16, 27, 41, 64, 128, 256} with all other iter-2 best params fixed. Measure on 2 kernels: `generic_sort` (uc14_sort) and `separable_gaussian` (uc2_conv).

**Cost**: 9 values × 2 kernels × 5 reps × ~2s per run = ~180 seconds.

**Enables**:
- **Q8**: Maps the full `unroll_max` response surface; determines if there's a sweet spot or monotone improvement
- **Q7**: Can be run at MEDIUM and EXTRALARGE sizes to test size-stability of optimal unroll_max

**How**: Add 9 OAT sweep points to `sens_uc_configs.json` for the specific kernels, run via `ablation_benchmarks.py`.

---

## Priority 2: Medium experiments (30 minutes – 4 hours)

### M4: TPC-H binary rebuild and Phase C re-run
**What**: Rebuild `TPCHBenchmark` linking the release runtime. Verify with `strings TPCHBenchmark | grep library_build_type`. Run Phase C transfer experiment: 3 configs (default, uc_workload_optimal, aggressive) × 22 TPC-H queries × 5 reps.

**Cost**: Build time ~10 minutes. Run time: 3 × 22 × 5 × (JIT overhead + query time per TPC-H query). TPC-H queries range from 0.1s to 5s depending on SF; at SF=1 estimate ~30 minutes total.

**Enables**:
- **Q4**: The only experiment that can answer spec SQ4 ("does optimal config generalize to TPC-H?"). Currently **unanswered**.

**How**: Fix the CMake target that links the debug runtime. Likely: identify which `target_link_libraries` in `benchmarks/CMakeLists.txt` pulls in the debug blob and switch to the release target.

---

### M5: Size-stability experiment for optimal unroll_max
**What**: 3 configs (`unroll_max=4, 27, 54`) × 3 sizes (MEDIUM, LARGE, EXTRALARGE) × 2 UC groups (`uc14_sort`, `uc2_conv`) × 5 reps.

**Cost**: 3 × 3 × 2 × 5 × ~5s (EXTRALARGE kernels) = ~450 seconds ≈ 8 minutes.

**Enables**:
- **Q7**: Directly answers whether `unroll_max=27` transfers across problem sizes
- **Q8**: Confirms or refutes whether the iter-2 sensitivity ranking is stable across sizes

**How**: Extend the size-scaling benchmark filter to include 3 named ablation configs; run via existing `run_evaluation.sh SIZE_SCALING_FILTER`.

---

### M6: Per-kernel Optuna mini-sweep (uc14_sort only, 30 trials)
**What**: Run a 30-trial Optuna study scoped to `generic_sort` only, varying `unroll_max` and `fixpoint_max` (holding other params at iter-2 best). Use `--benchmark-filter=uc14_sort/generic_sort`.

**Cost**: 30 trials × 2 kernels per filter × 5 reps × ~5s per run = ~25 minutes.

**Enables**:
- **Q7, Q8**: Identifies the true optimal `unroll_max` for `generic_sort` at its current baseline
- **Q2**: May discover whether there is a P0 config that matches P1's 17 % gain (via higher `unroll_max` alone)

**How**: Modify `optimize_benchmarks.py` to scope the study to a single kernel filter.

---

## Priority 3: Expensive experiments (> 4 hours)

### M7: Two-objective Pareto Optuna study (jit_ns vs spec_ns separately)
**What**: Run Optuna in multi-objective mode with objectives `[jit_ns, spec_ns]` independently across all UC kernels. 100 trials.

**Cost**: 100 trials × 18 kernels × 5 reps × ~2s = ~5 hours.

**Enables**:
- **Q10**: Produces the full Pareto frontier between JIT overhead and exec quality, answering whether the scalar combined-cost objective is a good approximation
- **Q9**: Identifies configs that minimize spec_ns for high-call-count workloads

**Deferred because**: The existing scalar Pareto plots (per-group, 50 trials) already provide partial insight. The two-objective study is only necessary if the thesis needs to make a formal claim about the objective function's correctness.

---

### M8: Perf counter profiling for batch_delta and box_filter
**What**: Run `perf stat --event=instructions,L1-icache-load-misses,L1-dcache-load-misses,branch-misses` for the specialized and unspecialized versions of `batch_delta` and `box_filter`. Requires `perf` access (kernel PMU counters).

**Cost**: 20 minutes of measurement time; significant setup effort to integrate perf into the benchmark pipeline.

**Enables**:
- **Q1, Q6**: Distinguishes icache pressure, dcache misses, and branch mispredictions as the cause of slowdown vs speedup. Currently, the cause for `batch_delta`'s regression and `box_filter`'s 2.8× speedup is inferred from ablation, not directly measured.

**Deferred because**: Requires kernel PMU access (may need `perf_event_paranoid=0`) and is not supported in the current benchmark framework. The analytical evidence from ablation and pass traces is sufficient for thesis purposes.

---

### M9: Per-size Optuna study (EXTRALARGE-only, 50 trials)
**What**: Run a full 50-trial Optuna study scoped to EXTRALARGE-only UC kernels, with the same search space as iter-2. Compare the resulting optimal config against the MEDIUM-size winner.

**Cost**: 50 trials × 18 kernels × 5 reps × ~15s (EXTRALARGE runtime) = ~10 hours.

**Enables**:
- **Q7**: Definitive answer to whether the optimal pipeline config changes with workload size
- **Q8**: Would reveal whether sensitivity rankings shift at EXTRALARGE (is `unroll_max` still the dominant secondary parameter?)

**Deferred because**: The analytical argument (inner loop trip count for conv is size-independent; sort's innermost comparison loop is size-independent) is likely sufficient for the thesis. Only run if the reviewer explicitly challenges the size-generalization claim.

---

## Summary Table

| ID | Experiment | Cost | RQs Addressed | Priority |
|----|-----------|------|---------------|----------|
| M1 | UC kernel pass traces | ~2 min | Q5, Q6, Q1, Q2 | **High** |
| M2 | P1 crash bisect (4 configs) | ~12 min | Q3 | **High** |
| M3 | `unroll_max` 1D sweep | ~3 min | Q8, Q7 | **High** |
| M4 | TPC-H rebuild + Phase C | ~40 min | Q4 | **High** (blocks SQ4) |
| M5 | Size-stability experiment | ~8 min | Q7, Q8 | Medium |
| M6 | Per-kernel Optuna (sort) | ~25 min | Q7, Q2, Q8 | Medium |
| M7 | Two-objective Pareto study | ~5 hours | Q10, Q9 | Low |
| M8 | Perf counter profiling | ~1 hour + setup | Q1, Q6 | Low |
| M9 | EXTRALARGE Optuna study | ~10 hours | Q7, Q8 | Low |
