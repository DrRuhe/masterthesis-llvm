# Tasks: IR Dump Preprocessing

**Input**: Design documents from `specs/004-ir-dump-preprocessing/`  
**Prerequisites**: plan.md ✅, spec.md ✅, research.md ✅, data-model.md ✅

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (no dependency on incomplete tasks in same phase)
- **[Story]**: Maps to user story from spec.md (US1, US2, US3)

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Add the required include for `GlobalDCEPass`. No new files or directories needed.

- [X] T001 Add `#include "llvm/Transforms/IPO/GlobalDCE.h"` to `comptime/IRDumpingPass.cpp` (after existing includes)

**Checkpoint**: File compiles cleanly.

---

## Phase 2: Foundational (Preprocessing Implementation)

**Purpose**: Insert the full compile-time preprocessing block into `IRDumpingPass::run()`. All user stories depend on this being complete before tests can pass.

**⚠️ CRITICAL**: No user story work can begin until this phase is complete. Tasks T002–T009 are all in `comptime/IRDumpingPass.cpp` and must be applied in order.

- [X] T002 In `comptime/IRDumpingPass.cpp`, change the `FuncNames` collection loop (currently lines 67–71) to: (a) only collect functions that are non-declaration, non-`__clangRS`, AND do NOT have `InternalLinkage` or `PrivateLinkage`; (b) simultaneously populate a `SmallPtrSet<Function*, 64> DCERoots` with the same functions. These are the specialization targets and GlobalDCE roots.

- [X] T003 In `comptime/IRDumpingPass.cpp`, remove the `getOrCreateIRDumpGlobals(M)` call from its current position (line 73, before serialization) — it will be re-inserted in T009. This prevents `PtrGV`/`LenGV` from being pruned by GlobalDCE in T007.

- [X] T004 In `comptime/IRDumpingPass.cpp`, insert a vtable BFS block immediately after the `DCERoots` collection: walk all constant globals with initializers recursively (via `SmallVector`/`SmallPtrSet` worklist), collect every `Function*` encountered, and set each such non-declaration function to `GlobalValue::WeakODRLinkage`. This makes vtable method implementations GlobalDCE roots so `DevirtualizeConstantVtableCallsPass` can find them at JIT time.

- [X] T005 In `comptime/IRDumpingPass.cpp`, insert a globals linkage loop after T004: for every non-declaration, non-internal, non-private `GlobalVariable` in the module: if it is constant with an initializer → set `WeakODRLinkage`; otherwise (Comdat cleared first) → set `AvailableExternallyLinkage`. Note: clearing Comdat before AvailableExternally prevents the verifier error "Declaration may not be in a Comdat" (see Decision 3b in research.md).

- [X] T006 In `comptime/IRDumpingPass.cpp`, insert erasure of `llvm.global_ctors` and `llvm.global_dtors` after T005:
  ```cpp
  if (auto *GCtors = M.getGlobalVariable("llvm.global_ctors"))
    GCtors->eraseFromParent();
  if (auto *GDtors = M.getGlobalVariable("llvm.global_dtors"))
    GDtors->eraseFromParent();
  ```

- [X] T007 In `comptime/IRDumpingPass.cpp`, insert zero-sized global removal after T006: iterate all non-declaration globals, collect those where `DL.getTypeAllocSize(G.getValueType()) == 0` into a `SmallVector`, then for each: call `replaceAllUsesWith(PoisonValue::get(G->getType()))` and `eraseFromParent()`.

- [X] T008 In `comptime/IRDumpingPass.cpp`, insert GlobalDCE execution after T007. Implementation uses a fresh PassBuilder + all four analysis managers (LAM, FAM, CGAM, MAM) with crossRegisterProxies — required for GlobalDCEPass which needs full analysis infrastructure.

- [X] T009 In `comptime/IRDumpingPass.cpp`, insert two operations after T008: (a) re-insert the `getOrCreateIRDumpGlobals(M)` call (removed in T003) here, before `WriteBitcodeToFile`; (b) iterate `DCERoots` and for each non-declaration function that does NOT have WeakODRLinkage (vtable functions), set `InternalLinkage`. Note: skip WeakODR functions — vtable BFS already assigned the correct linkage for DevirtualizeConstantVtableCallsPass.

**Checkpoint**: `ninja ClangRuntimeSpecializer` and `ninja LLVMRuntimeSpecializationComptimePlugin` build cleanly.

---

## Phase 3: User Story 1 — Compile with Plugin, Get Preprocessed Blob (Priority: P1) 🎯 MVP

**Goal**: A blob produced by the updated plugin has all preprocessing invariants: no `llvm.global_ctors`, target functions `InternalLinkage`, vtable functions `WeakODRLinkage`, no `__clangRS` definitions, zero-sized globals absent.

**Independent Test**: Compile a TU with the plugin; call `init()` and `getModuleStats()`; verify function count is significantly lower than for a raw un-preprocessed blob of the same TU.

- [X] T010 [US1] Write `test/WIP/ir-dump-preprocessing-blob-stats.cpp`: compile with the IR-dumping plugin, call `ClangRuntimeSpecializer::init()`, print `getModuleStats().FunctionCount` to stdout, and `FileCheck` that the value is below a reasonable threshold (e.g., `<= 5` for a minimal TU). This verifies that compile-time GlobalDCE pruned the blob. Actual count for minimal mypow TU: 2 (mypow + main). Pattern: `{{[1-9]}}` (single digit, ≤ 9).

- [X] T011 [US1] Run `ninja check-wip-runtime-specializer` and confirm T010 passes. If it fails, debug by printing `getModuleStats()` fields and adjusting the FileCheck threshold to the actual pruned count.

- [X] T012 [US1] Promote `test/WIP/ir-dump-preprocessing-blob-stats.cpp` to `test/smoke/ir-dump-preprocessing-blob-stats.cpp` once T011 passes.

**Checkpoint**: `ninja check-smoke-runtime-specializer` includes the new test and it passes.

---

## Phase 4: User Story 2 — JIT Specialization Uses Preprocessed Blob (Priority: P1)

**Goal**: All specialization paths (callSpecialized, specializeOnly, virtual methods) produce correct results when the blob was preprocessed at compile time. The vtable WeakODR invariant set by the IR-dumping pass (not by `prepareModuleForJIT`) is sufficient for `DevirtualizeConstantVtableCallsPass`.

**Independent Test**: All 18 existing smoke tests pass without modification.

- [X] T013 [US2] Run `ninja check-smoke-runtime-specializer` and confirm all 18 tests pass. Pay special attention to: `virtual-methods.cpp` (vtable devirtualization — relies on vtable functions having `WeakODRLinkage`, now set at compile time), `pure-specialized-function-is-equivalent.cpp` (correctness gate), `instruction-count.cpp` (pruned blob may change instruction counts; adjust expected counts if needed).

- [X] T014 [US2] If `instruction-count.cpp` fails because the blob is now smaller (fewer functions loaded into the JIT module), update the expected instruction count bounds in `test/smoke/instruction-count.cpp` to match the new post-preprocessing values. (N/A — instruction-count.cpp passed without changes.)

**Checkpoint**: All 18 smoke tests green. `ninja check-smoke-runtime-specializer` reports 0 failures.

---

## Phase 5: User Story 3 — Plugin Runs at the Right Moment (Priority: P2)

**Goal**: Verify the pass fires at `OptimizerLastEP`, after all host optimizations but before machine code. This is already structurally satisfied by `PassPlugin.cpp:39–42`; the task is to confirm it with a test.

**Independent Test**: The pass is deterministic: compiling the same TU twice produces byte-identical blobs.

- [X] T015 [US3] Write `test/WIP/ir-dump-deterministic.cpp`: compile the same TU twice (two separate `%clangxx` RUN lines to the same source, different output binaries), run both and compare outputs via `diff`. Prints FunctionCount, InstructionCount, BitcodeSizeBytes; diff ensures all three match.

- [X] T016 [US3] Run `ninja check-wip-runtime-specializer` to confirm T015 passes.

- [X] T017 [US3] Promote `test/WIP/ir-dump-deterministic.cpp` to `test/smoke/ir-dump-deterministic.cpp` once T016 passes.

**Checkpoint**: Determinism test green in smoke suite.

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Update stale comments, apply the spec 003 simplification unlocked by this feature, and run the final smoke gate.

- [X] T018 In `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.cpp`, update the stale comment at the line reading `// Safe: prepareModuleForJIT already gave the wrapper ExternalLinkage and vtable` to say that vtable WeakODR is now set by `IRDumpingPass` at compile time (spec 004), not by `prepareModuleForJIT`.

- [X] T019 In `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.cpp`, replace the entire body of `prepareModuleForJIT` (lines ~695–816, ~120 lines) with the simplified version that only sets the wrapper to `ExternalLinkage` (spec 003 plan, Phase 1). Remove: `Optimize`/`force-no-optimize` detection, `VTableFunctions` BFS worklist, function linkage loop (except wrapper), globals linkage loop, `llvm.global_ctors`/`dtors` erasure, zero-sized global removal.

- [X] T020 Run `ninja check-smoke-runtime-specializer`. All tests must be green. This is the final gate confirming spec 004 and spec 003 land correctly together. ✅ 26/26 passing (18 original + 2 new blob-stats/determinism smoke tests + 4 WIP + 2 promoted).

- [X] T021 [P] Run `ninja check-all-runtime-specializer` (or at minimum the benchmark suite with `--benchmark_filter=".*"`) to confirm no benchmark-level "Symbols not found" or JIT crash regressions. Fixed two issues found during TPCH benchmark: (a) vtable BFS restricted to `_ZTV*`/`_ZTI*`/`_ZTS*` C++ globals only (was incorrectly marking C dispatch table functions WeakODR); (b) `InvariantLoadToConstantPass` crashes on unmapped FinalAddr for large TUs — fixed by adding `mincore`-based safety guard + using `getSExtValue()` for signed offsets.

**Checkpoint**: All smoke tests green; spec 004 and spec 003 simplification landed together.

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: No dependencies — start immediately
- **Phase 2 (Foundational)**: Depends on Phase 1 — BLOCKS all user story phases; T002–T009 are strictly sequential (same file, order matters)
- **Phase 3 (US1)**: Depends on Phase 2 completion
- **Phase 4 (US2)**: Depends on Phase 2 completion — can start in parallel with Phase 3 once Phase 2 is done
- **Phase 5 (US3)**: Depends on Phase 2 completion — can run in parallel with Phases 3 and 4
- **Phase 6 (Polish)**: Depends on Phases 3, 4, 5 all green

### User Story Dependencies

- **US1 (P1)**: Needs Phase 2 complete; independent of US2/US3
- **US2 (P1)**: Needs Phase 2 complete; independent of US1/US3 (uses existing smoke tests)
- **US3 (P2)**: Needs Phase 2 complete; independent of US1/US2

### Within Phase 2

T002 → T003 → T004 → T005 → T006 → T007 → T008 → T009 (strictly sequential — same file, shared state: DCERoots set, module M)

### Parallel Opportunities

- Phases 3, 4, 5 can all start simultaneously once Phase 2 completes
- T021 (benchmark run) is independent of T018/T019 documentation changes

---

## Parallel Example: After Phase 2 Completes

```
Parallel track A (US1): T010 → T011 → T012
Parallel track B (US2): T013 → T014
Parallel track C (US3): T015 → T016 → T017
```

All tracks converge at Phase 6: T018 → T019 → T020 → T021.

---

## Implementation Strategy

### MVP (Phase 2 + Phase 4 only)

1. Complete Phase 1 (T001) + Phase 2 (T002–T009) — the preprocessing code
2. Complete Phase 4 (T013–T014) — smoke gate proving correctness
3. **STOP and VALIDATE**: if all 18 smoke tests pass, the core feature is done
4. Proceed to Phase 6 T018–T019 to apply the spec 003 simplification

### Full Delivery

1. Phase 1 → Phase 2 → Phases 3/4/5 in parallel → Phase 6
2. Each phase delivers an independently verifiable increment

---

## Notes

- T002–T009 are all in `comptime/IRDumpingPass.cpp`; apply in order, they share `DCERoots` state
- T019 (prepareModuleForJIT simplification) MUST land after T013 confirms smoke suite is green — do not apply T019 before confirming correctness of T013
- T014 (instruction-count update) is conditional: only needed if `instruction-count.cpp` breaks
- No new CMakeLists changes required; `GlobalDCEPass` is already linked via the LLVM libraries
