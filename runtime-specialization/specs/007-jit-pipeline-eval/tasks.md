# Tasks: JIT Pipeline Evaluation — Iteration Loop

**Input**: Design documents from `specs/007-jit-pipeline-eval/`
**Prerequisites**: plan.md ✅, spec.md ✅, research.md ✅, data-model.md ✅

**Organization**: Tasks are grouped by user story. Phase 2 (Foundational + Smoke Test) is
a hard gate — no user-story work begins until it passes. Experiments run sequentially
within the iteration: A → B → C → D → E → F (Reflection).

> **2026-05-20 path migration:** Output artifacts now live in the per-run folder
> `benchmarks/reports/<YYMMDD-HH-MM>-optimize-pipeline/` (auto-created by
> `run_evaluation.sh`). The `benchmarks/results/` paths referenced in already-completed
> `[X]` task descriptions below refer to iteration-1 outputs that were subsequently
> relocated to `benchmarks/reports/260520-13-00-optimize-pipeline/`. Reflection files in
> past runs are discoverable as `ls -d benchmarks/reports/*-optimize-pipeline*/` →
> `reflection*.md`. Going forward, the canonical filename is `reflection.md`.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no concurrent dependencies)
- **[Story]**: User story the task belongs to (US1–US5)

---

## Phase 1: Setup

**Purpose**: Environment verification and binary availability checks.

- [X] T001 Verify `scipy.stats.wilcoxon` is available in the Nix environment: `python3 -c "from scipy.stats import wilcoxon; print('ok')"`. If it fails, add `python312Packages.scipy` (or equivalent) to the benchmark Python environment in `flake.nix` and rebuild with `nix develop`.
- [X] T002 [P] Verify the release-build UC binary exists and lists benchmarks: `<uc_binary> --benchmark_list_tests --benchmark_filter='g:(uc1|uc2|uc7|uc8|uc12|uc14)'` must print ≥ 18 names (6 groups × 3 phases).
- [X] T003 [P] Verify the release-build TPC-H binary exists and lists benchmarks: SKIPPED — no TPC-H binary in project (RQ-12). Exp C skipped; documented as future work.

**Checkpoint**: Environment verified. All three checks pass.

---

## Phase 2: Foundational (Shared Infrastructure + Smoke Test)

**Purpose**: New DuckDB schema, runner scripts, and a smoke test. The smoke test is the
hard gate — no experiment run begins until it passes.

**⚠️ CRITICAL**: No full-scale experiment run can begin until T010 passes.

- [X] T004 Add `ablation_studies` table, `v_ablation_results` view, and `v_ablation_medians` view to `benchmarks/create_db.py` per `specs/007-jit-pipeline-eval/data-model.md`. Also add these to the schema initialisation block in `benchmarks/record_benchmark.py` (the `open_db` function path). SQL definitions are in `data-model.md`.

- [X] T005 Implement `benchmarks/ablation_benchmarks.py` per the contract in `specs/007-jit-pipeline-eval/plan.md` (Phase 1 → Contracts section). The script must: (a) accept BINARY, --db, --study-name, --configs, --extra-config, --benchmark-filter, --reps, --timeout; (b) inject env vars from each config's `env` dict into the subprocess environment; (c) run the binary once per config per rep; (d) parse benchmark JSON output; (e) write rows to `context`, `benchmarks`, and `ablation_studies` tables; (f) print a per-config summary line. Built-in config list: the 8 named configs in `data-model.md`. The `uc_workload_optimal` config is appended via `--extra-config JSON_STR` at runtime.

- [X] T006 [P] Implement `benchmarks/sensitivity_analysis.py` per the contract in `specs/007-jit-pipeline-eval/plan.md`. This is a thin wrapper around `ablation_benchmarks.py`'s core data-store functions: (a) read `--optimal-config PATH` (JSON file with params); (b) for `--param NAME`, generate OAT config objects (one per sweep value from `--sweep-values`); (c) delegate to the same `run_trial` + `store_to_db` functions as `ablation_benchmarks.py`. Config names follow the pattern `sens_{param}_{value}` in `ablation_studies.config_name`.

- [X] T007 Create `benchmarks/eval_smoke.duckdb` for the smoke test: `cd benchmarks && python3 create_db.py --db eval_smoke.duckdb`. Verify the new `ablation_studies` table exists: `python3 -c "import duckdb; c=duckdb.connect('benchmarks/eval_smoke.duckdb'); print(c.execute('SELECT * FROM ablation_studies LIMIT 0').description)"`.

- [X] T008 Run the Experiment A smoke component: `python3 benchmarks/optimize_benchmarks.py <uc_binary> --db benchmarks/eval_smoke.duckdb --study-name smoke_uc_A --n-trials 10 --seed 0 --benchmark-filter 'BM_g:(uc7|uc8);.*s:MINI;.*t:(jit_overhead|specialized_exec)' --apply-default-filters`. Verify: `v_optim_best_per_kernel WHERE study_name='smoke_uc_A'` returns rows for 2 groups; no `used_timeout_fallback=TRUE` rows.

- [X] T009 Run the Experiment B smoke component: `python3 benchmarks/ablation_benchmarks.py <uc_binary> --db benchmarks/eval_smoke.duckdb --study-name smoke_ablation --configs benchmarks/configs/ablation_subset.json --benchmark-filter 'BM_g:(uc7|uc8);.*s:MINI;.*t:(jit_overhead|specialized_exec|unspecialized)' --reps 3` where `ablation_subset.json` contains 3 configs: `[default, o3_only, no_unroll]`. Create `benchmarks/configs/ablation_subset.json` as part of this task. Verify: `v_ablation_medians WHERE study_name='smoke_ablation'` returns 3 config_names × 2 groups = 6 rows with non-null `med_jit_ns`.

- [X] T010 Run the Experiment C smoke component (TPC-H transfer): SKIPPED — no TPC-H binary (RQ-12). Exp D smoke verified: `SELECT * FROM v_optim_breakeven WHERE study_name='smoke_uc_A_v4'` returns rows with non-null break_even_calls.

**Checkpoint**: Smoke test passes all verifications. `eval_smoke.duckdb` contains valid data for all 5 paths. Full-scale experiments may now proceed.

---

## Phase 3: User Story 1 — Run Optimizer, Identify Best Config (Priority: P1) 🎯 MVP

**Goal**: Run `optimize_benchmarks.py` on UC benchmarks (MEDIUM size, 150 trials) to
find the best pipeline config. Extract and save it for downstream experiments.

**Independent Test**: `v_optim_best_per_kernel WHERE study_name='uc_optim_*'` returns 6
rows (one per UC group). Each row has non-null `total_ns` ≤ Default config's combined cost
(or Default is confirmed near-optimal within 5%).

- [X] T011 [US1] Patch `benchmarks/optimize_benchmarks.py` to extract and save Optuna parameter importance immediately after `study.optimize()` completes: call `optuna.importance.get_param_importances(study)` and write the result as JSON to `importance_<study_name>.json` in the same directory as `--output-best`. This is needed for Exp E and the Reflection phase without re-running Optuna.

- [ ] T012 [US1] Run Experiment A — UC full optimization: `python3 benchmarks/optimize_benchmarks.py <uc_binary> --db benchmarks/benchmarks.duckdb --study-name uc_optim_20260517 --n-trials 150 --seed 42 --n-parallel 1 --benchmark-filter 'BM_g:(uc1|uc2|uc7|uc8|uc12|uc14);.*s:MEDIUM;.*t:(jit_overhead|specialized_exec)' --apply-default-filters`. **RUNNING** (22/150 trials complete as of 2026-05-17). Study name: `uc_optim_20260517`.

- [ ] T013 [US1] Extract UC best config: query `v_optim_best_per_kernel` for the study name from T012 and write the result to `<REPORT_DIR>/uc_best_config.json`. Also write `<REPORT_DIR>/uc_workload_optimal.json` in the `--extra-config`-compatible format: a single-element JSON list with `{"name": "uc_workload_optimal", "env": {...}}` where `env` maps `CRS_DEFAULT_*` names to string values. `<REPORT_DIR>` is the per-run folder `benchmarks/reports/<YYMMDD-HH-MM>-optimize-pipeline/` (created by `run_evaluation.sh`). **PRELIMINARY** versions created using 22-trial best.

- [X] T014 [US1] Create `benchmarks/reporting/plot_pareto.py`: reads `v_optim_breakeven` for a given study name from `benchmarks.duckdb`, plots a scatter of `(t_jit_ns, t_spec_ns)` per trial per group, marks `Options::Default()` as a reference point, draws the Pareto frontier line per group. Saves one PNG per benchmark group to `benchmarks/results/pareto_<study_name>/`. CLI: `plot_pareto.py --db PATH --study-name STR [--output-dir DIR]`. **BUG**: frontier was rendered as upper-right envelope (maximize) instead of lower-left (minimize) — fixed in T034.

- [X] T015 [US1] Generate UC Pareto plots: 6 PNGs in `benchmarks/results/pareto_uc_optim_20260517/` (one per UC group). Note: generated from partial T012 data (24 trials); re-run when T012 completes for final version, and after T034 fix.

**Checkpoint**: Optimal UC config identified and stored. Pareto plots ready for thesis.

---

## Phase 3.5: Pareto reporting fixes & thesis-ready CSV export (US6, P2)

These tasks were added during iteration 1 after discovering that `plot_pareto.py` rendered
the Pareto frontier in the upper-right (maximize-maximize) instead of the lower-left
(minimize-minimize). They also introduce the thesis-ready Pareto reporting script
described in US6 / FR-002b.

- [X] T034 [US6] **Fix Pareto direction** in `benchmarks/reporting/plot_pareto.py`. Replaced inverted dominance check (was finding upper-right envelope) with: a point `p` is dominated iff some other point has all coords `<= p` AND at least one strictly less. Regenerated 6 per-group PNGs; Pareto polyline now traces lower-left envelope.

- [X] T035 [US6] **Implement `benchmarks/reporting/plot_pareto_configs.py`** per FR-002b. CLI: `--db PATH --study-name STR --output-dir DIR [--per-group]`. Reads both `optim_trial_params` and `ablation_studies` (joined with `v_ratios`). Normalizes env-var keys (`CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS` → `fixpoint_max`, etc.) and fills missing keys with `Options::Default()` values. Outputs PNG (scatter + star for default + polyline through Pareto-optimal points) and CSV (FR-002b columns).

- [X] T036 [US6] **Regenerate thesis Pareto outputs** for `uc_optim_20260517` and `ablation_uc_20260517`. Outputs:
  - `pareto_uc_optim_20260517[_<group>].{png,csv}` (combined + 6 per-group)
  - `pareto_ablation_uc_20260517[_<group>].{png,csv}` (combined + 6 per-group)
  Verified: Pareto rows sorted by ascending `jit_overhead_ms` have monotone non-increasing `specialized_exec_ms`; default rows flagged for each (group, kernel).

---

## Phase 4: User Story 2 — Ablation: Identify Which Passes Drive Speedup (Priority: P2)

**Goal**: Run 9 named pipeline configs × 3 reps on UC MEDIUM. Produce per-config speedup
bar charts to explain which pipeline stages matter.

**Independent Test**: `v_ablation_medians WHERE study_name='ablation_uc_*'` returns 9
config_names × 6 groups = 54 rows. The `o3_only` config has lower exec speedup than
`default` for the majority of groups.

- [X] T016 [US2] Create `benchmarks/configs/ablation_uc.json` listing 8 configs: default, o3_only, no_prune, no_o3_final, no_unroll, fixpoint_2 (replacing fixpoint_1 — RQ-15 SIGSEGV), aggressive, uc_workload_optimal. pipeline_1 excluded (RQ-11). File: `benchmarks/configs/ablation_uc.json`.

- [X] T017 [US2] Run Experiment B — UC ablation: completed. 24/24 reps OK (8 configs × 3 reps), all stored in `ablation_studies` study `ablation_uc_20260517`. `uc_workload_optimal` config used preliminary trial-17 params (fixpoint_max=7, unroll_max=4, large_module_max=0, early_prune=1, o3_final=1).

- [X] T018 [US2] Create `benchmarks/reporting/plot_ablation.py`: reads `v_ablation_medians` for a given study, computes exec speedup = `med_unspec_ns / med_spec_ns` and JIT overhead relative to Default, generates a grouped bar chart per group (x-axis: config, y-axis: exec speedup). Saves to `benchmarks/results/ablation_<study_name>.png`. CLI: `plot_ablation.py --db PATH --study-name STR [--output-dir DIR]`.

- [X] T019 [US2] Generate ablation bar charts: `benchmarks/results/ablation_ablation_uc_20260517.png`.

**Checkpoint**: Ablation analysis complete. Bar charts ready for thesis.

---

## Phase 5: User Story 3 — Cross-Workload Config Transfer (Priority: P2)

**Goal**: Apply the UC-optimal config to TPC-H. Determine whether a single universal config
suffices or workload-specific tuning is needed.

**Independent Test**: `v_ablation_medians WHERE study_name='transfer_*'` returns 4 config_names
for both workloads. A Wilcoxon test produces a p-value; the thesis conclusion follows from
whether p < 0.05.

- [X] T020 [US3] SKIPPED — no TPC-H binary available (RQ-12). Exp C skipped. Documented as future work.
- [X] T021 [US3] SKIPPED — no TPC-H binary (RQ-12).
- [X] T022 [US3] SKIPPED — no TPC-H binary (RQ-12).
- [X] T023 [US3] SKIPPED — no TPC-H binary (RQ-12).
- [X] T024 [US3] SKIPPED — no TPC-H binary (RQ-12).

**Checkpoint**: Cross-workload transfer analysis complete. Statistical conclusion documented.

---

## Phase 6: User Story 4 — Break-Even and Sensitivity Analysis (Priority: P3)

**Goal**: Compute break-even call counts for all UC groups (Exp D, no new runs). Run OAT
sensitivity sweep over all 6 parameters on UC MEDIUM (Exp E). Produce plots for thesis.

**Independent Test**: Break-even plot shows `break_even_calls` distribution for the UC
optimal study. Sensitivity plots show non-flat curves for ≥ 2 of the 6 parameters.

- [X] T025 [US4] Create `benchmarks/reporting/plot_breakeven.py`: reads `v_optim_breakeven WHERE study_name=STR AND break_even_calls IS NOT NULL`, generates a box plot of break-even call counts per group (sorted by median), annotates groups where `break_even_calls > 100` as "rarely worth it", saves to `benchmarks/results/breakeven_<study_name>.png`. CLI: `plot_breakeven.py --db PATH --study-name STR [--output-dir DIR]`.

- [X] T026 [US4] Generate break-even plots for the UC optimization study (from T012): Output `benchmarks/results/breakeven_uc_optim_20260517.png`. Regenerated with 24-trial data; re-run when T012 completes.

- [X] T027 [US4] Run Experiment E — sensitivity sweep complete. Study: `sens_uc_20260517`. 26 sweep points (5 params, fixpoint_max=1 → NULL row from SIGSEGV).

- [X] T028 [US4] Create `benchmarks/reporting/plot_sensitivity.py`: reads `v_ablation_medians WHERE study_name='sens_uc_*'`, groups rows by `config_name` prefix (param name), computes geomean combined cost per sweep value across all UC groups, plots one line per parameter normalised to the optimal point, overlays Optuna importance scores from `importance_<study>.json`. Saves to `benchmarks/results/sensitivity_<study_name>.png`. CLI: `plot_sensitivity.py --db PATH --study-name STR --importance-json PATH [--output-dir DIR]`.

- [X] T029 [US4] Generate sensitivity plots (`benchmarks/results/sensitivity_sens_uc_20260517.png`) and `benchmarks/results/sensitivity_summary.md`. Findings: all 5 params have ratio<1.10 in the geomean across kernels — `o3_final` is the most sensitive at 1.09, others 1.02-1.05. The optimization is robust within the search space; per-kernel ratios for small kernels show much larger effects (e.g. `early_prune=0` gave one kernel jit=2527ms vs 51ms).

**Checkpoint**: Break-even and sensitivity analyses complete. All quantitative outputs ready for the reflection.

---

## Phase 7: User Story 5 — Reflection: Infrastructure and Process Improvements (Priority: P2)

**Goal**: Write a structured reflection document that captures what was learned about the
*evaluation process itself* — producing actionable inputs for the next iteration.

**Independent Test**: `benchmarks/results/reflection_iteration1.md` exists and contains ≥ 1
concrete actionable improvement under each of: infrastructure, evaluation process, and
next-iteration scope.

- [X] T030 [US5] Write `benchmarks/results/reflection_iteration1.md` covering three mandatory scopes:

  **1. Infrastructure improvements** — cite specific results and propose concrete changes to:
  - `optimize_benchmarks.py`: e.g., multi-objective Optuna, warm-start from previous study, richer timeout handling
  - UC benchmark binaries: gaps in coverage, missing use cases, grouping/filter improvements
  - DuckDB schema / tooling: schema gaps, query ergonomics, missing views

  **2. Evaluation process improvements** — cite specific results and propose:
  - Trial budget: was 150 sufficient? (cite convergence plot from Exp A) — propose budget for next iteration
  - Repetition count: was 3 reps enough for ablation/transfer? (cite variance across reps)
  - Size selection: was MEDIUM representative? (any group where MEDIUM felt atypical)
  - Statistical approach: where was the Wilcoxon test borderline? what would strengthen it?

  **3. Next-iteration scope** — based on sensitivity results (T029), ablation (T019), and transfer (T024):
  - Parameters to prune from search space (insensitive per T029)
  - Parameters needing finer resolution (high-sensitivity per T029)
  - Experiments to skip, repeat, or add in next iteration
  - Proposed trial budget and workload additions for next iteration

  Each claim in the reflection MUST cite the study name, query, or plot that supports it.

**Checkpoint**: Reflection complete. The evaluation loop is closed. All iteration-1 outputs are ready for thesis integration.

---

## Phase 8: Polish & Orchestration

**Purpose**: Orchestration script, reproducibility record, end-to-end validation.

- [X] T031 Create `benchmarks/run_evaluation.sh` per the contract in `specs/007-jit-pipeline-eval/plan.md`. The script accepts `<uc_binary> <tpch_binary> [--smoke-only] [--db PATH]` and runs all phases in order: smoke → Exp A → Exp B → Exp C → Exp E → (manual) Exp F. Logs study names to `benchmarks/results/eval_run_log.txt`. Exits non-zero on smoke failure before any full-scale runs. Fixed: fixpoint_max sweep starts at 1 (not 0), o3_only uses fixpoint_max=1+early_prune=0, pipeline sweep removed, fixpoint_1 → fixpoint_2.

- [X] T032 [P] Create `benchmarks/results/README.md` documenting: the DuckDB study names used in this iteration, the machine description (CPU model, RAM, OS), git SHA of the binary, and a one-line description of each output file. This is the reproducibility record for the thesis.

- [ ] T033 Validate end-to-end: run `python3 -c "import duckdb; c=duckdb.connect('benchmarks/benchmarks.duckdb'); print(c.execute('SELECT study_name, COUNT(*) FROM context GROUP BY 1').fetchall())"` and confirm all expected study names are present. Confirm no NULL `run_id` in `ablation_studies`. Confirm `<REPORT_DIR>/reflection.md` exists in the latest `benchmarks/reports/<YYMMDD-HH-MM>-optimize-pipeline/` folder. **Pending T012, T017, T027, T030.**

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: No dependencies — start immediately
- **Phase 2 (Foundational)**: Depends on Phase 1; T004 and T005 must complete before T007–T010
- **T007–T010 (Smoke test)**: Must all pass before any Phase 3–7 task begins
- **Phase 3 (US1 — Exp A)**: Depends on smoke test passing
- **Phase 4 (US2 — Exp B)**: Depends on T013 (UC best config extracted)
- **Phase 5 (US3 — Exp C)**: Depends on T013 (UC best config); T021 and T022 can run in parallel
- **Phase 6 (US4 — Exp D+E)**: Depends on T012 (UC study name) and T013 (optimal config)
- **Phase 7 (US5 — Reflection)**: Depends on all Phase 3–6 tasks completing — reflection is grounded in data
- **Phase 8 (Polish)**: Depends on all Phase 3–7 tasks completing

### User Story Dependencies

- **US1 (P1)**: Depends only on smoke test. No dependencies on other user stories.
- **US2 (P2)**: Depends on T013 (UC best config from US1).
- **US3 (P2)**: Depends on T013 (UC best config from US1). Can run in parallel with US2.
- **US4 (P3)**: Depends on T012 (UC study) and T013 (UC optimal config). Can run after US1.
- **US5 (P2 reflection)**: Depends on US1–US4 all complete. Reflection is only meaningful with full data.

### Parallel Opportunities Within Phases

- T002 + T003 (Phase 1): verification only, run in parallel
- T005 + T006 (Phase 2): implement `ablation_benchmarks.py` and `sensitivity_analysis.py` in parallel
- T021 + T022 (Phase 5): UC and TPC-H transfer runs in parallel

---

## Parallel Execution Examples

### Phase 2 (Script Implementation)

```bash
# Implement ablation and sensitivity scripts in parallel:
Task: "Implement benchmarks/ablation_benchmarks.py"   # T005
Task: "Implement benchmarks/sensitivity_analysis.py"  # T006
```

### Phase 5 (Experiment C — Transfer)

```bash
# Run both transfer studies in parallel (different binaries):
python3 benchmarks/ablation_benchmarks.py <uc_binary>   --study-name transfer_uc_...   &  # T021
python3 benchmarks/ablation_benchmarks.py <tpch_binary> --study-name transfer_tpch_... &  # T022
wait
```

---

## Implementation Strategy

### MVP: User Story 1 Only (Exp A + Best Config + Pareto plots)

1. Complete Phase 1 (env setup)
2. Complete Phase 2 up to T010 (smoke test passes)
3. Complete T011–T015 (US1: Exp A + Pareto plots)
4. **STOP and validate**: `v_optim_best_per_kernel` returns a clear winner per UC group
5. This alone answers the core thesis question with quantitative evidence

### Full Iteration Delivery

1. Phase 1 + 2 → Infrastructure ready, smoke test passes
2. Phase 3 (US1) → Optimal config known, Pareto plots done → *MVP complete*
3. Phase 4 (US2) → Ablation explains *why* the optimal config works
4. Phase 5 (US3) → Transfer answers workload-specificity question
5. Phase 6 (US4) → Break-even and sensitivity fill remaining thesis figures
6. Phase 7 (US5) → Reflection closes the loop and seeds the next iteration
7. Phase 8 → Orchestration script, reproducibility record

---

## Notes

- [P] tasks operate on different files or different binaries and have no conflicting writes
- Study names include a datestamp suffix (e.g., `uc_optim_20260517`) — record these in T032
- DuckDB concurrent writes from two processes: use sequential runs or separate DBs + merge if conflicts arise
- All timing runs on the release build with CPU scaling disabled
- The smoke test (T010) is a hard gate: if any of T008–T010 fail, debug infrastructure before full experiments
- The Reflection (T030) is mandatory: without it the iteration loop has no feedback mechanism
