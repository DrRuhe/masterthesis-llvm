# Contract: Search Space Descriptor JSON Format

This document is the authoritative definition of the JSON file accepted by `optimize_benchmarks.py --search-space PATH`.

## Top-level object

```json
{
  "version": 1,
  "parameters": [ <ParameterDefinition>, ... ]
}
```

| Field | Type | Required | Constraint |
|---|---|---|---|
| `version` | integer | yes | Must be `1` |
| `parameters` | array | yes | Non-empty; each element is a ParameterDefinition |

## ParameterDefinition

```json
{
  "name":    "<optuna_key>",
  "env_var": "CRS_DEFAULT_<FIELD>",
  "type":    "<type_string>",
  <type-specific fields>
}
```

| Field | Type | Required | Notes |
|---|---|---|---|
| `name` | string | yes | Key in `params_json` column; Optuna trial parameter name |
| `env_var` | string | yes | ENV var injected into benchmark subprocess |
| `type` | string | yes | See type table below |

## Type-specific fields

| `type` | Extra fields | Constraint |
|---|---|---|
| `int` | `min` (integer), `max` (integer) | `min <= max` |
| `log_int` | `min` (integer ≥ 1), `max` (integer) | `1 <= min <= max` |
| `float` | `min` (number), `max` (number) | `min <= max` |
| `log_float` | `min` (number > 0), `max` (number) | `0 < min <= max` |
| `bool` | — | samples `0` or `1` |
| `categorical` | `choices` (non-empty array of integers or strings) | — |
| `int_or_zero` | `min` (integer ≥ 1), `max` (integer) | `1 <= min <= max`; value `0` means "disabled" |

## Canonical built-in default (as of spec 002)

```json
{
  "version": 1,
  "parameters": [
    { "name": "fixpoint_max",     "env_var": "CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS",     "type": "int",        "min": 0,    "max": 30      },
    { "name": "unroll_max",       "env_var": "CRS_DEFAULT_LOOP_UNROLL_COUNT",            "type": "log_int",    "min": 1,    "max": 512     },
    { "name": "large_module_max", "env_var": "CRS_DEFAULT_LARGE_MODULE_INSTR_THRESHOLD", "type": "int_or_zero","min": 1,    "max": 100000  },
    { "name": "early_prune",      "env_var": "CRS_DEFAULT_EARLY_PRUNE",                 "type": "bool"                                    },
    { "name": "o3_final",         "env_var": "CRS_DEFAULT_O3_FINAL",                    "type": "bool"                                    },
    { "name": "pipeline",         "env_var": "CRS_DEFAULT_PIPELINE",                    "type": "categorical","choices": [0, 1]            }
  ]
}
```

## Validation rules

1. `version` must equal `1`; unknown versions are rejected with an error message.
2. Each `name` must be unique within the descriptor.
3. Each `env_var` must be unique within the descriptor.
4. Type-specific range/choices constraints (table above) must hold; violations produce an error before the study starts.
5. Unknown top-level or parameter-level keys are ignored (forward compatibility).

## Versioning policy

The `version` field gates breaking changes to the format (e.g., new mandatory fields, renamed types). Additive changes (new optional fields, new type strings that fall back gracefully) do not require a version bump.
