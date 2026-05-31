# Benchmark Methodology Audit

**Date**: 2026-05-31  
**Auditor**: Claude Sonnet 4.6 (1M context)  
**Files inspected**: `thesis.typ` (§Evaluation onward), `run_evaluation.sh`, `ClangRuntimeSpecializerBenchmark.h`, `AllBenchmarks_main.cpp`, `optimize_benchmarks.py`, `ablation_benchmarks.py`, `record_benchmark.py`, `UC1Benchmark.cpp`, `UC1CountMatchingRowsLowKernels.cpp`, `specs/007-jit-pipeline-eval/spec.md`, `specs/008-use-case-benchmarks/spec.md`, `specs/011-uc-benchmark-abstraction-variants/spec.md`, `reports/260531-09-31-optimize-pipeline/reflection.md`, `reports/260523-11-35-open-research-questions/questions.md`, `reports/260525-121400-Pipeline/explanation.md`

---

## Summary

The benchmark infrastructure is technically sophisticated and shows clear awareness of several common pitfalls (batch-calling pattern to avoid dispatch-overhead contamination, a JIT warmup pass to separate LLVM startup cost from per-kernel JIT cost, multi-phase measurement separating JIT overhead from execution speedup, perf-counter recording, a `best_practice_env` context manager for ASLR and CPU frequency pinning). The most serious threat to thesis validity is that the environment hardening in `best_practice_env` is only invoked through `record_benchmark.py`'s manual-run path; the Optuna optimization loop (`optimize_benchmarks.py`) and the ablation loop (`ablation_benchmarks.py`) called from `run_evaluation.sh` do **not** invoke that path, meaning the bulk of reported measurements were taken without CPU pinning, without disabling Turbo Boost, and without ASLR mitigation. A second critical gap is that all benchmark repetition counts remain at one per process invocation (`--benchmark_repetitions=1`), so Google Benchmark cannot compute confidence intervals, and the 5-rep ablation relies entirely on inter-process variance rather than intra-run statistical guarantees. Additionally, the thesis evaluation chapter is largely a TODO skeleton—hardware, compiler version, build flags, and statistical methods are not yet written down—so claim validity cannot be fully assessed.

---

## Issues by Category

---

### Statistical Rigor

---

**Issue SR-1: All Optuna trial runs use `--benchmark_repetitions=1`; no intra-run variance estimate**  
- **Evidence**: `optimize_benchmarks.py` lines 376, 419: `"--benchmark_repetitions=1"`. `ablation_benchmarks.py` line 98: `"--benchmark_repetitions=1"`. The Optuna objective is computed from a single Google Benchmark run per trial.  
- **Severity**: HIGH  
- **Impact**: A single trial may land on an outlier JIT time (e.g., OS scheduling hiccup, memory pressure from prior trial's memory-mapped code). The TPE optimizer has no variance signal; it may consistently prefer configurations that happened to run in a low-noise slot. Phase B ablation uses 5 inter-process repetitions (`--reps 5`) but combines them as medians across processes, not within a process—so Google Benchmark's own mean/stddev infrastructure is never exercised.  
- **Suggested fix**: Add `--benchmark_repetitions=5` (or at least 3) inside each subprocess invocation so Google Benchmark reports a mean±stddev. Use the stddev to flag unstable measurements. At minimum, use `--benchmark_min_time=0.5` for short benchmarks to ensure adequate sample counts.

---

**Issue SR-2: JIT overhead benchmark has monotonically growing per-iteration latency due to dylib accumulation**  
- **Evidence**: `ClangRuntimeSpecializerBenchmark.h` `benchmarkJITOverhead` (line 77–100): each Google Benchmark iteration calls `specializeOnly`, which adds a new JITDylib module. `thesis.typ` line 630–631 explicitly notes: "JIT dylib state growth: each specializeOnly call in a loop adds a module to the ORC dylib; lookup time grows monotonically."  
- **Severity**: HIGH  
- **Impact**: The reported `jit_overhead` is the mean across multiple iterations of `specializeOnly`, but the first iteration pays only compilation cost while later iterations pay compilation + growing dylib lookup overhead. For benchmarks with many iterations (Google Benchmark auto-selects iteration count to fill ~0.5 s), later iterations are systematically slower, inflating the reported mean. This is not just noise—it is a structural upward bias in JIT overhead measurements.  
- **Suggested fix**: Restrict JIT overhead benchmarks to `->Iterations(1)->UseManualTime()` (the pattern already used for `jit_analysis` and `jit_init_warmup`). The `benchmarkJITOverhead` template already receives a `specArgs` tuple but does not use the `Iterations(1)` constraint. Using the single-invocation pattern ensures only the genuine per-call compilation cost is measured.

---

**Issue SR-3: No confidence intervals or significance tests are reported for speedup claims**  
- **Evidence**: The thesis evaluation chapter (`thesis.typ` §RQ1–RQ6) is entirely TODO stubs. The spec 007 acceptance scenario 2 mentions a "paired Wilcoxon rank-sum test (α=0.05)" for the transfer experiment, but this is a specification goal, not an implemented test. The reflection documents (e.g., `260531-09-31-optimize-pipeline/reflection.md`) report speedup ratios to two decimal places (e.g., `2.97×`) with no uncertainty bounds.  
- **Severity**: HIGH  
- **Impact**: For a Master's thesis making performance claims, reporting `2.97×` speedup without a confidence interval is not scientifically defensible. A reviewer can ask: is the difference between 2.97× and 2.38× real or within measurement noise?  
- **Suggested fix**: Report mean ± 1.96 × (stddev / √N) as a 95% CI alongside all speedup ratios. For Phase B ablation (5 reps), report the inter-rep coefficient of variation. For pairs of configurations, apply a paired t-test or Wilcoxon signed-rank test and report p-values.

---

**Issue SR-4: JIT overhead sample count is borderline for ~40–60 ms measurements**  
- **Evidence**: `thesis.typ` line 632–634 (TODO section): "~15 iterations at 40–60 ms each is borderline for a stable mean." Google Benchmark auto-selects iterations to run for ~0.5 s, yielding ~8–12 iterations at 40–60 ms. This is acknowledged but not yet fixed.  
- **Severity**: MEDIUM  
- **Impact**: With 8–12 samples drawn from a distribution with high right-skew (cold JIT compilation with occasional OS preemption), the sample mean can vary significantly run-to-run. The thesis acknowledges this but does not state what was done about it.  
- **Suggested fix**: Set `--benchmark_min_time=2` (2 seconds minimum) for JIT overhead benchmarks to force ≥ 30 iterations at 40–60 ms, improving mean stability. Alternatively, accept the limitation explicitly and report `median ± MAD` instead of `mean ± stddev` as the median is more robust to the right-skew.

---

**Issue SR-5: The Phase A optimizer objective conflates JIT overhead and execution speedup into a single scalar**  
- **Evidence**: `optimize_benchmarks.py` computes `objective = geomean(jit_ns + spec_ns)` per kernel. `reports/260523-11-35-open-research-questions/questions.md` RQ-10 explicitly flags this as a methodology weakness: "The objective implicitly assumes one call amortises the JIT overhead. For workloads with many calls this underweights execution speedup; for one-shot workloads it is appropriate."  
- **Severity**: MEDIUM  
- **Impact**: The "best" config found by the optimizer is only optimal for the scenario where each specialized function is called exactly once. A production query engine calling a specialized filter 10 million times should weight `spec_ns` far more heavily than `jit_ns`. The reported "best config" may not be the best config for the thesis's stated use cases.  
- **Suggested fix**: Report the Pareto front (already implemented in `plot_pareto.py`) as the primary result rather than the scalar objective. In the thesis, parameterize break-even N and show how the optimal configuration shifts as a function of expected call count.

---

### Experimental Design

---

**Issue ED-1: The unspecialized baseline for the Optuna optimizer runs without CPU pinning, ASLR mitigation, or governor control**  
- **Evidence**: `optimize_benchmarks.py` calls `subprocess.run([binary, ...])` directly without invoking `best_practice_env` (confirmed by searching for `best_practice_env` in that file—zero matches). The baseline measurement runs in whatever system state the operator machine is in. This applies to all 50 Phase A trials.  
- **Severity**: CRITICAL  
- **Impact**: The optimizer is choosing configurations based on measurements that mix noise sources: Turbo Boost frequency swings (a single core can jump from 2.4 GHz to 5.2 GHz on modern Intel CPUs), OS scheduling jitter, and ASLR-induced cache-layout variation. Two trials evaluated under different frequency states are not comparable. The "best config found" may reflect a lucky frequency boost rather than a better pipeline.  
- **Suggested fix**: Either (a) invoke `best_practice_env` inside `optimize_benchmarks.py` before the Optuna loop, or (b) document explicitly that environment hardening is not applied and note the expected variance inflation (e.g., ±5–10% on JIT times).

---

**Issue ED-2: No explicit verification that `best_practice_env` was active for thesis-reported numbers**  
- **Evidence**: `record_benchmark.py` stores a `best_practice_full` boolean in the `context` table (line 757), but `optimize_benchmarks.py` never calls `store_context` with `best_practice_full`; the field is null for all optimizer trial records. `run_evaluation.sh` calls `optimize_benchmarks.py`, `ablation_benchmarks.py`, and `sensitivity_analysis.py` without the `--benchmarking-best-practice` flag.  
- **Severity**: CRITICAL  
- **Impact**: If the thesis chapter presents numbers from `v_optim_best_per_kernel` and `v_ablation_medians` as the main empirical results, all those numbers were collected under uncontrolled system conditions.  
- **Suggested fix**: Before writing the evaluation chapter, run a final "thesis run" using `record_benchmark.py --benchmarking-best-practice` to collect the numbers that actually appear in the thesis. Verify `best_practice_full=TRUE` in the database for all cited rows.

---

**Issue ED-3: The jit_overhead benchmark measures a fresh JIT on every iteration, but subsequent iterations share the same LLJIT instance (accumulating dylibs)**  
- **Evidence**: `ClangRuntimeSpecializerBenchmark.h` `benchmarkJITOverhead` calls `ClangRuntimeSpecializer::init()` once before the loop (line 77), then calls `specializeOnly` in each iteration. `ClangRuntimeSpecializer::init()` is a singleton (returns the same instance). The LLJIT's `JITDylib` count grows each iteration, increasing symbol lookup overhead.  
- **Severity**: HIGH  
- **Impact**: The numbers reported for `jit_overhead` benchmarks with Google Benchmark's auto-selected iteration count (8–12 iterations for ~50 ms JIT) include a blend of "first cold JIT" and "Nth dylib-accumulated JIT." For ablation studies comparing 5 reps × N iterations, later reps pay more accumulated dylib overhead than earlier reps, introducing a systematic upward drift that is not pipeline-related.  
- **Suggested fix**: The `jit_analysis` benchmarks already use `->Iterations(1)` which avoids this. The `jit_overhead` benchmarks should do the same, with `UseManualTime()`. Alternatively, reset the specializer singleton between benchmark iterations (architecturally expensive but correct).

---

**Issue ED-4: The abstraction-level comparison hypothesis (abstract > low speedup) is not guaranteed to be testable from the current benchmark design**  
- **Evidence**: `specs/011-uc-benchmark-abstraction-variants/spec.md` SC-002: "For at least 4 of 6 use-cases, the abstract-tier variant shows a higher specialization speedup ratio than the low-level variant." This is a thesis hypothesis, not a constraint. Nothing in the benchmark design ensures that the abstract-tier implementations have more "eliminable" overhead than the low-tier ones.  
- **Severity**: MEDIUM  
- **Impact**: If the JIT cannot devirtualize the abstract-tier virtual calls (e.g., because vtable pointers are not constant at JIT time), the abstract-tier result will be equal to or worse than the low-tier result. The hypothesis could be refuted on a technicality of the JIT's devirtualization capability, not because the research question is wrong.  
- **Suggested fix**: Before committing to this as an RQ, verify on a representative abstract-tier benchmark that the JIT actually eliminates virtual dispatch (e.g., by checking instruction count reduction in `jit_analysis` output). If devirtualization consistently fails, reframe RQ3 as "can JIT devirtualize abstract C++ patterns?" rather than "does abstraction amplify speedup?"

---

**Issue ED-5: Transfer experiment (TPC-H) was incomplete due to a binary bug; RQ4 is currently unanswered**  
- **Evidence**: `reports/260523-11-35-open-research-questions/questions.md` RQ4: "Phase C (cross-workload transfer) was incomplete: TPC-H crashed with SIGSEGV due to a debug-mode runtime embedded in the release binary. Only the UC half ran (UC transfer showed no advantage over Default within noise). This question directly answers spec SQ4 ('Does the optimal configuration generalise across workloads?') and is currently **unanswered**."  
- **Severity**: HIGH  
- **Impact**: A thesis RQ ("does the optimal config generalise?") has no answer. The UC-to-UC transfer result (no advantage over Default) is consistent with the hypothesis that the default config is already near-optimal, but without TPC-H data the cross-workload claim cannot be made.  
- **Suggested fix**: Fix the TPC-H binary issue and run Phase C before thesis submission. If TPC-H results are unavailable, explicitly acknowledge in the thesis that RQ4 is unanswered and is future work.

---

**Issue ED-6: The Optuna study uses only 50 trials for a parameter space with up to 12 dimensions**  
- **Evidence**: `run_evaluation.sh` line 248: `--n-trials 50`. The search space descriptor in `optimize_benchmarks.py` has 12 parameters (6 shared + 2 P1-specific + 4 P2-specific). The iter-2 reflection (referenced in questions.md) used 150 trials but iter-3 reduced to 50.  
- **Severity**: MEDIUM  
- **Impact**: With 50 trials in a 12-dimensional mixed space (3 categorical choices for pipeline alone), Optuna TPE has limited exploration budget. The best config found may be a local optimum. The reflection notes "Optuna convergence curve" as a TODO—without a convergence plot it is unknown whether 50 trials is sufficient.  
- **Suggested fix**: Plot the Optuna convergence curve (objective vs. trial number) and verify that the best objective has not improved in the last ~15 trials. If still improving, increase to 100 trials. Report the convergence plot in the thesis.

---

### Confounds

---

**Issue C-1: JIT dylib symbol lookup time grows monotonically across benchmark suite execution (architectural, not measurement artifact)**  
- **Evidence**: `thesis.typ` line 630–631 acknowledges: "each specializeOnly call in a loop adds a module to the ORC dylib; lookup time grows monotonically. This is an architectural characteristic, not a measurement artifact, and is reported as-is." The JIT overhead benchmark runs all kernels sequentially within one process.  
- **Severity**: HIGH  
- **Impact**: The JIT overhead measured for a kernel that runs 30th in the benchmark sequence is systematically higher than if it ran first, due to the growing dylib. This makes cross-kernel JIT overhead comparisons within the same binary run incomparable. It also means the "first" kernel (`count_matching_rows`) was paying startup cost + dylib-0 overhead while later kernels pay dylib-N overhead.  
- **Suggested fix**: The `BM_jit_init_warmup` benchmark (added 2026-05-31) mitigates the LLVM startup cost. To also address dylib growth, measure each kernel's JIT overhead in isolation (separate process per kernel). This is more expensive but produces clean, comparable numbers. At minimum, report in the thesis that quoted JIT times include accumulated dylib overhead proportional to the kernel's sequential position.

---

**Issue C-2: The `jit_overhead` benchmark re-JITs the same function in each iteration, which is not the production use case**  
- **Evidence**: `benchmarkJITOverhead` in `ClangRuntimeSpecializerBenchmark.h` calls `specializeOnly` in every loop iteration with the same `specArgs`. In production, a function is specialized once and called many times. The repeated re-JIT measures "what if you specialize the same function N times?"—which is not the stated use case.  
- **Severity**: MEDIUM  
- **Impact**: The re-JIT overhead on the 2nd and later iterations is higher than the 1st because the dylib lookup must search through more entries. The reported mean JIT time thus reflects an artificial workload.  
- **Suggested fix**: This is already mitigated for the `jit_analysis` tier via `->Iterations(1)`. Consider adding a note in the thesis that the `jit_overhead` tier measures "worst-case repeated re-specialization" rather than "one-time specialization cost," which is what production users experience.

---

**Issue C-3: Unspecialized baselines for optimizer trials are measured once and reused across 50 trials**  
- **Evidence**: `optimize_benchmarks.py` `measure_unspecialized` (line 364) is called once before the Optuna loop. The resulting `unspec_ns` dict is used as the denominator for speedup across all 50 trials. Unspecialized time is itself subject to measurement noise.  
- **Severity**: MEDIUM  
- **Impact**: A single noisy unspecialized measurement can inflate or deflate all speedup ratios for the entire study. If the baseline happened to run slowly (system load spike), all 50 trials will show inflated speedup.  
- **Suggested fix**: Measure the unspecialized baseline at least 3 times and use the median. Alternatively, run the unspecialized benchmark as part of each trial invocation (already done in the `UC_FILTER_WITH_UNSPEC` filter path) and use the within-trial unspecialized measurement as the denominator.

---

**Issue C-4: No control for cross-kernel interference within a single binary invocation**  
- **Evidence**: `optimize_benchmarks.py` runs all filtered benchmarks in a single subprocess invocation (`run_all_benchmarks`). A kernel benchmark that causes large allocations or cache pressure affects subsequent kernels in the same run.  
- **Severity**: LOW  
- **Impact**: UC benchmarks involve 1 GB data buffers (per spec 008, FR-007). When multiple UC groups run sequentially, earlier groups leave L3/memory bandwidth in a polluted state for later groups. The last benchmark in the sequence is measured under different cache conditions than the first.  
- **Suggested fix**: Run each benchmark group in isolation (separate subprocess per group) when collecting thesis-reported numbers. This is slower but removes cross-kernel interference. The optimizer can continue using the combined invocation for speed, but the final validation run should use isolation.

---

**Issue C-5: Benchmark ordering within the binary is fixed and not randomized**  
- **Evidence**: `AllBenchmarks_main.cpp` registers benchmarks in a fixed order. Google Benchmark runs them in registration order unless `--benchmark_shuffle` is used. The evaluation scripts do not pass `--benchmark_shuffle`.  
- **Severity**: LOW  
- **Impact**: Systematic ordering effects (cache warm-up, OS scheduler state, TLB state) may consistently favor some kernels over others. This is a minor confound given the large batch sizes but worth acknowledging.  
- **Suggested fix**: Add `--benchmark_shuffle=true --benchmark_shuffle_seed=42` to the final thesis measurement runs to verify that ordering does not affect results.

---

### Reproducibility

---

**Issue R-1: Hardware specification is a TODO in the thesis; CPU model, cache sizes, and frequency scaling state are not written down**  
- **Evidence**: `thesis.typ` line 544–550: `#TODO[ Describe the evaluation machine: CPU model, core count, cache hierarchy. CPU frequency scaling is addressed (pin to a fixed frequency or note the warning). ]`. The benchmark JSON output contains `mhz_per_cpu` and `num_cpus` fields (from Google Benchmark's context), and the `record_benchmark.py` database schema stores these, but the thesis has not transcribed them.  
- **Severity**: HIGH  
- **Impact**: Without a documented hardware specification, the thesis claims cannot be reproduced or compared against related work. The JIT overhead numbers (40–60 ms) are strongly machine-specific; stating them without hardware context is misleading.  
- **Suggested fix**: Fill in the `#TODO` section with the actual CPU model (visible in benchmark output headers, e.g., `Run on (20 X 400 MHz CPU s) CPU Caches: L1 Data 48 KiB...`), LLVM version (21.1.0), build flags (Clang, `-O3`), OS version, and whether CPU frequency scaling was controlled for thesis-reported measurements.

---

**Issue R-2: Compiler version, build flags, and LLVM configuration are not documented in the thesis**  
- **Evidence**: `thesis.typ` line 547: "all benchmarks run in release build (-O3)." No other build metadata is present. The MEMORY.md mentions LLVM 21.1.0 but this does not appear in the thesis draft.  
- **Severity**: HIGH  
- **Impact**: The JIT pipeline behavior is highly version-specific (IPSCCP heuristics, inliner cost model, vectorizer). A reader attempting to reproduce results with a different LLVM version may see different outcomes.  
- **Suggested fix**: Add a table in §Setup covering: Clang/LLVM version, build type (Release, assertions disabled), CMake flags, and the Nix flake hash or commit SHA for full reproducibility.

---

**Issue R-3: `run_evaluation.sh` does not capture benchmark binary git SHA or build date in the report directory**  
- **Evidence**: `run_evaluation.sh` logs `UC binary: $UC_BINARY` and `TPCH binary: $TPCH_BINARY` to `eval_run_log.txt` (lines 98–99) but does not record the binary's build SHA or when it was compiled. `record_benchmark.py` passes `--benchmark_context=git_sha=...` when run directly, but `optimize_benchmarks.py` and `ablation_benchmarks.py` do not pass git SHA context.  
- **Severity**: MEDIUM  
- **Impact**: The reflection documents reference "the binary built on 2026-05-31 (includes uncommitted P0 fix)" but a reader inspecting the database cannot verify which binary produced which trial without external notes. If the binary is rebuilt between phases A and B of the same run, results are silently mixed.  
- **Suggested fix**: Record the binary SHA (`sha256sum $UC_BINARY`) in `eval_run_log.txt` at the start of the run. Pass `--benchmark_context=binary_sha256=...` to all benchmark invocations.

---

**Issue R-4: The benchmarks database (`benchmarks.duckdb`) is a binary file under git tracking with no export/snapshot mechanism**  
- **Evidence**: `git status` shows `benchmarks/eval_smoke.duckdb` as modified. The main `benchmarks.duckdb` is presumably also binary. There is no `dump_db.sh` or equivalent to produce a reproducible text export.  
- **Severity**: MEDIUM  
- **Impact**: A reviewer cannot inspect the raw data without the DuckDB binary and the exact file. DuckDB file format may change across versions. If the file is corrupted or upgraded, historical data is lost without a text-format backup.  
- **Suggested fix**: Add a `make export-db` target that runs `EXPORT DATABASE 'db_export/' (FORMAT CSV)` before each thesis submission, committing the CSV snapshots. This provides a human-readable, format-independent backup of all thesis-relevant data.

---

**Issue R-5: Optuna seed is fixed at 42 (good) but n-parallel is 1 (sequential), making runs reproducible but slow**  
- **Evidence**: `run_evaluation.sh` line 248: `--seed 42 --n-parallel 1`. Fixed seed is correct for reproducibility. However, with `n-parallel=1`, each of the 50 Phase A trials runs sequentially in one process—total Phase A time is 50 × 60 s timeout = up to 50 minutes, typically much less given most trials complete in ~10 s.  
- **Severity**: LOW  
- **Impact**: Not a validity threat, but the sequential design means evaluation runs cannot leverage multi-core machines. This is intentional (per MEMORY.md: "never run multiple AllBenchmarks processes at once; sequential only") to avoid OOM, which is the correct policy.  
- **Suggested fix**: No change needed; document the sequential-only constraint and its rationale in the thesis §Setup.

---

### Claim Validity

---

**Issue CV-1: The "annotation-free" claim requires the TU separation constraint, which is a non-trivial user burden**  
- **Evidence**: `ClangRuntimeSpecializer.h` MEMORY.md: "The TU separation pattern is a non-negotiable constraint. If kernel and benchmark infrastructure share a TU, the JIT blob contains vtable functions from benchmark infrastructure, causing 'Symbols not found' crashes." `specs/008-use-case-benchmarks/spec.md` FR-001: "Each benchmark MUST follow the TU separation pattern." The Discussion section of `thesis.typ` does not yet address this as a user burden.  
- **Severity**: HIGH  
- **Impact**: The thesis claims the system is "annotation-free" and "drop-in." But users must restructure their code to separate kernel TUs from application TUs. For existing codebases this may require significant refactoring. The claim should be qualified: "annotation-free at the call site, but requires TU-level organization of specialization targets."  
- **Suggested fix**: Add a "Limitations" subsection to the thesis explicitly stating the TU separation constraint and quantifying the refactoring burden (e.g., "requires moving target functions to separate compilation units, which typically involves N lines of change in existing codebases").

---

**Issue CV-2: Speedup claims are against a baseline that runs specialized functions through an indirect call pointer, not through a direct call**  
- **Evidence**: `thesis.typ` lines 585–596 correctly notes: "The ~0.5 ns difference reflects the cost of an indirect function call, not function body cost." The specialized `exec` benchmark calls through an opaque function pointer; the unspecialized benchmark calls the function directly (or with `DoNotOptimize`). For kernels that take ~1 ns (sub-microsecond), the ~0.5 ns indirect call overhead is a material fraction.  
- **Severity**: MEDIUM  
- **Impact**: For long-running batch kernels (MEDIUM/LARGE size), the 0.5 ns per-call overhead is negligible. But the thesis narrative should explicitly account for this when discussing cases like `column_scan` (0.84 ns/call unspec, ~1 ns effective call overhead in spec version). Claims of "1.04× speedup for column_scan" need to note that part of the apparent slowdown is the unavoidable indirect call overhead, not specialization quality.  
- **Suggested fix**: Include a "dispatch overhead" line in the results table for kernels where it is a significant fraction. The `BM_jit_init_warmup` / `mypow_bench` measurements already quantify this at ~0.5 ns; cite this number explicitly.

---

**Issue CV-3: P1 pipeline has known crash (SIGSEGV) under certain parameter combinations; the ablation config set excludes `fixpoint_1` due to this**  
- **Evidence**: `ablation_benchmarks.py` BUILTIN_CONFIGS line 38: `# fixpoint_1 excluded: fixpoint_max=1 + early_prune=1 (default) + o3_final=1 (default) → SIGSEGV (RQ-15)`. `reports/260523-11-35-open-research-questions/questions.md` RQ3: "100% crash rate" under specific P1 + unroll_max=27 combinations. Root cause is unknown.  
- **Severity**: HIGH  
- **Impact**: A production JIT system with known SIGSEGV conditions is a correctness issue, not just a performance issue. The thesis claims the system is production-quality ("drop-in"). An unreproduced crash under specific parameter combinations is a counterexample that must be addressed or explicitly scoped out.  
- **Suggested fix**: Before thesis submission, either (a) bisect and fix the P1 crash (root cause believed to be interaction of `unroll_max` and `p1_max_module_growth`, per RQ3 in questions.md), or (b) explicitly exclude P1 from the thesis's performance claims and scope the thesis to P0 and P2 only.

---

**Issue CV-4: The speedup decomposition (how much comes from constant folding vs. devirtualization vs. unrolling) is unquantified**  
- **Evidence**: `reports/260523-11-35-open-research-questions/questions.md` RQ6: "The ablation study (Phase B) tested named configurations (no_unroll, no_prune, O3Only, etc.) but did not isolate the contribution of individual transformation types within the fixpoint loop." The pass-trace infrastructure (`g_lastPassTrace`, `writePassTraceJSON`) exists and captures per-pass instruction count changes, but no analysis connecting per-pass IR reduction to execution speedup has been done.  
- **Severity**: MEDIUM  
- **Impact**: The thesis claims the system achieves speedup through "constant propagation, devirtualization, and dead-branch elimination." Without a quantitative decomposition, this is a narrative, not a finding. A reviewer will ask: "how much of the 2.97× speedup for box_filter comes from loop unrolling vs. constant propagation?"  
- **Suggested fix**: Run `jit_analysis` benchmarks for representative kernels, import pass traces, and compute per-pass instruction count reduction. Correlate with execution speedup. Even an informal "pass X removed Y% of instructions, and execution sped up by Z%" is stronger than no decomposition.

---

**Issue CV-5: The optimal config found for UC MEDIUM benchmarks has not been validated on UC LARGE/EXTRALARGE or Polybench**  
- **Evidence**: `specs/007-jit-pipeline-eval/spec.md` mentions: "Size-scaling data shows that relative speedup is stable across MEDIUM/LARGE/EXTRALARGE for all UC groups." But `reports/260523-11-35-open-research-questions/questions.md` RQ7 notes: "it did not validate that the **optimal pipeline configuration** is stable across sizes." The Optuna study optimizes only on MEDIUM.  
- **Severity**: MEDIUM  
- **Impact**: The thesis claims the optimal configuration is P2+O3. But `unroll_max` and `fixpoint_max` are size-sensitive parameters. At EXTRALARGE, loop unrolling may be more profitable (longer trip counts) or less profitable (icache pressure). The thesis should either run a size-stratified optimization or explicitly scope its optimal-config claim to MEDIUM.  
- **Suggested fix**: Run at least one Phase A study on LARGE size with the same search space. If the best config matches MEDIUM, claim robustness. If it differs, qualify the claim as "optimal for MEDIUM; configuration may differ at other sizes."

---

**Issue CV-6: Polybench results are not yet integrated into the thesis evaluation**  
- **Evidence**: `thesis.typ` §RQ1 TODO: "Present the main speedup results across UC benchmarks, PolyBench, and TPC-H." The reports directory contains polybench entries and the memory notes mention 30 kernels. But the evaluation chapter only discusses UC benchmarks in the reflection documents; no Polybench-specific RQ analysis is present.  
- **Severity**: MEDIUM  
- **Impact**: Polybench is the standard benchmark for JIT specialization papers (Numba, Julia, etc.). Without Polybench results the thesis cannot be positioned relative to prior work. The comparison table in the Discussion chapter (thesis.typ line 813–823) is a TODO.  
- **Suggested fix**: Run Phase B/C ablation on Polybench before thesis write-up. Include at least the geomean speedup across the 30 Polybench kernels for the optimal P2+O3 configuration.

---

## What's Done Well

The following methodology strengths are worth preserving and highlighting in the thesis:

**Three-phase measurement separation**: Cleanly separating `jit_overhead`, `specialized_exec`, and `unspecialized` into distinct benchmark phases is the correct design. It enables independent analysis of JIT cost vs. execution benefit and supports break-even analysis. This is stronger than many published JIT papers that only report net speedup.

**Batch-calling pattern**: The redesign of UC8 kernels from per-row to per-batch invocation (spec 008 FR-007b) correctly eliminates indirect dispatch overhead from the measurement. The research finding about "minimum viable kernel duration" (~1 µs) is a concrete, publishable result arising from this design decision.

**JIT warmup separation**: `BM_jit_init_warmup` correctly identifies and separates LLVM's one-time startup cost (LLJIT creation, TargetMachine init, first IR parse: ~2400 ms cold, ~40–60 ms after warmup) from the per-kernel JIT compilation cost. This fixes the `count_matching_rows` artifact. The warmup finding is itself a thesis-worthy result.

**`best_practice_env` infrastructure**: The implementation of ASLR disabling, Turbo Boost disabling, CPU governor pinning, HyperThread sibling disabling, and CPU affinity via taskset is textbook-correct benchmarking hygiene. The fact that it is implemented and invoked through `record_benchmark.py` is good; the gap is that it is not invoked during Optuna trials (see ED-1).

**Multi-size validation**: The size-scaling study confirming that relative speedup is stable across MEDIUM/LARGE/EXTRALARGE is a strong methodological point. This rules out "only works for contrived small inputs" as a critique.

**DuckDB-based reproducible storage**: Using a structured database with views (`v_optim_best_per_kernel`, `v_ablation_medians`, `v_optim_breakeven`) rather than ad-hoc CSV files enables auditable, query-based analysis. The schema design with study names and run IDs supports longitudinal comparison across iterations.

**Negative results documented**: The failure cases (Cases 1–4 in the reflection) are documented with root causes and thesis-ready claims. The `count_matching_rows` dispatch-overhead analysis is particularly strong evidence for the system's limitations. Publishing negative results strengthens the thesis's scientific credibility.

**Break-even analysis implemented**: The Typst macro `pareto-breakeven-table` and the `v_optim_breakeven` view compute break-even call counts analytically from the Pareto front. This gives the thesis a concrete "N calls to pay off" framing for each kernel.

**Pass-trace infrastructure**: The `PassRecord` / `g_lastPassTrace` / `writePassTraceJSON` infrastructure for per-pass instruction count tracking is the right tool for claim CV-4 (speedup decomposition). It is fully implemented; it just needs to be connected to an analysis.
