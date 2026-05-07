# Implementation Plan: Unified JIT Analysis Launcher

**Branch**: `006-analyze-launcher` | **Date**: 2026-05-07 | **Spec**: `specs/006-analyze-launcher/spec.md`
**Input**: Feature specification from `specs/006-analyze-launcher/spec.md`

## Summary

Create `benchmarks/analyze.py` — a Python CLI that orchestrates the full JIT analysis pipeline for a benchmark group: discovers `jit_analysis` benchmarks via `--benchmark_list_tests`, runs them in one subprocess invocation with `CRS_ASM_DUMP_DIR`/`CRS_PASS_TRACE_DIR`/`CRS_CHROME_TRACE_DIR` env vars set to a staging directory, extracts original function ASM from the ELF via `llvm-objdump`, produces normalized diffs, renders pass trace plots from JSON, and writes everything into a timestamped `reports/<ts>-analysis/` directory with per-benchmark subdirs and a `summary.md`. Requires a new `reporting/pass_trace_plot_json.py` module that renders pass trace plots directly from the standalone JSON files written by `benchmarkJITAnalysis` (bypassing DuckDB).

## Technical Context

**Language/Version**: Python 3.11+ (per `flake.nix`)
**Primary Dependencies**: stdlib only (`subprocess`, `pathlib`, `argparse`, `datetime`, `re`, `shutil`); `reporting/pass_trace_plot_json.py` (new module for JSON-mode plot rendering); `reporting/report_utils.py` (shared helpers already available)
**Storage**: Filesystem — `reports/<YYYYMMDD-hhmmss>-analysis/` tree created at runtime; no database changes
**Testing**: Manual acceptance tests against a built benchmark binary (Python orchestration tooling falls outside the lit/FileCheck scope by project precedent; see Constitution Check)
**Target Platform**: Linux x86-64, ELF binary, `llvm-objdump` or `objdump` on PATH
**Project Type**: CLI tool (Python script, peer to `record_benchmark.py` and `optimize_benchmarks.py`)
**Performance Goals**: Orchestration overhead < 5 s before first benchmark invocation (SC-005)
**Constraints**: Per-benchmark timeout configurable via `--timeout=N` (default 60 s); failure of one benchmark must not block others (FR-010)

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Principle | Status | Notes |
|-----------|--------|-------|
| I. Correctness & Safety | ✅ PASS | Python orchestration only; zero changes to the JIT engine or public C++ API |
| II. LLVM Coding Standards | ✅ PASS | Python file; LLVM C++ standards (Expected, UniquePtr, etc.) do not apply |
| III. Test-First Validation | ⚠️ JUSTIFIED PARTIAL | `analyze.py` is orchestration tooling, not a JIT code path. `lit`/`FileCheck` is inapplicable. Acceptance: end-to-end manual runs against a built binary validating User Stories 1–4. Project precedent: `record_benchmark.py` and `optimize_benchmarks.py` are equally untested by lit. |
| IV. Performance Measurement | ✅ PASS | `summary.md` records elapsed time and artifact count per benchmark; no ad-hoc timing injected into C++ production paths |
| V. Minimal Public API | ✅ PASS | No additions to the C++ public API; CLI flags are exactly those required by the spec |
| Backwards Compatibility | ✅ PASS | `reports/` is an additive-only output directory; no schema or library changes; no DuckDB migration needed |

**GATE RESULT**: ✅ PASS — all principles satisfied or justified.

**Post-design re-check**: Design introduces no C++ changes, no DuckDB schema changes, no API surface additions. Gate remains green.

## Project Structure

### Documentation (this feature)

```text
specs/006-analyze-launcher/
├── plan.md              # This file
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
└── tasks.md             # Phase 2 output (/speckit-tasks — NOT created here)
```

### Source Code (repository-relative)

```text
benchmarks/
├── analyze.py                              # NEW — the CLI launcher (this feature)
└── reporting/
    ├── pass_trace_plot_json.py             # NEW — JSON-mode pass trace renderer
    └── pass_trace_plot.py                  # UNCHANGED (DuckDB-backed; stays as-is)
```

**Structure Decision**: Two new files only. No new directories. `analyze.py` lives alongside `record_benchmark.py` and `optimize_benchmarks.py` as a peer orchestration script. Pass trace rendering is split into a new focused module rather than modifying the existing DuckDB-backed script (see research.md Decision 1).

## Complexity Tracking

No constitution violations to justify.
