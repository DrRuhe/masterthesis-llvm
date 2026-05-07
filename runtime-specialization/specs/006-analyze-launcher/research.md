# Research: Unified JIT Analysis Launcher (006-analyze-launcher)

## Decision 1: Pass trace plot rendering — JSON-mode vs. DuckDB extension

**Decision**: Add a new standalone module `reporting/pass_trace_plot_json.py` with a `plot_from_json(json_path: Path, output_path: Path) -> None` function. Do not modify `reporting/pass_trace_plot.py`.

**Rationale**:
- `benchmarkJITAnalysis` in `ClangRuntimeSpecializerBenchmark.h` writes a standalone JSON file via `writePassTraceJSON(state.name(), trace)`. This JSON is NOT imported into DuckDB during the analysis run; it is a direct file artifact.
- The existing `pass_trace_plot.py` reads pass trace data from the DuckDB `pass_traces` table via SQL. It has no JSON-file entry point.
- The JSON format (array of objects with `name`, `group`, `fixpoint_iter`, `fns_before/after`, `instrs_before/after`, `bbs_before/after`, `wall_time_ms`, `ir_changed`) contains all fields the plot needs. Reading it with `json.load` and converting to a `pd.DataFrame` is trivial.
- Keeping the modules separate avoids mixing two execution contexts (DuckDB-backed vs. filesystem-backed) in one file and prevents any risk of breaking the existing reporting workflow (`record_benchmark.py`/`reporting/`).

**Alternatives considered**:
- (a) Add `--json FILE` flag to `pass_trace_plot.py`: Rejected — conflates DuckDB mode and JSON mode; complicates CLI and internal data flow.
- (b) Import `pass_trace_plot` internals and monkey-patch the query functions: Rejected — too fragile; would break on any refactor of the plotting code.

---

## Decision 2: Artifact routing — staging dir approach

**Decision**: Use a two-phase staging approach to satisfy FR-003 (single invocation) while routing artifacts to per-benchmark subdirectories.

**Phase A — single invocation**: Create `<report_dir>/_staging/asm/`, `_staging/pass_trace/`, `_staging/chrome_trace/`. Set the three `CRS_*` env vars to those paths. Run all selected `jit_analysis` benchmarks with a single `--benchmark_filter=<name1>|<name2>|...` invocation.

**Phase B — routing**: After the invocation, for each benchmark:
1. **Pass trace JSON**: Look for `<sanitized_name>_pass_trace.json` in `_staging/pass_trace/`. The benchmark name sanitized with the same non-alphanumeric→`_` rule used by `writePassTraceJSON` in `ClangRuntimeSpecializerBenchmark.h` gives a deterministic filename.
2. **Chrome trace JSON**: Look for `<sanitized_name>_chrome_trace.json` in `_staging/chrome_trace/`. Same sanitization rule, from `benchmarkJITAnalysis`.
3. **Specialized ASM**: `CRS_ASM_DUMP_DIR` yields `<function_name>__specialized.asm` files. Match to the owning benchmark by extracting the `n:<kernel>` value from the benchmark name and searching for `*<kernel>*__specialized.asm` in `_staging/asm/`. This relies on the convention that function names contain the kernel name as a substring (e.g., `n:correlation` → `kernel_correlation__specialized.asm`). If no match, log a warning and continue.

The `_staging/` directory is removed after all artifacts are routed.

**Rationale**: FR-003 requires a single invocation for efficiency (one JIT init, one startup cost). The pass trace and chrome trace filenames are fully deterministic from the benchmark name. ASM routing requires a heuristic because `CRS_ASM_DUMP_DIR` is shared across all benchmarks in one invocation and ASM filenames are keyed by function name (not benchmark name).

**Alternatives considered**:
- (a) One subprocess per benchmark, each with its own subdir as the env var target: Violates FR-003.
- (b) Change JIT engine to embed benchmark name in ASM filename: Out of scope; requires C++ changes; the analyze.py spec explicitly excludes runtime changes.
- (c) Copy all ASM files into every benchmark subdir: Creates incorrect many-to-many mapping; confuses downstream readers.

---

## Decision 3: Original ASM extraction — inline Python vs. asm-compare.sh

**Decision**: Implement ASM extraction inline in Python. Do not call `asm-compare.sh` as a subprocess.

**Method**: Run `subprocess.run([objdump_path, '-d', '--no-show-raw-insn', binary])`, capture stdout, and extract the target function's section using a Python regex that mirrors the AWK logic in `asm-compare.sh`:
```
start: line matching r'^\S.*<FUNC_NAME>:'
end:   next line matching r'^\S.*<[^>]+>:' (different function)
```
Normalization: strip leading address+colon, collapse whitespace, drop blank lines (mirrors `asm-compare.sh`'s `normalize()`).

**Rationale**: `asm-compare.sh` is batch-unfriendly (one call per function, prints to stdout, not designed for programmatic use). The Python reimplementation does the same work more cleanly, handles missing symbols gracefully, and avoids a bash subprocess.

**objdump discovery**: `shutil.which('llvm-objdump') or shutil.which('objdump')`. If neither is found and ASM dumping is enabled, exit with a clear error (FR spec Assumption 5 / edge case spec).

---

## Decision 4: Benchmark listing and jit_analysis filtering

**Decision**: Use `[binary, '--benchmark_list_tests', '--benchmark_filter=<user_filter>']` to enumerate all matching benchmark names, then keep only those where the `t:` key equals `jit_analysis`. Implement `_parse_bm_name` inline (not imported from `optimize_benchmarks.py`).

**Rationale**: `optimize_benchmarks.py` already uses `--benchmark_list_tests` for the same purpose; the pattern is well-established. Importing from `optimize_benchmarks.py` would bring in heavy dependencies (DuckDB, numpy) not needed here. The `_parse_bm_name` helper is a simple regex; duplicating the ~10 lines is preferable to a cross-script import.

**Benchmark name format**: `BM_<key1>:<val1>;<key2>:<val2>;...` where `t:jit_analysis` is the type tag. The listing output is line-separated names (skip the `Benchmark` header line and empty lines).

---

## Decision 5: Combined filter construction

**Decision**: Join all selected benchmark names with `|` for `--benchmark_filter`. Google Benchmark interprets `|` as an OR operator in filter patterns.

**Exact match**: Use the exact full name (e.g., `BM_g:polybench;n:correlation;s:EXTRALARGE;t:jit_analysis;`) as the filter component. This avoids partial matches from substring-only filters.

**Rationale**: Running all selected benchmarks in one invocation (FR-003) requires a filter that matches exactly those benchmarks. `|` gives OR semantics without substring ambiguity when exact names are used.

---

## Decision 6: Summary file format

**Decision**: Write `summary.md` as a Markdown table in the report root after all benchmarks complete. Columns: `Benchmark`, `Status`, `Elapsed (s)`, `Artifacts`.

**Rationale**: Markdown is human-readable inline and renderable in GitHub/IDE. The `summary.md` acts as the first thing a developer opens after a run to see which benchmarks passed.

---

## Resolved Assumptions

- `--benchmark_list_tests` is available in the benchmark binary's Google Benchmark version. Confirmed: used by `optimize_benchmarks.py` today.
- `CRS_PASS_TRACE_DIR`, `CRS_CHROME_TRACE_DIR`, `CRS_ASM_DUMP_DIR` are respected by the benchmark binary. Confirmed: `benchmarkJITAnalysis` reads `CRS_PASS_TRACE_DIR` and `CRS_CHROME_TRACE_DIR`; the JIT engine reads `CRS_ASM_DUMP_DIR` (line 821 of `ClangRuntimeSpecializer.cpp`). No runtime changes needed.
- `pass_trace_plot.py` uses `ultraplot` and `matplotlib` already present in `flake.nix`. Confirmed: existing script imports both.
- The pass trace JSON written by `writePassTraceJSON` includes a `pass_idx` field. **Correction**: the JSON does NOT include `pass_idx`; it is an array and the index is implicit (array position). `pass_trace_plot_json.py` will derive `pass_idx` from `enumerate()`.
