## Verdict

Partially faithful, but the thesis currently uses this citation too broadly. Kistler supports claims about **runtime, profile-guided reoptimization of hot code in modular/extensible systems**. It does **not** cleanly support the broader framing about decades of work across many communities, and it is a weak citation for the specific claim about **classic dynamic-language systems choosing methods to compile at runtime**.

Sources:
- Primary source PDF permalink: https://escholarship.org/uc/item/8g19p0ns
- DOI / published version metadata: https://doi.org/10.1007/3-540-62599-2_30

## Supported

- The paper explicitly argues for **delaying optimization until runtime** rather than compile time, especially to recover intermodular optimization opportunities in modular/extensible systems. Source: https://escholarship.org/uc/item/8g19p0ns
- It describes an **adaptive profiler** that gathers runtime behavior such as call frequencies, call sequences, call durations, parameter information, and dynamic types. Source: https://escholarship.org/uc/item/8g19p0ns
- It describes a **dynamic optimizer** that periodically recompiles code consuming most execution time in the background, using profiler data. This does support a narrow “runtime optimization of hot code” claim. Source: https://escholarship.org/uc/item/8g19p0ns
- It discusses runtime reoptimization techniques such as **intermodular inlining, procedure cloning, and profile-guided register allocation**. Source: https://escholarship.org/uc/item/8g19p0ns

## Unfounded or Overstated

- The citation is too weak for the sentence claiming that research spans “runtime code generation, partial evaluation, and just-in-time compilation” across “multiple communities.” Kistler is one paper about runtime optimization in modular systems; it is not a broad survey and does not cover partial evaluation in any meaningful way. Source: https://escholarship.org/uc/item/8g19p0ns
- The citation is not a good fit for “classic dynamic-language systems already used runtime behaviour to decide which methods to compile.” Kistler’s system is not presented as a classic dynamic-language VM, and the paper emphasizes recompiling **modules/procedures** in a modular operating-system setting, not method-based JIT compilation in the usual VM sense. Source: https://escholarship.org/uc/item/8g19p0ns
- The same sentence also mentions “meta-tracing.” Kistler does not support that part at all; the citation should not be read as evidence for tracing-style JITs or meta-tracing. Source: https://escholarship.org/uc/item/8g19p0ns
- Even where the thesis says JIT compilation uses runtime information to optimize only hot parts, Kistler is better read as **runtime reoptimization / dynamic optimization** than as a canonical source for the general definition of JIT compilation. Source: https://escholarship.org/uc/item/8g19p0ns

## Suggested Improvements

- Keep Kistler only where the thesis wants evidence for **profile-guided runtime reoptimization of hot code** or for recovering optimization opportunities in **modular/extensible systems**.
- Remove Kistler from the broad “many communities / partial evaluation / JIT” scene-setting sentence unless that sentence is rewritten to be specifically about **runtime optimization**.
- Remove Kistler from the “classic dynamic-language systems decided which methods to compile” sentence. A VM/JIT-specific citation is better there; `@deutsch_efficient_1984` already fits that role more directly.
- If the sentence is meant to cover both hot-method JITs and broader runtime optimization, split it into two citations: one for **dynamic-language JIT history**, one for **profile-guided runtime reoptimization in modular systems**.

## Best Replacement Wording

Kistler describes an early **profile-guided runtime reoptimization** architecture for modular, extensible systems: a lightweight loader first generates native code without heavy optimization, an adaptive profiler records runtime behavior, and a background optimizer recompiles hot modules using that profile information. This makes it a good citation for broader runtime optimization, but not for classic dynamic-language hot-method JITs or meta-tracing. [Primary source](https://escholarship.org/uc/item/8g19p0ns)
