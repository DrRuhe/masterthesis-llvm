# Issues — LLVM Runtime Specializer Review

**Date**: 2026-05-31  
**Sources**: assumptions.md, bugs.md, tradeoffs.md, methodology.md in this directory  
**Scope**: Thesis-write-up gaps, open research questions, and infrastructure issues that
must be resolved before thesis submission.

---

## I. Blocking Issues (Must fix before submission)

### ISSUE-01: CONTRA-02 — Thesis explicitly says StaticMutabilityAnalysis was abandoned; it is in production use

**Detail**: `thesis.typ` §Design describes `StaticMutabilityAnalysis` as "superseded by
IPSCCP" and not implemented. The code uses it in both P0 (`JITPipelineInlining.cpp`) and
P1 (`JITPipelineFuncSpec.cpp`), including in the fixpoint loop and post-fixpoint cleanup.

**Fix needed**: Either (a) update the thesis Design §3.2.3 to say "the pass was initially
abandoned but was later reintroduced as an IPSCCP complement because ..."; or (b) remove
the pass from the pipelines. This is the most obvious contradiction a reviewer will find.

---

### ISSUE-02: Pipeline 0 description is materially wrong (omits ~15 passes, misplaces GVN)

**Detail**: Thesis §4.4 Pipeline 0 describes a fixpoint of: `IPSCCP → DevirtVtableCallsPass →
GlobalOpt → ConstantArgAlwaysInlinePass → AlwaysInliner → GVN`. The actual fixpoint runs
~15 passes including `StaticMutabilityAnalysis`, `InvariantLoadToConstantPass`,
`WholeProgramDevirtPass`, `ReversePostOrderFunctionAttrsPass`, and a large FPM with 12+
function-level passes. GVN is not a top-level fixpoint step — it runs inside an FPM adaptor
guarded by `!LargeModule`. A reader trying to reproduce Pipeline 0 from the thesis will
build a different pipeline.

**Fix needed**: Rewrite the Pipeline 0 description to accurately describe the pass sequence.
At minimum, distinguish the "conceptual" pipeline from the full implementation and note the
FPM adaptors.

---

### ISSUE-03: IRDumpingPass §4.2 incorrectly states InternalLinkage is set before GlobalDCE

**Detail**: Thesis §4.2 lists "Sets all other defined functions to InternalLinkage" as a
step before GlobalDCE. The code does it AFTER GlobalDCE so that target functions remain
as DCE roots during pruning. A reader implementing this from the thesis will produce a
broken preprocessor where target functions are pruned.

**Fix needed**: Swap the order in the thesis description and add a note explaining why:
"Target functions must remain external during GlobalDCE to prevent them from being pruned
as dead roots. InternalLinkage is assigned only after DCE completes."

---

### ISSUE-04: IRDumpingPass §4.2 claims wrong global visibility attributes

**Detail**: Thesis §4.2 states the blob global uses `protected` visibility, `dso_local`,
and `dllexport`. The code uses `InternalLinkage` + `UnnamedAddr::Global` — none of those
attributes are present in the actual implementation.

**Fix needed**: Correct the thesis description to accurately state `InternalLinkage` +
`UnnamedAddr::Global`. Remove the incorrect `protected`/`dso_local`/`dllexport` claims.

---

### ISSUE-05: Pipeline error silently swallowed — incorrect specialization returned

**Detail**: `withModuleDo` discards errors from `Pipeline.Run(PipeArgs)` (logs + continues).
A failing optimization pass produces a partially-transformed module that proceeds to JIT
compilation, potentially returning a specialized function with undefined behavior. The
caller receives no indication of failure.

**Fix needed**: The `IRTransformLayer` callback should return the error through the JIT's
error-handling infrastructure. Alternatively, change `withModuleDo` to propagate errors.
See `bugs.md` SPEC-004 for the code location.

---

### ISSUE-06: Optuna optimization trials ran without CPU pinning/ASLR control

**Detail**: `optimize_benchmarks.py` and `ablation_benchmarks.py` (called from
`run_evaluation.sh`) never invoke the `best_practice_env` context manager that disables
ASLR, pins Turbo Boost, sets CPU governor, and pins affinity. The `best_practice_full`
flag is null for all optimizer trial rows in the database. All Phase A (50 trials) and
Phase B (5-rep ablation) measurements were taken under uncontrolled system conditions.

**Fix needed**: Before writing the evaluation chapter, rerun all thesis-cited measurements
with `best_practice_env` active, or explicitly document the uncontrolled conditions and
quantify the expected noise floor.

---

### ISSUE-07: TPC-H cross-workload transfer experiment is unanswered (blocks RQ4)

**Detail**: The TPC-H experiment (spec 007 SQ4 / experiment C) crashed with SIGSEGV due
to a debug-mode runtime embedded in the release binary. The UC-to-UC transfer showed no
advantage over Default within noise. Without TPC-H data, the thesis cannot answer "does
the optimal configuration generalize across workloads?" — a stated research question.

**Fix needed**: Fix the TPC-H binary (isolate debug vs. release binary builds in the CMake
config; root cause is a debug-mode CRS runtime linked into the release benchmark binary)
and run experiment C. If TPC-H is truly impractical (module too large for JIT), scope
the research question explicitly.

---

### ISSUE-08: P1 pipeline has an unreproduced SIGSEGV under specific parameter combinations

**Detail**: 100% crash rate observed with `pipeline=1, unroll_max=27,
p1_max_module_growth=1.64` (documented in `questions.md` RQ3). The `fixpoint_1`
configuration is explicitly excluded from the ablation suite to avoid the crash.
Root cause is unknown. If P1 is presented as an evaluated pipeline alternative, this
defect must be disclosed.

**Fix needed**: Either (a) bisect and fix the P1 crash before including P1 results in the
thesis; or (b) scope the thesis to P0 and P2 only and note P1 is experimental. Do not
present P1 as a production-quality alternative while the crash is unresolved.

---

## II. High-Priority Issues (Should fix or explicitly acknowledge)

### ISSUE-09: No confidence intervals on any speedup claim

**Detail**: All speedup ratios (e.g., "2.97× for box_filter") are point estimates with no
uncertainty bounds. `--benchmark_repetitions=1` is used throughout; Google Benchmark's
mean/stddev infrastructure is never exercised. The Phase B ablation uses 5 inter-process
reps but combines them as medians without reporting variance.

**Fix needed**: Report mean ± 95% CI (or median ± MAD for skewed JIT times) alongside all
speedup ratios. For comparisons between configurations, report p-values. At minimum,
switch to `--benchmark_repetitions=5` and report stddev in the thesis evaluation tables.

---

### ISSUE-10: JIT overhead benchmark includes dylib teardown time (inflates reported numbers)

**Detail**: `benchmarkJITOverhead` measures JIT compile + symbol lookup + `munmap` of code
pages (the `SpecializedFunction<R>` temporary destructs inside the timed loop body). For
large modules, deallocation is non-trivial. The `jit_analysis` tier uses `Iterations(1)`
and `UseManualTime()` which correctly isolates only compilation time. `jit_overhead` uses
auto-iteration and measures a different thing.

**Fix needed**: Add `->Iterations(1)->UseManualTime()` to `benchmarkJITOverhead`, or
explicitly note in the thesis that JIT overhead numbers include deallocation. Cross-compare
`jit_overhead` and `jit_analysis` for the same kernel to quantify the gap.

---

### ISSUE-11: Missing hardware specification in the thesis evaluation chapter

**Detail**: The `#TODO` block in thesis §Hardware and Build Configuration has not been
filled in. CPU model, cache sizes, frequency scaling state, LLVM version, and build flags
are all unspecified. The JIT overhead numbers (40–60 ms) are machine-specific and
meaningless without hardware context.

**Fix needed**: Fill in the hardware section from the Google Benchmark JSON output headers
and the Nix flake. Minimum required: CPU model + core count, LLVM version, build type,
whether `best_practice_env` was active.

---

### ISSUE-12: Polybench framed as a speedup benchmark when it is fundamentally a regression test

**Detail**: For compute-bound loops (the majority of Polybench), specializing on array
size parameters only helps if the compiler cannot determine those sizes at compile time.
The unspecialized baseline is compiled with -O3 which already applies vectorization and
unrolling when bounds are statically known. Expected specialization benefit ≈ 0 for most
Polybench kernels.

**Fix needed**: Reframe Polybench in the thesis as a "regression control" suite: the
expected result is speedup ≈ 1.0 with no regressions. Present speedup < 1.05 as a success
(no regression), not as a failure. Restructure the Polybench RQ accordingly.

---

### ISSUE-13: Fixpoint convergence behavior has a thesis TODO but no data

**Detail**: Thesis §4.4 has a TODO: "How many fixpoint iterations are typically needed?"
This is also open question Q5 in `questions.md`. The pass-trace infrastructure
(`g_lastPassTrace`, `writePassTraceJSON`) can answer this — it tracks per-pass instruction
counts and convergence. The `jit_analysis` benchmarks already write `*_pass_trace.json`
files. The analysis connecting pass-trace data to convergence has not been done.

**Fix needed**: Run `jit_analysis` benchmarks on representative UC groups, import pass
traces, and plot instruction count vs. fixpoint iteration number. Show that convergence
typically occurs within 2–3 iterations for UC workloads. This closes the thesis TODO and
validates the default `MaxFixpointIterations=5`.

---

### ISSUE-14: Optuna objective assumes N=1 call; misaligned with the query-engine use case

**Detail**: The optimizer objective `geomean(jit_ns + spec_ns)` implicitly assumes one
specialized call pays back the JIT cost. The primary motivating use case (SQL query
compilation) specializes once and executes millions of rows — meaning `jit_ns` should be
amortized over N calls, making `spec_ns` far more important than the objective reflects.
The Pareto front infrastructure exists but is not the primary reported result.

**Fix needed**: Report the Pareto front as the primary result rather than the scalar
objective. Add a break-even analysis table to the thesis showing N_break-even per kernel
(how many calls needed for JIT cost to pay off). The infrastructure for this already
exists in `v_optim_breakeven`.

---

### ISSUE-15: Wrapper function name differs between thesis and code

**Detail**: Thesis §4.3 says the wrapper is `@__crs_wrapper_<N>`. Actual prefix is
`specialized_wrapper_<N>_<addr>` (and `_no_opt` suffix when `Optimize=false`). The
discrepancy is cosmetic but a reviewer who checks the code will notice.

**Fix needed**: Update thesis §4.3 to use the correct name `specialized_wrapper_<N>_<addr>`.

---

### ISSUE-16: IRDumpingPass credited with function name collection — wrong pass

**Detail**: Thesis §4.2 says IRDumpingPass "3. Collects the mangled names of all defined
functions in the TU." Collection is actually done by IRRewritingPass, which writes the
names into `!crs.func_names` named metadata. IRDumpingPass reads from that metadata. The
two-pass coordination is an architectural detail worth documenting correctly.

**Fix needed**: Update §4.2 to state that IRRewritingPass collects and stores function
names in `!crs.func_names` metadata, and IRDumpingPass reads from it. Add a sentence to
§IRRewritingPass describing the two-pass metadata handoff.

---

## III. Medium-Priority Issues (Acknowledge or note as limitations)

### ISSUE-17: `bool` argument serialization throws at runtime (latent bug)

`serializeArgumentToIR` excludes `bool` from the integer branch and has no bool-specific
case. Passing a `bool` to `callSpecialized` throws `ClangRuntimeSpecializerArgSerializationError`.
`serializeTypeToLLVM` correctly handles `bool → i1`, making the two functions inconsistent.
Should be fixed (add `bool` to the integral branch as `i1`) or documented in the type
support table.

---

### ISSUE-18: P2 pass sequence doesn't match spec FR-010 (missing AlwaysInliner)

Spec 015 FR-010 requires `AlwaysInlinerPass` in the P2 sequence. The code omits it and
relies on O3's cost-based inliner. When `EnableO3Final=false`, no inlining occurs after
`JitIPSCCPPass`, so specialized clones are never merged into the wrapper. Either update
the spec to reflect the O3-based inlining strategy, or add `AlwaysInlinerPass` explicitly.

---

### ISSUE-19: P2 is described as an evaluated pipeline but comparison to P0 across all UC groups is not in the thesis

Spec 015 SC-005 requires a measured comparison of P2 vs P0 across the full benchmark
suite. The thesis draft presents P2 as one of three pipelines covering "different tradeoff
points," but the empirical Pareto position of P2 relative to P0 is not yet measured or
reported. The thesis should not claim P2 is an evaluated alternative until this data exists.

---

### ISSUE-20: TU separation requirement qualifies the "annotation-free" claim

The system requires specialization targets to be in a separate TU from application code.
If they share a TU, the JIT blob contains vtable functions from benchmark/application
infrastructure, causing "Symbols not found" link failures. This is a non-trivial
structural requirement for existing codebases. The thesis claim of "annotation-free" and
"drop-in" should be qualified as "annotation-free at the call site, but requires TU-level
separation of specialization targets."

---

### ISSUE-21: Unspecialized baseline measured once and reused across all 50 Optuna trials

`optimize_benchmarks.py` calls `measure_unspecialized` once before the Optuna loop. All
50 trial speedup ratios use this single measurement as the denominator. A noisy baseline
measurement inflates or deflates all 50 speedup ratios consistently.

**Fix needed**: Measure the baseline at least 3 times and use the median, or include an
unspecialized run in each trial invocation (already done in the filter path that includes
`_WITH_UNSPEC` benchmarks).

---

### ISSUE-22: `--export-dynamic` requirement not stated in thesis implementation chapter

The JIT requires the host binary to be linked with `--export-dynamic` (or equivalent)
for `DynamicLibrarySearchGenerator::GetForCurrentProcess` to find external symbols.
Without this, JIT link fails with "Symbols not found" for any external function. Spec 003
mentions this requirement; the thesis body does not.

---

### ISSUE-23: LLJIT internal state grows monotonically; "JIT code freed after call" claim needs qualification

Thesis §4.3 says "JIT-compiled code is freed after the call." Machine code pages are freed
(via `removeJITDylib`), but the LLJIT `ExecutionSession` accumulates internal bookkeeping
(symbol tables, JIT compile records) per dylib created. In production use where
`callSpecialized` is called millions of times, this can cause OOM. The thesis should
clarify: "machine code is freed; LLJIT session state grows O(1) per call."

---

### ISSUE-24: Missing third soundness condition in the correctness section

§3.3 "Correctness" lists two soundness conditions for shallow serialization: (1) same
address space, (2) no concurrent modification. A third condition is needed: (3) globals
must be initialized before specialization; globals initialized only by static constructors
will appear zero-initialized in the JIT. This was the root cause of the Polybench `static`
variable crash documented in MEMORY.md, but it does not appear in the thesis correctness
section.

---

### ISSUE-25: Duplicate function names across TUs silently use the wrong module

`FuncToBlobIdx` is built by iterating all registered blobs; the last blob that registers
a name wins. Two TUs with the same function name (e.g., two `static` functions with the
same name in different TUs, or two TUs providing different implementations of a C function)
produce silent wrong-module cloning. The spec notes this as `[CLARIFICATION NEEDED]` but
no fix or diagnostic was added. At minimum, emit a warning when a name collision is
detected at `init()` time.

---

## IV. Infrastructure Gaps (Address before final submission)

### IG-01: No mechanism to verify thesis numbers were collected under best-practice conditions

`record_benchmark.py` stores `best_practice_full` in the context table, but
`optimize_benchmarks.py` never sets this flag. Before writing the evaluation chapter,
add a database query that asserts `best_practice_full=TRUE` for all thesis-cited rows.

### IG-02: Binary SHA not recorded in trial context; silent mixing of results from different builds

`optimize_benchmarks.py` and `ablation_benchmarks.py` do not pass binary SHA as benchmark
context. If the binary is rebuilt mid-study, results from two different binaries are mixed
in the database without attribution. Add `sha256sum $BINARY` to the context of each trial.

### IG-03: DuckDB file has no text-format export / backup mechanism

The `benchmarks.duckdb` file is binary and version-specific. Add a `make export-db` target
running `EXPORT DATABASE '...' (FORMAT CSV)` before each thesis submission milestone.

### IG-04: Optuna convergence plot not generated or inspected

With 50 trials in a 12-dimensional search space, convergence is not verified. Generate
the objective-vs-trial-number plot and confirm the best value plateaued in the last ~15
trials before claiming the optimizer found a good configuration.

### IG-05: V1 blob registration API (`clang_runtime_specializer_register_blob`) not present despite being referenced in MEMORY.md

MEMORY.md claims "v1 backward-compat: `register_blob(ptr, len)` still works." No such
function exists. Any TU compiled with an older plugin emitting the v1 call will fail at
`init()` with a missing symbol or `DumpedIRError`. Remove the v1 claim from MEMORY.md or
add the v1 implementation.
