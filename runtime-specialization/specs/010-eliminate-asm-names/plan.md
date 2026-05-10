# Implementation Plan: Eliminate Manual ASM Names from Specialization API

**Branch**: `010-eliminate-asm-names` | **Date**: 2026-05-10 | **Spec**: [spec.md](spec.md)  
**Input**: Feature specification from `specs/010-eliminate-asm-names/spec.md`

## Summary

Remove the `funcName` string parameter from `specializeLambda`, `specializeOnly`, and
`callSpecialized`.  The IRDumpingPass is extended to scan each TU for call sites of these
three APIs, resolve the target function name from the IR (lambda `operator()` mangled name
or named function pointer target), and rewrite those call sites to call internal
`*Resolved` variants that receive the embedded string as a compile-time constant.  A
zero-argument lambda and a fully-baked `specializeOnly` call are unified: both reach
`specializeOnlyImpl` with an empty variable-arg list and the resolved name.

---

## Technical Context

**Language/Version**: C++17, LLVM 18  
**Primary Dependencies**: LLVM/Clang plugin (`LLVMRuntimeSpecializationComptimePlugin`), LLJIT, `llvm::PassBuilder`  
**Storage**: N/A  
**Testing**: `lit` / `FileCheck` via `ninja check-wip-runtime-specializer` and `ninja check-smoke-runtime-specializer`  
**Target Platform**: Linux x86-64 (NixOS, Nix flake at `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/flake.nix`)  
**Project Type**: C++ library + LLVM compiler plugin  
**Performance Goals**: No regression in JIT compilation time; name resolution at the call site is free (compile-time rewriting, not runtime lookup)  
**Constraints**: Embedded function names must survive strip; no debug-symbol dependency  
**Scale/Scope**: 3 API functions changed; 1 pass extended; existing smoke tests updated; 4 new smoke tests added

---

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Principle | Status | Notes |
|-----------|--------|-------|
| I. Correctness & Safety | ✅ PASS | `assertSpecializedIsEquivalent` tests preserved; LLVM `Expected<T>` error model unchanged |
| II. LLVM Coding Standards | ✅ PASS | IR rewriting uses standard LLVM IR Builder APIs; mangled-name extraction via `llvm::Function::getName()`; no raw owning pointers introduced |
| III. Test-First Validation | ✅ PASS | WIP tests written before implementation; promoted to smoke on completion |
| IV. Performance Measurement | ✅ PASS | Name resolution is compile-time rewriting — zero runtime cost; no benchmark regression expected |
| V. Minimal Public API | ✅ PASS | `funcName` overloads removed (not retained for compatibility); no new public types added; dead API deleted outright |
| Backwards Compatibility | ✅ PASS | Old `funcName` overloads deleted; all callers updated in the same change; no shim |
| Specialization Scope Constraint | ✅ PASS | Single-threaded execution model unchanged |

No violations. Complexity Tracking table omitted.

---

## Project Structure

### Documentation (this feature)

```text
specs/010-eliminate-asm-names/
├── plan.md              ← this file
├── research.md          ← Phase 0 output
├── data-model.md        ← Phase 1 output
├── contracts/
│   └── api.md           ← Phase 1 output
└── tasks.md             ← Phase 2 output (/speckit.tasks)
```

### Source Code (affected paths)

```text
runtime/ClangRuntimeSpecializer/
├── ClangRuntimeSpecializer.h      # API changes: 3 function signatures + internal Resolved variants
└── ClangRuntimeSpecializer.cpp    # getTargetFunction() unchanged; specializeOnlyImpl() unified path

comptime/
└── IRDumpingPass.cpp              # Extended: call-site scan + IR rewrite

test/WIP/                          # New tests developed here first
├── specialized-lambda-no-funcname.cpp
├── specialize-only-funcptr.cpp
├── zero-arg-lambda-equals-speconly.cpp
└── missing-plugin-error.cpp

test/smoke/                        # Existing lambda tests promoted here; new tests promoted on completion
├── specialized-lambda-basic.cpp   # promote from WIP
├── specialized-lambda-void.cpp    # promote from WIP
└── specialized-lambda-equivalence.cpp  # promote from WIP

benchmarks/
└── (all existing callers of specializeLambda/specializeOnly/callSpecialized updated)
```

---

## Phase 0: Research

See [research.md](research.md) for full findings.

### Key Decisions

**1. IR rewriting strategy (call-site substitution)**

Decision: The IRDumpingPass rewrites each `specializeLambda(lambda)` /
`specializeOnly(&f, args...)` / `callSpecialized(&f, args...)` call in the IR to an
internal `*Resolved` variant that takes the resolved name as a `const char*` first
argument.

Rationale: Compile-time substitution requires no runtime address-to-name table, no
dependency on debug symbols, and survives strip/LTO.  The runtime API stays simple
(`specializeOnlyImpl` unchanged); only the calling convention at call sites changes, and
that change is invisible to the user.

Alternatives rejected:
- *Runtime address table* (address → name lookup): requires a global table, adds runtime
  cost at init(), and requires the function pointer to be a non-inline symbol at every
  call site.
- *Template-based name embedding*: would require `__builtin_FUNCTION()`-style compiler
  intrinsic that returns the name at compile time; not available for arbitrary callees
  in standard C++17 without compiler magic.

**2. Lambda `operator()` discovery in IR**

Decision: For a `specializeLambda(lambda_arg)` call, the pass extracts the IR type of
`lambda_arg` (a pointer/reference to the closure struct `%class.anon.N`), then searches
the module for the unique non-declaration function whose first parameter type is a pointer
to that closure struct — that function IS the lambda's `operator()`.  Its name is used as
the resolved name.

Rationale: Lambda closure types in LLVM IR are unique structs per lambda; the
`operator()` is the only callable member taking a pointer to the closure struct as its
first arg.  This is reliable without debug info.

**3. Named function pointer resolution**

Decision: For `specializeOnly<R>(&myFunc, args...)` / `callSpecialized<R>(&myFunc, args...)`,
the first argument in IR is a `ConstantExpr` bitcast of the global function value.  The
pass strips the bitcast and reads `GlobalValue::getName()` directly.

Rationale: Function pointer to name mapping via `GlobalValue::getName()` is O(1), always
present (functions are globals in LLVM IR), and requires no additional infrastructure.

**4. Unified zero-argument lambda path**

Decision: `specializeLambdaImpl` with an empty `ExplicitArgTypes` pack is eliminated as a
separate branch.  Instead, `specializeLambda(lambda)` with a 0-arg lambda calls
`specializeOnlyImpl(resolvedName, Opts, closure_address)` — exactly the same call as
`specializeOnly<R>(&equivalentFunc, closure_address)`.  The `SpecializedLambda<R>` wrapper
is constructed from the resulting function pointer.

Rationale: The spec explicitly requires the two code paths to be identical (FR-005,
SC-005).  The unification is mechanically simple: the closure address is the only baked
argument, and there are no explicit (variable) args.

**5. Internal `*Resolved` function naming**

Decision: Internal variants are named `__crs_specializeLambdaResolved`,
`__crs_specializeOnlyResolved`, `__crs_callSpecializedResolved` (or equivalent private
member overloads that accept `const char*` as their first argument, distinct from the
user-visible overloads via an `internal_tag` struct).

Rationale: Using a private tag type (`struct ResolvedTag{};`) avoids name collisions with
user-defined functions and cannot be called accidentally from user code.

---

## Phase 1: Design & Contracts

See [data-model.md](data-model.md) for the IR annotation structure and API change matrix.
See [contracts/api.md](contracts/api.md) for public API contracts.

### IR Rewriting Mechanism (Summary)

```
Before (user writes):
  specializeLambda<int>(lambda)

After IRDumpingPass rewrite:
  __crs_specializeLambdaResolved<int>("_ZZmainENK3$_0clEib", lambda)
  ──────────────────────────────────────┬────────────────────────────
                                         └─ injected compile-time constant
```

The pass locates each `CallInst` whose callee function name contains
`"specializeLambda"`, `"specializeOnly"`, or `"callSpecialized"`, resolves the target
name, creates a global `i8` array string constant (the name), and replaces the call with a
new `CallInst` to the `*Resolved` internal variant with the string pointer prepended.

### Zero-Arg Lambda / specializeOnly Unification

```
specializeLambda<R>([&ctx](){...})         specializeOnly<R>(&equivalentFunc, ctx)
          │                                          │
          └─────────── both reach ──────────────────┘
                  specializeOnlyImpl("resolvedName", Opts, ctx_addr)
                              │
                    wrapper IR: R wrapper() { return target(ctx_addr_const); }
```

The `SpecializedLambda<R>` is constructed from the result of `specializeOnlyImpl` when
`ExplicitArgTypes` is empty; `SpecializedFunction<R>` is returned for `specializeOnly`.
Both wrap the same JIT-compiled function pointer.

### Migration of Existing Call Sites

Every existing caller of the old `funcName`-string overloads is updated in the same
commit:
- Benchmark files under `benchmarks/`: `callSpecialized<R>("name", args...)` →
  `callSpecialized<R>(&name, args...)`
- Existing WIP tests: updated to remove funcName string; re-verified under
  `ninja check-wip-runtime-specializer`
- `assertSpecializedIsEquivalent`, `specializeFunctionOrFallback`, `specializeOrFallback`:
  updated to use function pointer as first arg instead of `const char*`

### Agent Context

Active feature plan updated in `CLAUDE.md` to point to this plan file.
