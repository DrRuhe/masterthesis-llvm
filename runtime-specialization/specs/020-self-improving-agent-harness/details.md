# Details: Self-Improving Agent Harness

## File Ownership

| File | Owner | Purpose |
|------|-------|---------|
| `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/AGENTS.md` | Workspace root | Defines the reflection harness, scoping rules, approval flow, and skill-update policy |
| `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/AGENTS.md` | Docs domain | Captures Typst/thesis-specific workflow and how docs lessons should be distilled |
| `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/AGENTS.md` | Runtime-specialization domain | Keeps constitution/spec/TODO workflow plus routing to parent guidance |
| `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/.codex/reflection.md` | Shared journal | Append-only record of session reflections, proposals, and approval outcomes |

## Reflection Schema

Each substantive session appends one top-level dated section:

```md
## 2026-06-15 — Short session label

### Scope
- Paths/domains touched
- Skills used

### What Went Well
- Durable practices worth repeating

### Friction / Cost
- Confusing, expensive, or repeated work

### AGENTS Changes Proposed
- Add:
- Shorten/remove:
- Target files:

### Skill Changes Proposed
- Skill:
- Add:
- Shorten/remove:
- Target files:

### Approval / Outcome
- Proposed batch:
- User decision:
- Applied files:
- Deferred/rejected files:
```

If no durable changes are warranted, the proposal sections still exist and say
`None`.

## Scoping Rules

1. Put guidance in the highest file that benefits from it without duplicating it.
2. Use the root `AGENTS.md` for cross-workspace behavior such as reflection
   triggers, approval flow, or skill policy.
3. Use `docs/AGENTS.md` only for thesis-writing and documentation workflow.
4. Use `runtime-specialization/AGENTS.md` only for LLVM runtime-specialization
   project behavior.
5. Update a skill only for skill-specific reusable guidance; do not move
   cross-cutting workflow into the skill.

## Approval Flow

1. Finish the substantive work.
2. Append the reflection entry to the journal.
3. Derive a single scoped batch proposal from that entry.
4. Wait for explicit approval before editing any `AGENTS.md` or skill file.
5. Apply only the approved subset and record the outcome back in the journal.

## Skill Updates

- Approved skill edits must use the `skill-creator` workflow.
- If `SKILL.md` changes materially, regenerate or validate `agents/openai.yaml`.
- Run the skill validation step after the edit.
- If a skill was used but no durable improvement was identified, record that
  explicitly in the reflection rather than silently skipping the skill review.

## Constitution Alignment

- The harness remains spec-driven by documenting its behavior in `specs/020-*`
  before or alongside implementation.
- The workflow does not weaken the runtime-specialization constitution; it adds
  a parent-level reflection loop that helps keep the instructions current.
