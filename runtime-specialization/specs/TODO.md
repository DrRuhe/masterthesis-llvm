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
- **Status**: 🔄 In Progress (2026-06-10)
- **Data Needed**: Rerun ablation suite with 5 reps per config; produce final corpus study.
- **Why**: Reflection 260601 shows P0 and P2 are now within measurement noise (~2.6ms).
- **Outcome**: Study `corpus_uc_final_20260610` (9 configs × 5 reps each) — currently running.
- **Reference**: `benchmarks/reports/260610-corpus-final/run_log.txt`; partial data already in benchmarks.duckdb
- **Progress**: Configs complete: default (1.21×), o3_only (1.02×), no_prune (running). 6 more configs pending.
- **ETA**: ~60 more minutes for ablation to complete.

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

---

## Evaluation: Research Question 2 (RQ2) — Binary-Size Overhead

### RQ2-001: Measure binary-size overhead of IR dump and runtime infrastructure
- **Status**: ✅ Complete (2026-06-10)
- **Measurement**: UC1ColumnScanLowKernels.cpp: 7.9 KB → 138.5 KB without/with plugin (+1,660%)
- **Dominant cost**: 91.7 KB IR bitcode blob in .rodata (70% of overhead per TU)
- **Reference**: `benchmarks/reports/260610-binary-size/binary_size_table.txt`

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
- **Status**: ✅ Complete (2026-06-10)
- **Data Collected**: 30 kernels × SMALL+MEDIUM × default config + P2 optimal config; 1 rep each.
- **Key Finding**: polybench shows 0.93-1.10× speedup (default) and 1.0-1.16× (P2). Minimal specialization benefit because polybench kernels lack constant pointer arguments.
- **Reference**: `benchmarks/reports/260610-polybench/polybench_speedup_default.txt`, `polybench_speedup_p2_optimal.txt`
- **Prerequisite**: 
  - [x] RQ1-002 (fix polybench JIT bug)
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
- **Status**: 🔄 Partially Complete (2026-06-10)
- **Done**: Fixed `plot_pareto_configs.py` for split jit/spec rows; generated 18 PNG+CSV for `uc_optim_iter3_20260601`.
- **Pending**: Generate Pareto plots for `corpus_uc_final_20260610` (waiting for ablation to complete).
- **Reference**: `benchmarks/reports/260610-pareto/` (18 PNG+CSV files committed)

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
| 2026-06-10 | Data collection | Completed: RQ1-002 (polybench funcptr fix), RQ2-001 (binary size), RQ3-001 (polybench eval), RQ4-002 (sensitivity), RQ5-001 (break-even), RQ6-002 (TPC-H scope), INF-002/003/004. RQ1-001 corpus ablation IN PROGRESS (6/9 configs done). | `specs/PLAN.md`, `benchmarks/reports/260610-*/` |
