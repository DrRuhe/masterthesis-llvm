# Verdict

Partially faithful. The thesis sentence at [thesis.typ:265](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/thesis.typ:265) is directionally right that PEAD is a domain-specific use of partial evaluation outside language-runtime JITs. But it is looser and broader than the paper. Eriksson et al. present partial evaluation of automatic differentiation for DAE compilation, specifically to produce mixtures of symbolic and AD code and to let the user trade code size against specialization/performance in Jacobian generation. They do not present PEAD as a general-purpose specialization architecture. Sources: [DOI](https://doi.org/10.1145/3624007.3624054), [ACM landing page](https://dl.acm.org/doi/10.1145/3624007.3624054).

# Supported

- "Outside language runtimes" is supported in substance. The paper is about DAE solvers and equation-based modeling-language compilation, not VM/interpreter runtime compilation. Source: [DOI](https://doi.org/10.1145/3624007.3624054).
- The thesis is justified in using this citation to show that partial evaluation is still being used in a modern systems/compiler setting. PEAD applies PE to AD in a current domain-specific compiler pipeline. Source: [DOI](https://doi.org/10.1145/3624007.3624054).
- The tradeoff claim is supported, but only if phrased carefully. The paper explicitly says specialization is parameterized to enable a tradeoff between code size and performance, and concludes that the Jacobian-specialization scheme lets the user control the tradeoff between code size and specialized code. Sources: [DOI](https://doi.org/10.1145/3624007.3624054), [ACM landing page](https://dl.acm.org/doi/10.1145/3624007.3624054).

# Unfounded or Overstated

- "where it can mediate between code size and specialization quality" is too vague and not the paper's framing. The paper is more specific: the tradeoff is between code size and performance / degree of specialization in PEAD-generated Jacobian code, not a broad notion of "specialization quality." Source: [DOI](https://doi.org/10.1145/3624007.3624054).
- The sentence risks making PEAD sound like evidence about partial evaluation as a general compiler architecture. This citation is narrower: it is about partial evaluation of automatic differentiation for DAEs, with structural-analysis-guided selective specialization. Source: [DOI](https://doi.org/10.1145/3624007.3624054).
- If the thesis wants support for a broad contrast against "general-purpose runtime compiler architecture," this citation is weak. The paper does not argue against runtime PE architectures; it just solves a different problem in a DSL/compiler setting.
- "recent work such as PEAD further shows that partial evaluation remains useful as a specialization technique" is acceptable as synthesis, but it is the thesis author's inference, not a claim made in those terms by the paper.

# Suggested Improvements

- Narrow the sentence so it says what PEAD actually does: partial evaluation of automatic differentiation in DAE compilation.
- Replace "specialization quality" with "performance" or "degree of specialization," which matches the paper more closely.
- Avoid using this citation to support any broader claim about runtime-compilation architecture. Keep it as evidence for domain-specific, compile-time specialization within a modeling-language / solver pipeline.

# Best Replacement Wording

Outside language runtimes, recent work such as PEAD shows that partial evaluation also remains useful in domain-specific compiler pipelines. Eriksson et al. apply partial evaluation to automatic differentiation for differential-algebraic-equation solving, using it to generate residual and Jacobian code that can trade code size against the degree of specialization and runtime performance, rather than to build a general-purpose runtime compilation architecture @eriksson_partial_2023.
