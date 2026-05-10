# Data Model: Eliminate Manual ASM Names

**Feature**: 010-eliminate-asm-names  
**Date**: 2026-05-10

---

## IR Annotation Structure

The IRDumpingPass emits one name-resolution global per specialization call site in the TU.

### Per-call-site global (new)

```llvm
; One entry per specializeLambda / specializeOnly / callSpecialized call site
@__crs_resolved_name_0 = private constant [N x i8] c"_ZZmainENK3$_0clEib\00", section ".rodata"
@__crs_resolved_name_1 = private constant [M x i8] c"_Z7myFunci\00",           section ".rodata"
```

**Invariants**:
- `PrivateLinkage` — referenced from the same TU's constructor only; linker keeps it
- Stored in `.rodata` (ELF default for constant globals) — survives `strip --strip-debug`
- Content: null-terminated mangled function name, identical to what `FuncToBlobIdx` indexes

### Existing per-TU globals (unchanged)

```llvm
@RuntimeSpecializeableIR_data  = ... ; bitcode blob (unchanged)
@RuntimeSpecializeableIR_ptr   = ... ; pointer to blob (unchanged)
@RuntimeSpecializeableIR_len   = ... ; blob length (unchanged)
@RuntimeSpecializeableIR_funcs = ... ; function name index array (unchanged)
```

---

## API Change Matrix

| Old overload | New overload | Change |
|---|---|---|
| `callSpecialized<R>(const char* name, args...)` | `callSpecialized<R>(F* func, args...)` | `name` → typed fn ptr |
| `callSpecialized<R>(const char* name, opts, args...)` | `callSpecialized<R>(F* func, opts, args...)` | same |
| `specializeOnly<R>(const char* name, args...)` | `specializeOnly<R>(F* func, args...)` | `name` → typed fn ptr |
| `specializeOnly<R>(const char* name, opts, args...)` | `specializeOnly<R>(F* func, opts, args...)` | same |
| `specializeLambda<R>(const char* name, Lambda&)` | `specializeLambda<R>(Lambda&)` | `name` removed entirely |
| `specializeLambda<R>(const char* name, Lambda&, opts)` | `specializeLambda<R>(Lambda&, opts)` | same |

**Internal resolved variants** (not user-visible; called only from pass-rewritten IR):

```cpp
// Called by the pass-rewritten callSpecialized/specializeOnly call sites
template <class R, class F, class... Args>
R callSpecializedResolved(const char* resolvedName, F* /*unused*/, Args&&... args);

template <class R, class F, class... Args>
auto specializeOnlyResolved(const char* resolvedName, F* /*unused*/, Args&&... args)
    -> SpecializedFunction<R>;

// Called by the pass-rewritten specializeLambda call sites
template <class R, class Lambda>
auto specializeLambdaResolved(const char* resolvedName, Lambda& lambda)
    -> SpecializedLambda<R, explicit_arg_types_of_lambda...>;
```

The `F*` argument in the resolved variants is kept (passed through from the rewritten
call) but unused — the pass already extracted the name from it at compile time.

---

## Call Site Rewriting: State Transitions

```
Source C++:
  callSpecialized<int>(&myFunc, 42)

→ IR before pass (user-facing no-funcName overload):
  call @_ZN..callSpecialized..Iij..(ptr @_Z6myFunci, i32 42)

→ IRDumpingPass resolution:
  strips bitcast → Function @_Z6myFunci → getName() = "_Z6myFunci"
  emits: @__crs_resolved_name_0 = private constant [12 x i8] c"_Z6myFunci\00"

→ IR after pass rewrite:
  call @_ZN..callSpecializedResolved..Iij..(ptr @__crs_resolved_name_0, ptr @_Z6myFunci, i32 42)

→ Runtime:
  callSpecializedResolved("_Z6myFunci", /*func ptr unused*/, 42)
  → specializeOnlyImpl("_Z6myFunci", Opts, 42)  [unchanged from current]
```

---

## Zero-Arg Lambda Unification

```
specializeLambda<R>([&ctx](){...})              specializeOnly<R>(&f, ctx_val)
        │                                                 │
        │  resolved name = op()_mangled                   │  resolved name = f_mangled
        │  baked args    = {&closure}                     │  baked args    = {ctx_val}
        └──────────────── specializeOnlyImpl(name, Opts, {single_arg}) ────────────┘
                                        │
                      SpecializedFunction<R> / SpecializedLambda<R>
                              (same JIT-compiled machine code)
```

When `ExplicitArgTypes...` is empty in `specializeLambdaImpl`, the `if constexpr` branch
calls `specializeOnlyImpl(resolvedName, Opts, closureAddress)`.  The JIT wrapper function
takes 0 parameters; the closure address is the single baked constant.

---

## `FuncToBlobIdx` lookup (unchanged)

No change to the runtime lookup mechanism.  The resolved name string from the rewritten
call site is still looked up in `FuncToBlobIdx` (built at `init()` time from
`g_registered_blobs[i].FuncNames`).  The resolved name is the mangled IR function name,
which is identical to the key already stored in `FuncToBlobIdx`.

---

## Error Conditions

| Condition | Error |
|---|---|
| TU compiled without plugin (no annotation global) | `ClangRuntimeSpecializerDumpedIRError` at runtime when name is not found in `FuncToBlobIdx` — same as today |
| Function pointer is not a compile-time constant | Compile-time `fatal_error` from the pass: "specializaion target must be a constant function pointer" |
| Lambda `operator()` not found in module (e.g., fully inlined before pass) | Compile-time `fatal_error`: "could not resolve lambda operator() for specializeLambda call" |
| Resolved name not in any registered blob | `ClangRuntimeSpecializerDumpedIRError` — same as today |
