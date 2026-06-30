# Paper-Code Audit: `docs/thesis.typ` vs `llvm/runtime-specialization`

Date: 2026-06-25
Mode: Static audit only
Paper artifact: `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/thesis.typ`
Repo artifact: `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization`

## Scope

This audit compares thesis claims against the current implementation, benchmark
artifacts, and thesis-tracker documents. It focuses on:

- runtime API and specialization constraints,
- JIT pipeline defaults and behavior,
- benchmark-backed evaluation claims,
- thesis TODOs or stale wording that contradict local evidence.

No builds, tests, or benchmark reruns were executed for this audit.

## Findings

### High

1. **Implementation chapter still says Pipeline 0 is the default, but the code and evaluation use Pipeline 2 as the default.**
   - Thesis claim: [thesis.typ](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/thesis.typ:828)
   - Code default: [ClangRuntimeSpecializer.h](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h:355), [ClangRuntimeSpecializer.h](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h:388)
   - Evaluation narrative: [thesis.typ](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/thesis.typ:1708)

2. **The thesis understates the single-threading limitation.**
   - Current prose mainly frames the risk as concurrent mutation of specialized arguments.
   - The implementation also has unsynchronized singleton state used during JIT compilation, and timeout mode explicitly leaves a detached background compile running.
   - Evidence: [thesis.typ](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/thesis.typ:1759), [ClangRuntimeSpecializer.h](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h:706), [ClangRuntimeSpecializer.h](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h:717), [ClangRuntimeSpecializer.cpp](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.cpp:335)

3. **The “annotation-free” API description is too broad for the actual implementation.**
   - The public entry points only work when the Clang plugin rewrites the call site.
   - Runtime-variable function pointers are unsupported.
   - Generic lambdas are unsupported.
   - Evidence: [thesis.typ](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/thesis.typ:482), [ClangRuntimeSpecializer.h](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h:531), [ClangRuntimeSpecializer.h](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h:897), [IRRewritingPass.cpp](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/comptime/IRRewritingPass.cpp:231), [speconly-runtime-funcptr-error.cpp](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/test/smoke/speconly-runtime-funcptr-error.cpp:1)

4. **The default-pipeline argument still relies on invalidated SQLite evidence.**
   - The thesis uses SQLite/TPC-H behavior to justify P2 as the safer default.
   - The repo’s later SQLite analysis shows the benchmark path was specializing the wrong blob because of a blob-registration collision.
   - Evidence: [thesis.typ](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/thesis.typ:1708), [thesis.typ](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/thesis.typ:1826), [specs/019-sqlite-specialization-analysis/report.md](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/specs/019-sqlite-specialization-analysis/report.md:208)

### Medium

5. **The implementation chapter overstates what gets embedded as IR.**
   - The pass actually serializes a preprocessed, pruned clone after linkage rewriting and `GlobalDCE`.
   - Evidence: [thesis.typ](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/thesis.typ:503), [IRDumpingPass.cpp](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/comptime/IRDumpingPass.cpp:85), [IRDumpingPass.cpp](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/comptime/IRDumpingPass.cpp:182)

6. **The thesis presents function-to-blob lookup as deterministic, but the runtime still uses “last registered wins” on collisions.**
   - Evidence: [thesis.typ](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/thesis.typ:768), [ClangRuntimeSpecializer.cpp](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.cpp:640)

7. **The headline UC pipeline-comparison numbers are ambiguous between oracle-style and deployable-config results.**
   - Thesis headline: `1.748x` vs `1.824x`
   - Tracker-backed deployable comparison: `default=1.665x`, `p0_o3_optimal=1.740x`, `uc_workload_optimal(P2)=1.592x`
   - Evidence: [thesis.typ](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/thesis.typ:1105), [thesis.typ](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/thesis.typ:1712), [specs/TODO.md](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/specs/TODO.md:52), [speedup_summary.txt](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/benchmarks/reports/260610-corpus-final/speedup_summary.txt:1)

8. **The binary-size prose contradicts the project’s recorded RQ2 measurement basis.**
   - Thesis prose cites roughly `+175.0%`.
   - Tracker-backed measurement records `7.9 KB -> 138.5 KB (+1660%)` for the UC1 TU case study.
   - Evidence: [thesis.typ](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/thesis.typ:1464), [specs/TODO.md](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/specs/TODO.md:114), [binary_size_table.txt](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/benchmarks/reports/260610-binary-size/binary_size_table.txt:1)

9. **The size-scaling section still lacks one settled source of truth.**
   - Thesis conclusion is stated as settled.
   - Tracker still marks the originally planned source run as partially unresolved.
   - Current plotting script points at a different dataset by default.
   - Evidence: [thesis.typ](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/thesis.typ:1494), [specs/TODO.md](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/specs/TODO.md:93), [plot_thesis_size_scaling.py](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/benchmarks/reporting/plot_thesis_size_scaling.py:21)

10. **The supported-architectures section still contains a live TODO that weakens a completed taxonomy claim.**
    - Evidence: [thesis.typ](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/thesis.typ:1294), [specs/TODO.md](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/specs/TODO.md:102)

11. **`specializeLambda` wording should be narrowed to shallow address-based capture, not deep closure snapshotting.**
    - Evidence: [thesis.typ](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/thesis.typ:787), [ClangRuntimeSpecializer.h](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h:84), [ClangRuntimeSpecializer.h](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h:880), [specialized-lambda-factory-lifetime.cpp](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/test/smoke/specialized-lambda-factory-lifetime.cpp:34)

## Recommended Remediation Order

1. Fix thesis statements that are plainly false today.
2. Repair evaluation arguments that rely on invalidated or ambiguous evidence.
3. Tighten implementation descriptions to match current code behavior.
4. Remove or resolve thesis TODOs that sit inside completed-claim sections.

## Default Decision For Follow-Up Work

When thesis and repo disagree, prefer fixing the thesis to match the validated
implementation and recorded artifacts. Only change code when the inconsistency
identifies a real implementation defect rather than stale thesis wording.
