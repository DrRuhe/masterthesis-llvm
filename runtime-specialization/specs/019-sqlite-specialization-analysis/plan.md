# Plan: SQLite Specialization Failure Analysis

## Spec + plan files
- [x] Write `specs/019-sqlite-specialization-analysis/spec.md`
- [x] Write `specs/019-sqlite-specialization-analysis/details.md`
- [x] Write this `specs/019-sqlite-specialization-analysis/plan.md`

## Check large-module guard
- [x] Inspect `JITPipelineIPSCCP.cpp` for any `IsLargeModule` guard on `InvariantLoadToConstantPass`. Record finding in `details.md`.

## Instrumentation: !invariant.load count
- [x] In `StaticMutabilityAnalysis.cpp`: add CRS_LOG (via `llvm::errs()`) emitting per-function count of annotated loads after `inferReadOnlyFields()`.
- [x] In `JitIPSCCPPass.cpp`: add post-solve count of `!invariant.load` loads and how many have constant lattice values.
- [x] In `InvariantLoadToConstantPass.cpp`: add counter for successful host-memory replacements.
- [x] Rebuild `ClangRuntimeSpecializer` in debug mode (and release).

## Instrumentation: constant-arg inlining count
- [x] In `ConstantArgFunctionSpecializationPass.cpp`: emit `Groups.size()` per function.
- [x] Rebuild.

## Collect !invariant.load + constant data
- [x] Run TPCHBenchmark Q1 (debug) and capture `[CRS-STAT]` output. Record: (a) annotated=0 for sqlite3, (b) SCCP folds=0, (c) InvariantLoad replacements=0 (pass skipped), (d) ConstantArg not in P2 pipeline.
- [x] Ran `simple_scan` as baseline: identical results.
- [x] Recorded in `details.md`.

## ASM comparison via analyze.py
- [x] Build `TPCHBenchmark` in release mode.
- [x] Run Q1 `jit_analysis` with `CRS_ASM_DUMP_DIR` directly (analyze.py doesn't support --db flag; ran binary manually with env vars). Output: `benchmarks/reports/260611-1528-sqlite-analysis/`.
- [x] Compared ASM: original=9187 lines (full dispatch loop), specialized=513 lines (7-instruction trampoline + padding). Dispatch switch fully preserved. Recorded in `details.md`.
- [x] simple_scan baseline: identical trampoline pattern.

## Pass-trace inspection
- [x] Ran Q1 with `CRS_PASS_TRACE_DIR`. Key finding: `JitIPSCCPPass` changes 0 instructions. `InvariantLoadToConstantPass` absent from "final" group (skipped). Recorded in `details.md`.

## Root cause analysis and report
- [x] Written `specs/019-sqlite-specialization-analysis/report.md`.

## Verification
- [x] `details.md` contains numeric data for Q1 and `simple_scan`.
- [x] `benchmarks/reports/260611-1528-sqlite-analysis/asm/` contains `sqlite3VdbeExec__specialized.asm` (513 lines) and `sqlite3VdbeExec__original.asm` (9187 lines).
- [x] `report.md` answers all three sub-questions with cited evidence and states root cause conclusion.

## Follow-up: IPSCCP constant propagation for Vdbe pointer
- [x] Add `InferStats` struct and escape/mutation breakdown to `StaticMutabilityAnalysis`.
- [x] Add `ArgLattice` (is sqlite3VdbeExec definition or declaration?) + `ModuleStores` to `JitIPSCCPPass`.
- [x] Collect data: `ArgLattice: is_declaration=1, call_sites_with_const_arg0=1`.
- [x] Extend `report.md` with follow-up section covering blob collision, escape analysis breakdown, and fundamental dispatch barrier (RC-1).
