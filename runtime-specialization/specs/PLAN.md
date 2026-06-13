# Evaluation Rerun, Figure Completion, and Thesis Integration Plan

## Summary

- Scope this plan to the remaining Evaluation-chapter work tracked in `specs/TODO.md`, plus the outstanding Evaluation-section figure and snippet TODOs in `docs/thesis.typ`.
- Replace provisional UC absolute numbers with a fresh thesis-grade best-practice rerun, while keeping older named studies only where they provide data the rerun cannot reproduce.
- Generate every remaining Evaluation figure as a report artifact under `benchmarks/reports/...`, expose each one through a stable relative symlink under `docs/assets/evaluation/...`, and update the thesis text so every figure is introduced with:
  1. the source study or report,
  2. the collected data and selection/filter used,
  3. the derived metrics or aggregation used to build the figure,
  4. the intended inference for the reader.
- Update `specs/TODO.md` as part of the work so evaluation statuses are consistent with the final rerun and figure completion.

## Important Changes And Interfaces

- Use `specs/PLAN.md` as the single planning artifact for this work.
- Keep source artifacts in dated study and report directories under `benchmarks/reports/...`; do not point the thesis directly at those dated paths.
- Create stable relative symlinks under `docs/assets/evaluation/rq1/`, `rq2/`, `rq3/`, and `rq4/` with semantic names such as `rq1_pareto_representatives.png` and `rq4_optuna_convergence.png`.
- Extend or add reporting scripts under `benchmarks/reporting/` as needed:
  - extend `size_scaling.py` to emit a thesis-specific 2-panel figure,
  - add a thesis UC config-comparison plotter if `plot_ablation.py` cannot produce the required 2-panel summary,
  - add `plot_binary_size_breakdown.py`,
  - add `plot_binary_size_vs_speedup.py`,
  - add `plot_rq3_suite_family_comparison.py`,
  - add `export_rq3_abstraction_table.py`,
  - add `plot_optuna_convergence.py`.
- Treat the figure-prose contract as mandatory: no Evaluation figure is inserted without nearby prose that names source study, population/filter, metric derivation, and interpretation.

## Implementation Plan

### Cross-Cutting And Rerun

- Normalize the Evaluation-related entries in `specs/TODO.md` before execution begins, especially the inconsistent `RQ3-001` state, so the rerun and figure work has an unambiguous checklist.
- Rebuild the benchmark binaries that feed final Evaluation data before any rerun or report regeneration that depends on post-fix code.
- Run `benchmarks/run_thesis_uc_final.sh` to create a new study named `corpus_uc_final_thesis_<YYYYMMDD>` with 5 reps for `default`, `no_o3_final`, `p0_o3_optimal`, and `uc_workload_optimal`.
- Fail the rerun task unless every recorded run has `best_practice_full=TRUE`; if any best-practice control fails, stop and document the exact control that failed instead of citing the study.
- Refresh the rerun’s summary artifacts in its report directory so Setup and RQ1 can cite the final study name, repetition count, and variance/noise summary directly.
- Update the Evaluation Setup prose so final cited UC absolute numbers come only from the thesis-grade rerun, not from exploratory `powersave` runs.
- Resolve the non-figure Evaluation cleanup items that block a clean chapter pass: add the missing PolyBench citation and rename the six use-case families to reader-facing `UC1` to `UC6`.

### RQ1 And RQ5

- Generate `rq1_pareto_representatives.png` as a 2x3 composite from the existing `260610-pareto` outputs, using exactly these kernels: `count_matching_rows`, `box_filter`, `multi_pattern_match`, `apply_row_delta`, `grouped_count`, and `generic_sort`.
- In the RQ1 prose for that figure, state that it is derived from the optimization-study Pareto exports, that each panel plots per-config `jit_overhead_ms` against `specialized_exec_ms`, and that the figure illustrates config tradeoffs rather than final absolute thesis-grade numbers.
- Generate `rq1_pipeline_comparison.png` from the new thesis rerun as a 2-panel summary: left panel geomean execution speedup across the 18 UC `low` and `MEDIUM` kernels, right panel median JIT overhead across the same kernel set, for `default`, `p0_o3_optimal`, `uc_workload_optimal`, and `no_o3_final`.
- In the RQ1 prose for that figure, explicitly name the kernel filter, the number of kernels, the aggregation rule for each panel, and why this figure is the one that carries the final thesis-grade absolute comparison.
- Extend `size_scaling.py` or add a thin wrapper so it can emit a thesis-specific `rq1_size_scaling.png` with only the two required panels: raw execution speedup and first-call speedup on log scale.
- Build `rq1_size_scaling.png` from `260517-15-04-size_scaling` using exactly `box_filter`, `generic_sort`, and `multi_pattern_match`, because those are the three kernels already named in the prose.
- In the size-scaling prose, state that the figure comes from the named size-scaling study, that the x-axis is benchmark size class, that first-call speedup is computed as `t_unspec / (t_jit + t_spec)`, and that the figure is used to justify `MEDIUM` as a representative optimization point.
- Keep the RQ5 break-even table as the main artifact, but add a short derivation sentence defining `break_even_calls = ceil(t_jit / (t_unspec - t_spec))` and a sentence explaining that `0` means the first specialized call already amortizes compilation.

### RQ2

- Generate `rq2_binary_size_breakdown.png` from the existing `260610-binary-size` report as a stacked comparison of baseline vs plugin-enabled object size, explicitly separating IR blob `.rodata`, extra `.text`, extra `.data`, and remaining baseline size.
- Keep that figure scoped to the measured UC1 translation unit; do not let the prose generalize it beyond that case study.
- Resolve blocker `RQ2-002` by collecting per-kernel binary-size deltas for the 18 UC `low` and `MEDIUM` kernels, joining those deltas with the final execution-speedup data, and generating `rq2_binary_size_vs_speedup.png`.
- The per-kernel size measurement must use the same kernel granularity throughout; if any kernel cannot be isolated cleanly at object-file level, document the exclusion explicitly and exclude it from both the plot and the prose summary.
- In the RQ2 prose, clearly separate the two analyses:
  - the stacked breakdown figure is a single-translation-unit decomposition,
  - the scatter plot is a cross-kernel tradeoff view.
- For both RQ2 figures, add explicit prose describing what was compiled, how overhead was computed, how speedup was joined to size data, and what conclusion each figure supports.

### RQ3

- First verify whether `260610-polybench` was generated from the fixed dedicated `PolyBenchBenchmark` binary; if not, rebuild and rerun PolyBench before using those numbers in the thesis.
- Generate `rq3_suite_family_comparison.png` as a 3-panel composite:
  - UC panel: geomean speedup by abstraction tier from the abstraction studies.
  - PolyBench panel: per-kernel speedup distribution for `default` and `P2 optimal`.
  - SQLite/TPC-H panel: boundary-case metrics from `260610-tpch-scope` showing module size and the practical no-gain outcome.
- Use that 3-panel design intentionally so the thesis does not imply that UC, PolyBench, and SQLite/TPC-H were all measured under identical protocols.
- Generate `rq3_abstraction_detail.csv` and `rq3_abstraction_detail.typ` from the three abstraction studies, with one row per UC kernel and the row maximum emphasized.
- Link the figure and the Typst-ready table artifact into `docs/assets/evaluation/rq3/`.
- Rewrite the RQ3 prose so each panel or table is introduced with its source studies, aggregation rule, and reason for inclusion:
  - geomean for UC abstraction summary,
  - per-kernel distribution for PolyBench,
  - descriptive case-study metrics for SQLite/TPC-H.

### RQ4

- Generate `rq4_optuna_convergence.png` from `uc_optim_iter3_20260601` with raw combined objective per trial, a best-so-far curve, and a marked annotation at trial 28 where the winning configuration first appears.
- Do not add a second new thesis figure for sensitivity unless the rewritten prose cannot stand without it; the existing sensitivity report remains supporting evidence rather than a required new figure.
- Rewrite the RQ4 discussion so the convergence figure is explicitly described as optimization-search behaviour, while the comparison table remains the place for cross-study configuration results.
- Fix the mixed-unit presentation problem in the current RQ4 table and text so objective values in milliseconds are not presented as directly comparable to geomean speedups in the same result slot.

### RQ6 And Remaining Blockers Tied To The Evaluation Arc

- Replace the SQLite pseudocode-only TODO with one real code snippet from the benchmarked SQLite dispatch path, highlighting the dynamic `p->aOp[p->pc]` access.
- Add one compact snippet for `count_matching_rows` and one compact snippet for either `column_scan` or `multi_predicate` so the “specializes but does not amortize” class is argued with concrete code, not only with summary prose.
- Add the short taxonomy paragraph promised at the end of RQ6.
- Add the LLJIT boundary-condition note tied to the `TrapUnreachable=true` fix in the Discussion and RQ6 boundary narrative.
- Update `specs/TODO.md` entries for `RQ1-004`, `RQ2-002`, `RQ6-001`, `RQ6-003`, `RQ6-004`, and `INF-006` as the work completes.

## Test Plan And Acceptance Criteria

- The new UC rerun exists under `benchmarks/reports/<date>-thesis-uc-final/`, has the expected 4 configs and 5 reps, and every run records `best_practice_full=TRUE`.
- Every new figure or generated table artifact lives under `benchmarks/reports/...` and has a stable relative symlink under `docs/assets/evaluation/...`.
- No Evaluation figure in `docs/thesis.typ` references a dated report path directly; all inserted assets go through the stable symlink layer.
- Every inserted figure has nearby prose that names the source study, the dataset/filter, the plotted or derived metrics, and the reader-facing inference.
- The Evaluation chapter compiles cleanly after figure insertion and no Evaluation-section TODO about final rerun provenance, figure generation, or missing RQ6 evidence remains in place.
- `specs/TODO.md` is internally consistent after the work: no item remains simultaneously “complete” and still blocked by an unchecked prerequisite.

## Assumptions And Defaults

- Scope is limited to Evaluation and the directly dependent Discussion and TODO-tracker updates needed to make the rerun and figures thesis-ready; unrelated thesis TODOs remain out of scope.
- Final thesis-grade absolute UC numbers come only from the new best-practice rerun; older named studies remain valid for figures that depend on optimization search, size scaling, PolyBench, sensitivity, or SQLite scope data that the rerun does not reproduce.
- Symlinks use relative paths and semantic filenames so `docs/thesis.typ` remains stable even when the underlying dated report directory changes.
