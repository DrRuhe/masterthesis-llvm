# Clarifications Needed After 2026-06-23 TODO Pass

This document lists the remaining thesis tasks that I did not continue automatically because they now require either a user decision, external evidence, or a broader writing push that is no longer a simple TODO cleanup.

## Editorial decisions needed in `docs/thesis.typ`

### 1. Introduction findings summary
- **Location**: `docs/thesis.typ` line ~150
- **Current TODO**: `#TODO[for each evaluation question, put $<5$ sentence summaries on the findings]`
- **Clarification needed**: Do you want the introduction to preview all six research questions, or only the strongest headline results?
- **Why I stopped**: This is an editorial scope choice, not a mechanical fill-in. A dense six-item preview can either strengthen the story or overload the introduction.

### 2. Related-work explanation of online vs. offline partial evaluation
- **Location**: `docs/thesis.typ` lines ~211–212
- **Current TODOs**:
  - `#TODO[this is not intuitive, what does this mean?]`
  - `#TODO[how do we come to this conclusion?]`
- **Clarification needed**: Do you want a compact intuitive explanation for non-PL readers, or do you want this distinction cut back to the minimum needed for the thesis argument?
- **Why I stopped**: The right rewrite depends on how much PL background you want to assume from the reader.

### 3. API example snippets
- **Location**: `docs/thesis.typ` lines ~486–495
- **Current TODOs**:
  - `TODO[code snippet showing specialization with function]`
  - `TODO[code snippet showing specialization with lambda]`
  - `#TODO[show example snippets of the current usage]`
- **Clarification needed**: Which API surface should be the canonical example: `callSpecialized`, `specializeOnly`, `specializeLambda`, or a combination?
- **Why I stopped**: I can add snippets, but the examples should match the story you want the thesis to tell about the preferred user-facing interface.

### 4. JIT pipeline presentation depth
- **Location**: `docs/thesis.typ` lines ~568, ~621, ~638
- **Current TODOs**:
  - `#TODO[I'm worried that we might have a problem with understandability here: It seems like we just throw a bunch of terms at the reader. ]`
  - `#TODO[polish the wording]`
  - `#TODO[list the different patterns which we aim to specify. and the reasoning why we target this pattern]`
- **Clarification needed**: Do you want this section to stay implementation-close, or should it become a more conceptual explanation with fewer pass names and more "problem -> mechanism -> effect" framing?
- **Why I stopped**: Both directions are valid, but they produce very different prose and figure/snippet needs.

### 5. Vtable-pointer undefined-behavior citation policy
- **Location**: `docs/thesis.typ` line ~635
- **Current TODO**: `#TODO[which standard, please put a link here]`
- **Clarification needed**: Is citing a secondary source such as cppreference acceptable here, or do you want a direct ISO C++ standard reference only?
- **Why I stopped**: The content is straightforward, but the acceptable citation standard is your editorial choice.

### 6. Unified speedup terminology
- **Location**: `docs/thesis.typ` line ~1121
- **Current TODO**: `#TODO[refactor the thesis to use these unified definitions for speedups.]`
- **Clarification needed**: Which notation should be treated as final throughout the thesis: `U/S`, "execution speedup", "amortized speedup", and `$U_p$` as currently drafted, or a different normalized vocabulary?
- **Why I stopped**: This requires a document-wide terminology sweep once the final naming scheme is fixed.

### 7. Figure/example quality concerns
- **Location**: `docs/thesis.typ` line ~1324
- **Current TODO**: `#TODO[these still seem weird and contrived. they dont actually show me anything]`
- **Clarification needed**: Should these examples be replaced entirely, or do you want them kept but re-explained?
- **Why I stopped**: This needs your judgment about whether the current examples are conceptually wrong or only poorly motivated.

### 8. Discussion comparison table
- **Location**: `docs/thesis.typ` line ~1698
- **Current TODO**: qualitative comparison table plus quantitative follow-up
- **Clarification needed**: Do you want a compact qualitative table only, or a stronger table that also reports concrete overhead numbers where the literature supports them?
- **Why I stopped**: This affects how much related-work verification and source triangulation is needed.

### 9. Future-work and conclusion scope
- **Location**: `docs/thesis.typ` lines ~1790 and ~1892
- **Current TODOs**:
  - future-work drafting block
  - conclusion drafting block
- **Clarification needed**: Should the conclusion stay tightly empirical, or should it also restate the implementation contributions in a more systems-oriented framing?
- **Why I stopped**: This is mostly writing work, but the thesis tone depends on your preferred closing emphasis.

## Tracker decisions needed in `specs/TODO.md`

### 10. Thesis size-scaling figure with missing `box_filter` point
- **Tracker item**: `RQ1-006`
- **Clarification needed**: Should I treat the three measured `box_filter` sizes as intentional and adjust the prose/figure accordingly, or should I keep treating the figure as incomplete until the missing `EXTRALARGE` point is recovered?
- **Why I stopped**: Both are reasonable, but they lead to different claims about provenance and completeness.

### 11. PolyBench partial-specialization appendix
- **Tracker item**: `RQ3-003`
- **Clarification needed**: Once the best-practice study is available, do you want the thesis appendix to document every PolyBench kernel's specialization choice, or only representative categories?
- **Why I stopped**: The scope of that appendix changes how much generated detail belongs in the thesis body versus the appendix.

## External evidence or author-input blockers

### 12. Nautilus comparison data
- **Tracker item**: `RW-001`
- **Clarification needed**: Are you willing to accept a comparison with only the published numbers we can extract locally, or do you want this left blocked unless we obtain clearer latency data from the paper or the authors?

### 13. Abstract and German summary
- **Location**: `docs/thesis.typ` lines ~9 and ~16
- **Current TODOs**:
  - `#TODO[Write an Abstract]`
  - `#TODO[Schreibe eine Zusammenfassung]`
- **Clarification needed**: Do you want me to draft these now from the current thesis state, or only once the evaluation and conclusion wording are frozen?

## Remaining work that is complex but does not need clarification

These items are still actionable later, but they are no longer simple TODO cleanup:

- `D-001`: move and tighten the existing design-journey material into a coherent chapter narrative.
- `RQ1-004` / `RQ6-001`: synthesize the specialization failure cases into thesis-quality prose.
- `RQ1-007`: build the "specializeable architectures" taxonomy from repo evidence.
- Large drafting blocks in the future-work and conclusion sections.
