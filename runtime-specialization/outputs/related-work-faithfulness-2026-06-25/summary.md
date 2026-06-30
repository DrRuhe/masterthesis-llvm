# Related Work Faithfulness Review

Reviewed section: [/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/thesis.typ:168](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/thesis.typ:168)

Per-paper reviews: `outputs/related-work-faithfulness-2026-06-25/per-paper/`

## Overall verdict

- Reviewed works: `27`
- Result: `0` fully clean, about `4` mostly faithful, about `23` partially faithful
- Dominant issue: the section is usually accurate on concrete mechanisms, but it repeatedly asks single system papers to support broader survey/taxonomy/novelty claims than they actually carry.

## Highest-priority corrections

- The discussion of `@consel_general_1996` is materially wrong on overhead and evidence. The paper explicitly avoids full run-time recompilation, reports concrete preliminary speedups, and does not support "unclear benefit" or "no concrete speedups". See [consel_general_1996.md](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/outputs/related-work-faithfulness-2026-06-25/per-paper/consel_general_1996.md).
- The AnyDSL paragraph overstates what `@leisa_anydsl_2018` proves. This paper supports compile-time online PE, not the stronger claim that AnyDSL has no runtime-specialization support in general; it also should not treat `@@` as just another filter. See [leisa_anydsl_2018.md](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/outputs/related-work-faithfulness-2026-06-25/per-paper/leisa_anydsl_2018.md).
- The Truffle/Graal comparison overstates loss of user control and understates what values and assumptions drive specialization in `@wurthinger_practical_2017`. The claim that Truffle cannot specialize against running-program values is too strong as written. See [wurthinger_practical_2017.md](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/outputs/related-work-faithfulness-2026-06-25/per-paper/wurthinger_practical_2017.md).
- The `weval` discussion overclassifies `@fallin_partial_2024` as a runtime PE system. The paper supports IR-level specialization of interpreters with bytecode, but the current implementation is framed much more as AOT/snapshot-style than as a Truffle-like runtime PE interface. See [fallin_partial_2024.md](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/outputs/related-work-faithfulness-2026-06-25/per-paper/fallin_partial_2024.md).
- The opening survey sentences and several JIT/history sentences are over-supported by narrow system papers such as `@deutsch_efficient_1984`, `@kistler_dynamic_1997`, and `@futamura_partial_1999`. These citations are good historical anchors for specific ideas, but weak support for cross-community framing. See [deutsch_efficient_1984.md](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/outputs/related-work-faithfulness-2026-06-25/per-paper/deutsch_efficient_1984.md), [kistler_dynamic_1997.md](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/outputs/related-work-faithfulness-2026-06-25/per-paper/kistler_dynamic_1997.md), and [futamura_partial_1999.md](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/runtime-specialization/outputs/related-work-faithfulness-2026-06-25/per-paper/futamura_partial_1999.md).

## Recurring patterns

- Mechanism descriptions are usually fine. This is true for MetaML, MetaOCaml, MacoCaml, Scala 3 staging, LMS, BuildIt, Nautilus, HyPer, and LingoDB-related claims.
- Literature-wide synthesis is the weak point. Claims like "existing MSP systems require ...", "the field contains ...", "we are not aware of ...", and "users have no control ..." often go beyond what the cited paper itself establishes.
- Several citations are being used one level too high:
  - use survey/dissertation citations for field-wide architecture claims
  - use system papers for concrete mechanism claims
  - avoid making novelty claims rest on a comparator paper alone

## Stronger framing that would improve the section

- Split "broad survey" support from "mechanism" support more aggressively. `@grulich_query_2023` is strong for query-compilation design space; system papers like `@grulich_query_2024`, `@neumann_efficiently_2011`, and `@jungmair_designing_2022` are stronger for concrete architecture points than for field-wide taxonomy.
- Narrow the MSP contrast from "all existing MSP systems require visible staging markers" to "the representative systems discussed here expose staging through quotations, code types, or staged value types, even if user-facing wrappers can sometimes hide that interface".
- Narrow the runtime-PE contrast from "users have no control and specialization is unpredictable" to "specialization timing is runtime-policy-driven rather than requested explicitly at a concrete application call site".
- Replace negative novelty claims like "we are not aware of a query-compilation system ..." with positive, directly defensible claims about your own interface: for example, "in the systems reviewed here, usability-oriented designs still expose staging information through typed staged values, query IRs, or interpreter frameworks."

## Best next edits

- Rewrite the opening paragraph so it cites survey/foundational sources only, and stop asking narrow system papers to carry the cross-community framing.
- Rewrite the Consel paragraph first; it contains the clearest factual errors.
- Rewrite the AnyDSL and Truffle/weval comparison paragraphs next; these contain the sharpest overstatements in the current comparison to your system.
- Rewrite the query-compilation novelty paragraph last; most of its issue is claim strength, not mechanism accuracy.
