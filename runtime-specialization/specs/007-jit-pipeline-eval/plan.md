# Implementation Plan: JIT Pipeline Evaluation — Iteration Loop

**Branch**: `007-jit-pipeline-eval` | **Date**: 2026-05-17 | **Spec**: [spec.md](spec.md)

## Summary

Design and implement the infrastructure to run one complete iteration of the JIT pipeline
evaluation loop: optimize (Exp A) → ablate (Exp B) → transfer (Exp C) → break-even (Exp D)
→ sensitivity (Exp E) → reflect (Exp F). `optimize_benchmarks.py` is the engine — it runs
150 Optuna TPE trials on UC benchmarks (MEDIUM size) to find the best pipeline config.
Supporting experiments interpret and validate that result. A reflection document closes the
loop by capturing infrastructure and process improvements for the next iteration.

Data flows into `benchmarks.duckdb`. Reporting scripts produce thesis figures. A smoke test
validates all data paths before committing compute to full runs.

---

## Technical Context

**Language/Version**: Python 3.12 (Nix flake), C++ (LLVM/Clang)  
**Primary Dependencies**: DuckDB, Optuna, scipy (stats), Google Benchmark, LLVM/LLJIT  
**Storage**: `benchmarks/benchmarks.duckdb` (existing) + `eval_smoke.duckdb` (smoke test)  
**Testing**: smoke-test run verifying DB writes + query correctness  
**Target Platform**: Linux x86-64 (single controlled machine, CPU scaling disabled)  
**Project Type**: Experimental methodology + data-collection scripts + reporting  
**Performance Goals**: Exp A: ≥ 150 Optuna trials on UC MEDIUM; smoke run < 5 min  
**Constraints**: Release builds only for timing data; no debug builds for performance claims  
**Scale/Scope**: 6 UC benchmark groups × MEDIUM size; 3 TPC-H queries (transfer only); 8+1 ablation configs; 6 sensitivity params

---

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-checked after Phase 1 design.*

| Principle | Status | Notes |
|-----------|--------|-------|
| I. Correctness & Safety | ✅ Pass | No new JIT code paths; this is measurement infrastructure |
| II. LLVM Coding Standards | ✅ Pass | No C++ changes; new work is Python scripts |
| III. Test-First Validation | ✅ Pass | Smoke-test run validates infrastructure before full runs |
| IV. Performance Measurement | ✅ Pass | This plan *is* the performance measurement infrastructure |
| V. Minimal Public API | ✅ Pass | No changes to the public C++ API |

**No violations.** Planning may proceed.

---

## Project Structure

### Documentation (this feature)

```text
specs/007-jit-pipeline-eval/
├── plan.md              # This file
├── research.md          # Phase 0 output (complete)
├── data-model.md        # Phase 1 output
└── tasks.md             # Phase 2 output (/speckit-tasks)
```

### Source Code

```text
benchmarks/
├── optimize_benchmarks.py        # Existing — Exp A (engine of the loop)
├── ablation_benchmarks.py        # NEW — Exp B + C (named configs, N reps)
├── sensitivity_analysis.py       # NEW — Exp E (OAT parameter sweep)
├── run_evaluation.sh             # NEW — orchestration: smoke + full runs
└── reporting/
    ├── plot_pareto.py             # NEW — Pareto frontier plots (Exp A)
    ├── plot_ablation.py           # NEW — per-config speedup bar charts (Exp B)
    ├── plot_breakeven.py          # NEW — break-even distribution plots (Exp D)
    └── plot_sensitivity.py        # NEW — OAT parameter sensitivity plots (Exp E)
```

---

## Phase 0: Research

All research questions are resolved. See [research.md](research.md) for full decisions.

### Summary of Key Decisions

| RQ | Decision |
|----|----------|
| RQ-1 | Coarse-grained ablation via 6 existing `CRS_DEFAULT_*` env vars; no C++ changes needed |
| RQ-2 | `scipy.stats.wilcoxon` — verify availability before implementing Exp C |
| RQ-3 | Filter: `BM_g:(uc1\|uc2\|uc7\|uc8\|uc12\|uc14);.*s:MEDIUM;.*t:(jit_overhead\|specialized_exec)` |
| RQ-4 | 6 UC groups × 3 phases = 18 rows per trial (uc1_sql, uc2_conv, uc7_dfa, uc8_ivm, uc12_groupby, uc14_sort) |
| RQ-5 | TPC-H (q1, q6, q3) used in transfer experiment only; not optimized |
| RQ-6 | `uc_workload_optimal` extracted from `v_optim_best_per_kernel` after Exp A; added via `--extra-config` |
| RQ-7 | Optuna importance extracted in same session as `study.optimize()` → `importance_<study>.json` |
| RQ-8 | 3 separate binary runs per config (not `--benchmark_repetitions=3`) → 3 independent `run_id`s |
| RQ-9 | MEDIUM size is representative; relative speedup stable across MEDIUM/LARGE/EXTRALARGE (~5× faster than EXTRALARGE) |

---

## Phase 1: Design

### Iteration Loop Design

The evaluation runs as a single ordered loop of 6 experiments. `optimize_benchmarks.py`
(Exp A) is the primary source of evidence; experiments B–E interpret and validate it;
Exp F (Reflection) produces the feedback for future iterations.

```
build release UC binary
       │
       ▼
[Smoke test] ──── eval_smoke.duckdb ──── verify pass criteria
       │
       ▼  (only if smoke passes)
       │
       ├──► [Exp A] optimize_benchmarks.py ─────────────────────────┐
       │         UC MEDIUM, 150 trials, seed=42                      │
       │         → uc_best_config.json                               │
       │         → importance_<study>.json                           │
       │                                                              ▼
       ├──► [Exp B] ablation_benchmarks.py              benchmarks.duckdb
       │         UC MEDIUM, 9 configs × 3 reps               │
       │                                                      │  v_optim_best_per_kernel
       ├──► [Exp C] ablation_benchmarks.py                    │  v_optim_breakeven
       │         UC MEDIUM + TPC-H, 4 configs × 3 reps       │  v_ablation_medians
       │                                                      │  v_ablation_results
       ├──► [Exp D] (no new runs)                             │
       │         query v_optim_breakeven from Exp A           │
       │                                                      │
       ├──► [Exp E] sensitivity_analysis.py ──────────────────┘
       │         UC MEDIUM, 6 params × sweep pts
       │
       └──► [Exp F] write reflection_iteration1.md
                 infrastructure + process + next-iteration scope
                        │
                        ▼
              reporting/ scripts ──► thesis figures
```

### Data Model

The existing `benchmarks.duckdb` schema supports Experiments A and D without changes.
Experiments B, C, and E need one new table. See [data-model.md](data-model.md) for SQL.

**New table**: `ablation_studies` (study_name, config_name, rep, params_json, run_id)  
**New views**: `v_ablation_results`, `v_ablation_medians`

Workload taxonomy (updated):

| Group | Kernel examples | Sizes used | Benchmark phases |
|-------|----------------|-----------|-----------------|
| `uc*` | uc1_sql, uc2_conv, uc7_dfa, uc8_ivm, uc12_groupby, uc14_sort | MINI (smoke), MEDIUM (full) | jit_overhead, specialized_exec, unspecialized |
| `tpch` | tpch_q1, tpch_q6, tpch_q3 | (single size — SF1) | jit_overhead, specialized_exec, unspecialized |

### Experiment-to-Run Schedule

| Exp | Script | Workload | Filter | n_trials/reps | Study name pattern | Tables written | Analysis query |
|-----|--------|----------|--------|---------------|-------------------|----------------|----------------|
| Smoke-A | `optimize_benchmarks.py` | UC MINI | uc7 + uc8 groups | 10, seed=0 | `smoke_uc_A` | `optim_trial_params`, `benchmarks` | `v_optim_best_per_kernel` |
| Smoke-B | `ablation_benchmarks.py` | UC MINI | uc7 + uc8 groups | 3 reps × 3 configs | `smoke_ablation` | `ablation_studies`, `benchmarks` | `v_ablation_medians` |
| Smoke-C | `ablation_benchmarks.py` | tpch q1 | q1 only | 3 reps × 1 config | `smoke_transfer` | `ablation_studies`, `benchmarks` | `v_ablation_results` |
| Smoke-E | `sensitivity_analysis.py` | UC MINI | uc7 + uc8 groups | 3 pts × 1 param | `smoke_sens` | `ablation_studies`, `benchmarks` | `v_ablation_medians` |
| **A-uc** | `optimize_benchmarks.py` | UC MEDIUM | all 6 groups | **150, seed=42** | `uc_optim_YYYYMMDD` | `optim_trial_params`, `benchmarks` | `v_optim_best_per_kernel`, `v_optim_breakeven` |
| **B-uc** | `ablation_benchmarks.py` | UC MEDIUM | all 6 groups | **3 reps × 9 configs** | `ablation_uc_YYYYMMDD` | `ablation_studies`, `benchmarks` | `v_ablation_medians` |
| **C** | `ablation_benchmarks.py` | UC MEDIUM + tpch | all UC + q1+q6+q3 | **3 reps × 4 configs each** | `transfer_YYYYMMDD` | `ablation_studies`, `benchmarks` | `v_ablation_medians` + Wilcoxon test |
| **D** | *(no new runs)* | — | — | — | — | — | `v_optim_breakeven WHERE study_name='uc_optim_*'` |
| **E** | `sensitivity_analysis.py` | UC MEDIUM | all 6 groups | **1 rep × 48 pts** | `sens_uc_YYYYMMDD` | `ablation_studies`, `benchmarks` | `v_ablation_medians` per param |
| **F** | *(document only)* | — | — | — | — | — | `benchmarks/results/reflection_iteration1.md` |

### Smoke Test Design

Validates all data paths before committing compute to full experiments.

1. **DB init**: create `eval_smoke.duckdb` with new schema
2. **Smoke-A**: UC MINI, 10 trials → verify `v_optim_best_per_kernel` returns 2 rows; no timeout fallbacks
3. **Smoke-B**: UC MINI, 3 configs × 3 reps → verify `v_ablation_medians` returns 6 rows; no NULL run_ids
4. **Smoke-C**: UC-optimal → TPC-H q1, 1 rep → verify row in `benchmarks` with `group='tpch'`
5. **Smoke-D**: query `v_optim_breakeven WHERE study_name='smoke_uc_A'` → verify non-null rows
6. **Smoke-E**: fixpoint_max ∈ {0,5,10} on uc7+uc8 MINI → verify 3 rows, OAT delta non-zero

**Pass criteria**: all 6 verifications pass; no `used_timeout_fallback=TRUE`; no orphaned rows.

### Script Interfaces / Contracts

#### `ablation_benchmarks.py`

```text
Usage: ablation_benchmarks.py BINARY [options]

  --db PATH                 DuckDB path (default: CWD/benchmarks.duckdb)
  --study-name STR          Study label (default: ablation_YYYYMMDD_HHMMSS)
  --configs PATH            JSON file listing named configs (default: built-in 8)
  --extra-config JSON_STR   Append one additional config object at runtime
  --benchmark-filter PATT   Forwarded to benchmark binary
  --reps N                  Repetitions per config (default: 3)
  --timeout SEC             Per-run timeout in seconds (default: 120)

Built-in configs (8): default, o3_only, no_prune, no_o3_final, no_unroll,
                      fixpoint_1, pipeline_1, aggressive
Dynamic 9th config: uc_workload_optimal — added via --extra-config at runtime
                    after Exp A completes
```

#### `sensitivity_analysis.py`

```text
Usage: sensitivity_analysis.py BINARY [options]

  --db PATH                 DuckDB path
  --study-name STR          Study label
  --optimal-config PATH     JSON with optimal params (from Exp A)
  --param NAME              Parameter to sweep
  --sweep-values V1,V2,...  Comma-separated values (overrides auto-grid)
  --benchmark-filter PATT   Forwarded to binary
  --reps N                  Reps per sweep point (default: 1)

Config naming pattern: sens_{param}_{value} stored in ablation_studies.config_name
Delegates to same run_trial + store_to_db functions as ablation_benchmarks.py
```

#### `run_evaluation.sh`

```text
Usage: ./run_evaluation.sh <uc_binary> <tpch_binary> [--smoke-only] [--db PATH]

Phases:
  0. smoke      — runs smoke test; exits non-zero on failure
  A. optim      — optimize_benchmarks.py on UC MEDIUM (150 trials)
  B. ablation   — ablation_benchmarks.py on UC MEDIUM (9 configs × 3 reps)
  C. transfer   — ablation_benchmarks.py with UC-optimal → TPC-H
  E. sensitivity — sensitivity_analysis.py on UC MEDIUM (6 params)
  F. (manual)   — researcher writes reflection_iteration1.md
```

### Reporting Pipeline

Both axes of every Pareto plot are **minimized** (lower-left = best). The Pareto frontier
is the lower-left envelope of the (jit_ns, spec_ns) cloud, drawn as a polyline connecting
non-dominated points sorted by ascending JIT overhead.

| Figure | Script | Query source |
|--------|--------|-------------|
| Pareto front (jit vs exec) per UC group, **Optuna trials** | `plot_pareto.py` | `v_optim_breakeven WHERE study_name='uc_optim_*'` |
| Pareto front (jit vs exec) per study, **named configs + CSV** | `plot_pareto_configs.py` | `optim_trial_params` ∪ `ablation_studies` joined with `v_ratios` |
| Per-group speedup vs Default (bar chart) | `plot_ablation.py` | `v_ablation_medians WHERE study_name='ablation_uc_*'` |
| Break-even distribution (box plot) | `plot_breakeven.py` | `v_optim_breakeven` for UC optimal study |
| Parameter sensitivity (OAT line chart) | `plot_sensitivity.py` | `v_ablation_medians WHERE study_name='sens_uc_*'` |
| Cross-workload degradation table | inline in `run_evaluation.sh` | `v_ablation_medians` + `scipy.stats.wilcoxon` |

`plot_pareto_configs.py` is the thesis-ready variant: it produces both a PNG and a CSV per
study (or per group with `--per-group`). The CSV columns are: typed parameter columns
(`fixpoint_max`, `unroll_max`, `large_module_max`, `early_prune`, `o3_final`), `group`,
`kernel`, `jit_overhead_ms`, `specialized_exec_ms`, `is_pareto_optimal` (bool),
`is_default` (bool). The CSV is designed to be loaded directly by pgfplots without
post-processing.

### Ablation Config Registry

Built-in configs in `ablation_benchmarks.py` (9 total including dynamic `uc_workload_optimal`):

```json
[
  {"name": "default",            "env": {}},
  {"name": "o3_only",            "env": {"CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS": "0",
                                          "CRS_DEFAULT_EARLY_PRUNE": "0",
                                          "CRS_DEFAULT_O3_FINAL": "1"}},
  {"name": "no_prune",           "env": {"CRS_DEFAULT_EARLY_PRUNE": "0"}},
  {"name": "no_o3_final",        "env": {"CRS_DEFAULT_O3_FINAL": "0"}},
  {"name": "no_unroll",          "env": {"CRS_DEFAULT_LOOP_UNROLL_COUNT": "1"}},
  {"name": "fixpoint_1",         "env": {"CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS": "1"}},
  {"name": "pipeline_1",         "env": {"CRS_DEFAULT_PIPELINE": "1",
                                          "CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS": "1"}},
  {"name": "aggressive",         "env": {"CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS": "20",
                                          "CRS_DEFAULT_LOOP_UNROLL_COUNT": "256"}},
  {"name": "uc_workload_optimal","env": "<extracted from v_optim_best_per_kernel after Exp A>"}
]
```

### Reflection Document Structure

The reflection at `benchmarks/results/reflection_iteration1.md` MUST cover:

```
## Reflection: Iteration 1

### 1. Infrastructure Improvements
- optimize_benchmarks.py: [specific actionable changes]
- Benchmark binaries / UC benchmark coverage: [specific gaps]
- DuckDB schema / tooling: [specific improvements]

### 2. Evaluation Process Improvements
- Trial budget: was 150 sufficient? (cite convergence plot)
- Workload selection: did UC groups represent diverse enough specialization patterns?
- Size choice: was MEDIUM representative? (cite per-group variance)
- Statistical approach: were 3 reps enough? where was variance high?

### 3. Next Iteration Scope
- Parameters to prune: [cite sensitivity result]
- Parameters needing finer resolution: [cite sensitivity result]
- Experiments to skip/repeat/add: [justify with data]
- Proposed trial budget for next run: [justify]
```

### Key Engineering Decisions

1. **`optimize_benchmarks.py` is the loop's engine**. It runs unchanged on UC benchmarks;
   supporting scripts (ablation, sensitivity) interpret its output rather than replacing it.

2. **UC benchmarks only for the optimizer**. TPC-H is used in transfer (Exp C) only,
   because UC benchmarks most directly reflect the intended runtime specialization use case.

3. **MEDIUM size for all UC runs**. Size-scaling data confirms relative speedup is stable
   across MEDIUM/LARGE/EXTRALARGE. MINI is used for smoke tests only.

4. **Reflection is a first-class experiment**. It is listed as Exp F in the schedule,
   has its own acceptance criteria, and its output (the reflection document) is the primary
   input to the next iteration. Without it the loop has no feedback mechanism.

5. **`ablation_benchmarks.py` reuses the existing schema**. The only new DB object is the
   `ablation_studies` table. All other joins go through existing `v_ratios` and `v_optim_*` views.

6. **Sensitivity analysis delegates to `ablation_benchmarks.py`**. OAT sweep points are
   modeled as named ablation configs (`sens_{param}_{value}`). No separate data store.

7. **Optuna importance must be extracted in the same Python session as `study.optimize()`**.
   The current `optimize_benchmarks.py` does not persist the Optuna study object. Importance
   extraction must be added as an output step before the process exits.

---

## Complexity Tracking

No constitution violations. No complexity justification required.
This is measurement infrastructure; it adds no C++ code to the core JIT path.
