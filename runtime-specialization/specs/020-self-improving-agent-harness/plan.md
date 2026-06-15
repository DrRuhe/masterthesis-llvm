# Plan: Self-Improving Agent Harness

- [x] Confirm the current workspace has no existing parent/docs `AGENTS.md` hierarchy and identify the shared reflection journal location.
- [x] Write `specs/020-self-improving-agent-harness/spec.md` with functional requirements and success criteria for the harness.
- [x] Write `specs/020-self-improving-agent-harness/details.md` capturing the reflection schema, scoping rules, and approval flow.
- [x] Add `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/AGENTS.md` defining the cross-workspace reflection harness, scoping rules, and skill-update policy.
- [x] Add `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/AGENTS.md` defining docs-specific workflow and how docs lessons feed the shared reflection harness.
- [x] Refactor `AGENTS.md` in `llvm/runtime-specialization` so it keeps project-specific rules while delegating shared harness behavior to the parent file.
- [x] Add `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/.codex/reflection.md` as the append-only journal template for future session reflections.
- [x] Verify the three `AGENTS.md` files have non-overlapping scopes and all point to the shared journal/approval flow consistently.
- [x] Verify the runtime-specialization `AGENTS.md` still requires reading `.specify/memory/constitution.md` and `specs/TODO.md` before relevant work.
