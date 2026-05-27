# Feature Specification: Pipeline 2 — JIT-IPSCCP

**Feature Branch**: `015-pipeline2-jit-ipsccp`  
**Created**: 2026-05-27  
**Status**: Draft

## User Scenarios & Testing *(mandatory)*

### User Story 1 — Select P2 and Get Better Specialization on Virtual Dispatch (Priority: P1)

A researcher benchmarking JIT specialization selects pipeline 2 (`CRS_DEFAULT_PIPELINE=2`) for a use case that involves virtual method calls or lambda captures. The specialization correctly eliminates virtual dispatch and propagates captured values as constants — producing specialized code at least as good as pipeline 0 on these patterns, without the code-size explosion risk of always-inline.

**Why this priority**: This is the primary motivation for P2. Pipelines 0 and 1 both have gaps for virtual dispatch: P0 uses a separate devirt pass that only runs per fixpoint iteration; P1 is missing devirt entirely. P2 subsumes both into a single pass that resolves vtable chains natively.

**Independent Test**: Can be tested by running the `virtual-methods` and `virtual-methods-simple` smoke tests with `CRS_DEFAULT_PIPELINE=2` and verifying that vtable loads are eliminated in the specialized output.

**Acceptance Scenarios**:

1. **Given** a benchmark where the target function dispatches through a virtual method on a runtime-constant object pointer, **When** pipeline 2 is selected, **Then** the specialized function contains no indirect virtual calls and the virtual method's body is inlined or directly called.
2. **Given** a benchmark that passes a lambda closure as a constant pointer, **When** pipeline 2 is selected, **Then** invariant-load values from the lambda's captured fields are propagated as constants through the specialized function.
3. **Given** a call to `specializeOnly` or `callSpecialized` with `Options` selecting pipeline 2, **When** the JIT compiles, **Then** no outer fixpoint loop runs — JitSCCPPass converges internally.

---

### User Story 2 — Fork JIT-IPSCCP with Clean Git History for Future Rebasing (Priority: P1)

A developer adding JIT-specific extensions to the IPSCCP solver follows a 3-commit discipline: (1) verbatim copy of upstream LLVM files, (2) compile-only adjustments with no logic changes, (3) JIT logic divergence. This makes it possible to `git rebase` the JIT-specific commits onto a newer LLVM IPSCCP version without losing the extension work.

**Why this priority**: Without the commit discipline, any future LLVM upgrade requires hand-merging a large, modified file with no clear diff boundary between "what LLVM changed" and "what we added." The 3-commit structure makes `git diff HEAD~2..HEAD~1` show only compilation adjustments and `git diff HEAD~1..HEAD` show only JIT logic — enabling targeted rebasing.

**Independent Test**: Can be tested by inspecting git log on the JitSCCP files: commit 1 should produce a zero-diff against the upstream source files; commit 2 should compile cleanly with zero logic changes; commit 3+ should contain only the JIT extensions.

**Acceptance Scenarios**:

1. **Given** the initial copy commit, **When** diffed against the upstream LLVM source files at the same version, **Then** the diff is empty (verbatim copy).
2. **Given** commit 2 (compile adjustments), **When** reviewed, **Then** only include/namespace/rename changes are present — no changes to visitor logic, lattice operations, or cost model.
3. **Given** commits 3+, **When** reviewed, **Then** changes are isolated to `visitLoadInst` invariant-load extension, `visitCallBase` indirect-call resolution, and `JitFunctionSpecializationOptions` integration.

---

### User Story 3 — Tune P2 Cost Model Parameters via Environment Variables (Priority: P2)

A researcher using `optimize.py` to find the best pipeline configuration can sweep P2-specific parameters (minimum function size, max clones, specialization iterations, force-specialization flag) independently from P0/P1 parameters. When pipeline 0 or 1 is selected, P2-specific parameters are not sampled (and vice versa), avoiding wasted Optuna trials.

**Why this priority**: The IPSCCP cost model has defaults calibrated for AOT compilation (e.g., MinFunctionSize=500) that are wrong for JIT specialization. The optimal values are use-case-dependent and must be discoverable through automated search. Pipeline-conditional sampling avoids corrupting the Optuna search history with irrelevant parameter combinations.

**Independent Test**: Can be tested by running `optimize.py` with `pipeline=2` and verifying that only P2-prefixed env vars are set in the trial environment; and with `pipeline=0` verifying that P2 vars are absent.

**Acceptance Scenarios**:

1. **Given** an Optuna trial where `pipeline=2` is sampled, **When** the trial environment is constructed, **Then** `CRS_P2_MIN_FUNC_SIZE`, `CRS_P2_MAX_CLONES`, `CRS_P2_FUNC_SPEC_ITERS`, and `CRS_P2_FORCE_SPEC` are set; P0/P1-specific vars are absent.
2. **Given** an Optuna trial where `pipeline=0` or `pipeline=1` is sampled, **When** the trial environment is constructed, **Then** no `CRS_P2_*` vars are present.
3. **Given** `CRS_P2_MIN_FUNC_SIZE=1 CRS_P2_FORCE_SPEC=1`, **When** JitSCCPPass runs, **Then** functions with fewer than 500 instructions are eligible for specialization and the profitability cost model is bypassed.

---

### User Story 4 — Add P2 Results to Benchmark Database and Analysis Reports (Priority: P3)

Benchmark runs with `CRS_DEFAULT_PIPELINE=2` are recorded in the DuckDB benchmark database alongside P0 and P1 results, with the pipeline value stored as a parameter. Existing analysis scripts and visualizations work without modification because P2 is simply another value of the `pipeline` parameter column.

**Why this priority**: The benchmark infrastructure already stores all `CRS_*` env vars as parameters. P2 results will appear automatically once the pipeline is registered. This story is about verifying that no schema changes are needed and that comparisons between P0/P1/P2 are immediately possible in SQL.

**Independent Test**: Can be tested by running a benchmark with `CRS_DEFAULT_PIPELINE=2`, importing it into DuckDB, and querying `SELECT pipeline, AVG(jit_overhead_ms) FROM results GROUP BY pipeline` to confirm P2 rows appear alongside P0 and P1.

**Acceptance Scenarios**:

1. **Given** a benchmark run with pipeline 2, **When** imported into DuckDB, **Then** `SELECT * FROM params WHERE key='CRS_DEFAULT_PIPELINE' AND value='2'` returns rows.
2. **Given** existing Pareto analysis scripts, **When** run against a database containing P0, P1, and P2 results, **Then** all three pipelines appear in the output without script modifications.

---

### Edge Cases

- What happens when `JitSCCPPass` is given a module where the target function is already fully constant-folded (no invariant loads, no vtable calls)? The solver should converge in one iteration and produce the same result as IPSCCP without extensions.
- What happens when an invariant-load pointer chain cannot be resolved (e.g., non-mapped page, null pointer)? The extension must fall through to the existing `mergeInValue` metadata path without crashing.
- What happens when a vtable slot resolves to a function not present in the JIT module (external symbol)? The indirect call remains as-is; solver marks callee as overdefined.
- What happens when `CRS_P2_MAX_CLONES=0` (unlimited) and there are many distinct constant patterns? The solver creates clones for all patterns; growth is bounded by `P1MaxModuleGrowth`-equivalent cap.
- What happens when `ForceSpecialization=true` and `MinFunctionSize=1` together? All functions with any constant argument are specialized, regardless of profitability. Module growth cap provides the only safety valve.

## Requirements *(mandatory)*

### Functional Requirements

**JitSCCP Fork**

- **FR-001**: The project MUST contain a local copy of `SCCPSolver.h/cpp`, `FunctionSpecialization.h/cpp`, and `IPSCCPPass` (SCCP.h/cpp IPO) under `runtime/ClangRuntimeSpecializer/JitSCCP/`, renamed to avoid ODR conflicts (`JitSCCPSolver`, `JitFunctionSpecializer`, `JitIPSCCPPass`).
- **FR-002**: The initial copy commit MUST be verbatim (zero diff against the upstream LLVM source at the version in use). Subsequent commits introduce compile adjustments, then JIT logic, in separate commits.
- **FR-003**: `JitSCCPInstVisitor::visitLoadInst` MUST resolve loads marked `!invariant.load` to host-memory constants when the pointer chain resolves to a mapped address, using the same resolution logic as `InvariantLoadToConstantPass`. The resolved value MUST be marked as a lattice constant via `markConstant`.
- **FR-004**: `JitSCCPInstVisitor::visitCallBase` MUST detect when the callee's lattice value is a constant function pointer to a function present in the JIT module, and treat the call as a tracked direct call, propagating constants from the call site into the callee via the solver's argument-tracking mechanism.
- **FR-005**: The pointer-chain resolution logic (vtable dereference chain, `mincore` page check, host memory read) MUST be factored into a shared utility reused by both `visitLoadInst` and `visitCallBase` extensions, and shared with `InvariantLoadToConstantPass`.

**JitFunctionSpecializationOptions**

- **FR-006**: A `JitFunctionSpecializationOptions` struct MUST exist with fields: `MinFunctionSize` (default 1), `MaxClones` (default 0 = unlimited), `FuncSpecMaxIters` (default 10), `ForceSpecialization` (default false), `SpecializeOnAddress` (default false), `SpecializeLiteralConstant` (default true).
- **FR-007**: `JitFunctionSpecializationOptions` MUST be read from `CRS_P2_*` env vars via `_envOr` in `Options::Default()` and stored as a nested field in the main `Options` struct.
- **FR-008**: `JitIPSCCPPass` MUST accept `JitFunctionSpecializationOptions` at construction time and pass them to `JitFunctionSpecializer`, replacing all `cl::opt` globals for these parameters.

**Pipeline 2 Structure**

- **FR-009**: Pipeline 2 MUST be registered in `JITPipelineRegistry.cpp` under index 2 with name `"jit-ipsccp"`.
- **FR-010**: The P2 pipeline MUST implement the following sequence with no outer fixpoint loop: linkage scrub (BFS AvailableExternal→Internal, strip AlwaysInline) → early `GlobalDCEPass` → `JitIPSCCPPass(AllowFuncSpec=true)` → `GlobalDCEPass` → `AlwaysInlinerPass` → `GlobalDCEPass` + `SimplifyCFGPass` + `InstCombinePass` cleanup.
- **FR-011**: `DevirtualizeConstantVtableCallsPass` and `WholeProgramDevirtPass` MUST NOT appear in the P2 pipeline (their function is subsumed by `JitIPSCCPPass`'s solver extensions).
- **FR-012**: `InvariantLoadToConstantPass` and `StaticMutabilityAnalysisPass` MUST NOT appear as standalone passes in the P2 pipeline main sequence (subsumed by solver). They MAY be run as a final cleanup pass after the inliner to replace any remaining invariant loads in the inlined body.

**optimize.py Parameter Domains**

- **FR-013**: The `pipeline` parameter in `optimize.py` MUST become a categorical integer choice `[0, 1, 2]` (currently a bool `[0, 1]`).
- **FR-014**: `optimize.py` MUST use conditional sampling: P1-specific parameters (`p1_inline_threshold`, `p1_max_module_growth`) are only sampled when `pipeline=1`; P2-specific parameters (`p2_min_func_size`, `p2_max_clones`, `p2_func_spec_iters`, `p2_force_spec`) are only sampled when `pipeline=2`.
- **FR-015**: Inactive pipeline parameters MUST be recorded as `None` (or absent) in the DuckDB params table for non-matching pipeline trials.

### Key Entities

- **JitSCCPSolver**: Local copy of LLVM's `SCCPSolver` with `JitSCCPInstVisitor` inner class extended for invariant-load and vtable-devirt resolution.
- **JitFunctionSpecializer**: Local copy of LLVM's `FunctionSpecializer` accepting `JitFunctionSpecializationOptions` instead of `cl::opt` globals.
- **JitIPSCCPPass**: Local copy of LLVM's `IPSCCPPass` wiring `JitSCCPSolver` and `JitFunctionSpecializer` together; registered as pipeline 2.
- **JitFunctionSpecializationOptions**: New struct in `ClangRuntimeSpecializer.h` containing all cost-model knobs, backed by `CRS_P2_*` env vars.
- **PointerChainResolver**: Shared utility factoring out invariant-load pointer resolution, `mincore` check, and host memory read — used by solver extensions and `InvariantLoadToConstantPass`.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: All existing smoke tests that pass with pipeline 0 or 1 also pass with pipeline 2 when the same use case is exercised (`ninja check-smoke-runtime-specializer` with `CRS_DEFAULT_PIPELINE=2`).
- **SC-002**: The `virtual-methods` and `virtual-methods-simple` smoke tests pass with a P2 `RUN:` line verifying vtable loads are eliminated from the specialized output, matching pipeline 0's existing behaviour.
- **SC-003**: The initial JitSCCP copy commit produces a zero byte-level diff against the upstream LLVM source files it copies.
- **SC-004**: `optimize.py` Optuna trials with `pipeline=0` contain no `CRS_P2_*` env vars; trials with `pipeline=2` contain no `CRS_P1_*` env vars.
- **SC-005**: A benchmark run with `CRS_DEFAULT_PIPELINE=2 CRS_P2_MIN_FUNC_SIZE=1 CRS_P2_FORCE_SPEC=0` on a virtual-method use case achieves JIT specialization overhead within 2× of pipeline 0 on the same use case (measured by `benchmarkJITOverhead`).

## Assumptions

- The upstream LLVM version in the project tree is fixed for the duration of this feature; no mid-feature LLVM upgrade is expected.
- `InvariantLoadToConstantPass`'s pointer-chain resolution logic can be cleanly extracted into a shared utility (`PointerChainResolver`) without changing its observable behaviour.
- The `AlwaysInlinerPass` after `JitIPSCCPPass` is sufficient to inline the specialized clones created by `JitFunctionSpecializer`; a cost-based inliner is not needed in P2.
- `PredicateInfo` (used by `SCCPSolver`) remains as a public LLVM header include and does not need to be copied into the project.
- Pipeline 2 is additive: P0 and P1 continue to work unchanged. The pipeline selector in `Options` determines which of the three pipelines runs.
- The DuckDB benchmark schema requires no changes: `CRS_DEFAULT_PIPELINE=2` is stored as a parameter value like any other.
