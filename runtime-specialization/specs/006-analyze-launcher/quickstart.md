# Quickstart: Unified JIT Analysis Launcher (006-analyze-launcher)

## Prerequisites

- A built benchmark binary (e.g., `./AllBenchmarks`) with `jit_analysis` benchmarks registered.
- `llvm-objdump` or `objdump` on `PATH` (required for original ASM extraction; auto-skipped if `--no-asm-dump`).
- Python dependencies: `matplotlib`, `ultraplot` (already in `flake.nix`).

## Basic usage

```bash
cd llvm/runtime-specialization/benchmarks

# Run full analysis for all polybench jit_analysis benchmarks:
./analyze.py ./AllBenchmarks --benchmarks_filter="g:polybench"
```

Output: `reports/YYYYMMDD-hhmmss-analysis/` with one subdirectory per matched `jit_analysis` benchmark.

## Selective output

```bash
# ASM diffs only (skip pass trace and chrome trace):
./analyze.py ./AllBenchmarks --benchmarks_filter="g:polybench" --no-pass-trace --no-chrome-trace

# Pass trace and chrome trace only (no ASM diff):
./analyze.py ./AllBenchmarks --benchmarks_filter="g:polybench" --no-asm-dump

# All outputs disabled (creates subdirs and summary.md but no artifact files):
./analyze.py ./AllBenchmarks --benchmarks_filter="g:polybench" --no-asm-dump --no-pass-trace --no-chrome-trace
```

## Timeout control

```bash
# Allow up to 120 s per benchmark (default is 60 s):
./analyze.py ./AllBenchmarks --benchmarks_filter="g:polybench" --timeout=120
```

## Reading the report

After a run:

```text
reports/20260507-143022-analysis/
├── summary.md                         ← open first: pass/fail, timing, artifact counts
└── BM_g_polybench_n_correlation_.../
    ├── orig_kernel_correlation.asm    ← AOT-compiled ASM from ELF
    ├── kernel_correlation__specialized.asm  ← JIT-specialized ASM
    ├── diff_kernel_correlation.diff   ← unified diff (- orig, + specialized)
    ├── *_pass_trace.json              ← per-pass pipeline metrics
    ├── pass_trace_plot.pdf            ← pipeline visualization
    └── *_chrome_trace.json            ← open in chrome://tracing
```

## Running twice (incremental)

Each invocation creates a fresh `reports/<new-timestamp>-analysis/` directory. Previous reports are never overwritten (SC-003).

## Environment variables managed internally

The following env vars are set only in the subprocess environment; they are not exported to the parent shell.

| Variable | Target |
|----------|--------|
| `CRS_ASM_DUMP_DIR` | `<report_dir>/_staging/asm/` |
| `CRS_PASS_TRACE_DIR` | `<report_dir>/_staging/pass_trace/` |
| `CRS_CHROME_TRACE_DIR` | `<report_dir>/_staging/chrome_trace/` |

## CLI reference

```
./analyze.py <binary> [options]

Positional:
  binary                    Path to the benchmark executable.

Options:
  --benchmarks_filter STR   Filter forwarded to --benchmark_filter (Google Benchmark).
                            Use key:value tags, e.g. "g:polybench" or "n:correlation".
  --timeout N               Per-benchmark wall-clock timeout in seconds (default: 60).
  --no-asm-dump             Disable original ASM extraction and diff generation.
  --no-pass-trace           Disable pass trace JSON collection and plot rendering.
  --no-chrome-trace         Disable chrome trace JSON collection.
  -h, --help                Show this help.
```
