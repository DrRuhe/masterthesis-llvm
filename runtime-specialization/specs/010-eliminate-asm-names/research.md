# Research: Eliminate Manual ASM Names — Phase 0 Findings

**Feature**: 010-eliminate-asm-names  
**Date**: 2026-05-10

---

## Q1: How to detect specialization API call sites in LLVM IR

**Decision**: Iterate over all `CallInst` / `InvokeInst` in all functions in the module.
For each, check whether the callee is a `Function` (not indirect) and whether
`callee->getName()` contains `"specializeLambda"`, `"specializeOnly"`, or
`"callSpecialized"` (using substring match on the demangled or mangled name).

**Rationale**: The API functions are template instantiations; their mangled names are
long but contain the recognizable unmangled tokens.  Because these are defined in a
header, their IR definitions appear as `available_externally` or are inlined by the time
the pass runs, but their call sites remain as direct calls to the template instantiation.

**Complication**: If the compiler has already inlined `specializeLambda` / `specializeOnly`
into the call site by the time IRDumpingPass runs, the call may no longer be visible as a
direct `CallInst` to those functions.  Mitigation: annotate the public user-facing
overloads (the no-funcName variants) with `__attribute__((noinline))` so the compiler
preserves them as call sites through the plugin pass.  After the pass rewrites them, the
compiler's regular inliner can inline the `*Resolved` internal variant freely.

**Alternatives considered**:
- *Attribute-based marking*: Annotate every `specializeLambda` call with a Clang
  attribute (`[[clang::annotate("crs.specialize")]]`); the pass reads these.  Rejected
  because it pushes annotation burden to the user, defeating the spec goal.
- *Custom IR intrinsic*: Define a new LLVM intrinsic `llvm.crs.specialize`.  Rejected:
  requires changes to the LLVM tree, not just the plugin.

---

## Q2: How to discover the lambda's `operator()` mangled name from IR

**Decision**: For a `specializeLambda(lambda_arg)` call:

1. Get the LLVM type of `lambda_arg`.  It is either a pointer-to-struct (`ptr` in opaque
   pointer mode, typed in non-opaque mode) or a struct value.  In the current codebase
   (LLVM 18 with opaque pointers), the lambda is passed by reference as `ptr`.

2. Because opaque pointers lose the struct type at the argument level, use the following
   approach: scan all `Function` definitions in the module and look for one that:
   - Is not a declaration
   - Has a name that, when demangled via `llvm::demangle()`, contains `"operator()"` or
     `"operator ()"`
   - Is called somewhere in the same basic block or function as the `specializeLambda`
     call (to disambiguate when multiple lambdas exist in the TU)

3. Alternatively (more robust): The `specializeLambda` call receives the lambda as a
   reference.  Trace the SSA def-use chain backward from `lambda_arg` to find the
   `alloca` that holds the closure struct.  The `alloca`'s type (even with opaque
   pointers, `alloca` preserves the allocated type) gives the closure struct type.  Search
   for a `Function` whose first parameter is a pointer to that struct type and whose
   demangled name contains `"operator()"`.

**Preferred approach**: The `alloca`-tracing method (option 3 above) is robust and works
with opaque pointers.  The demangled-name filter distinguishes `operator()` from other
methods of the same closure type (e.g., constructors).

**Rationale**: Lambda closure types are unique LLVM struct types; the `operator()` is the
only method with the right combination of struct pointer + "operator()" in the name.
Using `llvm::demangle()` (available in LLVM Support) avoids hand-parsing mangling.

**Alternatives considered**:
- *Debug metadata*: Read `DISubprogram` to get the operator() name.  Rejected: debug
  info may be absent in release builds; the spec requires no debug-symbol dependency.
- *Type-based search alone* (no demangling): Scan for functions whose first parameter
  matches the closure struct pointer type.  Rejected: constructors and destructors also
  have that signature; demangling is needed to filter.

---

## Q3: How to extract the callee name from a function pointer argument

**Decision**: For `specializeOnly<R>(&myFunc, args...)` / `callSpecialized<R>(&myFunc, args...)`,
the function pointer argument in IR is:

```llvm
%fptr = bitcast ptr @_Z7myFunci to ptr   ; or directly ptr @_Z7myFunci in opaque mode
call @_ZN..specializeOnly..(..., ptr @_Z7myFunci, ...)
```

The pass:
1. Gets the first argument of the `CallInst` (index 0 after removing the old `funcName`
   param; in the new no-funcName overloads it IS index 0).
2. Strips `ConstantExpr` bitcasts via `Value::stripPointerCasts()` (or
   `ConstantExpr::stripPointerCasts()`).
3. Checks if the result is a `Function` (i.e., `dyn_cast<Function>(stripped)`).
4. If yes, reads `Function::getName()` — this is the mangled name and is exactly what
   `getTargetFunction()` expects.

**Rationale**: Function pointer to `GlobalValue` resolution is O(1) in LLVM IR and
requires no external tools.  `stripPointerCasts()` handles the common case of a bitcast
wrapper around the function global.

**Complication**: The function pointer might be a runtime value (e.g., passed in as a
parameter) rather than a compile-time constant.  In that case `dyn_cast<Function>` fails.
The pass emits a compile-time diagnostic error (`llvm::report_fatal_error` or a
`DiagnosticInfo`) explaining that only compile-time-constant function pointers are
supported.  This is consistent with the existing requirement that the target function must
be in an IR blob.

---

## Q4: How to rewrite a CallInst to insert a new string argument

**Decision**: Use the following LLVM IR Builder pattern:

```cpp
// 1. Create a global string constant for the resolved name
auto* NameStr = llvm::ConstantDataArray::getString(Ctx, ResolvedName);
auto* NameGV = new llvm::GlobalVariable(
    M, NameStr->getType(), /*isConstant=*/true,
    llvm::GlobalValue::PrivateLinkage, NameStr,
    "__crs_resolved_name_" + std::to_string(SiteIdx));
// Mark in a section that survives strip (use default .rodata)
// Cast to i8* / ptr for passing as argument
auto* NamePtr = llvm::ConstantExpr::getBitCast(
    NameGV, llvm::PointerType::getUnqual(Ctx));

// 2. Build new argument list: name ptr + original args (minus old funcName)
llvm::SmallVector<llvm::Value*> NewArgs = {NamePtr};
for (auto& Arg : CI->args()) NewArgs.push_back(Arg.get());

// 3. Replace call with new call to the *Resolved internal function
auto* NewCall = llvm::CallInst::Create(ResolvedFn, NewArgs, "", CI);
NewCall->copyMetadata(*CI);
CI->replaceAllUsesWith(NewCall);
CI->eraseFromParent();
```

The `ResolvedFn` is either found in the module (if the internal overload is already
present as a declaration) or declared on-the-fly as an `extern` function with the right
signature.

**Rationale**: This is the standard LLVM pattern for call-site instrumentation passes
(e.g., sanitizers, profiling).  `copyMetadata` preserves calling convention, debug
location, etc.

**Section for name globals**: Use default section (`.rodata` on ELF).  The global is
`PrivateLinkage` so the linker keeps it (it's referenced by the constructor function
that the pass already emits).  It will NOT be stripped by `strip --strip-unneeded` because
it is referenced from a non-debug function.

---

## Q5: How does `specializeLambdaImpl` unify with `specializeOnlyImpl` for 0-arg lambdas

**Decision**: When `ExplicitArgTypes...` is empty, `specializeLambdaImpl` forwards
directly to `specializeOnlyImpl`:

```cpp
// Inside specializeLambdaImpl (simplified):
if constexpr (sizeof...(ExplicitArgs) == 0) {
  // Treat closure address as the sole baked argument
  auto Result = specializeOnlyImpl(ResolvedName, Opts, &lambda);
  return makeSpecLambda<R>(Result);  // wraps into SpecializedLambda<R>
}
```

For non-zero explicit args, the existing `specializeLambdaImpl` path (build wrapper with
explicit parameters + closure constant) is used unchanged.

The `specializeOnly` case for fully-baked args also reaches `specializeOnlyImpl` with 0
variable args — the existing code already handles this (the wrapper function has 0
parameters in the JIT IR).

**Rationale**: The `if constexpr` branch at compile time costs nothing at runtime and
makes the unification explicit and verifiable via `getLastPassTrace()` in the test.

---

## Q6: What changes are needed in helper free functions

The following free functions in the header currently take `const char* funcName`:

| Old signature | New signature |
|---|---|
| `specializeFunctionOrFallback(funcName, F, args...)` | `specializeFunctionOrFallback(F, args...)` — F encodes the name |
| `specializeMethodOrFallback(funcName, MF, obj, args...)` | `specializeOrFallback(MF, obj, args...)` |
| `assertSpecializedIsEquivalent(funcName, F, ...)` | `assertSpecializedIsEquivalent(F, ...)` |
| `compareFunctionInstructionCounts(funcName, F, ...)` | `compareFunctionInstructionCounts(F, ...)` |

For these helpers, the callee name is derived by:
- Taking the address of `F` (or `MF`)
- The IR rewriting pass will also detect calls to these helper functions and inject the
  resolved name via the same mechanism as the primary API functions
- Alternatively (simpler): these helpers internally call `specializeOnly<R>(&F, args...)`
  so the name is resolved transitively

**Decision**: Migrate helpers to accept a function pointer / callable as the first
parameter; internally they call the new `specializeOnly<R>(&F, args...)` overload.  The
pass rewrites the call to the helper's internal `specializeOnly` call, not the helper
call itself.  This avoids listing every helper function in the pass's detection logic.

---

## Summary of Design Decisions

| # | Decision | Status |
|---|----------|--------|
| 1 | IR rewrite at call site (pass substitution) | ✅ Decided |
| 2 | Lambda op() discovery via alloca-type + demangling | ✅ Decided |
| 3 | Named function via `stripPointerCasts()` + `Function::getName()` | ✅ Decided |
| 4 | `CallInst::Create` replacement with prepended name arg | ✅ Decided |
| 5 | 0-arg lambda → `specializeOnlyImpl` via `if constexpr` | ✅ Decided |
| 6 | Helper free functions delegate to `specializeOnly<R>(&F, ...)` | ✅ Decided |
