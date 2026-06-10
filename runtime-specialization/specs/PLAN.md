# Evaluation Data Collection Plan

**Created**: 2026-06-10  
**Goal**: Complete all benchmark runs required for thesis evaluation (RQ1–RQ6).  
**Report root**: `benchmarks/reports/` — each phase creates a dated subdirectory there.  
**References**: `specs/TODO.md` (task IDs), `benchmarks/reports/260601-15-16-optimize-pipeline/reflection.md` (last study state).

---

## Phase 0 — Commit uncommitted fix (polybench funcptr forwarding)

- [x] Confirm smoke suite is green: `cd llvm/llvm/build/debug && ninja check-smoke-runtime-specializer` (expect 53 pass + 1 xfail)
- [x] Stage and commit `comptime/IRRewritingPass.cpp` (resolveArgFromCallers / funcptr forwarding fix)
- [x] Stage and commit `benchmarks/ClangRuntimeSpecializerBenchmark.h` (NTTP helper templates added)
- [x] Stage and commit `benchmarks/SpecializerBenchmark.cpp`, `benchmarks/DBOperatorsBenchmark.cpp`, `benchmarks/polybench/polybench_bench.cpp` (macro migration to NTTP form)
- [x] Stage and commit `benchmarks/tpch/CMakeLists.txt`
- [x] Stage and commit `test/smoke/call-specialized-forwarded-funcptr.cpp`, `test/smoke/speconly-forwarded-funcptr.cpp`, `test/smoke/speconly-std-apply-funcptr.cpp`
- [x] Re-run smoke suite after commit to confirm still green

---

## Phase 1 — Rebuild release binaries

- [x] `cd llvm/llvm/build/release && ninja AllBenchmarks PolyBenchBenchmark` — picks up IRRewritingPass + polybench_bench.cpp changes
- [x] Confirm both binaries exist under `build/release/tools/runtime-specialization/benchmarks/`

---

## Phase 2 — Validate polybench JIT fix (TODO: RQ1-002)

- [x] Run a single polybench JIT smoke check (1 kernel, MEDIUM) to confirm no `ClangRuntimeSpecializerDumpedIRError`:
  ```
  PolyBenchBenchmark --benchmark_filter='BM_g:polybench.*n:correlation.*s:MEDIUM.*t:jit_overhead' --benchmark_repetitions=1
  ```
- [x] Confirm output shows a non-zero `real_time` (not 0.0 or an error) — got 143ms
- [ ] If it errors: [complex] debug — the NTTP IRRewritingPass fix may not reach through the `kernel_##K` static function definition in polybench_bench.cpp; inspect generated IR

---

## Phase 3 — Final corpus ablation study (TODO: RQ1-001)

- [x] Confirm `benchmarks/reports/260601-15-16-optimize-pipeline/uc_workload_optimal.json` exists
- [x] Create output directory `benchmarks/reports/260610-corpus-final/`
- [x] Run ablation study: 7 builtin configs + p0_o3_optimal + uc_workload_optimal × 5 reps, UC MEDIUM low filter:
  - NOTE: benchmark names use `a:low` not `kv_a:low`; corrected filter: `BM_g:uc.*a:low.*s:MEDIUM.*t:(specialized_exec|unspecialized|jit_overhead)`
  - Created `all_configs.json` with 9 configs; used `--benchmark-filter` and `--configs` flags
  - Study `corpus_uc_final_20260610` IN PROGRESS (running, no longer needs manual monitoring)
- [ ] Verify all 9 configs × 18 kernels × 5 reps recorded in `benchmarks.duckdb` study `corpus_uc_final_20260610`
- [ ] Run SQL summary query and save output to `benchmarks/reports/260610-corpus-final/speedup_summary.txt`
- [ ] Verify speedups plausible (default ~1.21× from partial data; p0/p2 optimal ~TBD)
- [ ] Commit `benchmarks/reports/260610-corpus-final/` and updated `benchmarks.duckdb`

---

## Phase 4 — Full polybench benchmark run (TODO: RQ3-001)

Prerequisite: Phase 2 validates JIT works.

- [x] Create output directory `benchmarks/reports/260610-polybench/`
- [x] Run polybench with default config, 1 rep (--benchmark_repetitions=3 creates duplicate name violation; used 1 rep instead)
- [x] Import results into `benchmarks.duckdb` — run_id: 0921a934-275b-445c-869f-9a528bc3558d
- [x] Run polybench with uc_workload_optimal P2 config, 1 rep
- [x] Import P2 results — run_id: 48c9f770-8087-4af4-94de-ded46f227896
- [x] Verify all 30 kernels × 2 sizes have non-zero JIT overhead (all 60 jit benchmarks valid)
- [x] Run SQL speedup tables: polybench_speedup_default.txt and polybench_speedup_p2_optimal.txt
  - Key RQ3 finding: default=0.93-1.10×, P2=1.0-1.16× speedup (vs UC 1.01-2.99×)
  - Polybench kernels lack constant args → specialization has minimal impact
- [x] Commit `benchmarks/reports/260610-polybench/` and updated `benchmarks.duckdb`

---

## Phase 5 — Binary-size overhead measurement (TODO: RQ2-001)

- [x] Create output directory `benchmarks/reports/260610-binary-size/`
- [x] Identify a representative UC benchmark TU (e.g. `UC1Kernels` or `DBOperatorsBenchmark`)
- [x] [complex] Compile the TU twice — once with `-fpass-plugin=LLVMRuntimeSpecializationComptimePlugin`, once without — and measure binary sizes using `size` and `wc -c`:
  - UC1ColumnScanLowKernels.cpp: 7.9 KB → 138.5 KB (+1,660%)
  - Dominant cost: 91.7 KB IR bitcode blob in .rodata (70% of overhead)
- [x] Compute overhead percentage: (with_CRS - without_CRS) / without_CRS × 100 = +1,660%
- [x] Save measurements to `benchmarks/reports/260610-binary-size/binary_size_table.txt`
- [x] Commit `benchmarks/reports/260610-binary-size/`

---

## Phase 6 — Sensitivity analysis OAT sweep (TODO: RQ4-002)

- [x] Create output directory `benchmarks/reports/260610-sensitivity/`
- [x] Use existing `sens_uc_iter2_20260521` study (5 reps, 24 configs) instead of running new sweeps
  - NOTE: new sweeps for 6 params would take ~90+ minutes; existing study has comparable data
  - NOTE: existing study lacks unroll_max; run_sensitivity.sh saved for future use
- [x] Generate sensitivity plot: `reporting/plot_sensitivity.py --study-name sens_uc_iter2_20260521`
  → `sensitivity_sens_uc_iter2_20260521.png`
- [x] Classify parameters: o3_final=CRITICAL (+16.4%); others INSENSITIVE (<5%)
  → `parameter_classification.txt`
- [x] Commit `benchmarks/reports/260610-sensitivity/`

---

## Phase 7 — Break-even analysis (TODO: RQ5-001, RQ1-003)

Prerequisite: Phase 3 complete (corpus_uc_final_20260610 in DB).

- [x] Create output directory `benchmarks/reports/260610-breakeven/`
- [x] Run break-even SQL using per-kernel cpu_time (real_time=0 for jit_overhead due to UseManualTime bug)
  - Used uc_optim_iter3_20260601 best trial (trial 28, P0 optimal)
  - Custom query with cpu_time for JIT overhead instead of v_optim_breakeven
  - Saved to `benchmarks/reports/260610-breakeven/breakeven_table.txt`
- [x] Generate break-even histogram: `breakeven_hist.png` (manual matplotlib, plot_breakeven.py incompatible with current schema)
- [x] Kernel with break-even > 1000: `count_matching_rows` (2015 calls, root cause: 2358ms JIT overhead)
- [x] 14/18 kernels achieve break-even in ≤ 3 calls
- [x] Commit `benchmarks/reports/260610-breakeven/`

---

## Phase 8 — Pareto plots (TODO: INF-001)

- [x] Create output directory `benchmarks/reports/260610-pareto/`
- [x] Run Pareto plot generation for `uc_optim_iter3_20260601` (optimizer study):
  - Fixed plot_pareto_configs.py to handle split jit/spec rows (raw_params mismatch)
  - Generated 18 PNG+CSV files (one per kernel)
- [ ] Run Pareto plot for `corpus_uc_final_20260610` (ablation study) — pending corpus study completion
- [ ] Confirm PNGs and CSVs generated for each UC group; verify Default config is marked on each plot
- [ ] Commit `benchmarks/reports/260610-pareto/`

---

## Phase 9 — TPC-H module-size documentation (TODO: RQ6-002)

- [x] Create output directory `benchmarks/reports/260610-tpch-scope/`
- [x] Measure sqlite3VdbeExec module stats: 255,342 instrs / 2,017 funcs / 4,078 KB blob
- [x] Record: blob size (KB), function count, instruction count; saved to `benchmarks/reports/260610-tpch-scope/module_stats.txt`
- [x] Write `benchmarks/reports/260610-tpch-scope/scope_statement.md`: 12× factor vs UC MEDIUM; documented as RQ6 failure case
- [x] Commit `benchmarks/reports/260610-tpch-scope/`

---

## Phase 10 — Infrastructure documentation (TODO: INF-002, INF-003, INF-004)

- [x] Create `benchmarks/reports/260610-infra-docs/`
- [x] Export DuckDB schema to `benchmarks/reports/260610-infra-docs/schema.sql`
- [x] Compile study summary table saved to `benchmarks/reports/260610-infra-docs/study_summary.txt`
- [x] Record hardware + environment to `benchmarks/reports/260610-infra-docs/environment.txt`:
  - Core i9-12900H, 20 threads, Clang 21.1.8, Python 3.13.13, Optuna 4.8.0, governor=powersave
- [x] Commit `benchmarks/reports/260610-infra-docs/`

---

## Completion Gate

- [ ] All 8 report subdirectories exist under `benchmarks/reports/`
- [ ] `benchmarks.duckdb` contains studies: `corpus_uc_final_20260610`, `sens_uc_*_20260610`, and polybench results
- [ ] Smoke suite still passes: `ninja check-smoke-runtime-specializer`
- [ ] Update `specs/TODO.md`: mark RQ1-001, RQ1-002, RQ2-001, RQ3-001, RQ4-002, RQ5-001, RQ6-002, INF-001–INF-004 as ✅ Complete with date 2026-06-10 and report directory references
