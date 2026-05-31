# Design Tradeoffs Audit

**Date**: 2026-05-31  
**Scope**: Full system — serialization, pipeline design, scope/generality, correctness,
evaluation methodology.  
**Sources**: `thesis.typ`, `constitution.md`, specs 003/007/012/014/015, pipeline
implementations (`JITPipelineInlining.cpp`, `JITPipelineFuncSpec.cpp`), analysis reports
in `benchmarks/reports/`.

---

## Summary

Nine tradeoffs are CRITICAL or HIGH severity because they are load-bearing for the thesis
argument and are either undocumented, unjustified, or contradict each other in the current
draft.  The most important are:

1. **Shallow serialization + inttoptr** — the core correctness assumption is stated but
   the constraint it imposes on what CAN be specialized (deeply nested graphs, moved
   pointers) is only half-articulated.
2. **No static-initializer re-execution** — mentioned only in implementation notes, not
   in the thesis correctness section.
3. **JIT overhead ~40–60 ms for small kernels; 1+ s for Polybench** — the break-even
   analysis is called for in the thesis draft but not yet written; several research
   questions remain unanswered (see `questions.md`).
4. **Single-objective Optuna** — the optimizer's scalar metric collapses the
   JIT-overhead/speedup tradeoff, and the chosen objective assumes exactly one call
   pays back the JIT cost.  This is flagged as W6 in spec 007 but not yet addressed.
5. **P1 SIGSEGV under specific parameter cross-products** — an active correctness
   defect in a publicly claimed pipeline variant; not mentioned in the thesis.
6. **Polybench specialization frequently provides no execution speedup** — the thesis
   draft claims specialization is beneficial but the evidence for compute-bound kernels
   is weak.

---

## Tradeoffs

### T-01: Shallow Serialization (inttoptr) vs. Deep Copy

- **Values in tension**: Correctness/simplicity vs. optimization depth
- **Current choice**: Pointer arguments are passed as `inttoptr(i64 hostAddress)` IR
  constants.  The JIT module reads live host memory.  No copy, no write-back.
- **Justification**: Three earlier approaches (RTTI reconstruction, stack promotion with
  write-back, static mutability analysis) were each abandoned because they were fragile,
  required complex write-back logic, or were superseded by IPSCCP.  The current approach
  is correct under the same-address-space / no-concurrent-modification assumptions that
  any specialization scheme requires.  The thesis chapter "Argument Serialization:
  Approaches Tried" gives a thorough narrative.
- **Documented in thesis**: YES — Design chapter, §Shallow Serialization (Current
  Approach).  Limitations section names the two correctness conditions explicitly.
- **Scientific concern**: The thesis correctly states both conditions but does not
  fully characterize what cannot be specialized as a consequence.  Specifically:
  (a) a pointer that is reallocated between `specializeOnly` and the specialized call
  produces silently wrong results (the baked-in address is stale); (b) an object passed
  by value where the IR function takes a pointer-to-local is treated as an address,
  so IPSCCP propagates the address but loads from it in the specialized function will
  read the caller's original stack frame — undefined behaviour if the frame is gone.
  These are not merely "future work"; they are observable failure modes for certain
  call patterns, and the thesis should either enumerate them explicitly or state why
  they cannot arise in the evaluated benchmarks.
- **Severity**: HIGH

---

### T-02: No Re-execution of Static Initializers

- **Values in tension**: Correctness vs. simplicity
- **Current choice**: `IRDumpingPass` erases `llvm.global_ctors` and
  `llvm.global_dtors` from the JIT blob at compile time.  Static initializers have
  already run in the host process; they must not re-run in the JIT module (which would
  execute constructors a second time with side effects).
- **Justification**: Correct for all evaluated benchmarks, where global state is either
  absent or fully initialized before `init()` is called.  The cost is that JIT-visible
  globals must get their initial values from host memory (via `AvailableExternallyLinkage`
  and `DynamicLibrarySearchGenerator`) rather than from the blob.
- **Documented in thesis**: PARTIALLY — IRDumpingPass section (§IRDumpingPass, step 2)
  mentions erasure of ctors/dtors.  The *correctness justification* — why this is safe
  and what would break if the assumption is violated — appears only in MEMORY.md
  (critical bug fix notes) and the polybench `static` variable bug.  The thesis
  correctness section (§Correctness) does not mention this assumption at all.
- **Scientific concern**: The correctness section lists two soundness conditions for
  shallow serialization but misses a third: global state must be fully initialized
  before specialization, or the JIT will see null/uninitialized values for globals that
  were set by constructors.  This tripped up the polybench benchmarks (the `static`
  global bug) and required a non-trivial fix.  The thesis should include this as a
  third soundness condition.
- **Severity**: HIGH

---

### T-03: P0 Fixpoint Inlining — Quality vs. Code-Size Explosion

- **Values in tension**: Specialization quality vs. module/code size
- **Current choice**: Inline any call site where at least one argument is a compile-time
  constant (`ConstantArgAlwaysInlinePass`), iterate to fixpoint.  On small kernels this
  collapses the wrapper to a handful of instructions.  On large TUs (sqlite3 amalgamation,
  DuckDB) it explodes the module: sqlite3 grows from 234K to 3M+ instructions.
- **Justification**: P0 is the default for small-to-medium kernels where the thesis use
  cases live.  P1 and P2 were designed explicitly to address the large-module failure mode.
  The three-pipeline architecture is the documented response to this tradeoff.
- **Documented in thesis**: YES — §JIT Pipeline Design Rationale names the code-size
  explosion as P0's failure mode.  §Pipeline 0 describes the pass sequence.
- **Scientific concern**: The thesis claims P0 is the default, but empirical data shows
  P0 does NOT consistently win over simpler approaches (O3Only) on several UC groups.
  For uc2_conv, uc8_ivm, uc12_groupby, and all Polybench kernels, exec speedup is near
  1.0 or slightly below.  The thesis must be careful not to claim P0 provides speedup
  across the board; the data shows it is workload-dependent.  The section on P0 should
  note this limitation explicitly.
- **Severity**: MEDIUM

---

### T-04: P0 Fixpoint Convergence — Is the Iteration Cap Principled?

- **Values in tension**: JIT overhead vs. specialization quality (diminishing returns)
- **Current choice**: `MaxFixpointIterations` defaults to 5 (from `Options::Default()`).
  The loop also exits early if instruction count is unchanged between iterations.
- **Justification**: The Optuna optimizer found `fixpoint_max=7` as the best value for
  the UC workloads in iter-2/iter-3 optimization runs.  The instruction-count convergence
  check provides a principled early-exit criterion.
- **Documented in thesis**: PARTIALLY — The pipeline section describes the fixpoint loop
  but does not discuss how the default iteration cap was chosen or whether the loop
  converges before hitting the cap in practice.  The thesis draft has a TODO: "How many
  fixpoint iterations are typically needed?"
- **Scientific concern**: Research question Q5 in `questions.md` explicitly identifies
  this as unanswered: "Is the fixpoint loop actually converging before the iteration cap?"
  The pass-trace infrastructure (`writePassTraceJSON`) can answer this but the analysis
  has not been done.  The thesis must answer Q5 before claiming the fixpoint loop is
  efficient, or must frame it as a known limitation.
- **Severity**: HIGH

---

### T-05: P1 Budget-Aware Inlining — Budget Derivation

- **Values in tension**: Specialization quality vs. code-size growth bound
- **Current choice**: P1 uses LLVM's cost-based inliner with threshold
  `CRS_DEFAULT_P1_INLINE_THRESHOLD` (default: 225, matching LLVM O3).  A separate
  module-size growth cap (`CRS_DEFAULT_P1_MAX_MODULE_GROWTH`, default 2.0x) terminates
  the fixpoint loop if exceeded.
- **Justification**: LLVM's O3 inline threshold is a well-established default for
  general-purpose code.  The 2.0x growth cap prevents runaway specialization.  Both are
  configurable via env vars and swept by the Optuna optimizer.
- **Documented in thesis**: PARTIALLY — The budget threshold is mentioned in the spec
  (spec 014 FR-003/FR-004) and in the pipeline chapter, but the *scientific justification*
  for the 225 default (why O3's threshold is appropriate for a JIT context) is absent.
  The thesis does not explain whether the 225 threshold was chosen because it works well,
  or because it was convenient to reuse LLVM's default.
- **Scientific concern**: The P1 pipeline has an active correctness issue: OAT sweep runs
  with `pipeline=1, unroll_max=27, p1_max_module_growth=1.64` crashed with SIGSEGV
  (100% rate, as documented in Q3 of `questions.md`).  This is not mentioned anywhere in
  the thesis draft.  If P1 is presented as a valid pipeline alternative, this defect must
  be acknowledged.  Additionally, P1 helped uc14_sort (+17%) but hurt uc2_conv and
  uc8_ivm (−37 to −46%) — the conditions under which P1 is beneficial are not yet
  characterized, making it difficult to give users guidance on when to choose it.
- **Severity**: CRITICAL (because of the SIGSEGV; the rest is HIGH)

---

### T-06: P2 JIT-IPSCCP — Convergence Without Outer Fixpoint

- **Values in tension**: Completeness of propagation vs. JIT overhead
- **Current choice**: P2 runs `JitIPSCCPPass` once with JIT-specific extensions
  (invariant-load folding, vtable devirtualization).  No outer fixpoint loop.  The solver
  converges internally.
- **Justification**: IPSCCP's sparse conditional constant propagation algorithm converges
  in a single pass over the program (it is already a fixpoint algorithm internally).  An
  outer loop is not needed for the types of constant propagation P2 targets.
- **Documented in thesis**: YES — §Pipeline 2 states "Pipeline 2 converges internally
  within a single IPSCCP invocation — no outer fixpoint loop is needed."
- **Scientific concern**: The claim that P2 converges "reliably" has not been validated
  across all UC benchmarks.  The empirical results (`FR-001` in spec 015: "SC-005: JIT
  overhead within 2× of P0") are stated as success criteria but the benchmark data
  comparing P2 to P0 across all UC groups is not fully reported in the thesis draft.
  The statement that P2 is "at least as good as P0 on virtual dispatch patterns" is a
  design claim, not yet an empirical finding in the thesis.  The evaluations in
  `260527-*` reports only cover a subset of benchmarks.
- **Severity**: HIGH

---

### T-07: O3 Final Pass — JIT Overhead vs. Code Quality

- **Values in tension**: JIT compilation time vs. specialized execution quality
- **Current choice**: `EnableO3Final` (default: true based on the Optuna winner config
  `o3_final=1`) runs LLVM's full O3 pipeline after the custom fixpoint loop.  This is
  the costliest step and can double JIT overhead on large modules.
- **Justification**: The Optuna optimization found that O3 final is among the most
  important parameters for execution quality.  Without it, standard transformations like
  vectorization, loop optimizations, and peephole cleanup are missing.
- **Documented in thesis**: PARTIALLY — The O3 final step is listed in the pipeline
  description.  The tradeoff is not explicitly discussed.  The thesis has a TODO asking
  for key findings from the pipeline analysis but the actual data showing O3 final's
  marginal contribution is not yet presented.
- **Scientific concern**: The ablation study was designed to test `no_o3_final` (spec 007
  FR-005) but the results are not yet in the thesis.  The Optuna parameter importance
  data (available in `importance_*.json` files) should be cited.  Spec 007 FR-004b
  requires this data to be extracted and reported.
- **Severity**: MEDIUM

---

### T-08: Speculative Devirtualization — Correctness Risk

- **Values in tension**: Specialization quality (eliminating vtable dispatch) vs.
  semantic correctness if the vtable is replaced at runtime
- **Current choice**: P0 (`DevirtualizeConstantVtableCallsPass`) and P2
  (`JitSCCPInstVisitor::visitCallBase`) read vtable function pointers from host memory
  at JIT compilation time and specialize on them.
- **Justification**: The thesis correctly identifies this as speculative.  The assumption
  holds for the common C++ pattern of a fixed concrete type at construction.  It would be
  violated by replacing a vtable pointer manually or constructing a new object at the
  same address with a different type — both of which are undefined behavior in standard
  C++.
- **Documented in thesis**: YES — §Validity of Speculative Devirtualization.  The
  justification is scientifically sound.
- **Scientific concern**: The thesis argument is correct but incomplete: it only considers
  the case of a vtable replaced by UB.  A valid scenario where the assumption breaks is
  *object reuse via placement new*: if the same memory address is used for an object of
  type A followed by an object of type B, and both objects are specialized against, the
  second specialization would be correct but the first (if reused) would not.  This is
  not UB.  The thesis should note this edge case under "Edge Cases" in the correctness
  section.
- **Severity**: MEDIUM

---

### T-09: TrapUnreachable = true — ABI Change for UB Code

- **Values in tension**: Preventing JITLink crashes (via 0-byte .text sections) vs.
  changing behavior of code that reaches `unreachable` instructions
- **Current choice**: LLJIT is initialized with `TrapUnreachable=true`, causing all
  `unreachable` instructions to emit a `ud2` trap instruction rather than 0 machine bytes.
- **Justification**: Without this, fully-optimized wrappers whose bodies collapse to
  `unreachable` (e.g., because internal globals were null-initialized in the JIT clone)
  produce 0-byte .text sections that crash JITLink's `setMutableContent`.  This is the
  bug documented in MEMORY.md in detail.
- **Documented in thesis**: PARTIALLY — `FR-003` in spec 003 requires this and the
  `.cpp` comment explains the crash.  The thesis implementation section mentions the
  LLJIT configuration but does not explain why `TrapUnreachable=true` is a forced
  global setting rather than an option.  Most importantly: this setting affects *all*
  JIT-compiled code, not just wrappers that would otherwise be 0-byte.  The thesis does
  not discuss whether this could affect the semantics of any benchmark.
- **Scientific concern**: For all benchmarks in the evaluation, `unreachable` in the
  specialized code means the constant propagation determined the code path is dead.
  Trapping on dead code is semantically harmless.  However, the thesis should state this
  explicitly: "All `unreachable` instructions in the evaluated benchmarks represent dead
  code paths proven dead by constant propagation; trapping on them cannot be reached and
  does not affect benchmark results."
- **Severity**: LOW

---

### T-10: Per-TU Blob vs. Whole-Program IR

- **Values in tension**: JIT performance (cloning only relevant TU) vs. cross-TU
  optimization opportunity
- **Current choice**: Each TU compiled with the plugin registers its own bitcode blob.
  At JIT time, only the TU containing the target function is cloned.  Cross-TU inlining
  is not performed.
- **Justification**: Cloning a merged multi-TU module for each specialization call would
  be catastrophically slow for large binaries.  The per-TU design makes JIT overhead
  proportional to the complexity of the target function's TU, not the entire program.
  The thesis § "IR Blob Size and Per-TU Indexing" justifies this explicitly.
- **Documented in thesis**: YES — §IRDumpingPass and §IR Blob Size and Per-TU Indexing.
- **Scientific concern**: The limitation this creates — the JIT cannot inline functions
  from other TUs — is not discussed.  For the UC benchmarks, kernels are deliberately
  separated into a single TU (the "Benchmark TU Separation Pattern"), so cross-TU
  inlining is not needed.  But for a user who writes a kernel spanning multiple TUs, the
  system would see only declarations for external functions and leave them as indirect
  calls.  The thesis should note this as a usage constraint (single-TU requirement for
  optimal specialization) and explain the workaround (link kernels into a single TU).
- **Severity**: MEDIUM

---

### T-11: Module Clone per Call — Memory vs. Correctness

- **Values in tension**: Memory usage and clone time vs. correctness isolation
- **Current choice**: Every `specializeOnly` or `callSpecialized` call clones the target
  TU's `BlobModule` into a fresh working copy.  The original `BlobModule` is never
  modified.
- **Justification**: The JIT optimization pipeline modifies the working copy heavily
  (linkage changes, inlining, GlobalDCE, etc.).  If the original blob module were used
  directly, the second specialization call would start from an already-modified module.
  The clone ensures idempotency.
- **Documented in thesis**: YES — §Debug vs. Release Context Handling discusses the
  debug-mode variant (re-parsing from bitcode); the release-mode `CloneModule` path is
  mentioned in the architecture overview.  The Threats to Validity section notes "module
  cloning scales with blob size."
- **Scientific concern**: The Threats to Validity mention is the right place.  However,
  the thesis should also note that this is *not* the only strategy: blob-level caching
  of optimization results (keyed on the specialization arguments) would eliminate
  redundant JIT overhead for repeated calls with the same arguments.  This is mentioned
  as a possible optimization nowhere in the thesis.
- **Severity**: LOW

---

### T-12: Single-Threaded Specialization Assumption

- **Values in tension**: Correctness of specialization vs. usability in concurrent
  programs
- **Current choice**: The system assumes no concurrent modification of specialized
  arguments during the `callSpecialized` / `specializeOnly` call.  `init()` must be
  called from a single thread before concurrent use.  The system provides no mechanism
  to detect violations.
- **Justification**: Evaluating specialization correctness in the presence of concurrent
  writes would be semantically undefined in any specialization scheme.  The thesis
  frames this as an out-of-scope concern with a clear rationale.
- **Documented in thesis**: YES — §Concurrent Modification and §Limitations §Concurrent
  Modification.  The design chapter even notes this is the same assumption in all
  related work.
- **Scientific concern**: None for the benchmark results.  One minor gap: the thesis
  notes that "Authors of other approaches do not discuss this explicitly" — but does not
  say why this is *more* surprising for the annotation-free approach.  The annotation-
  free design makes it easy for a programmer to forget that specialization is happening
  at all, increasing the risk of accidental concurrent writes.  This is partially
  addressed in the Limitations section but could be strengthened.
- **Severity**: LOW

---

### T-13: Serialization Scope — What Cannot Be Specialized

- **Values in tension**: Ease-of-use ("annotation-free") vs. silent degradation for
  unsupported argument types
- **Current choice**: Integer, floating-point, pointer, and class (by address) types are
  serializable.  Structs by value, arrays, references, non-trivial class types,
  `std::function`, and `std::vector` are not.  Unsupported types throw
  `ClangRuntimeSpecializerArgSerializationError`.
- **Justification**: A reasonable pragmatic boundary.  The most common arguments to
  JIT-specialized kernels are pointers and scalars; complex types would require deep copy
  infrastructure that was explicitly abandoned.
- **Documented in thesis**: PARTIALLY — FR-016 in spec 003 enumerates serializable types
  and the unsupported-type error.  The thesis API description mentions this constraint
  only in passing ("Serialization handles only integer, floating-point, pointer, and
  class types").  The thesis should explicitly list what cannot be specialized and why,
  so readers understand the annotation-free claim's scope.
- **Scientific concern**: The "annotation-free" claim is the thesis's primary
  differentiator.  A reviewer will ask: "What happens if I pass a `std::vector<int>` as
  an argument?"  The answer (runtime exception) must be clearly stated.  This should be
  part of the Limitations section.
- **Severity**: HIGH

---

### T-14: Single-Architecture Evaluation

- **Values in tension**: Generality of results vs. evaluation cost
- **Current choice**: All benchmarks run on a single i9-12900H (Alder Lake) machine,
  x86-64 Linux.  ARM/AArch64 and RISC-V are untested.
- **Justification**: Acceptable for a thesis; generality beyond x86-64 is correctly
  identified as future work.
- **Documented in thesis**: PARTIALLY — The TODO in §Hardware and Build Configuration
  and the Threats to Validity section acknowledge single-architecture evaluation.
  Spec 007 FR-020 documents the evaluation machine.  The thesis does not yet have the
  evaluation hardware section written.
- **Scientific concern**: None for the core argument.  However, the specific
  measurement threat from the i9-12900H's hybrid architecture (P-cores and E-cores with
  different frequencies) is documented in spec 007 FR-020 but not yet in the thesis.
  The OS scheduler may migrate benchmark threads mid-run.  CPU affinity pinning (`taskset
  -c 0-11`) is recommended in the spec but not confirmed as applied.  This matters for
  reproducibility and the JIT overhead measurements in particular.
- **Severity**: MEDIUM

---

### T-15: Polybench — Specialization Provides Minimal Speedup for Compute-Bound Kernels

- **Values in tension**: Demonstrating specialization benefit vs. choosing benchmarks
  where benefit is guaranteed
- **Current choice**: Polybench/C 4.2.1 (30 kernels) is used alongside the UC
  benchmarks.  The thesis claims these "evaluate specialization on pure computational
  kernels with well-understood performance characteristics."
- **Justification**: Polybench is a standard benchmark for numerical kernels and provides
  broad coverage.  Including it alongside synthetic UC benchmarks strengthens external
  validity.
- **Documented in thesis**: PARTIALLY — §Benchmark Suites describes Polybench.  The
  thesis does not discuss the expected specialization benefit for compute-bound loops.
- **Scientific concern**: For purely compute-bound loops (Polybench is dominated by
  nested array loops with simple arithmetic), specializing on array size parameters bakes
  in the loop bounds.  This enables loop unrolling and vectorization, but the -O3
  baseline already applies these optimizations when bounds are known at compile time.
  Preliminary data (pass-trace files from `260507-162137-analysis/`) shows JIT analysis
  successfully runs on all 30 kernels, but the actual execution speedup data for
  Polybench is not yet in the thesis.  Given that the UC benchmarks (which have
  control-flow and indirect-dispatch patterns) are where specialization is most
  valuable, the thesis should explicitly frame Polybench as a "control" suite that tests
  whether specialization causes regressions on compute-bound code, not as a suite that
  is expected to show large speedups.
- **Severity**: MEDIUM

---

### T-16: Synthetic Use Cases (UC1–UC14) vs. TPC-H

- **Values in tension**: Controlled validity (UC benchmarks designed to exhibit
  specialization benefit) vs. ecological validity (TPC-H as a real workload)
- **Current choice**: 6 UC groups (SQL expression, convolution, DFA, IVM, GROUP BY,
  sort) form the primary evaluation workload.  TPC-H is used for cross-workload transfer
  only (spec 007 Experiment C), but the experiment was incomplete (SIGSEGV from a
  debug-mode binary in the release benchmark).
- **Justification**: The UC benchmarks are designed to exercise patterns where
  specialization is expected to help (virtual dispatch, loop bound constants, comparator
  inlining).  TPC-H provides a real-workload sanity check.
- **Documented in thesis**: PARTIALLY — §Benchmark Suites describes both.  The thesis
  RQ section calls for TPC-H results but they are marked TODO.  The incomplete TPC-H
  transfer experiment is documented in `questions.md` Q4 as "unanswered."
- **Scientific concern**: The TPC-H cross-workload transfer experiment is a stated thesis
  research question (SQ4 in spec 007).  Without it, the thesis cannot answer whether the
  optimal UC pipeline generalizes.  The SIGSEGV was caused by a debug-mode runtime
  embedded in the release binary — a fixable infrastructure bug.  This needs to be
  resolved and the experiment completed before the thesis is submitted.
- **Severity**: CRITICAL

---

### T-17: Abstraction Variant Levels (L1/L2/L3) — Scientific Purpose

- **Values in tension**: Testing a specific hypothesis ("more abstract code benefits more
  from specialization") vs. multiplying evaluation effort without clear expected outcome
- **Current choice**: Each UC group has three abstraction levels: low-level (manual
  inlining, no virtual dispatch), tradeoff (policy-based templates), and abstract
  (virtual dispatch, `std::function`).  Three benchmark variants per UC × three levels
  = 54 configurations.
- **Justification**: The central hypothesis of RQ3 is that higher-abstraction code gives
  the JIT more overhead to eliminate.  The three-level design tests this hypothesis in
  a controlled way: all three levels implement the same algorithm with the same data.
- **Documented in thesis**: YES — §Benchmark Suites and §RQ3 state the hypothesis and
  the design.
- **Scientific concern**: The hypothesis is scientifically well-motivated and the
  evaluation design is sound.  The concern is that for cases where specialization
  produces no speedup (low-abstraction conv/ivm), the abstraction level comparison
  becomes undefined: is the "abstract" level better or worse?  If the abstract level
  has virtual dispatch overhead and the JIT can't fully devirtualize it (due to P0
  not handling it, or P2 not yet having benchmark data), the result could be *worse*
  than the low-level version.  The thesis should present the abstraction-level results
  with a clear statement of which pipeline was used for each, since P0 and P2 have
  different devirtualization capabilities.
- **Severity**: MEDIUM

---

### T-18: Optuna Single-Objective vs. Pareto Optimization

- **Values in tension**: Simplicity of optimization objective vs. accurate modeling of
  the JIT-overhead/speedup tradeoff
- **Current choice**: `optimize_benchmarks.py` uses `geomean(jit_overhead_ns +
  specialized_exec_ns)` as a scalar objective.  This implicitly assumes one call
  amortizes JIT overhead (N=1).
- **Justification**: A scalar objective is simpler to implement and sufficient for
  finding configurations that are good across the range of call counts.  The Pareto
  front is plotted separately for deeper analysis.
- **Documented in thesis**: PARTIALLY — Spec 007 explicitly flags this as methodology
  weakness W6 and W4.  The thesis RQ section calls for break-even analysis but it is
  not yet written.  The breakeven formula is mentioned in the thesis but the results are
  not.
- **Scientific concern**: The objective is correct for one-shot workloads and misleading
  for high-call-count workloads.  Since the primary motivating use case (query
  compilation) involves specializing once and executing millions of times, the current
  objective underweights execution speedup.  The Pareto front partially addresses this,
  but the thesis should explicitly state the assumed call count distribution for each UC
  group and justify why the scalar objective is appropriate for the evaluation.  Research
  question Q10 in `questions.md` directly asks whether the objective models the use case
  correctly — this is unresolved.
- **Severity**: HIGH

---

### T-19: JIT Caching — No Caching of Specialized Functions

- **Values in tension**: JIT overhead amortization vs. implementation complexity and
  memory footprint
- **Current choice**: Each `callSpecialized` call re-JITs from scratch.  `specializeOnly`
  returns a handle that caches the result until destruction.  There is no automatic
  memoization: two calls to `callSpecialized` with identical arguments each pay full JIT
  cost.
- **Justification**: The `specializeOnly` API gives the user explicit control over
  lifetime.  Automatic memoization would require a cache key (the argument values),
  memory management (eviction), and thread-safety for the cache.  These are all
  non-trivial.
- **Documented in thesis**: PARTIALLY — The Threats to Validity section in the draft
  mentions "module cloning scales with blob size" as a cost, but does not note that
  `callSpecialized` re-JITs on every call.  The API section distinguishes `callSpecialized`
  from `specializeOnly` but does not explain the performance implications.
- **Scientific concern**: For the thesis benchmarks, `specializeOnly` is used for
  execution-time measurements, so the re-JIT overhead is not double-counted.  However,
  a real deployment using `callSpecialized` in a loop would pay full JIT cost per
  iteration.  The thesis should add a sentence to the API design section clarifying this
  and directing users toward `specializeOnly` for repeated calls.
- **Severity**: LOW

---

### T-20: Debug vs. Release Context Handling (Shared vs. Fresh LLVMContext)

- **Values in tension**: Debug-mode correctness vs. performance
- **Current choice**: Debug builds re-parse the blob from bitcode into a fresh
  `LLVMContext` per call, avoiding `ValueHandles` accumulation bugs.  Release builds
  use `CloneModule` + shared `TSCtx`.
- **Justification**: The shared-context optimization in release builds is important for
  performance (bitcode re-parsing is expensive).  The debug-mode workaround is a
  correctness fix for a known LLVM bug.
- **Documented in thesis**: YES — §Debug vs. Release Context Handling explains both paths
  and the root cause.
- **Scientific concern**: This is the correct approach, but the thesis should make clear
  that benchmark results are all from release builds.  The TODO in §Hardware and Build
  Configuration should note: "All benchmarks use release build (NDEBUG=1); debug builds
  do not support DuckDB-scale modules and are used only for correctness testing."
- **Severity**: LOW

---

## Missing Tradeoff Discussions

The following tradeoffs are present in the code or analysis reports but do not appear
in the current thesis draft:

**MT-01: P1 correctness defect under specific parameter combinations.**
  The OAT sweep crash (SIGSEGV with `unroll_max=27, p1_max_module_growth=1.64`) is
  documented in `questions.md` Q3 and `260523-11-35-open-research-questions/questions.md`
  but appears nowhere in the thesis.  If P1 is presented as an evaluated pipeline, its
  known stability boundary must be disclosed.

**MT-02: No serialization of function pointers or `std::function`.**
  The thesis "annotation-free" claim applies only to the call site.  If the target
  function receives a callback (function pointer or `std::function`) as an argument, the
  callback's address is baked in, but the JIT cannot optimize through it (it is an
  opaque pointer).  This limits specialization benefit for higher-order patterns without
  virtual dispatch.  Not discussed in the thesis.

**MT-03: `LargeModuleThreshold` gates on instruction count post-prune, not pre-prune.**
  The threshold classifies "large modules" after `GlobalDCE` removes dead code.  For a
  module that prunes aggressively, the pre-prune count could cross the threshold while
  the post-prune count does not, potentially enabling expensive passes that were
  supposed to be disabled.  No empirical validation that the default threshold (5000
  instructions) is the right boundary.

**MT-04: Large code model always on.**
  `JITTargetMachineBuilder` is forced to `CodeModel::Large`.  This prevents JIT code
  from using RIP-relative addressing for accesses to host globals, increasing code size
  and potentially degrading instruction cache behavior.  The cost is not measured.
  The justification (sqlite3_temp_directory > 2 GB from JIT allocation) is correct but
  the performance impact is not quantified.  Not discussed in the thesis.

**MT-05: Instrumentation overhead on benchmarks.**
  The `benchmarkJITAnalysis` tier uses `atomicrmw add` increments per basic block.
  These are executed by the specialized function and counted as part of execution time.
  The analysis tier is used for instruction-count measurement, not performance
  measurement, but if results from the two tiers are mixed in the thesis the overhead
  must be clearly stated.

**MT-06: P2 not yet evaluated against P0/P1 across all UC groups.**
  Spec 015 is in Draft status and describes P2 as a new pipeline.  The thesis claims
  three pipelines "cover different tradeoff points" but P2's empirical position on the
  Pareto front relative to P0 and P1 is not yet measured.  SC-005 in spec 015 requires
  this measurement.  The thesis should not present P2 as an evaluated alternative until
  the data exists.

**MT-07: The `specializeOrFallback` API silently degrades to unspecialized execution.**
  If the binary is compiled without the plugin (common in CI, debug builds, or
  third-party integration), `specializeOrFallback` silently falls back to the original
  function.  This is by design, but a programmer who forgets to compile with the plugin
  will never see a specialization speedup and may think the system is broken.  The thesis
  mentions the fallback mechanism but does not discuss the debugging experience when the
  plugin is absent.

**MT-08: `GlobalSpecializationCount` counter is not reset between benchmark iterations.**
  The `ClangRuntimeSpecializer::GlobalSpecializationCount` used in wrapper names grows
  monotonically.  After many benchmark iterations (e.g., benchmarks running 10k
  iterations), the counter reaches large values.  This has no correctness consequence
  but the JITDylib accumulation (each `specializeOnly` call creates a dylib that is only
  freed on destructor) means that within a single benchmark process run, LLJIT's dylib
  count grows continuously.  The thesis mentions "JIT dylib state growth" as an
  architectural characteristic in the TODO at §Ensuring Accurate Timings but this is not
  yet in the final text.
