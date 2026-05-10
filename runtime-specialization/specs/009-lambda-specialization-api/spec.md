# Feature Specification: Lambda Specialization API

**Feature Branch**: `WIP-specialization/009-lambda-specialization-api`  
**Created**: 2026-05-10  
**Status**: Draft

## Overview

This feature adds a `specializeLambda` free function that lets a caller specialize a
lambda whose captures represent a fixed "context" (e.g., a configured object instance,
a compiled predicate, a fixed schema) while keeping the lambda's explicit arguments
variable at every call site.

The result is a `SpecializedLambda<R, ExplicitArgTypes...>` — a RAII, move-only
callable that owns its JIT-compiled machine code and frees it on destruction.  Its
`operator()` has exactly the same explicit-argument signature as the original lambda.

The API fits naturally alongside the existing `specializeOnly` / `callSpecialized`
primitives: the caller still names the underlying kernel function (`funcName`), so the
TU-separation contract is preserved and the IRDumpingPass blob registration mechanism
is unchanged.

---

## User Scenarios & Testing *(mandatory)*

### User Story 1 — Specialize a Lambda and Call with Variable Args (Priority: P1)

A developer has a kernel function in a dedicated TU and a lambda in the calling TU that
closes over a fixed context (e.g., a data-structure instance) while forwarding the
remaining arguments as explicit parameters.  They call `specializeLambda` once (paying
the JIT overhead) and then call the returned `SpecializedLambda` repeatedly with
different explicit arguments — without re-paying the compilation cost each time.

**Why this priority**: This is the sole purpose of the new API.  Everything else builds
on this basic usage path being correct and ergonomic.

**Independent Test**: A smoke test calls `specializeLambda<R>("kernel", lambda)`,
then invokes the result with several different `(x, y)` inputs and asserts that each
return value equals the result of calling the original lambda directly.

**Acceptance Scenarios**:

1. **Given** a kernel function `int kernel(SomeType* ctx, int x, bool y)` in a kernel
   TU (compiled with IRDumpingPass), and a lambda `[&inst](int x, bool y) { return
   kernel(&inst, x, y); }` in the calling TU, **When** `specializeLambda<int>("kernel",
   lambda)` is called, **Then** it returns a valid `SpecializedLambda<int, int, bool>`
   without throwing.

2. **Given** a valid `SpecializedLambda<int, int, bool> spec`, **When** `spec(42, false)`
   and `spec(7, true)` are called on the same object, **Then** each call returns the
   same integer that `lambda(42, false)` resp. `lambda(7, true)` would return.

3. **Given** a lambda with no captures, **When** `specializeLambda` is called,
   **Then** it succeeds and the result is callable (no constants are baked in, but the
   pipeline still runs at least O3).

4. **Given** a lambda returning `void`, **When** `specializeLambda<void>` is called
   and the result is invoked, **Then** no exception is thrown and any side effects
   observable through a shared output parameter match those of the original lambda.

---

### User Story 2 — Behavioral Equivalence Assertion (Priority: P2)

A developer uses `assertSpecializedIsEquivalent` (or a lambda-aware equivalent) to
automatically verify that the JIT-specialized path produces identical results to the
original lambda for a set of reference inputs before any benchmark runs.

**Why this priority**: Correctness must be validated programmatically, not just by
inspection, before performance numbers are trusted.  This story is independent of
Story 1 and can be implemented as a thin wrapper over the existing assertion helpers.

**Independent Test**: A smoke test calls `assertSpecializedLambdaIsEquivalent` with a
reference input set and verifies it completes without throwing
`ClangRuntimeSpecializerChangesBehaviorError`.

**Acceptance Scenarios**:

1. **Given** a kernel that computes a deterministic function of its arguments, **When**
   `assertSpecializedLambdaIsEquivalent("kernel", lambda, referenceInputs)` is called,
   **Then** it completes without error.

2. **Given** a kernel that has been intentionally mutated to return a wrong value,
   **When** `assertSpecializedLambdaIsEquivalent` is called, **Then** it throws
   `ClangRuntimeSpecializerChangesBehaviorError`.

---

### User Story 3 — RAII Lifetime / Resource Cleanup (Priority: P3)

A developer creates multiple `SpecializedLambda` objects in a loop (e.g., re-specializing
whenever the fixed context changes) and expects each old specialization to be freed when
the corresponding `SpecializedLambda` goes out of scope, with no JIT dylib leak.

**Why this priority**: JIT code consumes mapped memory; leaking it across many
specialization cycles would make long-running processes unusable.  This story is
verifiable independently of the call-correctness story above.

**Independent Test**: Create and destroy a `SpecializedLambda` inside a loop; verify
(via `/proc/self/status` VmRSS or `RSSMemoryManager`) that resident memory does not
grow unboundedly across iterations.

**Acceptance Scenarios**:

1. **Given** a loop that creates a `SpecializedLambda`, calls it once, and then lets it
   go out of scope 100 times, **When** the loop completes, **Then** the net increase in
   resident-set memory is below a 10 MB threshold.

2. **Given** a moved-from `SpecializedLambda` (after `std::move`), **When** it is
   destroyed, **Then** no double-free or use-after-free is observed (verified with
   AddressSanitizer in a debug build).

---

### Edge Cases

- Lambda that captures by value vs. by reference: by-reference captures are
  serialized as pointer constants (the address of the referent); by-value captures are
  serialized as value constants (the copy stored in the closure struct).  The caller
  must ensure captured referents remain alive for the lifetime of the `SpecializedLambda`.

- Lambda with multiple captures of mixed types (pointer + integer + float): all
  captures must be serializable by the existing `serializeArgumentToIR` logic;
  non-serializable capture types (e.g., `std::string`) produce a
  `ClangRuntimeSpecializerArgSerializationError`.

- Lambda whose kernel TU is not compiled with IRDumpingPass (i.e., the function is
  absent from all registered blobs): `specializeLambda` throws
  `ClangRuntimeSpecializerDumpedIRError`, consistent with existing behavior of
  `specializeOnly`.

- `SpecializedLambda` that timed out (when `Options::JITTimeoutSeconds` is set): the
  returned object is falsy (`!spec == true`); calling it should throw or be undefined
  — the accepted behavior is to throw `ClangRuntimeSpecializerError`.

- Generic lambda (`auto` params): not supported in this version; the caller must use
  a concrete lambda type whose `operator()` has a fixed signature.

---

## Requirements *(mandatory)*

### Functional Requirements

**specializeLambda API**

- **FR-001**: A free function template `specializeLambda<R>(funcName, lambda)` and an
  overload `specializeLambda<R>(funcName, lambda, opts)` MUST be provided in the
  `clangRuntimeSpecializer` namespace, following the same overload-disambiguation
  pattern used by `specializeOnly`.

- **FR-002**: The `funcName` parameter MUST identify the target kernel function in the
  IR blob using the same lookup mechanism as `specializeOnly` / `callSpecialized`.

- **FR-003**: The lambda's captured state (its closure struct) MUST be serialized as
  JIT constants, using the same `serializeArgumentToIR` infrastructure used by
  `specializeOnlyImpl`.

- **FR-004**: The lambda's explicit argument types (deduced from its `operator()`
  signature) MUST define the parameter types of the returned specialized callable; they
  MUST NOT be serialized as JIT constants.

- **FR-005**: `specializeLambda` MUST return a `SpecializedLambda<R, ExplicitArgTypes...>`
  where `R` is the template argument and `ExplicitArgTypes...` is deduced from the
  lambda's `operator()`.

**SpecializedLambda type**

- **FR-006**: `SpecializedLambda<R, Args...>` MUST provide `operator()(Args...)` that
  invokes the JIT-compiled specialized function and returns `R`.

- **FR-007**: `SpecializedLambda<R, Args...>` MUST be move-only (copy-construction and
  copy-assignment deleted), consistent with `SpecializedFunction<R>`.

- **FR-008**: The destructor of `SpecializedLambda<R, Args...>` MUST release the
  associated JITDylib, freeing the JIT-compiled machine code.

- **FR-009**: `SpecializedLambda<R, Args...>` MUST provide `explicit operator bool()`
  returning `false` when the object was default-constructed or moved-from, and `true`
  when it holds a valid specialization.

- **FR-010**: A default-constructed or timed-out `SpecializedLambda` MUST be callable
  without undefined behavior; it MUST throw `ClangRuntimeSpecializerError` if invoked
  while falsy.

**Equivalence assertion**

- **FR-011**: A helper function `assertSpecializedLambdaIsEquivalent<R>(funcName,
  lambda, inputTuples)` MUST be provided that, for each input tuple, compares the
  result of calling `lambda(args...)` against the result of calling
  `spec_lambda(args...)` and throws `ClangRuntimeSpecializerChangesBehaviorError` on
  mismatch.

**Tests**

- **FR-012**: At least three smoke tests covering: (a) basic call correctness, (b) void
  return, and (c) no-capture lambda MUST be added to the project's smoke test suite and
  pass under `ninja check-smoke-runtime-specializer` in debug and release builds.

- **FR-013**: At least one smoke test MUST verify RAII cleanup by constructing and
  destroying a `SpecializedLambda` inside a loop and checking that the process does not
  crash or trigger AddressSanitizer errors.

### Key Entities

- **`specializeLambda<R>`**: Free function template that performs partial JIT
  specialization of a lambda — captures become constants, explicit args remain variable.

- **`SpecializedLambda<R, Args...>`**: RAII, move-only callable wrapping a JIT-compiled
  function pointer and owning the associated JITDylib.  Analogous to
  `SpecializedFunction<R>` but parameterized on the explicit argument types.

- **Kernel function**: A named C function in a kernel TU (compiled with IRDumpingPass)
  whose first parameters correspond to the lambda's captures and whose remaining
  parameters correspond to the lambda's explicit args.

- **Closure constant**: The serialized value of a lambda capture, embedded as an LLVM
  IR `Constant` in the JIT wrapper function.

---

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: For every smoke test input pair `(lambda, (x, y))`, `spec_lambda(x, y)`
  returns the same value as `lambda(x, y)` — zero behavioral regressions across all
  test cases.

- **SC-002**: The instruction count of the JIT-compiled specialized function (measured
  via `getLastTransformStats`) is strictly lower than the instruction count of the
  unspecialized kernel for at least one smoke test fixture that captures a non-trivial
  context (demonstrating that constant propagation fired).

- **SC-003**: All three required smoke tests pass under both debug and release builds
  (`ninja check-smoke-runtime-specializer`) with no failures.

- **SC-004**: Constructing and destroying a `SpecializedLambda` in a 100-iteration loop
  does not cause AddressSanitizer to report any errors in a debug build.

- **SC-005**: The `assertSpecializedLambdaIsEquivalent` helper correctly raises
  `ClangRuntimeSpecializerChangesBehaviorError` when given a kernel that returns a
  deliberately wrong value — verified by at least one negative-path test.

---

## Assumptions

- The kernel function's parameter order must match the split: the prefix parameters
  corresponding to captures come first, and the parameters matching the lambda's
  explicit args come after.  This ordering convention is the caller's responsibility;
  the API does not validate it.

- Only lambdas with a single, non-overloaded, non-generic `operator()` are supported;
  generic lambdas (`auto` params) are out of scope.

- All captured types must be serializable by the existing `serializeArgumentToIR`
  infrastructure (integral, floating-point, pointer, or struct-by-address).  Captures
  of types that do not meet this constraint (e.g., `std::string`, non-trivially-copyable
  objects passed by value) are out of scope; the error behavior is consistent with
  existing serialization failures.

- The kernel TU must follow the existing TU-separation pattern: the kernel function is
  in a `.cpp` file compiled with the IRDumpingPass plugin, with no Google Benchmark or
  specializer benchmark headers included.

- The referent of any by-reference capture must remain alive for the entire lifetime of
  the `SpecializedLambda`; the API does not extend the lifetime of captured references.

- `SpecializedLambda` is single-threaded (not safe to call `operator()` concurrently
  from multiple threads on the same instance), consistent with the project-wide
  single-threaded execution constraint.

- The implementation reuses `specializeOnlyImpl` internally; no new JIT infrastructure
  is introduced.  The key difference from `specializeOnly` is that the JIT wrapper
  function takes `ExplicitArgTypes...` as parameters rather than zero parameters, and
  `SpecializedLambda::operator()` passes them through.
