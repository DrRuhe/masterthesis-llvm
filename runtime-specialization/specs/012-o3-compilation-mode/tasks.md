# Tasks: O3 Compilation Mode for Tests and Benchmarks

**Input**: Design documents from `specs/012-o3-compilation-mode/`
**Prerequisites**: plan.md ✅ spec.md ✅ research.md ✅ data-model.md ✅

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (independent files, no in-phase dependencies)
- **[Story]**: Which user story this task belongs to
- Tests are included where the spec's acceptance criteria require verifying observable behaviour

---

## Phase 1: Setup

**Purpose**: Establish a failing canary that confirms the -O3 problem exists before implementing the fix.

- [x] T001 Create `test/WIP/o3-compilation-basic.cpp` — a minimal test that compiles with `-O3 -fpass-plugin=...`, calls `callSpecialized` or `specializeLambda`, and asserts the JIT result is correct; confirm it either passes (no problem) or fails with a meaningful error (documents the baseline)

---

## Phase 2: Foundational — IRDumpingPass Split (US3)

**Purpose**: The pass split is a hard prerequisite for US1 and US2. No benchmark or test change at -O3 can succeed until `IRRewritingPass` runs before the -O3 optimizer.

**⚠️ CRITICAL**: Phases 3 and 4 MUST NOT begin until Phase 2 is complete and `ninja check-smoke-runtime-specializer` passes at -O0.

- [x] T002 [US3] Update `comptime/RuntimeSpecializerPass.h` — add `IRRewritingPass : public PassInfoMixin<IRRewritingPass>` class declaration (mirrors the existing `IRDumpingPass` declaration)

- [x] T003 [US3] Create `comptime/IRRewritingPass.cpp` — move from `IRDumpingPass.cpp`: `SpecLambdaSite`/`SpecFuncPtrSite` structs; all `find*` helpers (`findLambdaProxyFunc`, `findLambdaArgIdx`, `findResolvedFuncInBody`, `findSpecOnlyResolvedInBody`, `findFunctionPtrArg`); `isSpecializeLambdaUserCall`/`isSpecOnlyFuncPtrUserCall` predicates; Phase 0 (detect `specializeLambda` sites), Phase 0b (detect funcptr sites), Phase N (rewrite lambda calls), Phase N2 (rewrite funcptr calls); function-name collection (FR-010 loop); write collected names to `!crs.func_names` named metadata and set `!crs.rewriting_done` flag; add `#include` for `IRDumpingPass.cpp` headers plus `llvm/IR/Metadata.h`; guard with `if (M.getNamedMetadata("crs.rewriting_done")) return PreservedAnalyses::all();`

- [x] T004 [US3] Update `comptime/IRDumpingPass.cpp` — remove Phases 0, 0b, N, N2 and the function-name collection loop; replace the `FuncNames` construction with: read `!crs.func_names` named metadata operands and push each MDString value into `FuncNames`; remove `SpecLambdaSites`, `SpecFuncPtrSites`, `LambdaTargets`, `SiteIdx`; remove all helper functions now in `IRRewritingPass.cpp`; keep the DCE-roots loop (now driven by `FuncNames` from metadata), cloning, vtable BFS, linkage fixup, GlobalDCE, serialisation, and constructor-emit sections unchanged

- [x] T005 [US3] Update `comptime/PassPlugin.cpp` — add `#include "IRRewritingPass.h"` (or declare `IRRewritingPass` via the header); register `IRRewritingPass` in `registerPipelineStartCallback` (so it runs before any -O3 optimisations); add `IRRewritingPass` to `registerPipelineParsingCallback` under name `"runtime-specialization-IR-rewriting"`; keep `IRDumpingPass` registered at `registerOptimizerLastEPCallback` unchanged

- [x] T006 [US3] Update `comptime/CMakeLists.txt` — add `IRRewritingPass.cpp` to the `add_llvm_pass_plugin(LLVMRuntimeSpecializationComptimePlugin ...)` source list alongside `IRDumpingPass.cpp` and `PassPlugin.cpp`

- [x] T007 [US3] Build (`ninja LLVMRuntimeSpecializationComptimePlugin`) and run `ninja check-smoke-runtime-specializer` — all -O0 smoke tests must pass (zero regressions); the WIP canary from T001 should also pass at -O0; fix any compilation errors or test failures before proceeding

**Checkpoint**: `ninja check-smoke-runtime-specializer` green at -O0. IRRewritingPass and IRDumpingPass cooperate correctly. Ready for Phases 3 and 4.

---

## Phase 3: User Story 1 — Benchmarks at -O3 (Priority: P1) 🎯 MVP

**Goal**: All UC kernel TUs compile at -O3. The unspecialized benchmark baseline reflects optimized static code, enabling fair comparison with JIT-specialised results.

**Independent Test**: Build `AllBenchmarks` target; run one UC benchmark binary; confirm it executes without JIT errors and the unspecialized baseline time is reported.

- [x] T008 [US1] Update `benchmarks/CMakeLists.txt` — in the `foreach(UC IN ITEMS UC1 UC2 UC7 UC8 UC12 UC14)` kernel loop, change `-O0` to `-O3` for all `KERN_OBJ` `add_custom_command` compile commands; preserve `-fpass-plugin=${PLUGIN_LIB}` and all other flags

- [x] T009 [P] [US1] Update `benchmarks/use-cases/UC1SqlPredicate/CMakeLists.txt` — change `-O0` to `-O3` in the standalone UC1 kernel compile command (the one that uses `-fpass-plugin`)

- [x] T010 [P] [US1] Update `benchmarks/use-cases/UC2Convolution/CMakeLists.txt` — change `-O0` to `-O3` in the standalone UC2 kernel compile command

- [x] T011 [P] [US1] Update `benchmarks/use-cases/UC7DfaRegex/CMakeLists.txt` — change `-O0` to `-O3` in the standalone UC7 kernel compile command

- [x] T012 [P] [US1] Update `benchmarks/use-cases/UC8IVM/CMakeLists.txt` — change `-O0` to `-O3` in the standalone UC8 kernel compile command

- [x] T013 [P] [US1] Update `benchmarks/use-cases/UC12GroupBy/CMakeLists.txt` — change `-O0` to `-O3` in the standalone UC12 kernel compile command

- [x] T014 [P] [US1] Update `benchmarks/use-cases/UC14Sort/CMakeLists.txt` — change `-O0` to `-O3` in the standalone UC14 kernel compile command

- [x] T015 [US1] Build `AllBenchmarks` (`ninja AllBenchmarks`) and execute one UC benchmark (e.g., UC1) to confirm: (a) no JIT-link crash, (b) `assertSpecializedIsEquivalent` assertions pass if present, (c) both unspecialized and specialised timings are reported; fix any -O3 compilation failures (e.g., missing `extern` on globals, ODR issues) before proceeding

**Checkpoint**: `AllBenchmarks` builds and runs at -O3 without errors. Unspecialized baseline is measurably higher-quality than the prior -O0 baseline.

---

## Phase 4: User Story 2 — Smoke Tests at -O3 (Priority: P2)

**Goal**: Every smoke test that uses `-fpass-plugin=` passes at -O3 in addition to -O0.

**Independent Test**: `ninja check-smoke-runtime-specializer` with all -O3 `RUN:` lines passes.

- [x] T016 [P] [US2] Add `-O3 RUN:` lines to the `specializeLambda`-based smoke tests — for each of the following files, add a paired `// RUN: %clangxx -g -O3 -fpass-plugin=... %s -o %t.o3.exe` line and a corresponding `// RUN: %t.o3.exe [...] | FileCheck %s --check-prefix=EXE` line (reusing the existing EXE prefix if observable output is identical at -O3): `test/smoke/specialized-lambda-basic.cpp`, `test/smoke/specialized-lambda-complex-no-funcname.cpp`, `test/smoke/specialized-lambda-no-captures.cpp`, `test/smoke/specialized-lambda-no-captures-no-funcname.cpp`, `test/smoke/specialized-lambda-void.cpp`, `test/smoke/specialized-lambda-zero-arg-no-funcname.cpp`, `test/smoke/zero-arg-lambda-equals-speconly.cpp`

- [x] T017 [P] [US2] Add `-O3 RUN:` lines to the `specializeOnly`/`callSpecialized`/function-equivalence smoke tests — same pattern as T016 for: `test/smoke/specialized-function-is-equivalent.cpp`, `test/smoke/pure-specialized-function-is-equivalent.cpp`, `test/smoke/pure-specialized-method-is-equivalent.cpp`, `test/smoke/call-specialized-instance.cpp`, `test/smoke/speconly-funcptr.cpp`

- [x] T018 [P] [US2] Add `-O3 RUN:` lines to the remaining plugin-exercising smoke tests — same pattern for: `test/smoke/instruction-count.cpp` (add `EXE-O3-10` prefix if instruction counts differ at -O3, otherwise reuse `EXE-10`), `test/smoke/read-only-inference.cpp`, `test/smoke/static-analysis-robust.cpp`, `test/smoke/funcspec-max-groups-env.cpp`, `test/smoke/virtual-methods-simple.cpp`, `test/smoke/db-operators-modifies-argument-state.cpp`

- [x] T019 [US2] Run `ninja check-smoke-runtime-specializer` — confirm 100% of both -O0 and -O3 `RUN:` lines pass; for any -O3 failure, diagnose (check `build/debug/tools/runtime-specialization/test/smoke/Output/` for generated scripts) and fix the root cause (likely a FileCheck pattern difference or a remaining -O3 IR shape issue in the pass)

**Checkpoint**: `ninja check-smoke-runtime-specializer` fully green including all new -O3 RUN lines.

---

## Phase 5: Polish & Cross-Cutting Concerns

**Purpose**: Promote the WIP canary to smoke, ensure WIP suite is also -O3 green.

- [x] T020 Promote `test/WIP/o3-compilation-basic.cpp` to `test/smoke/o3-compilation-basic.cpp` and update `CMakeLists.txt` add_lit_testsuite dependencies if needed; confirm it appears in the smoke suite output

- [x] T021 Run `ninja check-smoke-runtime-specializer` one final time on the complete suite (all -O0 and -O3 lines) to confirm SC-001 and SC-002 are both met; record pass count in a comment at the top of this tasks.md

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: No dependencies — start immediately
- **Phase 2 (Foundational/US3)**: Depends on Phase 1 completion; BLOCKS Phases 3 and 4
- **Phase 3 (US1)**: Depends on Phase 2; T009–T014 are parallelizable once T008 is done; T015 depends on T008–T014
- **Phase 4 (US2)**: Depends on Phase 2; T016–T018 are parallelizable; T019 depends on T016–T018
- **Phase 5 (Polish)**: Depends on Phases 3 and 4

### User Story Dependencies

- **US3 (P3 in spec, Foundational in implementation)**: No story dependencies — implement first despite being spec-P3
- **US1 (P1)**: Depends on US3 (IRRewritingPass must be in place before kernel TUs compile at -O3)
- **US2 (P2)**: Depends on US3 (IRRewritingPass must be in place before -O3 tests can pass)
- US1 and US2 are independent of each other and can proceed in parallel after US3

### Within Phase 3

- T008 must complete before T009–T014 (parent CMakeLists changes the foreach loop, per-UC changes are separate targets)
- T009–T014 are fully parallel (different files)
- T015 depends on T008–T014

### Within Phase 4

- T016, T017, T018 are fully parallel (different test files)
- T019 depends on T016–T018

---

## Parallel Execution Examples

### Phase 3 (after T008 completes)

```text
Parallel:
  T009: benchmarks/use-cases/UC1SqlPredicate/CMakeLists.txt
  T010: benchmarks/use-cases/UC2Convolution/CMakeLists.txt
  T011: benchmarks/use-cases/UC7DfaRegex/CMakeLists.txt
  T012: benchmarks/use-cases/UC8IVM/CMakeLists.txt
  T013: benchmarks/use-cases/UC12GroupBy/CMakeLists.txt
  T014: benchmarks/use-cases/UC14Sort/CMakeLists.txt
Then: T015 (build + verify AllBenchmarks)
```

### Phase 4

```text
Parallel:
  T016: specializeLambda smoke tests (7 files)
  T017: specializeOnly/callSpecialized smoke tests (5 files)
  T018: remaining plugin smoke tests (6 files)
Then: T019 (run ninja check-smoke-runtime-specializer)
```

---

## Implementation Strategy

### MVP (US3 + US1 only)

1. Phase 1: T001
2. Phase 2: T002–T007 (pass split + -O0 verification)
3. Phase 3: T008–T015 (benchmark CMake + verification)
4. **STOP and validate**: AllBenchmarks runs at -O3 with fair baselines

### Full Delivery

1. Complete MVP above
2. Phase 4: T016–T019 (all smoke tests at -O3)
3. Phase 5: T020–T021 (polish + final verification)

---

## Notes

- `[P]` tasks within a phase touch independent files and can be done in parallel
- Phase 2 is the highest-risk phase; budget extra time for debugging the pass split
- `instruction-count.cpp` is the most likely smoke test to need a separate `-O3` FileCheck prefix, since -O3 host compilation changes how much code survives to the JIT input
- Do NOT change `-O0` → `-O3` in any CMakeLists until T007 passes; premature changes will cause blob-generation failures
- Smoke tests without `-fpass-plugin=` (`missing-plugin-funcptr-error.cpp`, `missing-plugin-lambda-error.cpp`) need no -O3 RUN lines
