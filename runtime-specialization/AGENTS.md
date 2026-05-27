# LLVM Runtime Specializer

`.specify/memory/constitution.md` is the authoritative reference for architecture,
build/test commands, and non-negotiable project rules. Read it first when starting
work on this project.

## Spec-driven workflow

Each feature lives in its own subdirectory under `specs/`, named
`specs/NNN-feature-name/` (sequential numbering, kebab-case). A feature directory
contains three conceptual files:

- **`spec.md`** — goals, desired state, acceptance criteria. The *what* and *why*.
- **`plan.md`** — an actionable, granular checklist of tasks. The *how*, as work units.
- **`details.md`** — implementation details: data models, conventions, decisions,
  references, gotchas. The supporting context that doesn't fit in spec or plan.

### `spec.md` structure

Use these sections:

- **Overview** — one paragraph: what the feature is, why it exists.
- **User Scenarios** — narrative scenarios, each with explicit acceptance scenarios
  (Given / When / Then or equivalent).
- **Functional Requirements** — numbered, testable requirements, ID'd `FR-001`,
  `FR-002`, … Every FR must be testable.
- **Success Criteria** — measurable outcomes, ID'd `SC-001`, `SC-002`, …
- **Edge Cases** — known boundary conditions and how they should be handled.
- **Clarifications** — appended as the spec is refined (see Clarify step).

Make informed defaults for unspecified details and call them out so they can be
challenged.

### `plan.md` structure

A flat markdown checklist. No phase headers, no task IDs, no parallel markers — just
granular checkboxes. Example:

    - [ ] Verify <precondition> before making changes
    - [ ] Update `path/to/file.cpp` to do X
    - [ ] [complex] Debug failing JIT build after change Y
    - [ ] Verify SC-002 by running `ninja check-smoke-runtime-specializer`

Rules for plan.md tasks:

- **Granular.** One task = one well-defined change or check, not a phase of work.
- **Include precondition tasks** (e.g. "Confirm test X currently passes before
  modifying file Y").
- **Include verification tasks tied to spec acceptance criteria** (e.g. "Verify
  SC-003 holds by …").
- **Mark `[complex]`** when a task is not straightforward: debugging, architectural
  decisions, multi-file refactors, performance investigations, anything where a
  fresh subagent would benefit from an isolated context.

### `details.md`

Free-form. Typical contents:

- Data model decisions and schemas.
- Conventions chosen for this feature (naming, file layout, error handling).
- Cross-references to constitution rules that apply.
- Prior art, API references, known gotchas.

## Modes

The workflow runs in one of two modes:

- **Plan mode** — used for steps 1–4 below (Specify, Clarify, Plan, Analyze).
  Produces or refines `spec.md`, `plan.md`, and `details.md` under
  `specs/NNN-feature-name/`. No source code is written and no commits are made
  during plan mode. Plan mode ends when the user approves the artifacts.
- **Auto mode** — used for step 5 (Implement). Picks one `plan.md` (latest by
  default, or whichever feature the user names) and works through its checklist
  end-to-end: executing tasks, marking checkboxes, committing changes, halting on
  failure.

Do not mix modes. Plan mode never executes tasks. Auto mode never edits
`spec.md` / `plan.md` / `details.md` beyond ticking the plan's checkboxes as
tasks complete.

## Workflow steps

### 1. Specify
Write `spec.md` per the structure above. Every requirement gets an `FR-###`; every
outcome gets an `SC-###`. Call out informed defaults.

### 2. Clarify (before planning)
Scan `spec.md` for ambiguity: vague terms, missing edge cases, untestable
requirements, undefined data shapes, unstated assumptions. Ask the user
focused clarifying questions, one at a time. Record each answer in a
`## Clarifications` section of `spec.md` under a `### Session YYYY-MM-DD`
subheading, and update affected FR/SC/edge-case sections inline.

### 3. Plan
Write `plan.md` and `details.md` together. `details.md` captures decisions and
context; `plan.md` captures the granular checklist that realizes them.

### 4. Analyze (before implementing)
Cross-check `spec.md`, `plan.md`, `details.md` for:

- **Coverage gaps** — every `FR-###` and `SC-###` has at least one task that
  delivers it and at least one task that verifies it.
- **Inconsistencies** — terminology drift, contradictory statements between
  documents.
- **Ambiguities** — vague tasks that won't survive auto-mode execution.
- **Untestable requirements** — FRs/SCs without a clear way to check them.

Report findings to the user before implementation begins. Do not enter auto mode
while critical issues are open.

### 5. Implement (auto mode)
When the user asks to implement a plan in auto mode:

1. **Resolve the active plan.** Default to the highest-numbered
   `specs/NNN-*/plan.md`. The user may name a different feature explicitly; respect
   that override.
2. **Load context.** Read `.specify/memory/constitution.md`, then the feature's
   `spec.md`, `plan.md`, and `details.md`.
3. **Work through `plan.md` tasks in order, top to bottom.** Do not reorder or
   skip ahead.
4. **For each task:**
   - If marked `[complex]`, spawn a subagent (`Agent` tool) to execute it. Brief
     the subagent with the task text, the relevant spec/plan/details excerpts, and
     enough context to act without re-discovering the project.
   - Otherwise, execute the task directly.
   - On completion, the executing agent (or the main session for non-complex
     tasks) **marks the corresponding checkbox in `plan.md` as `[x]`**.
   - If the task produced file changes, the executing agent creates a git commit
     before moving on. The commit message should reference the task text.
5. **Halt on failure.** If a task fails (build broken, test red, subagent reports
   an unrecoverable error):
   - Stop immediately.
   - Leave the task's checkbox unchecked.
   - Do not commit partial work.
   - Report the failure to the user, including which task failed and why.
   - Do not skip the task and continue.
6. **Final verification.** Once all tasks are checked, confirm the implementation
   satisfies the acceptance criteria in `spec.md` (the `SC-###`s and acceptance
   scenarios).
