# Data Model: JIT Pipeline Evaluation

## Existing Entities (unchanged)

| Entity | Table / View | Purpose |
|--------|-------------|---------|
| Benchmark run context | `context` | Hardware, binary, git SHA per run |
| Raw benchmark rows | `benchmarks` | One row per (run_id, benchmark_name) |
| Parsed + joined view | `v_parsed` | Adds git_sha, run_ts, KV tags |
| Ns-normalized view | `v_ns` | All times in nanoseconds |
| Pivoted ratios | `v_ratios` | Per (run_id, kernel): jit_ns, spec_ns, unspec_ns |
| Optimization trial params | `optim_trial_params` | Maps Optuna trial → run_id + params_json |
| Unspecialized baselines | `unspec_baselines` | Median unspec_ns per (study_name, kernel) |
| Optimization sessions | `optimization_sessions` | Study metadata: binary, n_trials, status |
| Optim results view | `v_optim_results` | Joins trial params with per-kernel timing |
| Break-even view | `v_optim_breakeven` | Computes min call count to recover JIT overhead |
| Per-kernel best view | `v_optim_best_per_kernel` | Best trial per kernel per study |

## New Entities

### Table: `ablation_studies`

Maps named ablation/sensitivity/transfer configs to their DuckDB `run_id` values.

| Column | Type | Notes |
|--------|------|-------|
| `study_name` | VARCHAR (PK) | Logical grouping (e.g., `ablation_polybench_20260510`) |
| `config_name` | VARCHAR (PK) | Human-readable config (e.g., `o3_only`, `workload_optimal`) |
| `rep` | INTEGER (PK) | 0-indexed repetition (0, 1, 2 for 3 reps) |
| `params_json` | JSON | Exact env var overrides applied for this run |
| `run_id` | VARCHAR (FK → context) | Links to the raw benchmark data |

### View: `v_ablation_results`

Joins `ablation_studies` with per-kernel timing from `v_ratios`.

| Column | Source | Description |
|--------|--------|-------------|
| `study_name` | ablation_studies | Logical study |
| `config_name` | ablation_studies | Named config |
| `rep` | ablation_studies | Repetition index |
| `params_json` | ablation_studies | Config params |
| `kernel` | v_ratios | Benchmark kernel name |
| `group` | v_ratios | Benchmark group (polybench / tpch) |
| `t_jit_ns` | v_ratios | JIT overhead in nanoseconds |
| `t_spec_ns` | v_ratios | Specialized execution in nanoseconds |
| `t_unspec_ns` | v_ratios | Unspecialized execution in nanoseconds |

### View: `v_ablation_medians`

Aggregates `v_ablation_results` to median per (study, config, kernel).

| Column | Type | Description |
|--------|------|-------------|
| `study_name` | VARCHAR | |
| `config_name` | VARCHAR | |
| `kernel` | VARCHAR | |
| `group` | VARCHAR | |
| `med_jit_ns` | DOUBLE | Median JIT overhead across reps |
| `med_spec_ns` | DOUBLE | Median specialized exec across reps |
| `med_unspec_ns` | DOUBLE | Median unspecialized exec across reps |
| `n_reps` | BIGINT | Number of repetitions contributing |

## Entity Relationships

```
context ──── benchmarks          (1:many via run_id)
   │
   └──── optim_trial_params      (1:1 via run_id, optional)
   │
   └──── ablation_studies        (1:1 via run_id)
              │
              └──► v_ablation_results  (via v_ratios JOIN)
                        │
                        └──► v_ablation_medians  (aggregated)
```

## Workload Taxonomy

| Group | Kernel examples | Sizes used | Benchmark phases |
|-------|----------------|-----------|-----------------|
| `polybench` | correlation, gemm, lu, floyd_warshall, … (30 total) | MINI (smoke), EXTRALARGE (full) | jit_overhead, specialized_exec, unspecialized |
| `tpch` | tpch_q1, tpch_q6, tpch_q3 | (single size — SF1) | jit_overhead, specialized_exec, unspecialized |

## Ablation Config Registry

The 8+1 named configs (defined as built-in constants in `ablation_benchmarks.py`):

```json
[
  {"name": "default",       "env": {}},
  {"name": "o3_only",       "env": {"CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS": "0",
                                     "CRS_DEFAULT_EARLY_PRUNE": "0",
                                     "CRS_DEFAULT_O3_FINAL": "1"}},
  {"name": "no_prune",      "env": {"CRS_DEFAULT_EARLY_PRUNE": "0"}},
  {"name": "no_o3_final",   "env": {"CRS_DEFAULT_O3_FINAL": "0"}},
  {"name": "no_unroll",     "env": {"CRS_DEFAULT_LOOP_UNROLL_COUNT": "1"}},
  {"name": "fixpoint_1",    "env": {"CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS": "1"}},
  {"name": "pipeline_1",    "env": {"CRS_DEFAULT_PIPELINE": "1",
                                     "CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS": "1"}},
  {"name": "aggressive",    "env": {"CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS": "20",
                                     "CRS_DEFAULT_LOOP_UNROLL_COUNT": "256"}},
  {"name": "workload_optimal", "env": "<extracted from v_optim_best_per_kernel after Exp A>"}
]
```

## Sensitivity Sweep Parameters

The 6 parameters for the OAT sensitivity analysis:

| Param | Env var | Range | Grid points |
|-------|---------|-------|-------------|
| `fixpoint_max` | `CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS` | 0–30 | {0, 1, 2, 3, 5, 8, 15, 30} |
| `unroll_max` | `CRS_DEFAULT_LOOP_UNROLL_COUNT` | 1–512 | {1, 2, 4, 8, 16, 32, 64, 128, 256, 512} |
| `large_module_max` | `CRS_DEFAULT_LARGE_MODULE_INSTR_THRESHOLD` | 0–100000 | {0, 1000, 5000, 10000, 50000, 100000} |
| `early_prune` | `CRS_DEFAULT_EARLY_PRUNE` | 0 or 1 | {0, 1} |
| `o3_final` | `CRS_DEFAULT_O3_FINAL` | 0 or 1 | {0, 1} |
| `pipeline` | `CRS_DEFAULT_PIPELINE` | 0 or 1 | {0, 1} |

For each parameter sweep: all other parameters held at workload-optimal values from Exp A.
Total sweep evaluations: 8+10+6+2+2+2 = **30 polybench runs** (each with 1 rep).
