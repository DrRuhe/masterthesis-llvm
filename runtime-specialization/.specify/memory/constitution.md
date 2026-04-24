<!--
SYNC IMPACT REPORT
==================
Version change: 2.1.0 → 2.2.0
Added sections:
  - Claude Interaction Protocol (AskUserQuestion tool requirement; clarify during spec/plan)

---
Previous: 2.1.0 → 2.2.0
Added sections:
  - Specialization Scope Constraint (single-threaded programs only; no concurrent mutation of specialized arguments)
Templates requiring updates: None.
Follow-up TODOs: None.

---
Previous: 1.0.0 → 2.0.0
Modified principles:
  - IV. Benchmark-Driven Performance → IV. Performance Measurement (softened from MUST to SHOULD;
    removed strict 10% regression threshold; reflects research-prototype reality)
Added sections:
  - Project Purpose (engineering decisions must be motivated/documented)
  - Backwards Compatibility (no legacy support; DuckDB data preservation exception)
  - System Environment (flake.nix as single env spec)
  - Project Architecture (consolidated from CLAUDE.md)
  - Key File Locations (consolidated from CLAUDE.md)
  - Build & Test Commands (consolidated from CLAUDE.md)
  - Documentation Requirements (consolidated from CLAUDE.md)
Removed sections: None (content from Build & Environment Constraints merged with new sections)
Consolidated: /home/Jakob.Gerhardt/CLionProjects/Masterarbeit/CLAUDE.md (deleted after merge)
              /home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/CLAUDE.md (deleted after merge)
Templates requiring updates:
  - .specify/templates/plan-template.md ✅
  - .specify/templates/spec-template.md ✅
  - .specify/templates/tasks-template.md ✅
Follow-up TODOs: None.
-->

# LLVM Runtime Specializer Constitution

## Project Purpose

This project is a **Master's thesis** investigating JIT-based runtime specialization using
LLVM. The primary goal is to understand where, how, and to what extent runtime
specialization of C++ functions yields measurable performance improvements.

**Non-negotiable consequence**: every significant engineering decision MUST be motivated by
a research reason (correctness, performance, generality, simplicity). That motivation MUST
be documented in the relevant specification under `.specify/specs/` before or alongside
implementation — not reconstructed after the fact. Decisions that exist purely for
engineering convenience and cannot be tied back to the research questions are candidates
for removal.

## Specialization Scope Constraint

Runtime specialization in this project is **exclusively evaluated on single-threaded programs**
(or on code paths where no concurrent thread can observe or mutate the arguments being
specialized during the specialization call).

**Non-negotiable consequence**: the JIT bakes runtime argument values in as IR constants at
the moment `callSpecialized` / `specializeOnly` is called. If another thread can concurrently
modify the memory those arguments point to, the baked-in constant is stale and the specialization
is semantically incorrect. Because the thesis studies the *performance benefit* of specialization,
not its thread-safety properties, this scenario is out of scope and MUST NOT be used as a
benchmark or correctness test case.

Concretely:
- Benchmark workloads MUST NOT share specialized-function arguments across threads while a
  specialization is in progress.
- The `init()` singleton MUST be initialized from a single thread before any concurrent
  `callSpecialized` / `specializeOnly` calls are made (see also the single-thread `init()`
  assumption in spec 003).
- Any future multi-threaded workload MUST document why concurrent argument mutation is
  impossible before it is admitted as a valid specialization target.

**Rationale**: Correctness is the prerequisite for all performance claims. Specializing under
concurrent mutation would produce results that cannot be trusted, undermining the thesis argument.

## Core Principles

### I. Correctness & Safety (NON-NEGOTIABLE)

Specialized functions MUST be semantically equivalent to the original function for all
input values. JIT-compiled code MUST NOT introduce undefined behaviour beyond what the
source already contains. The public API MUST NOT expose LLVM internals that could be
misused (no raw `Module`/`Function` pointers in public signatures). Equivalence
assertions (`assertSpecializedIsEquivalent`) MUST be used in test code to verify
correctness of new specialization paths.

**Rationale**: Incorrect specialization silently corrupts program results; detection after
the fact is hard. Safety is the prerequisite for every other property.

### II. LLVM Coding Standards

All C++ code MUST follow LLVM coding standards:

- Error handling via `llvm::Expected<T>` / `llvm::Error`; raw exceptions are forbidden.
- Ownership expressed with `std::unique_ptr`; raw owning pointers are forbidden.
- LLVM utilities preferred over STL equivalents where an LLVM type exists
  (`llvm::Twine`, `llvm::ArrayRef`, `llvm::StringRef`, `llvm::SmallVector`, etc.).
- Logging via the `CRS_LOG(Level, Message)` macro; `std::cerr`/`printf` are forbidden
  in library code.
- Namespace: `clangRuntimeSpecializer` for all new symbols.

**Rationale**: Consistency with the surrounding LLVM codebase makes the library
maintainable and ensures correct interaction with LLVM's memory and error model.

### III. Test-First Validation

All new functionality MUST be covered by `lit`/`FileCheck` integration tests before the
feature is considered complete:

- Tests are placed in `test/WIP/` during development and MUST pass
  `ninja check-wip-runtime-specializer` before advancing.
- Tests MUST be promoted to `test/smoke/` or `test/complete/` upon feature completion.
- CMake targets (`ninja check-*`) MUST be used exclusively; invoking `llvm-lit` directly
  is forbidden — it uses different environment settings and will fail due to linking
  issues. The test suite auto-builds all necessary dependencies.
- The full smoke suite (`ninja check-smoke-runtime-specializer`) MUST remain green on
  every commit to the main branch.

**Debugging test failures**:
1. Confirm you are using the correct CMake target (not `llvm-lit`).
2. Check generated scripts in `build/debug/tools/runtime-specialization/test/WIP/Output/`.
3. Add `--dump-input=always` to the `FileCheck` line for a detailed diff.

**Rationale**: LLVM's JIT pipeline is complex; manual inspection is insufficient to catch
regressions. The lit/FileCheck framework provides the environment LLVM expects.

### IV. Performance Measurement

This is a research prototype where benchmarking infrastructure is actively being
developed. Performance measurement is important but MUST be approached pragmatically:

- Where benchmarking infrastructure is in place, performance claims SHOULD be supported
  by recorded benchmark data (DuckDB database under `benchmarks/`).
- Benchmark helpers in `ClangRuntimeSpecializerBenchmark.h` SHOULD be used for
  consistent measurement; ad-hoc timing in production code paths is forbidden.
- Obvious regressions (e.g., order-of-magnitude slowdowns) MUST be investigated before
  merging; precise regression thresholds are intentionally not enforced while the
  benchmarking setup matures.
- When benchmarking infrastructure does not yet cover a given scenario, a documented
  qualitative justification in the specification is sufficient.

**Rationale**: The thesis argument benefits from empirical data, but demanding perfect
coverage from an evolving benchmarking system would block progress on the actual research.

### V. Minimal Public API

The public API surface MUST remain minimal and ergonomic:

- Dead API (unused public methods/types) MUST be removed rather than retained for
  hypothetical future use.
- No premature abstractions: three similar call-sites are preferable to a wrapper that
  exists only to reduce three lines.
- Every addition to the public API requires both a test and a thesis documentation update.
- Template parameters MUST NOT encode information that can be passed as a runtime
  argument (function names, option flags, etc.).

**Rationale**: A bloated API increases cognitive overhead for library users and creates
maintenance debt that conflicts with the thesis timeline.

## Backwards Compatibility

This is a WIP research prototype with no existing users and no legacy systems that require
support. Therefore:

- Extra code paths, conditional branches, or API overloads whose sole purpose is to
  remain backwards compatible with a previous internal version MUST NOT be created.
- When a better design is found, old interfaces are deleted outright; callers are updated
  in the same change.

**Exception — DuckDB benchmark data**: The schema and import scripts for the benchmark
database (`benchmarks/benchmarks.duckdb`) MUST be evolved with explicit migrations.
Data MUST NOT be lost. If a schema change is needed, depending on complexity write a migration script before
altering the schema, and verify the existing data survives the migration. For incompatible schema changes, 
ask the user if archival (moving the db file into benchmarks/db_archive) of the old data is permissible to avoid migration.

## System Environment

The system environment is fully specified by the Nix flake at:

```
/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/flake.nix
```

This file is the single, authoritative, self-documenting environment specification.
Parallel environment management is forbidden:

- Missing Python modules MUST be added to `flake.nix`, not installed via `pip install`.
- Missing system tools MUST be added to `flake.nix`, not installed via `apt`, `brew`,
  or any other package manager.
- Version pins belong in `flake.nix`; pinning via `.python-version`, `.nvmrc`, or similar
  side-channel files is forbidden.

## Project Architecture

### How Runtime Specialization Works

1. **Compile-time**: The `runtime-specialization-IR-dumping` LLVM pass dumps each
   function's bitcode into the executable as a data blob.
2. **Runtime init**: `ClangRuntimeSpecializer::init()` locates and parses those blobs.
3. **Specialization**: `callSpecialized<R>(funcName, args...)` builds a thin wrapper that
   substitutes runtime argument values as LLVM constants, then JIT-compiles it.
4. **Optimization**: The JIT pipeline (IPSCCP → DevirtVtable → GlobalOpt →
   ConstantArgAlwaysInlinePass → AlwaysInliner → GVN, in a fixpoint loop, followed by
   O3) propagates the constants and eliminates dead code.
5. **Execution**: The optimized specialized function pointer is cached and called directly.

### Key Implementation Symbols

| Symbol | Location | Role |
|--------|----------|------|
| `callSpecialized<R>()` | `ClangRuntimeSpecializer.h` | Primary public entry point |
| `serializeArgumentToIR()` | `.cpp` | Converts runtime values → LLVM constants |
| `buildWrapperIR()` | `.cpp` | Creates the specialization wrapper |
| `prepareModuleForJIT()` | `.cpp` | Fixes linkage for JIT (vtables, globals, ctors) |
| `addModuleAndLookup()` | `.cpp` | Compiles module, returns function pointer |
| `IRDumpingPass` | `comptime/` | Compile-time pass that embeds bitcode blobs |

### Key File Locations

```
runtime-specialization/
├── runtime/ClangRuntimeSpecializer/
│   ├── ClangRuntimeSpecializer.h      # Public API + header-only helpers
│   └── ClangRuntimeSpecializer.cpp    # Core JIT engine
├── comptime/                          # LLVM IR-dumping pass
├── benchmarks/
│   ├── ClangRuntimeSpecializerBenchmark.h  # Benchmark helpers
│   ├── polybench/                     # Polybench kernel benchmarks
│   ├── tpch/                          # TPC-H query benchmarks
│   └── benchmarks.duckdb              # Recorded results
├── test/
│   ├── WIP/                           # Tests under active development
│   ├── smoke/                         # Core invariant tests (always green)
│   └── complete/                      # Extended test suite
└── .specify/memory/constitution.md    # This file
```

Thesis (Typst): `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/thesis.typ`
Nix-Flake:      `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/flake.nix`
LLVM repo:      `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/`
Build (debug):  `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/llvm/build/debug`
Build (release):`/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/llvm/build/release`

To understand current in-progress state, check uncommitted changes in the LLVM repo:

```bash
cd /home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm && git status -s
```

## Build & Test Commands

All build and test commands run from the debug build directory unless noted.

```bash
# --- Build ---
cd /home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/llvm/build/debug

ninja ClangRuntimeSpecializer          # Build the runtime library only

# --- Test (always use ninja targets, never llvm-lit directly) ---
ninja check-wip-runtime-specializer    # WIP tests (inner development loop)
ninja check-smoke-runtime-specializer  # Core smoke suite (gate before commit)
ninja check-all-runtime-specializer    # Full suite (before feature completion)

# --- Inspect test output ---
cat tools/runtime-specialization/test/<suite>/Output/<test-name>.cpp.script

```

### Test File Template

```cpp
// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe 1 | FileCheck %s --check-prefix=EXE --dump-input=always

// EXE: expected output here
int main(int argc, char** argv) {
  // Use the runtime specializer here
}
```

## Documentation Requirements

The specifications MUST document the research:

1. Describe what changed and why (research motivation).
2. Compare the current implementation against what the thesis already says; fix
   mismatches.
3. Update `docs/thesis.typ` with:
   - Concise bullet points.
   - Technical details without elaborate examples.
   - Accurate reflection of the current implementation.

**Typst style**: bullet points; extremely concise; focus on "why","what"; minimal
examples; no lengthy "how" narratives.

## Development Workflow

1. **WIP tests first**: write a test in `test/WIP/` that captures expected behaviour
   before writing implementation code.
2. **Iterate**: `ninja check-wip-runtime-specializer` is the inner loop; fix until green.
3. **Smoke gate**: run `ninja check-smoke-runtime-specializer` before every commit;
   green is a hard gate on the main branch.
4. **Promote tests**: move passing WIP tests to `test/smoke/` (core invariants) or
   `test/complete/` (extended coverage) before merging.
5. **Benchmark** (where infrastructure exists): record results for pipeline or API
   changes.
6. **Document**: update `docs/thesis.typ`; see Documentation Requirements above.
7. **Specification**: ensure the relevant spec under `.specify/specs/` reflects the
   engineering decision and its research motivation.

## Claude Interaction Protocol

- **Clarifying questions MUST use the `AskUserQuestion` tool** — never embed questions in
  plain text output. Questions written as prose are easily missed; `AskUserQuestion`
  surfaces them as a distinct UI element the user can respond to directly.
- **Clarifying questions MUST be asked during specification and planning** (i.e., when
  running `/speckit-clarify`, `/speckit-specify`, or `/speckit-plan`) — not deferred to
  implementation. Open ambiguities found late cause rework; resolve them before code is
  written.

## Governance

This constitution is the single authoritative reference for non-negotiable rules on the
LLVM Runtime Specializer project. It supersedes any conflicting guidance in comments,
commit messages, or informal notes. All project instructions are contained here inline —
no external documents are required to understand the rules.

**Amendment procedure**:
1. Identify the principle or section to change.
2. Bump the version: MAJOR for removals/redefinitions, MINOR for new sections/principles,
   PATCH for clarifications.
3. Update this file and the Sync Impact Report comment at the top.
4. If the change affects the development workflow, update the relevant specification.

**Versioning policy**: `MAJOR.MINOR.PATCH` per the rules above.

**Compliance review**: every feature plan (`/speckit-plan`) MUST include a Constitution
Check section that gates Phase 0 research on principle compliance.

**Version**: 2.2.0 | **Ratified**: 2026-04-23 | **Last Amended**: 2026-04-24
