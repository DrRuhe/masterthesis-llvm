# Data Model: Unified JIT Analysis Launcher (006-analyze-launcher)

This feature is entirely filesystem-based. No database schema changes. The entities below are Python dataclasses used internally by `analyze.py`.

## Core Entities

### AnalysisConfig

Holds the user's CLI input for one `./analyze.py` invocation.

| Field | Type | Default | Source |
|-------|------|---------|--------|
| `binary` | `Path` | — | positional argument |
| `filter` | `str` | — | `--benchmarks_filter` |
| `timeout_s` | `int` | `60` | `--timeout` |
| `asm` | `bool` | `True` | `not --no-asm-dump` |
| `pass_trace` | `bool` | `True` | `not --no-pass-trace` |
| `chrome_trace` | `bool` | `True` | `not --no-chrome-trace` |

### BenchmarkRun

Tracks execution state and artifact inventory for one `jit_analysis` benchmark.

| Field | Type | Description |
|-------|------|-------------|
| `name` | `str` | Fully-qualified benchmark name (e.g., `BM_g:polybench;n:correlation;s:EXTRALARGE;t:jit_analysis;`) |
| `sanitized_name` | `str` | `name` with all non-alphanumeric characters replaced by `_` |
| `subdir` | `Path` | `report_dir / sanitized_name` |
| `status` | `Literal["pass", "fail", "timeout"]` | Outcome after execution and artifact routing |
| `elapsed_s` | `float` | Wall-clock time of the benchmark subprocess |
| `artifacts` | `list[Path]` | Paths of all files written under `subdir` |
| `error_msg` | `str \| None` | Populated on `fail` or `timeout`; `None` on `pass` |

### AnalysisReport

Top-level container for a single `analyze.py` run.

| Field | Type | Description |
|-------|------|-------------|
| `report_dir` | `Path` | `reports/<YYYYMMDD-hhmmss>-analysis/` relative to CWD |
| `benchmarks` | `list[BenchmarkRun]` | One entry per discovered `jit_analysis` benchmark |
| `start_time` | `datetime` | Wall-clock start of the analysis run |

## State Transitions for BenchmarkRun.status

```
[discovered]
     │
     ▼
  [running] ──── timeout ────► timeout
     │
     ├── non-zero exit ──────► fail
     ├── expected artifact
     │   missing ────────────► fail
     │
     └── all expected artifacts
         present ────────────► pass
```

## Filesystem Layout (runtime output)

```text
reports/
└── YYYYMMDD-hhmmss-analysis/
    ├── summary.md                                       # written last; lists all benchmarks
    ├── _staging/                                        # temp; removed after artifact routing
    │   ├── asm/                                         # CRS_ASM_DUMP_DIR
    │   ├── pass_trace/                                  # CRS_PASS_TRACE_DIR
    │   └── chrome_trace/                                # CRS_CHROME_TRACE_DIR
    └── BM_g_polybench_n_correlation_s_EXTRALARGE_t_jit_analysis_/
        ├── orig_kernel_correlation.asm                  # original ELF ASM (if asm enabled)
        ├── kernel_correlation__specialized.asm          # JIT-specialized ASM (if asm enabled)
        ├── diff_kernel_correlation.diff                 # unified diff (if asm enabled)
        ├── <sanitized_name>_pass_trace.json             # pass trace JSON (if pass_trace enabled)
        ├── pass_trace_plot.pdf                          # rendered plot (if pass_trace enabled)
        └── <sanitized_name>_chrome_trace.json           # chrome trace (if chrome_trace enabled)
```

## Artifact naming conventions

| Artifact | Filename pattern | Produced by |
|----------|-----------------|-------------|
| Original ASM | `orig_<function_name>.asm` | `analyze.py` via `llvm-objdump` |
| Specialized ASM | `<function_name>__specialized.asm` | JIT engine via `CRS_ASM_DUMP_DIR` |
| Normalized diff | `diff_<function_name>.diff` | `analyze.py` via `difflib.unified_diff` |
| Pass trace JSON | `<sanitized_bm_name>_pass_trace.json` | `writePassTraceJSON` in benchmark binary |
| Pass trace plot | `pass_trace_plot.pdf` | `reporting/pass_trace_plot_json.py` |
| Chrome trace | `<sanitized_bm_name>_chrome_trace.json` | `benchmarkJITAnalysis` via `CRS_CHROME_TRACE_DIR` |

## summary.md format

```markdown
# Analysis Report — YYYYMMDD-hhmmss

| # | Benchmark | Status | Elapsed (s) | Artifacts |
|---|-----------|--------|-------------|-----------|
| 1 | BM_g:polybench;n:correlation;... | pass | 2.3 | 6 |
| 2 | BM_g:polybench;n:covariance;...  | fail | 1.1 | 0 |
```
