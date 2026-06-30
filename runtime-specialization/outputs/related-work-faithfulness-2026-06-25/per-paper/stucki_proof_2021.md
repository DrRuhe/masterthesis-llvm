# Verdict

Partially faithful. This citation solidly supports the thesis's description of Scala 3's staged programming model: `Expr[T]`, quotes `'{...}`, splices `${...}`, `inline`-based macros, and runtime `run`. It does **not** cleanly support the broader claim that Scala 3 necessarily forces staging markers to "propagate through the staged region" or that stage boundaries always remain user-visible in the same way for end users. The paper explicitly presents `inline` wrappers as a way to hide `Expr`-typed interfaces from macro users.

Sources: [primary paper PDF](</home/Jakob.Gerhardt/Zotero/storage/BPJ9D5R9/Stucki et al. - 2021 - Proof of Multi-Stage Programming withGenerative and Analytical Macros.pdf>), [Scala 3 quoted-code docs](https://docs.scala-lang.org/scala3/guides/macros/quotes.html), [Scala 3 runtime staging docs](https://docs.scala-lang.org/scala3/reference/metaprogramming/staging.html)

# Supported

- The thesis is on solid ground when it says Scala 3 uses quoted code `'{...}`, splices `${...}`, and `Expr[T]` for staged code. The paper's Section 2.1 introduces exactly that API.
- The claim that Scala 3 uses `inline` methods as the entry point for macros is supported. The paper states that top-level splices are restricted to `inline` methods in practice.
- The claim that Scala 3 supports runtime multi-stage programming via `run` is supported. The paper explicitly says "`run` compiles and executes quoted code at runtime," and the official runtime staging docs expose `scala.quoted.staging.run`.
- The high-level comparison "Scala 3 is an MSP-style system with explicit quotes/splices and stage-consistency rules" is supported.

# Unfounded or Overstated

- "The stage boundaries remain part of the user-facing program representation" is too broad as written. It is true for macro implementers, but the paper explicitly says `inline` wrappers can hide `Expr` types and "encapsulat[e] all aspects of metaprogramming from the end users." That weakens the thesis's current framing.
- The table row and follow-up prose claiming that in Scala 3 "staged code is written in `Expr[T]`, quotes, and splices" and that these markers "tend to propagate through the staged region" is only partially supported. The paper shows that macro implementations manipulate `Expr`, but it also emphasizes that user-facing macro signatures need not mention `Expr`.
- "Existing MSP systems require staging markers inside the staged computation itself" is too sweeping if this citation is meant to support it for Scala 3 specifically. Scala 3 macro callers can invoke an `inline` macro through an ordinary-looking function call; the explicit markers are unavoidable in the macro implementation, not necessarily at every use site.
- If the thesis wants to support a general sociotechnical claim about annotation burden or signature pollution across staged systems, this paper is not the strongest sole support. It is better evidence for Scala 3's mechanism and safety model than for a broad burden comparison.

# Suggested Improvements

- Narrow the Scala 3 claim from "user-facing stage boundaries remain explicit" to "macro implementations explicitly manipulate `Expr`, quotes, and splices; Scala 3 can hide some of this behind `inline` macro entry points."
- Keep `@stucki_proof_2021` for the concrete Scala 3 mechanism claims: `Expr[T]`, quotes, splices, `inline`, runtime `run`, and stage-consistency restrictions.
- Do not use this citation as primary evidence for the stronger cross-system claim that staging annotations generally propagate into surrounding signatures. For Scala 3, this paper cuts both ways because it explicitly advertises encapsulation of `Expr` from end users.
- If the thesis wants to keep the runtime `run` claim, citing the official Scala 3 runtime staging reference alongside the paper would make the API-level statement cleaner and less inferential.

# Best Replacement Wording

Scala 3 provides a typed staging interface based on quoted code `'{...}`, splices `${...}`, and `Expr[T]`, with `inline` methods as the practical entry point for macros and `run` for runtime multi-stage execution [Stucki et al. 2021; Scala 3 staging reference]. In contrast to our approach, the staged logic in Scala 3 macro implementations is expressed explicitly in terms of quotes, splices, and `Expr` values. However, the paper also notes that `inline` wrappers can hide `Expr`-typed interfaces from macro users, so the main difference is better framed as where staging annotations must appear: in Scala 3's macro implementation, rather than necessarily in every user-facing call site.
