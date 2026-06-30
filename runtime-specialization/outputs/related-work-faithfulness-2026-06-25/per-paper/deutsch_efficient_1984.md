# Verdict

Partially faithful, but over-broad in both thesis uses. Deutsch and Schiffman (1984) is a good citation for **early runtime translation of Smalltalk methods**, **method-level compilation/execution**, and **inline caching / method-lookup acceleration**. It is **not** strong support for broad claims about JIT spanning many research communities, nor for general claims about compiling only the program's "hot parts" using profiling such as hot call edges. Primary source: [DOI](https://doi.org/10.1145/800017.800542), [ACM entry](https://dl.acm.org/doi/10.1145/800017.800542). Thesis passages: [thesis.typ](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/thesis.typ:171), [thesis.typ](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/thesis.typ:279).

# Supported

- The paper supports citing an early dynamic-language runtime that performs **runtime translation** from bytecode-like virtual-machine code to native code when a procedure is about to execute.
- It supports **method/procedure-based compilation** as a compilation unit.
- It supports the claim that runtime information is used during execution machinery, especially **receiver class information** for message sends and **inline caches / linked sends** to accelerate method lookup.
- It supports describing the work as an early ancestor of JIT-style execution, although the paper itself speaks of **"dynamic translation"**, not modern adaptive/JIT terminology.

# Unfounded or Overstated

- The related-work opening sentence at [thesis.typ](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/thesis.typ:171) is too broad for this citation. This paper is about a Smalltalk-80 implementation; it does **not** support the wider claim about runtime code generation across programming languages, databases, and high-performance code generation communities.
- The JIT section at [thesis.typ](/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/docs/thesis.typ:279) overstates what this paper shows with "used runtime behaviour to decide which methods to compile." In Deutsch and Schiffman, translation happens when a procedure is about to execute and native code is cached. That is weaker and more specific than a modern **hotness-driven compilation policy**.
- The sentence "JIT compilation uses runtime information to compile and optimize only the hot parts of a program" should **not** rely on this citation. The paper discusses runtime translation and caching, but not a general adaptive-optimization story driven by profiling of hot regions.
- The follow-up sentence about "hot call edges and observed types" is **not supported** by this paper. Receiver-class-based method lookup caches are present, but the paper does not present the broader profile-guided inlining framework implied by that wording.

# Suggested Improvements

- Keep `@deutsch_efficient_1984` only where the thesis wants an **early concrete example** of runtime native-code generation in a dynamic-language VM.
- Remove it from the broad cross-community survey sentence unless the wording is narrowed to "early dynamic-language and VM work".
- In the JIT paragraph, attribute **hotness-driven adaptive optimization**, **profile-guided inlining**, and broader runtime optimization claims to later sources such as `@kistler_dynamic_1997` or other adaptive-optimization surveys, not to Deutsch and Schiffman.
- If you want to keep this citation in the JIT paragraph, use it specifically for **method-level dynamic translation plus inline caches in Smalltalk-80**.

# Best Replacement Wording

Classic dynamic-language systems already performed runtime native-code generation at the method level. For example, Deutsch and Schiffman's Smalltalk-80 implementation dynamically translated virtual-machine code to native code on demand, cached translated methods, and used inline caches to accelerate method lookup @deutsch_efficient_1984. Later work broadened this into adaptive optimization and tracing-based compilation policies driven more explicitly by runtime profiling @kistler_dynamic_1997 @izawa_amalgamating_2020.
