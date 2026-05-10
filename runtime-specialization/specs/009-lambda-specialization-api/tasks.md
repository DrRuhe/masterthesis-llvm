# Tasks: Lambda Specialization API

**Input**: Design documents from `specs/009-lambda-specialization-api/`  
**Prerequisites**: plan.md ✅, spec.md ✅, research.md ✅, data-model.md ✅, contracts/api.md ✅

**Tests**: Required — FR-012/013 mandate smoke tests.  
Tests written as WIP first (`test/WIP/`), promoted to `test/smoke/` on green.  
Each phase contains explicit **debug tasks** that iterate until the corresponding acceptance criteria pass.

**Acceptance Criteria reference** (from spec.md):

| ID | Criterion |
|----|-----------|
| SC-001 | `spec_lambda(x,y)` returns same value as `lambda(x,y)` for all test inputs — zero behavioral regressions |
| SC-002 | Instruction count of JIT-compiled function is strictly lower than unspecialized for at least one fixture |
| SC-003 | All three required smoke tests pass in debug AND release builds |
| SC-004 | 100-iteration SpecializedLambda construction/destruction loop produces no AddressSanitizer errors |
| SC-005 | `assertSpecializedLambdaIsEquivalent` raises `ClangRuntimeSpecializerChangesBehaviorError` for a deliberately wrong kernel |

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no shared state dependencies)
- **[Story]**: Which user story owns this task (US1 / US2 / US3)

---

## Phase 1: Setup

**Purpose**: Confirm clean build baseline before any changes.

- [ ] T001 Run `ninja ClangRuntimeSpecializer` from `llvm/llvm/build/debug`; confirm zero errors and zero warnings on the existing codebase — this is the baseline against which all new changes are validated

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core types and helpers required by every user story.  Nothing in Phase 3+ can compile until this phase is complete.

**⚠️ CRITICAL**: No user story work can begin until T006 builds cleanly.

- [ ] T002 Add `detail::LambdaTraits<MemberFnPtr>` template (two specializations: `R(L::*)(Args...) const` and `R(L::*)(Args...)`) plus `LambdaExplicitArgs<Lambda>` convenience alias to `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h` inside `namespace detail` — exposes `RetType`, `ArgTypes` (as `std::tuple<Args...>`), and `FnPtrType` (see research.md §1)

- [ ] T003 [P] Add `detail::serializeTypeToLLVM<T>(llvm::LLVMContext&)` static template and `detail::tupleToLLVMTypes<Tuple, std::index_sequence<I...>>(llvm::LLVMContext&)` to `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h` — maps C++ types to `llvm::Type*` without runtime values; handles `int`→`IntNTy`, `bool`→`Int1Ty`, `float`→`FloatTy`, `double`→`DoubleTy`, pointer/class→`PointerType::getUnqual` (see research.md §2)

- [ ] T004 Add `SpecializedLambda<R, Args...>` class to `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h` immediately after the existing `SpecializedFunction<R>` class — fields: `R(*FnPtr)(Args...)`, `llvm::orc::JITDylib* Dylib`, `llvm::orc::ExecutionSession* ES`; implements move-only semantics, `operator()(Args...)` (throws `ClangRuntimeSpecializerError` when falsy), `call(Args...)` alias, `explicit operator bool()`; destructor calls `detail::removeJITDylibNoexcept` (see data-model.md, contracts/api.md)

- [ ] T005 Implement `specializeLambdaImpl<Lambda>(const char* funcName, Lambda& lambda, const Options& opts)` private method on `ClangRuntimeSpecializer` in `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.cpp` — follows the same structure as `specializeOnlyImpl` but: (a) builds wrapper `FunctionType` using `tupleToLLVMTypes<LambdaExplicitArgs<Lambda>>`; (b) serializes `lambda` (class type → address of closure struct) as the single constant first arg; (c) creates wrapper parameters for `ExplicitArgTypes...` and forwards them in the IR `CreateCall`; (d) returns `JITResult` same as `specializeOnlyImpl` (see research.md §3, data-model.md)

- [ ] T006 Add two `specializeLambda<R>(funcName, lambda)` and `specializeLambda<R>(funcName, lambda, opts)` free function overloads to `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h` — call `ClangRuntimeSpecializer::init()->specializeLambdaImpl<Lambda>`, deduce `ExplicitArgTypes...` via `LambdaExplicitArgs<Lambda>`, return `SpecializedLambda<R, ExplicitArgTypes...>` (see contracts/api.md); add `static_assert` that `Lambda::operator()` is non-generic (fires if `LambdaTraits` cannot be instantiated)

- [ ] T007 Run `ninja ClangRuntimeSpecializer` from `llvm/llvm/build/debug`; if it fails, read the compiler errors and fix type or template instantiation issues in T002–T006 until the build is error-free and warning-free

**Checkpoint**: `ninja ClangRuntimeSpecializer` clean — proceed to user story phases.

---

## Phase 3: User Story 1 — Specialize a Lambda and Call with Variable Args (Priority: P1) 🎯 MVP

**Goal**: `specializeLambda<R>("kernel", lambda)` JIT-specializes a named kernel; the returned `SpecializedLambda` returns correct results for different explicit arg values on subsequent calls.

**Independent Test**: `ninja check-wip-runtime-specializer` passes for all three US1 WIP tests.

**Acceptance Criteria targeted**: SC-001, SC-002, SC-003 (debug + release)

### Tests for User Story 1

- [ ] T008 [US1] Write `test/WIP/specialized-lambda-basic.cpp` — single `.cpp` compiled with IRDumpingPass: define `extern "C" int add_with_ctx(Ctx* ctx, int x, bool negate) __asm__("add_with_ctx")` where `Ctx` holds a base `int` value; lambda `[&ctx](int x, bool negate){ return add_with_ctx(&ctx, x, negate); }`; call `specializeLambda<int>("add_with_ctx", lambda)`; invoke result with `(5, false)` and `(5, true)`; `printf` both results; FileCheck asserts: `INFO: Specializing call to: add_with_ctx`, correct numeric outputs for both calls, and that the JIT-optimized IR shown in debug log contains `ret i32` with a reduced instruction count compared to source (SC-001, SC-002)

- [ ] T009 [P] [US1] Write `test/WIP/specialized-lambda-void.cpp` — define `extern "C" void accumulate(State* s, int delta) __asm__("accumulate")` writing to `s->total`; lambda captures `State*`; `specializeLambda<void>("accumulate", lambda)` returns `SpecializedLambda<void, int>`; call `spec_lambda(7)` and `spec_lambda(3)`; `printf` `s->total`; FileCheck asserts the printed total is correct (SC-001)

- [ ] T010 [P] [US1] Write `test/WIP/specialized-lambda-no-captures.cpp` — captureless lambda `[](int x){ return triple(x); }` wrapping `extern "C" int triple(int x) __asm__("triple")`; `specializeLambda<int>("triple", lambda)`; call with `(4)` and `(7)`; FileCheck asserts `INFO: Specializing call to: triple` and correct results (SC-001, verifies empty-closure edge case from spec §Edge Cases)

### Debug User Story 1

- [ ] T011 [US1] Run `ninja check-wip-runtime-specializer` targeting only the three new tests; for any failure: read `.../test/WIP/Output/<test>.cpp.script` for the exact failing command, add `--dump-input=always` to the `FileCheck` line if needed, then fix the root cause in the implementation (T005/T006) or the test FileCheck patterns; repeat until all three tests pass

- [ ] T012 [US1] Verify SC-002 explicitly: in `specialized-lambda-basic.cpp`, enable `ClangRuntimeSpecializer::LogLevel::Debug` and ensure the FileCheck pattern for `ret i32 <constant>` (or reduced instruction IR) is present and matched; if the JIT pipeline did not propagate the closure constant, debug by adding `CRS_LOG(Debug, ...)` around the wrapper IR construction in `specializeLambdaImpl` and check that the closure address is correctly embedded as an `inttoptr` constant

- [ ] T013 [US1] Verify SC-003 (release build): run `ninja check-wip-runtime-specializer` from `llvm/llvm/build/release`; fix any release-only compilation errors (typically missing `assert` includes or `NDEBUG` path differences in `specializeOnlyImpl`) until all three tests pass in release too

**Checkpoint**: SC-001, SC-002, SC-003 met — `specializeLambda` end-to-end correct in debug and release.

---

## Phase 4: User Story 2 — Behavioral Equivalence Assertion (Priority: P2)

**Goal**: `assertSpecializedLambdaIsEquivalent` detects correct behaviour (positive path) and rejects a wrong kernel (negative path, SC-005).

**Independent Test**: `ninja check-wip-runtime-specializer` passes for `specialized-lambda-equivalence`.

**Acceptance Criteria targeted**: SC-005

### Implementation for User Story 2

- [ ] T014 [US2] Add `assertSpecializedLambdaIsEquivalent<R>(funcName, lambda, inputs)` free function template to `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h` — `inputs` is any range of `std::tuple<ExplicitArgTypes...>`; calls `specializeLambda<R>` once; for each element calls `std::apply(lambda, args)` and `std::apply(spec, args)` and compares with `==`; throws `ClangRuntimeSpecializerChangesBehaviorError` on mismatch; logs `INFO: assertSpecializedLambdaIsEquivalent passed for: {funcName}` on success (see contracts/api.md)

### Tests for User Story 2

- [ ] T015 [US2] Write `test/WIP/specialized-lambda-equivalence.cpp` — two sub-tests in one file: (a) **positive path**: `assertSpecializedLambdaIsEquivalent` on a correct kernel with three input tuples; FileCheck asserts `INFO: assertSpecializedLambdaIsEquivalent passed`; (b) **negative path**: a second kernel `int wrong_kernel(Ctx*, int x, bool) { return x + 1; }` where result is off-by-one; catch the thrown error and `printf` a sentinel; FileCheck asserts the sentinel is printed (SC-005)

### Debug User Story 2

- [ ] T016 [US2] Run `ninja check-wip-runtime-specializer` for `specialized-lambda-equivalence.cpp`; for failures: check that the positive path does not accidentally throw (inspect the lambda and kernel signatures match exactly), and that the negative path does throw (check that `HasEqualityOperator<R>` is satisfied for `int` — it should be); fix implementation or test until both paths pass

- [ ] T017 [US2] Verify SC-005 is unambiguously covered: confirm that FileCheck pattern for the negative-path sentinel is specific enough to not match accident output; confirm the catch block does not swallow unexpected exception types; re-run until FileCheck is deterministically green

**Checkpoint**: SC-005 met — equivalence helper is correct for both paths.

---

## Phase 5: User Story 3 — RAII Lifetime / Resource Cleanup (Priority: P3)

**Goal**: 100-iteration construction/destruction loop completes without crash or AddressSanitizer errors.

**Independent Test**: `ninja check-wip-runtime-specializer` passes for `specialized-lambda-raii` in debug (ASAN) build.

**Acceptance Criteria targeted**: SC-004

### Tests for User Story 3

- [ ] T018 [US3] Write `test/WIP/specialized-lambda-raii.cpp` — define a simple kernel `extern "C" int identity(Ctx*, int x) __asm__("identity")`; loop 100 iterations: create `SpecializedLambda<int, int>` via `specializeLambda<int>`, call it once with a counter value, let it go out of scope; after loop `printf "RAII loop done"`; FileCheck asserts `RAII loop done` appears and no crash output

### Debug User Story 3

- [ ] T019 [US3] Run `ninja check-wip-runtime-specializer` for `specialized-lambda-raii.cpp` in the debug build (ASAN is default in debug); if ASAN reports use-after-free or double-free: inspect `SpecializedLambda::cleanup()` — confirm the move constructor nulls `Dylib` and `ES` before the moved-from destructor runs; compare with `SpecializedFunction<R>::cleanup()` line-by-line and fix any divergence

- [ ] T020 [US3] Verify SC-004 quantitatively: run the RAII test 3 times and confirm no ASAN error in any run; if ASAN is suppressed in the test runner, run the test binary directly with `ASAN_OPTIONS=detect_leaks=1 ./specialized-lambda-raii.exe 1` and confirm exit code 0

**Checkpoint**: SC-004 met — JIT resources freed correctly across 100 cycles.

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Promote all passing WIP tests to the smoke suite and verify no regressions.

- [ ] T021 Promote `test/WIP/specialized-lambda-basic.cpp` → `test/smoke/specialized-lambda-basic.cpp`
- [ ] T022 [P] Promote `test/WIP/specialized-lambda-void.cpp` → `test/smoke/specialized-lambda-void.cpp`
- [ ] T023 [P] Promote `test/WIP/specialized-lambda-no-captures.cpp` → `test/smoke/specialized-lambda-no-captures.cpp`
- [ ] T024 [P] Promote `test/WIP/specialized-lambda-raii.cpp` → `test/smoke/specialized-lambda-raii.cpp`
- [ ] T025 Run `ninja check-smoke-runtime-specializer` from `llvm/llvm/build/debug`; if any previously-passing smoke test now fails, read the failure log, identify the regression, and fix it before continuing
- [ ] T026 Run `ninja check-smoke-runtime-specializer` from `llvm/llvm/build/release`; fix any release-only failures (repeat SC-003 verification for the full suite)
- [ ] T027 [P] Add a one-line doc comment above `specializeLambda` in `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h` stating: "funcName must name a kernel whose first parameter accepts a pointer to Lambda's closure struct; remaining parameters match Lambda::operator() in order."

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: No dependencies — start immediately
- **Phase 2 (Foundational)**: After Phase 1 — **BLOCKS all user stories**
- **Phase 3 (US1)**: After Phase 2 complete
- **Phase 4 (US2)**: After Phase 2 complete; independent of Phase 3 (but reading US1 tests first is useful)
- **Phase 5 (US3)**: After Phase 2 complete; independent of US1/US2
- **Phase 6 (Polish)**: After Phases 3, 4, 5 all WIP-green

### Within Phase 2 (Foundational)

```
T002 ──┐
T003 ──┤──► T004 ──► T005 ──► T006 ──► T007 (build check)
```
T002 and T003 are parallel; T004 requires T002; T005 requires T003 and T004; T006 requires T005.

### Within Each User Story Phase

```
Write tests (parallel) ──► Debug/iterate ──► Verify acceptance criteria ──► Checkpoint
```

---

## Parallel Example: Phase 2

```
# Run in parallel:
T002: LambdaTraits in ClangRuntimeSpecializer.h
T003: serializeTypeToLLVM in ClangRuntimeSpecializer.h

# Sequential after both complete:
T004: SpecializedLambda class in ClangRuntimeSpecializer.h
T005: specializeLambdaImpl in ClangRuntimeSpecializer.cpp
T006: specializeLambda free function in ClangRuntimeSpecializer.h
T007: ninja build check
```

## Parallel Example: Phase 3 (US1)

```
# Write tests in parallel after T007:
T008: specialized-lambda-basic.cpp
T009: specialized-lambda-void.cpp
T010: specialized-lambda-no-captures.cpp

# Sequential debug after tests written:
T011: run + fix failures
T012: verify SC-002 (instruction count)
T013: verify SC-003 (release build)
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Phase 1 + 2 → types compile, build clean (T001–T007)
2. Phase 3 → three WIP tests + debug loop (T008–T013)
3. **STOP**: confirm SC-001, SC-002, SC-003 all pass
4. If MVP is sufficient, skip to Phase 6 polish

### Incremental Delivery

1. Phase 1+2 → Foundation ready
2. Phase 3 → `specializeLambda` correct (SC-001, SC-002, SC-003)
3. Phase 4 → equivalence helper correct (SC-005)
4. Phase 5 → RAII safe (SC-004)
5. Phase 6 → Full smoke suite green, no regressions

---

## Notes

- All build/test commands from `llvm/llvm/build/debug` unless stated
- Never invoke `llvm-lit` directly — always use `ninja check-*` targets
- Debug tasks (T007, T011–T013, T016–T017, T019–T020, T025–T026) are **open-ended**: keep iterating until the criterion passes; do not mark complete while any FileCheck pattern or ASAN report fails
- Debug workflow: `ninja check-wip-runtime-specializer` → read `Output/<test>.cpp.script` → add `--dump-input=always` to FileCheck if needed → fix implementation → repeat
- Each debug task references specific SC-### criteria so "done" is unambiguous
