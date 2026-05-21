# Spec 014 — Pipeline 1 Budget-Aware Inlining + P1 Test Coverage

## Overview

The runtime specializer ships two JIT pipelines, selected via
`Options::OptimizationPipelineToUse` (env: `CRS_DEFAULT_PIPELINE`):

- **Pipeline 0** ("inlining"): inlines the entire specialization target into
  the wrapper, then constant-propagates and folds. For small kernels this
  collapses the wrapper to a single `ret`. For large translation units
  (sqlite3 amalgamation, DuckDB) it explodes code size — sqlite3 grows from
  234K to 3M+ instructions.
- **Pipeline 1** ("func-spec"): meant to use
  `ConstantArgFunctionSpecializationPass` to *clone* the target with
  constants baked into the clone, then inline the clone into the wrapper
  *only when the inline cost stays within a budget*. The goal is identical
  specialization on small kernels with bounded code-size growth on large
  ones.

Today Pipeline 1 is effectively a no-op: it strips linkage/inlining
attributes and returns. The fixpoint count is hardcoded to 0
(`JITPipelineFuncSpec.cpp:81`), so the func-spec pass never runs and no
inlining happens.

This spec defines the implementation of Pipeline 1 with a *cost-based,
env-var-configurable inlining budget* and the smoke-test coverage that
verifies the pipeline behaves correctly across the existing API surface.

## User Scenarios

### User Story 1 — Pipeline 1 specializes small kernels (Priority: P1)

A user calls `specializeOrFallback(mypow, 2)` (or any small extern-C kernel)
with `CRS_DEFAULT_PIPELINE=1`. The JIT-compiled wrapper folds to a single
`ret i32 9` (because `3^2 = 9`).

**Acceptance Scenarios**:

1. Given `CRS_DEFAULT_PIPELINE=1` and the `pure-specialized-function-is-equivalent.cpp`
   smoke test, when the test runs, then the wrapper IR contains `ret i32 9`.
2. Given a lambda specialization (e.g. `specialized-lambda-basic.cpp`), when
   the test runs under P1, then printed results match the P0 reference
   exactly.
3. Given any smoke test in the P1 bucket (see `details.md`), when run with
   `CRS_DEFAULT_PIPELINE=1`, then it passes its FileCheck assertions.

### User Story 2 — Pipeline 1 caps code-size growth on large modules (Priority: P1)

A user JIT-specializes a kernel whose post-specialization body is too large
to inline (e.g. `sqlite3VdbeExec`-class). Pipeline 1 must NOT inline the
clone into the wrapper if doing so would blow past the configured budget;
instead, the wrapper stays as a single call to the cspec clone.

**Acceptance Scenarios**:

1. Given a large target function, when Pipeline 1 runs, then the JIT module
   instruction count after specialization stays below a configured budget
   multiplier of the original (e.g. ≤ 2× original module size by default).
2. Given a tiny target function, when Pipeline 1 runs with default budget,
   then the wrapper inlines the cspec clone and folds to `ret <const>`.
3. Given the same large target with the budget multiplier raised via env
   var, when Pipeline 1 runs, then inlining proceeds further.

### User Story 3 — All applicable smoke tests gain P1 coverage (Priority: P1)

CI runs every smoke test under both pipelines so a regression in either is
caught.

**Acceptance Scenarios**:

1. Given the smoke-test directory, when `ninja check-smoke-runtime-specializer`
   runs, then every test in the "P1-applicable" bucket (see details.md) has
   at least one RUN line with `CRS_DEFAULT_PIPELINE=1` and passes.
2. Given the existing P0 RUN lines, when this change lands, then no P0 line
   regresses (test count and pass count under P0 unchanged).

### Edge Cases

- The wrapper is `alwaysinline`. Since it's the JIT entry, nothing inlines
  it. P1 must NOT mark the cspec clones or the target function as
  alwaysinline — that defeats the whole point.
- `ConstantArgFunctionSpecializationPass` has an "in-place specialization"
  branch (case A) when only one constant-arg group exists and there are no
  unspecialized call sites. This branch replaces arg uses in F directly
  rather than cloning. The budget-aware inliner must treat the in-place
  case the same way — cost-based inlining of `F` into the wrapper.
- Lambda specialization wraps `operator()` of a closure type with the
  closure passed by pointer. The `Pipeline1TargetFuncName` detection
  (current code at `JITPipelineFuncSpec.cpp:31-41`) walks the wrapper for
  its unique non-declaration callee. Verify this still picks up the lambda
  operator after the linkage scrub.
- Tests with strict IR-level expectations (`pure-specialized-function-is-equivalent.cpp`,
  `specialized-function-is-equivalent.cpp`, `struct-arg-hide.cpp`) must
  continue to assert the same final IR shape under P1 with the default
  budget (their kernels are small).
- Tests covering P0-only mechanisms (`DevirtualizeConstantVtableCallsPass`,
  `InvariantLoadToConstantPass`, `StaticMutabilityAnalysisPass`) are P0-only
  and intentionally have no P1 RUN line.
- Debug-build `pImpl->ValueHandles` corruption (spec 005) is a known LLVM
  bug for some FunctionPassManager passes on large modules. P1's intra-clone
  cleanup passes (InstCombine, SimplifyCFG) must respect the same
  `IsLargeModule` guard P0 uses.

## Functional Requirements

- **FR-001**: Pipeline 1 MUST run `ConstantArgFunctionSpecializationPass`
  when `Opts.MaxFixpointIterations > 0`.
- **FR-002**: Pipeline 1 MUST NOT set `AlwaysInline` on the cspec clones
  produced by `ConstantArgFunctionSpecializationPass`, nor on the target
  function in the in-place specialization case.
- **FR-003**: Pipeline 1 MUST use LLVM's cost-based inliner with a
  configurable threshold to decide whether to inline a cspec clone into the
  wrapper. The wrapper itself remains `alwaysinline` and is not affected.
- **FR-004**: The inliner threshold MUST be configurable via env var
  `CRS_DEFAULT_P1_INLINE_THRESHOLD` (default: LLVM's O3 default of 225).
- **FR-005**: A complementary module-size growth cap MUST be configurable
  via env var `CRS_DEFAULT_P1_MAX_MODULE_GROWTH` (default: 2.0 = no more
  than 2× the pre-fixpoint module size). When exceeded, the fixpoint loop
  terminates early.
- **FR-006**: Pipeline 1 MUST honor `Opts.MaxFixpointIterations` and
  `Opts.LoopUnrollCount` (the unroll cap also acts as a budget knob inside
  cspec clones).
- **FR-007**: Pipeline 1 MUST run a final small cleanup pass set
  (InstCombine, SimplifyCFG, GlobalDCE) but MUST NOT run the full LLVM O3
  pipeline — O3 contains its own inliner that would defeat the budget.
- **FR-008**: Every smoke test under `test/smoke/` that exercises
  `specializeOrFallback`, `specializeLambda`, `callSpecialized`, or
  `assertSpecializedIsEquivalent` and does NOT depend on a P0-only pass
  (`DevirtualizeConstantVtableCallsPass`, `InvariantLoadToConstantPass`,
  `StaticMutabilityAnalysisPass`) MUST gain a `CRS_DEFAULT_PIPELINE=1` RUN
  line and pass.
- **FR-009**: Tests in the "P0-only" bucket (see `details.md` for the full
  list) MUST be left unchanged.
- **FR-010**: The new env vars MUST be documented in the same comment block
  as the other `CRS_DEFAULT_*` env vars in `ClangRuntimeSpecializer.h`.
- **FR-011**: The existing warning at
  `ClangRuntimeSpecializer.cpp:487-491` claiming
  "MaxFixpointIterations > 1 has no effect on the func-spec pipeline" MUST
  be removed since this change makes it have an effect.

## Success Criteria

- **SC-001**: `ninja check-smoke-runtime-specializer` is green with all P1
  RUN lines included (target: ≥ 95% of P1 RUN lines pass; ideally 100%).
- **SC-002**: `pure-specialized-function-is-equivalent.cpp` with
  `CRS_DEFAULT_PIPELINE=1` produces `ret i32 9` in the wrapper IR.
- **SC-003**: On a synthetic large module (≥ 50k instructions), Pipeline 1
  with default budget keeps the post-fixpoint module instruction count
  ≤ 2× the pre-fixpoint count. (Implementation note: this can be verified
  via a small lit test that prints `getLastTransformStats()` counts.)
- **SC-004**: P0 smoke-test pass rate is unchanged after this work lands
  (no P0 RUN line regresses).
- **SC-005**: Increasing `CRS_DEFAULT_P1_INLINE_THRESHOLD` from the default
  to a large value (e.g. 10000) on a small kernel produces identical IR to
  the default — i.e. small kernels already inline at any reasonable
  threshold.

## Clarifications

### Session 2026-05-21

- **Q**: Should Pipeline 1 mark the cspec clones as `alwaysinline` so the
  wrapper absorbs them via `AlwaysInlinerPass`?
  **A**: No. Always-inlining defeats P1's purpose (avoiding code-size
  explosion). Use the cost-based inliner with an env-var-configurable
  budget instead. The wrapper itself stays `alwaysinline` because it's the
  JIT entry symbol, not because it gets inlined into anything.

- **Q**: Should every smoke test gain a P1 RUN line, or only a
  representative subset?
  **A**: All applicable smoke tests. Tests that depend on P0-only passes
  (devirt, invariant-load, static-mutability) stay P0-only.

- **Q**: Should the FR-005 module-size growth cap be a hard limit (abort
  inlining mid-iteration) or a soft check (terminate the fixpoint after
  the current iteration)?
  **A**: Soft. Terminate the next fixpoint iteration; don't try to
  partially unwind an in-progress inliner pass. Simpler, and the cost-based
  per-callsite threshold is the primary budget mechanism anyway.
