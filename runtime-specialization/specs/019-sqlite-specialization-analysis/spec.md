# Spec 019: SQLite Specialization Failure Analysis

## Overview

Spec 018 established that TPC-H queries on SQLite (`sqlite3VdbeExec`) show no
execution speedup from JIT specialization (unspecialized ~600 ms, specialized
~630 ms). The design hypothesis was that specializing on a known `Vdbe*` pointer
would allow IPSCCP to constant-fold the opcode dispatch switch, eliminating
irrelevant handlers. This spec documents an empirical investigation to determine
*why* no speedup is observed, by measuring the pipeline's actual effect on the
SQLite IR and comparing specialized vs. unspecialized native code.

## User Scenarios

**Scenario A — Counting invariant-load annotations**
Given the SQLite specialization pipeline runs on a Q1 query,
when the JIT executes `StaticMutabilityAnalysis`,
then I can read from logs how many loads were annotated `!invariant.load`,
how many SCCP visited, and how many `InvariantLoadToConstantPass` replaced.

**Scenario B — Counting constant inlinings**
Given the same pipeline run,
when `ConstantArgFunctionSpecializationPass` runs,
then I can read from logs how many distinct constant-arg patterns were detected.

**Scenario C — ASM comparison**
Given `analyze.py` is run against the release `TPCHBenchmark` binary for Q1,
when the tool produces specialized and original ASM for `sqlite3VdbeExec`,
then I can compare whether the dispatch switch structure differs and record
the line-count delta.

**Scenario D — Report**
Given the collected data,
then a `report.md` exists in this directory that answers the three sub-questions
with concrete numbers and states the proven or most-likely root cause.

## Functional Requirements

- **FR-001**: Instrumentation logs emitted by `StaticMutabilityAnalysis` MUST report
  the total number of loads annotated `!invariant.load` per-module for each JIT call.
- **FR-002**: Instrumentation logs MUST report how many `!invariant.load`-marked loads
  the JitSCCPSolver visited and attempted to fold.
- **FR-003**: Instrumentation logs MUST report how many loads `InvariantLoadToConstantPass`
  successfully replaced with IR constants, and how many `ConstantArgFunctionSpecializationPass`
  specialization groups were created per function.
- **FR-004**: `analyze.py` MUST be runnable against `TPCHBenchmark` for Q1 to produce
  specialized and original ASM files for `sqlite3VdbeExec`.

## Success Criteria

- **SC-001**: `details.md` contains numeric instrumentation data for Q1 and `simple_scan`
  (at minimum: annotated loads, SCCP visits, InvariantLoad replacements, ConstantArg groups).
- **SC-002**: The analyze.py report directory for Q1 contains at least one `*sqlite3VdbeExec*`
  specialized ASM file and a corresponding original ASM file.
- **SC-003**: `report.md` exists, answers all three sub-questions with cited evidence from
  SC-001/SC-002 data, and states a root cause conclusion with supporting rationale.

## Edge Cases

- SQLite is classified as "large module" (far more than the default 3-instruction threshold
  after pruning). Any pipeline component gated on `IsLargeModule` may be silently skipped —
  this must be checked explicitly (FR-002/FR-003).
- `InvariantLoadToConstantPass` was documented as running only on small modules (spec-005);
  verify current code matches this description.
- The dispatch loop in `sqlite3VdbeExec` iterates over `p->aOp[pc]` where `pc` changes
  every iteration — even a constant `p` does not make `pc` static.

## Clarifications

<!-- Appended during investigation -->
