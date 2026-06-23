# LLVM Runtime Specializer: Thesis Completion Checklist

**Last Updated**: 2026-06-12  
**Status**: Active — tracks missing data, open questions, and completion blockers for thesis evaluation sections.

This document is the authoritative reference for high-level tasks required to complete the thesis. It is organized by thesis chapter and mapped to research questions. Sessions that discover new open tasks MUST update this file with a link to the relevant spec or reflection document.

---

## Quick Reference: Blocked vs. Unblocked Work

- 🟢 **Unblocked**: Can be started immediately.
- 🟡 **Partially Blocked**: Requires one or more prerequisites; prerequisites listed.
- 🔴 **Blocked**: Cannot start until blockers are resolved.

---

## Related Work & Background

### RW-001: Verify Nautilus performance data for comparison
- **Status**: 🟡 Partially Blocked
- **Data Needed**: Nautilus compilation latency, first-query overhead, steady-state speedup, baseline system.
- **Why**: Thesis.typ carries a TODO comment (line ~258) to extract Nautilus data for related-work comparison.
- **Outcome**: Citation-ready performance table comparing Nautilus query-compilation latency to CRS JIT latency.
- **Reference**: `docs/thesis.typ` line 258–265, `TODO[Extract Nautilus...]`
- **Prerequisite**: Nautilus evaluation paper must be located and read; may require email contact with authors.

---

## Design & Implementation Justification

### D-001: Document failed alternatives and design journey
- **Status**: 🟢 Unblocked
- **Data Needed**: Completion of design chapter writeup; currently scattered in thesis.typ comments.
- **Why**: Thesis.typ section "Design" must justify why the current specialization model and IR-dump-based approach were chosen over alternatives.
- **Outcome**: Complete narrative explaining failed alternatives (type reflection, call-site collection, `__builtin_dump_struct`) and tradeoffs.
- **Reference**: `docs/thesis.typ` line 338–399; already contains detailed content.
- **Next Step**: Copy existing content into proper Design chapter; add missing rationale for IR serialization vs. alternatives.

### D-002: Verify ARMv8 portability scope statement
- **Status**: 🟢 Unblocked
- **Data Needed**: Statement of scope: are results portable to ARM, or x86-64-specific?
- **Why**: Discussion chapter must address portability threats; currently assumed but not validated.
- **Outcome**: Explicit scope statement: "Results are x86-64 specific; ARM/RISC-V may differ due to cache hierarchy and instruction set differences."
- **Reference**: Constitution principle "Hardware Specificity"; Spec 007 FR-020; Design chapter should cite this early.
- **Next Step**: Run a polybench trial on ARM if available, or document as "out of scope for this thesis."

---

## Evaluation: Research Question 1 (RQ1) — Speedup & JIT Overhead

### RQ1-001: Finalize P0 vs. P2 pipeline comparison after !invariant.load fix
- **Status**: ✅ Complete (2026-06-10)
- **Outcome**: Study `corpus_uc_final_20260610` (9 configs × 5 reps = 45 runs, all OK)
- **Final ranking (geomean speedup, UC MEDIUM+low)**:
  1. p0_o3_optimal: 1.740× (Optuna P0 optimal: fixpoint=16, unroll=62, lmod=3)
  2. default: 1.665×
  3. o3_only: 1.659×
  4. uc_workload_optimal (P2): 1.592× (low unrolling=5 hurts grouped ops)
  5. no_o3_final: 1.453× (WORST — O3 final is critical)
- **Key finding**: P0 Optuna-optimal beats default by +4.5%; P2 does NOT outperform P0 on execution speedup.
- **Reference**: `benchmarks/reports/260610-corpus-final/speedup_summary.txt`, `benchmarks/reports/260610-pareto/`

### RQ1-002: Repair and validate PolybenchBenchmark JIT tests
- **Status**: ✅ Complete (2026-06-10)
- **Fix Applied**: Added `resolveArgFromCallers` funcptr forwarding in `IRRewritingPass.cpp`; migrated polybench_bench.cpp to NTTP template form; all 30 kernels now produce valid JIT times.
- **Validation**: correlation kernel MEDIUM: 144ms JIT overhead (non-zero, valid).
- **Reference**: Commit `ae42899c090a` (IRRewritingPass fix), commit `e8ca752365c8` (polybench NTTP migration).
- **Smoke tests added**: `test/smoke/call-specialized-forwarded-funcptr.cpp`, `speconly-forwarded-funcptr.cpp`, `speconly-std-apply-funcptr.cpp`.

### RQ1-003: Confirm break-even call counts for all UC kernels under final optimal config
- **Status**: ✅ Complete (2026-06-10)
- **Data**: Used uc_optim_iter3_20260601 best trial (P0 optimal, trial 28) with per-kernel cpu_time for JIT overhead.
- **Outcome**: 14/18 kernels break even in ≤3 calls; count_matching_rows=2015 calls (outlier: 2358ms JIT).
- **Reference**: `benchmarks/reports/260610-breakeven/breakeven_table.txt`, `breakeven_hist.png`.

### RQ1-004: Document the three failure cases for RQ6
- **Status**: 🟡 Partially Blocked
- **Data Needed**: Synthesize reflection findings on `count_matching_rows`, `column_scan`, `multi_predicate` into thesis-ready narrative with root causes.
- **Why**: These three kernels are the thesis's answer to "where does specialization break down?" Reflections document them; now need writeup.
- **Outcome**: RQ6 section with three case studies, each with root cause analysis and practical guidance.
- **Reference**: Reflection 260531 §8 "Specialization Failure Cases"; Reflection 260527 §12 "Minimum Viable Kernel Duration."
- **Prerequisite**: None — all data already collected.
- **Effort**: ~1 hour (writeup + verification against reflection documents).

### RQ1-005: Complete the thesis UC first-call quadrants rerun for `per_uc_best`
- **Status**: ✅ Complete (2026-06-21)
- **Data Collected**: Study `corpus_uc_first_call_quadrants_20260620_160209` recorded both configs (`default`, `per_uc_best`) across all intended 18 kernels, all 3 abstraction levels, and all 4 sizes under best-practice controls. Coverage is complete at 6470 phase rows / 1294 distinct tuples; the missing two tuples from the old 1296 expectation are the intentionally omitted `count_matching_rows` `abstract` `EXTRALARGE` `specialized_exec` points (one per config).
- **Why**: This unblocks regeneration of the thesis-grade RQ1 table, boxplot, and 4-quadrant figure from a single completed study without mixing partial shard studies.
- **Outcome**: `benchmarks/reporting/plot_rq1_first_call_quadrants.py` and the related RQ1 export scripts can now be rerun directly against `corpus_uc_first_call_quadrants_20260620_160209`.
- **Reference**: `benchmarks/run_thesis_uc_first_call_quadrants.sh`; study `corpus_uc_first_call_quadrants_20260620_160209`
- **Supersedes**: Partial study `corpus_uc_first_call_quadrants_20260619_153349`; report dir `benchmarks/reports/20260619_153349-thesis-uc-first-call-quadrants/`

### RQ1-006: Resolve incomplete source data for the thesis size-scaling figure
- **Status**: 🟡 Partially Blocked
- **Data Needed**: Either the missing `box_filter` `EXTRALARGE` point for the thesis source run `260517-15-04-size_scaling`, or an explicit thesis/prose decision that the figure intentionally shows only the three measured `box_filter` sizes.
- **Why**: `specs/PLAN.md` points `rq1_size_scaling.png` at `260517-15-04-size_scaling` with kernels `box_filter`, `generic_sort`, and `multi_pattern_match`, but the stored CSV currently has only 3 size rows for `box_filter` while the other two kernels have 4.
- **Outcome**: The size-scaling figure has unambiguous provenance and complete/intentional coverage for each displayed kernel.
- **Reference**: `benchmarks/reporting/plot_thesis_size_scaling.py`; `benchmarks/reports/260517-15-04-size_scaling/data.csv`; `specs/PLAN.md`
- **Prerequisite**: None if the thesis accepts the partial line; otherwise locate or regenerate the missing point.
- **Effort**: ~15-60 minutes depending on whether the missing point can be recovered from existing artifacts.

### RQ1-007: Build thesis taxonomy for "Specializeable Architectures"
- **Status**: 🟢 Unblocked
- **Data Needed**: Collect repository-backed evidence for supported and unsupported architectural patterns, then map the UC1 outlier kernels into that taxonomy without over-claiming unsupportedness.
- **Why**: `docs/thesis.typ` line 1240 currently contains only a TODO stub, but the intended subsection should answer which architectural patterns CRS technically supports and which it does not.
- **Outcome**: A drafting-ready evidence ledger and subsection plan covering supported patterns such as helper-call inlining / callback specialization / vtable devirtualization, plus unsupported patterns such as SQLite-style shared mutable state threaded through an interpreter dispatch loop.
- **Reference**: `specs/021-specializeable-architectures-taxonomy/`
- **Prerequisite**: None — most source material already exists in local specs, smoke tests, and benchmark reports.
- **Effort**: ~1-2 hours (evidence collection + claim-strength review)

---

## Evaluation: Research Question 2 (RQ2) — Binary-Size Overhead

### RQ2-001: Measure binary-size overhead of IR dump and runtime infrastructure
- **Status**: ✅ Complete (2026-06-10)
- **Measurement**: UC1ColumnScanLowKernels.cpp: 7.9 KB → 138.5 KB without/with plugin (+1,660%)
- **Dominant cost**: 91.7 KB IR bitcode blob in .rodata (70% of overhead per TU)
- **Reference**: `benchmarks/reports/260610-binary-size/binary_size_table.txt`

### RQ2-002: Analyze binary-size vs. execution speedup tradeoff
- **Status**: ✅ Complete (2026-06-12)
- **Data Collected**: Per-kernel low-abstraction UC translation-unit object sizes measured without/with the IR-dumping plugin using the release clang toolchain, joined with `corpus_uc_final_20260610` execution speedups.
- **Why**: Thesis must address whether the binary overhead is justified by speedup gains.
- **Outcome**: Scatterplot `benchmarks/reports/thesis-figures/rq2/rq2_binary_size_vs_speedup.png` plus measurement CSV `rq2_binary_size_measurements.csv`; the single-TU stacked breakdown figure lives at `rq2_binary_size_breakdown.png`.
- **Reference**: `benchmarks/reporting/measure_uc_binary_size.py`, `plot_binary_size_vs_speedup.py`, `plot_binary_size_breakdown.py`; `benchmarks/reports/thesis-figures/rq2/`

---

## Evaluation: Research Question 3 (RQ3) — Higher Abstraction Levels

### RQ3-001: Fix polybench JIT bug and rerun polybench benchmark
- **Status**: ✅ Complete (2026-06-10)
- **Data Collected**: 30 kernels × SMALL+MEDIUM × default config + P2 optimal config; 1 rep each.
- **Key Finding**: polybench shows 0.93-1.10× speedup (default) and 1.0-1.16× (P2). Minimal specialization benefit because polybench kernels lack constant pointer arguments.
- **Reference**: `benchmarks/reports/260610-polybench/polybench_speedup_default.txt`, `polybench_speedup_p2_optimal.txt`
- **Prerequisite**: 
  - [x] RQ1-002 (fix polybench JIT bug)
  - [x] Benchmark rerun completed with the fixed dedicated `PolyBenchBenchmark` binary
- **Effort**: ~1.5 hours (bug fix + recompile + run 60 trials: 30 kernels × 2 sizes × 1 rep baseline).

### RQ3-003: Reevaluate PolyBench with partial specialization and thesis appendix rationale
- **Status**: 🟡 Partially Blocked
- **Data Needed**: A best-practice default-pipeline PolyBench study using the new lambda-based partial-specialization path across `SMALL`/`MEDIUM`/`LARGE`/`EXTRALARGE`, plus the generated size-scaling plots and optional `$U_p$` summary table.
- **Why**: The previous PolyBench study specialized all benchmark-visible arguments and therefore did not model repeated calls with stable kernel state and changing inputs. The thesis needs a more realistic PolyBench evaluation and an appendix that explains the specialization choice for each kernel.
- **Outcome**: Thesis-ready RQ3 figures for amortized speedup and JIT time across input size, a conditional summary table when the data is sufficiently stable, and an appendix section listing the per-kernel specialization decision and modeled scenario.
- **Reference**: `specs/022-polybench-partial-specialization-evaluation/`, `benchmarks/run_thesis_polybench_partial_specialization.sh`, `benchmarks/reporting/export_polybench_partial_specialization.py`
- **Prerequisite**:
  - [x] Partial-specialization benchmark infrastructure implemented
  - [ ] Best-practice PolyBench study recorded in DuckDB
  - [ ] Thesis appendix section wired into `docs/thesis.typ`
- **Effort**: ~1-2 hours for the full study plus artifact generation once the build is available.

### RQ3-002: Evaluate TPC-H on CRS (if feasible)
- **Status**: ❌ SKIPPED — TPC-H JIT overhead > 2 minutes (infeasible, documented as RQ6 failure case)
- **Decision Made**: TPC-H module (255,342 instrs) is 12× too large for practical specialization.
  Documenting as RQ6 failure case is more valuable than attempting fix.
- **Outcome**: TPC-H documented in `benchmarks/reports/260610-tpch-scope/scope_statement.md`
  as RQ6 module-size failure case (answer (b)). This establishes the practical size limit.
- **Reference**: `benchmarks/reports/260610-tpch-scope/module_stats.txt`, `scope_statement.md`; RQ6-002 ✅ Complete

---

## Evaluation: Research Question 4 (RQ4) — Generalization of Pipeline Settings

### RQ4-001: Run transfer experiment: UC-optimal config on TPC-H
- **Status**: ❌ SKIPPED — TPC-H JIT overhead > 2 minutes (infeasible, see RQ6-002)
- **Decision Made**: TPC-H JIT time far exceeds 5-minute threshold → experiment skipped.
  RQ4 scope narrowed to "UC workload family" generalization question.
- **Alternative Evidence**: The `corpus_uc_p0_p2_20260601` ablation study compared p0_o3_optimal vs default
  across ALL UC sizes/abstractions. p0_o3_optimal tuned for MEDIUM/low actually performed WORSE overall
  (default: 1.71× vs p0_o3_optimal: 1.65×), demonstrating that UC-optimal settings do NOT generalize
  to other UC sizes/abstractions. This is a weaker form of the generalization question.
- **Reference**: `benchmarks/reports/260610-tpch-scope/scope_statement.md`; memory notes "TPC-H Benchmark Notes"

### RQ4-002: Run sensitivity analysis (OAT) for pipeline parameters
- **Status**: ✅ Complete (2026-06-10)
- **Approach**: Used existing `sens_uc_iter2_20260521` study (5 reps, 24 configs covering early_prune, fixpoint_max, o3_final, p1_inline_threshold, p1_max_module_growth, pipeline).
- **Finding**: o3_final = CRITICAL (+16.4% combined cost when disabled); all other parameters INSENSITIVE (<5%).
- **Reference**: `benchmarks/reports/260610-sensitivity/parameter_classification.txt`, `sensitivity_sens_uc_iter2_20260521.png`
- **Note**: unroll_max not covered in existing study; run_sensitivity.sh available for future sweep.

---

## Evaluation: Research Question 5 (RQ5) — Break-Even Analysis

### RQ5-001: Compute and visualize break-even call counts for all configs
- **Status**: ✅ Complete (2026-06-10)
- **Data**: 18 kernels from uc_optim_iter3_20260601 best trial (P0 optimal).
  - 14/18 kernels: break-even ≤ 3 calls; 1 outlier: count_matching_rows = 2015 calls
  - Summary: "78% of UC kernels achieve break-even in ≤ 3 calls; count_matching_rows requires 2015 calls."
- **Reference**: `benchmarks/reports/260610-breakeven/breakeven_table.txt`, `breakeven_hist.png`

### RQ5-002: Identify and explain outliers in break-even distribution
- **Status**: ✅ Complete (2026-06-10)
- **Finding**: count_matching_rows has 2358ms JIT overhead (vs 42-58ms for other kernels) due to cold LLVM JIT compilation of a large IR module. Root cause: JIT startup warmup not implemented for this kernel.
- **Reference**: `benchmarks/reports/260610-breakeven/breakeven_table.txt`; also documented in Reflection 260531 §6.1.

---

## Evaluation: Research Question 6 (RQ6) — Failure Cases & Boundaries

### RQ6-001: Document all specialization failure cases with root causes
- **Status**: 🟡 Partially Blocked
- **Data Needed**: Synthesis of reflected findings on failure cases into thesis-ready narrative.
- **Why**: RQ6 is the thesis's explicit answer to "where does specialization break down?"
- **Outcome**: Four documented failure cases with root causes, practical guidance, and scope implications:
  1. **JIT Startup Overhead**: `count_matching_rows` (now fixed with JIT warmup; Reflection 260531 §8 Case 1).
  2. **Fundamentally Poor Candidate**: `count_matching_rows` exec speedup still 1.01× (Reflection 260531 §8 Case 2).
  3. **Marginal Candidates**: `column_scan`, `multi_predicate` (Reflection 260531 §8 Case 3).
  4. **Dispatch Overhead (Fixed)**: UC8 kernels pre-batch-conversion (Reflection 260531 §8 Case 4).
- **Reference**: Reflection 260531 §8; Reflection 260527 §12 "Minimum Viable Kernel Duration."
- **Prerequisite**: None — all data already collected.
- **Effort**: ~1 hour (writeup + cross-reference to reflections).

### RQ6-002: Quantify module-size limitation (TPC-H case study)
- **Status**: ✅ Complete (2026-06-10)
- **Data**: sqlite3VdbeExec: 255,342 instrs / 2,017 funcs / 4,078 KB blob; 12× larger than UC MEDIUM (~21k).
- **Outcome**: TPC-H documented as RQ6 failure case. JIT overhead > 2 min under P0.
- **Reference**: `benchmarks/reports/260610-tpch-scope/module_stats.txt`, `scope_statement.md`

### RQ6-005: Input-size limits scatterplot (Spec 018)
- **Status**: ✅ COMPLETE (2026-06-11)
- **Outcome**: All 29 queries (7 synthetic + 22 TPC-H, nOps 9–230) pass in 103–117ms for both p0_optimal and p2_optimal configs. No timeout failures. Key finding: GlobalDCE prune reduces 2234-function sqlite3 module to 21 functions; nOp count does not affect JIT overhead.
- **Scatterplot**: `benchmarks/reports/260611-sqlite3-input-size/input_size_limits.png` (47 data points)
- **Script**: `benchmarks/reporting/plot_input_size_limits.py`
- **Findings documented in**: `specs/018-input-size-limits/spec.md` Clarifications (CL-001 to CL-003)
- **Reference**: `specs/018-input-size-limits/`

### RQ6-006: Fix `specializeLambda` closure-lifetime crash in UC2 tradeoff edge detection
- **Status**: ✅ Complete (2026-06-19)
- **Data Collected**: `specializeLambda` now heap-owns a decayed closure inside the returned `SpecializedLambda` handle and JITs against that owned object, so serialized closure pointers no longer refer to factory-stack storage. Added focused smoke regression coverage for factory-returned lambdas, helper-struct captures, zero-arg lambdas, and moved lambdas.
- **Why**: `AllBenchmarks` reproducibly segfaults on `BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:SMALL;t:specialized_exec`. Investigation on 2026-06-18 showed `specializeLambdaImpl` serializes the lambda closure with `serializeArgumentToIR(Builder, lambda)`, and `serializeArgumentToIR` lowers class types to `inttoptr(&value)`, i.e. the address of the live C++ closure object on the factory stack. The UC2 tradeoff kernel captures `SobelFilter sf` by value, so the specialized code ends up dereferencing stale factory-frame state after `create_edge_detection_tradeoff_specialized()` returns.
- **Outcome**: The original UC2 crash is fixed by owning the closure in the returned handle; the affected benchmark repros run without segfaults, and UC14 no longer needs the temporary `std::function`/`specializeOnly` workaround. Manual verification covered `uc2_conv/edge_detection`, `uc14_sort/struct_sort`, and representative tradeoff specialized-exec kernels from UC1, UC8, and UC12.
- **Reference**: `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h`, `test/smoke/specialized-lambda-factory-lifetime.cpp`, `benchmarks/use-cases/UC14Sort/UC14Kernels.h`
- **Effort**: ~2-4 hours (runtime fix + regression tests + UC rerun).

### RQ6-003: Verify LLJIT crash fix and document boundary conditions
- **Status**: 🟢 Unblocked
- **Data Needed**: Document the `TrapUnreachable=true` fix and `setMutableContent` assertion that motivated it.
- **Why**: This is a boundary condition that could affect portability; thesis should document it.
- **Outcome**: Explanation in Discussion or RQ6 section: "LLJIT assertion 'MutableContent.data()' occurs when [condition]; mitigation: [fix]. This may affect portability to [other LLVM versions / platforms]."
- **Reference**: Constitution "Critical Bug Fix: JITLink setMutableContent Crash"; Memory notes.
- **Prerequisite**: None — already fixed.
- **Effort**: ~30 minutes (writeup + verification against LLVM source).

### RQ6-004: Document scope limitations explicitly in Discussion
- **Status**: 🟢 Unblocked
- **Data Needed**: Thesis Discussion section must explicitly state:
  1. Single-threaded only (no concurrent mutation of specialized arguments).
  2. x86-64 only (or "tested on x86-64; ARM portability unknown").
  3. Module size cap (based on TPC-H findings).
  4. Minimum kernel duration rule (~1 µs per call).
- **Why**: Thesis claims must be scoped to tested conditions; Discussion must enumerate threats and limitations.
- **Outcome**: Discussion section with explicit scope + limitations subsection.
- **Reference**: Constitution Specialization Scope Constraint; Spec 007 Methodology Weaknesses (W1–W6).
- **Prerequisite**: RQ6-001 to RQ6-003 complete.
- **Effort**: ~1 hour (writeup + cross-reference).

---

## Evaluation: Cross-Cutting Infrastructure & Reporting

### INF-001: Generate thesis-ready Pareto plots for all studies
- **Status**: ✅ Complete (2026-06-10)
- **Done**: Fixed `plot_pareto_configs.py`; generated 18 PNG+CSV for `uc_optim_iter3_20260601` AND `corpus_uc_final_20260610`.
- **Reference**: `benchmarks/reports/260610-pareto/` (36 PNG+CSV files total)

### INF-002: Verify DuckDB schema stability and document for reproducibility
- **Status**: ✅ Complete (2026-06-10)
- **Reference**: `benchmarks/reports/260610-infra-docs/schema.sql` (7 tables, 8 views documented)

### INF-003: Create summary table of all studies (names, dates, trial counts, best configs)
- **Status**: ✅ Complete (2026-06-10)
- **Reference**: `benchmarks/reports/260610-infra-docs/study_summary.txt`

### INF-004: Document all infrastructure and measurement setup for reproducibility
- **Status**: ✅ Complete (2026-06-10)
- **Summary**: Core i9-12900H (20T, 5GHz), Clang 21.1.8, Python 3.13.13, Optuna 4.8.0, governor=powersave
- **Reference**: `benchmarks/reports/260610-infra-docs/environment.txt`

### INF-005: Build a no-assertions benchmark toolchain for thesis timing runs
- **Status**: 🟡 Partially Blocked
- **Data Needed**: A benchmark build tree where `ClangRuntimeSpecializer` and the linked `libLLVM.so` are compiled in true release mode for measurement.
- **Why**: Investigation on 2026-06-18 showed the current `llvm/build/release` tree is configured with `CMAKE_BUILD_TYPE=Release` but `LLVM_ENABLE_ASSERTIONS=ON`; the resulting `ClangRuntimeSpecializer.cpp` compile command includes `-DNDEBUG ... -UNDEBUG -D_DEBUG`, and benchmark binaries report `***WARNING*** Library was built as DEBUG. Timings may be affected.` This means the thesis timing runs are currently using debug-only CRS code paths and assertion-enabled LLVM internals.
- **Outcome**: Benchmark binaries report `library_build_type=release`, no debug warning is printed at startup, and timing studies are run from that tree.
- **Reference**: `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/llvm/build/release/compile_commands.json` entry for `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.cpp`; `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/llvm/build/release/CMakeCache.txt` (`CMAKE_BUILD_TYPE=Release`, `LLVM_ENABLE_ASSERTIONS=ON`).
- **Prerequisite**: Provision a separate LLVM build directory with `-DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_ASSERTIONS=OFF` for benchmarking, or explicitly override the CRS/benchmark targets to restore `NDEBUG` and document the remaining assertion overhead in libLLVM.
- **Effort**: ~1-2 hours to configure + rebuild benchmark-relevant targets, longer for a full clean rebuild.

### INF-006: Generate a Nix-derived devshell SBOM for the appendix
- **Status**: ✅ Complete (2026-06-12)
- **Data Needed**: Machine-generated package inventory for the thesis devshell, derived from `flake.nix` / the built devshell closure.
- **Why**: The evaluation chapter must document software provenance at appendix level, not just mention a few top-level tool versions.
- **Outcome**: Appendix-ready SBOM artifact plus a short pointer from `Hardware and Build Configuration`.
- **Reference**: `benchmarks/reporting/generate_devshell_sbom.py`; `benchmarks/reports/thesis-appendix/`; `docs/assets/devshell-sbom/`; `docs/thesis.typ`.
- **Effort**: ~30 minutes (generation script + artifact + appendix hook).

### INF-007: Rerun the final UC corpus under thesis-grade best-practice controls
- **Status**: 🔴 Blocked
- **Data Needed**: Fresh `default` / `p0_o3_optimal` / `uc_workload_optimal` / `no_o3_final` UC MEDIUM+low measurements collected with `best_practice_full=TRUE`.
- **Why**: The thesis should cite a final dataset gathered under the benchmark best-practice protocol rather than exploratory runs.
- **Outcome**: New ablation study with 5 reps per config, explicit DB provenance, and updated speedup/JIT numbers for the evaluation chapter.
- **Reference**: `benchmarks/run_thesis_uc_final.sh`; `benchmarks/ablation_benchmarks.py`; `specs/PLAN.md`.
- **Blocker**: Current environment cannot satisfy the required sudo-backed benchmark controls. Both sandboxed and escalated runs on 2026-06-12 failed to apply ASLR disable, Turbo disable, governor switch, and SMT sibling isolation, so resulting runs do not satisfy `best_practice_full=TRUE`.
- **Next Step**: Re-run on the target machine with working sudo askpass or equivalent privileged access, then refresh the final UC report directory and chapter numbers.

### INF-009: Rewrite the evaluation chapter around reader-facing RQ framing
- **Status**: 🟡 Partially Blocked
- **Data Needed**: Final prose pass that gives each RQ motivation, RQ-specific measurement description, results, and conclusion.
- **Why**: The current evaluation reads as a flat result dump and assumes implementation-internal context.
- **Outcome**: Evaluation text that is thesis-reader-facing, avoids internal spec references, and clearly explains what each RQ allows the reader to infer.
- **Reference**: `docs/thesis.typ` Evaluation section; evaluation review 2026-06-12; `specs/PLAN.md`; `benchmarks/reports/thesis-figures/`.
- **Prerequisite**: Final UC rerun numbers should be available before locking the cited absolute metrics.
- **Effort**: ~2-3 hours (prose + figure/table caption pass).

### INF-008: Rename the six presented UC benchmark families to reader-facing UC1-UC6
- **Status**: 🟢 Unblocked
- **Data Needed**: Consistent thesis-only numbering scheme and mapping from internal benchmark IDs (UC1, UC2, UC7, UC8, UC12, UC14).
- **Why**: Sparse internal IDs are meaningful to the implementation but confusing to the thesis reader.
- **Outcome**: Evaluation and implementation chapters refer to the six benchmark families as UC1-UC6, with the internal mapping explained once if needed.
- **Reference**: `specs/008-use-case-benchmarks/spec.md`; `specs/011-uc-benchmark-abstraction-variants/spec.md`; evaluation review 2026-06-12.
- **Prerequisite**: None.
- **Effort**: ~30 minutes (terminology pass).

---

## Discussion & Validity Threats

### DISC-001: Write methodology weaknesses section
- **Status**: 🟡 Partially Blocked
- **Data Needed**: Synthesize Spec 007 Methodology Weaknesses (W1–W6) into Discussion section narrative.
- **Why**: Discussion chapter must acknowledge limitations and threats to validity upfront; reader credibility depends on this.
- **Outcome**: Discussion section subsection "Methodology Weaknesses & Framing" covering: limited workload diversity, hardware specificity, Optuna trial budget, single objective, measurement noise, definition of "best."
- **Reference**: Spec 007 §"Methodology Weaknesses and Framing" (W1–W6); Reflection 260531 discusses these implicitly.
- **Prerequisite**: None — all content in Spec 007.
- **Effort**: ~1 hour (adapt Spec 007 into Discussion prose).

### DISC-002: Document validity threats and mitigation strategies
- **Status**: 🟡 Partially Blocked
- **Data Needed**: Enumerate threats (hardware noise, measurement bias, workload selection, generalization) and corresponding mitigations.
- **Why**: Thesis credibility depends on transparent acknowledgment of threats and evidence of mitigation attempts.
- **Outcome**: Discussion section subsection "Threats to Validity & Mitigation" covering each identified threat and what was done to mitigate it (≥3 reps median, documented hardware, fixed seed for reproducibility, etc.).
- **Reference**: Spec 007 Methodology Weaknesses; Constitution.
- **Prerequisite**: None.
- **Effort**: ~1 hour (synthesis + writeup).

### DISC-003: Quantify measurement noise (variance across reps)
- **Status**: 🟡 Partially Blocked
- **Data Needed**: Compute inter-run coefficient of variation (CV) for key metrics (jit_ns, spec_ns) across all repetitions in corpus study.
- **Why**: Discussion must justify the claim "measurement noise is small enough that ≥3 reps median is reliable."
- **Outcome**: Summary statistics: mean CV across UC kernels, per-kernel range (min/max CV); statement in Discussion: "Inter-run variance averaged X% (median Y%), justifying use of median across 5 repetitions."
- **Reference**: Spec 007 W5 Measurement Noise; FR-020.
- **Prerequisite**: RQ1-001 corpus study complete.
- **Effort**: ~30 minutes (SQL aggregation + statistics).

---

## Future Work & Open Questions

### FW-001: Identify concrete next-iteration improvements
- **Status**: 🟢 Unblocked
- **Data Needed**: Synthesize reflection documents (260601, 260531, 260527) into concrete next-iteration roadmap.
- **Why**: Thesis Future Work section should not be vague; it should propose concrete, evidence-grounded improvements.
- **Outcome**: Future Work section listing 3–5 concrete next steps, each tied to a reflection finding or open question.
- **Examples** (from reflections):
  - "Investigate whether early GlobalDCE can reduce `count_matching_rows` JIT time (see Reflection 260531 §6.1)."
  - "Run sensitivity analysis on P2-specific parameters to identify candidates for search-space pruning (see Reflection 260531 §4)."
  - "Implement multi-objective Optuna to map Pareto frontier more efficiently (see Spec 007 W4)."
- **Reference**: All reflection documents; Spec 007 FR-018 next-iteration scope.
- **Prerequisite**: All evaluation questions (RQ1–6) complete.
- **Effort**: ~1 hour (synthesis + writeup).

### FW-002: Propose multi-threaded specialization as future direction
- **Status**: 🟢 Unblocked
- **Data Needed**: Discuss why concurrent-argument-mutation was out of scope; sketch a potential design for thread-safe specialization.
- **Why**: Constitution explicitly constrains to single-threaded; Future Work should acknowledge this and propose a solution.
- **Outcome**: Future Work subsection on thread-safe specialization: design sketch, challenges, potential solutions.
- **Reference**: Constitution Specialization Scope Constraint; Design section Concurrent Modification.
- **Prerequisite**: None.
- **Effort**: ~45 minutes (writeup + design sketch).

### FW-003: Propose ARM and cross-architecture portability study
- **Status**: 🟢 Unblocked
- **Data Needed**: Discuss why x86-64 was chosen; propose how portability study could be conducted on ARM or RISC-V.
- **Why**: Discussion and Future Work must address architecture scope; thesis should propose a concrete next step.
- **Outcome**: Future Work subsection on cross-architecture evaluation: hypothesis (specialization benefit may vary with cache hierarchy), proposed measurement (run UC benchmarks on ARM64 Graviton or RISC-V, compare speedups), expected effort.
- **Reference**: D-002 "Verify ARMv8 portability scope statement"; Spec 007 W2 Hardware Specificity.
- **Prerequisite**: None.
- **Effort**: ~45 minutes (writeup + design sketch).

---

## Conclusion & Thesis Statements

### CONC-001: Synthesize research question findings into conclusion
- **Status**: 🔴 Blocked (Depends on all RQ evaluations)
- **Data Needed**: For each RQ (RQ1–6), distill the key finding into a 2–3 sentence statement suitable for conclusion.
- **Why**: Conclusion must summarize thesis contributions clearly; each RQ must be answered.
- **Outcome**: Conclusion section with 6 subsections (one per RQ), each stating the finding and its significance.
- **Example**: "RQ1: We demonstrate up to 3.2× execution speedup with acceptable JIT overhead (53–70ms per specialization) for database operator workloads at MEDIUM scale."
- **Reference**: Thesis.typ line 123 (TODO comment to add findings).
- **Prerequisite**: All RQ evaluations complete (RQ1–6 sections).
- **Effort**: ~2 hours (synthesis + writeup).

### CONC-002: State thesis contributions clearly
- **Status**: 🟡 Partially Blocked
- **Data Needed**: Distill system design, implementation, and evaluation into 3–5 concrete contributions.
- **Why**: Conclusion must state what was contributed to the research community.
- **Outcome**: Contributions section in Conclusion: 
  1. (System) An annotation-free runtime specialization system for C++.
  2. (Method) An iterative evaluation loop (Spec 007) for optimizing JIT pipeline parameters.
  3. (Finding) Empirical evidence that specialization achieves X–Y× speedup for runtime-stable workloads.
  4. (Finding) Documentation of specialization failure cases and practical guidance for when specialization is worthwhile.
  5. (Artifact) Open-source implementation and benchmark suite with reproducible results.
- **Reference**: Thesis.typ introduction outline; Related Work positioning.
- **Prerequisite**: All evaluations complete.
- **Effort**: ~1 hour (synthesis + writeup).

---

## Summary by Status

### 🟢 Unblocked (Start Immediately)
- D-001: Design chapter completion
- RQ1-004, RQ6-001, RQ6-003, RQ6-004: Failure case documentation
- INF-002, INF-003, INF-004: Infrastructure & reproducibility documentation
- DISC-001, DISC-002: Methodology & validity threats
- FW-001, FW-002, FW-003: Future work
- CONC-002: Contributions synthesis

### 🟡 Partially Blocked
- RQ1-001: Needs corpus study rerun (~1 hour)
- RQ1-002, RQ1-003: Depends on RQ1-001
- RQ1-004: Writeup only (data ready)
- RQ2-001, RQ2-002: Binary-size measurement (no known blocker)
- RQ3-002, RQ4-001: Depends on polybench fix
- RQ4-002: Sensitivity analysis (can start now)
- RQ5-001, RQ5-002: Depends on RQ1-001 corpus study
- RQ6-002: Depends on TPC-H investigation decision
- INF-001: Pareto plotting utility may be missing
- DISC-003: Depends on corpus study
- CONC-001: Depends on all RQs

### 🔴 Blocked (Prerequisites Required)
- RW-001: Requires Nautilus paper retrieval
- RQ1-002: Polybench JIT bug fix (2-hour diagnosis/implementation)
- RQ3-001: Depends on RQ1-002 fix
- RQ3-002: TPC-H feasibility decision required
- RQ4-001: Depends on RQ3-002 decision

---

## Priority Matrix: What to Do First

**Critical Path** (Required for thesis completion):
1. ✅ **RQ1-001**: Rerun corpus study with 5 reps (validates P0 vs. P2 conclusion)
2. ✅ **RQ1-003**: Confirm break-even calls (for RQ5 section)
3. 🟠 **RQ1-002**: Fix polybench JIT bug (opens up RQ3 evaluation)
4. 🟠 **RQ2-001**: Measure binary-size overhead (answer RQ2, currently missing)
5. 🟠 **RQ5-001**: Compute break-even distribution (answer RQ5)
6. ✅ **RQ6-001**: Write failure case narrative (answer RQ6)
7. ✅ **INF-002 to INF-004**: Documentation for reproducibility
8. ✅ **DISC-001, DISC-002**: Write Discussion section
9. ✅ **CONC-001, CONC-002**: Write Conclusion section

**Nice-to-Have** (Improve thesis but not blocking):
- RQ3-001: Full polybench evaluation (depends on fix)
- RQ4-001: Transfer experiment (depends on TPC-H feasibility)
- RQ4-002: Sensitivity analysis (valuable but secondary)
- RW-001: Nautilus comparison (enhances related work)
- FW-* : Future work section (valuable but not blocking)

---

## How to Use This Document

1. **Sessions starting evaluation work**: Consult the Priority Matrix above. Mark your target tasks as "🔄 In Progress" with session date.
2. **Sessions discovering new open tasks**: Add entries to the relevant section (organized by RQ and thesis chapter) with:
   - Unique ID (e.g., RQ2-002)
   - Status (🟢/🟡/🔴)
   - Data Needed (concise description)
   - Why (research motivation)
   - Outcome (what thesis section uses this result)
   - Reference (spec/reflection document if applicable)
   - Prerequisite list (what must be done first)
   - Effort estimate (for planning)
3. **Sessions completing tasks**: Update the task status to "✅ Complete" with completion date and link to relevant commit/analysis.
4. **Sessions identifying blockers**: Update status to 🔴 and explicitly list prerequisite tasks.

---

## Session Log

| Date | Session | Status Update | Reference |
|------|---------|---------------|-----------|
| 2026-06-10 | Initial creation | All tasks identified and prioritized | — |
| 2026-06-10 | Data collection | Completed ALL tasks in PLAN.md: RQ1-001 (corpus ablation, p0_opt=1.74×), RQ1-002 (polybench fix), RQ2-001 (binary size +1660%), RQ3-001 (polybench eval, 0.93-1.16×), RQ4-002 (sensitivity, o3_final=CRITICAL), RQ5-001 (break-even, 14/18≤3 calls), RQ6-002 (TPC-H 12× too large), INF-001/002/003/004. 25 commits, 9 report dirs. | `specs/PLAN.md`, `benchmarks/reports/260610-*/` |
