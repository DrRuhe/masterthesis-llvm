# Spec 020: Self-Improving Agent Harness

## Overview

This spec introduces a hierarchical `AGENTS.md` harness for the Master's thesis
workspace so the agent improves its operating instructions from real sessions
instead of relying on static guidance alone. Each substantive session must end
with a short reflection, that reflection must be appended to a central journal,
and the agent must derive scoped `AGENTS.md` and skill improvement proposals
from the reflection before making any edits.

## User Scenarios

**Scenario A — Runtime-specialization session closes with a reflection**
Given a substantive session inside `llvm/runtime-specialization`,
when the agent finishes the requested work,
then it appends a reflection entry to the central journal and proposes only the
`AGENTS.md` edits that belong at the root or runtime-specialization scope.

Acceptance:
- Given a runtime-specialization task with file or tool work, when the session
  closes, then the journal entry records what went well, what was costly, and
  what should be added/removed from `AGENTS.md`.
- Given the reflection identifies a repo-local lesson, when the proposal is
  presented, then it targets `llvm/runtime-specialization/AGENTS.md` instead of
  duplicating the guidance in a broader file.

**Scenario B — Docs session uses the same harness**
Given a substantive session inside `docs/`,
when the agent finishes the work,
then it appends a reflection entry to the same central journal and routes any
documentation-specific guidance to `docs/AGENTS.md`.

Acceptance:
- Given a lesson only affects Typst or thesis editing, when the proposal is
  generated, then it targets `docs/AGENTS.md` rather than the runtime project.
- Given a lesson generalizes across the workspace, when the proposal is
  generated, then it targets the root `AGENTS.md`.

**Scenario C — Skill use produces a skill-improvement proposal**
Given the agent used a skill during a substantive session,
when the session reflection is written,
then the reflection includes a skill-specific subsection and proposes a skill
update when durable improvement is warranted.

Acceptance:
- Given a skill helped or hindered the work, when the reflection is recorded,
  then the entry explicitly states what in the skill should be added, shortened,
  or removed.
- Given the user approves a skill update, when the skill is edited, then the
  agent uses the `skill-creator` workflow instead of ad-hoc edits.

## Functional Requirements

- **FR-001**: The workspace MUST use a three-level `AGENTS.md` hierarchy at:
  `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/AGENTS.md`,
  `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/AGENTS.md`, and
  `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/AGENTS.md`.
- **FR-002**: The root `AGENTS.md` MUST define the self-improving harness:
  reflection trigger conditions, required reflection questions, journal path,
  scoping rules for `AGENTS.md` edits, single-batch approval behavior, and the
  skill-reflection rule.
- **FR-003**: `docs/AGENTS.md` MUST contain only documentation-domain guidance:
  Typst/thesis workflow expectations, the shared reflection hook, and rules for
  deciding when a lesson belongs in docs scope versus the root file.
- **FR-004**: `llvm/runtime-specialization/AGENTS.md` MUST contain only
  runtime-specialization-specific workflow guidance plus explicit routing rules
  that push cross-workspace guidance upward to the root file.
- **FR-005**: Substantive sessions MUST end with an append-only reflection entry
  written to `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/.codex/reflection.md`.
- **FR-006**: A reflection entry MUST include, at minimum: date, scope, paths or
  domains touched, skills used, what went well, what was costly or confusing,
  guidance to add, guidance to shorten/remove, proposed `AGENTS.md` edits, and
  proposed skill edits.
- **FR-007**: The agent MUST treat a session as substantive when it involved
  exploration, review, editing, testing, benchmarking, or skill usage. Trivial
  chit-chat may be skipped.
- **FR-008**: After writing the reflection, the agent MUST present one scoped
  batch proposal listing the exact `AGENTS.md` and skill files that should
  change. The agent MUST NOT edit those files before explicit approval.
- **FR-009**: The agent MUST scope each proposed change to the highest useful
  owner: root for cross-workspace rules, `docs/AGENTS.md` for thesis/docs rules,
  `runtime-specialization/AGENTS.md` for project-local workflow, and the
  specific skill directory for skill-specific operating guidance.
- **FR-010**: When a used skill needs improvement and the user approves the
  change, the agent MUST use the `skill-creator` skill to update the skill and
  validate the updated skill before closing the task.
- **FR-011**: Reflection entries MUST record the user’s approval outcome for the
  proposed `AGENTS.md` and skill changes so the journal remains auditable.
- **FR-012**: The first implementation of this harness MUST NOT create deeper
  `AGENTS.md` layers below the three agreed files unless repeated reflections
  later justify them.

## Success Criteria

- **SC-001**: The three `AGENTS.md` files exist and have non-overlapping scopes:
  global harness, docs workflow, and runtime-specialization workflow.
- **SC-002**: `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/.codex/reflection.md`
  exists and documents the required append-only journal structure.
- **SC-003**: The runtime-specialization `AGENTS.md` explicitly requires reading
  the constitution and `specs/TODO.md` before relevant work while also joining
  the shared reflection harness defined at the root.
- **SC-004**: The harness documents a single-batch approval workflow that allows
  partial approval while forbidding pre-approval `AGENTS.md` or skill edits.
- **SC-005**: The harness documents the skill-update path clearly enough that a
  later session can update a used skill only after reflection plus approval.

## Edge Cases

- A session may touch both `docs/` and `runtime-specialization/`; the reflection
  should still be one journal entry with multiple proposed file targets.
- A substantive session may conclude that no durable instruction changes are
  warranted; the journal entry should record “no proposed edits” rather than
  omitting the reflection.
- A lesson may initially appear project-local but later generalize; the routing
  rules must allow future reflections to move guidance upward.
- A skill may be used without needing a skill edit; the skill subsection should
  still record that no durable change was identified.
- Parent-level files live outside the current repo root; implementations may
  require elevated write approval even though the workflow itself remains the
  same.

## Clarifications

### Session 2026-06-15

- The central reflection journal path is fixed at
  `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/.codex/reflection.md`.
- Reflections are append-only; they are not rewritten into a rolling summary.
- Approval is requested as one scoped batch per session, but the approved subset
  may be smaller than the proposed set.
