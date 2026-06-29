# Overview

This feature aligns `docs/thesis.typ` with the current runtime-specialization
implementation and the benchmark artifacts already recorded in the repo. The
goal is to remove contradictions between thesis claims, code behavior, and
evaluation evidence so the thesis can be defended from the checked-in sources
without relying on outdated assumptions or invalidated experiments.

Informed default: when a mismatch is discovered, the remediation should prefer
correcting the thesis to match validated implementation and recorded artifacts.
Code changes are only in scope when the inconsistency exposes a real repo defect
rather than stale or over-strong thesis wording.

## User Scenarios

### Scenario 1 — Reader audits an implementation claim

A thesis reader checks an implementation claim against the codebase.

Acceptance scenario:
- Given a claim about APIs, pipeline defaults, serialization behavior, or
  runtime constraints, when the reader cross-checks `docs/thesis.typ` against
  the repo, then the thesis matches the code or explicitly scopes any remaining
  uncertainty.

### Scenario 2 — Reader audits an evaluation claim

A thesis reader checks a reported evaluation conclusion against the local
artifacts.

Acceptance scenario:
- Given a thesis statement about speedup, pipeline choice, binary size,
  size-scaling, or failure cases, when the reader traces the cited evidence in
  the repo, then the thesis points to one authoritative local source and does
  not rely on invalidated benchmark narratives.

### Scenario 3 — Maintainer continues thesis cleanup

A future maintainer wants a concrete repair checklist.

Acceptance scenario:
- Given the audit findings, when the maintainer opens the remediation plan, then
  the work is split into granular thesis, artifact, and possible code tasks with
  explicit verification steps.

## Functional Requirements

- **FR-001**: The thesis MUST describe the current default JIT pipeline
  consistently with the implementation and the evaluation chapter.
- **FR-002**: The thesis MUST scope the public specialization API accurately,
  including plugin-rewrite requirements and currently unsupported call-site
  forms where those limits materially affect claims of annotation-freedom.
- **FR-003**: The thesis MUST describe runtime safety and scope constraints
  accurately, including the effective single-threaded requirement of the current
  implementation.
- **FR-004**: The thesis MUST describe embedded IR and lambda specialization in
  terms consistent with the actual shallow-serialization implementation.
- **FR-005**: The thesis MUST not rely on invalidated SQLite/TPC-H evidence to
  justify the default-pipeline choice.
- **FR-006**: Each thesis evaluation claim called out by the audit MUST map to
  one authoritative local artifact, or the claim MUST be weakened until that
  provenance is honest.
- **FR-007**: Live TODO markers inside sections that make completed technical
  claims MUST be removed, resolved, or downgraded so they no longer contradict
  the surrounding thesis narrative.
- **FR-008**: The remediation plan MUST distinguish thesis-only fixes from
  repo/code fixes so implementation work is only proposed where the repo itself
  is defective.

## Success Criteria

- **SC-001**: No audited contradiction remains where `docs/thesis.typ` states a
  different pipeline default than the code.
- **SC-002**: The thesis’s API and safety descriptions no longer overclaim
  support relative to the current implementation.
- **SC-003**: The default-pipeline, binary-size, size-scaling, and
  supported-architectures sections each reference a defensible local source of
  truth or are rewritten to state their uncertainty explicitly.
- **SC-004**: The audit report and remediation spec/plan/details together give a
  successor enough context to execute the fixes without redoing the full audit.

## Edge Cases

- Some mismatches may reflect genuinely better newer evidence than the current
  thesis prose. In that case, the thesis should be corrected rather than the new
  evidence being forced back into the old narrative.
- Some evaluation claims may remain directionally true while their originally
  cited artifact is invalidated. These claims should be re-grounded in valid
  artifacts or weakened, not silently retained.
- Some inconsistencies may expose real implementation defects, such as blob
  lookup collisions affecting benchmark validity. Those cases should produce
  explicit code tasks instead of being treated as prose-only cleanup.

## Clarifications

### Session 2026-06-25

- Defaulted remediation toward thesis corrections first, code changes second,
  unless the audit points to a real implementation bug.
- Treated the local repo, `specs/TODO.md`, and checked-in benchmark reports as
  the authoritative evidence base for this planning pass.
