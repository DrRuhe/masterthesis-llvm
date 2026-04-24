# Research: JIT Pipeline Infrastructure

## 1. Pipeline Interface Design

**Decision**: Function-pointer struct (`PipelineEntry { StringRef Name; Error(*Run)(PipelineRunArgs&); }`) with a shared `PipelineRunArgs` parameter struct.

**Why**: 
- No heap allocation (vs. `std::function`).
- No virtual dispatch or RTTI (constitution §II).
- Each pipeline TU includes only `JITPipeline.h`; no sibling dependencies.
- `llvm::Error` return satisfies LLVM error-handling convention.

**`PipelineRunArgs` fields** (all by reference or value-copy):
```cpp
struct PipelineRunArgs {
    llvm::Module&                     Mod;
    const Options&                    Opts;
    llvm::PassBuilder&                PB;          // pre-constructed with target machine
    llvm::PassInstrumentationCallbacks& PIC;       // pre-populated with trace callbacks
    std::string&                      CurrentGroup;       // written by pipeline for trace labels
    int&                              CurrentFixpointIter; // written by pipeline for trace labels
    bool                              IsLargeModule;
};
```

`PIC` is stack-local per transform call (not shared across modules); it is populated by the outer transform lambda with trace callbacks before dispatch. The pipeline receives it as-is and registers its module pass managers against it normally.

**Alternatives rejected**:
- `std::function<Error(PipelineRunArgs&)>`: heap allocates; no benefit over function pointer since pipelines are stateless.
- Virtual base class `IJITPipeline`: requires RTTI or careful non-RTTI design; over-engineering for 2–3 pipelines.
- Lambdas stored in `ClangRuntimeSpecializer.cpp`: reverts to the monolith; cannot be in separate TUs without std::function.

**Registry**: A separate `JITPipelineRegistry.cpp` declares the static array and defines `getRegisteredPipelines()`. Adding pipeline N requires creating one new TU + editing only `JITPipelineRegistry.cpp` (satisfies FR-008).

---

## 2. Env Var Renaming

**Decision**: Rename three env vars to follow the `CRS_DEFAULT_<FIELDNAME_UPPERCASE>` convention. Keep three that already match. Add one new one.

| Old Name | New Name | Field |
|---|---|---|
| `CRS_DEFAULT_FIXPOINT` | `CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS` | `MaxFixpointIterations` |
| `CRS_DEFAULT_UNROLL` | `CRS_DEFAULT_LOOP_UNROLL_COUNT` | `LoopUnrollCount` |
| `CRS_DEFAULT_LARGE_MOD` | `CRS_DEFAULT_LARGE_MODULE_INSTR_THRESHOLD` | `LargeModuleInstrThreshold` |
| `CRS_DEFAULT_EARLY_PRUNE` | `CRS_DEFAULT_EARLY_PRUNE` | `EnableEarlyPrune` |
| `CRS_DEFAULT_O3_FINAL` | `CRS_DEFAULT_O3_FINAL` | `EnableO3Final` |
| `CRS_DEFAULT_PIPELINE` | `CRS_DEFAULT_PIPELINE` | `OptimizationPipelineToUse` |
| *(none)* | `CRS_DEFAULT_FUNC_SPEC_MAX_GROUPS` | `FuncSpecMaxGroups` |

`EnableEarlyPrune` and `EnableO3Final` drop the "Enable" prefix; `OptimizationPipelineToUse` uses the abbreviated form — both are spec-mandated in the descriptor example.

**Impact**: Breaking change for any script using the old names. Acceptable: no external users; `optimize_benchmarks.py` is updated atomically in this same change.

**Alternatives rejected**: Keeping both old and new names (violates "no legacy support" principle); keeping old names (violates FR-003).

---

## 3. DB Schema State

**Decision**: No schema migration needed. Both `params_json JSON` (in `optim_trial_params`) and `search_space_json JSON` (in `optimization_sessions`) columns already exist from the prior migration commit. Existing rows have NULL in `search_space_json`; this is acceptable per spec assumptions.

**Action required**: The `optimization_sessions` INSERT at lines 564–571 of `optimize_benchmarks.py` must be updated to include `search_space_json` in the column list and pass the serialized descriptor as its value.

---

## 4. `objective()` Generalization Approach

**Decision**: Replace the hardcoded `magic` dict construction and `_make_env()` with two descriptor-driven helpers:

```python
def _sample_params(trial: optuna.Trial, descriptor: dict) -> dict[str, Any]:
    """Returns {name: sampled_value} for each parameter in the descriptor."""

def _params_to_env(params: dict[str, Any], descriptor: dict) -> dict[str, str]:
    """Returns {env_var: str(value)} for subprocess injection."""
```

The `int_or_zero` type requires two Optuna calls (one `suggest_categorical([True, False])` for the zero/non-zero choice, then a conditional `suggest_int`). This mirrors the existing implementation but is now driven by the `min`/`max` fields of the descriptor entry rather than CLI args.

**CLI arg changes**: `--fixpoint-hi`, `--unroll-hi`, `--large-mod-hi` are removed. Their bounds now live in the built-in default descriptor (or are overridden by `--search-space`). `--search-space PATH` is added (optional; defaults to built-in).

**Built-in default descriptor** (Python dict literal in `optimize_benchmarks.py`):
```python
DEFAULT_SEARCH_SPACE = {
    "version": 1,
    "parameters": [
        {"name": "fixpoint_max",    "env_var": "CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS",     "type": "int",        "min": 0,    "max": 30      },
        {"name": "unroll_max",      "env_var": "CRS_DEFAULT_LOOP_UNROLL_COUNT",            "type": "log_int",    "min": 1,    "max": 512     },
        {"name": "large_module_max","env_var": "CRS_DEFAULT_LARGE_MODULE_INSTR_THRESHOLD", "type": "int_or_zero","min": 1,    "max": 100000  },
        {"name": "early_prune",     "env_var": "CRS_DEFAULT_EARLY_PRUNE",                 "type": "bool"                                    },
        {"name": "o3_final",        "env_var": "CRS_DEFAULT_O3_FINAL",                    "type": "bool"                                    },
        {"name": "pipeline",        "env_var": "CRS_DEFAULT_PIPELINE",                    "type": "categorical","choices": [0, 1]            },
    ]
}
```

---

## 5. Pipeline-Incompatible Option Warnings

**Decision**: Add validation in `specializeOnlyImpl()` immediately after `Options` are resolved. Check:

1. `OptimizationPipelineToUse` out of range `[0, N-1]`: WARNING + clamp to 0.
2. `FuncSpecMaxGroups > 0` with pipeline 0 selected: WARNING (field is ignored by pipeline 0).
3. `MaxFixpointIterations` set to a value > 1 with pipeline 1 selected: WARNING (pipeline 1 only runs the first iteration of the fixpoint path — subsequent iterations are effectively no-ops but emit a warning for clarity).

These are the only cross-pipeline incompatibilities at the time of implementation. New incompatibilities should be documented here when new pipelines are added.
