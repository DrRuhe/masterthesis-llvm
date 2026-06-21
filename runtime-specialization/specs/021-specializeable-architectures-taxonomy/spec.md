# Spec 021: Specializeable Architectures Taxonomy

## Overview

This spec defines the material-gathering and argument structure for the thesis
subsection `Specializeable Architectures` in `docs/thesis.typ`. The goal is to
replace the current TODO stub with a general taxonomy of architectural patterns
that the runtime-specialization approach supports and patterns it does not
support. This subsection should become the thesis's canonical reference for CRS
support and limitations, so existing SQLite/TPC-H failure analysis currently
living near `@rq-limits` should be migrated or condensed into this taxonomy
instead of being maintained as a separate parallel explanation. The section
must classify the current outlier kernels into that taxonomy instead of
treating them as isolated anecdotes. Economic benefit is not part of the
per-pattern taxonomy itself; it is only a downstream consequence of whether the
relevant specialization mechanism is supported.

## User Scenarios

**Scenario A — Reader wants a fast taxonomy**
Given a thesis reader reaches `Specializeable Architectures`,
when they scan the subsection headings,
then they can quickly identify which architectural patterns are supported,
or unsupported.

Acceptance:
- Given a subsection heading such as `Nested Function Calls` or
  `Vtable Devirtualization`, when the reader enters the subsection, then the
  first sentence states what the pattern is and when it appears in practice.
- Given an unsupported pattern such as a shared mutable state object threaded
  through a dispatch loop, when the reader reaches that subsection, then the
  text explains why the current CRS analysis cannot expose the specialized path
  simplification that pattern would require.

**Scenario B — Reader wants concrete evidence**
Given a reader wants to trust the taxonomy,
when they read each subsection,
then they find at least one concrete code example or benchmark-backed case that
shows why the pattern is classified as supported or unsupported.

Acceptance:
- Given a supported pattern, when the subsection argues it is supported, then it
  cites a concrete corpus benchmark, smoke test, or unit-test style snippet
  already present in the repository.
- Given an unsupported pattern, when the subsection argues the limitation, then
  it cites the relevant benchmark report, reflection, or code path showing the
  failure mode.

**Scenario C — Outlier kernels are explained by taxonomy**
Given the three UC1 outlier kernels `count_matching_rows`, `multi_predicate`,
and `column_scan`,
when the reader looks for them,
then each kernel is assigned to one or more taxonomy categories with a short
root-cause explanation.

Acceptance:
- Given `sqlite3VdbeExec`, when the unsupported-pattern subsection is read, then
  it is explicitly classified as a shared-state interpreter/dispatch-loop case.
- Given `count_matching_rows`, `multi_predicate`, and `column_scan`, when the
  taxonomy is read, then each appears in the subsection whose reasoning best
  explains its poor outcome.

## Functional Requirements

- **FR-001**: The thesis subsection MUST be structured as a taxonomy of
  architectural patterns, not as a flat list of benchmark anecdotes.
- **FR-002**: Each taxonomy subsection MUST begin with one sentence that states
  what the pattern is and when it is used.
- **FR-003**: Each taxonomy subsection MUST explicitly classify the pattern as
  either supported or unsupported under the current CRS approach.
- **FR-004**: Each supported taxonomy subsection MUST cite at least one local
  repository artifact that demonstrates successful specialization, using corpus
  benchmarks, smoke tests, or focused implementation snippets already present in
  the repository.
- **FR-005**: Each unsupported taxonomy subsection MUST cite at least one local
  repository artifact that demonstrates the failure mode or analysis limit.
- **FR-006**: The taxonomy MUST include a subsection for the SQLite/TPC-H
  pattern where a mutable state object is threaded through an interpreter-style
  dispatch loop.
- **FR-006a**: The SQLite/TPC-H support-limit explanation currently living in
  the later cross-workload-limits discussion MUST be treated as source material
  for this subsection, so `Specializeable Architectures` becomes the primary
  place where thesis readers learn what CRS does and does not support.
- **FR-007**: The taxonomy MUST classify the current UC1 outlier kernels
  `count_matching_rows`, `multi_predicate`, and `column_scan` into the
  appropriate taxonomy entries instead of discussing them only in aggregate.
- **FR-008**: The collected material MUST keep technical support as the primary
  classification axis. Any statement about missing speedups MUST be phrased as a
  consequence of unsupported patterns, not as a per-pattern profitability study.
- **FR-009**: The plan for this section MUST identify the exact evidence source
  for each taxonomy entry: thesis lines, benchmark report paths, spec/reflection
  paths, and code/test paths.
- **FR-010**: The plan MUST identify any taxonomy entries that still lack
  evidence and specify whether to weaken the claim, gather evidence, or move the
  pattern to future work.

## Success Criteria

- **SC-001**: A finite list of taxonomy subsections exists, each with a named
  pattern, support classification, and at least one evidence source path.
- **SC-002**: The SQLite/TPC-H shared-state interpreter pattern has a dedicated
  unsupported entry with source-backed reasoning.
- **SC-002a**: The plan identifies how the current SQLite/TPC-H narrative in
  `@rq-limits` will be migrated, summarized, or cross-referenced so the thesis
  has one canonical support/limitations taxonomy rather than two competing
  explanations.
- **SC-003**: `count_matching_rows`, `multi_predicate`, and `column_scan` are
  each mapped to taxonomy entries with an explicit explanation of which missing
  or unsupported specialization mechanism best explains their result.
- **SC-004**: The material plan identifies at least one supported pattern shown
  by nested calls/inlining-style evidence and at least one supported pattern
  shown by devirtualization-style evidence, if such evidence exists locally.
- **SC-005**: The resulting plan is specific enough that a later drafting pass
  can collect material without reopening high-level scope questions.

## Edge Cases

- A kernel may fit more than one taxonomy entry. The plan should choose a
  primary category and note secondary effects without turning the section into a
  general performance taxonomy.
- Some repository artifacts show technical support but not compelling speedup.
  These still count as support evidence because this section is not a per-pattern
  profitability analysis.
- Some attractive headings may not be defensible if no local evidence exists
  (for example a devirtualization heading without a convincing benchmark or
  test). In that case the plan must weaken or remove the heading rather than
  infer support.
- The subsection title promises a general taxonomy, but the thesis already has a
  later cross-workload-limits section. The plan must avoid duplicating the full
  SQLite/TPC-H narrative if a short cross-reference is enough.

## Clarifications

### Session 2026-06-21

- The target is a **general taxonomy of specializeable architectures**, not only
  an RQ1-local explanation of the three UC1 outliers.
- The intended shape is a hierarchy like:
  `Specializeable Architectures` → pattern subsections such as
  `Nested Function Calls`, `Vtable Devirtualization`, and unsupported-pattern
  subsections such as `Passing around State Object`.
- Each subsection should explain the feature/pattern in one sentence and then
  evaluate whether CRS supports it.
- For supported patterns, code snippets from the benchmark corpus or unit/smoke
  tests are acceptable evidence if they demonstrate that the pattern
  specializes.
- The outlier kernels must be analyzed and classified into the taxonomy rather
  than treated separately from the taxonomy.
- The taxonomy should focus solely on **technical support**. The intended thesis
  move is: if a kernel exhibits no speedup and its relevant architectural
  pattern is unsupported, that lack of speedup is expected.
- The subsection should become the **complete reference** for what CRS supports
  and does not support. The existing SQLite/TPC-H reasoning should therefore be
  folded into this section rather than duplicated elsewhere.
- Brainstorming additional architectural patterns is part of scope. Candidate
  headings may be added during evidence collection, but any heading without
  concrete local evidence must remain `open` or be removed before drafting.
