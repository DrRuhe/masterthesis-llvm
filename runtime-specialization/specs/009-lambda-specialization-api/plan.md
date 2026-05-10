# Implementation Plan: Lambda Specialization API

**Branch**: `WIP-specialization/009-lambda-specialization-api` | **Date**: 2026-05-10 | **Spec**: [spec.md](spec.md)

## Summary

Add `specializeLambda<R>(funcName, lambda, [opts])` — a free function that JIT-specializes
a named kernel function using the lambda's closure struct as a constant, while keeping the
lambda's explicit argument types variable.  The result is `SpecializedLambda<R, Args...>`:
a RAII, move-only callable whose `operator()(Args...)` invokes the JIT-compiled
machine code.  Three smoke tests cover basic correctness, void return, and RAII lifetime.

## Technical Context

**Language/Version**: C++17, LLVM/Clang (project toolchain per flake.nix)  
**Primary Dependencies**: LLVM ORC JIT (LLJIT, JITDylib, IRTransformLayer), IRDumpingPass plugin  
**Storage**: N/A (library feature)  
**Testing**: lit/FileCheck smoke tests (`ninja check-smoke-runtime-specializer`); WIP tests during development (`ninja check-wip-runtime-specializer`)  
**Target Platform**: Linux x86-64 (NixOS, per flake.nix)  
**Project Type**: C++ library — public header additions + small `.cpp` extensions  
**Performance Goals**: JIT latency comparable to `specializeOnly` for the same kernel; `SpecializedLambda::operator()` overhead indistinguishable from a plain function-pointer call  
**Constraints**: Single-threaded usage (constitution §Specialization Scope Constraint); LLVM coding standards throughout  
**Scale/Scope**: ≤ 150 lines of new template code in `ClangRuntimeSpecializer.h`; ≤ 50 lines in `.cpp`; 3–4 new smoke test files

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-checked after Phase 1 design.*

| Principle | Status | Notes |
|-----------|--------|-------|
| I. Correctness & Safety | **PASS** | `assertSpecializedLambdaIsEquivalent` required by FR-011; equivalence-checked before any benchmark use |
| II. LLVM Coding Standards | **PASS** | `SpecializedLambda` uses `std::unique_ptr`-equivalent RAII; `CRS_LOG`; `clangRuntimeSpecializer` namespace; no raw owning pointers |
| III. Test-First Validation | **PASS** | ≥ 3 smoke tests required (FR-012/013); all must pass `check-smoke-runtime-specializer` |
| IV. Performance Measurement | **PASS** | SC-002 requires instruction-count comparison; qualitative baseline sufficient (no benchmark harness needed for a pure API feature) |
| V. Minimal Public API | **PASS** | Three additions: `specializeLambda`, `SpecializedLambda`, `assertSpecializedLambdaIsEquivalent` — each directly required by a user story; no dead API |

No gate violations — proceeding to Phase 0.

## Project Structure

### Documentation (this feature)

```text
specs/009-lambda-specialization-api/
├── plan.md              ← this file
├── research.md          ← Phase 0 output
├── data-model.md        ← Phase 1 output
├── contracts/           ← Phase 1 output
│   └── api.md
└── tasks.md             ← Phase 2 output (/speckit-tasks — NOT created here)
```

### Source Code

```text
runtime/ClangRuntimeSpecializer/
├── ClangRuntimeSpecializer.h        ← LambdaTraits, SpecializedLambda, specializeLambda, assertSpecializedLambdaIsEquivalent
└── ClangRuntimeSpecializer.cpp      ← specializeLambdaImpl body

test/WIP/
├── specialized-lambda-basic.cpp     ← WIP during development
├── specialized-lambda-void.cpp
├── specialized-lambda-no-captures.cpp
└── specialized-lambda-raii.cpp

test/smoke/                          ← promotion target after WIP green
├── specialized-lambda-basic.cpp
├── specialized-lambda-void.cpp
├── specialized-lambda-no-captures.cpp
└── specialized-lambda-raii.cpp
```
