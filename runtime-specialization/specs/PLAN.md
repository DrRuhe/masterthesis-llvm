# Evaluation Data Collection Plan

**Created**: 2026-06-10  
**Goal**: Complete all benchmark runs required for thesis evaluation (RQ1–RQ6).  
**Report root**: `benchmarks/reports/` — each phase creates a dated subdirectory there.  
**References**: `specs/TODO.md` (task IDs), `benchmarks/reports/260601-15-16-optimize-pipeline/reflection.md` (last study state).

---

## Phase 0 — Commit uncommitted fix (polybench funcptr forwarding)

- [ ] Confirm smoke suite is green: `cd llvm/llvm/build/debug && ninja check-smoke-runtime-specializer` (expect 53 pass + 1 xfail)
- [ ] Stage and commit `comptime/IRRewritingPass.cpp` (resolveArgFromCallers / funcptr forwarding fix)
- [ ] Stage and commit `benchmarks/ClangRuntimeSpecializerBenchmark.h` (NTTP helper templates added)
- [ ] Stage and commit `benchmarks/SpecializerBenchmark.cpp`, `benchmarks/DBOperatorsBenchmark.cpp`, `benchmarks/polybench/polybench_bench.cpp` (macro migration to NTTP form)
- [ ] Stage and commit `benchmarks/tpch/CMakeLists.txt`
- [ ] Stage and commit `test/smoke/call-specialized-forwarded-funcptr.cpp`, `test/smoke/speconly-forwarded-funcptr.cpp`, `test/smoke/speconly-std-apply-funcptr.cpp`
- [ ] Re-run smoke suite after commit to confirm still green

---

## Phase 1 — Rebuild release binaries

- [ ] `cd llvm/llvm/build/release && ninja AllBenchmarks PolyBenchBenchmark` — picks up IRRewritingPass + polybench_bench.cpp changes
- [ ] Confirm both binaries exist under `build/release/tools/runtime-specialization/benchmarks/`

---

## Phase 2 — Validate polybench JIT fix (TODO: RQ1-002)

- [ ] Run a single polybench JIT smoke check (1 kernel, MEDIUM) to confirm no `ClangRuntimeSpecializerDumpedIRError`:
  ```
  PolyBenchBenchmark --benchmark_filter='BM_g:polybench.*n:correlation.*s:MEDIUM.*t:jit_overhead' --benchmark_repetitions=1
  ```
- [ ] Confirm output shows a non-zero `real_time` (not 0.0 or an error)
- [ ] If it errors: [complex] debug — the NTTP IRRewritingPass fix may not reach through the `kernel_##K` static function definition in polybench_bench.cpp; inspect generated IR

---

## Phase 3 — Final corpus ablation study (TODO: RQ1-001)

- [ ] Confirm `benchmarks/reports/260601-15-16-optimize-pipeline/uc_workload_optimal.json` exists (P2-optimal config for injection)
- [ ] Create output directory `benchmarks/reports/260610-corpus-final/`
- [ ] Run ablation study: 7 builtin configs + p0_o3_optimal + p2_o3_optimal × 5 reps, UC MEDIUM "low" abstraction filter:
  ```
  python3 benchmarks/ablation_benchmarks.py \
    build/release/tools/runtime-specialization/benchmarks/AllBenchmarks \
    --study-name corpus_uc_final_20260610 \
    --filter 'BM_g:.*s:MEDIUM.*kv_a:low.*t:(specialized_exec|unspecialized|jit_overhead)' \
    --reps 5 \
    --extra-configs benchmarks/reports/260601-15-16-optimize-pipeline/uc_workload_optimal.json \
    --out-dir benchmarks/reports/260610-corpus-final/
  ```
  (adjust CLI flags to match ablation_benchmarks.py actual interface)
- [ ] Verify all 9 configs × 18 kernels × 5 reps recorded in `benchmarks.duckdb` study `corpus_uc_final_20260610`
- [ ] Run SQL summary query and save output to `benchmarks/reports/260610-corpus-final/speedup_summary.txt`:
  ```sql
  SELECT config_name, kernel,
         ROUND(med_unspec_ns::DOUBLE/med_spec_ns, 3) AS speedup, n_reps
  FROM v_ablation_medians
  WHERE study_name='corpus_uc_final_20260610'
  ORDER BY config_name, speedup DESC;
  ```
- [ ] Verify geomean speedups per config are plausible (default ~1.39×, p0/p2 optimal ~2.2–2.3×)
- [ ] Commit `benchmarks/reports/260610-corpus-final/` and updated `benchmarks.duckdb`

---

## Phase 4 — Full polybench benchmark run (TODO: RQ3-001)

Prerequisite: Phase 2 validates JIT works.

- [ ] Create output directory `benchmarks/reports/260610-polybench/`
- [ ] Run polybench with default config, 3 reps, SMALL + MEDIUM sizes, all phases:
  ```
  PolyBenchBenchmark \
    --benchmark_filter='BM_g:polybench.*s:(SMALL|MEDIUM).*t:(jit_overhead|specialized_exec|unspecialized)' \
    --benchmark_repetitions=3 \
    --benchmark_out=benchmarks/reports/260610-polybench/polybench_default.json \
    --benchmark_out_format=json
  ```
- [ ] Import results into `benchmarks.duckdb` using `record_benchmark.py`
- [ ] Run polybench with p2_o3_optimal config (env vars from `uc_workload_optimal.json`), 3 reps, same filter:
  ```
  CRS_DEFAULT_PIPELINE=2 CRS_DEFAULT_O3_FINAL=1 ... PolyBenchBenchmark ...
  --benchmark_out=benchmarks/reports/260610-polybench/polybench_p2_optimal.json
  ```
- [ ] Verify both runs produce non-zero `jit_overhead` real_time for all 30 kernels × 2 sizes
- [ ] Run SQL query to compute per-kernel speedup table and save to `benchmarks/reports/260610-polybench/speedup_table.txt`
- [ ] Commit `benchmarks/reports/260610-polybench/` and updated `benchmarks.duckdb`

---

## Phase 5 — Binary-size overhead measurement (TODO: RQ2-001)

- [ ] Create output directory `benchmarks/reports/260610-binary-size/`
- [ ] Identify a representative UC benchmark TU (e.g. `UC1Kernels` or `DBOperatorsBenchmark`)
- [ ] [complex] Compile the TU twice — once with `-fpass-plugin=LLVMRuntimeSpecializationComptimePlugin`, once without — and measure binary sizes using `size` and `wc -c`:
  - Without plugin: `clang++ -O2 -c UC1Kernels.cpp -o without_plugin.o && size without_plugin.o`
  - With plugin: same + `-fpass-plugin=...` → `size with_plugin.o`
  - Full linked binaries: compare `SpecializerBenchmark` (with CRS) vs a stripped version without linking the CRS runtime (`-lClangRuntimeSpecializer` omitted if possible, or use `size` on the final binary sections)
  - Record: `.text` size, `.data` size, total; IR-blob contribution via `objdump -s --section=.rodata | grep -c crs_blob` or similar
- [ ] Compute overhead percentage: `(with_CRS - without_CRS) / without_CRS × 100`
- [ ] Save measurements to `benchmarks/reports/260610-binary-size/binary_size_table.txt`
- [ ] Commit `benchmarks/reports/260610-binary-size/`

---

## Phase 6 — Sensitivity analysis OAT sweep (TODO: RQ4-002)

- [ ] Create output directory `benchmarks/reports/260610-sensitivity/`
- [ ] Run OAT sweep for `fixpoint_max` (values: 1,2,4,8,16,24) starting from p0_o3_optimal base, 3 reps, UC MEDIUM low abstraction filter:
  ```
  python3 benchmarks/sensitivity_analysis.py \
    build/release/tools/runtime-specialization/benchmarks/AllBenchmarks \
    --study-name sens_uc_fixpoint_20260610 --param fixpoint_max \
    --sweep-values 1,2,4,8,16,24 --reps 3 \
    --base-config benchmarks/reports/260601-15-16-optimize-pipeline/best_uc_optim_iter3_20260601.json \
    --filter 'BM_g:.*s:MEDIUM.*kv_a:low.*t:(specialized_exec|jit_overhead)'
  ```
- [ ] Run OAT sweep for `unroll_max` (values: 1,4,16,32,64,128), same base + filter
- [ ] Run OAT sweep for `large_module_max` (values: 0,1,2,4,8), same
- [ ] Run OAT sweep for `early_prune` (values: 0,1), same
- [ ] Run OAT sweep for `o3_final` (values: 0,1), same
- [ ] Run OAT sweep for `pipeline` (values: 0,2 — skip 1 per known crash), same
- [ ] Verify all sweep studies recorded in `benchmarks.duckdb`
- [ ] Generate sensitivity plot: `python3 benchmarks/reporting/plot_sensitivity.py --studies sens_uc_*_20260610 --out benchmarks/reports/260610-sensitivity/sensitivity.png`
- [ ] Classify each parameter as critical (>10% combined cost change), moderate (5–10%), or insensitive (<5%) and save classification to `benchmarks/reports/260610-sensitivity/parameter_classification.txt`
- [ ] Commit `benchmarks/reports/260610-sensitivity/` and updated `benchmarks.duckdb`

---

## Phase 7 — Break-even analysis (TODO: RQ5-001, RQ1-003)

Prerequisite: Phase 3 complete (corpus_uc_final_20260610 in DB).

- [ ] Create output directory `benchmarks/reports/260610-breakeven/`
- [ ] Run break-even SQL query against `corpus_uc_final_20260610` and save:
  ```sql
  SELECT kernel,
         ROUND(t_jit_ns/1e6, 1)   AS jit_ms,
         ROUND(t_spec_ns/1e6, 2)  AS spec_ms,
         ROUND(unspec_ns/1e6, 2)  AS unspec_ms,
         ROUND(break_even_calls)  AS break_even_calls,
         ROUND(unspec_ns::DOUBLE/t_spec_ns, 2) AS exec_speedup
  FROM v_optim_breakeven
  WHERE study_name='corpus_uc_final_20260610'
  ORDER BY break_even_calls NULLS LAST;
  ```
  Save to `benchmarks/reports/260610-breakeven/breakeven_table.txt`
- [ ] Generate break-even histogram plot: `python3 benchmarks/reporting/plot_breakeven.py --study corpus_uc_final_20260610 --out benchmarks/reports/260610-breakeven/breakeven_hist.png`
- [ ] Identify any kernel with break-even > 1000 calls; document root cause (expected: `count_matching_rows`)
- [ ] Verify break-even counts for p0_o3_optimal and p2_o3_optimal configs are plausible vs. reflection §4 baseline
- [ ] Commit `benchmarks/reports/260610-breakeven/`

---

## Phase 8 — Pareto plots (TODO: INF-001)

- [ ] Create output directory `benchmarks/reports/260610-pareto/`
- [ ] Run Pareto plot generation for `uc_optim_iter3_20260601` (optimizer study):
  ```
  python3 benchmarks/reporting/plot_pareto_configs.py \
    --study uc_optim_iter3_20260601 \
    --out-dir benchmarks/reports/260610-pareto/
  ```
- [ ] Run Pareto plot for `corpus_uc_final_20260610` (ablation study)
- [ ] Confirm PNGs and CSVs generated for each UC group; verify Default config is marked on each plot
- [ ] Commit `benchmarks/reports/260610-pareto/`

---

## Phase 9 — TPC-H module-size documentation (TODO: RQ6-002)

- [ ] Create output directory `benchmarks/reports/260610-tpch-scope/`
- [ ] Measure sqlite3VdbeExec module stats by running AllBenchmarks with filter `BM_g:tpch` and extracting `jit_module_instrs` counter from the JSON output (runs only unspecialized; does not attempt JIT)
- [ ] Record: blob size (KB), function count, instruction count; save to `benchmarks/reports/260610-tpch-scope/module_stats.txt`
- [ ] Write `benchmarks/reports/260610-tpch-scope/scope_statement.md`: "TPC-H (sqlite3VdbeExec) has X instructions / Y functions / Z KB blob. UC MEDIUM modules average ~21k instructions. JIT overhead for TPC-H exceeds 2 minutes under P0 (measured 2026-05-31). Conclusion: module-size limit for practical specialization is ~21k instructions; TPC-H exceeds this by factor N×. Documented as RQ6 failure case."
- [ ] Commit `benchmarks/reports/260610-tpch-scope/`

---

## Phase 10 — Infrastructure documentation (TODO: INF-002, INF-003, INF-004)

- [ ] Create `benchmarks/reports/260610-infra-docs/`
- [ ] Export DuckDB schema to `benchmarks/reports/260610-infra-docs/schema.sql`:
  ```
  duckdb benchmarks/benchmarks.duckdb ".schema"
  ```
- [ ] Compile study summary table (study name, date, phase, trial/rep count, best config, best combined ms) via SQL and save to `benchmarks/reports/260610-infra-docs/study_summary.txt`
- [ ] Record hardware + environment to `benchmarks/reports/260610-infra-docs/environment.txt`:
  - `uname -a`, `lscpu | grep -E 'Model|CPU|Thread|Core|Socket|MHz'`
  - `clang --version`, `python3 --version`, `python3 -c "import optuna; print(optuna.__version__)"`
  - CPU frequency scaling status: `cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor`
- [ ] Commit `benchmarks/reports/260610-infra-docs/`

---

## Completion Gate

- [ ] All 8 report subdirectories exist under `benchmarks/reports/`
- [ ] `benchmarks.duckdb` contains studies: `corpus_uc_final_20260610`, `sens_uc_*_20260610`, and polybench results
- [ ] Smoke suite still passes: `ninja check-smoke-runtime-specializer`
- [ ] Update `specs/TODO.md`: mark RQ1-001, RQ1-002, RQ2-001, RQ3-001, RQ4-002, RQ5-001, RQ6-002, INF-001–INF-004 as ✅ Complete with date 2026-06-10 and report directory references
