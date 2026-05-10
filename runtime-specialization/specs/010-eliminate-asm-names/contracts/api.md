# API Contract: Eliminate Manual ASM Names

**Feature**: 010-eliminate-asm-names  
**Date**: 2026-05-10  
**Namespace**: `clangRuntimeSpecializer`

---

## Overview

All three specialization entry points lose the `const char* funcName` first argument.
`callSpecialized` and `specializeOnly` gain a typed function-pointer first argument
instead.  `specializeLambda` takes only the lambda (no pointer, no name).

The function-pointer variants are constrained to compile-time-constant callees; the
IRDumpingPass rewrites these call sites at compile time to inject the resolved mangled
name.

---

## Public API Changes

### `callSpecialized`

**Before** (removed):
```cpp
template <class R, class... Args>
R callSpecialized(const char* funcName, Args&&... args);

template <class R, class... Args>
R callSpecialized(const char* funcName, const Options& opts, Args&&... args);
```

**After** (new):
```cpp
// func must be a compile-time-constant function pointer (resolved by IRDumpingPass).
// The pass rewrites this call to callSpecializedResolved(resolvedName, func, args...).
template <class R, class F, class... Args>
R callSpecialized(F* func, Args&&... args);

template <class R, class F, class... Args>
R callSpecialized(F* func, const Options& opts, Args&&... args);
```

**Semantics**: Identical to before.  `func` is only present to allow the pass to extract
the function name; it is not called directly.  All arguments in `args...` are serialized
as JIT constants.

---

### `specializeOnly`

**Before** (removed):
```cpp
template <class R, class... Args>
SpecializedFunction<R> specializeOnly(const char* funcName, Args&&... args);

template <class R, class... Args>
SpecializedFunction<R> specializeOnly(const char* funcName, const Options& opts, Args&&... args);
```

**After** (new):
```cpp
// func must be a compile-time-constant function pointer (resolved by IRDumpingPass).
template <class R, class F, class... Args>
SpecializedFunction<R> specializeOnly(F* func, Args&&... args);

template <class R, class F, class... Args>
SpecializedFunction<R> specializeOnly(F* func, const Options& opts, Args&&... args);
```

**Semantics**: Identical to before.  Returns a `SpecializedFunction<R>` (cached JIT
function pointer).

---

### `specializeLambda`

**Before** (removed):
```cpp
template <class R, class Lambda>
auto specializeLambda(const char* funcName, Lambda& lambda)
    -> SpecializedLambda<R, explicit_arg_types_of_lambda...>;

template <class R, class Lambda>
auto specializeLambda(const char* funcName, Lambda& lambda, const Options& opts)
    -> SpecializedLambda<R, explicit_arg_types_of_lambda...>;
```

**After** (new):
```cpp
// The IRDumpingPass detects this call, resolves lambda's operator() mangled name,
// and rewrites it to specializeLambdaResolved(resolvedName, lambda [, opts]).
template <class R, class Lambda>
auto specializeLambda(Lambda& lambda)
    -> SpecializedLambda<R, explicit_arg_types_of_lambda...>;

template <class R, class Lambda>
auto specializeLambda(Lambda& lambda, const Options& opts)
    -> SpecializedLambda<R, explicit_arg_types_of_lambda...>;
```

**Semantics**: Identical to before (spec 009).  Captures are serialized as JIT constants;
explicit args define the `SpecializedLambda` parameter signature.

**Zero-arg lambda**: When the lambda has no explicit parameters (all inputs are captures),
`specializeLambda<R>(lambda)` internally calls `specializeOnlyImpl(resolvedName, Opts,
closureAddress)` — the same code path as `specializeOnly<R>(&equivalentFunc,
closureValue)`.  The returned `SpecializedLambda<R>` has `operator()()` (no arguments).

---

## Helper Free Functions

| Old | New |
|---|---|
| `specializeFunctionOrFallback(funcName, F, args...)` | `specializeFunctionOrFallback(F, args...)` |
| `specializeOrFallback(funcName, MF, obj, args...)` | `specializeOrFallback(MF, obj, args...)` |
| `assertSpecializedIsEquivalent(funcName, F, ...)` | `assertSpecializedIsEquivalent(F, ...)` |
| `compareFunctionInstructionCounts(funcName, F, ...)` | `compareFunctionInstructionCounts(F, ...)` |

All helpers now derive the function name via the `specializeOnly<R>(&F, args...)`
overload internally; the IRDumpingPass rewrites the internal `specializeOnly` call.

---

## `SpecializedLambda<R, Args...>` (unchanged)

No changes to the `SpecializedLambda` type.  All guarantees from spec 009 (move-only,
`operator bool`, destructor releases JITDylib, throws on falsy invocation) remain.

---

## Error Behavior

| Situation | Behavior |
|---|---|
| TU not compiled with plugin | `ClangRuntimeSpecializerDumpedIRError` at runtime |
| Non-constant function pointer to `specializeOnly`/`callSpecialized` | Compile-time fatal error from IRDumpingPass |
| Lambda `operator()` unresolvable (fully inlined before pass) | Compile-time fatal error from IRDumpingPass |
| Resolved name not in any registered blob | `ClangRuntimeSpecializerDumpedIRError` at runtime |
| Lambda capture not serializable | `ClangRuntimeSpecializerArgSerializationError` at runtime |

---

## Compile-Time Constraint (IMPORTANT)

The `func` argument to `callSpecialized<R>(F* func, ...)` and `specializeOnly<R>(F*
func, ...)` MUST be a compile-time-constant function pointer (a named function or a
static member function).  Passing a runtime-variable function pointer causes a
compile-time error from the IRDumpingPass.

For lambdas, the `lambda` argument to `specializeLambda<R>(lambda)` MUST be a concrete,
non-generic lambda visible in the same TU compiled with the IR-dumping plugin.  The
lambda's `operator()` must not be fully inlined by the compiler before the plugin pass
runs (guaranteed by the `[[gnu::noinline]]` annotation on the public user-facing
`specializeLambda` overload).

---

## Usage Examples

```cpp
// Named function — no funcName string needed
int result = callSpecialized<int>(&myKernel, 42, true);
auto spec = specializeOnly<int>(&myKernel, 42, true);

// Lambda — complex body allowed; no funcName, no separate kernel TU required
auto lambda = [&obj](int x, bool y) {
  for (int i = 0; i < x; i++) {
    if (y) obj.foo(i, x);
    else   obj.bar(i, x);
  }
  return x;
};
auto specL = specializeLambda<int>(lambda);
specL(5, true);   // calls JIT-specialized version

// Zero-arg lambda — same code path as specializeOnly with all args baked
auto specZ = specializeLambda<double>([&params]() {
  return doComputation(params);
});
specZ();  // takes no arguments; all constants baked in
```
