# Feature Specification: Eliminate Manual ASM Names from Specialization API

**Feature Branch**: `010-eliminate-asm-names`  
**Created**: 2026-05-10  
**Status**: Draft  
**Input**: User description: "The current runtime specializer API requires that functions are registered with a __asm__ name, which is then used to lookup the function. This process is error prone and fundamentally limits the usability of specializeLambda, so in this spec I want to specify that: The API should NOT require manual asm names anymore. The Compile Time Passes should embed a reference to specialization calls so that at runtime the right function can be resolved."

## Overview

The current `specializeLambda` API (spec 009) requires the caller to provide a `funcName`
string that identifies the kernel function in the JIT blob.  The `callSpecialized` and
`specializeOnly` APIs have the same requirement.  These strings must match functions'
mangled / `__asm__`-annotated names exactly, which is error-prone and forces an artificial
separation between "kernel TU" and "calling TU".  For `specializeLambda` specifically, the
constraint also prevents the lambda body from containing anything other than a single call
to the named kernel.

This feature removes the `funcName` parameter from all three APIs.  The compile-time
IR-dumping pass detects every call to `specializeLambda`, `specializeOnly`, and
`callSpecialized` in the compiled translation unit, determines the identity of the target
function from the IR (the lambda's `operator()` or the named function pointer), and embeds
that identity in the binary as a compile-time constant.  At runtime the specializer uses
the embedded identity to look up the correct IR from the registered blob — no `funcName`
string, no `__asm__` attribute, no manual registration.

As a result, the lambda body may contain arbitrary logic (loops, conditionals, calls to
multiple methods) without restriction, and regular function specialization requires nothing
more than passing a function pointer.  The caller writes:

```cpp
auto spec  = specializeLambda<R>(lambda);           // lambda body is the target
auto* fptr = specializeOnly<R>(&myFunc, arg1, arg2); // named function, args are constants
callSpecialized<R>(&myFunc, arg1, arg2);             // same, calls immediately
```

**Unified code path**: a zero-argument lambda (one with no explicit parameters, only
captures) is functionally equivalent to `specializeOnly` called with no remaining variable
arguments — both specialize a callable with all inputs baked in as constants.  The runtime
implementation MUST route both cases through the same underlying JIT compilation path so
that no behavioral difference exists between them.

---

## User Scenarios & Testing *(mandatory)*

### User Story 1 — Specialize a Complex Lambda Without Naming Its Target (Priority: P1)

A developer wants to specialize a lambda that contains real logic — not just a single
call to a named kernel.  Under the old API, this was impossible without wrapping the
entire body in a named free function; under the new API, the developer simply passes the
lambda directly with no `funcName`.

**Why this priority**: This is the primary motivation for the spec.  Everything else
(error cases, migration) is secondary to making this basic usage pattern work.

**Independent Test**: A smoke test defines a lambda with a loop and a branch that calls
two different methods of a captured object, calls `specializeLambda<int>(lambda)`, invokes
the result with several argument combinations, and asserts that each return value matches
direct invocation of the original lambda.

**Acceptance Scenarios**:

1. **Given** a lambda `[&obj](int x, bool y){ for(int i=0;i<x;i++){if(y) obj.foo(i,x); else obj.bar(i,x);} return x; }` compiled with the IR-dumping plugin, **When** `specializeLambda<int>(lambda)` is called, **Then** it returns a valid, callable `SpecializedLambda<int, int, bool>` without error.

2. **Given** a valid `SpecializedLambda<int, int, bool> spec` from the scenario above, **When** `spec(5, true)` and `spec(5, false)` are called, **Then** both return the same integer as direct invocation of `lambda(5, true)` and `lambda(5, false)` respectively.

3. **Given** a lambda that captures nothing and performs a simple computation, **When** `specializeLambda<int>(lambda)` is called, **Then** the call succeeds and the result is callable.

4. **Given** a lambda returning `void` with side effects through a captured reference, **When** `specializeLambda<void>(lambda)` is called and the result is invoked, **Then** the side effects match those of direct lambda invocation.

---

### User Story 2 — Automatic Name Resolution at Compile Time (Priority: P1)

A developer calls `specializeLambda` without annotating any function with `__asm__` or
manually registering a `funcName`.  The compile-time pass handles all name resolution
transparently, so the developer never needs to know the lambda's mangled name.

**Why this priority**: Ergonomics and correctness are co-equal concerns here.  A system
where the developer must track and spell mangled names will accumulate silent mismatches
over time.  Automatic resolution at compile time is the only approach that is both
ergonomic and provably correct.

**Independent Test**: A smoke test containing a `specializeLambda` call compiles and runs
correctly without any `__asm__` attribute, `funcName` string, or call to a named kernel
function — verified by grep on the test source.

**Acceptance Scenarios**:

1. **Given** source code that calls `specializeLambda<R>(lambda)` with no `funcName` argument and no `__asm__` annotation anywhere, **When** the source is compiled with the IR-dumping plugin and the binary is run, **Then** the specialization succeeds and produces correct results.

2. **Given** a lambda defined inline at the call site (not assigned to a named variable prior to the call), **When** `specializeLambda` is called with it, **Then** the compile-time pass still resolves the identity correctly and the binary runs without error.

3. **Given** two different lambdas defined in the same translation unit, **When** each is passed to `specializeLambda` separately, **Then** each specialization resolves to its own `operator()` independently, with no cross-contamination.

---

### User Story 3 — Normal Function Specialization Without a Name String (Priority: P2)

A developer wants to specialize an ordinary named function (not a lambda) without
providing a `funcName` string.  They pass a function pointer directly; the compile-time
pass resolves and embeds its name.  Zero-argument lambdas and ordinary function
specialization with all arguments baked in travel the same runtime code path and are
interchangeable in behavior.

**Why this priority**: Consistency across the whole API surface.  If lambdas no longer
need `funcName`, named functions should not either.  The unified code path also removes
a hidden branch that could otherwise diverge in behavior over time.

**Independent Test**: A smoke test calls both `specializeOnly<int>(&computeFoo, 42)` and
`specializeLambda<int>([]{return computeFoo(42);})` on an equivalent function, and verifies
that both produce the same output and that neither call site contains a `funcName` string.
A second test confirms that a 0-argument lambda and `specializeOnly` with fully-baked
arguments go through the same JIT wrapper code path (verified via pass trace counts being
identical for structurally identical inputs).

**Acceptance Scenarios**:

1. **Given** a free function `int computeFoo(int x)` compiled with the IR-dumping plugin, **When** `specializeOnly<int>(&computeFoo, 42)` is called with no `funcName` argument, **Then** a specialized function pointer is returned that, when called, produces the same result as `computeFoo(42)`.

2. **Given** a zero-argument lambda `[x=42](){ return computeFoo(x); }` equivalent to the above, **When** `specializeLambda<int>(lambda)` is called, **Then** the returned `SpecializedLambda` produces the same result and the JIT transform statistics (instruction count, pass count) are identical to those of scenario 1.

3. **Given** both a named function and a zero-argument lambda that encode the same computation, **When** each is specialized separately, **Then** the specializer's internal code path (observable via `getLastPassTrace`) shows no structural difference — both invoke the same sequence of JIT passes.

---

### User Story 4 — Clear Error When IR Is Absent (Priority: P3)

A developer accidentally calls `specializeLambda` or `specializeOnly` on a function whose
translation unit was not compiled with the IR-dumping plugin.  The system detects the
missing IR at runtime and reports a clear error rather than silently producing wrong
results or crashing.

**Why this priority**: The absence-of-plugin error already exists for the current
`funcName`-based path; this story ensures the new no-`funcName` path reports it with equal
clarity for both lambda and regular-function call sites.

**Independent Test**: A smoke test links both a `specializeLambda` call and a
`specializeOnly` call against a TU compiled *without* the plugin and asserts that both
throw (or return a falsy result) with a diagnostic identifying the missing blob.

**Acceptance Scenarios**:

1. **Given** a lambda in a TU compiled without the IR-dumping plugin, **When** `specializeLambda` is called on that lambda, **Then** a `ClangRuntimeSpecializerDumpedIRError` is thrown (or the returned `SpecializedLambda` is falsy) with a message that identifies the unresolved function.

2. **Given** a named function in a TU compiled without the IR-dumping plugin, **When** `specializeOnly` is called with its address, **Then** the same `ClangRuntimeSpecializerDumpedIRError` is raised — error handling is identical for lambdas and regular functions.

3. **Given** a TU compiled *with* the plugin but a binary that omitted the plugin-generated initialization, **When** any of the specialization APIs is called, **Then** `ClangRuntimeSpecializerDumpedIRError` is raised (consistent behavior regardless of the failure reason).

---

### Edge Cases

- Lambda defined inside a template function: the compile-time pass must still resolve the
  lambda's unique `operator()` instance for each template instantiation that appears in
  the translation unit.

- Lambda passed to `specializeLambda` through an intermediate variable vs. at the call
  site: both forms must resolve to the same `operator()`.

- Two translation units each defining structurally identical but distinct lambda types:
  each must resolve to its own `operator()` with no collision (guaranteed by C++ ODR —
  lambda closure types are always local to their TU).

- Lambda that captures a `std::string` or other non-serializable type: the error
  `ClangRuntimeSpecializerArgSerializationError` is raised, consistent with existing
  behavior for unserializable captures under the old API.

- Zero-argument lambda vs. `specializeOnly` with all args baked: both produce a callable
  that takes no remaining variable arguments.  The JIT wrapper in both cases calls the
  target with only constant operands; the runtime MUST route them through the same
  `specializeOnlyImpl` entry point.

- Named function passed as a pointer to `specializeOnly` or `callSpecialized`: the
  compile-time pass must unambiguously map the function pointer to the IR function in the
  blob.  If the function is overloaded or a template, only the instantiation whose address
  is taken is resolved.

- Translation unit compiled without the IR-dumping plugin: error is
  `ClangRuntimeSpecializerDumpedIRError` for all three APIs, same as today for missing
  blobs.

---

## Requirements *(mandatory)*

### Functional Requirements

**API changes**

- **FR-001**: A `specializeLambda<R>(lambda)` overload and a
  `specializeLambda<R>(lambda, opts)` overload MUST be provided, replacing the existing
  `specializeLambda<R>(funcName, lambda)` and `specializeLambda<R>(funcName, lambda, opts)`
  overloads that take an explicit `funcName` string.

- **FR-002**: `specializeOnly` and `callSpecialized` MUST gain overloads that accept a
  typed function pointer as the first argument instead of a `funcName` string — e.g.,
  `specializeOnly<R>(&myFunc, args...)` and `callSpecialized<R>(&myFunc, args...)`.  The
  old `funcName`-string overloads MUST be removed.

- **FR-003**: The `funcName` string parameter MUST be absent from all revised overloads;
  the function identity MUST be determined solely from information embedded at compile
  time — no `__asm__` attribute, no string literal, no runtime lookup by name in calling
  code.

- **FR-004**: The returned type of `specializeLambda` MUST remain
  `SpecializedLambda<R, ExplicitArgTypes...>`, preserving all existing guarantees (RAII,
  move-only, `operator bool`, exception on invocation of a falsy object) established in
  spec 009.

- **FR-005**: A zero-argument lambda (lambda with no explicit parameters, only captures)
  MUST travel the same runtime code path as `specializeOnly<R>(&f, args...)` when all
  variable arguments are baked in.  The runtime MUST NOT contain a special branch that
  distinguishes these two cases after the function identity has been resolved.

**Compile-time pass changes**

- **FR-006**: The IR-dumping pass MUST detect every call to `specializeLambda`,
  `specializeOnly`, and `callSpecialized` within the translation unit being compiled and,
  for each such call, determine the mangled name of the target function (lambda's
  `operator()` or named function pointer) from the IR.

- **FR-007**: The detected function identity MUST be embedded in the binary as a
  compile-time constant (e.g., a string literal in a static global), associated with the
  call site, so that the runtime specializer can retrieve it without receiving it as a
  parameter from calling code.

- **FR-008**: The embedded identity information MUST survive standard linking and stripping
  steps (i.e., it MUST NOT rely solely on debug symbols).

- **FR-009**: The existing per-TU blob registration mechanism (bitcode blob + function
  index) MUST continue to work unchanged; the new compile-time embedding is an additional
  annotation layered on top, not a replacement for the blob.

**Runtime behavior**

- **FR-010**: When any of the revised specialization APIs is called at runtime, the
  specializer MUST retrieve the function identity from the compile-time-embedded annotation
  and use it to locate the target IR in the registered blobs, without requiring the caller
  to supply the name.

- **FR-011**: A zero-argument lambda and a `specializeOnly` call with all arguments baked
  in MUST produce identical JIT wrapper IR structure when the target functions encode the
  same computation — verified by comparing pass trace output.

- **FR-012**: If no compile-time annotation is found for the call site (e.g., because the
  TU was compiled without the plugin), the specializer MUST throw
  `ClangRuntimeSpecializerDumpedIRError` with a diagnostic identifying the unresolved
  function — for both lambda and named-function call sites.

- **FR-013**: All correctness guarantees of `SpecializedLambda` and `specializeOnly`
  (behavioral equivalence, RAII cleanup, `operator bool` semantics) MUST be preserved
  unchanged.

**Tests**

- **FR-014**: At least three smoke tests covering: (a) a lambda with a loop and a branch
  calling two methods, (b) a capture-free lambda, and (c) a `void`-returning lambda MUST
  pass under `ninja check-smoke-runtime-specializer` in debug and release builds.

- **FR-015**: At least one smoke test MUST verify that a zero-argument lambda and an
  equivalent `specializeOnly<R>(&f)` call produce the same output and the same JIT pass
  trace, confirming the unified code path.

- **FR-016**: At least one smoke test MUST verify the missing-plugin error path for both a
  lambda call site and a named-function call site — both must produce
  `ClangRuntimeSpecializerDumpedIRError`.

- **FR-017**: At least one smoke test MUST verify that two distinct lambdas in the same TU
  each specialize to their own `operator()` independently, with no cross-contamination.

### Key Entities

- **`specializeLambda<R>(lambda [, opts])`**: Revised free function template. Accepts the
  lambda directly; derives function identity from compile-time-embedded annotation.

- **`specializeOnly<R>(&func, args...)`** / **`callSpecialized<R>(&func, args...)`**:
  Revised overloads accepting a typed function pointer instead of a `funcName` string;
  identity resolved from compile-time annotation.

- **`SpecializedLambda<R, Args...>`**: Unchanged from spec 009 — RAII, move-only callable
  wrapping a JIT-compiled function pointer.

- **Compile-time identity annotation**: A static global (or equivalent) embedded by the
  IR-dumping pass that associates each specialization call site with the mangled name of
  the target function (lambda `operator()` or named function).

- **Unified JIT entry point**: The single `specializeOnlyImpl` code path that handles both
  zero-argument lambdas and named-function specializations with all args baked — no
  diverging branch between the two.

- **Closure / argument constants**: The serialized values of lambda captures or function
  arguments, embedded as LLVM IR `Constant`s in the JIT wrapper. Resolution and
  serialization behavior are unchanged.

---

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Every smoke test invocation via `SpecializedLambda` or a `specializeOnly`
  function pointer returns the same value as direct invocation of the original callable —
  zero behavioral regressions across all test cases.

- **SC-002**: Source code for all smoke tests contains no `__asm__` attribute and no
  `funcName` string literal passed to any specialization API — verified by automated grep
  in the test suite.

- **SC-003**: All required smoke tests (FR-014 through FR-017) pass under both debug and
  release builds (`ninja check-smoke-runtime-specializer`) with no failures.

- **SC-004**: A lambda containing a loop and two conditional method calls specializes with
  a lower instruction count (measured via `getLastTransformStats`) than the unspecialized
  `operator()` when the captures are non-trivially constant, demonstrating that constant
  propagation fired on the complex body.

- **SC-005**: A zero-argument lambda and a `specializeOnly` call encoding the same
  computation produce an identical JIT pass trace (same pass sequence and instruction
  counts), confirming the unified runtime code path.

- **SC-006**: Calling any specialization API on a function from a TU compiled without the
  plugin produces `ClangRuntimeSpecializerDumpedIRError` (or a falsy result) in 100% of
  test runs — no silent success, no crash.

---

## Assumptions

- The compile-time pass can unambiguously identify the lambda's `operator()` from the IR
  at the `specializeLambda` call site. C++ lambdas have unique, anonymous closure types,
  so each lambda's `operator()` has a unique mangled name within a TU; across TUs, ODR
  guarantees there is no collision.

- For named functions passed by pointer, the compile-time pass can determine the callee's
  mangled name from the function pointer argument in the IR.  Overloaded or templated
  functions are resolved to the specific instantiation whose address is taken.

- A zero-argument lambda (all inputs as captures) and `specializeOnly` with all arguments
  baked are semantically equivalent: in both cases the JIT wrapper takes zero variable
  parameters and the result is a zero-arg callable.  The existing `specializeOnlyImpl`
  infrastructure is sufficient to handle both without a new code path.

- The binary retains the compile-time annotations in sections that survive standard link
  and strip. If the project's build pipeline strips all non-essential sections, the
  embedding strategy will need to use a section that is exempt from stripping (e.g., the
  same section used by existing blob registration globals).

- Lambdas whose `operator()` is inlined away entirely by the compiler before the
  IR-dumping pass runs are out of scope; the pass runs before inlining, so this is not
  expected to occur in practice with the current plugin invocation order.

- Generic lambdas (`auto` parameters) remain out of scope, consistent with spec 009.

- All old overloads that take an explicit `funcName` string — for `specializeLambda`,
  `specializeOnly`, and `callSpecialized` — are removed; no backwards-compatibility shim
  is provided, consistent with the project's no-legacy-support policy.

- The lambda or target function may be arbitrarily complex internally (loops, branches,
  multiple method calls) as long as all baked-in inputs are serializable by the existing
  `serializeArgumentToIR` infrastructure.

- TU separation is still recommended for large projects to keep blob sizes manageable,
  but is no longer architecturally required by the API — a lambda defined in the same TU
  as its call site will work correctly because the IR-dumping pass captures the entire TU.
