# Findings 024: Implementation Chapter Analysis

**Date**: 2026-06-30  
**Target**: `docs/thesis.typ`, chapter `= Implementation <implementation>`  
**Scope**: Review-only analysis of the current implementation chapter before any prose edits.

## Review Criteria

Each subsection was evaluated against the following questions:

- `Completeness`: Are important mechanisms, contracts, limits, and failure modes stated?
- `Necessity`: Does the detail earn its place in a thesis implementation chapter?
- `Rationale`: Does the text explain why this design was chosen instead of a simpler alternative?
- `Evidence strength`: Does the text claim more than the implementation or later evaluation supports?
- `Process clarity`: Does the chapter preserve one clear execution narrative?
- `Terminology`: Does the wording follow the thesis glossary precisely enough?

## Bottom Line

The chapter already explains the main execution path well: compile-time target resolution, per-TU IR-blob registration, runtime symbol reconnection, wrapper-based specialization, and the qualitative differences between Pipelines 0, 1, and 2 are all understandable from the current text.

The blockers are not the core mechanism, but the chapter's contracts and boundaries. The current draft still contains one visible `#TODO`, overstates the determinism of name-to-blob lookup, under-specifies the startup/API/failure contract, and includes several engineering details that do not yet contribute enough thesis value.

## Priority Findings

### 1. Publication blocker: visible TODO and overstated lookup guarantees

- `qualify` [high]: `Resolving IR Symbols and Host Addresses` (`docs/thesis.typ:875-883`) contains a visible `#TODO` and currently overstates the runtime mapping as deterministic.
- The text should say what the current implementation actually guarantees:
  direct resolved-name lookup without heuristic whole-program search, but only warning-based handling when duplicate names are registered.
- This is the clearest mismatch between prose and implementation reality, and it weakens the correctness story in the middle of the chapter.

### 2. Missing startup and failure contract

- `expand` [high]: `Initialization` and `Runtime Specialization Interface` (`docs/thesis.typ:860-906`) describe the happy path, but not the contract a reader needs in order to understand the system end to end.
- Missing topics:
  - whether specialization sites must be compiled with the plugin
  - whether `init()` is explicit, implicit, or both
  - what happens when no IR blob is registered
  - what happens when specialization fails
  - how duplicate resolved names are handled
- These gaps matter because the chapter otherwise sounds more total and automatic than the current system actually is.

### 3. Missing operational semantics for the public interface

- `expand` [high]: `Runtime Specialization Interface` (`docs/thesis.typ:895-906`) is too thin for semantics that affect how readers interpret first-call and amortized costs later in the thesis.
- The text should clarify:
  - whether identical requests are recompiled or reused
  - what the returned handle owns
  - when a handle becomes invalid
  - how `callSpecialized` differs operationally from `specializeOnly`
  - why all three entry points exist instead of exposing only one reusable primitive plus wrappers

### 4. The wrapper mechanism needs one more correctness bridge

- `expand` [medium]: `Wrapper Construction` (`docs/thesis.typ:912-920`) correctly states that pointers and objects become host addresses, but it does not restate the key invariant that only loads later proven runtime-invariant may be folded.
- Without that sentence, readers can misread the mechanism as "object state becomes constant" instead of the more accurate "address becomes constant, and selected reachable loads may later become specialization constants if analysis permits."
- This is especially important because the earlier correctness chapter already established the invariant; the implementation chapter should reconnect to it briefly.

### 5. Several design choices are described without enough "why this way?"

- `expand` [medium]: the chapter often explains what CRS does, but not why the chosen design beats a simpler alternative.
- The highest-value rationale gaps are:
  - `IRRewritingPass` supported-call-site boundary (`docs/thesis.typ:811-825`)
  - automatic registration plus deferred `init()` rather than one eager startup path (`docs/thesis.typ:862-865`)
  - three public entry points instead of a smaller interface (`docs/thesis.typ:897-906`)
  - shallow serialization instead of deep-copy/ownership-tracking alternatives (`docs/thesis.typ:900`)
  - wrapper-based constant injection instead of directly specializing target bodies (`docs/thesis.typ:916-920`)
  - three named pipelines instead of one monolithic configurable pipeline (`docs/thesis.typ:933-964`)

### 6. The execution-order narrative weakens in the middle

- `condense/reorder` [medium]: the chapter promises to describe the system "in the order in which it executes" (`docs/thesis.typ:802-805`), but `Runtime Specialization Interface` and `Debug vs. Release Context Handling` interrupt the runtime flow between symbol resolution and specialization-time construction.
- The implementation chapter should either:
  - keep the process narrative strict and move side topics out of the main path, or
  - mark these sections as API/context digressions so the reader does not expect a pure execution trace.

### 7. Some details are low-value engineering noise at current thesis depth

- `remove` [medium]: `Debug vs. Release Context Handling` (`docs/thesis.typ:926-931`) reads like an LLVM-debugging workaround, not a thesis-relevant mechanism. Keep it only if later claims depend on this split.
- `remove` [medium]: wrapper naming scheme details (`docs/thesis.typ:922-923`) do not support a later thesis claim and can be dropped.
- `remove` [medium]: the Pipeline 2 maintenance note about patch structure and rebasing (`docs/thesis.typ:964`) is repository-maintenance commentary, not thesis argument.
- `remove-or-justify` [medium]: `Instruction Instrumentation` (`docs/thesis.typ:966-972`) should stay only if later evaluation or discussion explicitly interprets instrumentation-derived evidence.
- `condense` [medium]: the `IRDumpingPass` preprocessing list (`docs/thesis.typ:834-849`) should emphasize the transformations that matter thesis-wise:
  constructor removal, host-global reuse, dead-code pruning, and preservation of devirtualization metadata.

### 8. Repeated or too-strong rationale should be tightened

- `condense` [low]: `IRRewritingPass` and `IRDumpingPass` both repeat the same broad "do structure work at compile time so runtime stays cheap" motivation. The distinction should be sharper:
  early rewriting preserves recoverability; late dumping amortizes cleanup and size reduction.
- `qualify` [low]: `fully annotation-free and deterministic` (`docs/thesis.typ:819`) is too strong. `annotation-free` is supported; `deterministic` needs qualification.
- `qualify` [low]: `remaining JIT preparation is intentionally lightweight` (`docs/thesis.typ:924`) sounds stronger than the surrounding evidence supports.

### 9. Terminology discipline should be improved

- `qualify/normalize` [low]: the chapter sometimes uses looser terms such as `blob`, `runtime constants`, and "runtime values become compile-time constants" where the glossary has sharper distinctions.
- The most important normalization targets are:
  - `IR blob`
  - `runtime-invariant value`
  - `specialization constant`
  - `runtime specializer`
  - `JIT` for the compiler machinery rather than the whole system

### 10. The chapter lacks a closing synthesis

- `expand` [high]: after `Instruction Instrumentation`, the text jumps directly to `= Evaluation` (`docs/thesis.typ:966-976`).
- Add a short closing paragraph that states which implementation choices are essential and what the evaluation is about to test:
  correctness-preserving runtime constant injection, symbol/state reconnection, and the tradeoff between specialization quality and JIT overhead across the pipeline variants.

## Subsection Dispositions

- `Compile-Time Preparation`: keep, but expand supported-boundary rationale and condense the preprocessing laundry list.
- `Runtime Initialization and Symbol Resolution`: keep, but qualify guarantees and add explicit contract/failure behavior.
- `Runtime Specialization Interface`: keep, but condense repeated API exposition and expand operational semantics.
- `Wrapper Construction`: keep, but add the constant-address vs invariant-load distinction.
- `Debug vs. Release Context Handling`: remove unless later evidence depends on it.
- `JIT Pipelines`: keep, but avoid repeating the same tradeoff framing three times.
- `Instruction Instrumentation`: remove unless later sections use those counters.

## Recommended Remediation Order

1. Remove the visible `#TODO` and qualify the duplicate-name lookup semantics.
2. Add the missing startup/API/failure contract details.
3. Add the missing correctness bridge in `Wrapper Construction`.
4. Add a short closing synthesis before `= Evaluation`.
5. Condense or remove thesis-low-value engineering details.
6. Normalize terminology and tighten repeated rationale.

## Result

The implementation chapter does not need a conceptual rewrite. It needs a contract pass, a qualification pass, and a compression pass. If those are done carefully, the existing execution-order structure is already strong enough to support the evaluation chapter.
