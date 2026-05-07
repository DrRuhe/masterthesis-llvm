# Tasks: Unified JIT Analysis Launcher

**Input**: Design documents from `specs/006-analyze-launcher/`
**Prerequisites**: plan.md ✅, spec.md ✅, research.md ✅, data-model.md ✅

## Phase 1: Setup

**Purpose**: Create the two new files. No existing files are modified.

- [X] T001 Create `benchmarks/reporting/pass_trace_plot_json.py` — standalone JSON-mode pass trace renderer (shared by T010)
- [X] T002 Create empty `benchmarks/analyze.py` with shebang, license header, module docstring

---

## Phase 2: Foundational Helpers (benchmarks/analyze.py)

**Purpose**: Parse utilities and data structures used by all user stories.

- [X] T003 [P] [US1] Implement `_parse_bm_name(name)` — parses `key:value;...` tags into dict
- [X] T004 [P] [US1] Implement `_sanitize_name(name)` — replaces non-alphanumeric chars with `_`
- [X] T005 [P] [US1] Implement `_find_objdump()` — `shutil.which('llvm-objdump') or shutil.which('objdump')`; returns path or None
- [X] T006 [US1] Implement `AnalysisConfig` dataclass in `analyze.py`

**Checkpoint**: Helpers ready — core pipeline can now be built

---

## Phase 3: User Story 1 — Full Analysis Pipeline (Priority: P1)

**Goal**: Single command → timestamped report dir with all artifacts per matched `jit_analysis` benchmark.

**Independent Test**: `./analyze.py ./AllBenchmarks --benchmarks_filter="g:polybench"` → `reports/<ts>-analysis/` exists with subdirs containing orig/spec/diff ASM, pass trace JSON+plot, chrome trace.

### Implementation for User Story 1

- [X] T007 [US1] Implement `list_jit_analysis_benchmarks(binary, filter_str, timeout)` in `analyze.py` — runs `[binary, '--benchmark_list_tests', '--benchmark_filter=<filter>']`, parses names, filters for `t:jit_analysis`
- [X] T008 [US1] Implement `create_report_dir(cwd)` in `analyze.py` — creates `reports/<YYYYMMDD-hhmmss>-analysis/` and `_staging/{asm,pass_trace,chrome_trace}/` subdirs; returns paths
- [X] T009 [US1] Implement `run_benchmarks(binary, names, staging, config)` in `analyze.py` — builds `|`-joined filter, sets `CRS_*` env vars, calls subprocess with timeout; returns `(returncode, elapsed_s, stderr)`
- [X] T010 [US1] Implement `extract_orig_asm(binary, func_name, out_path, objdump)` in `analyze.py` — runs objdump, extracts function section via regex, normalizes, writes `out_path`
- [X] T011 [US1] Implement `normalize_asm(text)` and `write_diff(orig_path, spec_path, diff_path)` in `analyze.py`
- [X] T012 [US1] Implement `route_artifacts(staging, report_dir, benchmarks, config)` in `analyze.py` — for each benchmark: move pass trace JSON, chrome trace JSON; match+copy ASM files; call `extract_orig_asm`; write diff; call `plot_from_json` if pass_trace enabled
- [X] T013 [US1] Implement `write_summary(report_dir, results)` in `analyze.py` — writes `summary.md` table after all benchmarks complete
- [X] T014 [US1] Implement progress printing in main loop — `[i/N] Running <name>... done (2.3s)` pattern (FR-014)
- [X] T015 [US1] Implement `main()` and `argparse` CLI in `analyze.py` — positional `binary`, `--benchmarks_filter`, `--timeout`, `--no-asm-dump`, `--no-pass-trace`, `--no-chrome-trace`; print report_dir on completion (FR-011)

**Checkpoint**: User Story 1 fully functional — run full analysis with one command

---

## Phase 4: User Story 2 — Selective Component Execution (Priority: P2)

**Goal**: `--no-asm-dump`, `--no-pass-trace`, `--no-chrome-trace` each independently suppress their component.

**Independent Test**: Run with `--no-pass-trace --no-chrome-trace` — output subdirs contain only ASM files; no JSON, no PDF.

### Implementation for User Story 2

- [X] T016 [US2] Wire `config.asm` flag through `route_artifacts` — skip `extract_orig_asm`, `write_diff`, and ASM file move when `asm=False`
- [X] T017 [US2] Wire `config.pass_trace` flag — skip pass trace JSON move and `plot_from_json` call when `pass_trace=False`; CRS_PASS_TRACE_DIR always set (benchmarkJITAnalysis throws if absent)
- [X] T018 [US2] Wire `config.chrome_trace` flag — skip chrome trace JSON move when `chrome_trace=False`; CRS_CHROME_TRACE_DIR always set (benchmarkJITAnalysis throws if absent)

**Checkpoint**: All three `--no-*` flags independently disable their artifact generation

---

## Phase 5: Edge Cases and Error Handling (Priority: P2/P3)

**Goal**: Graceful handling of missing binary, timeout, failed benchmarks, missing objdump, no matches.

- [X] T019 [US1] Validate `binary` is executable at startup; exit with clear error if not (Acceptance Scenario 3 of US1)
- [X] T020 [US1] Handle empty `jit_analysis` match list — print informative message and exit 0 without creating report dir (Acceptance Scenario 2 of US1)
- [X] T021 [US1] Handle benchmark timeout — kill subprocess, set `BenchmarkRun.status = "timeout"`, continue to next (edge case spec)
- [X] T022 [US1] Handle missing objdump — if `--no-asm-dump` not set and objdump not on PATH, exit with clear diagnostic (edge case spec)
- [X] T023 [US1] Handle inlined/missing symbol in ELF — write empty `orig_<func>.asm` and log warning rather than failing (edge case spec)
- [X] T024 [US1] Handle failed `pass_trace_plot_json` render — log warning, continue; artifact missing but benchmark not marked fail

---

## Phase 6: Polish

- [X] T025 Make `analyze.py` executable (`chmod +x`) and verify shebang works
- [X] T026 Verify `pass_trace_plot_json.py` produces valid PDF output matching the style of `pass_trace_plot.py`

---

## Dependencies & Execution Order

- **Phase 1** (T001–T002): No dependencies
- **Phase 2** (T003–T006): Depends on T002
- **Phase 3** (T007–T015): Depends on Phase 2 + T001
- **Phase 4** (T016–T018): Depends on Phase 3 (flags already parsed; wiring touches same functions)
- **Phase 5** (T019–T024): Depends on Phase 3 (error handling added to existing functions)
- **Phase 6** (T025–T026): Depends on all prior phases
