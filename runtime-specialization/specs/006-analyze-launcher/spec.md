# Feature Specification: Unified JIT Analysis Launcher

**Feature Branch**: `006-analyze-launcher`  
**Created**: 2026-05-07  
**Status**: Draft  

## User Scenarios & Testing *(mandatory)*

### User Story 1 — Run Full Analysis for a Benchmark Group (Priority: P1)

A developer investigating JIT specialization quality runs a single command against a benchmark binary and a benchmark group filter. The tool automatically discovers the relevant analysis benchmarks, executes them, and organizes all output (ASM diffs, pass trace plots, chrome traces) into a timestamped report directory — without the developer needing to know function symbol names or benchmark filter strings in advance.

**Why this priority**: This is the complete end-to-end workflow. Everything else is a refinement. Without this, the developer must manually orchestrate multiple steps: listing benchmarks, constructing filters, setting env vars, running objdump, running plotting scripts — all of which the tool replaces.

**Independent Test**: Run `./analyze.py ./AllBenchmarks --benchmarks_filter="g:polybench"` against a real benchmark binary. Verify that a `reports/YYYYMMDD-hhmmss-analysis/` directory is created, containing a subdirectory for each matched jit_analysis benchmark, each with an original ASM file, a specialized ASM file, a normalized diff, a pass trace JSON, and a rendered pass trace plot.

**Acceptance Scenarios**:

1. **Given** a built benchmark binary and a valid group filter, **When** the developer runs `./analyze.py <binary> --benchmarks_filter="g:polybench"`, **Then** the tool creates `reports/<timestamp>-analysis/` containing one subdirectory per matched jit_analysis benchmark.
2. **Given** no jit_analysis benchmarks match the filter, **When** the tool runs, **Then** it prints a clear message and exits without creating empty output directories.
3. **Given** the benchmark binary is not executable or not found, **When** the tool runs, **Then** it exits with a descriptive error before attempting any benchmarks.

---

### User Story 2 — Selective Component Execution (Priority: P2)

A developer who only wants to inspect ASM — not pass trace or chrome trace — can suppress the unwanted outputs via flags, making the run faster and the output directory less cluttered.

**Why this priority**: Each analysis component has runtime cost. ASM dumping requires running the JIT once; pass trace and chrome trace add overhead. Developers iterating on ASM quality should not be forced to wait for unneeded outputs.

**Independent Test**: Run `./analyze.py ./AllBenchmarks --benchmarks_filter="g:polybench" --no-pass-trace --no-chrome-trace`. Verify that the output directories contain ASM files but no pass trace JSON, no chrome trace JSON, and no rendered plots.

**Acceptance Scenarios**:

1. **Given** `--no-asm-dump` is passed, **When** the tool runs, **Then** no original, specialized, or diff ASM files are written.
2. **Given** `--no-pass-trace` is passed, **When** the tool runs, **Then** no pass trace JSON or pass trace plot is written.
3. **Given** `--no-chrome-trace` is passed, **When** the tool runs, **Then** no chrome trace JSON is written.
4. **Given** all three `--no-*` flags are passed, **When** the tool runs, **Then** each benchmark subdirectory is still created (as a record of execution) but contains no analysis artifacts.

---

### User Story 3 — Automatic Function Name Discovery (Priority: P2)

A developer does not need to look up or specify the C symbol name of the function being specialized. The tool discovers the name automatically by inspecting the artifacts produced when the JIT runs, so the command stays simple and correct even when benchmark-to-symbol mappings are non-obvious.

**Why this priority**: The original `asm-compare.sh` required the user to supply the function name manually, making it error-prone and hard to batch across many benchmarks. Automatic discovery eliminates this friction.

**Independent Test**: Run the full analysis against at least two benchmarks in the same group. Verify that the original ASM file in each benchmark subdirectory corresponds to the correct function for that benchmark (check symbol name matches the specialized ASM filename).

**Acceptance Scenarios**:

1. **Given** the JIT has run a jit_analysis benchmark and produced a specialized ASM file, **When** the tool scans the output, **Then** it derives the original function name from the ASM filename without any manual input.
2. **Given** a function whose symbol name is mangled or contains non-alphanumeric characters, **When** the tool extracts the original ASM, **Then** it correctly matches the symbol in the binary and produces a non-empty original ASM file.

---

### User Story 4 — Incremental / Repeated Runs (Priority: P3)

A developer runs analyze.py multiple times (e.g., before and after a code change) and expects each run to produce a fresh, independently timestamped report directory without overwriting previous results.

**Why this priority**: Comparison across runs is only meaningful if previous results are preserved. This is a correctness property, not a performance optimization.

**Independent Test**: Run the tool twice in succession. Verify two distinct `reports/<timestamp>-analysis/` directories exist and neither overwrites the other.

**Acceptance Scenarios**:

1. **Given** a previous analysis report exists, **When** the tool runs again, **Then** a new timestamped directory is created alongside the old one.

---

### Edge Cases

- What if a jit_analysis benchmark crashes or exceeds the configurable timeout? The tool should kill the process, continue to the next benchmark, and record the failure in the summary.
- What if the original function symbol is inlined away or absent from the binary's symbol table? The tool should write an empty or annotated placeholder ASM file and log a warning rather than failing.
- What if the binary outputs no benchmarks for the given filter? The tool should exit cleanly with an informative message.
- What if objdump is not on PATH? The tool should detect this early and either skip the original-ASM step (if `--no-asm-dump` not set) or exit with a clear diagnostic.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The tool MUST accept a benchmark binary path as a required positional argument and a `--benchmarks_filter` flag whose value is forwarded to the binary's native benchmark filter mechanism.
- **FR-002**: The tool MUST query the binary using the provided filter to enumerate all available benchmark names, then narrow the list to those whose type tag is `t:jit_analysis`.
- **FR-003**: The tool MUST run the selected jit_analysis benchmarks as a single invocation using a combined filter of their fully-qualified names, so each analysis benchmark executes exactly once per `./analyze.py` invocation.
- **FR-004**: The tool MUST write all output into a subdirectory of `reports/` named `YYYYMMDD-hhmmss-analysis`, relative to the working directory where the tool is invoked.
- **FR-005**: For each executed jit_analysis benchmark, the tool MUST create a subdirectory within the report directory named by replacing all non-alphanumeric characters in the fully-qualified benchmark name with underscores (e.g., `BM_g_polybench_n_correlation_s_EXTRALARGE_t_jit_analysis_`), and place all artifacts for that benchmark there.
- **FR-006**: When ASM dumping is enabled (default), the tool MUST: (a) capture specialized ASM by directing JIT output to the benchmark subdirectory; (b) for each `*__specialized.asm` file found (one or more), derive the function name from the filename; (c) extract the corresponding original ASM from the binary for each function, including all functions it transitively calls/jumps to (BFS up to 50 functions). All pairs are placed in the same benchmark subdirectory.
- **FR-007**: When pass trace is enabled (default), the tool MUST direct pass trace JSON output to the benchmark subdirectory and generate a rendered pass trace plot in that subdirectory.
- **FR-008**: When chrome trace is enabled (default), the tool MUST direct chrome trace JSON output to the benchmark subdirectory.
- **FR-009**: The tool MUST support `--no-asm-dump`, `--no-pass-trace`, and `--no-chrome-trace` flags to disable individual analysis components independently.
- **FR-010**: When a benchmark execution fails (non-zero exit, timeout, or missing expected output files), the tool MUST log the failure, skip that benchmark's artifact generation, and continue with remaining benchmarks.
- **FR-011**: The tool MUST print the path of the created report directory upon completion.
- **FR-012**: The tool MUST support a `--timeout=N` flag (default: 60 seconds) that sets the maximum wall-clock time allowed per benchmark execution; a benchmark exceeding this limit is killed and treated as a failure per FR-010.
- **FR-013**: The tool MUST write a `summary.md` file in the report root listing every benchmark analyzed, its pass/fail status, elapsed time, and the count of artifacts produced.
- **FR-014**: During execution, the tool MUST print one progress line per benchmark when it starts and one when it completes (with status and elapsed time), e.g. `[1/4] Running BM_g_polybench_n_correlation... done (2.3s)`.

### Key Entities

- **Report directory**: Timestamped top-level container for a single `./analyze.py` run; contains one subdirectory per executed benchmark.
- **Benchmark subdirectory**: Named after the fully-qualified benchmark name with all non-alphanumeric characters (semicolons, colons, spaces) replaced by underscores (e.g., `BM_g_polybench_n_correlation_s_EXTRALARGE_t_jit_analysis_`); holds all artifacts for that benchmark.
- **Specialized ASM**: Disassembly of the JIT-compiled, runtime-specialized version of the target function.
- **Original ASM**: Disassembly of the AOT-compiled version of the same function extracted from the benchmark binary's ELF symbol table.
- **Original ASM**: Disassembly of the AOT-compiled version of the same function extracted from the benchmark binary's ELF symbol table, including all functions it transitively calls or tail-calls (BFS, cap 50).
- **Pass trace plot**: A rendered visual showing per-pass instruction/function/BB counts and timings across the JIT pipeline.
- **Chrome trace**: A JSON file compatible with Chrome's tracing viewer showing JIT pipeline timing in a flame-graph format.
- **Summary file** (`summary.md`): Markdown index at the report root listing every benchmark run, its pass/fail status, elapsed time, and artifact counts. Written after all benchmarks complete.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: A developer can run a full multi-benchmark analysis for a benchmark group with a single command and without specifying any function symbol names manually.
- **SC-002**: Each benchmark produces a complete set of enabled artifacts (original ASM with callees, specialized ASM with function header, pass trace plot, chrome trace) in the correct subdirectory without manual post-processing.
- **SC-003**: Running the tool twice in succession produces two independent, non-overlapping report directories with distinct timestamps.
- **SC-004**: When one benchmark fails, the remaining benchmarks still produce their full artifact sets (failure isolation: zero cross-benchmark contamination).
- **SC-005**: The tool completes benchmark discovery, filter construction, and report-directory setup in under 5 seconds before the first benchmark invocation begins.

## Assumptions

- The benchmark binary uses Google Benchmark's `--benchmark_list_tests` (or equivalent) output format to enumerate benchmark names, and accepts a `--benchmark_filter` argument for filtering.
- Benchmark names encode their metadata as semicolon-delimited key:value tags (e.g., `t:jit_analysis`, `g:polybench`, `n:correlation`), consistent with the existing naming convention in the codebase.
- The JIT writes specialized ASM files when the `CRS_ASM_DUMP_DIR` environment variable is set, naming each file `<function_name>__specialized.asm`; this naming convention is stable and used for auto-discovery.
- The `CRS_PASS_TRACE_DIR` and `CRS_CHROME_TRACE_DIR` environment variables are already supported by the JIT and benchmark infrastructure; no runtime changes are needed to enable those outputs.
- `llvm-objdump` or `objdump` is available on PATH for extracting original function ASM from the ELF binary.
- The existing `pass_trace_plot.py` script (or its equivalent) can be invoked to render a pass trace JSON file into a plot; the analyze.py script drives it as a subprocess or by importing it.
- The tool is invoked from the root of the `benchmarks/` directory (or a subdirectory thereof), so `reports/` is a sibling of the benchmark binary.
- Mobile/Windows support is out of scope; the tool targets Linux x86-64 with an ELF binary.

## Clarifications

### Session 2026-05-07

- Q: How should each benchmark's output subdirectory be named? → A: Full sanitized name — all non-alphanumeric characters (semicolons, colons, spaces) replaced by underscores, e.g. `BM_g_polybench_n_correlation_s_EXTRALARGE_t_jit_analysis_`.
- Q: When a jit_analysis benchmark produces more than one `*__specialized.asm` file, how should they be handled? → A: Treat each as a separate artifact pair — one (orig, spec, diff) triplet per file, all placed in the same benchmark subdirectory.
- Q: Should the tool enforce a per-benchmark execution timeout? → A: Yes, configurable via `--timeout=N` flag with a default of 60 seconds.
- Q: Should the tool write a top-level summary file in the report directory? → A: Yes, write `summary.md` listing each benchmark, its pass/fail status, elapsed time, and artifact counts.
- Q: How verbose should progress output be during execution? → A: One line per benchmark on start and completion, e.g. `[1/4] Running BM_... done (2.3s)`.
