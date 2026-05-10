# Tasks: Eliminate Manual ASM Names from Specialization API

**Input**: Design documents from `specs/010-eliminate-asm-names/`  
**Prerequisites**: plan.md ✅, spec.md ✅, research.md ✅, data-model.md ✅, contracts/api.md ✅

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies on incomplete tasks)
- **[Story]**: Which user story this task belongs to ([US1]=lambda specialization, [US2]=auto name resolution, [US3]=named-function+0-arg, [US4]=error paths)

---

## Phase 1: Setup

**Purpose**: Understand the current implementation before making changes. This is a refactor of an existing project; no new project structure is created.

- [ ] T001 Read `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h` lines 432–720 to understand the current `callSpecialized`, `specializeOnly`, `specializeLambda`, `specializeOnlyImpl`, and `specializeLambdaImpl` overload structure before any changes are made

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Add the internal `*Resolved` overloads (called by pass-rewritten IR) and the zero-argument lambda unification branch. These additions are pure extensions to the existing header — they do not remove or break any existing code — and are prerequisites for all user-story phases.

**⚠️ CRITICAL**: No user-story implementation work can begin until T002–T004 are complete.

- [ ] T002 Add internal `specializeLambdaResolved<R>(const char* resolvedName, Lambda& lambda)` and `specializeLambdaResolved<R>(const char* resolvedName, Lambda& lambda, const Options& opts)` private member overloads to `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h` — these are the targets called by pass-rewritten `specializeLambda` call sites; they forward to `specializeLambdaImpl(resolvedName, lambda, opts)`
- [ ] T003 [P] Add internal `specializeOnlyResolved<R>(const char* resolvedName, F* /*unused*/, Args&&... args)` and `callSpecializedResolved<R>(const char* resolvedName, F* /*unused*/, Args&&... args)` private member overloads to `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h` — these forward directly to `specializeOnlyImpl(resolvedName, opts, args...)`; the `F*` argument is accepted but unused at runtime (the pass already extracted the name from it)
- [ ] T004 Implement `if constexpr (sizeof...(ExplicitArgs) == 0)` branch in `specializeLambdaImpl` in `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h` — when the explicit-arg pack is empty, call `specializeOnlyImpl(ResolvedName, Opts, &lambda)` instead of building a separate wrapper; this unifies the 0-arg lambda and `specializeOnly` runtime code paths (FR-005, FR-011)

**Checkpoint**: Internal resolved overloads exist; 0-arg lambda unification branch in place. User-story implementation can now begin.

---

## Phase 3: User Stories 1 & 2 — Lambda Specialization + Automatic Name Resolution (Priority: P1) 🎯 MVP

**Goal**: `specializeLambda<R>(lambda)` works without any `funcName` string or `__asm__` annotation. The lambda body may contain arbitrary logic. The IRDumpingPass automatically resolves and embeds the lambda's `operator()` name at compile time.

**Independent Test**: `ninja check-wip-runtime-specializer` passes for all lambda-no-funcname WIP tests. After promotion, `ninja check-smoke-runtime-specializer` passes.

### WIP Tests (write first — expected to compile-fail until T008–T011 are done)

- [ ] T005 Write `test/WIP/lambda-no-funcname-complex.cpp`: lambda `[&obj](int x, bool y){ for(...){if(y) obj.foo(i,x); else obj.bar(i,x);} return x; }`, calls `specializeLambda<int>(lambda)` (no funcName), invokes with `(5, true)` and `(5, false)`, FileCheck asserts results match direct invocation — covers FR-014a
- [ ] T006 [P] Write `test/WIP/lambda-no-funcname-capture-free.cpp`: a lambda with no captures performing a simple arithmetic computation, calls `specializeLambda<int>(lambda)`, FileCheck asserts result is correct — covers FR-014b
- [ ] T007 [P] Write `test/WIP/lambda-no-funcname-multi-tu.cpp`: two structurally-different lambdas in the same TU, each passed to `specializeLambda` separately, FileCheck asserts each resolves to its own `operator()` with independent results — covers FR-017

### Implementation for User Stories 1 & 2

- [ ] T008 Add `specializeLambda<R>(Lambda& lambda)` and `specializeLambda<R>(Lambda& lambda, const Options& opts)` user-facing public overloads to `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h` — these overloads have placeholder bodies (e.g., `abort()`) that the IRDumpingPass will always rewrite before runtime; annotate with `[[gnu::noinline]]` (implementation detail per spec) to ensure call sites remain visible to the pass; remove the existing `funcName` overloads only after all callers are updated (see T033)
- [ ] T009 Implement lambda `operator()` discovery in `comptime/IRDumpingPass.cpp`: given a `CallInst` to `specializeLambda`, trace the SSA def-use chain of the lambda argument back to its `alloca` to recover the closure struct type; search the module for a non-declaration `Function` whose first parameter type is a pointer to that struct and whose demangled name (via `llvm::demangle()`) contains `"operator()"` — this is the `operator()` mangled name to embed
- [ ] T010 Implement call-site detection loop in `IRDumpingPass::run()` in `comptime/IRDumpingPass.cpp`: after the existing blob-serialization code, iterate over all `CallInst`/`InvokeInst` in all functions of the module; for each, check if `callee->getName()` contains `"specializeLambda"` (using mangled-name substring match) to identify call sites to rewrite
- [ ] T011 Implement call-site rewriting for `specializeLambda` in `comptime/IRDumpingPass.cpp`: for each detected `specializeLambda` call site, (1) create a `private constant [N x i8]` global `@__crs_resolved_name_K` containing the null-terminated mangled `operator()` name, (2) replace the `CallInst` with a new call to `specializeLambdaResolved` with the name-pointer prepended and the original lambda argument forwarded, (3) copy metadata via `copyMetadata`
- [ ] T012 Add FR-018 compile-time error to `comptime/IRDumpingPass.cpp`: after the `operator()` discovery step (T009), if no matching function is found in the module, call `llvm::report_fatal_error` with a diagnostic that identifies the call site location (file + line from debug metadata if available, otherwise function name) — abort compilation; do NOT silently continue
- [ ] T013 Run `ninja check-wip-runtime-specializer` and iterate until T005, T006, and T007 all pass; fix any IR-rewriting bugs in `comptime/IRDumpingPass.cpp` and `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h`

### Migrate Spec 009 WIP Tests

- [ ] T014 Migrate `test/WIP/specialized-lambda-basic.cpp` (spec 009): replace `specializeLambda<int>("add_with_ctx", lambda)` with `specializeLambda<int>(lambda)`; remove all `__asm__` annotations and the `funcName` string; update FileCheck patterns if they reference the old funcName call log messages; verify the test passes under `ninja check-wip-runtime-specializer`
- [ ] T015 [P] Migrate `test/WIP/specialized-lambda-void.cpp` (spec 009): same migration pattern as T014 — remove funcName, update `specializeLambda<void>` call; verify
- [ ] T016 [P] Migrate `test/WIP/specialized-lambda-equivalence.cpp` (spec 009): update `assertSpecializedLambdaIsEquivalent<R>(funcName, lambda, inputs)` call to `assertSpecializedLambdaIsEquivalent<R>(lambda, inputs)` (the helper is updated in T026); update FileCheck patterns; verify

### Promote to Smoke Suite

- [ ] T017 Run `ninja check-wip-runtime-specializer` and confirm all six lambda tests pass (T005–T007 new tests + T014–T016 migrated spec 009 tests)
- [ ] T018 Promote all six passing lambda WIP tests to `test/smoke/` (move files; update `test/smoke/CMakeLists.txt` to include them); run `ninja check-smoke-runtime-specializer` and verify all tests pass in both debug and release builds

**Checkpoint**: `specializeLambda<R>(lambda)` works end-to-end with no `funcName`; smoke suite is green; spec 009 superseded and its tests promoted.

---

## Phase 4: User Story 3 — Named Function Specialization + 0-arg Lambda Unification (Priority: P2)

**Goal**: `specializeOnly<R>(&myFunc, args...)` and `callSpecialized<R>(&myFunc, args...)` work without a `funcName` string. A 0-arg lambda and an equivalent fully-baked `specializeOnly` call travel the same runtime code path and produce the same JIT pass trace. All helper functions have `funcName` removed.

**Independent Test**: `ninja check-wip-runtime-specializer` passes for speconly-funcptr and zero-arg-lambda-equals-speconly tests. Helper function callers compile and produce correct results. Benchmark build succeeds.

### WIP Tests (write first)

- [ ] T019 Write `test/WIP/speconly-funcptr.cpp`: define `int computeFoo(int x)` (with IRDumpingPass plugin), call `specializeOnly<int>(&computeFoo, 42)` (no funcName string), invoke the returned function pointer, FileCheck asserts result equals `computeFoo(42)` — covers FR-015 scenario 1
- [ ] T020 [P] Write `test/WIP/zero-arg-lambda-equals-speconly.cpp`: define `int computeFoo(int x)`, specialize via (a) `specializeLambda<int>([x=42]{ return computeFoo(x); })` and (b) `specializeOnly<int>(&computeFoo, 42)`, call both, assert equal results; call `getLastPassTrace()` on both paths and FileCheck that the pass sequence and instruction counts are identical — covers FR-015 scenario 2 and SC-005

### Implementation for User Story 3

- [ ] T021 Add `specializeOnly<R>(F* func, Args&&... args)` and `specializeOnly<R>(F* func, const Options& opts, Args&&... args)` user-facing public overloads to `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h` — annotate with `[[gnu::noinline]]`; the pass rewrites these to `specializeOnlyResolved`; placeholder body (e.g., `abort()`)
- [ ] T022 [P] Add `callSpecialized<R>(F* func, Args&&... args)` and `callSpecialized<R>(F* func, const Options& opts, Args&&... args)` user-facing public overloads to `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h` — same pattern as T021
- [ ] T023 Implement function pointer extraction in `comptime/IRDumpingPass.cpp`: for `specializeOnly`/`callSpecialized` call sites, get argument 0 (the function pointer), call `Value::stripPointerCasts()` to strip bitcasts, `dyn_cast<Function>()` to get the callee global, and `Function::getName()` to get the mangled name — this is the resolved name to embed
- [ ] T024 Extend the call-site detection loop in `IRDumpingPass::run()` (`comptime/IRDumpingPass.cpp`) to also detect calls whose callee name contains `"specializeOnly"` or `"callSpecialized"`; apply the same rewriting pattern as T011 — create `@__crs_resolved_name_K` global and replace the call with `specializeOnlyResolved` / `callSpecializedResolved`
- [ ] T025 Add FR-018 compile-time error for `specializeOnly`/`callSpecialized` call sites in `comptime/IRDumpingPass.cpp`: if the first argument is NOT a compile-time constant function (i.e., `dyn_cast<Function>` fails after stripping casts), call `llvm::report_fatal_error` with a diagnostic explaining that the function pointer must be a compile-time constant
- [ ] T026 Update helper free functions in `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h`: remove `const char* funcName` first parameter from `specializeFunctionOrFallback`, `specializeOrFallback`, `assertSpecializedIsEquivalent`, `compareFunctionInstructionCounts`, and `assertSpecializedLambdaIsEquivalent`; update each helper to derive the function name internally by delegating to `specializeOnly<R>(&F, args...)` (so the pass handles name resolution transitively via the `specializeOnly` call site inside the helper)
- [ ] T027 Update all benchmark source files under `benchmarks/` that call the old funcName-string overloads: replace `callSpecialized<R>("name", args...)` → `callSpecialized<R>(&name, args...)`, `specializeOnly<R>("name", args...)` → `specializeOnly<R>(&name, args...)`, and helper calls that took funcName — iterate over all `.cpp` files under `benchmarks/polybench/`, `benchmarks/tpch/`, and `benchmarks/*.cpp`
- [ ] T028 Update `benchmarks/ClangRuntimeSpecializerBenchmark.h`: update `benchmarkJITOverhead`, `benchmarkJITOverheadMethod`, `benchmarkJITAnalysis`, `registerBudgetBenchmarks` helper templates that internally call specialization APIs or helpers with funcName strings
- [ ] T029 Run `ninja check-wip-runtime-specializer` and iterate until T019 and T020 both pass; confirm pass trace comparison in T020 shows identical pass sequence and instruction counts
- [ ] T030 Promote T019 and T020 tests to `test/smoke/` (move files; update `test/smoke/CMakeLists.txt`); run `ninja check-smoke-runtime-specializer` and verify all tests pass

**Checkpoint**: Named function specialization works without funcName; 0-arg lambda and `specializeOnly` share the unified code path; helper functions updated; benchmarks compile.

---

## Phase 5: User Story 4 — Error Paths + Old API Removal (Priority: P3)

**Goal**: Any call site where the pass cannot resolve the target function identity causes a compile-time fatal error with a clear diagnostic. All old `funcName`-string overloads are removed.

**Independent Test**: Attempting to compile a file with an unresolvable `specializeLambda` call fails with the expected diagnostic. All existing smoke tests continue to pass after old overloads are deleted.

### WIP Tests (write first)

- [ ] T031 Write `test/WIP/missing-plugin-lambda-error.cpp`: a lambda in a TU compiled **without** the IRDumpingPass plugin, called via `specializeLambda<int>(lambda)`; the test's RUN line deliberately omits `-fpass-plugin=...`; FileCheck expects `ClangRuntimeSpecializerDumpedIRError` or a falsy `SpecializedLambda` — covers FR-016 and SC-006
- [ ] T032 [P] Write `test/WIP/missing-plugin-funcptr-error.cpp`: same pattern for `specializeOnly<int>(&myFunc, 42)` without the plugin — covers FR-016 for named-function call sites; FileCheck expects `ClangRuntimeSpecializerDumpedIRError`

### Remove Old funcName Overloads

- [ ] T033 Delete the old `specializeLambda<R>(const char* funcName, Lambda&)`, `specializeOnly<R>(const char* funcName, args...)`, and `callSpecialized<R>(const char* funcName, args...)` overloads from `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h` — these are the previously-public funcName-string overloads; confirm no user-visible overload now accepts a `const char*` as its first parameter
- [ ] T034 Update existing smoke tests that used the old funcName API: in `test/smoke/call-specialized.cpp` replace `specializeOrFallback("mypow", mypow, argc)` with `specializeOrFallback(mypow, argc)`; in `test/smoke/pure-specialized-function-is-equivalent.cpp` update `assertSpecializedIsEquivalent` call to remove the funcName argument; update any other smoke tests similarly
- [ ] T035 Run `ninja check-smoke-runtime-specializer` and fix all regressions introduced by the old overload deletion; iterate until the full smoke suite is green

### Verify Compile-Time Error Behavior

- [ ] T036 Write a negative-compile test (or a FileCheck-on-stderr test) in `test/WIP/` that calls `specializeOnly` with a runtime-variable function pointer (e.g., passed as a function parameter) and confirm the IRDumpingPass emits a `fatal_error` containing the expected diagnostic text — this validates FR-018 for the non-constant-pointer case

### Promote to Smoke Suite

- [ ] T037 Promote T031, T032, and T036 tests to `test/smoke/` (move files; update `test/smoke/CMakeLists.txt`); run `ninja check-smoke-runtime-specializer` in both debug and release builds and verify all pass

**Checkpoint**: Old API completely removed; error paths verified; full smoke suite green.

---

## Final Phase: Polish & Cross-Cutting Concerns

- [ ] T038 Run `ninja check-all-runtime-specializer` (full suite including `test/complete/`); fix any failures not already covered by the smoke suite
- [ ] T039 [P] Update `docs/thesis.typ` — add concise bullet points under the JIT specialization API section documenting: (a) compile-time name resolution via IRDumpingPass call-site rewriting, (b) unified 0-arg lambda / `specializeOnly` code path, (c) arbitrary lambda bodies now supported
- [ ] T040 [P] Clean up `test/WIP/` — remove any WIP test files for spec 009 that have been superseded and fully replaced by smoke tests; confirm `test/WIP/` contains only genuinely in-progress tests

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1** (Setup): No dependencies — start immediately
- **Phase 2** (Foundational): Depends on Phase 1 — **BLOCKS** all user-story phases
- **Phase 3** (US1+US2, P1): Depends on Phase 2 completion
- **Phase 4** (US3, P2): Depends on Phase 2 completion; can start in parallel with Phase 3 after T003 and T004 are done
- **Phase 5** (US4, P3): Depends on Phase 3 AND Phase 4 completion (old overload deletion requires all callers updated)
- **Final Phase**: Depends on Phase 5 completion

### Within Phase 3

- T005–T007 (WIP tests): write first; can be done in parallel; will fail to compile until T008 is done
- T008 (new public overloads): unblocks T009–T012
- T009–T012 (pass changes): T009 feeds T011; T010 and T009 can overlap; T012 is after T009
- T013 (verify): after T009–T012
- T014–T016 (migration): can be done in parallel after T013 passes
- T017 (verify all): after T014–T016
- T018 (promote): after T017

### Within Phase 4

- T019–T020 (WIP tests): write first, in parallel
- T021–T022 (new overloads): in parallel, after T003 is done
- T023–T025 (pass extension): T023 feeds T024; T025 is after T023
- T026 (helpers): after T021 (needs new `specializeOnly` overload to delegate to)
- T027–T028 (benchmark callers): after T026 (helpers updated first)
- T029 (verify): after T023–T028
- T030 (promote): after T029

### User Story Dependencies

- **US1+US2** (P1): Starts after Phase 2 — no dependency on US3/US4
- **US3** (P2): Starts after Phase 2 — no dependency on US1/US2; can proceed in parallel
- **US4** (P3): Starts after US1+US2 AND US3 are complete (old overload deletion in T033 requires all callers already updated)

---

## Parallel Opportunities

### Phase 3 Tests (write simultaneously)
```
T005 + T006 + T007  — three different WIP test files, no shared state
```

### Phase 3 Spec 009 Migration (after T013 passes)
```
T014 + T015 + T016  — three different WIP test files migrated in parallel
```

### Phase 4 WIP Tests
```
T019 + T020  — two different WIP test files
```

### Phase 4 Header Additions
```
T021 + T022  — specializeOnly and callSpecialized overloads in same header; non-conflicting additions
```

### Final Phase
```
T039 + T040  — thesis update and WIP cleanup are independent
```

---

## Implementation Strategy

### MVP (Phase 3 only: Lambda Specialization)

1. Complete Phase 1 (Setup) + Phase 2 (Foundational)
2. Complete Phase 3 (US1+US2): `specializeLambda(lambda)` works end-to-end
3. **STOP and VALIDATE**: smoke suite passes; `specializeLambda` usable without funcName
4. This is the highest-value deliverable — demo-able independently

### Incremental Delivery

1. Setup + Foundational → internal resolved variants in place
2. Phase 3: Lambda specialization → test & promote → smoke green (**MVP**)
3. Phase 4: Named function + helper migration → benchmarks compile → test & promote
4. Phase 5: Error paths + old API removal → final smoke green

### Single-Developer Sequential Order

T001 → T002 → T003 → T004 → T005 → T006 → T007 → T008 → T009 → T010 → T011 → T012 → T013 → T014 → T015 → T016 → T017 → T018 → T019 → T020 → T021 → T022 → T023 → T024 → T025 → T026 → T027 → T028 → T029 → T030 → T031 → T032 → T033 → T034 → T035 → T036 → T037 → T038 → T039 → T040

---

## Notes

- **Test-first per constitution**: Write WIP tests (T005–T007, T019–T020, T031–T032) before the corresponding implementation tasks
- **Constitution smoke gate**: `ninja check-smoke-runtime-specializer` must be green before any commit to main
- **Build dirs**: debug = `llvm/llvm/build/debug`; release = `llvm/llvm/build/release`
- **`noinline` on public overloads is an implementation detail** — not a formal spec requirement; use `[[gnu::noinline]]` or `LLVM_ATTRIBUTE_NOINLINE` as appropriate for the LLVM coding style
- **Spec 009 is superseded**: do not promote the old funcName-based WIP tests; only promote the migrated (no-funcName) versions
- **Old overloads (T033)**: delete AFTER all callers are updated (T026–T028, T034) to avoid a broken intermediate state
- Each [P] task operates on a different file from its parallel sibling — no merge conflicts
