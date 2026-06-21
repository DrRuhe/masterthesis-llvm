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
