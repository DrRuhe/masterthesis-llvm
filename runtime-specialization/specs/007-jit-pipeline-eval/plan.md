# Implementation Plan: JIT Pipeline Evaluation Methodology

**Branch**: `007-jit-pipeline-eval` | **Date**: 2026-05-10 | **Spec**: [spec.md](spec.md)

## Summary

Design and implement the infrastructure to run five benchmark experiments (A–E) that
together answer "what is the best possible JIT pipeline for a given workload?" Each
experiment maps to specific benchmark runs with defined env-var configs, data flows
into `benchmarks.duckdb`, and reporting scripts produce the thesis figures.

The core optimization tool (`optimize_benchmarks.py`) covers Experiment A.
Experiments B–E require new scripts or query layers on top of the existing schema.
A small-scale smoke test is planned first to verify the full pipeline before committing
compute to 150-trial full runs.

---

## Technical Context

**Language/Version**: Python 3.12 (Nix flake), C++ (LLVM/Clang)  
**Primary Dependencies**: DuckDB, Optuna, scipy (stats), Google Benchmark, LLVM/LLJIT  
**Storage**: `benchmarks/benchmarks.duckdb` (existing) + `eval_smoke.duckdb` (smoke test)  
**Testing**: smoke-test run verifying DB writes + query correctness  
**Target Platform**: Linux x86-64 (single controlled machine, CPU scaling disabled)  
**Project Type**: Experimental methodology + data-collection scripts + reporting  
**Performance Goals**: Experiment A: ≥150 trials per workload; Experiment B: 3 reps per config  
**Constraints**: All measurement runs in release build; no debug builds for timing data  
**Scale/Scope**: ~30 polybench kernels × 5 sizes; 3 TPC-H queries; 8 ablation configs; 6 sensitivity params

---

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-checked after Phase 1 design.*

| Principle | Status | Notes |
|-----------|--------|-------|
| I. Correctness & Safety | ✅ Pass | No new JIT code paths; this is measurement infrastructure |
| II. LLVM Coding Standards | ✅ Pass | No C++ changes in the core path; new work is Python scripts |
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
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
└── tasks.md             # Phase 2 output (/speckit.tasks)
```

### Source Code

```text
benchmarks/
├── optimize_benchmarks.py        # Existing — covers Experiment A unchanged
├── ablation_benchmarks.py        # NEW — Experiment B + C (named configs, N reps)
├── sensitivity_analysis.py       # NEW — Experiment E (OAT parameter sweep)
├── run_evaluation.sh             # NEW — orchestration: smoke test + full runs
└── reporting/
    ├── plot_pareto.py             # NEW — Pareto frontier plots (Exp A)
    ├── plot_ablation.py           # NEW — per-config speedup bar charts (Exp B)
    ├── plot_breakeven.py          # NEW — break-even distribution plots (Exp D)
    └── plot_sensitivity.py        # NEW — OAT parameter sensitivity plots (Exp E)
```

---

## Phase 0: Research

### What Existing Infrastructure Covers

| Experiment | Coverage by existing tools |
|------------|---------------------------|
| A — Optimization | ✅ `optimize_benchmarks.py` with `--n-trials=150 --seed=42`. No changes needed. |
| B — Ablation | ⚠️ `record_benchmark.py` can record single runs but has no config-iteration or study-label support. Needs new `ablation_benchmarks.py`. |
| C — Transfer | ⚠️ Can run binary with fixed env vars, but needs a wrapper to iterate over 4 configs × 2 workloads. Shares `ablation_benchmarks.py` design. |
| D — Break-even | ✅ `v_optim_breakeven` DuckDB view computes this from Experiment A data. No new runs needed. |
| E — Sensitivity | ⚠️ Optuna importance is extractable from Experiment A study (already in Optuna storage). OAT sweeps need a new script. |

### Gap Analysis: Per-Pass Ablation (Experiment B)

The current `Options` struct exposes only coarse knobs via env vars. There are no
individual enable/disable flags for IPSCCP, DevirtVtable, or GVN within the fixpoint loop.

**Decision**: Work with existing knobs. The 8 ablation configs below are fully expressible
without C++ changes and answer the core ablation question ("which pipeline stages contribute
most?") at the granularity the existing architecture exposes. Fine-grained pass-level
ablation would require new `Options` fields — deferred as out-of-scope for the thesis.

**Ablation Config Definitions** (8 named configs, all expressible via existing env vars):

| Config name | `CRS_DEFAULT_*` env overrides | What is disabled |
|-------------|-------------------------------|-----------------|
| `default` | (none — all at compiled defaults) | Baseline |
| `o3_only` | `MAX_FIXPOINT_ITERATIONS=0`, `EARLY_PRUNE=0`, `O3_FINAL=1` | Entire fixpoint loop + early prune |
| `no_prune` | `EARLY_PRUNE=0` | Early GlobalDCE before fixpoint |
| `no_o3_final` | `O3_FINAL=0` | Final O3 pass after fixpoint |
| `no_unroll` | `LOOP_UNROLL_COUNT=1` | Loop unrolling (effectively disabled) |
| `fixpoint_1` | `MAX_FIXPOINT_ITERATIONS=1` | Fixpoint convergence (single pass only) |
| `pipeline_1` | `PIPELINE=1`, `MAX_FIXPOINT_ITERATIONS=1` | Switch to function-spec cloning pipeline |
| `aggressive` | `MAX_FIXPOINT_ITERATIONS=20`, `LOOP_UNROLL_COUNT=256` | Upper bound (more aggressive) |
| `workload_optimal` | (read from `v_optim_best_per_kernel` after Exp A completes) | Dynamic: best config found by Optuna |

Note: `workload_optimal` is added to the ablation set only after Experiment A completes.

### Gap Analysis: Statistical Tests (Experiment C)

Wilcoxon rank-sum requires `scipy.stats.wilcoxon`. Check `flake.nix` for scipy availability
before finalizing Experiment C implementation.

### Smoke Test Design

The smoke test is a minimal end-to-end run that validates all data paths before committing
compute to full experiments. It uses a tiny benchmark filter and few trials.

**Smoke test components:**

1. **DB initialization**
   - Create `benchmarks/eval_smoke.duckdb` via `create_db.py --db eval_smoke.duckdb`

2. **Exp A smoke (polybench, MINI, 3 kernels, 10 trials)**
   ```sh
   python3 optimize_benchmarks.py <polybench_binary> \
     --db eval_smoke.duckdb \
     --study-name smoke_polybench_A \
     --n-trials 10 \
     --seed 0 \
     --benchmark-filter 'g:polybench.*n:(gemm|correlation|lu);.*s:MINI;.*t:(jit_overhead|specialized_exec)'
   ```
   **Verify**: `v_optim_best_per_kernel` returns 3 rows; `v_optim_breakeven.break_even_calls` non-null for at least 1 kernel.

3. **Exp B smoke (3 configs × 2 polybench MINI kernels)**
   - Manually invoke `ablation_benchmarks.py` (once built) with config subset `[default, o3_only, no_unroll]`
   - Filter: same 3 kernels at MINI
   - **Verify**: `ablation_studies` table has 3 × 3 rows (3 configs, 3 reps each); `v_ratios` produces per-run speedup; no NULL run_ids.

4. **Exp C smoke (transfer: polybench-optimal env vars → TPC-H q1)**
   - Extract best `params_json` from smoke study
   - Run TPC-H q1 under those env vars using `ablation_benchmarks.py`
   - **Verify**: row appears in `benchmarks` table with tpch group label and the correct run_id.

5. **Exp D smoke (break-even query)**
   - Run DuckDB query: `SELECT * FROM v_optim_breakeven WHERE study_name='smoke_polybench_A'`
   - **Verify**: returns rows; `break_even_calls` has expected sign (positive for kernels with speedup).

6. **Exp E smoke (sensitivity: 1 parameter, 3 sweep points)**
   - Invoke `sensitivity_analysis.py` for `fixpoint_max` ∈ {0, 5, 10} on 2 MINI kernels
   - **Verify**: 3 rows in DB for the sweep; OAT delta is non-zero.

**Smoke test pass criteria**: All 6 verifications pass without assertion errors, DuckDB
contains no orphaned rows (all benchmarks have a valid run_id in context), and no
`used_timeout_fallback=TRUE` rows appear (MINI benchmarks are fast).

---

## Phase 1: Design

### Data Model

The existing `benchmarks.duckdb` schema supports Experiments A and D without changes.
Experiments B, C, and E need one new table.

#### New Table: `ablation_studies`

```sql
CREATE TABLE IF NOT EXISTS ablation_studies (
    study_name   VARCHAR NOT NULL,
    config_name  VARCHAR NOT NULL,
    params_json  JSON    NOT NULL,   -- env var overrides applied
    rep          INTEGER NOT NULL,   -- 0-indexed repetition number
    run_id       VARCHAR REFERENCES context(run_id),
    PRIMARY KEY (study_name, config_name, rep)
);
```

**Rationale**: Connects named ablation/transfer/sensitivity configs to the raw benchmark
rows in `benchmarks`/`context`. Queries join `ablation_studies` → `context` → `v_ratios`
to get per-kernel timings for each config and rep.

#### New View: `v_ablation_results`

```sql
CREATE OR REPLACE VIEW v_ablation_results AS
SELECT
    a.study_name, a.config_name, a.rep,
    a.params_json,
    r.kernel, r."group",
    r.t_jit_ns, r.t_spec_ns, r.t_unspec_ns
FROM ablation_studies a
JOIN v_ratios r USING (run_id);
```

#### New View: `v_ablation_medians`

```sql
CREATE OR REPLACE VIEW v_ablation_medians AS
SELECT
    study_name, config_name, kernel, "group",
    PERCENTILE_CONT(0.5) WITHIN GROUP (ORDER BY t_jit_ns)    AS med_jit_ns,
    PERCENTILE_CONT(0.5) WITHIN GROUP (ORDER BY t_spec_ns)   AS med_spec_ns,
    PERCENTILE_CONT(0.5) WITHIN GROUP (ORDER BY t_unspec_ns) AS med_unspec_ns,
    COUNT(*) AS n_reps
FROM v_ablation_results
GROUP BY study_name, config_name, kernel, "group";
```

**No other schema changes.** Experiments A and D use the existing `optim_*` tables
and `v_optim_*` views unchanged.

### Contracts / Script Interfaces

#### `ablation_benchmarks.py`

```text
Usage: ablation_benchmarks.py BINARY [options]

Arguments:
  BINARY                    Path to compiled benchmark binary

Options:
  --db PATH                 DuckDB path (default: CWD/benchmarks.duckdb)
  --study-name STR          Study label (default: ablation_YYYYMMDD_HHMMSS)
  --configs PATH            JSON file listing named configs (default: built-in 8 configs)
  --benchmark-filter PATT   Benchmark filter forwarded to binary
  --reps N                  Repetitions per config (default: 3)
  --timeout SEC             Per-run timeout in seconds (default: 120)

Config JSON format:
  [{"name": "default", "env": {}},
   {"name": "o3_only", "env": {"CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS": "0", ...}},
   ...]

Stored in: ablation_studies + context + benchmarks tables.
```

#### `sensitivity_analysis.py`

```text
Usage: sensitivity_analysis.py BINARY [options]

Arguments:
  BINARY                    Path to compiled benchmark binary

Options:
  --db PATH                 DuckDB path
  --study-name STR          Study label
  --optimal-config PATH     JSON file with optimal params_json (from Exp A best-config output)
  --param NAME              Parameter to sweep (one of: fixpoint_max, unroll_max, ...)
  --sweep-values V1,V2,...  Comma-separated values to test (overrides auto-grid)
  --benchmark-filter PATT   Forwarded to binary
  --reps N                  Repetitions per sweep point (default: 1)

Stores sweep results in: ablation_studies (config_name = "sens_{param}_{value}") + context + benchmarks.
Prints Optuna importance for the corresponding Experiment A study if --optuna-study is provided.
```

#### `run_evaluation.sh`

```text
Orchestration script. Usage:
  ./run_evaluation.sh <polybench_binary> <tpch_binary> [--smoke-only] [--db PATH]

Phases:
  0. smoke      — runs smoke test against eval_smoke.duckdb; exits on failure
  A. optim      — runs optimize_benchmarks.py for polybench + tpch
  B. ablation   — runs ablation_benchmarks.py for polybench (all EXTRALARGE kernels)
  C. transfer   — runs ablation_benchmarks.py with cross-workload configs
  E. sensitivity — runs sensitivity_analysis.py for all 6 parameters on polybench

Use --smoke-only to run only phase 0.
```

### Experiment-to-Run Schedule

The table below maps each experiment to the concrete benchmark runs, the data stored,
and what the downstream analysis queries.

| Exp | Script | Workload | Filter | n_trials/reps | Study name pattern | Tables written | Analysis query |
|-----|--------|----------|--------|---------------|-------------------|----------------|----------------|
| Smoke-A | `optimize_benchmarks.py` | polybench MINI | 3 kernels | 10, seed=0 | `smoke_polybench_A` | `optim_trial_params`, `benchmarks` | `v_optim_best_per_kernel` |
| Smoke-B | `ablation_benchmarks.py` | polybench MINI | 3 kernels | 3 reps × 3 configs | `smoke_ablation` | `ablation_studies`, `benchmarks` | `v_ablation_medians` |
| Smoke-C | `ablation_benchmarks.py` | tpch q1 | q1 only | 3 reps × 1 config | `smoke_transfer` | `ablation_studies`, `benchmarks` | `v_ablation_results` |
| Smoke-E | `sensitivity_analysis.py` | polybench MINI | 2 kernels | 3 pts × 1 param | `smoke_sens` | `ablation_studies`, `benchmarks` | `v_ablation_medians` |
| **A-poly** | `optimize_benchmarks.py` | polybench EXTRALARGE | all kernels | **150, seed=42** | `polybench_optim_YYYYMMDD` | `optim_trial_params`, `benchmarks` | `v_optim_best_per_kernel`, `v_optim_breakeven` |
| **A-tpch** | `optimize_benchmarks.py` | tpch all | q1+q6+q3 | **150, seed=42** | `tpch_optim_YYYYMMDD` | `optim_trial_params`, `benchmarks` | `v_optim_best_per_kernel` |
| **B-poly** | `ablation_benchmarks.py` | polybench EXTRALARGE | all kernels | **3 reps × 9 configs** | `ablation_polybench_YYYYMMDD` | `ablation_studies`, `benchmarks` | `v_ablation_medians` |
| **C** | `ablation_benchmarks.py` | polybench + tpch EXTRALARGE | all | **3 reps × 4 configs each** | `transfer_YYYYMMDD` | `ablation_studies`, `benchmarks` | `v_ablation_medians` + Wilcoxon test |
| **D** | *(no new runs)* | — | — | — | — | — | `v_optim_breakeven WHERE study_name='polybench_optim_*'` |
| **E** | `sensitivity_analysis.py` | polybench EXTRALARGE | all kernels | **1 rep × 48 pts** | `sens_polybench_YYYYMMDD` | `ablation_studies`, `benchmarks` | `v_ablation_medians` per param |

### Data Flow Diagram

```
build release binary
       │
       ▼
[Smoke test] ──── eval_smoke.duckdb ──── verify pass criteria
       │
       ▼  (only if smoke passes)
       │
       ├──► optimize_benchmarks.py (polybench, 150 trials) ──────────┐
       │                                                               │
       ├──► optimize_benchmarks.py (tpch, 150 trials) ───────────────┤
       │                                                               │
       │    [Extract best params_json from v_optim_best_per_kernel]   │
       │                                        │                     ▼
       ├──► ablation_benchmarks.py ─────────────┤          benchmarks.duckdb
       │    (8 built-in configs + workload_opt) │               │
       │                                        │           v_optim_breakeven
       ├──► ablation_benchmarks.py ─────────────┤           v_ablation_medians
       │    (cross-workload transfer, 4 configs) │           v_ablation_results
       │                                        │
       └──► sensitivity_analysis.py ────────────┘
            (6 params × 8 pts)
                        │
                        ▼
              reporting/ scripts ──► thesis figures
```

### Reporting Pipeline

Each figure maps to a DuckDB query + a Python plotting script:

| Figure | Script | Query source |
|--------|--------|-------------|
| Pareto front (jit vs exec) per workload | `plot_pareto.py` | `v_optim_breakeven WHERE study_name IN (...)` |
| Per-kernel speedup vs Default (bar chart) | `plot_ablation.py` | `v_ablation_medians WHERE study_name='ablation_polybench_*'` |
| Break-even distribution (box plot) | `plot_breakeven.py` | `v_optim_breakeven` for polybench optimal study |
| Parameter sensitivity (OAT line chart) | `plot_sensitivity.py` | `v_ablation_medians WHERE study_name='sens_polybench_*'` |
| Cross-workload degradation table | inline in `run_evaluation.sh` | `v_ablation_medians` + `scipy.stats.wilcoxon` |

### Key Engineering Decisions

1. **`ablation_benchmarks.py` stores into the existing schema** (not a parallel schema).
   The new `ablation_studies` table is the only addition. This preserves the ability to
   join with `v_ratios` and `v_optim_*` views for unified queries.

2. **Experiment D requires no new runs**. `v_optim_breakeven` already derives
   break-even from the optimization study data. The only work is the reporting script.

3. **Sensitivity analysis reuses `ablation_benchmarks.py` infrastructure**.
   Each OAT sweep point is modeled as a named ablation config (e.g., `sens_fixpoint_max_5`).
   The `sensitivity_analysis.py` script is a thin wrapper that generates those config
   objects and delegates to the same data-store functions.

4. **Statistical test for Experiment C (Wilcoxon) runs post-hoc** from DuckDB data.
   It is not embedded in the benchmark runner. The `run_evaluation.sh` script invokes
   it after all transfer runs complete, printing p-values and summary statistics.

5. **Scipy availability**: `scipy` must be present in `flake.nix`. Verify before implementing
   Experiment C. If absent, add to the Nix flake as part of the implementation tasks.

---

## Complexity Tracking

No constitution violations. No complexity justification required.
