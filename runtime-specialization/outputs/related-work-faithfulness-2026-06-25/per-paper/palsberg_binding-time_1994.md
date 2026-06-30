## Verdict

Partially faithful, but too broad. Palsberg and Schwartzbach (1994) supports the classic `static`/`dynamic` binding-time vocabulary and the idea that binding-time analysis marks what can be eliminated during partial evaluation. It does **not** strongly support the thesis's broader wording about `static values` in general, nor is it the right citation for the offline-vs-online contrast. This paper is a comparison of two binding-time analyses for the **pure lambda calculus**, not a general survey of partial evaluation practice.

Primary source: [IEEE DOI](https://doi.org/10.1109/ICCL.1994.288372)  
Useful companion source for the broader offline-C-language claim: [Andersen 1993, ACM DOI](https://dl.acm.org/doi/10.1145/154630.154636)

## Supported

- The thesis is justified in citing this paper for the classic binding-time distinction between `static (compile-time)` and `dynamic (run-time)`. The paper states exactly that in its introduction.
  Source: [IEEE DOI](https://doi.org/10.1109/ICCL.1994.288372)

- The paper supports the claim that binding-time analysis is used to determine which program parts can be evaluated when some input is known, and that static entities are the ones eliminated during partial evaluation.
  Source: [IEEE DOI](https://doi.org/10.1109/ICCL.1994.288372)

- It also supports using two-level annotated terms as the presentation form of binding-time information in classic offline PE work.
  Source: [IEEE DOI](https://doi.org/10.1109/ICCL.1994.288372)

## Unfounded or Overstated

- `static values` is too loose for this citation. The paper is explicitly about the pure lambda calculus and even notes that, in that setting, there are no first-order values and no static input in the sense used in richer languages. That makes it a weaker citation for a general statement about `values` than the thesis currently suggests.
  Source: [IEEE DOI](https://doi.org/10.1109/ICCL.1994.288372)

- The citation is too weak for `Offline partial evaluators determine this distinction through a binding-time analysis before specialization`. That statement is directionally compatible with the paper, but this paper's actual contribution is narrower: it compares two BTA formulations and proves one more powerful than the other. Andersen (1993) is the stronger citation for an explicit offline pipeline that constructs a two-level program and feeds it to an offline partial evaluator.
  Sources: [IEEE DOI](https://doi.org/10.1109/ICCL.1994.288372), [Andersen 1993, ACM DOI](https://dl.acm.org/doi/10.1145/154630.154636)

- The paper does not support the online half of the sentence at all: `whereas online approaches interleave specialization with evaluation and therefore need other mechanisms to control when specialization should happen`. That comparison belongs to a source that actually discusses online PE or staging control, not this paper.
  Source: [IEEE DOI](https://doi.org/10.1109/ICCL.1994.288372)

- `That terminology is useful here because it makes the core tradeoff explicit ...` is the thesis author's synthesis, not something established by this paper. Palsberg and Schwartzbach study analysis precision, not specialization profitability or design tradeoffs for runtime systems.
  Source: [IEEE DOI](https://doi.org/10.1109/ICCL.1994.288372)

## Suggested Improvements

- Keep `@palsberg_binding-time_1994` only on the narrow claim that classic PE/BTA literature distinguishes `static` from `dynamic` and represents that distinction with two-level annotations.

- Let `@andersen_binding-time_1993` carry the stronger claim about an offline workflow that computes binding times before specialization.

- Let `@leisa_anydsl_2018` or a PE survey/textbook carry the online-vs-offline contrast.

- Rephrase `static values` to `program parts`, `expressions`, or `entities` if this citation remains attached, because that matches the paper's actual framing better.

## Best Replacement Wording

In classic partial-evaluation terminology, binding-time analysis distinguishes `static` compile-time parts from `dynamic` run-time parts of a program and records that distinction in a two-level representation @palsberg_binding-time_1994. For the stronger claim that an offline partial evaluator computes this distinction before specialization and then specializes the resulting annotated program, `@andersen_binding-time_1993` is the better citation.
