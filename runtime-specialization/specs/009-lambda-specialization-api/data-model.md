# Data Model: Lambda Specialization API

**Phase 1 output for plan.md**  
**Date**: 2026-05-10

---

## Key Entities

### `SpecializedLambda<R, Args...>`

New RAII callable type, analogous to the existing `SpecializedFunction<R>`.

| Field | Type | Role |
|-------|------|------|
| `FnPtr` | `R(*)(Args...)` | JIT-compiled specialized function pointer; null when falsy |
| `Dylib` | `llvm::orc::JITDylib*` | Owns the JIT-compiled machine code; freed on destruction |
| `ES` | `llvm::orc::ExecutionSession*` | Needed to call `removeJITDylib` on cleanup |

**Invariants**:
- `FnPtr != nullptr ↔ Dylib != nullptr ↔ bool(*this) == true`
- Move sets source fields to null (no double-free)
- Copy is deleted (exclusive ownership of JITDylib)

**State transitions**:
```
Default-constructed / timed-out → falsy (FnPtr=null, Dylib=null)
specializeLambdaImpl succeeds   → truthy (FnPtr≠null, Dylib≠null)
std::move(spec)                 → source becomes falsy, target is truthy
~SpecializedLambda (truthy)     → removeJITDylibNoexcept called; FnPtr=null
```

---

### `detail::LambdaTraits<MemberFnPtr>`

Compile-time type extraction from a lambda's `operator()`.

| Alias | Meaning |
|-------|---------|
| `RetType` | Return type of `operator()` |
| `ArgTypes` | `std::tuple<T1, T2, ...>` of explicit parameter types |
| `FnPtrType` | `RetType(*)(T1, T2, ...)` — matches `SpecializedLambda<RetType, T1, T2, ...>` |

Specializations provided for:
- `R (Lambda::*)(Args...) const` — const lambda (common case)
- `R (Lambda::*)(Args...)` — mutable lambda

---

### Kernel Function (convention, not a C++ type)

A named C function in a kernel TU (compiled with IRDumpingPass) with the following
parameter layout:

```
R kernel( <closure_ptr_arg>, <explicit_arg_1>, ..., <explicit_arg_N> )
```

- **Closure pointer arg**: the single serialized constant — a pointer to the lambda's
  closure struct, passed as an opaque pointer (the kernel casts it to the expected type)
- **Explicit args**: correspond 1-to-1 to the lambda's `operator()` parameters; NOT
  serialized; forwarded as live JIT wrapper parameters

This layout is a **caller convention** documented in the spec; the API does not validate it.

---

### JIT Wrapper IR (internal, per-call)

Generated inside `specializeLambdaImpl`; not a C++ type.

```llvm
; signature: R(ExplicitArgTypes...)
define R @specialized_lambda_wrapper_NNN(T1 %a0, T2 %a1, ...) {
entry:
  ; first call arg: lambda closure address baked as a pointer constant
  ; remaining call args: forwarded directly from wrapper parameters
  %r = call alwaysinline R @kernel(ptr inttoptr(i64 CLOSURE_ADDR to ptr), T1 %a0, T2 %a1, ...)
  ret R %r
}
```

The wrapper name is unique per specialization call (via `createUniqueWrapperName`).

---

## Relationships

```
specializeLambda<R>("funcName", lambda)
        │
        ├─ looks up funcName in IR blob → Kernel Function IR
        ├─ serializes &lambda as constant → Closure Pointer Arg
        ├─ deduces ExplicitArgTypes via LambdaTraits<Lambda>
        ├─ builds JIT Wrapper IR (non-zero-arg)
        ├─ runs JIT pipeline (same as specializeOnlyImpl)
        └─ returns SpecializedLambda<R, ExplicitArgTypes...>
                        │
                        └─ operator()(args...) → calls JIT-compiled wrapper → calls kernel
```
