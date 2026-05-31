# Thesis Scope Decisions

**Date**: 2026-05-31  
**Purpose**: Record which review findings are in-scope (must address) vs. out-of-scope
(UX/polish/future-work).  
**Thesis claims in scope**: (1) Scientific correctness, (2) Performance measurement,
(3) Generality / annotation-free design.  
**NOT a claim**: Production-readiness — thread safety, error-handling robustness, and
API completeness are engineering concerns for future work.

---

## A. Must Fix — Code Changes Required

### A-1 P2 CORRECTNESS BUG (CRITICAL — threatens correctness claim)
`JitSCCPSolver::visitLoadInst` block 2 (lines 1827–1837 of `JitSCCPSolver.cpp`) folds
**any** load whose pointer is a constant in the SCCP lattice, including loads from fields
that are written to by the function. StaticMutabilityAnalysis is not run before
`JitIPSCCPPass` in P2's main phase, so writes are not excluded. A function that stores
to a field of its argument and then reads it will use the pre-execution host-memory value
instead of the post-store value — silently wrong result.

**Fix**:
1. Add `StaticMutabilityAnalysis::StaticMutabilityAnalysisPass()` to P2 **before**
   `JitIPSCCPPass` (currently only in cleanup Phase 3).
2. Remove the unconditional `resolveConstantPtrLoad` block from `visitLoadInst`, or gate
   it on `I.hasMetadata(LLVMContext::MD_invariant_load)` — rely only on the annotation
   path for host-memory folding.

### A-2 `bool` Argument Serialization (HIGH — latent bug)
`serializeArgumentToIR` excludes `bool` from the integral branch but has no bool case.
Calling `callSpecialized<R>(&f, true)` throws at runtime. `serializeTypeToLLVM`
correctly handles `bool → i1`.

**Fix**: 1-line change — add `bool` to the integral branch, emitting `ConstantInt::get(i1, value)`.

### A-3 JIT Overhead Benchmark Includes Dylib Teardown (HIGH — inflates measurement)
`benchmarkJITOverhead` destructs the `SpecializedFunction<R>` temporary inside the timed
loop body, measuring JIT compile + `munmap`. Should measure compile only.

**Fix**: Add `->Iterations(1)->UseManualTime()` to `benchmarkJITOverhead`, consistent
with how `benchmarkJITAnalysis` already works.

### A-4 Final Thesis Measurements Must Use `--best-practices`
Optuna Phase A (config selection) and Phase B (ablation) were run without CPU pinning,
ASLR control, or Turbo Boost control. This is acceptable for config selection (ordering
of configs is noise-robust). Final speedup numbers cited in the thesis **must** come from
a `record_benchmark.py --benchmarking-best-practice` run. Verify `best_practice_full=TRUE`
in the database for all thesis-cited rows.

### A-5 Diagnostic on FuncToBlobIdx Name Collision (MEDIUM)
When two TUs define a function with the same name, `FuncToBlobIdx` silently uses the
second blob. Add a `log(Warning, ...)` when a name collision is detected at `init()` time.

---

## B. Must Fix — Thesis Text Corrections

These require no code changes but are direct factual errors a reviewer will catch.

### B-1 StaticMutabilityAnalysis story (CRITICAL)
§3.2.3 says the pass was "superseded by IPSCCP" and not implemented. Correct: the pass
was reinvestigated and found to complement stock IPSCCP in P0/P1 — it marks loads from
never-written fields as `!invariant.load` so that `InvariantLoadToConstantPass` can fold
them to host-memory constants, enabling stock IPSCCP to propagate them. P2's JitIPSCCPPass
has invariant-load resolution built in but relies on the same annotation for cleanup.

### B-2 Pipeline 0 description (HIGH)
Thesis lists 6 passes; implementation has ~15. The thesis omits all custom
`StaticMutabilityAnalysis`, `InvariantLoadToConstantPass`, `WholeProgramDevirtPass`,
`ReversePostOrderFunctionAttrsPass`, and all FPM adaptor passes. GVN is NOT a top-level
fixpoint step — it's inside a `PostInlineFPM`. Fix: name the four novel custom passes and
their roles; say "standard LLVM cleanup passes (SROA, EarlyCSE, InstCombine, etc.) run
in FPM adaptors at each fixpoint step." Fix the GVN placement.

### B-3 IRDumpingPass InternalLinkage ordering (HIGH)
Thesis lists InternalLinkage assignment BEFORE GlobalDCE. Code assigns it AFTER (so
target functions remain DCE roots during pruning). A reader implementing from the thesis
gets a broken preprocessor. Add: "Target functions must remain external during DCE to
serve as DCE roots; InternalLinkage is assigned only after DCE completes."

### B-4 Blob global visibility attributes (HIGH)
Thesis claims `protected` visibility, `dso_local`, `dllexport`. Code uses
`InternalLinkage` + `UnnamedAddr::Global`. Fix the description.

### B-5 Wrapper function name (MEDIUM)
Thesis says `@__crs_wrapper_<N>`. Actual prefix: `specialized_wrapper_<N>_<addr>` (with
`_no_opt` suffix when `Optimize=false`).

### B-6 Function name collection (MEDIUM)
Thesis credits IRDumpingPass with collecting function names. Actual: IRRewritingPass
writes them to `!crs.func_names` metadata; IRDumpingPass reads that metadata.

### B-7 Add third soundness condition (HIGH)
§3.3.1 lists two soundness conditions. Third required: "Globals initialized by static
constructors will appear zero-initialized in the JIT (constructors are erased from the
blob). Functions that depend on constructor-initialized global state must access those
globals as non-static (AvailableExternallyLinkage) so the JIT resolves them from host
memory."

---

## C. Must Address — New Data Required

### C-1 P2+O3 across full benchmark suite (HIGH)
Thesis presents P2 as an evaluated pipeline but P2 vs P0 comparison across all UC groups
is not complete. Run the full P2+O3 benchmark pass for the thesis evaluation chapter.

### C-2 Polybench results (HIGH)
Run Polybench under the optimal P2+O3 config; report geomean speedup. Frame as: "Polybench
verifies that specialization does not harm compute-bound kernels; speedup ≈ 1.0 is the
expected and desired outcome." Report results confirming no regressions.

### C-3 Confidence intervals (HIGH)
Final thesis measurement run must use `--benchmark_repetitions=5` so Google Benchmark
reports mean ± stddev. Report these in all speedup tables.

### C-4 Hardware specification section (HIGH)
Fill in the `#TODO` in §Hardware and Build Configuration with: CPU model, core count,
L1/L2/L3 cache sizes, LLVM version (21.1.0), build type (Release, NDEBUG), CMake flags,
whether `best_practice_env` was active.

---

## D. Acknowledge in Thesis — No Code Change

These go in a "Limitations and Deployment Requirements" section (one paragraph each).

| Item | Content |
|------|---------|
| TU separation requirement | "Annotation-free at the call site; specialization target functions should be in their own TU for clean blob isolation. Code that mixes kernel and application infrastructure in one TU risks JIT link failures on statically-linked symbols." |
| `--export-dynamic` | "The host binary must be linked with `--export-dynamic` (or equivalent) for `DynamicLibrarySearchGenerator` to resolve external symbols at JIT time." |
| x86-64 + Linux ELF | "The TrapUnreachable workaround, Large code model, and perf.map integration are x86-64/Linux-specific. Portability to AArch64 is possible (LLJIT supports it) but untested." |
| Constructor ordering | "Blob registration constructors (priority 65535) must fire before `init()` is called. Calling `init()` from another priority-65535 constructor produces a DumpedIRError." |
| LLJIT session bookkeeping growth | "Machine code pages are freed after each `callSpecialized` call; LLJIT's SymbolStringPool retains symbol name entries until explicitly cleared. For workloads with many repeated calls, periodically invoking `specializeOnly` with an explicit handle lifetime avoids unbounded pool growth." |
| P1 stability boundary | "P1 produces SIGSEGV under certain parameter combinations (unroll_max ≥ 27 with p1_max_module_growth ≤ 1.64). P1 is presented as experimental; the safe parameter range is: unroll_max ≤ 16, p1_max_module_growth ≥ 2.0." |
| Duplicate function names | "If two TUs compiled with the plugin define a function with the same mangled name, `FuncToBlobIdx` uses the later-registered blob. A diagnostic warning is emitted at `init()` time." |
| TPC-H impractical | "TPC-H JIT overhead exceeds 2 minutes (sqlite3VdbeExec module: 234K functions). Specialization is impractical for translation units of this size; the system is designed for per-kernel TUs." |
| Pipeline error robustness | "In the current implementation, a failing JIT optimization pass is logged and execution continues with a partially-transformed module. In production use, this path would require error propagation." |
| P2 conservatism | "P2's JitIPSCCPPass folds invariant loads annotated by StaticMutabilityAnalysis, which proves fields are never written to in the IR. Both P0/P1 and P2 are correct under the no-concurrent-modification assumption." |

---

## E. Out of Scope — Engineering / UX Polish

The following issues do NOT affect correctness claims, measurement validity, or
generality claims. They are research-prototype quality items and can be listed as
"Future Work / Engineering Improvements" or left for follow-up after the thesis.

| Item | Disposition |
|------|------------|
| BUG-001: JIT timeout data race | Out of scope — TPC-H JIT is impractical anyway |
| SPEC-001: Linear scan in prepareModuleForJIT | Out of scope — O(N) but N is tiny |
| SPEC-003: Debug-mode LLVMContext diverges from FR-008 | Update spec to acknowledge exception |
| SPEC-004: Pipeline errors silently swallowed | Out of scope — no production-readiness claim |
| SPEC-005: P2 missing AlwaysInliner (vs spec FR-010) | Update FR-010 to match impl |
| SPEC-006: `_no_opt` wrapper suffix undocumented | Update spec |
| SPEC-007: IRRewriting name filter too broad | Out of scope |
| SPEC-008: getModuleStats unsynchronized | Out of scope |
| MINOR-001: Dead `normalArgs` parameter | Out of scope |
| MINOR-002: `const_cast` → should be `mutable` | **Trivial fix** (1 line) — fix it |
| MINOR-003: `cantFail` on dylib generator | Out of scope |
| MINOR-004: Instrumentation fragility | Out of scope |
| MINOR-005: Disassembler 0xC3 heuristic | Out of scope |
| MINOR-006: `setOptions` global state mutation in benchmark | Out of scope |
| MINOR-007: V1 blob API missing from MEMORY.md | **Fix MEMORY.md** (remove the v1 claim) |
| MINOR-008: FR-007 `__clangRS` edge case | Out of scope |
| MISSING-08: specializeOrFallback silently no-ops for lambdas | Out of scope |
| MISSING-09: JIT code freed via RAII nuance | Out of scope |
| MISSING-14: Generic lambdas not supported | Out of scope |
| IG-01: best_practice verification in DB | Addressed by A-4 |
| IG-02: Binary SHA in trial context | Out of scope |
| IG-03: DuckDB text export | Out of scope |
| IG-04: Optuna convergence plot | Out of scope (50 trials is sufficient given plateau data) |
| ISSUE-09 (no CIs on Optuna trials) | Out of scope for config selection; addressed by A-4 for final numbers |
| MISSING-02: LLJIT SymbolStringPool growth | Note in D (limitations); no code fix needed |

---

## Summary: Ordered Action List

### Before writing the evaluation chapter:
1. **Fix P2 correctness bug** (A-1) — add StaticMutabilityAnalysis before JitIPSCCPPass, gate resolveConstantPtrLoad on `!invariant.load`
2. **Fix bool serialization** (A-2) — 1 line
3. **Fix benchmarkJITOverhead** (A-3) — Iterations(1) + UseManualTime()
4. **Add FuncToBlobIdx collision warning** (A-5)
5. **Run final measurements** under `--best-practices --benchmark_repetitions=5` (A-4, C-3)
6. **Run Polybench** under P2+O3 and collect results (C-2)
7. **Fix MINOR-002** (const_cast → mutable, 1 line)
8. **Fix MEMORY.md** (remove V1 API claim, MINOR-007)

### When writing the thesis:
9. **Correct thesis text** items B-1 through B-7
10. **Add hardware section** (C-4)
11. **Reframe Polybench** as regression suite (C-2)
12. **Add Pareto front** as supplementary result
13. **Remove fixpoint TODO**, replace with "typically 2–3 iterations in practice"
14. **Add Limitations section** with items from D
15. **Qualify annotation-free claim** with TU separation note
