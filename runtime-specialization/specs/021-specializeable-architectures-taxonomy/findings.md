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
