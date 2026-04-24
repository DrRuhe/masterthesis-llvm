# Implementation Plan: JIT Pipeline Infrastructure

**Branch**: `002-jit-pipeline-infrastructure` | **Date**: 2026-04-24 | **Spec**: [spec.md](spec.md)  
**Input**: Feature specification from `specs/002-jit-pipeline-infrastructure/spec.md`

## Summary

Extract pipelines 0 and 1 from the monolithic `ClangRuntimeSpecializer.cpp` transform lambda into dedicated source files behind a `JITPipeline.h` dispatch interface; standardize three env var names to follow `CRS_DEFAULT_<FIELDNAME_UPPERCASE>` convention and add `CRS_DEFAULT_FUNC_SPEC_MAX_GROUPS`; add WARNING + clamp for invalid pipeline/option values; replace the hardcoded search space in `optimize_benchmarks.py` with a JSON descriptor loaded via `--search-space PATH` (falling back to a built-in default covering all 6 env-var-exposed options), writing the descriptor verbatim to `optimization_sessions.search_space_json` on study start.

## Technical Context

**Language/Version**: C++17 (JIT library), Python 3.x (optimizer script)  
**Primary Dependencies**: LLVM/ORC JIT, DuckDB, Optuna  
**Storage**: DuckDB `benchmarks.duckdb` — `optim_trial_params.params_json` and `optimization_sessions.search_space_json` already present; no schema change needed  
**Testing**: lit/FileCheck for C++ (`ninja check-wip-runtime-specializer`); manual subprocess smoke test for Python  
**Target Platform**: Linux x86-64  
**Project Type**: JIT compiler library + Python optimization tooling  
**Performance Goals**: No regression in JIT compilation time or specialized execution time — the pipeline split is pure refactoring  
**Constraints**: LLVM coding standards (no raw exceptions → `llvm::Error`; no RTTI; `CRS_LOG` macro); `Options` public C++ API (fluent builder) unchanged; existing DuckDB data survives (schema already correct); env var rename is acceptable per "no legacy support" policy but `optimize_benchmarks.py` must be updated atomically  
**Scale/Scope**: 2 pipelines today; 6 env-var-exposed options; 1 JSON descriptor format with 7 parameter types

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Principle | Status | Notes |
|-----------|--------|-------|
| I. Correctness & Safety | PASS | Pipeline split is pure refactoring — no IR transform logic changes. Both pipelines already covered by smoke tests. `assertSpecializedIsEquivalent` must be included in new WIP pipeline-dispatch tests. |
| II. LLVM Coding Standards | PASS (conditional) | `JITPipeline.h` interface must use `llvm::Error`, `llvm::StringRef`, `llvm::ArrayRef`. No `std::function` (heap allocates). No virtual dispatch. PIC and helper lambdas remain stack-local per transform call. Re-check after Phase 1 design. |
| III. Test-First Validation | **REQUIRES ACTION** | WIP tests needed before implementation: (a) pipeline dispatch via `CRS_DEFAULT_PIPELINE` env var, (b) WARNING + clamp on invalid pipeline index, (c) renamed env var reading (`CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS` etc.), (d) `FuncSpecMaxGroups` via `CRS_DEFAULT_FUNC_SPEC_MAX_GROUPS`. |
| IV. Performance Measurement | PASS | Existing polybench / TPC-H benchmark baselines remain valid after refactoring. No perf-sensitive logic changes. |
| V. Minimal Public API | PASS | `Options` struct interface (C++ builder methods) unchanged. Env var names are I/O, not public C++ API surface. `JITPipeline.h` is internal (not included in public header). |
| Backwards Compatibility — DB | PASS | `params_json` + `search_space_json` columns already exist. Rows written before this feature have NULL in `search_space_json` (acceptable per spec assumption). |
| Backwards Compatibility — Env Vars | ACCEPTABLE BREAK | `CRS_DEFAULT_FIXPOINT`, `CRS_DEFAULT_UNROLL`, `CRS_DEFAULT_LARGE_MOD` renamed. No external users; `optimize_benchmarks.py` updated in the same change. |

**Post-design re-check**: Confirm after Phase 1 that (a) `JITPipeline.h` exposes no LLVM types in the public `.h` path, (b) no new public C++ API symbols are added.

## Project Structure

### Documentation (this feature)

```text
specs/002-jit-pipeline-infrastructure/
├── plan.md                           # This file
├── research.md                       # Phase 0 output
├── data-model.md                     # Phase 1 output
├── quickstart.md                     # How to add a new pipeline
├── contracts/
│   └── search-space-descriptor.md   # Authoritative descriptor JSON contract
└── tasks.md                          # Phase 2 output (/speckit-tasks — NOT created here)
```

### Source Code Changes

```text
runtime/ClangRuntimeSpecializer/
├── ClangRuntimeSpecializer.h         existing — rename 3 env vars in Options::Default(); add CRS_DEFAULT_FUNC_SPEC_MAX_GROUPS
├── ClangRuntimeSpecializer.cpp       existing — strip pipeline bodies; add dispatch switch + WARNING/clamp
├── JITPipeline.h                     NEW — PipelineRunArgs struct, PipelineEntry, getRegisteredPipelines()
├── JITPipelineInlining.cpp           NEW — pipeline 0 body, extracted from transform lambda
├── JITPipelineFuncSpec.cpp           NEW — pipeline 1 body, extracted from transform lambda
├── JITPipelineRegistry.cpp           NEW — getRegisteredPipelines() static array (add new pipeline here only)
├── DevirtualizeConstantVtableCalls.* unchanged
├── StaticMutabilityAnalysis.*        unchanged
├── InvariantLoadToConstant.*         unchanged
└── ConstantArgFunctionSpecializationPass.* unchanged

benchmarks/
├── optimize_benchmarks.py            modified — add --search-space, descriptor-driven objective + env mapping; write search_space_json on study start
└── (built-in default descriptor is a Python dict literal inline — no separate file)

test/WIP/
├── pipeline-dispatch.cpp             NEW — smoke test: CRS_DEFAULT_PIPELINE=0/1 dispatch
├── pipeline-invalid-index.cpp        NEW — WARNING + clamp on CRS_DEFAULT_PIPELINE=99
└── options-env-vars.cpp              NEW — renamed env var reading (CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS etc.)
```

**CMakeLists.txt** (`runtime/ClangRuntimeSpecializer/CMakeLists.txt`): add `JITPipelineInlining.cpp`, `JITPipelineFuncSpec.cpp`, `JITPipelineRegistry.cpp` to source list.

## Complexity Tracking

> No constitution violations.
