## Verdict

Partially faithful. Andersen (1993) is a good citation for the classic *static vs. dynamic* binding-time distinction and for an *offline* workflow where a binding-time analysis produces a two-level program that can be fed to an offline partial evaluator. It is not a strong citation for the broader thesis framing that partial evaluation "provides the clearest vocabulary," nor for the comparison to *online* partial evaluation. Those claims are broader than this paper's scope and should be carried by a survey/textbook or an online-PE source instead.

Primary source: [ACM DOI page](https://dl.acm.org/doi/10.1145/154630.154636)

## Supported

- The thesis claim that classic PE uses a binding-time distinction between values known early and values left for the residual program is supported. Andersen defines binding-time analysis as classifying program parts into `static (compile-time)` and `dynamic (run-time)`.
- The thesis claim that offline partial evaluators use a prior binding-time analysis is supported. Andersen presents an `automatic binding-time analysis`, constructs a `two-level` annotated C program, and states that such a program can be input to `an offline partial evaluator`.
- The paper is especially relevant if the thesis wants to emphasize that making binding times explicit is important for specializing imperative programs with pointers and dynamic allocation, which is this paper's actual contribution.

## Unfounded or Overstated

- `PE provides the clearest vocabulary for describing specialization from partially known inputs.` This is not established by Andersen (1993). The paper assumes PE/BTA vocabulary and extends it to C with pointers; it does not argue that this vocabulary is the clearest among competing frameworks.
- `Since the Futamura line of work ...` Andersen (1993) is not the right support for the historical Futamura framing. That belongs with Futamura or a PE survey/history source.
- `Offline partial evaluators determine this distinction through a binding-time analysis before specialization, whereas online approaches interleave specialization with evaluation ...` The first half is supported; the online/offline contrast is not. Andersen discusses offline BTA, not online PE mechanisms.
- `That terminology is useful here because it makes the core tradeoff explicit: profitable specialization usually depends on knowing, inferring, or declaring which inputs are available early enough to be treated as static.` This is a reasonable thesis inference, but it is not a claim Andersen directly establishes. His paper is about correctness/feasibility of binding-time separation for C, not a general profitability argument.

## Suggested Improvements

- Keep Andersen (1993) only on the sentence fragment that defines `static` versus `dynamic` values and the offline BTA workflow.
- Move the historical Futamura claim to `@futamura_partial_1999` or a PE survey/textbook.
- Move the online-vs-offline contrast to a source that actually discusses both models, such as `@leisa_anydsl_2018` if that source makes the distinction explicitly, or a standard PE survey/textbook if not.
- If the thesis wants to justify why binding-time terminology is analytically useful for CRS, present that as the thesis's own framing rather than as something Andersen proved.

## Best Replacement Wording

Partial evaluation provides standard terminology for specialization from partially known inputs. In the classic offline formulation, a binding-time analysis classifies program entities as `static` or `dynamic`, yielding a two-level program in which static parts may be evaluated earlier while dynamic parts remain in the residual program @andersen_binding-time_1993 @palsberg_binding-time_1994. This thesis uses that vocabulary only as a descriptive lens for which runtime inputs CRS can treat as effectively constant; the broader historical Futamura framing and the contrast to online specialization should be supported by separate citations.
