# Tasks: JIT Pipeline Evaluation Methodology

**Input**: Design documents from `specs/007-jit-pipeline-eval/`
**Prerequisites**: plan.md ✅, spec.md ✅, research.md ✅, data-model.md ✅

**Organization**: Tasks are grouped by user story. Phases 1–2 are foundational
infrastructure; Phases 3–6 correspond to Experiments A–E from the spec.
The smoke test (end of Phase 2) MUST pass before any full-scale run begins.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no concurrent dependencies)
- **[Story]**: User story the task belongs to (US1–US4)

---

## Phase 1: Setup

**Purpose**: Environment verification and database initialisation.
These tasks have no dependencies and can run immediately.

- [ ] T001 Verify `scipy.stats.wilcoxon` is available in the Nix environment: run `python3 -c "from scipy.stats import wilcoxon; print('ok')"`. If it fails, add `python312Packages.scipy` (or equivalent) to the benchmark Python environment in `flake.nix` and rebuild with `nix develop`.
- [ ] T002 [P] Verify that the release-build polybench binary exists and lists benchmarks: `<polybench_binary> --benchmark_list_tests --benchmark_filter='g:polybench'` must print ≥ 90 names.
- [ ] T003 [P] Verify that the release-build TPC-H binary exists and lists benchmarks: `<tpch_binary> --benchmark_list_tests --benchmark_filter='g:tpch'` must print ≥ 9 names (3 kernels × 3 phases).

**Checkpoint**: Environment is clean. All three verifications pass.

---

## Phase 2: Foundational (Shared Infrastructure + Smoke Test)

**Purpose**: The new DuckDB schema, runner scripts, and a smoke test must all be complete
before any full-scale experiment run. The smoke test is the hard gate.

**⚠️ CRITICAL**: No full-scale experiment run can begin until the smoke test (T010) passes.

- [ ] T004 Add `ablation_studies` table, `v_ablation_results` view, and `v_ablation_medians` view to `benchmarks/create_db.py` per data-model.md. Also add these to the schema initialisation block in `benchmarks/record_benchmark.py` (the `open_db` function path). SQL definitions are in `specs/007-jit-pipeline-eval/data-model.md`.

- [ ] T005 Implement `benchmarks/ablation_benchmarks.py` per the contract in `specs/007-jit-pipeline-eval/plan.md` (Phase 1 → Contracts section). The script must: (a) accept BINARY, --db, --study-name, --configs, --benchmark-filter, --reps, --timeout; (b) inject env vars from each config's `env` dict into the subprocess environment; (c) run the binary once per config per rep; (d) parse benchmark JSON output; (e) write rows to `context`, `benchmarks`, and `ablation_studies` tables; (f) print a per-config summary line. Built-in config list: the 8 named configs in data-model.md (not including `workload_optimal` which is dynamic). The `workload_optimal` config is added via a `--extra-config JSON_STR` flag that appends one more config object at runtime.

- [ ] T006 [P] Implement `benchmarks/sensitivity_analysis.py` per the contract in `specs/007-jit-pipeline-eval/plan.md`. This is a thin wrapper around `ablation_benchmarks.py`'s core data-store functions: (a) read `--optimal-config PATH` (JSON file with params); (b) for `--param NAME`, generate OAT config objects (one per sweep value from `--sweep-values`); (c) delegate to the same `run_trial` + `store_to_db` functions as `ablation_benchmarks.py`. Config names follow the pattern `sens_{param}_{value}` stored in `ablation_studies.config_name`.

- [ ] T007 Create `benchmarks/eval_smoke.duckdb` for the smoke test: `cd benchmarks && python3 create_db.py --db eval_smoke.duckdb`. Verify the new `ablation_studies` table exists: `python3 -c "import duckdb; c=duckdb.connect('benchmarks/eval_smoke.duckdb'); print(c.execute('SELECT * FROM ablation_studies LIMIT 0').description)"`.

- [ ] T008 Run the Experiment A smoke component: `python3 benchmarks/optimize_benchmarks.py <polybench_binary> --db benchmarks/eval_smoke.duckdb --study-name smoke_polybench_A --n-trials 10 --seed 0 --benchmark-filter 'BM_g:polybench;n:(gemm|correlation|lu);s:MINI;t:(jit_overhead|specialized_exec)' --apply-default-filters`. Verify: `v_optim_best_per_kernel WHERE study_name='smoke_polybench_A'` returns rows for 3 kernels; no `used_timeout_fallback=TRUE` rows.

- [ ] T009 Run the Experiment B smoke component: `python3 benchmarks/ablation_benchmarks.py <polybench_binary> --db benchmarks/eval_smoke.duckdb --study-name smoke_ablation --configs benchmarks/configs/ablation_subset.json --benchmark-filter 'BM_g:polybench;n:(gemm|correlation);s:MINI;t:(jit_overhead|specialized_exec|unspecialized)' --reps 3` where `ablation_subset.json` contains only 3 configs: `[default, o3_only, no_unroll]`. Create `benchmarks/configs/ablation_subset.json` as part of this task. Verify: `v_ablation_medians WHERE study_name='smoke_ablation'` returns 3 config_names × 2 kernels = 6 rows with non-null `med_jit_ns`.

- [ ] T010 Run the Experiment C smoke component (TPC-H transfer): extract the `params_json` of the best trial from `smoke_polybench_A` via `SELECT params_json FROM v_optim_best_per_kernel WHERE study_name='smoke_polybench_A' LIMIT 1`, convert to an `--extra-config` JSON string, and run `ablation_benchmarks.py` against the TPC-H binary with `--study-name smoke_transfer --benchmark-filter 'g:tpch.*t:(jit_overhead|specialized_exec|unspecialized)' --reps 1`. Verify: `v_ablation_results WHERE study_name='smoke_transfer'` has rows with `group='tpch'`. Also verify Exp D smoke: `SELECT * FROM v_optim_breakeven WHERE study_name='smoke_polybench_A'` returns rows with non-null `break_even_calls` for at least 1 kernel.

**Checkpoint**: Smoke test passes all verifications. `eval_smoke.duckdb` contains valid data for all 5 paths. Full-scale runs may now proceed.

---

## Phase 3: User Story 1 — Optimal Pipeline per Workload (Priority: P1) 🎯 MVP

**Goal**: Run Experiment A (150-trial Optuna optimization) for both workloads, extract
best configs, and produce Pareto frontier plots.

**Independent Test**: `v_optim_best_per_kernel WHERE study_name='polybench_optim_*'` returns
≥ 1 row per polybench kernel; similarly for tpch. Each row has non-null `total_ns` < Default
config's combined cost (or Default is confirmed near-optimal within 5%).

- [ ] T011 [US1] Patch `benchmarks/optimize_benchmarks.py` to extract and save Optuna parameter importance immediately after `study.optimize()` completes: call `optuna.importance.get_param_importances(study)` and write the result as JSON to `importance_<study_name>.json` in the same directory as `--output-best`. This is needed for Experiment E reporting without re-running Optuna.

- [ ] T012 [US1] Run Experiment A — polybench full optimization: `python3 benchmarks/optimize_benchmarks.py <polybench_binary> --db benchmarks/benchmarks.duckdb --study-name polybench_optim_$(date +%Y%m%d) --n-trials 150 --seed 42 --n-parallel 4 --benchmark-filter 'g:polybench.*s:EXTRALARGE.*(jit_overhead|specialized_exec)' --apply-default-filters`. Record the study name. Verify status is `complete` in `optimization_sessions`.

- [ ] T013 [P] [US1] Run Experiment A — TPC-H full optimization: `python3 benchmarks/optimize_benchmarks.py <tpch_binary> --db benchmarks/benchmarks.duckdb --study-name tpch_optim_$(date +%Y%m%d) --n-trials 150 --seed 42 --n-parallel 2 --benchmark-filter 'g:tpch.*(jit_overhead|specialized_exec)' --apply-default-filters`. Record the study name. Verify status is `complete`.

- [ ] T014 [US1] Extract per-workload best configs: query `v_optim_best_per_kernel` for both study names and write the results to `benchmarks/results/polybench_best_config.json` and `benchmarks/results/tpch_best_config.json`. Also write a `benchmarks/results/polybench_workload_optimal.json` in the `--extra-config`-compatible format (single-element list with `{"name": "workload_optimal", "env": {...}}`).

- [ ] T015 [US1] Create `benchmarks/reporting/plot_pareto.py`: reads `v_optim_breakeven` for a given study name from `benchmarks.duckdb`, plots a scatter of `(t_jit_ns, t_spec_ns)` per trial per kernel, marks `Options::Default()` reference point, draws the Pareto frontier line per kernel. Saves one plot per kernel group to `benchmarks/results/pareto_<study_name>.png`. CLI: `plot_pareto.py --db PATH --study-name STR [--output-dir DIR]`.

- [ ] T016 [US1] Generate polybench + TPC-H Pareto plots by running `plot_pareto.py` for both study names. Verify plots are written to `benchmarks/results/` and contain the Default reference marker.

**Checkpoint**: Optimal configs for both workloads are identified and stored. Pareto plots ready for thesis.

---

## Phase 4: User Story 2 — Ablation Study (Priority: P2)

**Goal**: Run Experiment B (8+1 named ablation configs, 3 reps each) for polybench
EXTRALARGE. Produce per-config speedup bar charts.

**Independent Test**: `v_ablation_medians WHERE study_name='ablation_polybench_*'` returns
9 config_names × 30 kernels = 270 rows. The `o3_only` config has lower exec speedup than
`default` for the majority of kernels.

- [ ] T017 [US2] Create `benchmarks/configs/ablation_polybench.json` listing all 9 configs (8 built-in + `workload_optimal` whose `env` dict is read from `benchmarks/results/polybench_workload_optimal.json` produced in T014). Format: JSON array of `{"name": str, "env": {str: str}}`.

- [ ] T018 [US2] Run Experiment B — polybench ablation: `python3 benchmarks/ablation_benchmarks.py <polybench_binary> --db benchmarks/benchmarks.duckdb --study-name ablation_polybench_$(date +%Y%m%d) --configs benchmarks/configs/ablation_polybench.json --benchmark-filter 'g:polybench.*s:EXTRALARGE.*(jit_overhead|specialized_exec|unspecialized)' --reps 3`. Verify: 9 configs × 3 reps × 30 kernels × 3 phases = 2430 rows in `benchmarks` (approximate; exact count depends on which phases matched).

- [ ] T019 [US2] Create `benchmarks/reporting/plot_ablation.py`: reads `v_ablation_medians JOIN v_ablation_results` for a given study, computes exec speedup ratio = `med_unspec_ns / med_spec_ns` and JIT overhead ratio = `med_jit_ns / (unspec_ns for default config)`, generates a grouped bar chart per kernel (x-axis: config, y-axis: exec speedup). Saves to `benchmarks/results/ablation_<study_name>.png`. CLI: `plot_ablation.py --db PATH --study-name STR [--output-dir DIR]`.

- [ ] T020 [US2] Generate ablation bar charts by running `plot_ablation.py` for the ablation study. Verify that the `workload_optimal` config bar is the tallest (highest exec speedup) or tied with `aggressive`; `o3_only` bar is the shortest.

**Checkpoint**: Ablation analysis complete. Bar charts ready for thesis.

---

## Phase 5: User Story 3 — Cross-Workload Config Transfer (Priority: P2)

**Goal**: Run Experiment C (4 configs × 2 workloads, 3 reps). Apply Wilcoxon rank-sum test
to determine whether the optimal config is workload-specific.

**Independent Test**: `v_ablation_medians WHERE study_name='transfer_*'` returns 4 config_names
× both groups' kernels. A Wilcoxon test script produces a p-value; the thesis conclusion
(workload-specific vs. universal) follows from whether p < 0.05.

- [ ] T021 [US3] Create `benchmarks/configs/transfer_configs.json` listing 4 configs:
  `default` (no overrides), `aggressive` (as in ablation), `polybench_optimal` (env from
  `polybench_best_config.json`), `tpch_optimal` (env from `tpch_best_config.json`).

- [ ] T022 [US3] Run Experiment C — polybench under foreign configs: `python3 benchmarks/ablation_benchmarks.py <polybench_binary> --db benchmarks/benchmarks.duckdb --study-name transfer_polybench_$(date +%Y%m%d) --configs benchmarks/configs/transfer_configs.json --benchmark-filter 'g:polybench.*s:EXTRALARGE.*(jit_overhead|specialized_exec|unspecialized)' --reps 3`.

- [ ] T023 [P] [US3] Run Experiment C — TPC-H under foreign configs: `python3 benchmarks/ablation_benchmarks.py <tpch_binary> --db benchmarks/benchmarks.duckdb --study-name transfer_tpch_$(date +%Y%m%d) --configs benchmarks/configs/transfer_configs.json --benchmark-filter 'g:tpch.*(jit_overhead|specialized_exec|unspecialized)' --reps 3`.

- [ ] T024 [US3] Implement Wilcoxon test in `benchmarks/reporting/wilcoxon_transfer.py`: reads `v_ablation_medians` for both transfer studies, computes per-kernel combined cost (`med_jit_ns + med_spec_ns`) for each config, runs `scipy.stats.wilcoxon(own_optimal_costs, foreign_optimal_costs)` across kernels, and prints: p-value, conclusion (significant/not-significant at α=0.05), raw degradation %, and a summary table. CLI: `wilcoxon_transfer.py --db PATH --polybench-study STR --tpch-study STR`.

- [ ] T025 [US3] Run `wilcoxon_transfer.py` for both transfer studies. Record the p-values and conclusion in `benchmarks/results/transfer_summary.md` (one paragraph: p-value, conclusion, degradation %).

**Checkpoint**: Cross-workload transfer analysis complete. Statistical conclusion ready for thesis.

---

## Phase 6: User Story 4 — Break-Even and Sensitivity Analysis (Priority: P3)

**Goal**: Run Experiment D (DuckDB query only) and Experiment E (OAT sensitivity sweep).
Produce break-even distribution plots and OAT sensitivity charts.

**Independent Test**: Break-even plot shows `break_even_calls` distribution for polybench optimal
study. Sensitivity plots show non-flat curves for at least 2 of the 6 parameters.

- [ ] T026 [US4] Create `benchmarks/reporting/plot_breakeven.py`: reads `v_optim_breakeven WHERE study_name=STR AND break_even_calls IS NOT NULL`, generates a box plot of break-even call counts per kernel (sorted by median), annotates kernels where `break_even_calls > 1000` as "rarely worth it", saves to `benchmarks/results/breakeven_<study_name>.png`. CLI: `plot_breakeven.py --db PATH --study-name STR [--output-dir DIR]`.

- [ ] T027 [US4] Generate break-even plots for the polybench optimization study (from T012). Verify the plot contains ≥ 20 kernels and identifies at least 1 kernel with infinite break-even (no speedup).

- [ ] T028 [US4] Run Experiment E — sensitivity analysis for all 6 parameters. For each parameter, invoke: `python3 benchmarks/sensitivity_analysis.py <polybench_binary> --db benchmarks/benchmarks.duckdb --study-name sens_polybench_$(date +%Y%m%d) --optimal-config benchmarks/results/polybench_best_config.json --param PARAM --sweep-values VALUES --benchmark-filter 'g:polybench.*s:EXTRALARGE.*(jit_overhead|specialized_exec|unspecialized)' --reps 1`. Sweep values per parameter are defined in `data-model.md`. Run all 6 parameters sequentially (one study name used for all, config_name encodes parameter and value).

- [ ] T029 [US4] Create `benchmarks/reporting/plot_sensitivity.py`: reads `v_ablation_medians WHERE study_name='sens_polybench_*'`, groups rows by `config_name` prefix (param name), computes geomean combined cost per sweep value across all kernels, plots one line per parameter as a function of sweep value (normalized to the optimal point). Also overlays Optuna importance scores from `importance_<polybench_study>.json`. Saves to `benchmarks/results/sensitivity_<study_name>.png`. CLI: `plot_sensitivity.py --db PATH --study-name STR --importance-json PATH [--output-dir DIR]`.

- [ ] T030 [US4] Generate sensitivity plots and verify at least 2 parameters show >10% cost variation across their sweep range (i.e., the parameter is "sensitive"). Document which parameters are below 5% variation as "insensitive" in `benchmarks/results/sensitivity_summary.md`.

**Checkpoint**: Break-even and sensitivity analysis complete. All US4 outputs ready for thesis.

---

## Phase 7: Polish & Orchestration

**Purpose**: Create the orchestration script, verify all results are self-consistent,
and ensure outputs are reproducible from a single command.

- [ ] T031 Create `benchmarks/run_evaluation.sh` per the contract in `specs/007-jit-pipeline-eval/plan.md`. The script accepts `<polybench_binary> <tpch_binary> [--smoke-only] [--db PATH]` and runs all phases in order: smoke test → Exp A → Exp B → Exp C → Exp E → reporting. It must log the study names generated in each phase to `benchmarks/results/eval_run_log.txt`. On smoke failure it must exit non-zero before any full-scale runs.

- [ ] T032 [P] Create `benchmarks/results/README.md` documenting: the DuckDB study names used, the machine description (CPU model, RAM, OS), the git SHA of the binary used, and a one-line description of each output file. This is the reproducibility record for the thesis.

- [ ] T033 Validate end-to-end: run `python3 -c "import duckdb; c=duckdb.connect('benchmarks/benchmarks.duckdb'); print(c.execute('SELECT COUNT(*) FROM context').fetchone())"` and confirm all expected study names are present in `optimization_sessions` and `ablation_studies`. Confirm no NULL `run_id` in `ablation_studies`.

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: No dependencies — start immediately
- **Phase 2 (Foundational)**: Depends on Phase 1 — T004 and T005 must complete before T007–T010
- **T007–T010 (Smoke test)**: Must all pass before any Phase 3–6 task begins
- **Phase 3 (US1 — Exp A)**: Depends on smoke test passing; T012 and T013 can run in parallel
- **Phase 4 (US2 — Exp B)**: Depends on T014 (polybench best config extracted)
- **Phase 5 (US3 — Exp C)**: Depends on T014 (both best configs extracted)
- **Phase 6 (US4 — Exp D+E)**: Depends on T012 (polybench study name) and T014 (optimal config)
- **Phase 7 (Polish)**: Depends on all Phase 3–6 tasks completing

### User Story Dependencies

- **US1 (P1)**: Depends only on Phase 2 smoke test. No dependencies on other user stories.
- **US2 (P2)**: Depends on T014 (polybench best config from US1).
- **US3 (P2)**: Depends on T014 (both best configs from US1). Can run in parallel with US2.
- **US4 (P3)**: Depends on T012 (polybench study) and T014 (optimal config). Can run after US1.

### Parallel Opportunities Within Phases

- T002 + T003 (Phase 1): both are verification only, run in parallel
- T005 + T006 (Phase 2): implement `ablation_benchmarks.py` and `sensitivity_analysis.py` in parallel
- T012 + T013 (Phase 3): polybench and TPC-H Optuna runs in parallel
- T022 + T023 (Phase 5): polybench and TPC-H transfer runs in parallel

---

## Parallel Execution Examples

### Phase 3 (Experiment A)

```bash
# Run both optimization studies in parallel (different binaries, no DB conflict)
python3 benchmarks/optimize_benchmarks.py <polybench_binary> --study-name polybench_optim_... &
python3 benchmarks/optimize_benchmarks.py <tpch_binary>      --study-name tpch_optim_...     &
wait
```

*Note: Both write to the same DuckDB file — `optimize_benchmarks.py` uses a threading lock per trial write, but running two separate processes against the same DuckDB file may cause write conflicts. If conflicts occur, run sequentially or use separate DB files and merge.*

### Phase 5 (Experiment C)

```bash
# Run both transfer studies in parallel (different binaries)
python3 benchmarks/ablation_benchmarks.py <polybench_binary> --study-name transfer_polybench_... &
python3 benchmarks/ablation_benchmarks.py <tpch_binary>      --study-name transfer_tpch_...     &
wait
```

---

## Implementation Strategy

### MVP: User Story 1 Only (Experiment A + Pareto plots)

1. Complete Phase 1 (env setup)
2. Complete Phase 2 up to T010 (smoke test passes)
3. Complete T011–T016 (US1: Exp A runs + Pareto plots)
4. **STOP and validate**: `v_optim_best_per_kernel` returns a clear winner per workload
5. This alone answers the core thesis question with quantitative evidence

### Incremental Delivery

1. Phase 1 + 2 → Infrastructure ready, smoke test passes
2. Phase 3 (US1) → Optimal configs known, Pareto plots done → *MVP complete*
3. Phase 4 (US2) → Ablation explains *why* the optimal config works
4. Phase 5 (US3) → Transfer experiment answers workload-specificity question
5. Phase 6 (US4) → Break-even and sensitivity fill in the remaining thesis figures

---

## Notes

- [P] tasks operate on different files or different binaries and have no conflicting writes
- Study names include a datestamp suffix (e.g., `polybench_optim_20260510`) — record these in T032
- DuckDB concurrent writes from two processes: use sequential runs or separate DBs + merge if conflicts arise
- All timing runs must be on the release build with CPU scaling disabled
- The smoke test is a hard gate: if T008–T010 fail, debug the infrastructure before running expensive full experiments
