# Data Model: Benchmark Analysis & Reporting System

**Branch**: `001-benchmark-analysis-reporting` | **Date**: 2026-04-24

## Entities (DuckDB tables)

### `context`
One row per benchmark run (per binary invocation).

| Column | Type | Description |
|--------|------|-------------|
| run_id | VARCHAR PK | UUID |
| run_ts | TIMESTAMP | Wall-clock time of recording |
| git_sha | VARCHAR | HEAD SHA at recording time |
| date | VARCHAR | Date string from benchmark JSON |
| host_name | VARCHAR | Hostname |
| executable | VARCHAR | Path to the benchmark binary |
| num_cpus | INTEGER | CPU count from benchmark context |
| mhz_per_cpu | INTEGER | CPU frequency |
| cpu_scaling_enabled | BOOLEAN | Whether CPU scaling was active |
| library_version | VARCHAR | CRS library version string |
| library_build_type | VARCHAR | debug / release |

### `benchmarks`
One row per benchmark result. Base columns are fixed; dynamic columns are added on demand via `ALTER TABLE … ADD COLUMN`.

| Column | Type | Description |
|--------|------|-------------|
| run_id | VARCHAR FK | References context.run_id |
| name | VARCHAR | Full benchmark name |
| family_index | INTEGER | |
| per_family_instance_index | INTEGER | |
| run_type | VARCHAR | "iteration" or "aggregate" |
| repetitions | INTEGER | |
| repetition_index | INTEGER | |
| threads | INTEGER | |
| iterations | BIGINT | |
| real_time | DOUBLE | Raw time in `time_unit` |
| cpu_time | DOUBLE | CPU time in `time_unit` |
| time_unit | VARCHAR | ns / us / ms / s |
| kv_g | VARCHAR | Parsed KV: group |
| kv_n | VARCHAR | Parsed KV: kernel name |
| kv_t | VARCHAR | Parsed KV: phase |
| kv_raw_params | VARCHAR | Unparsed suffix after KV prefix |
| *(dynamic)* | DOUBLE/BIGINT/VARCHAR | Hardware counters, JIT stats, etc. |

### `pass_traces`
One row per compiler pass per `benchmarkJITAnalysis` invocation.

| Column | Type | Description |
|--------|------|-------------|
| run_id | VARCHAR FK | References context.run_id |
| benchmark_name | VARCHAR | Name of the analysis benchmark |
| pass_idx | INTEGER | Sequential pass index |
| pass_name | VARCHAR | LLVM pass class name |
| pass_group | VARCHAR | prune / initial / fixpoint / postfix / final |
| fixpoint_iter | INTEGER | Iteration counter (-1 for non-fixpoint) |
| fns_before | BIGINT | Function count before pass |
| fns_after | BIGINT | Function count after pass |
| instrs_before | BIGINT | Instruction count before pass |
| instrs_after | BIGINT | Instruction count after pass |
| bbs_before | BIGINT | Basic-block count before pass |
| bbs_after | BIGINT | Basic-block count after pass |
| wall_time_ms | DOUBLE | Pass wall time in ms |
| ir_changed | BOOLEAN | Whether pass modified the IR |

### `optimization_sessions` *(to be added — FR-027)*
One row per `optimize_benchmarks.py` invocation. Written at start as `incomplete`; updated to `complete` on normal exit.

| Column | Type | Description |
|--------|------|-------------|
| study_name | VARCHAR PK | Optuna study name |
| binary | VARCHAR | Path to benchmark binary |
| n_trials | INTEGER | Requested trial budget |
| started_at | TIMESTAMP | Start timestamp |
| completed_at | TIMESTAMP | Completion timestamp (NULL until complete) |
| status | VARCHAR | 'incomplete' or 'complete' |
| search_space_json | VARCHAR | Serialized search space descriptor JSON; NULL for pre-spec-002 rows |

### `optim_trial_params`
One row per Optuna trial.

| Column | Type | Description |
|--------|------|-------------|
| study_name | VARCHAR FK | References optimization_sessions.study_name |
| trial_id | INTEGER | Optuna trial number |
| run_id | VARCHAR FK (nullable) | References context.run_id |
| params_json | VARCHAR | Complete parameter set as JSON object keyed by parameter name |
| used_timeout_fallback | BOOLEAN | True if any benchmark timed out |
| obj_jit_ns | DOUBLE | Geomean JIT overhead across kernels |
| obj_exec_ns | DOUBLE | Geomean exec time across kernels |
| obj_combined_ns | DOUBLE | Geomean (jit + exec) across kernels |

### `unspec_baselines`
Per-kernel unspecialized median for a study (measured before optimization trials).

| Column | Type | Description |
|--------|------|-------------|
| study_name | VARCHAR FK | References optimization_sessions.study_name |
| kernel | VARCHAR | Kernel name |
| unspec_ns | DOUBLE | Median unspecialized execution time in ns |

## Views

| View | Purpose |
|------|---------|
| `v_parsed` | benchmarks + context join with readable aliases (phase, kernel, group, raw_params) |
| `v_ns` | All times converted to nanoseconds; rows without a phase filtered |
| `v_ratios` | (run_id, kernel, raw_params, group) → (t_unspec_ns, t_spec_ns, t_jit_ns) pivot |
| `v_jit_stats` | JIT stats columns for jit_overhead iteration rows |
| `v_budget_sweep` | Pareto sweep rows (phase IN jit_budget_sweep / exec_budget_sweep) |
| `v_optim_results` | Per-kernel (jit, exec, unspec) per trial; JOIN optim_trial_params ↔ v_ratios ↔ unspec_baselines |
| `v_optim_breakeven` | v_optim_results + break_even_calls = jit_ns / (unspec_ns - spec_ns) |
| `v_optim_best_per_kernel` | Best trial per (study_name, kernel); **filters `status = 'complete'`** via JOIN on optimization_sessions |

## State transitions

`optimization_sessions.status`:
```
[study.optimize() starts] → INSERT status='incomplete'
[study.optimize() returns normally] → UPDATE status='complete', completed_at=NOW()
[SIGINT / crash] → row remains status='incomplete'
```
