# LLVM Runtime Specializer: Thesis Completion Checklist

**Last Updated**: 2026-06-10  
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
- **Status**: 🟡 Partially Blocked
- **Data Needed**: Rerun `uc_optim_iter3_20260601` ablation suite (DEFAULT, O3Only, no-prune, no-o3-final, no-unroll, fixpoint-1, aggressive, pipeline-1, P2-optimal) with full 5 repetitions per config; produce final corpus study.
- **Why**: Reflection 260601 shows P0 and P2 are now within measurement noise (~2.6ms), reversing the 260531 finding. This is the thesis's definitive answer to "which pipeline is best."
- **Outcome**: Consensus corpus study (`corpus_uc_p0_p2_20260601_FINAL` with 5 reps each) showing median speedups per kernel; used as primary thesis table.
- **Reference**: `benchmarks/reports/260601-15-16-optimize-pipeline/reflection.md` §1, §2.
- **Blocker**: None — ready to run.
- **Effort**: ~1 hour (5 reps × 9 configs × 18 kernels = 810 benchmark runs; parallel on AllBenchmarks binary).

### RQ1-002: Repair and validate PolybenchBenchmark JIT tests
- **Status**: 🔴 Blocked
- **Data Needed**: Fix `benchmarkJITOverhead` in polybench_bench.cpp to pass `funcName` argument through `std::apply` call chain; rerun polybench JIT benchmarks.
- **Why**: Reflection 260601 §3 identifies a pre-existing bug: `specializeOnly<R>(F, opts, args...)` was called without funcName after the funcptr API migration. IRRewritingPass cannot recover funcName → runtime lookup fails.
- **Outcome**: PolybenchBenchmark binary produces valid JIT times for all 30 polybench kernels; used for RQ3 (higher abstraction level evaluation).
- **Reference**: Reflection 260601 §3; `benchmarks/polybench/polybench_bench.cpp`; API migration in spec 008.
- **Prerequisite**: 
  - [ ] Understand the `std::apply` template call chain in polybench_bench.cpp
  - [ ] Determine if funcName can be recovered from template context or must be passed explicitly
  - [ ] Implement fix and verify no polybench regressions
- **Effort**: ~2 hours (diagnosis + fix + validation).
- **Ticket**: `LINKING-ISSUES.md` exists; polybench funcptr issue should be documented there.

### RQ1-003: Confirm break-even call counts for all UC kernels under final optimal config
- **Status**: 🟡 Partially Blocked
- **Data Needed**: Run `SELECT * FROM v_optim_breakeven WHERE study_name='uc_optim_iter3_20260601'` and verify break-even counts for each kernel are plausible and within thesis claim scope.
- **Why**: RQ5 depends on this; thesis must state "kernel X breaks even after Y calls." Reflection 260531 provided these for 18 kernels; 260601 may differ slightly post-fix.
- **Outcome**: Summary table: break-even calls per kernel, organized by UC group; used to answer "when is specialization worth it?"
- **Reference**: Spec 007 Experiment D (FR-012 to FR-013); Reflection 260531 §4 per-kernel scorecard.
- **Prerequisite**: RQ1-001 must be complete (corpus study provides final exec speedups).
- **Effort**: ~15 minutes (SQL query + table formatting).

### RQ1-004: Document the three failure cases for RQ6
- **Status**: 🟡 Partially Blocked
- **Data Needed**: Synthesize reflection findings on `count_matching_rows`, `column_scan`, `multi_predicate` into thesis-ready narrative with root causes.
- **Why**: These three kernels are the thesis's answer to "where does specialization break down?" Reflections document them; now need writeup.
- **Outcome**: RQ6 section with three case studies, each with root cause analysis and practical guidance.
- **Reference**: Reflection 260531 §8 "Specialization Failure Cases"; Reflection 260527 §12 "Minimum Viable Kernel Duration."
- **Prerequisite**: None — all data already collected.
- **Effort**: ~1 hour (writeup + verification against reflection documents).

---

## Evaluation: Research Question 2 (RQ2) — Binary-Size Overhead

### RQ2-001: Measure binary-size overhead of IR dump and runtime infrastructure
- **Status**: 🔴 Blocked
- **Data Needed**: 
  1. Compile a representative UC benchmark without IRDumpingPass; measure binary size.
  2. Compile the same benchmark with IRDumpingPass; measure binary size and break down IR dump size.
  3. Link CRS runtime library; measure size of compiled runtime code (ClangRuntimeSpecializer.cpp, LLJIT wrappers, etc.).
  4. Compute overhead as: (with-CRS - without-CRS) / without-CRS.
- **Why**: RQ2 requires this measurement; currently no data collected.
- **Outcome**: Table showing per-kernel binary-size overhead (%, MB); summary: "CRS adds X% binary overhead on average."
- **Reference**: Spec 007 does not explicitly require RQ2 measurement (only RQ1, RQ3–6 covered in FR-001 to FR-019); RQ2 must be addressed in evaluation chapter.
- **Prerequisite**: None — straightforward measurement.
- **Effort**: ~1 hour (compile variants + `size` analysis + table).
- **Note**: This is a **critical gap**. Thesis introduction lists RQ2 as a research question, but no eval plan exists. Must be added to evaluation agenda.

### RQ2-002: Analyze binary-size vs. execution speedup tradeoff
- **Status**: 🟡 Partially Blocked
- **Data Needed**: Scatter plot of binary-size overhead (x-axis) vs. execution speedup (y-axis), per UC kernel.
- **Why**: Thesis must address whether the binary overhead is justified by speedup gains.
- **Outcome**: Scatterplot and summary statement: "Binary overhead is between X% and Y%; speedup ranges from A× to B×. Ratio of speedup benefit to binary cost is favorable for K/18 kernels."
- **Prerequisite**: RQ2-001 must be complete.
- **Effort**: ~30 minutes (DuckDB query + gnuplot).

---

## Evaluation: Research Question 3 (RQ3) — Higher Abstraction Levels

### RQ3-001: Fix polybench JIT bug and rerun polybench benchmark
- **Status**: 🔴 Blocked (Depends on RQ1-002)
- **Data Needed**: Complete polybench JIT overhead and specialized execution times for all 30 kernels at MEDIUM and LARGE sizes.
- **Why**: PolybenchBenchmark tests higher-level abstraction (denser operators, more complex control flow) than UC1–UC14. Essential for RQ3 claim: "Does abstraction level amplify specialization benefit?"
- **Outcome**: Polybench speedup table and comparison to UC kernels; answer: "Polybench kernels show X% higher/lower speedup than UC kernels on average."
- **Reference**: Spec 007 does not explicitly cover polybench; RQ3 evaluation must justify why it was chosen as a representative higher-abstraction workload.
- **Prerequisite**: 
  - [ ] RQ1-002 (fix polybench JIT bug)
  - [ ] Binary must be rebuilt with bug fix
- **Effort**: ~1.5 hours (bug fix + recompile + run 60 trials: 30 kernels × 2 sizes × 1 rep baseline).

### RQ3-002: Evaluate TPC-H on CRS (if feasible)
- **Status**: 🔴 Blocked
- **Data Needed**: Attempt JIT specialization on TPC-H Query 1; measure JIT time and specialized execution time.
- **Why**: TPC-H represents even higher abstraction (full query operators); testing it answers "does specialization scale to realistic database workloads?"
- **Known Issue**: Reflection 260531 §6.1 and memory notes state "JIT overhead > 2 minutes even with P0 (sqlite3VdbeExec module too large) — impractical." This is an RQ6 failure case, not a success case.
- **Outcome**: Either (a) demonstrate that TPC-H can be specialized with reasonable overhead, or (b) document it as an RQ6 failure case with explanation.
- **Reference**: Memory notes "TPC-H Benchmark Notes"; Spec 007 secondary workload mention.
- **Prerequisite**: Decision: Is TPC-H worth investigating, or should it be marked as "out of scope / documented as failure case"?
- **Effort**: ~30 minutes if skipping; ~2 hours if attempting fix.
- **Note**: **Decision Point for Thesis Scope**. If TPC-H overhead is inherent to module size, this is valuable RQ6 finding. If it can be fixed, it's valuable for RQ3. Current hypothesis: it cannot be fixed without extracting the specific query from sqlite3VdbeExec — out of scope.

---

## Evaluation: Research Question 4 (RQ4) — Generalization of Pipeline Settings

### RQ4-001: Run transfer experiment: UC-optimal config on TPC-H
- **Status**: 🟡 Partially Blocked
- **Data Needed**: Apply UC-optimal config (from RQ1-001) to TPC-H; measure combined cost (jit_ns + spec_ns); run Wilcoxon rank-sum test comparing UC-optimal vs. Default on TPC-H.
- **Why**: RQ4 asks "Do UC-optimal settings generalize to other workloads?" Transfer experiment is the direct answer.
- **Outcome**: 
  - p-value from Wilcoxon test; if p ≤ 0.05 and ratio > 1.05, conclude "UC-optimal does not generalize."
  - If p > 0.05 or ratio ≤ 1.05, conclude "UC-optimal generalizes (or no significant difference detected)."
  - Thesis statement for RQ4: "Configuration generalization depends on workload properties. UC-optimal settings [do/do not] generalize to TPC-H with significant impact."
- **Reference**: Spec 007 Experiment C (FR-009 to FR-011); User Story 3.
- **Prerequisite**: 
  - [ ] RQ3-002 must inform feasibility (if TPC-H JIT is impractical, this experiment is too)
  - [ ] UC-optimal config must be finalized (RQ1-001)
- **Effort**: ~2 hours if TPC-H is viable; likely infeasible if TPC-H module is too large.
- **Decision Point**: If TPC-H JIT time > 5 min, this experiment should be skipped and RQ4 scope narrowed to "UC workload family."

### RQ4-002: Run sensitivity analysis (OAT) for pipeline parameters
- **Status**: 🟡 Partially Blocked
- **Data Needed**: Vary each of 6 parameters (fixpoint_max, unroll_max, large_module_max, early_prune, o3_final, pipeline) across full range while holding others at UC-optimal; record combined cost per sweep point.
- **Why**: RQ4 must identify which parameters are critical vs. insensitive; sensitivity analysis is the measurement.
- **Outcome**: Classification of all 6 parameters as "critical" (>10% cost change), "moderate" (5–10%), or "insensitive" (<5%); used to justify future search-space pruning.
- **Reference**: Spec 007 Experiment E (FR-014 to FR-016); Reflection 260531 Parameter Importance (§4) already exists.
- **Prerequisite**: None — can start immediately on last corpus study.
- **Effort**: ~1.5 hours (6 params × 10 sweep points × 5 kernels ≈ 300 trials; parallel on AllBenchmarks).

---

## Evaluation: Research Question 5 (RQ5) — Break-Even Analysis

### RQ5-001: Compute and visualize break-even call counts for all configs
- **Status**: 🟡 Partially Blocked
- **Data Needed**: For every (study, group) pair from Experiments A and B, compute break-even calls and visualize distribution.
- **Why**: RQ5 asks "how many calls are needed to break even on overhead?" This is the direct measurement.
- **Outcome**: 
  - Histogram of break-even calls across all UC groups and configs.
  - Scatter plot of module instruction count vs. break-even count.
  - Summary: "X% of UC kernels achieve break-even in ≤ 10 calls; Y% require > 100 calls."
- **Reference**: Spec 007 Experiment D (FR-012 to FR-013); Reflection 260531 §4 per-kernel scorecard.
- **Prerequisite**: RQ1-001 (corpus study must be complete).
- **Effort**: ~1 hour (SQL aggregation + gnuplot).

### RQ5-002: Identify and explain outliers in break-even distribution
- **Status**: 🟡 Partially Blocked
- **Data Needed**: For any kernel with break-even > 1000 calls, investigate root cause.
- **Why**: Outliers may indicate specialization candidates where benefit is marginal; thesis must explain these.
- **Outcome**: Case studies of 2–3 high break-even kernels; explanation tied to RQ1-004 failure cases.
- **Reference**: Reflection 260531 §6.1 (`count_matching_rows` with 25469 break-even calls) is the archetype.
- **Prerequisite**: RQ5-001 complete.
- **Effort**: ~1 hour (analysis + documentation).

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
- **Status**: 🟡 Partially Blocked
- **Data Needed**: Document why TPC-H JIT time exceeds 2 minutes; measure if a smaller query or extracted submodule can be specialized.
- **Why**: Module size is a known scalability boundary; TPC-H is the thesis's evidence for this limitation.
- **Outcome**: Explicit scope statement: "Specialization is practical for modules ≤ X instructions (based on UC MEDIUM = 21k instrs); TPC-H with Y instructions exceeds this threshold."
- **Reference**: Memory notes "TPC-H Benchmark Notes"; Reflection 260531 §6.1.
- **Prerequisite**: RQ3-002 decision (is TPC-H feasibility being investigated?).
- **Effort**: ~30 minutes (measurement + documentation) if investigating; 0 if skipping.

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
- **Status**: 🟡 Partially Blocked
- **Data Needed**: Run `plot_pareto_configs.py` (or implement if missing) for each optimizer study; produce PNG + CSV in thesis-friendly format.
- **Why**: Spec 007 FR-002b requires thesis-ready Pareto reporting; plots must be directly includable in thesis.
- **Outcome**: Set of Pareto plots (one per UC group per study) showing frontier with Default marked; CSVs suitable for pgfplots inclusion in thesis.
- **Reference**: Spec 007 Experiment A (FR-002, FR-002b); User Story 6.
- **Prerequisite**: None — utility function should exist or be straightforward to implement.
- **Effort**: ~2 hours (implement script if missing + run on all studies + format for thesis).
- **Note**: Reflection 260601 mentions this script missing; check `benchmarks/reporting/`.

### INF-002: Verify DuckDB schema stability and document for reproducibility
- **Status**: 🟢 Unblocked
- **Data Needed**: Document DuckDB schema version, table structure (v_ablation_medians, v_optim_best_per_kernel, v_optim_breakeven, optim_trial_params); store alongside thesis.
- **Why**: Thesis claims must be reproducible; schema must be stable and documented.
- **Outcome**: Schema documentation in thesis appendix; reproducibility statement: "All results stored in benchmarks/benchmarks.duckdb schema version X; queries provided in appendix."
- **Reference**: Spec 007 FR-021.
- **Prerequisite**: None.
- **Effort**: ~30 minutes (schema documentation + test queries).

### INF-003: Create summary table of all studies (names, dates, trial counts, best configs)
- **Status**: 🟢 Unblocked
- **Data Needed**: Compile metadata for all studies: `uc_optim_iter*`, `ablation_uc_iter*`, `corpus_uc_*`, `sens_uc_*`.
- **Why**: Thesis must document which studies were run, when, and what they found; a summary table provides easy reference.
- **Outcome**: Table in thesis Appendix: study name, date, phase, trial/rep count, best config, best combined cost.
- **Reference**: Spec 007 FR-007.
- **Prerequisite**: None.
- **Effort**: ~30 minutes (SQL aggregation + table formatting).

### INF-004: Document all infrastructure and measurement setup for reproducibility
- **Status**: 🟢 Unblocked
- **Data Needed**: Hardware spec, compiler version, LLVM build flags, Python environment (Optuna version, pandas, etc.), CPU frequency scaling settings.
- **Why**: Spec 007 FR-020, FR-021 require this; Discussion section must document threats to validity related to measurement setup.
- **Outcome**: Reproducibility statement in thesis Appendix: "Experiments run on [hardware]; compiled with [compiler]; Optuna X.Y, Python Z.W; CPU frequency scaling [enabled/disabled]."
- **Reference**: Constitution Evaluation Machine table; Spec 007 FR-020–FR-021.
- **Prerequisite**: None.
- **Effort**: ~30 minutes (data collection + table formatting).

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
