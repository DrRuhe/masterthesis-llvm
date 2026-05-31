# Bug Report — Runtime Specialization Code Review

**Reviewer**: Claude Sonnet 4.6 (automated)  
**Date**: 2026-05-31  
**Scope**: `runtime/ClangRuntimeSpecializer/`, `comptime/`, `benchmarks/`  
**Branch**: `runtime-specialization`

---

## Critical Bugs

### BUG-001: Data Race on LLJIT State After JIT Timeout

- **Location**: `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h:647–677`
- **Description**: When `JITTimeoutSeconds > 0` and the timeout fires, a `std::thread` is
  detached that continues calling `addModuleAndLookup` on `this` (the singleton
  `ClangRuntimeSpecializer`). If the caller immediately invokes a new specialization call
  (e.g., the Google Benchmark framework starts the next iteration of
  `BM_UC5_jit_overhead`), two threads concurrently access:
  1. `GlobalSpecializationCount` — incremented via `const_cast` (not atomic, no lock).
  2. `CurrentCallOptions` — written by `specializeOnlyImpl` at line 647 before the new
     JIT thread launches, and read by the IRTransformLayer callback.
  3. `JIT->createJITDylib`, `JIT->addIRModule`, `JIT->lookup` — LLJIT has internal locks
     for symbol resolution but `createJITDylib` mutates the dylib table, which is not
     documented as safe for concurrent calls from user code.
  The only guard in production code is `g_last_jit_timed_out` in
  `benchmarks/tpch/duckdb_tpch_bench.cpp:133`, which gates `phaseSpecializedExec` but
  NOT `phaseJITOverhead`. A second call to `phaseJITOverhead` while the background
  thread is still running will race.
- **Steps to reproduce**: Enable `JITTimeoutSeconds` on a large module (e.g., DuckDB)
  such that the timeout consistently fires. Run multiple benchmark iterations of the JIT
  overhead benchmark. Races appear as assertion failures or corrupted JITDylib names in
  debug builds.
- **Impact**: Undefined behavior — data corruption, assertion failure, or silent wrong
  results. This is classified critical because the DuckDB TPC-H benchmark uses
  `withJITTimeoutSeconds(60)` in production benchmark runs.

---

### BUG-002: `bool` Arguments Cannot Be Serialized as JIT Constants

- **Location**: `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h:892–920`
  (`serializeArgumentToIR`)
- **Description**: The `serializeArgumentToIR` function explicitly excludes `bool` from
  the integral branch (`is_integral_v<D> && !is_same_v<D, bool>`, line 895). No
  subsequent branch handles `bool`: `is_floating_point_v<bool>` is false,
  `is_pointer_v<bool>` is false, `is_class_v<bool>` is false. Every call to
  `specializeOnly<R>(&myFunc, true, …)` or `callSpecialized<R>(&myFunc, boolArg, …)`
  where any argument is `bool` **throws
  `ClangRuntimeSpecializerArgSerializationError`** at runtime.
  
  By contrast, `serializeTypeToLLVM` correctly maps `bool → i1`
  (line 109), so type deduction works but value baking fails. The two functions are
  inconsistent.
- **Steps to reproduce**:
  ```cpp
  bool flag = true;
  callSpecialized<int>(&myFunc, flag, 42);  // throws at runtime
  ```
- **Impact**: Any function with a `bool` parameter that the user wants to bake as a JIT
  constant silently blows up with a serialization error. The spec does not define
  `bool` behaviour (FR-016 says "integer types (non-bool)"), so this is an unspecified
  gap that manifests as a runtime error rather than a compile-time diagnostic.

---

### BUG-003: JIT Overhead Benchmark Includes Dylib Teardown Time

- **Location**: `benchmarks/ClangRuntimeSpecializerBenchmark.h:87–91`
  (`benchmarkJITOverhead` loop body)
- **Description**: Inside the timed benchmark loop, `specializeOnly` returns a
  `SpecializedFunction<R>` temporary that is passed to `benchmark::DoNotOptimize`. The
  temporary's destructor — which calls `removeJITDylibNoexcept(ES, Dylib)`, i.e.,
  unmapping the compiled machine-code pages — fires at the end of the statement,
  **inside the timed loop body**. The benchmark is supposed to measure JIT compilation
  overhead, but it actually measures:

  > JIT compilation time + symbol lookup time + JIT memory deallocation time

  `removeJITDylib` on Linux involves `munmap` on the code pages plus a DenseMap
  update in the `ExecutionSession`, which is non-trivial for large modules. The
  resulting numbers are systematically higher than true JIT compilation time and cannot
  be directly compared against e.g. the analysis benchmarks that use `benchmarkJITAnalysis`
  (which also includes teardown but uses `UseManualTime`).
- **Steps to reproduce**: Profile a `benchmarkJITOverhead` call with perf; observe that
  `munmap` and `ExecutionSession::removeJITDylib` appear in the hot path inside the
  measured interval.
- **Impact**: Published JIT overhead measurements are inflated by an unknown and
  variable amount. On large modules (DuckDB, polybench) the deallocation dominates. The
  methodology inconsistency makes cross-benchmark comparisons unreliable.

---

## Spec Inconsistencies

### SPEC-001: `prepareModuleForJIT` Searches Linearly for the Wrapper by Name

- **Spec**: Spec 003 FR-019 / Spec 004 FR-008 — `prepareModuleForJIT` MUST set the
  wrapper function to `ExternalLinkage`. "This is the only linkage modification it
  performs."
- **Implementation**: `ClangRuntimeSpecializer.cpp:688–697`
  ```cpp
  for (auto &F : M)
    if (F.getName() == WrapperName)
      F.setLinkage(llvm::GlobalValue::ExternalLinkage);
  ```
  The function iterates **all functions in the module** and sets linkage on the first
  name match. The wrapper is always the last function added to the module (created by
  `llvm::Function::Create` just before this call), so the linear scan is O(N) for no
  reason. A direct reference to the just-created `NewFunc` (which is already in scope
  in `specializeOnlyImpl`) should be used instead. Beyond efficiency, the linear scan
  has a subtle correctness risk: if a blob function has the same mangled name as
  `UniqueWrapperName` (vanishingly unlikely given the counter + address, but possible
  with name collisions after truncation), its linkage would be incorrectly overwritten.
- **Severity**: LOW (correctness risk is theoretical; performance impact is minor)

---

### SPEC-002: `FuncToBlobIdx` Silently Overwrites on Duplicate Function Names

- **Spec**: Spec 003 Edge Cases — "second registration overwrites the first in
  `FuncToBlobIdx`; the second TU's definition is used. [CLARIFICATION NEEDED]"
- **Implementation**: `ClangRuntimeSpecializer.cpp:620–623`
  ```cpp
  for (size_t i = 0; i < g_registered_blobs.size(); ++i)
    for (const auto& Name : g_registered_blobs[i].FuncNames)
      Instance->FuncToBlobIdx[Name] = i;
  ```
  The last blob that registers a given name wins. If two TUs both define a C function
  with the same name (legal in C with static linkage — but also possible with mangled
  names via `__asm__`), `getTargetFunction` will always return the function from the
  **second** blob's module. When the call site in the first TU is specialized, the
  clone is from the second TU's module. The second TU's module may have a different
  set of helper functions available, different inlining state, and different linkage
  invariants. This is a **silent semantic error** — no diagnostic is emitted.
- **Severity**: MEDIUM — silent wrong-module cloning when names collide.

---

### SPEC-003: Debug Mode `specializeOnlyImpl` Violates FR-008 (Shared LLVMContext)

- **Spec**: Spec 003 FR-008 — "All blob modules MUST share a single
  `ThreadSafeContext` (`LLVMContext`)."
- **Implementation**: `ClangRuntimeSpecializer.h:592–611` (debug path)
  ```cpp
  #ifndef NDEBUG
  auto FreshLLVMCtx = std::make_unique<llvm::LLVMContext>();
  llvm::orc::ThreadSafeContext NewTSCtx(std::move(FreshLLVMCtx));
  // re-parse blob into FreshCtxPtr ...
  ```
  In debug builds, each `specializeOnlyImpl` call re-parses the blob bitcode into a
  **fresh, per-call `LLVMContext`**, entirely separate from the shared `TSCtx`. The
  spec requires all modules to share one context; the debug path creates a new one per
  call. The same divergence occurs in `specializeLambdaImpl` (lines 743–757).
  The MEMORY.md documents this workaround (pImpl->ValueHandles corruption), but the
  spec does not acknowledge it.
- **Severity**: MEDIUM — spec and implementation diverge in the debug build, which is
  the build used during development and CI. Any test that relies on FR-008 (e.g.,
  cross-module type identity) will silently behave differently in debug vs release.

---

### SPEC-004: `withModuleDo` Silently Swallows Pipeline Errors

- **Spec**: Spec 003 FR-032 — "CurrentCallOptions MUST be set from
  `specializeOnlyImpl` before the JIT IR transform layer runs."  
  Implied by the spec: JIT compilation failures should be surfaced to the caller.
- **Implementation**: `ClangRuntimeSpecializer.cpp:501–505`
  ```cpp
  if (auto Err = getRegisteredPipelines()[ValidPipeline].Run(PipeArgs)) {
    // Log but don't propagate — withModuleDo doesn't return Error.
    log(LogLevel::Error, ...);
  }
  ```
  The IRTransformLayer callback uses `TSM.withModuleDo(...)`, whose return type is
  `void` (determined by the lambda's return type). If the pipeline `Run()` returns an
  error, the error is logged and **discarded**. Execution continues with a partially
  transformed module. The subsequent `JIT->addIRModule` and `JIT->lookup` may succeed
  on the bad module, returning a specialized function with undefined behavior, or may
  fail with a cryptic linker error. No `Error` is propagated back to `specializeOnlyImpl`.
  The `JIT->addIRModule` wrapping `TSM.withModuleDo` is where LLVM would propagate
  errors back, but only if the transform layer itself returned an `Expected<TSM>`.
  Since the transform lambda discards the pipeline error and returns normally, the JIT
  proceeds as if optimization succeeded.
- **Severity**: HIGH — a failing optimization pass (e.g., an assertion in a custom
  pass, or an IPSCCP solver failure) produces a silently incorrect specialization
  rather than a thrown exception that the caller can handle.

---

### SPEC-005: Spec 015 FR-010 Requires `AlwaysInlinerPass` in P2; Implementation Omits It

- **Spec**: Spec 015 FR-010 — "The P2 pipeline MUST implement the following sequence
  …: … → `GlobalDCEPass` → `AlwaysInlinerPass` → `GlobalDCE + SimplifyCFGPass +
  InstCombinePass` cleanup."
- **Implementation**: `runtime/ClangRuntimeSpecializer/JITPipelineIPSCCP.cpp:68–119`
  No `AlwaysInlinerPass` appears. The comment at line 123 states: "No explicit
  AlwaysInliner here: O3's cost-model inliner handles all inlining." The O3 pass
  (via `Args.PB.buildPerModuleDefaultPipeline`) is run at Phase 4 (lines 152–157) only
  when `Opts.EnableO3Final` is true. If O3 is disabled, no inlining occurs at all after
  `JitIPSCCPPass`, meaning SCCP-specialised clones are never merged into the wrapper.
- **Severity**: MEDIUM — when `EnableO3Final=false` (e.g., `Options::O3Only()` sets it
  true but many custom configs may not), P2 produces unoptimised call chains. The spec
  says always run AlwaysInliner; the code conditionally runs O3.

---

### SPEC-006: Spec 003 FR-015 Wrapper Name Format Undocumented `_no_opt` Suffix

- **Spec**: Spec 003 FR-015 — "Its name is `'specialized_wrapper_<N>_<addr>'`."
- **Implementation**: `ClangRuntimeSpecializer.h:579`
  ```cpp
  std::string UniqueWrapperName = createUniqueWrapperName() + (Opts.Optimize ? "" : "_no_opt");
  ```
  And line 740 for lambda variant. The actual wrapper name is
  `specialized_wrapper_<N>_<addr>_no_opt` when `Opts.Optimize = false`. The spec does
  not mention this suffix. Tests that match wrapper names (e.g., FileCheck patterns
  searching for `specialized_wrapper_`) may not match in the no-optimize path.
- **Severity**: LOW — smoke tests use `%` wildcards in most FileCheck patterns; risk is
  limited to tests with exact wrapper-name assertions.

---

### SPEC-007: Spec 010 IRRewritingPass Name-Substring Filter Too Broad

- **Spec**: Spec 010 FR-006 — "The IR-dumping pass MUST detect every call to
  `specializeLambda`, `specializeOnly`, and `callSpecialized`…"
- **Implementation**: `comptime/IRRewritingPass.cpp:163–168` and `202–209`
  ```cpp
  if (F.getName().contains("specializeLambda") || ...)
    continue;
  ```
  The pass skips scanning any function whose mangled name CONTAINS the substring
  `"specializeLambda"`, `"specializeOnly"`, `"callSpecialized"`, etc. This is intended
  to skip the CRS infrastructure functions. However, it also skips any
  **user-defined function** whose name happens to contain those substrings (e.g.,
  `mySpecializeLambdaHelper`, `wrapperForCallSpecialized`). If a user's function calls
  `specializeLambda` from inside such a helper, the call site will not be rewritten.
  The binary will compile silently; at runtime `resolvedName` is `nullptr`, and
  `ClangRuntimeSpecializerDumpedIRError` is thrown.
- **Severity**: LOW — in practice, users won't name their functions this way; but the
  spec requires detection of EVERY call site, and this filter can silently miss some.

---

### SPEC-008: Spec 003 FR-034 `getModuleStats()` Called Before `init()` Returns Correct Zero Values, But After `init()` Before Blob Parse Completes It May Return Partial Data

- **Spec**: Spec 003 FR-034 — "`getModuleStats()` MUST return… zero-initialized structs
  if `init()` has not been called."
- **Implementation**: `ClangRuntimeSpecializer.cpp:216–231`
  ```cpp
  if (!Instance || Instance->BlobModules.empty()) return Stats;
  ```
  The check `Instance->BlobModules.empty()` is performed without any lock. `init()` is
  supposed to be called from a single thread (spec assumption), but if `getModuleStats()`
  is called from another thread while `init()` is mid-way through populating
  `BlobModules` (line 603–617), a partial `BlobModules` vector will be iterated.
  The spec says `init()` is single-threaded; this is an assumption, not an enforcement.
  There is no synchronization primitive guaranteeing that `getModuleStats()` callers see
  a fully-initialized state.
- **Severity**: LOW — single-threaded init is documented; multi-threaded init is out of scope.

---

## Minor Issues

### MINOR-001: `normalArgs` Parameter in `benchmarkJITOverhead` Is Dead (Type-Deduction Only)

- **Location**: `benchmarks/ClangRuntimeSpecializerBenchmark.h:70–100`
- **Description**: `normalArgs` is declared as a parameter of `benchmarkJITOverhead`
  but is used ONLY for `decltype` return-type deduction (line 82:
  `using R = decltype(std::apply(InvokeNormal, normalArgs))`). The parameter is never
  actually invoked in the benchmark loop. The loop only uses `specArgs`. This makes the
  function signature misleading: callers must pass `normalArgs` even though it has no
  effect. All existing call sites pass the same value for both (e.g.,
  `std::make_tuple(x), std::make_tuple(x)`), which works but is confusing.

---

### MINOR-002: `createUniqueWrapperName` Uses `const_cast` to Mutate `GlobalSpecializationCount`

- **Location**: `ClangRuntimeSpecializer.cpp:685`
  ```cpp
  return "specialized_wrapper_" + std::to_string(
    const_cast<ClangRuntimeSpecializer*>(this)->GlobalSpecializationCount++) + ...;
  ```
- **Description**: `createUniqueWrapperName` is declared `const` but modifies
  `GlobalSpecializationCount` via `const_cast`. The counter should be `mutable` instead.
  The `const_cast` is a code smell that misleads the `const` qualifier.

---

### MINOR-003: `cantFail` on `DynamicLibrarySearchGenerator::GetForCurrentProcess` Is Overly Optimistic

- **Location**: `ClangRuntimeSpecializer.cpp:289`
  ```cpp
  Instance->JIT->getMainJITDylib().addGenerator(
      llvm::cantFail(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(...)));
  ```
- **Description**: `cantFail` aborts the process (via `report_fatal_error`) if the call
  fails. Failure is unlikely but possible if the dynamic linker is in a broken state.
  On systems where `dlopen(nullptr)` fails (some sandboxed environments), this will
  crash the process unconditionally at `init()`. The surrounding code uses
  `Expected<T>` with proper error handling; only this one call uses `cantFail`.
  `JD.define(absoluteSymbols(...))` on line 306 also uses `cantFail`.

---

### MINOR-004: Instruction Instrumentation Inserts Atomics After Instrumentation Runs on the Same Module (Cross-BB Safety)

- **Location**: `ClangRuntimeSpecializer.cpp:519–572`
- **Description**: The instrumentation pass iterates all functions and BBs, collects
  instruction counts for each BB, then inserts `AtomicRMW` increments at
  `BB.getFirstInsertionPt()`. The newly-inserted `AtomicRMW` instructions are then
  included in subsequent BB iterations if the pass later visits the same function.
  In practice, each function's BBs are iterated once in document order, and the
  insertions happen at the head of the CURRENT BB, so subsequent BBs in the same
  function are unaffected. But if two functions share a BB (impossible in valid LLVM IR)
  or if the pass is run twice on the same module (not the current design), the counters
  would be over-counted. This is safe for the current single-pass design but fragile.

---

### MINOR-005: Disassembler Heuristic Stops at `0xC3` / `0xCB` Without Accounting for x86 REX Prefix

- **Location**: `ClangRuntimeSpecializer.cpp:794`
  ```cpp
  bool IsRet = (Bytes[PC] == 0xC3 || Bytes[PC] == 0xCB);
  ```
- **Description**: The JIT assembly dumper (used when `CRS_ASM_DUMP_DIR` is set)
  terminates disassembly when it sees a byte `0xC3` (RETN) or `0xCB` (RETF). But
  `0xC3` can also appear as an immediate operand in a non-return instruction. The MCDisassembler
  is already used to correctly parse instruction sizes and names; the single-byte
  heuristic is redundant and wrong. The correct approach is to check the disassembled
  `MCInst` opcode. This causes truncated disassembly output for functions whose last
  `ret` is a far-return or whose penultimate byte is `0xC3`.

---

### MINOR-006: `benchmarkLambdaJITAnalysis` Calls `RS->setOptions(opts)` Which Mutates Global Instance State

- **Location**: `benchmarks/ClangRuntimeSpecializerBenchmark.h:254, 263`
  ```cpp
  RS->setOptions(opts);
  // ...
  RS->setOptions(ClangRuntimeSpecializer::Options::Default());
  ```
- **Description**: `benchmarkLambdaJITAnalysis` modifies the singleton's global
  `CurrentOptions` via `setOptions`. This is unlike all other benchmark helpers
  (`benchmarkJITOverhead`, `benchmarkSpecializedExec`) which pass opts per-call. If the
  state loop has multiple iterations (it won't because `Iterations(1)` is required, but
  this is not enforced inside the helper), or if another benchmark runs concurrently
  (not the current design), the global state mutation is a hazard. The reset
  `RS->setOptions(Default())` also permanently changes the instance's default after the
  benchmark helper exits, which is surprising behaviour.

---

### MINOR-007: V1 Blob Registration API Missing Despite Being Referenced in MEMORY.md

- **Location**: `ClangRuntimeSpecializer.cpp` — no `clang_runtime_specializer_register_blob` (v1)
- **Description**: The project's MEMORY.md states "v1 backward-compat: `register_blob(ptr, len)` still works; fallback scans parsed module to build index." No such function exists in the current codebase. Only `clang_runtime_specializer_register_blob_v2` is defined. If any TU was compiled with an older plugin that emits the v1 registration call, `FuncToBlobIdx` will not be populated for that TU, and `getTargetFunction` will throw `ClangRuntimeSpecializerDumpedIRError`.

---

### MINOR-008: Spec 004 FR-007 "No `__clangRS` Functions" Guarantee Only Holds If DCE Runs

- **Location**: `comptime/IRDumpingPass.cpp:182–200` (GlobalDCE block), `comptime/IRDumpingPass.cpp:307–309` (return)
- **Description**: FR-007 states "after compile-time GlobalDCE, no function with a
  `__clangRS` prefix MUST remain." The DCE correctly removes them because erasing
  `llvm.global_ctors` makes `__clangRS_register_blob_*` unreachable. However, if a user
  function has a call to a `__clangRS_*`-prefixed function for some reason, it would
  keep that function alive through GlobalDCE roots. The spec does not account for
  user-introduced `__clangRS`-prefix functions.
  Also: the filter `!F.getName().starts_with("__clangRS")` at line 109 prevents these
  from being added as GlobalDCE roots, meaning they ARE discardable, so this is safe
  unless the user maliciously names things `__clangRS_*`. Low severity in practice.

---

## Summary

| ID | Severity | Category | Status |
|----|----------|----------|--------|
| BUG-001 | CRITICAL | Data race (timeout) | Open |
| BUG-002 | HIGH | Bool arg serialization | Open |
| BUG-003 | HIGH | Benchmark methodology | Open |
| SPEC-001 | LOW | prepareModuleForJIT linear scan | Open |
| SPEC-002 | MEDIUM | Duplicate function name silent overwrite | Open (noted in spec) |
| SPEC-003 | MEDIUM | Debug-mode LLVMContext divergence from spec | Open |
| SPEC-004 | HIGH | Pipeline errors silently swallowed | Open |
| SPEC-005 | MEDIUM | P2 missing AlwaysInliner vs spec FR-010 | Open |
| SPEC-006 | LOW | `_no_opt` wrapper suffix undocumented in spec | Open |
| SPEC-007 | LOW | IRRewriting name-substring filter too broad | Open |
| SPEC-008 | LOW | getModuleStats unsynchronized | Open |
| MINOR-001 | LOW | Dead `normalArgs` parameter | Open |
| MINOR-002 | LOW | `const_cast` on `GlobalSpecializationCount` | Open |
| MINOR-003 | LOW | `cantFail` on dylib generator setup | Open |
| MINOR-004 | LOW | Instrumentation pass fragility | Open |
| MINOR-005 | LOW | Disassembler termination heuristic | Open |
| MINOR-006 | LOW | `setOptions` global state mutation in benchmark | Open |
| MINOR-007 | LOW | V1 blob API missing | Open |
| MINOR-008 | LOW | FR-007 `__clangRS` guarantee edge case | Open |
