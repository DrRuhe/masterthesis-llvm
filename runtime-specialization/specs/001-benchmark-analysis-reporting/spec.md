# Feature Specification: Benchmark Analysis & Reporting System

**Feature Branch**: `001-benchmark-analysis-reporting`  
**Created**: 2026-04-23  
**Status**: Draft  
**Input**: User description: "The benchmarking system should allow me to run benchmarks, record their results in the duckdb for later analysis and have reporting scripts that can generate plots/other reports on the data."

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Run and Record Benchmark (Priority: P1)

A researcher wants to execute a benchmark binary against the JIT specializer, automatically persist all results (timing, hardware counters, git context) to the analytical data store, and later recall them for comparison. They invoke `record_benchmark.py` with a path to the compiled benchmark binary, an optional filter pattern, optional performance-isolation flags, and receive confirmation that results were stored including a unique run identifier.

**Why this priority**: Without reliable result capture, all other analysis is impossible. This is the foundational flow.

**Independent Test**: Invoke `record_benchmark.py <binary>` against any single benchmark binary; verify that the data store contains a new context row with a run ID, timestamp, git SHA, and at least one benchmark row with timing and hardware counter data.

**Acceptance Scenarios**:

1. **Given** the data store exists, **When** the user runs `record_benchmark.py <binary>`, **Then** all benchmark metrics (timing, hardware perf counters, JIT stats) are recorded atomically with a timestamp, unique run ID, and current git SHA; the run ID is printed to stdout.
2. **Given** the `--benchmarking-best-practice` flag is supplied and the user has appropriate privileges, **When** the benchmark runs, **Then** ASLR is disabled, CPU governor is set to performance, Intel Turbo Boost is disabled, SMT siblings of benchmark CPUs are taken offline, and CPU affinity is set via taskset; the data store records include the isolation-mode flag.
3. **Given** the `--benchmarking-best-practice` flag is supplied, **When** best-practice setup fails on any step, **Then** the script exits with a clear error before running the benchmark and no record is written.
4. **Given** the benchmark binary exits with a non-zero code, **When** `record_benchmark.py` detects the failure, **Then** it exits without writing to the data store.
5. **Given** the benchmark ran successfully but the DB write fails, **When** the error is caught, **Then** the raw JSON output is preserved on disk and the user is shown the exact command to re-import it with `--record-json`.
6. **Given** the user passes `--benchmark_filter=PATTERN`, **When** the binary is invoked, **Then** only benchmarks matching the pattern are run and recorded.
7. **Given** the user passes `--sudo-askpass PATH`, **When** sudo is needed, **Then** the askpass helper is used instead of interactive input, allowing the script to run in non-terminal environments (CI, remote sessions).

---

### User Story 2 - Import Saved Benchmark Output (Priority: P1)

A researcher has a previously saved Google Benchmark JSON output file (e.g., from a crash-recovery scenario or a run on a remote machine) and wants to import it into the data store without re-running the benchmark.

**Why this priority**: Same priority as recording — this is the recovery path that keeps data durable even when recording fails mid-flight.

**Independent Test**: Save a benchmark JSON manually, then invoke `record_benchmark.py --record-json <file>`; verify that a new run record appears in the data store with the correct metrics.

**Acceptance Scenarios**:

1. **Given** a valid Google Benchmark JSON file exists, **When** `record_benchmark.py --record-json <path>` is invoked, **Then** the metrics are imported into the data store and the run ID is printed; the source file is not deleted.
2. **Given** an optional `--pass-trace-dir` is provided, **When** `*_pass_trace.json` files are present in that directory, **Then** per-pass records are imported into the `pass_traces` table in the same transaction.
3. **Given** the JSON file is malformed, **When** `--record-json` is invoked, **Then** the script prints the first 10 lines of the file and exits with a descriptive error; no partial record is written.

---

### User Story 3 - Initialize Data Store (Priority: P1)

A researcher setting up the system for the first time (or on a new machine) wants to create a fresh, correctly-schematized data store before running any benchmarks.

**Why this priority**: Required before any other workflow; must not overwrite existing data.

**Independent Test**: Run `create_db.py --db benchmarks.duckdb`; verify the file is created with the full schema (tables: `context`, `benchmarks`, `pass_traces`, `optim_trial_params`, `unspec_baselines`; views: `v_parsed`, `v_ns`, `v_ratios`, `v_jit_stats`, `v_budget_sweep`, `v_optim_results`, `v_optim_breakeven`, `v_optim_best_per_kernel`).

**Acceptance Scenarios**:

1. **Given** no data store file exists at the target path, **When** `create_db.py` is invoked, **Then** the file is created with the complete schema and a success message is printed.
2. **Given** a data store file already exists at the target path, **When** `create_db.py` is invoked, **Then** the script exits with an error and the existing file is not modified.
3. **Given** the target path is provided via the `BENCHPLOT_DB_PATH` environment variable, **When** `create_db.py` is invoked without `--db`, **Then** the env var path is used.

---

### User Story 4 - Optimize JIT Pipeline Configuration (Priority: P2)

A researcher wants to discover which JIT pipeline parameters yield the best performance for a given benchmark. They invoke `optimize_benchmarks.py` with a path to the benchmark binary, and the optimizer iterates candidate configurations using Bayesian search, recording each trial's metrics so the globally best configuration can be identified.

**Why this priority**: Optimization is a primary research goal, but depends on reliable recording (P1) being in place.

**Independent Test**: Invoke `optimize_benchmarks.py <binary> --n-trials=5` against a single benchmark; verify that five trial records appear in the data store and the script outputs the best configuration and break-even call count.

**Acceptance Scenarios**:

1. **Given** a benchmark binary and trial budget are specified, **When** `optimize_benchmarks.py` runs, **Then** the unspecialized baseline is measured first (3 repetitions, median), then at least `n_trials` distinct configurations are evaluated, and all trial metrics are stored in the data store as they complete (not only at the end).
2. **Given** individual benchmarks time out, **When** the trial is processed, **Then** the timed-out kernels use `timeout_ns` for JIT cost and `unspec_ns` for exec cost as fallback, a `used_timeout_fallback` flag is stored, and the optimizer continues with the remaining kernels.
3. **Given** `--n-parallel > 1`, **When** the study runs, **Then** trials execute concurrently with thread-safe DB writes; no trial data is corrupted by concurrent access.
4. **Given** the optimization run completes, **When** the user queries the data store, **Then** the best-found configuration parameters, their achieved geomean metrics, and per-kernel break-even call counts are retrievable by study name.
5. **Given** the optimization is interrupted (e.g., Ctrl-C), **When** the process exits, **Then** all fully completed trials are preserved in the data store, no in-progress trial leaves a partial record, and the optimization session is marked as `incomplete` in the data store so it can be excluded from reports and comparisons.
6. **Given** `--output-best PATH` is specified, **When** the run completes, **Then** the best config is written as JSON to that path; if omitted, it is written to `best_<study_name>.json`.
7. **Given** `--seed INT` is provided, **When** the TPE sampler is initialized, **Then** the trial sequence is reproducible across runs with the same seed.

---

### User Story 5 - Generate Analysis Reports and Plots (Priority: P3)

A researcher wants to visualize benchmark results and compare execution time and memory across kernels and runs. They invoke one of the standard reporting scripts, which produces a timestamped report directory containing a PDF plot, a CSV data export, and a `regenerate.py` script for exact reproduction.

**Why this priority**: Reporting depends on historical data existing (P1/P2) and can be added incrementally.

**Independent Test**: Given a populated data store with at least one run, invoke any reporting script; verify that a `benchmarks/reports/YYMMDD-HH-MM-<script>/` directory is created containing the report (E.g. a plot, csv file, ...) and a `regenerate.py` script.

**Acceptance Scenarios**:

1. **Given** the data store contains at least one run, **When** a reporting script is invoked, **Then** a new timestamped directory is created under `benchmarks/reports/`, containing `plot.pdf`, `data.csv`, and an executable `regenerate.py`.
2. **Given** `regenerate.py` from a prior report is executed, **When** it runs, **Then** the same report script is used with the same query/arguments to regenerate the report in a new timestamped directory (the original is not overwritten).
3. **Given** `--run-id` is omitted, **When** a reporting script is invoked, **Then** the most recent run in the data store is used automatically.
4. **Given** `--filter REGEX` is provided, **When** the script runs, **Then** only kernels whose names match the regex are included in the output.
5. **Given** the data store is empty or no runs match the filter, **When** a reporting script is invoked, **Then** it exits gracefully with a descriptive message rather than crashing.
6. **Given** multiple reporting scripts are available, **When** invoked, **Then** each produces its specific plot type: `runtime_comparison.py` (absolute time bars: unspecialized / specialized / JIT overhead / combined), `specialization_overhead.py` (normalized bars relative to unspecialized), `combined.py` (both charts in one PDF), `memory_per_jit.py` (peak RSS delta and JIT bitcode blob size per kernel).

---

### User Story 6 - Analyze JIT Pipeline Pass Behavior (Priority: P4)

A researcher wants to understand which compiler passes in the JIT pipeline take the most time and how IR size evolves through the pipeline. After running a benchmark with pass instrumentation enabled, they invoke a pipeline-analysis reporting script to visualize per-pass wall time and per-pass instruction/function/basic-block counts.

**Why this priority**: This is a specialized research capability that requires pass-trace data from an instrumented benchmark run and builds on P1 and P3.

**Independent Test**: Given a data store with pass-trace records for at least one benchmark, invoke `pass_time_plot.py` or `pass_trace_plot.py`; verify that the output plot distinguishes pipeline groups (prune / initial / fixpoint / postfix / final) and correctly labels fixpoint iterations.

**Acceptance Scenarios**:

1. **Given** pass-trace records exist for a benchmark run, **When** `pass_time_plot.py` is invoked, **Then** a horizontal bar chart is produced showing per-pass wall time, with fixpoint passes stacked by iteration (green = IR changed, red = unchanged), and background bands for each pipeline group.
2. **Given** pass-trace records exist, **When** `pass_trace_plot.py` is invoked, **Then** a 5-subplot figure is produced showing instruction count, function count, basic-block count, per-pass wall time, and IR-changed flag across all passes, with vertical dashed lines at fixpoint iteration boundaries.
3. **Given** `--benchmark PATTERN` is provided, **When** the script runs, **Then** only pass records matching the SQL LIKE pattern are visualized.
4. **Given** no pass-trace records exist for the requested run/pattern, **When** a pipeline-analysis script is invoked, **Then** it exits with a descriptive error rather than producing an empty or misleading plot.

---

### Edge Cases

- What happens when the data store file is missing at startup for `record_benchmark.py`? → Exits with an error directing the user to run `create_db.py` first.
- What happens when `create_db.py` is invoked and the target file already exists? → Exits with an error; no overwrite.
- How does `record_benchmark.py` behave if the same benchmark is run twice in rapid succession? → Each run gets a unique UUID run ID; duplicate timestamps are allowed.
- What happens if a benchmark binary crashes and produces no JSON output? → The empty/missing file is detected, the user is shown a clear error, and no record is written.
- How does `optimize_benchmarks.py` handle a search space with only one viable configuration? → Single-config study completes normally; best config = the only config evaluated.
- What happens when a reporting script encounters records from a pre-existing schema missing newer columns (e.g., `jit_blob_kb`)? → Views are defined with `CREATE OR REPLACE` and guarded `try/except`; scripts check for column existence before querying.
- What happens when `best_practice_env` teardown fails (e.g., cannot re-enable a disabled CPU)? → Each teardown step is attempted independently; warnings are printed but the script does not exit with an error on teardown failure.

## Requirements *(mandatory)*

### Functional Requirements

**record_benchmark.py**

- **FR-001**: `record_benchmark.py` MUST accept a path to a compiled benchmark binary and execute it with JSON output directed to a temporary file.
- **FR-002**: `record_benchmark.py` MUST collect hardware performance counters (instructions, cpu-cycles, branch-misses, L1-icache-load-misses, L1-icache-loads, iTLB-load-misses) alongside timing data for every benchmark row.
- **FR-003**: `record_benchmark.py` MUST capture the current git SHA and embed it in the run context record.
- **FR-004**: `record_benchmark.py` MUST persist all data atomically in a single transaction; if the DB write fails, any partial write MUST be rolled back.
- **FR-005**: `record_benchmark.py` MUST NOT write any record to the data store if the benchmark binary exits with a non-zero code.
- **FR-006**: `record_benchmark.py` MUST preserve the raw benchmark JSON on disk and print a re-import command if the DB write fails after a successful benchmark run.
- **FR-007**: `record_benchmark.py` MUST accept `--benchmarking-best-practice` to apply: ASLR disable, performance CPU governor, Intel Turbo Boost disable, SMT sibling offline, and taskset CPU affinity. Each step requires sudo.
- **FR-008**: `record_benchmark.py` MUST auto-select the two highest-indexed non-boot P-cores for benchmarking when `--benchmark-cpus` is not specified in best-practice mode.
- **FR-009**: `record_benchmark.py` MUST restore all modified system settings (governor, SMT, ASLR, Turbo Boost) after the benchmark completes, even if the benchmark fails.
- **FR-010**: `record_benchmark.py` MUST support `--sudo-askpass PATH` to enable non-interactive sudo via an askpass helper.
- **FR-011**: `record_benchmark.py` MUST support `--record-json PATH` to import a previously saved Google Benchmark JSON file into the data store without re-running the binary.
- **FR-012**: `record_benchmark.py` MUST support `--pass-trace-dir DIR` to import `*_pass_trace.json` files in the same transaction as the benchmark data.
- **FR-013**: `record_benchmark.py` MUST support `--benchmark_filter PATTERN` to restrict which benchmarks within the binary are executed.
- **FR-014**: `record_benchmark.py` MUST resolve the data store path in this order: `--db` flag → `BENCHPLOT_DB_PATH` env var → `./benchmarks.duckdb`.
- **FR-015**: `record_benchmark.py` MUST extend the `benchmarks` table schema dynamically to accommodate new counter columns produced by any binary, without requiring a schema migration.

**create_db.py**

- **FR-016**: `create_db.py` MUST create a new data store file with the full schema (all tables and views) when the target file does not exist.
- **FR-017**: `create_db.py` MUST exit with an error and leave the existing file untouched when the target file already exists.
- **FR-018**: `create_db.py` MUST follow the same DB path resolution as `record_benchmark.py` (flag → env var → default).

**optimize_benchmarks.py**

- **FR-044**: `optimize_benchmarks.py` MUST error and exit when the target data store file does not exist, printing a message directing the user to run `create_db.py` first. It MUST NOT create the data store itself; creation is exclusively the responsibility of `create_db.py`. Optimizer-specific tables (`optim_trial_params`, `unspec_baselines`, `optimization_sessions`) and their associated views are appended to an existing store via `CREATE TABLE/VIEW IF NOT EXISTS` on first use.
- **FR-019**: `optimize_benchmarks.py` MUST measure the unspecialized baseline (3 repetitions, median per kernel) before starting the optimization loop and persist baselines to the data store.
- **FR-020**: `optimize_benchmarks.py` MUST run each benchmark in an independent subprocess with a configurable per-benchmark timeout, using timeout/unspec fallback values when a benchmark times out.
- **FR-021**: `optimize_benchmarks.py` MUST persist every trial's parameters and objectives to the data store immediately upon trial completion, not only at the end of the study.
- **FR-022**: `optimize_benchmarks.py` MUST support `--n-parallel` for concurrent trial execution with thread-safe DB writes.
- **FR-023**: `optimize_benchmarks.py` MUST output the best-found configuration and per-kernel break-even call counts to the terminal upon study completion.
- **FR-024**: `optimize_benchmarks.py` MUST write the best configuration to a JSON file (default: `best_<study_name>.json`; overridable via `--output-best`).
- **FR-025**: `optimize_benchmarks.py` MUST support `--study-name` for named, re-identifiable studies; if omitted, a timestamp-based name is generated.
- **FR-026**: `optimize_benchmarks.py` MUST support `--seed` for reproducible TPE sampler initialization.
- **FR-027**: `optimize_benchmarks.py` MUST write an optimization session record at study start (status = `incomplete`) and update it to `complete` upon normal termination; interrupted studies remain `incomplete` and MUST NOT appear in best-config queries or comparison reports unless explicitly requested.
- **FR-041**: `optimize_benchmarks.py` MUST support a `--search-space PATH` argument that loads the set of parameters to optimize, their types, ranges, and ENV var bindings from a JSON descriptor file at `PATH`. When this argument is supplied, no hardcoded parameter list is used.
- **FR-042**: When `--search-space` is not provided, `optimize_benchmarks.py` MUST fall back to a built-in default descriptor that covers all currently standardized optimizable options, producing the same behavior as the previous hardcoded search space.
- **FR-043**: The `optim_trial_params` table MUST store all trial parameters exclusively in a `params_json JSON` column. On every trial write, this column MUST be populated with a JSON object keyed by parameter name containing the complete parameter set for that trial. There are no separate fixed columns for individual JIT options.
- **FR-044**: Introducing a new optimizable option MUST NOT require code changes to `optimize_benchmarks.py`; the new parameter MUST be expressible solely by adding an entry to a JSON descriptor file.
- **FR-046**: The `optimization_sessions` table MUST include a `search_space_json JSON` column. When a study starts, the descriptor (whether loaded from `--search-space` or the built-in default) MUST be written to this column so the exact search space definition is permanently associated with every study.

**Reporting scripts**

- **FR-028**: All reporting scripts MUST open the data store in read-only mode.
- **FR-029**: All reporting scripts MUST create a timestamped output directory under `benchmarks/reports/YYMMDD-HH-MM-<script_name>/` and write `regenerate.py` into it before any plotting work.
- **FR-030**: All reporting scripts MUST write both `plot.pdf` and `data.csv` to the report directory.
- **FR-031**: `regenerate.py` MUST reproduce the exact same report in a new timestamped directory when executed.
- **FR-032**: All reporting scripts MUST default to the most recent run when `--run-id` is not provided.
- **FR-033**: All reporting scripts MUST accept `--filter REGEX` to scope output to matching kernel names.
- **FR-034**: Optimization-result reporting scripts MUST exclude studies with `status = incomplete` by default; an explicit flag MUST be required to include them.
- **FR-035**: `runtime_comparison.py` MUST produce a grouped bar chart with four bars per kernel group: unspecialized, specialized, JIT overhead, and combined (specialized + JIT), on a log-scaled time axis.
- **FR-036**: `specialization_overhead.py` MUST produce normalized bars (relative to unspecialized = 1.0) on a log scale, with a reference line at 1.0.
- **FR-037**: `combined.py` MUST produce both the runtime comparison and specialization overhead charts stacked in a single PDF.
- **FR-038**: `memory_per_jit.py` MUST plot peak RSS delta (`max_bytes_used`) and JIT bitcode blob size (`jit_blob_kb`) per kernel on a log-scaled memory axis.
- **FR-039**: `pass_time_plot.py` MUST produce a horizontal bar chart of per-pass wall time with fixpoint passes stacked by iteration and background shading per pipeline group.
- **FR-040**: `pass_trace_plot.py` MUST produce a 5-subplot figure covering instruction count, function count, basic-block count, per-pass wall time, and IR-changed flag, with vertical dashed lines at fixpoint iteration boundaries.

### Key Entities *(include if feature involves data)*

- **Context**: One record per benchmark run; attributes: run ID (UUID), run timestamp, git SHA, host name, executable path, CPU count, MHz/CPU, CPU scaling flag, library version, library build type.
- **BenchmarkRow**: One row per benchmark within a run; base attributes: run ID, name, family/instance index, run type, repetitions, repetition index, threads, iterations, real time, CPU time, time unit, KV-parsed fields (group, kernel, phase, raw params). Dynamic counter columns (hardware perf counters, JIT stats) are added on demand.
- **PassTrace**: One record per compiler pass per benchmarkJITAnalysis invocation; attributes: run ID, benchmark name, pass index, pass name, pipeline group, fixpoint iteration, instruction/function/basic-block counts before and after, wall time in ms, IR-changed flag.
- **OptimizationSession**: One record per `optimize_benchmarks.py` invocation; attributes: study name, benchmark binary, trial budget, start timestamp, end timestamp, status (`complete` | `incomplete`). Written at study start as `incomplete`; updated to `complete` on normal exit.
- **OptimizationTrial**: One record per Optuna trial; attributes: study name, trial ID, linked run ID, `params_json` (complete parameter set as a JSON object keyed by parameter name), timeout-fallback flag, geomean objective values (JIT, exec, combined).
- **SearchSpaceDescriptor**: A JSON document loaded at optimizer start that defines which parameters are optimized, their types, ENV var bindings, and ranges/choices. Provided via `--search-space` or defaulted to the built-in descriptor.
- **ParameterDefinition**: One entry in a SearchSpaceDescriptor; attributes: name (Optuna parameter name), env_var (ENV var set when invoking the benchmark subprocess), type (integer range, log-scaled integer, boolean, categorical, or log-integer-or-zero), and type-appropriate range or choice fields.
- **UnspecializedBaseline**: Per-kernel unspecialized median time for a study; attributes: study name, kernel name, unspec time in ns.

### Documentation Requirements

- **DR-001**: The implementation plan (`plan.md`) MUST include a complete, authoritative data model of the DuckDB schema: every table (columns, types, primary keys, foreign keys) and every view (purpose and the tables/columns it exposes), kept in sync with the actual DDL in the scripts.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: A researcher can invoke `record_benchmark.py`, have results stored, and query those results.
- **SC-002**: A benchmark run that fails mid-DB-write leaves the data store unchanged and the raw JSON output recoverable for manual re-import.
- **SC-003**: `optimize_benchmarks.py` completes a 10-trial optimization sweep without data loss even when interrupted after at least 5 trials.
- **SC-004**: Reporting scripts produce a complete report directory (PDF + CSV + regenerate.py) from a data store containing 50+ runs in under 30 seconds.
- **SC-005**: All stored runs are retrievable and queryable without data loss after 100 consecutive benchmark executions.
- **SC-006**: A researcher can reproduce any previously reported plot by executing the `regenerate.py` from its report directory.
- **SC-007**: `create_db.py` initializes a fresh data store with the complete schema in under 5 seconds.
- **SC-008**: Once `params_json` is in place, a new optimizable JIT parameter can be added to the optimization workflow by editing only a JSON descriptor file, with zero changes to `optimize_benchmarks.py` and no further schema changes; all existing and new trial records coexist in the same data store and are queryable via `params_json`.

## Assumptions

- A single researcher (the thesis author) is the primary user; multi-user access control is out of scope.
- The data store is a file on the local filesystem; remote/cloud storage is out of scope.
- Best-practice performance isolation (CPU pinning, governor, SMT, ASLR, Turbo Boost) requires sudo and is optional; the standard recording path works without it.
- Benchmark binaries are already compiled; compilation is not a responsibility of these scripts.
- Reporting scripts produce files (PDF plots, CSV tables) rather than an interactive web UI.
- The optimization algorithm is Optuna's TPE sampler; the default search space covers MaxFixpointIterations, LoopUnrollCount, LargeModuleInstrThreshold, EarlyPrune, and O3Final plus any pipeline-selector or pipeline-specific options exposed via the standard `CRS_DEFAULT_*` ENV var convention. New options are added to the search space by updating the descriptor JSON file, not by modifying the optimizer script.
- Pass-trace data is only present when benchmarks are compiled with pass instrumentation (`benchmarkJITAnalysis`); reporting scripts for pass traces degrade gracefully when no such data exists.
- Existing data in the data store from prior runs must remain valid after script updates; schema changes add columns rather than dropping or altering existing ones.
- Hardware perf counter availability depends on the host kernel and CPU; if counters are unavailable, Google Benchmark silently omits them and the schema extension mechanism handles the missing columns gracefully.
