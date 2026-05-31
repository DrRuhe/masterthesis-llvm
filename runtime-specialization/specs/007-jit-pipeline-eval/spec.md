# Feature Specification: JIT Pipeline Evaluation — Iteration Loop


**Feature Branch**: `007-jit-pipeline-eval`  
**Created**: 2026-05-10  
**Status**: Draft  

## Overview

This specification defines an **iterative evaluation loop** for answering the thesis
research question:

> **"What is the best possible Specialization JIT pipeline for a given workload?"**

The loop is anchored by `optimize_benchmarks.py` — an Optuna TPE optimizer that searches
the JIT pipeline parameter space and returns the best config found within a trial budget.
Each iteration runs the optimizer, inspects the results, performs supporting experiments
(ablation, transfer, sensitivity), and ends with a **reflection phase** that documents what
was learned and what to do differently next time.

The goal is not a single definitive answer but a converging understanding: each iteration
either confirms a finding or reveals a gap that narrows the next iteration's search.

### Iteration Structure

```
┌─────────────────────────────────────────────────────────────────┐
│  ITERATION N                                                    │
│                                                                 │
│  1. Optimize  ─► optimize_benchmarks.py (UC MEDIUM, 150 trials) │
│  2. Analyze   ─► ablation + transfer + sensitivity experiments  │
│  3. Reflect   ─► document findings, gaps, infra improvements   │
│                                                                 │
│  ──► feeds into ITERATION N+1 (if warranted)                   │
└─────────────────────────────────────────────────────────────────┘
```

**Workloads in scope**: UC benchmarks (uc1_sql, uc2_conv, uc7_dfa, uc8_ivm,
uc12_groupby, uc14_sort) at MEDIUM size — these most directly reflect the intended use of
runtime specialization. TPC-H is used in the transfer experiment only.

**Why MEDIUM size**: Size-scaling data (`benchmarks/reports/260517-13-30-size_scaling/`)
shows that relative speedup is stable across MEDIUM/LARGE/EXTRALARGE for all UC groups.
MEDIUM reduces per-trial runtime ~5× vs EXTRALARGE without changing the optimizer's
objective landscape.

---

## Sub-Questions

| ID  | Sub-Question |
|-----|-------------|
| SQ1 | What does "best" mean for a given workload and call pattern? |
| SQ2 | What is the shape of the JIT-overhead vs. execution-speedup tradeoff space? |
| SQ3 | Which pipeline components contribute most to execution quality? |
| SQ4 | Does the optimal configuration generalize across workloads? |
| SQ5 | Under what conditions (call frequency, module size) does specialization pay off? |
| SQ6 | What should be improved in the infrastructure and process to answer these questions better? |

---

## User Scenarios & Testing *(mandatory)*

### User Story 1 — Run the Optimizer and Identify Best Config (Priority: P1) 🎯

The thesis researcher runs `optimize_benchmarks.py` on UC benchmarks (MEDIUM size, 150
Optuna TPE trials, seed=42) to find the pipeline configuration that minimizes
geomean(jit_overhead_ns + specialized_exec_ns) across all UC groups.

**Why this priority**: This is the direct, empirical answer to the core research question.
The optimizer is the engine of the iteration loop — everything else interprets its output.

**Independent Test**: After running the experiment, the researcher can query
`v_optim_best_per_kernel` in DuckDB and recover a specific `params_json` that is
demonstrably better than (or equivalent to) `Options::Default()` on the combined metric.

**Acceptance Scenarios**:

1. **Given** the UC benchmark binary, **When** `optimize_benchmarks.py` completes 150 trials, **Then** a `params_json` is stored in DuckDB for each UC group; Optuna convergence data is available.
2. **Given** the best config, **When** compared to `Options::Default()`, **Then** the improvement (or equivalence) is quantified in percentage terms on geomean jit+exec.
3. **Given** the Pareto front from trial data, **When** plotted per UC group, **Then** the tradeoff between JIT overhead and execution speedup is visible and at least one config Pareto-dominates `Options::Default()` for ≥ 4 of 6 groups.

---

### User Story 2 — Ablation: Identify Which Passes Drive Speedup (Priority: P2)

The researcher systematically disables individual pipeline stages to measure each stage's
marginal contribution to execution speedup on UC benchmarks (MEDIUM size). This answers
"which passes are essential?" and supports thesis claims about the pipeline design.

**Why this priority**: Without ablation, the thesis cannot make claims about *why* the
optimized pipeline outperforms a simpler one. The ablation produces the evidence.

**Independent Test**: For at least one UC group, the researcher can show that removing a
specific pipeline stage reduces execution speedup by a measurable amount relative to Default.

**Acceptance Scenarios**:

1. **Given** the full Default pipeline, **When** each stage is individually disabled (O3Only, no-early-prune, no-loop-unroll, no-fixpoint, no-o3-final), **Then** exec speedup for each config is recorded and the delta vs. Default is computed.
2. **Given** the ablation results, **When** stages are ranked by their speedup contribution, **Then** the ranking is stable across at least 3 UC groups (i.e., the same stages appear most/least important).

---

### User Story 3 — Cross-Workload Config Transfer (Priority: P2)

The researcher applies the UC-optimal config to TPC-H benchmarks, measuring the
performance gap to determine whether a single universal config or workload-specific configs
are more appropriate.

**Why this priority**: The thesis must take a position on whether the optimal pipeline is
workload-specific. Without this experiment the question is unanswered.

**Independent Test**: Run TPC-H benchmarks under the UC-optimal config and under
`Options::Default()`. A ratio > 1.2 (>20% worse vs Default or vs own-optimal) suggests
the UC config does not generalise; a ratio ≤ 1.05 suggests a universal config is viable.

**Acceptance Scenarios**:

1. **Given** the UC-optimal `params_json`, **When** TPC-H benchmarks run with those parameters, **Then** the combined jit+exec geomean degradation vs. `Options::Default()` is quantified.
2. **Given** both UC and TPC-H transfer results, **When** a paired Wilcoxon rank-sum test (α = 0.05) across groups is applied, **Then** the thesis can make a statistically grounded claim about workload-specificity.

---

### User Story 4 — Break-Even and Payoff Characterization (Priority: P3)

The researcher quantifies the minimum call count required for JIT specialization to
recover its overhead under each pipeline configuration, producing practical guidance
for when specialization is worth the cost.

**Why this priority**: JIT overhead is real and the thesis must address it directly. The
break-even characterization makes the "when to specialize" claim concrete and testable.

**Independent Test**: From `v_optim_breakeven`, produce a plot of break-even call count
vs. pipeline configuration for each UC group. Verify that at least one configuration
achieves break-even in ≤ 10 calls for the groups where specialization provides speedup.

**Acceptance Scenarios**:

1. **Given** the full optimization study results, **When** break-even is computed for the optimal config, **Then** break-even call counts are available for all groups where `unspec_ns > spec_ns`.
2. **Given** break-even data, **When** compared across pipeline variants, **Then** configurations with lower JIT overhead achieve smaller break-even counts even if their execution speedup is slightly worse.
3. **Given** groups where `spec_ns ≥ unspec_ns` (no exec speedup), **Then** they are flagged as "never worth specializing" and excluded from optimization claims.

---

### User Story 5 — Reflection: Improve Infrastructure and Process (Priority: P2)

After experiments complete, the researcher writes a structured reflection document that
captures what was learned about the *evaluation process itself* — not just the JIT
pipeline results. This reflection is the primary output for driving future iterations.

**Why this priority**: The evaluation loop is only as good as the feedback it produces.
Without a reflection phase, each iteration starts from scratch. With it, the researcher
accumulates process knowledge that compounds across iterations.

**Reflection scope**:

- **Infrastructure improvements**: What changes to `optimize_benchmarks.py`, the
  benchmark binaries, or the DuckDB schema would make future iterations faster, more
  reliable, or more informative? (Examples: better timeout handling, richer parameter
  space, multi-objective Optuna support, additional UC benchmarks.)
- **Evaluation improvements**: What changes to the *process* — trial budget, workload
  selection, size choice, repetition count, statistical tests — would produce higher
  confidence results in less time? (Examples: pruning insensitive parameters, using
  warm-start from previous study, focusing trials on high-variance groups.)
- **Finding quality**: Which results were statistically robust? Which were noisy or
  borderline? What would be needed to confirm borderline findings?

**Independent Test**: A reflection document exists at
`benchmarks/reports/<YYMMDD-HH-MM>-optimize-pipeline/reflection.md` and contains at least
one concrete, actionable improvement for each of the three scopes above.

**Acceptance Scenarios**:

1. **Given** the optimization study, **When** the reflection is written, **Then** it identifies at least one parameter that can be removed from future searches (insensitive finding from sensitivity analysis) or one that needs finer resolution (high-sensitivity finding).
2. **Given** the ablation and transfer results, **When** the reflection is written, **Then** it documents whether the current ablation config set adequately covers the pipeline design space, or whether new configs are needed.
3. **Given** the full iteration's data, **When** the reflection is written, **Then** it proposes a concrete scope for the *next* iteration: which experiments to repeat, which to skip, and what to add.

---

### User Story 6 — Thesis-Ready Pareto Reporting (Priority: P2)

The researcher needs to drop Pareto plots and their underlying data straight into the
thesis without further data wrangling. The reporting script must (a) plot the Pareto front
in the correct orientation (both axes minimized — lower-left is best), (b) clearly mark
`Options::Default()` for comparison, and (c) emit a CSV with the structure described in
FR-002b so the same data can be re-rendered in the thesis using pgfplots/LaTeX or filtered
into summary tables.

**Why this priority**: US1 already produces a Pareto plot but it is not thesis-ready —
the iteration-1 plot had the frontier oriented to the upper-right (maximization) instead
of the lower-left (minimization), and the underlying data was not exported in a thesis-
friendly CSV. Without this script the researcher would re-do the work manually when
writing the thesis chapter.

**Independent Test**:

1. Run `plot_pareto_configs.py --db benchmarks/benchmarks.duckdb --study-name <study> --output-dir benchmarks/reports/<YYMMDD-HH-MM>-optimize-pipeline/`.
2. Open the resulting CSV in pandas: rows where `is_pareto_optimal=True`, sorted by
   `jit_overhead_ms` ascending, MUST have `specialized_exec_ms` monotonically
   non-increasing. Exactly one row per (group) MUST have `is_default=True`.
3. Open the PNG: the connected Pareto frontier line MUST sit at the bottom-left of the
   plot, with `Options::Default()` rendered as a star, distinguishable from the
   Pareto-optimal points.

**Acceptance Scenarios**:

1. **Given** the `uc_optim_YYYYMMDD` study, **When** the script runs, **Then** a PNG and
   CSV are produced for the study; the CSV columns match FR-002b exactly.
2. **Given** the CSV, **When** loaded in LaTeX/pgfplots, **Then** filtering rows where
   `is_pareto_optimal=True` produces a polyline that traces the lower-left envelope of
   the scatter (no further computation required in the LaTeX side).
3. **Given** the `--per-group` flag, **When** the script runs, **Then** one PNG+CSV pair
   per UC group is written, named `pareto_<study>_<group>.{png,csv}`.

---

### Edge Cases

- A UC group where specialized execution is *slower* than unspecialized: flagged as a
  regression; the pipeline is analyzed for over-specialization. Documented in reflection.
- A study where no trial beats `Options::Default()`: the result is reported as
  "Default is near-optimal for this workload" — a valid, publishable finding. The
  reflection documents whether the search space should be expanded or the objective changed.
- Groups that time out during optimization trials: fallback values used per the existing
  `optimize_benchmarks.py` protocol; documented in reflection as infrastructure gap.
- Reflection reveals a fundamental infrastructure gap (e.g., the optimizer objective
  doesn't reflect the actual use case): this becomes the primary input to the next iteration,
  which may change the optimizer before running any new experiments.

---

## Requirements *(mandatory)*

### Functional Requirements

**Experiment A — Pareto Frontier Mapping via Optimizer (addresses SQ1, SQ2)**

- **FR-001**: The evaluation MUST run `optimize_benchmarks.py` on UC benchmarks (MEDIUM
  size, 6 groups) with a fixed random seed (seed=42), storing results in a named DuckDB
  study (`uc_optim_YYYYMMDD`). The trial budget SHOULD be ≥ 50 trials; the original target
  of 150 is not required given observed TPE convergence. In study `uc_optim_iter3_20260531`
  (50 trials), the objective plateaued at trial 6 (169.3ms) with no improvement through
  trial 49, and parameter importance showed `pipeline` (56%) and `large_module_max` (39%)
  accounting for ≥95% of explained variance — confirming 50 trials is sufficient to
  characterize this search space. A budget of ≥ 50 trials is therefore acceptable; runs
  with fewer than 50 trials MUST include a plateau check before claiming convergence.
- **FR-002**: The evaluation MUST plot the per-group Pareto frontier of
  (jit_overhead_ns ↓, specialized_exec_ns ↓) from the Optuna trial data, with
  `Options::Default()` marked as a reference point. **Both axes are minimized**: a config
  `A` Pareto-dominates `B` iff `A.jit ≤ B.jit AND A.spec ≤ B.spec` with at least one
  strict inequality. The Pareto-optimal set is therefore the **lower-left envelope** of
  the point cloud; the frontier line connects these points sorted by ascending JIT
  overhead, along which specialized exec is monotonically non-increasing.
- **FR-002b**: A thesis-ready Pareto reporting script (`benchmarks/reporting/plot_pareto_configs.py`)
  MUST produce, for each named study:
  1. A PNG with x = JIT overhead (ms), y = specialized exec (ms), one dot per sampled
     configuration; the default config is marked with a star (★); Pareto-optimal points
     are highlighted in a distinct color and connected by a line in the lower-left.
  2. A CSV (`<output-dir>/pareto_<study_name>.csv` or `pareto_<study_name>_<group>.csv`
     when `--per-group` is set) containing one row per (config, kernel) with typed
     parameter columns (`fixpoint_max`, `unroll_max`, `large_module_max`, `early_prune`,
     `o3_final`) plus `group`, `kernel`, `jit_overhead_ms`, `specialized_exec_ms`,
     `is_pareto_optimal` (bool), and `is_default` (bool). The CSV is intended for direct
     inclusion in the thesis (e.g., pgfplots, LaTeX tables) and MUST NOT require post-
     processing to be filtered by Pareto-optimality or default status.
- **FR-003**: The "best config" for the UC workload MUST be defined as the configuration
  that minimizes geomean(jit_overhead_ns + specialized_exec_ns) across all UC groups. This
  matches the existing `optimize_benchmarks.py` objective and implicitly assumes the number
  of calls is unknown. The Pareto frontier (FR-002) is reported as supplementary information.
- **FR-004**: The experiment MUST record the exact `params_json` for the best config in
  DuckDB and export it to `<REPORT_DIR>/uc_best_config.json` (where `<REPORT_DIR>` is the
  per-run folder `benchmarks/reports/<YYMMDD-HH-MM>-optimize-pipeline/`) for use in later
  experiments.
- **FR-004b**: Optuna parameter importance MUST be extracted immediately after
  `study.optimize()` completes and stored as `importance_<study_name>.json` alongside the
  best-config output. This is needed for the sensitivity analysis and reflection phase.

**Experiment B — Ablation Study (addresses SQ3)**

- **FR-005**: The ablation MUST test the following named configs against UC benchmarks
  (MEDIUM): `O3Only`, `Default`, `no_prune`, `no_o3_final`, `no_unroll`, `fixpoint_1`,
  `aggressive`, `pipeline_1`, and `uc_workload_optimal` (from Experiment A).
- **FR-006**: For each config, the ablation MUST record per-group jit_overhead_ns,
  specialized_exec_ns, and exec speedup ratio (unspec_ns / spec_ns).
- **FR-007**: ≥ 3 repetitions per config; median values used for comparisons.
- **FR-008**: Results stored in DuckDB (`ablation_studies` table) with config name as label.

**Experiment C — Cross-Workload Config Transfer (addresses SQ4)**

- **FR-009**: The transfer experiment MUST run UC benchmarks (MEDIUM) and TPC-H under four
  shared configs: `Default`, `aggressive`, `uc_optimal`, `uc_workload_optimal`.
- **FR-010**: Per-group combined cost (jit_ns + spec_ns) is computed under each config;
  a paired Wilcoxon rank-sum test (α = 0.05) determines statistical significance of the
  difference between UC-optimal and Default on TPC-H.
- **FR-011**: All transfer runs use the same hardware as Experiments A and B.

**Experiment D — Break-Even Analysis (addresses SQ5)**

- **FR-012**: Break-even MUST be computed from `v_optim_breakeven` for every (study, group)
  combination produced by Experiments A and B.
- **FR-013**: Results MUST include: (a) distribution of break-even counts per pipeline config,
  (b) per-group table showing which config achieves the lowest break-even count,
  (c) scatter plot of module instruction count vs. break-even count.

**Experiment E — Sensitivity Analysis (addresses SQ2, SQ3)**

- **FR-014**: OAT sensitivity analysis MUST vary each parameter across its full range while
  holding others fixed at the UC-optimal value; combined cost recorded per sweep point.
- **FR-015**: Optuna parameter importance scores (from `importance_<study>.json`) MUST be
  reported alongside OAT results.
- **FR-016**: Parameters are classified as "critical" (>10% cost change) or "insensitive"
  (<5% change) to guide future search space pruning.

**Reflection Phase (addresses SQ6)**

- **FR-017**: A reflection document MUST be written at
  `<REPORT_DIR>/reflection.md` (per-run folder created by `run_evaluation.sh` at
  `benchmarks/reports/<YYMMDD-HH-MM>-optimize-pipeline/`) after all experiments complete.
- **FR-018**: The reflection MUST cover three scopes:
  1. **Infrastructure**: actionable improvements to `optimize_benchmarks.py`, benchmark
     binaries, DuckDB schema, or tooling (with specific suggested changes).
  2. **Evaluation process**: actionable improvements to trial budget, workload selection,
     repetition count, statistical approach, or size selection (with concrete proposals).
  3. **Next iteration scope**: explicit proposal for what the next iteration should do
     differently — which experiments to skip, repeat, or add; which parameters to prune
     from the search space based on sensitivity results.
- **FR-019**: The reflection MUST be grounded in data: each claim MUST cite a specific
  result (study name, query, or plot) as evidence.

**Methodology-Level Requirements**

- **FR-020**: All experiments MUST be run in a controlled environment: CPU scaling disabled,
  no background load, fixed CPU affinity where possible (documented in the thesis).

  **Evaluation machine (recorded 2026-05-31)**:

  | Property | Value |
  |---|---|
  | CPU | Intel Core i9-12900H (Alder Lake, 12th Gen) |
  | Architecture | x86_64 |
  | P-cores / E-cores | 6P + 8E = 14 cores, 20 logical CPUs (HT on P-cores) |
  | Base / Boost clock | 2.5 GHz base / 5.0 GHz max boost |
  | L1d cache | 544 KiB (14 instances: 6 × 48 KiB P-core + 8 × 32 KiB E-core) |
  | L1i cache | 704 KiB (14 instances) |
  | L2 cache | 11.5 MiB (8 instances: 6 × 1.25 MiB P-core + 1 × 2 MiB E-core shared) |
  | L3 cache | 24 MiB (shared) |
  | RAM | 64 GiB DDR5 |
  | OS | Linux 6.17.0-1023-oem (Ubuntu OEM kernel) |
  | Compiler | Clang 18.1.8 (project-built LLVM 21.1, release mode) |
  | Build flags | `-O3 -g` with IRDumpingPass plugin, release build (NDEBUG) |
  | CPU frequency scaling | Variable (not pinned; scaling governor: `schedutil`). Note: benchmarks record `cpu MHz` per-run; statistical noise from clock variation is captured in the ≥3-rep median. |
  | NUMA topology | 1 NUMA node (all 20 CPUs on node 0) |

  **Threats from this configuration**: The i9-12900H is a hybrid architecture with
  performance (P) and efficiency (E) cores running at different clock speeds. The OS
  scheduler may migrate benchmark threads between P-cores and E-cores mid-run, adding
  measurement noise. Mitigation: use ≥3 repetitions, report median. CPU affinity
  pinning to P-cores via `taskset -c 0-11` is recommended for final thesis measurements.
- **FR-021**: The exact binary (git SHA, build flags), Python environment, and DuckDB schema
  version MUST be recorded alongside results for reproducibility.
- **FR-022**: Each experiment MUST have a defined fallback if a benchmark times out (consistent
  with `optimize_benchmarks.py`'s existing timeout handling).

### Key Entities

- **Workload**: A set of UC benchmark groups (uc1–uc14 at MEDIUM size); defined by the
  binary path and benchmark filter pattern.
- **Group**: A single UC benchmark group (e.g., `uc8_ivm`); the unit of measurement.
- **Pipeline configuration**: A fully-specified `Options` struct (fixpoint_max, unroll_max,
  large_module_threshold, early_prune, o3_final, pipeline variant); serialized as `params_json`.
- **Pareto-optimal config**: A config where no other config simultaneously achieves
  **lower-or-equal** JIT overhead **and** lower-or-equal specialized exec time with at
  least one strictly lower. Both metrics are minimized; the Pareto-optimal set is the
  lower-left envelope of the (jit, spec) cloud.
- **Break-even count**: The minimum number of calls to the specialized function for its
  total cost (JIT overhead + N × spec_exec) to be less than N × unspec_exec.
- **Iteration**: One complete run of the evaluation loop (optimize → analyze → reflect).
- **Reflection document**: A structured markdown file capturing infrastructure improvements,
  evaluation process improvements, and next-iteration scope.

---

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: At least one pipeline config is identified that achieves ≥ 10% improvement
  in the primary cost metric over `Options::Default()`, or Default is confirmed as
  near-optimal (within 5%) — either outcome is a publishable finding.
- **SC-002**: The ablation study produces a ranking of pipeline components by speedup
  contribution where the top-2 components together account for ≥ 70% of the total speedup
  gap between `O3Only` and the UC-optimal config.
- **SC-003**: The transfer experiment produces a concrete degradation percentage with a
  p-value; the thesis conclusion on workload-specificity is statistically grounded.
- **SC-004**: Break-even call counts are computed for every UC group that shows exec speedup;
  the distribution is summarized (median, min, max).
- **SC-005**: The sensitivity analysis classifies all 6 parameters as critical or insensitive;
  at least one is classified insensitive (search space can be pruned in next iteration).
- **SC-006**: A reflection document exists at `<REPORT_DIR>/reflection.md` (the per-run
  folder `benchmarks/reports/<YYMMDD-HH-MM>-optimize-pipeline/`) with ≥ 1 actionable
  improvement per scope (infrastructure, process, next iteration). Past iterations remain
  discoverable as `benchmarks/reports/*-optimize-pipeline/reflection*.md`.
- **SC-007**: All results are stored in DuckDB with metadata (git SHA, machine info, study
  name) sufficient to reproduce thesis figures.

---

## Methodology Weaknesses and Framing

### W1: Limited Workload Diversity

Only UC benchmarks (runtime specialization use cases) are the primary workload; TPC-H is
secondary (transfer only). Conclusions may not generalise to interactive workloads, event-
driven systems, or workloads with unpredictable argument distributions.

**Framing**: Explicitly acknowledged in threats-to-validity. Claims scoped to "UC-style
workloads with stable runtime argument values." The reflection phase may propose adding new
UC benchmarks in the next iteration.

### W2: Hardware Specificity

All timing results are collected on a single machine. Results are not portable across
architectures.

**Framing**: Document the machine (CPU model, cache hierarchy, memory). State results
constitute a case study on this hardware. The reflection may note whether hardware
portability matters for the thesis claims.

### W3: Optuna Trial Budget

With 150 trials over a 6-parameter space, TPE may not find the global optimum. The
optimizer finds a *good* config, not provably the *best*.

**Framing**: Use language such as "best config found within 150 trials" rather than
"optimal config." Include convergence plots. The reflection phase must assess whether
150 trials were sufficient (plateau check) and recommend a budget for the next iteration.

### W4: Single Objective

`optimize_benchmarks.py` uses geomean(jit + exec) as a scalar objective, collapsing the
JIT-overhead / speedup tradeoff. The Pareto front (Experiment A) supplements this but the
primary ranking is scalar.

**Mitigation**: Experiment A plots the Pareto front; the reflection documents whether the
scalar objective was misleading for any UC group.

### W5: Measurement Noise

JIT compilation times are noisy (LLVM passes, OS scheduler, memory allocation). Exec times
are more stable but can vary for short kernels.

**Mitigation**: ≥ 3 repetitions and median values; document inter-run variance. The
reflection assesses whether noise materially affected any conclusion.

### W6: Definition of "Best" is Application-Dependent

The chosen objective implicitly assumes one call amortizes JIT overhead. The break-even
analysis (Experiment D) provides complementary guidance. The reflection must document
whether the objective aligned with the actual thesis use case.

---

## Assumptions

- The UC benchmark binaries compiled with the IRDumpingPass represent the target workload
  space for this thesis.
- The DuckDB benchmarks database schema is stable (or will be migrated before experiments run).
- CPU frequency scaling can be disabled on the target machine.
- The 6-parameter search space in `optimize_benchmarks.py` covers the most important
  pipeline knobs; other parameters are out of scope for this study.
- All measurements use release builds.
- MEDIUM size is representative for UC benchmarks (validated by size-scaling data).
