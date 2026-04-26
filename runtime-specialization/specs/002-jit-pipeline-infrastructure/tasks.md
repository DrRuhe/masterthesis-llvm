# Tasks: JIT Pipeline Infrastructure

**Input**: Design documents from `specs/002-jit-pipeline-infrastructure/`  
**Prerequisites**: plan.md ✓, spec.md ✓, research.md ✓, data-model.md ✓, contracts/ ✓, quickstart.md ✓

**Organization**: Grouped by user story for independent delivery. US1 (C++) and US2 (Python) are fully independent and can be executed in parallel.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel with other [P]-marked tasks in the same phase
- **[Story]**: Which user story this task belongs to

---

## Phase 1: Setup

**Purpose**: Standardize env var names and prepare for pipeline interface work. These changes are self-contained and unblock all subsequent phases.

- [ ] T001 Rename `CRS_DEFAULT_FIXPOINT` → `CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS` in `Options::Default()` in `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h`
- [ ] T002 Rename `CRS_DEFAULT_UNROLL` → `CRS_DEFAULT_LOOP_UNROLL_COUNT` in `Options::Default()` in `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h`
- [ ] T003 Rename `CRS_DEFAULT_LARGE_MOD` → `CRS_DEFAULT_LARGE_MODULE_INSTR_THRESHOLD` in `Options::Default()` in `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h`
- [ ] T004 Add `static const int kFuncSpecMaxGroups = (int)_envOr("CRS_DEFAULT_FUNC_SPEC_MAX_GROUPS", 0.0); O.FuncSpecMaxGroups = kFuncSpecMaxGroups;` to `Options::Default()` in `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h`
- [ ] T005 Grep all files under `test/` and `benchmarks/` for the old env var names (`CRS_DEFAULT_FIXPOINT`, `CRS_DEFAULT_UNROLL`, `CRS_DEFAULT_LARGE_MOD`) and update any occurrences to the new names; run `ninja check-smoke-runtime-specializer` to confirm no regressions

**Checkpoint**: `Options::Default()` now reads 7 env vars with canonical names; existing smoke suite passes.

---

## Phase 2: Foundational (Blocking Prerequisite)

**Purpose**: Define the `JITPipeline.h` interface that all pipeline extraction (US3) and dispatch validation (US1) depend on.

**⚠️ CRITICAL**: US1 dispatch implementation and US3 pipeline extraction cannot begin until this phase is complete.

- [ ] T006 Create `runtime/ClangRuntimeSpecializer/JITPipeline.h` defining: `struct PipelineRunArgs { llvm::Module& Mod; const Options& Opts; llvm::PassBuilder& PB; llvm::PassInstrumentationCallbacks& PIC; std::string& CurrentGroup; int& CurrentFixpointIter; bool IsLargeModule; };`; `struct PipelineEntry { llvm::StringRef Name; llvm::Error (*Run)(PipelineRunArgs&); };`; and the declaration `llvm::ArrayRef<PipelineEntry> getRegisteredPipelines();`

**Checkpoint**: `JITPipeline.h` compiles cleanly when included; interface ready for use in dispatch and extraction tasks.

---

## Phase 3: User Story 1 — Pipeline Selection & Validation (Priority: P1) 🎯 MVP

**Goal**: A researcher can switch between pipeline 0 and pipeline 1 by setting `CRS_DEFAULT_PIPELINE`; invalid values produce a WARNING and clamp gracefully; incompatible option combinations produce informative WARNINGs.

**Independent Test**: Build any existing benchmark binary. Run with `CRS_DEFAULT_PIPELINE=0`, `CRS_DEFAULT_PIPELINE=1`, and `CRS_DEFAULT_PIPELINE=99`. Verify the first two complete without error and the third logs a WARNING and runs as pipeline 0.

### Tests for User Story 1

> **Write these FIRST; they must FAIL before implementation (constitution §III)**

- [ ] T007 [P] [US1] Write `test/WIP/pipeline-dispatch.cpp`: two test cases compile a specializable function and call it with `CRS_DEFAULT_PIPELINE=0` and `CRS_DEFAULT_PIPELINE=1`; use FileCheck to verify each run completes and produces a valid specialized result (no crash, no abort)
- [ ] T008 [P] [US1] Write `test/WIP/pipeline-invalid-index.cpp`: set `CRS_DEFAULT_PIPELINE=99` in the test env; FileCheck for `WARNING` in stderr output and verify the program exits successfully (no abort, result is correct)
- [ ] T009 [P] [US1] Write `test/WIP/options-env-vars.cpp`: set `CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS=3`, `CRS_DEFAULT_LOOP_UNROLL_COUNT=32`, `CRS_DEFAULT_LARGE_MODULE_INSTR_THRESHOLD=5000` in the test env; specialize a function and FileCheck that at most 3 fixpoint iterations run (via pass trace output or log)
- [ ] T010 [P] [US1] Write `test/WIP/funcspec-max-groups-env.cpp`: set `CRS_DEFAULT_FUNC_SPEC_MAX_GROUPS=2` with `CRS_DEFAULT_PIPELINE=1`; verify the specialization succeeds; set `CRS_DEFAULT_FUNC_SPEC_MAX_GROUPS=1` with `CRS_DEFAULT_PIPELINE=0`; FileCheck for `WARNING` about inapplicable option

### Implementation for User Story 1

- [ ] T011 [US1] In `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.cpp` `specializeOnlyImpl()`: after resolving `Options`, call `getRegisteredPipelines()` and validate `OptimizationPipelineToUse`; if out of range, emit `CRS_LOG(WARNING, ...)` and clamp to 0
- [ ] T012 [US1] In `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.cpp` `specializeOnlyImpl()`: add WARNING if `FuncSpecMaxGroups > 0` and pipeline == 0 ("FuncSpecMaxGroups ignored by inlining pipeline"); add WARNING if `MaxFixpointIterations > 1` and pipeline == 1 ("MaxFixpointIterations > 1 has no effect on func-spec pipeline")
- [ ] T013 [US1] Run `ninja check-wip-runtime-specializer` from `llvm/llvm/build/debug`; iterate until all 4 WIP tests (T007–T010) pass
- [ ] T014 [US1] Run `ninja check-smoke-runtime-specializer` from `llvm/llvm/build/debug` to confirm the smoke suite is unbroken

**Checkpoint**: `CRS_DEFAULT_PIPELINE` switches pipelines correctly; invalid indices produce WARNINGs without abort; renamed env vars work; smoke suite green.

---

## Phase 4: User Story 2 — Optimize Across Pipeline Selection (Priority: P1)

**Goal**: `optimize_benchmarks.py --search-space <descriptor.json>` drives Optuna using the descriptor's parameters; when `--search-space` is omitted the built-in default (6 parameters including `pipeline`) is used; the descriptor is written to `optimization_sessions.search_space_json` at study start.

**Independent Test**: Run `optimize_benchmarks.py` against a compiled benchmark binary with `--search-space` pointing to a descriptor that includes `pipeline` as a `categorical` with `choices: [0, 1]`. After the study completes, query DuckDB and verify: (a) some trials have `pipeline=0` and others `pipeline=1` in `params_json`; (b) `optimization_sessions.search_space_json` contains the exact descriptor used.

> **Note**: US2 is fully independent of US1/US3 C++ work — it can run in parallel with Phase 3.

### Implementation for User Story 2

- [ ] T015 [P] [US2] Add `DEFAULT_SEARCH_SPACE` dict constant to `benchmarks/optimize_benchmarks.py` with 6 entries covering all 7 env-var-exposed `Options` fields using new env var names and types from `contracts/search-space-descriptor.md`
- [ ] T016 [P] [US2] Implement `_load_descriptor(path: Path | None) -> dict` in `benchmarks/optimize_benchmarks.py`: if `path` is None return `DEFAULT_SEARCH_SPACE`; otherwise parse JSON from file, validate `version == 1`, validate each entry has required fields and `type` is one of the 7 known types, raise `SystemExit` with descriptive message on validation failure
- [ ] T017 [US2] Implement `_sample_params(trial: optuna.Trial, descriptor: dict) -> dict[str, Any]` in `benchmarks/optimize_benchmarks.py`: for each parameter entry dispatch on `type` to the appropriate `trial.suggest_*` call; `int_or_zero` uses `suggest_categorical([True, False])` + conditional `suggest_int`; return `{name: value}` dict
- [ ] T018 [US2] Implement `_params_to_env(params: dict[str, Any], descriptor: dict) -> dict[str, str]` in `benchmarks/optimize_benchmarks.py`: build `{entry.env_var: str(params[entry.name])}` for each descriptor entry; used to override subprocess env vars
- [ ] T019 [US2] Add `--search-space PATH` (optional, default None) argument to `argparse` in `benchmarks/optimize_benchmarks.py`; remove `--fixpoint-hi`, `--unroll-hi`, `--large-mod-hi` arguments (replaced by descriptor); call `_load_descriptor(args.search_space)` after parsing and store as `descriptor`
- [ ] T020 [US2] Update `objective()` closure in `benchmarks/optimize_benchmarks.py` to replace the hardcoded `magic` dict construction with `magic = _sample_params(trial, descriptor)` and replace `_make_env(magic)` with `{**os.environ, **_params_to_env(magic, descriptor)}`; update `store_trial_params` call to pass the resulting `magic` as `params_json`
- [ ] T021 [US2] Update the `optimization_sessions` INSERT in `benchmarks/optimize_benchmarks.py` (session start, currently around line 564) to include `search_space_json` in the column list and pass `json.dumps(descriptor)` as its value
- [ ] T022 [US2] Manual validation: run `optimize_benchmarks.py <binary> --n-trials 6 --n-parallel 1 --search-space <path-to-2-pipeline-descriptor.json>`; verify in DuckDB that `optim_trial_params.params_json` is non-NULL for all 6 rows, that both `pipeline=0` and `pipeline=1` appear across trials, and that `optimization_sessions.search_space_json` equals the descriptor file contents

**Checkpoint**: Descriptor-driven optimization works end-to-end; built-in default covers all 6 tunable parameters; study metadata is fully reproducible from DB.

---

## Phase 5: User Story 3 — Pipeline Code Separation (Priority: P2)

**Goal**: Each JIT compilation pipeline lives in its own source file; adding pipeline N requires creating one new file and editing only `JITPipelineRegistry.cpp`; no existing pipeline file is modified.

**Independent Test**: After extraction, run `ninja check-smoke-runtime-specializer` — all existing smoke tests must pass. Then set `CRS_DEFAULT_PIPELINE=0` and `CRS_DEFAULT_PIPELINE=1` and verify both produce correct results.

- [ ] T023 [P] [US3] Create `runtime/ClangRuntimeSpecializer/JITPipelineInlining.cpp`: extract pipeline 0 body from the IR transform lambda in `ClangRuntimeSpecializer.cpp` (initial pass + fixpoint loop + O3 final), implement as `llvm::Error runInliningPipeline(PipelineRunArgs& Args)` using `Args.Mod`, `Args.Opts`, `Args.PB`, `Args.PIC`, `Args.CurrentGroup`, `Args.CurrentFixpointIter`, `Args.IsLargeModule` in place of the previously captured/global references; add matching header `JITPipelineInlining.h`
- [ ] T024 [P] [US3] Create `runtime/ClangRuntimeSpecializer/JITPipelineFuncSpec.cpp`: extract pipeline 1 body from the IR transform lambda in `ClangRuntimeSpecializer.cpp` (linkage conversion + ConstantArgFunctionSpecializationPass fixpoint), implement as `llvm::Error runFuncSpecPipeline(PipelineRunArgs& Args)`; add matching header `JITPipelineFuncSpec.h`
- [ ] T025 [US3] Create `runtime/ClangRuntimeSpecializer/JITPipelineRegistry.cpp`: include `JITPipelineInlining.h` and `JITPipelineFuncSpec.h`; define `static const PipelineEntry kPipelines[] = {{"inlining", runInliningPipeline}, {"func-spec", runFuncSpecPipeline}};`; define `llvm::ArrayRef<PipelineEntry> getRegisteredPipelines() { return kPipelines; }`
- [ ] T026 [US3] Update the IR transform lambda in `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.cpp`: replace the inline pipeline bodies with a `PipelineRunArgs` construction and a call to `getRegisteredPipelines()[ValidatedPipeline].Run(Args)`; the `PIC` callbacks and trace state setup remain in the outer lambda
- [ ] T027 [US3] Add `JITPipelineInlining.cpp`, `JITPipelineFuncSpec.cpp`, `JITPipelineRegistry.cpp` to the source list in `runtime/ClangRuntimeSpecializer/CMakeLists.txt`
- [ ] T028 [US3] Run `ninja check-smoke-runtime-specializer` from `llvm/llvm/build/debug` to verify zero regressions from the extraction

**Checkpoint**: Both pipelines are in dedicated TUs; `JITPipelineInlining.cpp` and `JITPipelineFuncSpec.cpp` contain zero references to each other; smoke suite green; adding a third pipeline requires only the steps in `quickstart.md`.

---

## Phase 6: User Story 4 — Cross-Study Query Validation (Priority: P3)

**Goal**: Trial records from studies with different search spaces coexist in the same DuckDB file; `params_json` is non-NULL for every row; new parameters appear in `params_json` without schema migration.

**Independent Test**: Run two studies with different descriptors (e.g., one without `pipeline`, one with). Query `SELECT COUNT(*), COUNT(params_json) FROM optim_trial_params` and verify both counts are equal. Query `optimization_sessions` and verify `search_space_json` is populated for new studies.

- [ ] T029 [US4] Manual validation: after completing US2 tasks, run two consecutive `optimize_benchmarks.py` studies — one with the built-in default descriptor (6 params) and one with a custom descriptor that adds a 7th fictional parameter (e.g., `CRS_EXTRA_TEST=1`); open DuckDB and verify: (a) `SELECT COUNT(*) = COUNT(params_json) FROM optim_trial_params` (no NULLs); (b) rows from the first study have 6-key `params_json`, rows from the second have 7-key `params_json`; (c) old rows (if any, from before this feature) are unaffected; (d) `optimization_sessions.search_space_json` matches the descriptor used for each study

**Checkpoint**: Schema extensibility confirmed; cross-study queries work without migration.

---

## Final Phase: Polish & Cross-Cutting Concerns

- [ ] T030 Promote `test/WIP/pipeline-dispatch.cpp`, `test/WIP/pipeline-invalid-index.cpp`, `test/WIP/options-env-vars.cpp`, `test/WIP/funcspec-max-groups-env.cpp` to `test/smoke/` (update RUN lines if needed) and run `ninja check-smoke-runtime-specializer` to confirm they pass as smoke tests
- [ ] T031 Run `ninja check-all-runtime-specializer` from `llvm/llvm/build/debug`; investigate any failures
- [ ] T032 [P] Add a concise bullet to the "JIT Optimization Pipeline" section of `docs/thesis.typ` noting that pipelines are now separated into dedicated TUs (`JITPipelineInlining`, `JITPipelineFuncSpec`) and that pipeline selection is controlled by `CRS_DEFAULT_PIPELINE` with validated clamping

---

## Dependencies & Execution Order

### Phase Dependencies

```
Phase 1 (Setup)
  └─→ Phase 2 (Foundation — JITPipeline.h)
        ├─→ Phase 3 (US1 — dispatch/validation) ──────────────────┐
        └─→ Phase 5 (US3 — pipeline extraction)  ──────────────────┤
                                                                    ↓
Phase 4 (US2 — Python descriptor) [INDEPENDENT — runs in parallel] ┤
  └─→ Phase 6 (US4 — cross-study query validation)                 │
                                                                    ↓
                                                   Final Phase (Polish)
```

### User Story Dependencies

- **US1 (P1)**: Requires Phase 1 + Phase 2. No dependency on US2 or US3.
- **US2 (P1)**: Requires Phase 1 (for new env var names in DEFAULT_SEARCH_SPACE). No dependency on US1 or US3. **Can run in parallel with Phase 3.**
- **US3 (P2)**: Requires Phase 2 (JITPipeline.h). No dependency on US1 or US2. Can start after Phase 2.
- **US4 (P3)**: Requires US2 completion (params_json must be populated by descriptor-driven objective).

### Within Each Phase

- Tests (T007–T010) MUST be written before implementation (T011–T012) — constitution §III
- T023 and T024 are parallel (different files, no dependency on each other)
- T015 and T016 are parallel (different functions in same file — write to different regions)

---

## Parallel Opportunities

### Phase 3 (US1) — Test-writing tasks can run in parallel:
```
T007: test/WIP/pipeline-dispatch.cpp
T008: test/WIP/pipeline-invalid-index.cpp
T009: test/WIP/options-env-vars.cpp
T010: test/WIP/funcspec-max-groups-env.cpp
```

### Phase 4 (US2) — Descriptor infrastructure can start in parallel with Phase 3:
```
T015: DEFAULT_SEARCH_SPACE dict
T016: _load_descriptor()
```

### Phase 5 (US3) — Pipeline extraction:
```
T023: JITPipelineInlining.cpp  (independent of T024)
T024: JITPipelineFuncSpec.cpp  (independent of T023)
```

---

## Implementation Strategy

### MVP (US1 only — pipeline switching & validation)

1. Complete Phase 1 (Setup)
2. Complete Phase 2 (Foundation)
3. Complete Phase 3 (US1: tests + WARNING/clamp + env var reading)
4. **Validate**: `CRS_DEFAULT_PIPELINE=0/1/99` all behave correctly; smoke suite green

### Full delivery order

1. Phase 1 → Phase 2 → Phase 3 (US1) + Phase 4 (US2) in parallel → Phase 5 (US3) → Phase 6 (US4) → Final

---

## Notes

- [P] tasks = can be worked on in parallel within the same phase (different files or independent regions)
- Constitution §III mandates WIP test files exist and fail BEFORE implementing T011–T012
- The ninja gate tasks (T013, T014, T028, T031) are hard checkpoints — do not proceed past them with failing tests
- All env var renames in Phase 1 are atomic with optimize_benchmarks.py updates in Phase 4 (T019–T020 use new names in DEFAULT_SEARCH_SPACE and _params_to_env)
- quickstart.md documents the process for adding a fourth pipeline after US3 is complete
