# Feature Specification: JIT Pipeline Infrastructure

**Feature Branch**: `002-jit-pipeline-infrastructure`  
**Created**: 2026-04-24  
**Status**: Draft  
**Input**: User description: "multiple JIT compilation pipelines with clean code separation, options that determine which pipeline is used, settable and optimizable by optimize_benchmarks.py; infrastructure only — inliner and constant-arg pipelines covered in separate specs"

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Compare Pipeline Strategies (Priority: P1)

A researcher wants to compare any number of registered JIT compilation strategies to determine which yields better execution performance on a given workload. They run the same benchmark suite once per strategy — selecting a strategy by changing a single option value — while all measurement, recording, and reporting infrastructure works identically for every run.

**Why this priority**: Comparing strategies is the primary research goal; without the ability to switch cleanly, no meaningful comparison is possible.

**Independent Test**: Build and run any existing benchmark binary with `CRS_DEFAULT_PIPELINE=0` and then again with `CRS_DEFAULT_PIPELINE=1` (or any other registered index); verify that each run completes, produces valid DuckDB records, and that the recorded `kv_g` / `kv_t` phase rows differ in timing as expected for each strategy.

**Acceptance Scenarios**:

1. **Given** a compiled benchmark binary, **When** the researcher sets `CRS_DEFAULT_PIPELINE=0` (or `1`) and records the run, **Then** the JIT uses the corresponding pipeline for every specialization call in that process.
2. **Given** `CRS_DEFAULT_PIPELINE` is not set, **When** the benchmark runs, **Then** the JIT uses pipeline 0 (the default inlining pipeline) with no user intervention.
3. **Given** an invalid `CRS_DEFAULT_PIPELINE` value (e.g., `99`), **When** the benchmark runs, **Then** the JIT logs a WARNING-level diagnostic, clamps to the nearest valid pipeline index, and continues execution without aborting.
4. **Given** pipeline-specific options (e.g., max clone groups for the cloning pipeline), **When** the selected pipeline does not use those options, **Then** the JIT logs a WARNING-level diagnostic for each non-applicable non-default option value but does not abort.

---

### User Story 2 - Optimize Across Pipeline Selection (Priority: P1)

A researcher wants to include pipeline selection as a parameter in an automated Optuna optimization sweep, discovering which combination of pipeline index and pipeline-specific settings minimizes total JIT + execution overhead for each kernel in the workload. The search space is described externally — in a hand-editable JSON file — so that adding a new pipeline or a new parameter requires no code changes to the optimizer script. The descriptor and all trial parameters are stored in the database so the exact optimization setup is always reproducible.

**Search Space Descriptor Design**: The format is a JSON document modeled on Optuna's `suggest_*` API (the same semantic model used by W&B Sweeps and NNI). Each parameter entry declares its name (Optuna trial key), the ENV var the optimizer sets when invoking the benchmark subprocess, a `type`, and type-specific fields. The types map one-to-one to Optuna calls:

| Type | Optuna call | Fields |
|---|---|---|
| `int` | `suggest_int(min, max)` | `min`, `max` |
| `log_int` | `suggest_int(min, max, log=True)` | `min`, `max` |
| `float` | `suggest_float(min, max)` | `min`, `max` |
| `log_float` | `suggest_float(min, max, log=True)` | `min`, `max` |
| `bool` | `suggest_categorical([0, 1])` | — |
| `categorical` | `suggest_categorical(choices)` | `choices` |
| `int_or_zero` | `suggest_categorical([0]) ∪ suggest_int(min, max, log=True)` | `min`, `max` |

Example descriptor:
```json
{
  "version": 1,
  "parameters": [
    { "name": "fixpoint_max",    "env_var": "CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS",     "type": "int",        "min": 1,    "max": 20     },
    { "name": "unroll_max",      "env_var": "CRS_DEFAULT_LOOP_UNROLL_COUNT",            "type": "log_int",    "min": 1,    "max": 256    },
    { "name": "large_module_max","env_var": "CRS_DEFAULT_LARGE_MODULE_INSTR_THRESHOLD", "type": "int_or_zero","min": 1000, "max": 100000 },
    { "name": "early_prune",     "env_var": "CRS_DEFAULT_EARLY_PRUNE",                 "type": "bool"                                  },
    { "name": "o3_final",        "env_var": "CRS_DEFAULT_O3_FINAL",                    "type": "bool"                                  },
    { "name": "pipeline",        "env_var": "CRS_DEFAULT_PIPELINE",                    "type": "categorical","choices": [0, 1]          }
  ]
}
```

**DB schema for parameters**: Rather than extending `optim_trial_params` with a new fixed column for every new option (which requires schema migrations and breaks historical queries), the chosen approach is a **hybrid schema**: fixed columns exist only for the original 5 parameters (maintained for backwards-compatible queries), while a `params_json` column stores the complete parameter set as a JSON object for every trial. New parameters appear only in `params_json` — no migration required. The descriptor itself is stored in `optimization_sessions.search_space_json` so the exact search space for any historical study is always queryable.

**Why this priority**: Pipeline comparison is the central research question; automating the sweep across strategies is the direct empirical method for answering it.

**Independent Test**: Run `optimize_benchmarks.py` with `--search-space <descriptor.json>` where the descriptor includes a `pipeline` parameter of type `categorical` with choices `[0, 1]`; verify that some trials use pipeline 0 and others use pipeline 1, that results for all trials are stored in the DuckDB `optim_trial_params` table (with `params_json` populated), and that `v_optim_best_per_kernel` returns results attributed to the winning pipeline.

**Acceptance Scenarios**:

1. **Given** a search space descriptor that includes `pipeline` as a categorical parameter, **When** `optimize_benchmarks.py` runs, **Then** trials are sampled across all listed pipeline choices and the winning pipeline is identified.
2. **Given** a search space descriptor that omits `pipeline`, **When** `optimize_benchmarks.py` runs, **Then** the optimizer behaves as before (single pipeline, fixed by whatever ENV var is set in the environment).
3. **Given** no `--search-space` flag, **When** `optimize_benchmarks.py` runs, **Then** it uses a built-in default descriptor that covers all standardized ENV-var-exposed options.
4. **Given** a descriptor that includes a new parameter not in the current fixed `optim_trial_params` columns, **When** a trial is written, **Then** all parameters appear in `params_json`; no schema migration is required.
5. **Given** a study completes, **When** the researcher queries `optimization_sessions`, **Then** the `search_space_json` column contains the exact descriptor used for that study, so the optimization setup is fully reproducible from the database alone.

---

### User Story 3 - Add a New Pipeline Without Regressions (Priority: P2)

A developer implementing a new JIT compilation strategy wants to register it alongside the existing pipelines. After the addition, selecting the new pipeline via the option works correctly; the existing pipelines are unaffected (no regressions); and the new pipeline's specific options follow the standard ENV var convention so they are immediately discoverable and optimizable.

**Why this priority**: Supports the research workflow of exploring new specialization approaches without destabilizing validated baselines.

**Independent Test**: After adding the new pipeline, run `ninja check-smoke-runtime-specializer`; verify all existing smoke tests pass. Set `CRS_DEFAULT_PIPELINE=<new_index>` and run any benchmark; verify it completes and produces valid records.

**Acceptance Scenarios**:

1. **Given** a new pipeline added as a dedicated source file implementing the pipeline interface, **When** the project is built, **Then** all existing tests pass and the new pipeline is reachable via its designated option value.
2. **Given** a new pipeline option (ENV var) follows the `CRS_DEFAULT_<FIELDNAME_UPPERCASE>` naming convention, **When** a researcher adds it to a search space descriptor, **Then** `optimize_benchmarks.py` tunes it without code changes.
3. **Given** the new pipeline source file is the only file modified or added, **When** the diff is reviewed, **Then** no existing pipeline source file has been modified.

---

### User Story 4 - Query Trial Results Across Heterogeneous Search Spaces (Priority: P3)

A researcher reviewing optimization history has trials from different studies — some that tuned only the 5 classic parameters, some that also included pipeline selection, and some that included future per-pipeline parameters not yet known. They want to query and compare these trials in a single DuckDB session without encountering schema errors or silent data loss.

**Why this priority**: Long-term data integrity matters for thesis repeatability; this is a prerequisite before schema-breaking parameter additions are made.

**Independent Test**: Insert trials from two studies with different search spaces into the same DuckDB file; query `SELECT * FROM optim_trial_params` and confirm all rows are present; query `params_json` column; verify rows without certain fixed columns have NULL in those columns but valid JSON in `params_json`.

**Acceptance Scenarios**:

1. **Given** trials from two studies with different parameter sets exist in the same DB, **When** the researcher queries `optim_trial_params`, **Then** all rows are returned; fixed columns are NULL for parameters absent from that study's search space; `params_json` is non-NULL and contains the complete parameter set for every row.
2. **Given** a new parameter is added to a search space descriptor, **When** a new study is run and stored, **Then** the new parameter appears in `params_json` of new rows; existing rows are not modified; no migration script is required.
3. **Given** a researcher wants to filter by the new parameter, **When** they query `params_json::json->'new_param'`, **Then** they retrieve valid values from all studies that included it.

---

### Edge Cases

- What happens when `CRS_DEFAULT_PIPELINE` is set to a valid index but pipeline-specific ENV vars are missing? → Pipeline-specific options use their in-Options defaults; no error.
- What happens when two search space descriptors define the same parameter with conflicting ranges? → The descriptor loaded last (the file on disk) takes precedence; the built-in default is overridden entirely.
- What happens when `params_json` is queried for a trial written before the column was added? → The column is NULL for those rows (pre-migration rows); this is acceptable and documented.
- What happens when a pipeline source file fails to compile? → Standard build error; no silent degradation. The pipeline cannot be registered if its translation unit doesn't compile.
- What happens when MaxFixpointIterations is set to a value not supported by the selected pipeline (e.g., the cloning pipeline doesn't use a fixpoint loop)? → The JIT logs a WARNING and ignores the option; the pipeline runs with its own fixed behavior.

## Requirements *(mandatory)*

### Functional Requirements

**JIT System — Pipeline Selection & Options**

- **FR-001**: The JIT system MUST support a set of named, mutually-exclusive compilation pipelines identified by non-negative integer indices. Selecting a pipeline MUST be controlled by exactly one field in the `Options` struct (`OptimizationPipelineToUse`).
- **FR-002**: `Options::Default()` MUST read the pipeline selector from the `CRS_DEFAULT_PIPELINE` environment variable. When absent or unparseable, it MUST default to pipeline index 0.
- **FR-003**: Every `Options` field that is intended to be tunable by `optimize_benchmarks.py` MUST have a corresponding environment variable following the naming convention `CRS_DEFAULT_<FIELDNAME_UPPERCASE>`. `Options::Default()` MUST read all such ENV vars.
- **FR-004**: When an `Options` field receives an out-of-range or pipeline-incompatible value (whether from ENV vars or direct assignment), the JIT system MUST emit a `WARNING`-level log message and clamp or ignore the value. It MUST NOT assert, throw, or abort.
- **FR-005**: The preset factories `Default()`, `O3Only()`, `Aggressive()`, `Fast()`, `NoOptimize()`, and `FromExpectedRuntime()` MUST produce `Options` values that are valid for all registered pipelines (i.e., they MUST NOT set the pipeline selector to a non-existent index).

**JIT System — Code Separation**

- **FR-006**: Each JIT compilation pipeline MUST be implemented in its own dedicated source file (translation unit). The implementation of pipeline N MUST NOT appear in the source file of pipeline M (N ≠ M).
- **FR-007**: The shared infrastructure (LLJIT setup, bitcode loading, wrapper generation, pass instrumentation, stats collection) MUST reside in a source file that is independent of any specific pipeline's pass sequences.
- **FR-008**: Adding a new pipeline MUST be achievable by creating one new source file and registering the pipeline in the infrastructure; it MUST NOT require modifications to any existing pipeline's source file. Any new `Options` fields introduced by the pipeline MUST follow the `CRS_DEFAULT_*` ENV var convention (FR-003) and MUST be expressible as entries in the search space descriptor (FR-010) without further code changes to `optimize_benchmarks.py`.

**Optimizer — Search Space Descriptor**

- **FR-009**: `optimize_benchmarks.py` MUST support a `--search-space PATH` argument. When supplied, the set of parameters to optimize, their types, ranges, and ENV var bindings MUST be loaded from a JSON file at `PATH` rather than from hardcoded values. When a study starts, the serialized descriptor JSON MUST be written to `optimization_sessions.search_space_json` so the exact search space is permanently queryable for that study.
- **FR-010**: The JSON search space descriptor MUST support at minimum these parameter types (each maps directly to an Optuna `suggest_*` call):
  - `int`: linear integer range (`min`, `max`) → `suggest_int(min, max)`
  - `log_int`: log-scaled integer range (`min`, `max`) → `suggest_int(min, max, log=True)`
  - `float`: linear float range (`min`, `max`) → `suggest_float(min, max)`
  - `log_float`: log-scaled float range (`min`, `max`) → `suggest_float(min, max, log=True)`
  - `bool`: binary choice (0 or 1) → `suggest_categorical([0, 1])`
  - `categorical`: explicit list of integer or string `choices` → `suggest_categorical(choices)`
  - `int_or_zero`: log-scaled integer range augmented with a zero choice; zero means "disabled" (as used for `LargeModuleInstrThreshold`)
- **FR-011**: Each entry in the search space descriptor MUST specify: `name` (used as the Optuna parameter name), `env_var` (the ENV var set when invoking the benchmark subprocess), `type` (one of FR-010), and the type-appropriate range/choice fields.
- **FR-012**: When `--search-space` is not provided, `optimize_benchmarks.py` MUST fall back to a built-in default descriptor equivalent to the current hardcoded search space (fixpoint, unroll, large_mod, early_prune, o3_final, pipeline).
- **FR-013**: Introducing a new ENV-var-exposed `Options` field MUST NOT require code changes to `optimize_benchmarks.py`; it MUST be expressible solely by adding an entry to a descriptor JSON file.

**DB Schema — Parameter Storage**

- **FR-014**: The `optim_trial_params` table MUST include a `params_json` VARCHAR column. On every trial write, this column MUST be populated with a JSON object containing the complete set of parameters for that trial (keyed by their `name` from the search space descriptor).
- **FR-015**: The existing fixed parameter columns in `optim_trial_params` (`fixpoint_max`, `unroll_max`, `large_module_max`, `early_prune`, `o3_final`) MUST be populated when the corresponding parameter appears in the active search space, and MUST be NULL when it does not appear (i.e., the search space did not include that parameter).
- **FR-016**: When a trial is written with parameters that have no corresponding fixed column (e.g., a future `func_spec_max_groups` parameter), those parameters MUST appear in `params_json` without requiring a schema migration. A fixed column MAY be added later via an explicit migration.
- **FR-017**: The `optimization_sessions` table MUST include a `search_space_json` VARCHAR column. When a study starts, the complete serialized descriptor (the JSON document, whether loaded from `--search-space` or the built-in default) MUST be written to this column so the exact search space definition is permanently associated with the study.

### Key Entities *(include if feature involves data)*

- **JITPipeline**: One compilation strategy implementation. Attributes: pipeline index (non-negative integer), display name, source file. Registered at compile time; identified by its index in `OptimizationPipelineToUse`.
- **SearchSpaceDescriptor**: A JSON document that defines the set of parameters `optimize_benchmarks.py` optimizes for a given study. Attributes: `version` integer, list of `parameters` each with `name`, `env_var`, `type`, and type-appropriate range/choices. Provided via `--search-space` or defaulted to the built-in descriptor. Persisted verbatim to `optimization_sessions.search_space_json` at study start.
- **ParameterDefinition**: One entry in a SearchSpaceDescriptor. Attributes: `name` (Optuna parameter key), `env_var` (ENV var set in benchmark subprocess), `type` (one of the 7 types in FR-010), plus `min`/`max` for range types or `choices` for categorical.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: A researcher can benchmark the same workload under two different pipeline strategies by changing a single option value; both runs complete without error and produce valid, separately queryable DuckDB records in under 5 additional seconds of setup overhead versus a single run.
- **SC-002**: Adding a new pipeline requires modifying zero existing source files; only one new source file is created plus a registration entry.
- **SC-003**: `optimize_benchmarks.py` can optimize across pipeline selection (including a new pipeline option not in the original hardcoded search space) using only a descriptor JSON file update, with zero changes to the Python source.
- **SC-004**: Trial records from studies with different search spaces coexist in the same DuckDB file and are all queryable via `params_json` without schema migration or data loss.
- **SC-005**: All existing smoke tests (`ninja check-smoke-runtime-specializer`) pass after the infrastructure refactoring; no regressions introduced.

## Assumptions

- Pipeline indices are non-negative integers assigned at compile time; runtime plugin loading is out of scope.
- The `Options` struct remains the single public configuration entry point; no per-pipeline sub-struct is exposed in the public API.
- The search space descriptor JSON format is versioned (a `version` field) to support future evolution; the initial version is `1`.
- `CRS_DEFAULT_PIPELINE` uses the integer index; human-readable pipeline names are for documentation only.
- The built-in default descriptor covers all ENV-var-exposed Options fields present at the time this feature is implemented; new fields added later require updating the default descriptor.
- Backwards compatibility of the `optim_trial_params` table: the existing 5 fixed columns are preserved; `params_json` is added as a new additive column. Fixed columns exist for fast equality-filter queries; `params_json` is the authoritative complete record.
- `float` and `log_float` parameter types (FR-010) are included for completeness and future pipeline-specific options; no current Options field requires them.
- `optimization_sessions.search_space_json` is a new additive column; rows written before this feature (without a search space) will have NULL in that column.
- The `FuncSpecMaxGroups` field (pipeline 1 specific, currently without an ENV var) MUST be given a standard ENV var (`CRS_DEFAULT_FUNC_SPEC_MAX_GROUPS`) as part of this infrastructure work; it is not part of the pipeline-specific specs 003/004.
