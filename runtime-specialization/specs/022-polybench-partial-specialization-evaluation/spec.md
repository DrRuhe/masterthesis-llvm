# PolyBench Partial-Specialization Evaluation

## Overview

This feature reevaluates the PolyBench benchmark suite using partial runtime specialization instead of the current "specialize every benchmark argument" setup. The goal is to model more realistic repeated-call scenarios where only a stable subset of kernel inputs remains fixed across calls, then use that setup to collect thesis-grade default-pipeline measurements, size-scaling plots, and appendix material explaining the specialization choice for each kernel.

## User Scenarios

1. A benchmark developer wants PolyBench specialized-execution benchmarks to model repeated calls with stable state and changing request-specific inputs.
   Acceptance scenario:
   Given a PolyBench kernel benchmark,
   When the specialized benchmark is registered,
   Then the specialization path uses a lambda-based partial-specialization setup where some kernel state is captured and at least one semantically relevant input remains runtime-variable whenever the kernel has such an input in the exposed benchmark model.

2. A thesis author wants a reproducible best-practice study for PolyBench under the default pipeline.
   Acceptance scenario:
   Given a built `PolyBenchBenchmark` binary,
   When the thesis runner script is executed,
   Then it records a DuckDB ablation study with PolyBench `unspecialized`, `jit_overhead`, and `specialized_exec` rows for `SMALL`, `MEDIUM`, `LARGE`, and `EXTRALARGE` under best-practice benchmark controls.

3. A thesis author wants figure-generation and appendix material from that study.
   Acceptance scenario:
   Given a completed PolyBench partial-specialization study,
   When the reporting script is executed,
   Then it emits PolyBench amortized-speedup and JIT-time size-scaling plots, emits a per-kernel summary table when the data is sufficiently size-stable, and provides appendix-ready specialization-decision material.

## Functional Requirements

- FR-001: PolyBench specialized-execution benchmarks MUST stop using the tuple-only `benchmarkSpecializedExec` path and instead use a lambda-based path that can specialize only part of the kernel state.
- FR-002: The PolyBench benchmark infrastructure MUST provide reusable helpers for lambda-based `jit_overhead` and `specialized_exec` measurements that preserve the existing benchmark naming and timing conventions.
- FR-003: The PolyBench partial-specialization setup MUST preserve the current benchmark phases and benchmark name schema: `unspecialized`, `jit_overhead`, `specialized_exec`, and `jit_analysis`.
- FR-004: The PolyBench partial-specialization setup MUST support all 30 PolyBench kernels currently registered in `polybench_bench.cpp`.
- FR-005: The new spec directory’s `details.md` MUST document, for every PolyBench kernel, which inputs are specialized, which remain runtime-variable, and which repeated-call scenario that split represents.
- FR-006: A dedicated thesis runner script MUST record a default-pipeline PolyBench study in DuckDB using best-practice controls and the size tiers `SMALL`, `MEDIUM`, `LARGE`, and `EXTRALARGE`.
- FR-007: A dedicated reporting script MUST generate one amortized-speedup-vs-size plot and one JIT-time-vs-size plot from the recorded PolyBench study.
- FR-008: The reporting pipeline MUST generate a summary table with amortized speedup and `$U_p$` only when the PolyBench size-scaling behavior is stable enough to justify that summary, and MUST otherwise surface that the summary was skipped.
- FR-009: The thesis appendix MUST gain a section that lists the PolyBench specialization decision for each kernel and the real-world scenario it represents.
- FR-010: `specs/TODO.md` MUST be updated to track this PolyBench reevaluation as an evaluation/thesis follow-up task.

## Success Criteria

- SC-001: All 30 PolyBench kernels build and register valid `unspecialized`, `jit_overhead`, `specialized_exec`, and `jit_analysis` benchmarks after the partial-specialization refactor.
- SC-002: The PolyBench thesis runner completes a best-practice default-pipeline study covering the four requested size tiers.
- SC-003: The reporting script emits thesis-ready PolyBench size-scaling artifacts into `benchmarks/reports/thesis-figures/rq3/`.
- SC-004: The appendix section in `docs/thesis.typ` references a complete per-kernel rationale rather than only aggregate PolyBench results.

## Edge Cases

- Kernels whose benchmark-visible parameter list has only one scalar size parameter cannot leave a benchmark size argument partially specialized in the same way as higher-arity kernels; for those kernels, the documented specialization subset may rely on stable runtime state captured into the lambda closure while leaving the exposed size parameter dynamic.
- If the size-scaling data is not stable enough to support a single per-kernel `$U_p$` summary, the reporting flow must still produce the two plots and must not fabricate a summary table.
- The dedicated PolyBench flow must keep using `PolyBenchBenchmark`, because prior repo notes indicate that `AllBenchmarks` is not the valid execution path for PolyBench measurements.

## Clarifications

### Session 2026-06-23

- Use size tiers `SMALL`, `MEDIUM`, `LARGE`, and `EXTRALARGE` for the thesis study and plots; exclude `MINI`.
- Treat "remain constant across input size" qualitatively. The reporting flow may use a conservative heuristic to decide whether to emit the summary table, but the thesis prose must still frame stability as an evidence judgment rather than a hard theorem.
