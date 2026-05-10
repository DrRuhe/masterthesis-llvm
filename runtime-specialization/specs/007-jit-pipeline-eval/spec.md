# Feature Specification: JIT Pipeline Evaluation Methodology

**Feature Branch**: `007-jit-pipeline-eval`  
**Created**: 2026-05-10  
**Status**: Draft  

## Overview

This specification defines an empirical evaluation methodology for answering the thesis
research question:

> **"What is the best possible Specialization JIT pipeline for a given workload?"**

The methodology decomposes the question into five sub-questions, each addressed by a
targeted experiment. Together the experiments produce claims suitable for the thesis
discussion: which pipeline configuration performs best per workload, which pipeline
components drive performance, when specialization pays off, and how workload-specific the
optimal configuration is.

The existing `optimize_benchmarks.py` (Optuna TPE, 80 trials, 6 parameters) is a
necessary starting tool, but it falls short of answering the research question on its own.
It minimizes a single scalar objective (geomean of jit+exec), which collapses the
JIT-overhead / execution-speedup tradeoff into one number and gives no insight into *why*
certain configurations work, or whether the found optimum transfers across workloads.

---

## Sub-Questions

| ID  | Sub-Question |
|-----|-------------|
| SQ1 | What does "best" mean for a given workload and call pattern? |
| SQ2 | What is the shape of the JIT-overhead vs. execution-speedup tradeoff space? |
| SQ3 | Which pipeline components contribute most to execution quality? |
| SQ4 | Does the optimal configuration generalize across workloads? |
| SQ5 | Under what conditions (call frequency, module size) does specialization pay off? |

---

## User Scenarios & Testing *(mandatory)*

### User Story 1 — Determine Optimal Pipeline per Workload (Priority: P1)

The thesis researcher runs a systematic search that produces — for each benchmark workload
(polybench, TPC-H) — the pipeline configuration that minimizes the total cost of JIT
compilation plus specialized execution, as well as the Pareto-optimal configs that
separately minimize JIT overhead and maximize execution speedup.

**Why this priority**: This is the direct, empirical answer to the research question. All
other experiments refine or explain this answer.

**Independent Test**: After running the experiment, the researcher can query
`v_optim_best_per_kernel` in DuckDB and recover a specific `params_json` for each workload
that is demonstrably better than `Options::Default()` on the combined metric.

**Acceptance Scenarios**:

1. **Given** a polybench benchmark binary, **When** the Pareto sweep experiment completes, **Then** a set of configs is identified that Pareto-dominates `Options::Default()` on (jit_overhead_ns, specialized_exec_ns) for at least 80% of polybench kernels.
2. **Given** a TPC-H benchmark binary, **When** the same experiment runs, **Then** a workload-specific best config is identified and stored in the DuckDB study.
3. **Given** both optimal configs, **When** compared to `Options::Default()`, **Then** the improvement (or equivalence) is quantified in percentage terms on geomean jit+exec.

---

### User Story 2 — Ablation: Identify Which Passes Drive Speedup (Priority: P2)

The researcher systematically disables individual pipeline stages to measure each stage's
marginal contribution to execution speedup. This answers "which passes are essential?"
and supports thesis claims about the pipeline design.

**Why this priority**: Without ablation, the thesis cannot make claims about *why* the
optimized pipeline outperforms a simpler one. The ablation produces the evidence.

**Independent Test**: For at least one polybench kernel, the researcher can show that
removing IPSCCP from the fixpoint loop reduces execution speedup by a measurable amount,
while removing loop unrolling has a different (larger or smaller) effect.

**Acceptance Scenarios**:

1. **Given** the full Default pipeline, **When** each pipeline stage is individually disabled (O3Only, no-IPSCCP, no-DevirtVtable, no-early-prune, no-loop-unroll, no-fixpoint), **Then** the exec speedup for each config is recorded and the delta vs. Default is computed.
2. **Given** the ablation results, **When** stages are ranked by their speedup contribution, **Then** the ranking is stable across at least 3 polybench kernels (i.e., the same stages appear most/least important).

---

### User Story 3 — Cross-Workload Config Transfer (Priority: P2)

The researcher applies the polybench-optimal config to TPC-H benchmarks and vice versa,
measuring the performance gap to determine whether a single universal config or
workload-specific configs are more appropriate.

**Why this priority**: The thesis must take a position on whether the optimal pipeline is
workload-specific. Without this experiment the question is unanswered.

**Independent Test**: Run TPC-H benchmarks under the polybench-optimal config and under
the TPC-H-optimal config. Record the ratio of combined costs. A ratio > 1.2 (>20% worse)
suggests the configs are genuinely workload-specific; a ratio ≤ 1.05 suggests a universal
config is viable.

**Acceptance Scenarios**:

1. **Given** the polybench-optimal `params_json`, **When** TPC-H benchmarks run with those parameters, **Then** the combined jit+exec geomean degradation vs. TPC-H-optimal is quantified.
2. **Given** the TPC-H-optimal `params_json`, **When** polybench benchmarks run with those parameters, **Then** the degradation vs. polybench-optimal is quantified.
3. **Given** both transfer experiments, **When** a paired Wilcoxon rank-sum test (or paired t-test if normality holds) across per-kernel costs shows no statistically significant difference (α = 0.05) between own-optimal and foreign-optimal configs, **Then** the thesis can claim a single universal config suffices; if the test is significant, workload-specific tuning is recommended.

---

### User Story 4 — Break-Even and Payoff Characterization (Priority: P3)

The researcher quantifies the minimum call count required for JIT specialization to
recover its overhead under each pipeline configuration, producing practical guidance
for when specialization is worth the cost.

**Why this priority**: JIT overhead is real and the thesis must address it directly. The
break-even characterization makes the "when to specialize" claim concrete and testable.

**Independent Test**: From `v_optim_breakeven`, produce a plot of break-even call count
vs. pipeline configuration for each kernel. Verify that at least one configuration
achieves break-even in ≤ 100 calls for the kernels where specialization provides speedup.

**Acceptance Scenarios**:

1. **Given** the full optimization study results, **When** break-even is computed for the optimal config, **Then** break-even call counts are available for all kernels where `unspec_ns > t_spec_ns`.
2. **Given** break-even data, **When** compared across pipeline variants, **Then** configurations with lower JIT overhead achieve smaller break-even counts even if their execution speedup is slightly worse.
3. **Given** kernels where `t_spec_ns ≥ unspec_ns` (no exec speedup), **Then** they are flagged as "never worth specializing" and excluded from optimization claims.

---

### Edge Cases

- A benchmark kernel where specialized execution is *slower* than unspecialized: flagged as
  a regression candidate; the pipeline is analyzed for over-specialization (e.g., incorrect
  constant propagation or over-unrolling causing code bloat).
- A study where no trial beats `Options::Default()`: the result is reported as
  "Default is near-optimal for this workload" — a valid, publishable finding.
- Kernels that time out during optimization trials: fallback values are used per the
  existing `optimize_benchmarks.py` protocol; timed-out kernels are excluded from ablation
  claims but retained in break-even analysis.
- Cross-workload transfer where both workloads are dominated by JIT overhead (short
  kernel, few iterations): break-even may be infinite; this scenario is separately
  characterized under SQ5.

---

## Requirements *(mandatory)*

### Functional Requirements

**Experiment A — Pareto Frontier Mapping (addresses SQ1, SQ2)**

- **FR-001**: The evaluation MUST run `optimize_benchmarks.py` for each workload (polybench, TPC-H) with ≥ 150 trials and a fixed random seed (for reproducibility), storing results in named DuckDB studies.
- **FR-002**: The evaluation MUST plot the per-kernel Pareto frontier of (jit_overhead_ns, specialized_exec_ns) from the Optuna study data, with `Options::Default()` marked as a reference point.
- **FR-003**: The "best config" for each workload MUST be defined as the configuration that minimizes geomean(jit_overhead_ns + specialized_exec_ns) across all kernels in the workload. This matches the existing `optimize_benchmarks.py` objective and implicitly assumes the number of calls is unknown (one call amortizes the JIT overhead in the cost model). The Pareto frontier (FR-002) is reported as supplementary information but does not define the primary ranking.
- **FR-004**: The experiment MUST record the exact `params_json` for each Pareto-optimal config in the DuckDB study so results are reproducible.

**Experiment B — Ablation Study (addresses SQ3)**

- **FR-005**: The ablation MUST test the following discrete pipeline configurations against the same set of benchmarks: `O3Only` (no fixpoint), `Default` (baseline), `no_ipsccp` (fixpoint without IPSCCP), `no_devirt` (no DevirtVtable pass), `no_early_prune` (early_prune=false), `no_unroll` (unroll_max=1), `no_o3_final` (o3_final=false), and the workload-specific optimal from Experiment A.
- **FR-006**: For each configuration, the ablation MUST record per-kernel jit_overhead_ns, specialized_exec_ns, and exec speedup ratio (unspec_ns / spec_ns).
- **FR-007**: The ablation MUST run each configuration with ≥ 3 repetitions to estimate measurement variance; median values MUST be used for comparisons.
- **FR-008**: Results MUST be stored in the DuckDB benchmarks database using the existing schema (context + benchmarks tables) with the pipeline config name recorded as a study label.

**Experiment C — Cross-Workload Config Transfer (addresses SQ4)**

- **FR-009**: The transfer experiment MUST run each workload under four configs: its own optimal, the other workload's optimal, `Options::Default()`, and `Options::Aggressive()`.
- **FR-010**: The transfer experiment MUST compute per-kernel cost (jit_ns + spec_ns) under both own-optimal and foreign-optimal configs, then apply a paired Wilcoxon rank-sum test (α = 0.05) across kernels to determine whether the difference is statistically significant. The raw degradation percentage (`(cost_foreign / cost_own − 1) × 100%`) MUST also be reported for interpretability.
- **FR-011**: The transfer experiment MUST be run on the same hardware as Experiments A and B to avoid confounding.

**Experiment D — Break-Even Analysis (addresses SQ5)**

- **FR-012**: The break-even analysis MUST use the `v_optim_breakeven` view to compute break-even call counts for every (study, kernel) combination produced by Experiments A and B.
- **FR-013**: Results MUST be presented as: (a) a distribution (box plot or table) of break-even counts per pipeline config, (b) a per-kernel table showing which configs achieve the lowest break-even count, and (c) a scatter plot of module instruction count vs. break-even count to test whether module size predicts payoff.

**Experiment E — Sensitivity Analysis (addresses SQ2, SQ3)**

- **FR-014**: For the workload-specific optimal config from Experiment A, a one-at-a-time (OAT) sensitivity analysis MUST be performed: each parameter is varied across its full range while holding the others fixed at their optimal value; combined cost is recorded.
- **FR-015**: Optuna parameter importance scores (from the TPE model) MUST be extracted and reported alongside the OAT results.
- **FR-016**: The sensitivity analysis identifies which parameters are "critical" (>10% cost change from optimum) vs. "insensitive" (<5% change), providing practical guidance for default settings.

**Methodology-Level Requirements**

- **FR-017**: All experiments MUST be run in a controlled environment: CPU scaling disabled, no background load, fixed CPU affinity where possible (documented in the thesis).
- **FR-018**: The exact binary (git SHA, build flags), Python environment, and DuckDB schema version MUST be recorded alongside results for reproducibility.
- **FR-019**: The thesis MUST explicitly state that results are hardware-specific (machine described in FR-017) and that generalization to other architectures is not claimed.
- **FR-020**: Each experiment MUST have a defined fallback if a benchmark times out: the timeout value is used as the JIT cost, and the unspecialized cost is used as the exec cost (consistent with `optimize_benchmarks.py`).

### Key Entities

- **Workload**: A benchmark suite (polybench or TPC-H) comprising multiple kernels; defined by the binary path and benchmark filter pattern.
- **Kernel**: A single specializable function within a workload (e.g., `correlation`, `gemm`, `q1`); the unit of measurement.
- **Pipeline configuration**: A fully-specified `Options` struct (fixpoint_max, unroll_max, large_module_threshold, early_prune, o3_final, pipeline variant); serialized as `params_json`.
- **Pareto-optimal config**: A configuration where no other config simultaneously achieves lower JIT overhead and lower exec time.
- **Break-even count**: The minimum number of calls to the specialized function for its total cost (JIT overhead + N × spec_exec) to be less than N × unspec_exec.
- **Ablation config**: A named variant of the pipeline where exactly one component is disabled relative to the Default config.

---

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: For each workload, at least one pipeline configuration is identified that achieves ≥ 10% improvement in the primary cost metric over `Options::Default()`, or `Default` is confirmed as near-optimal (within 5%).
- **SC-002**: The ablation study produces a ranking of pipeline components by speedup contribution where the top-2 components together account for ≥ 70% of the total speedup gap between `O3Only` and the workload-optimal config.
- **SC-003**: The cross-workload transfer experiment produces a concrete degradation percentage; the thesis conclusion on workload-specificity is supported by a measurable threshold (not a subjective judgment).
- **SC-004**: Break-even call counts are computed for every kernel that shows exec speedup; the distribution is summarized (median, min, max) per workload.
- **SC-005**: The sensitivity analysis identifies at least one parameter as "insensitive" (< 5% cost impact), providing evidence that the search space can be pruned for future optimization runs.
- **SC-006**: All experimental results are stored in the DuckDB database with enough metadata (git SHA, machine info, study name) to reproduce the figures in the thesis.

---

## Methodology Weaknesses and Framing

The following weaknesses are inherent to this methodology. Each is documented here so the
thesis can address them explicitly.

### W1: Limited Workload Diversity

Only two workload domains are studied (dense linear algebra via polybench; SQL analytics
via TPC-H). Both are batch/analytical workloads with long-running inner loops. Conclusions
may not generalize to: interactive workloads, event-driven systems, or workloads with
unpredictable argument distributions.

**Framing**: The thesis must explicitly acknowledge limited workload diversity in a
**threats-to-validity** section. Claims are scoped to "dense linear algebra and SQL
analytics batch workloads with stable runtime argument values." The limitation is
not addressed by adding new benchmarks; it is documented as a known constraint
consistent with the thesis's defined specialization scope (single-threaded, stable
arguments).

### W2: Hardware Specificity

All timing results are collected on a single machine. JIT pipeline performance is
sensitive to cache sizes, branch predictor behavior, and memory bandwidth. Results are not
portable across architectures.

**Framing**: Document the machine in the methodology section (CPU model, cache hierarchy,
memory). State explicitly that results constitute a case study on this hardware, not a
general benchmark.

### W3: Optuna Trial Budget

With 80–150 trials over a 6-parameter space, TPE may not find the global optimum.
The optimization finds a *good* config, not provably the *best*.

**Framing**: Use language such as "best config found within N trials" rather than "optimal
config". Include Optuna convergence plots to show when improvement plateaus.

### W4: Single Objective in Existing Infrastructure

`optimize_benchmarks.py` uses geomean(jit + exec) as a scalar objective. This collapses
the tradeoff into a single value and may miss Pareto-optimal configs that sacrifice one
dimension for large gains in the other.

**Mitigation**: Experiment A explicitly plots the Pareto front; this extends the current
infrastructure rather than replacing it.

### W5: Measurement Noise

JIT compilation times are noisy (LLVM pass execution, OS scheduler jitter, memory
allocation). Exec times are more stable but can vary 5–15% for short kernels.

**Mitigation**: Use ≥ 3 repetitions and median values; document inter-run variance.
For kernels with high variance, widen the "equivalent" band in cross-workload comparisons.

### W6: Definition of "Best" is Application-Dependent

The chosen objective — minimize geomean(jit + exec) — implicitly assumes one call amortizes
the JIT overhead. A user who calls the specialized function millions of times would
rationally prefer maximum exec speedup at the cost of higher JIT overhead; a user calling
it once would prefer near-zero JIT overhead.

**Framing**: The thesis should document this assumption explicitly: results are optimal for
workloads where the number of specialization calls is unknown or small. The break-even
analysis (Experiment D) provides complementary guidance for readers who know their call
frequency.

---

## Assumptions

- The existing polybench and TPC-H benchmark binaries compiled with the IRDumpingPass
  represent the target workload space for this thesis.
- The DuckDB benchmarks database schema is stable (or will be migrated if changed before
  experiments run).
- CPU frequency scaling can be disabled on the target machine (required for meaningful JIT
  timing measurements).
- The 6-parameter search space in `optimize_benchmarks.py` covers the most important
  pipeline knobs; other parameters (e.g., pass ordering) are out of scope for this study.
- Results from debug builds are not used for performance claims; all measurements are from
  release builds.
- The `Options::FromExpectedRuntime()` heuristic is treated as a comparison baseline, not
  as ground truth.
