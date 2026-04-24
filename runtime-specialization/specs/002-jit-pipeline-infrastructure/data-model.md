# Data Model: JIT Pipeline Infrastructure

## Entities

### JITPipeline

Represents one compilation strategy. Registered at compile time via `JITPipelineRegistry.cpp`.

| Attribute | Type | Notes |
|---|---|---|
| index | `int` | Non-negative; position in `getRegisteredPipelines()` array |
| name | `llvm::StringRef` | Human-readable label (e.g., `"inlining"`, `"func-spec"`); used in logs and trace records |
| run | `Error(*)(PipelineRunArgs&)` | The pipeline implementation function |

**Invariant**: `getRegisteredPipelines()[i].index == i` always holds (index = position in array).  
**Valid range**: `[0, getRegisteredPipelines().size() - 1]`. Out-of-range values in `OptimizationPipelineToUse` are clamped to 0 after a WARNING.

---

### PipelineRunArgs

Parameter bundle passed from the outer IR transform dispatch to a pipeline implementation. Stack-allocated per transform call; never heap-allocated.

| Field | Type | Ownership | Set by |
|---|---|---|---|
| `Mod` | `llvm::Module&` | JIT (caller) | IR transform lambda |
| `Opts` | `const Options&` | Caller | `ClangRuntimeSpecializer::specializeOnlyImpl()` |
| `PB` | `llvm::PassBuilder&` | Caller | IR transform lambda (pre-constructed) |
| `PIC` | `llvm::PassInstrumentationCallbacks&` | Caller | IR transform lambda (pre-populated with trace callbacks) |
| `CurrentGroup` | `std::string&` | Caller, written by pipeline | Written by pipeline to label trace records per phase |
| `CurrentFixpointIter` | `int&` | Caller, written by pipeline | Written by pipeline; `-1` = not in fixpoint loop |
| `IsLargeModule` | `bool` | Caller | Computed from `Opts.LargeModuleInstrThreshold` before dispatch |

---

### SearchSpaceDescriptor

A JSON document defining which parameters `optimize_benchmarks.py` optimizes for a given study.

| Field | Type | Required | Notes |
|---|---|---|---|
| `version` | integer | yes | Must be `1` for this implementation |
| `parameters` | array of ParameterDefinition | yes | At least one entry |

Stored verbatim in `optimization_sessions.search_space_json` at study start. Rows from before this feature have `NULL` in that column.

---

### ParameterDefinition

One entry in a `SearchSpaceDescriptor.parameters` array.

| Field | Type | Required | Notes |
|---|---|---|---|
| `name` | string | yes | Optuna trial key; used as the key in `params_json` |
| `env_var` | string | yes | ENV var set in benchmark subprocess |
| `type` | string | yes | One of: `int`, `log_int`, `float`, `log_float`, `bool`, `categorical`, `int_or_zero` |
| `min` | number | for `int`, `log_int`, `float`, `log_float`, `int_or_zero` | Lower bound (inclusive) |
| `max` | number | for `int`, `log_int`, `float`, `log_float`, `int_or_zero` | Upper bound (inclusive) |
| `choices` | array | for `categorical` | Explicit list of integer or string values |

**Type → Optuna mapping**:

| Type | Optuna call | Notes |
|---|---|---|
| `int` | `suggest_int(min, max)` | linear |
| `log_int` | `suggest_int(min, max, log=True)` | log-scaled |
| `float` | `suggest_float(min, max)` | linear |
| `log_float` | `suggest_float(min, max, log=True)` | log-scaled |
| `bool` | `suggest_categorical([0, 1])` | — |
| `categorical` | `suggest_categorical(choices)` | — |
| `int_or_zero` | `suggest_categorical([True, False])` → conditional `suggest_int(min, max, log=True)` | zero = "disabled" |

---

## State Transitions: Options Validation

```
OptimizationPipelineToUse value
  ├── in [0, N-1]  →  dispatch to pipeline[value]
  └── out of range →  WARNING "pipeline index N out of range [0, M]; clamping to 0"
                      clamp to 0, dispatch to pipeline[0]

FuncSpecMaxGroups > 0 AND pipeline == 0
  →  WARNING "FuncSpecMaxGroups is ignored by the inlining pipeline"

MaxFixpointIterations > 1 AND pipeline == 1
  →  WARNING "MaxFixpointIterations > 1 has no effect on the func-spec pipeline"
```

Validation runs once in `specializeOnlyImpl()` before the IR transform is invoked.

---

## DB Schema (unchanged — existing columns referenced)

```sql
-- optim_trial_params (existing)
-- params_json JSON NOT NULL  →  {name: value} for all descriptor parameters for this trial

-- optimization_sessions (existing column, now written)
-- search_space_json JSON  →  verbatim descriptor JSON used for this study
```

No DDL changes required.
