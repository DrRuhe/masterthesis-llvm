# Findings 021: Specializeable Architectures Taxonomy

## Task 1 — Confirm the target thesis context by rereading `docs/thesis.typ` around `@specializeable-architectures`, `@rq-limits`, and the existing RQ1 outlier references so the taxonomy does not duplicate later sections.

- `docs/thesis.typ` still contains only a TODO stub at `@specializeable-architectures`.
- The RQ1 outlier discussion already points readers from the amortization section to `@specializeable-architectures`.
- `@rq-limits` currently contains the concrete SQLite/TPC-H support-limit narrative, including the shared mutable state object and dynamic opcode-dispatch reasoning.
- The target drafting shape is therefore: put the architectural-support taxonomy at `@specializeable-architectures`, and keep `@rq-limits` for the cross-workload consequence rather than the full mechanism explanation.

## Task 2 — Decide how the current SQLite/TPC-H explanation at `@rq-limits` will be migrated, condensed, or cross-referenced so `Specializeable Architectures` becomes the canonical support/limitations reference.

- The SQLite/TPC-H explanation should be migrated conceptually, not duplicated verbatim.
- `@specializeable-architectures` should own the primary unsupported-pattern entry for:
  - shared mutable state threaded through a call graph
  - interpreter dispatch over a dynamic program counter / opcode stream
- `@rq-limits` should be reduced to a short consequence section: SQLite/TPC-H is a structural failure case, therefore cross-workload pipeline conclusions must not over-generalize from the UC corpus.

## Task 3 — Confirm `specs/TODO.md` records this taxonomy task and links back to spec 021 before any thesis drafting starts.

- `specs/TODO.md` already contains `RQ1-007: Build thesis taxonomy for "Specializeable Architectures"`.
- The entry already links back to `specs/021-specializeable-architectures-taxonomy/`.
- No TODO edit is required at this stage because the task is present, scoped correctly, and points to the active spec directory.

## Task 4 — Build the candidate taxonomy heading list in `details.md` and mark each entry as `supported`, `unsupported`, or `open` based only on inspected local evidence.

- The candidate list is now stable enough to treat as a current taxonomy ledger.
- Supported entries confirmed from local source/tests/specs:
  - `Flat Batch Kernels with Fixed Layout or Threshold Parameters`
  - `Immutable Lookup Tables or Coefficient Arrays`
  - `Nested Function Calls`
  - `By-Value Captured Helper Objects / Policy Structs`
  - `Function-Pointer Callbacks`
  - `Vtable Devirtualization`
- Unsupported entries confirmed from local SQLite analysis and design constraints:
  - `Shared Mutable State Object Threaded Through the Call Graph`
  - `Interpreter Dispatch Loop over a Dynamic Program Counter`
  - `Opaque External Callees / Cross-Blob Boundaries` as a secondary unsupported entry

## Task 5 — Verify that each proposed heading has at least one repository artifact that can serve as evidence; remove or weaken any heading whose evidence is not local and concrete.

- Every surviving heading now has at least one concrete local artifact path in `details.md`.
- The only weakened entry is `Opaque External Callees / Cross-Blob Boundaries`:
  keep it as a secondary unsupported heading or footnote, not as a central thesis claim.
- The previous `Escaping Pointer State Hidden Behind Conservative Mutability` heading was removed as a standalone taxonomy entry and folded into the SQLite shared-state heading.

## Task 6 — Brainstorm additional architectural-pattern headings from benchmark families, smoke tests, and pipeline specs, then record them in `details.md` as `supported`, `unsupported`, or `open`.

- Additional brainstormed headings were considered, but the stronger decision was to fold them into existing entries rather than proliferate weak headings.
- `Abstract Result Collectors` was folded into `Vtable Devirtualization` plus `Flat Batch Kernels...`.
- `Schema-Specialized Aggregation` was folded into `Flat Batch Kernels...`.
- No new `open` heading survived this pass; the current list is already broad enough for the thesis subsection without inventing poorly evidenced categories.

## Task 7 — Collect support evidence for a flat-batch kernel pattern where scalar thresholds, row-layout metadata, or fixed bucket/layout parameters become JIT constants inside one large loop body.

- UC1 low kernels provide the clearest minimal code evidence:
  `count_matching_rows`, `multi_predicate`, and `column_scan` all capture fixed
  layout/threshold parameters and run one batch loop over `rows`.
- The stronger corpus-level evidence is in `benchmarks/reports/260610-corpus-final/speedup_summary.txt`
  and `benchmarks/reports/260610-breakeven/breakeven_table.txt`, where grouped
  aggregation and IVM kernels are strong positive examples of the same
  architecture.

## Task 8 — Collect support evidence for immutable lookup-table / coefficient-array patterns from convolution or DFA benchmarks where a fixed table pointer or coefficient set is captured as specialization state.

- UC2 and UC7 give direct local evidence:
  - `UC2Kernels.h` declares `g_kernel_coeffs` as specialization state.
  - `UC2EdgeDetectionTradeoffKernels.cpp` uses immutable Sobel coefficient arrays.
  - `UC7EmailMatchTradeoffKernels.cpp` captures `g_dfa_table`.
  - `UC7MultiPatternMatchTradeoffKernels.cpp` captures `g_multi_dfa_table_tradeoff`.

## Task 9 — Collect support evidence for a nested-helper/inlining style pattern from specs, smoke tests, benchmark kernels, or pass traces.

- `specs/003-jit-specialization-core/spec.md` provides the implementation-level
  claim that specialization works by cloning one blob and forcing inlining of
  the target call.
- The strongest observed evidence is in the pass traces for
  `edge_detection` and `generic_sort`, where `ModuleInlinerPass` reduces
  function count from `8` to `7`.

## Task 10 — Collect support evidence for a constant function-pointer or callback-style pattern from `generic_sort`/forwarded-function-pointer artifacts.

- The benchmark evidence is `UC14GenericSortLowKernels.cpp` and
  `UC14GenericSortTradeoffKernels.cpp`, both of which make the comparator a
  specialization constant and keep it in the same TU for inlining.
- The regression-test evidence is the trio of smoke tests covering direct,
  helper-forwarded, and tuple-forwarded function-pointer specialization.

## Task 11 — Collect support evidence for a vtable-devirtualization pattern from smoke tests, UC abstract kernels, and pipeline specs.

- `test/smoke/virtual-methods.cpp` is the strongest minimal artifact: it checks
  that the specialized IR no longer contains a vtable load.
- `specs/015-pipeline2-jit-ipsccp/spec.md` and
  `runtime/ClangRuntimeSpecializer/DevirtualizeConstantVtableCalls.cpp`
  establish that this is an intentional first-class mechanism rather than an
  accidental optimization.
- UC abstract kernels (`UC1*AbstractKernels.cpp`, `UC8ApplyRowDeltaAbstractKernels.cpp`)
  provide workload-level examples of the same pattern.

## Task 12 — Collect support evidence for by-value captured helper objects or policy structs whose fields become JIT constants without requiring virtual dispatch.

- `BinaryPredicateScanner`, `SobelFilter`, and `MultiKeySorter` are the three
  best local examples.
- These are useful because they separate “helper object capture” from the
  stronger devirtualization/function-pointer headings, making the taxonomy less
  dependent on abstract dispatch examples alone.

## Task 13 — Collect unsupported-pattern evidence for the SQLite/TPC-H shared mutable state object threaded through an interpreter-style dispatch loop.

- `specs/019-sqlite-specialization-analysis/report.md` explicitly identifies the
  escaped `Vdbe*` state object as the primary immediate cause of failure.
- `details.md` for spec 019 adds the concrete mechanism: zero sqlite3 loads are
  annotated `!invariant.load` because the state object escapes through the call
  graph.

## Task 14 — Collect unsupported-pattern evidence for the dynamic opcode-stream / interpreter-dispatch aspect of `sqlite3VdbeExec` and decide whether it should be a separate taxonomy entry or folded into the shared-state entry.

- The dynamic opcode-stream reasoning is independently strong enough to keep as
  a separate unsupported entry.
- The key fact is `p->aOp[pc].opcode`: even with a fixed `Vdbe*`, the changing
  `pc` means the optimizer still cannot collapse execution to one fixed path.
- This is distinct from the escaped-state problem, so the taxonomy should keep
  both reasons visible.

## Task 15 — Consider open/brainstormed unsupported-pattern candidates such as opaque external callees, cross-blob boundaries, or dynamic state hidden behind escaping pointers; keep only those with concrete local evidence.

- `Opaque External Callees / Cross-Blob Boundaries` survives as a secondary
  unsupported entry because spec 005 contains a direct design decision based on
  this limitation.
- `Dynamic state hidden behind escaping pointers` does not survive as its own
  reader-facing taxonomy heading; it is better treated as the mechanism inside
  the SQLite shared-state failure case.
- No broader unsupported heading for “opaque external callees” was kept beyond
  the cross-blob formulation because the local evidence is architectural rather
  than benchmark-facing.

## Task 16 — Inspect the UC1 outlier kernels `count_matching_rows`, `multi_predicate`, and `column_scan` in the benchmark sources and note which architectural pattern each actually exhibits.

- All three outlier families are still instances of the supported
  `Flat Batch Kernels with Fixed Layout or Threshold Parameters` architecture.
- Their tradeoff tiers also instantiate `By-Value Captured Helper Objects`.
- Their abstract tiers also instantiate `Vtable Devirtualization`.
- The key point is that none of the three source families looks like SQLite’s
  unsupported shared-state interpreter pattern.

## Task 17 — Analyze `count_matching_rows` deliberately across low/tradeoff/abstract variants: identify which supported or unsupported taxonomy pattern it instantiates, which specialization mechanism is actually available to CRS, and whether the thesis should present it as a simple-supported-pattern case rather than an unsupported one.

- `count_matching_rows` is the simplest supported batch-scan case in the corpus.
- CRS can specialize exactly the mechanisms the source exposes: `row_stride`,
  `col_offset`, and `threshold`; the abstract tier additionally exposes a
  devirtualization opportunity.
- The thesis should present it as a simple supported pattern with limited
  payoff, not as an unsupported pattern.

## Task 18 — Analyze `multi_predicate` deliberately across low/tradeoff/abstract variants: identify whether it is best explained by flat layout constants, helper-object capture, abstract predicate devirtualization, or another taxonomy entry, and record the strongest evidence path.

- Primary classification: `Flat Batch Kernels with Fixed Layout or Threshold Parameters`.
- Secondary classification: `By-Value Captured Helper Objects` in tradeoff tier,
  `Vtable Devirtualization` in abstract tier.
- Strongest evidence path: the UC1 source files themselves plus the final-corpus
  speed tables showing marginal-but-real improvement under some configs rather
  than a structural specialization failure.

## Task 19 — Analyze `column_scan` deliberately across low/tradeoff/abstract variants: identify how the output-buffer write path affects its taxonomy classification, whether any collector/result-abstraction pattern belongs in the taxonomy, and which entry should own the kernel family in the thesis discussion.

- `column_scan` should stay under the primary flat-batch heading.
- The output-buffer write path explains why less work disappears after
  specialization, but it does not imply an unsupported architecture.
- The abstract collector pattern is real, but it belongs as a secondary example
  under `Vtable Devirtualization`, not as a new standalone heading.

## Task 20 — Cross-check the outlier-kernel classifications against the reflection/report evidence in `benchmarks/reports/260531-2115-Review/`, `specs/008-use-case-benchmarks/spec.md`, and any final thesis text that already references them.

- The current thesis text already describes these kernels in terms of fixed
  predicates and row-layout metadata, which matches the final classification.
- The review report adds cautionary context about historical `count_matching_rows`
  JIT-overhead artifacts and about `column_scan` retaining substantial output
  work, but it does not provide evidence that either kernel family is
  architecturally unsupported.
- `specs/008-use-case-benchmarks/spec.md` is also aligned: it promises technical
  specialization opportunities, not guaranteed large speedups for every kernel.

## Task 21 — Decide whether each outlier kernel can be defended as an instance of an unsupported pattern; if not, weaken the intended claim in `details.md` so the later thesis draft does not overstate the evidence.

- None of the three UC1 outliers can be defended as an unsupported architecture.
- `details.md` now treats all three as supported architectures with weak or
  marginal economic payoff.
- The later thesis draft should explicitly separate:
  - unsupported structural failure cases: SQLite/TPC-H
  - supported but low-payoff cases: `count_matching_rows`, `multi_predicate`,
    `column_scan`

## Task 22 — For each supported taxonomy entry, record one minimal code snippet candidate path and one stronger supporting artifact path in `details.md`.

- `details.md` now contains a drafting ledger with both a minimal code snippet
  path and a stronger artifact path for each supported heading.
- The strongest supported snippet candidates are:
  - `UC1CountMatchingRowsLowKernels.cpp` for flat batch kernels
  - `UC7EmailMatchTradeoffKernels.cpp` for immutable tables
  - `call-specialized-forwarded-funcptr.cpp` for callback specialization
  - `virtual-methods.cpp` for devirtualization

## Task 23 — For each unsupported taxonomy entry, record one root-cause artifact path and one thesis-cross-reference path in `details.md`.

- The unsupported headings now each have:
  - a root-cause artifact path in spec 019 or spec 005/004
  - a thesis-facing cross-reference path or source-material pointer
- SQLite/TPC-H is now ready to be migrated into the taxonomy section without
  losing the later `@rq-limits` consequence discussion.

## Task 24 — Draft the per-entry evidence ledger in `details.md` with: heading, classification, one-sentence definition, primary evidence, secondary evidence, and affected kernels.

- The drafting ledger is now present in `details.md` and uses exactly that
  structure, with the addition of a minimal snippet candidate column.
- This should be enough for a direct thesis drafting pass.

## Task 25 — Verify FR-001 through FR-010 are covered by at least one concrete collection task in this plan.

- All FRs are now explicitly checked in the `Coverage Check` section of
  `details.md`.
- No FR remains without a concrete artifact or decision path.

## Task 26 — Verify SC-001 through SC-005 by checking that every taxonomy entry, the SQLite case, the three UC1 outliers, and the two user-requested example headings (`Nested Function Calls`, `Vtable Devirtualization`) have explicit evidence or an explicit weakening decision.

- All SCs are now explicitly checked in `details.md`.
- The user-requested headings `Nested Function Calls` and
  `Vtable Devirtualization` both have concrete evidence.
- The one weakened claim is the secondary unsupported cross-blob heading, and
  that weakening is called out directly.

## Task 27 — Report any remaining evidence gaps or over-strong planned claims to the user before any drafting pass edits `docs/thesis.typ`.

- No critical evidence gaps remain for the main subsection.
- The main draft-time caution is to keep `Opaque External Callees / Cross-Blob Boundaries`
  secondary and to avoid calling the UC1 outliers unsupported.
- With those cautions, the materials are ready for a thesis-writing pass.
