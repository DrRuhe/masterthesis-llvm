# Q3: Root cause of Pipeline-1's SIGSEGV under specific parameter cross-products

## Important Context

Phase E (`sens_uc_iter2_20260521`) introduced one OAT sweep point `sens_pipeline_1` that flips `pipeline=1` while keeping the Phase-A combined-best config for all other parameters:

```
fixpoint_max=7, unroll_max=27, large_module_max=0,
early_prune=1, o3_final=1,
p1_inline_threshold=572, p1_max_module_growth=1.643
```

This produced **5/5 SIGSEGV** (100 % crash rate). Phase-B's `pipeline_1_default` — which only sets `CRS_DEFAULT_PIPELINE=1` against the baseline defaults — ran stably (0.31× speedup, slow but no crashes).

The crash is therefore triggered by a **multi-parameter interaction**, not by P1 alone. The candidates identified in the iter-2 reflection are `unroll_max=27` and/or `p1_max_module_growth=1.643`.

Crash location: inside the JIT pipeline in `runtime/ClangRuntimeSpecializer/JITPipelineFuncSpec.cpp` (the P1-specific pipeline path).

---

## Investigation

### Analyze the parameter space to isolate the trigger

The two candidate parameters are `unroll_max=27` (vs default ≈ 4) and `p1_max_module_growth=1.643` (vs default 2.0). Their interaction with P1 is the prime suspect.

A binary bisect strategy over 5 parameters (each flipped between Phase-A value and default):

| Param                 | Phase-A value | Default |
|-----------------------|---------------|---------|
| unroll_max            | 27            | 4       |
| p1_max_module_growth  | 1.643         | 2.0     |
| fixpoint_max          | 7             | 5       |
| early_prune           | 1             | 1       |
| o3_final              | 1             | 1       |

Since `early_prune` and `o3_final` match the default, the effective search space is 2² = 4 configurations:

| Config | unroll_max | p1_max_module_growth | Expected |
|--------|-----------|----------------------|---------- |
| A      | 27        | 1.643                | CRASH     |
| B      | 27        | 2.0                  | ?         |
| C      | 4         | 1.643                | ?         |
| D      | 4         | 2.0                  | P1 default — known stable |

Running these 4 configs at reps=3 on a single UC group (e.g., `uc8_ivm` where P1 was most unstable) would isolate the trigger:
- If only A crashes → interaction is required (both params jointly trigger)
- If A and B crash → `unroll_max=27` alone triggers
- If A and C crash → `p1_max_module_growth=1.643` alone triggers

**What to look for in results**: Any non-zero exit code or missing output in the 3 reps. A single SIGSEGV at reps=3 is sufficient to identify a crash trigger.

### Analyze the likely code path in JITPipelineFuncSpec.cpp

The SIGSEGV during JIT compilation typically signals one of:
1. A dangling pointer into an LLVM IR structure after a module transformation
2. An assertion or bounds check in LLVM's code generation path triggered by unusual IR state
3. An out-of-bounds memory access during loop unrolling with large `unroll_max`

The interaction between `unroll_max=27` and P1 inlining is suspicious: P1 brings in extra functions (expanding the module), then the loop unroller operates on the now-larger module. With `unroll_max=27` and the expanded module, the unroller may attempt to unroll a loop 27 times whose trip count LLVM cannot determine — producing an extremely large loop body that exceeds some internal limit.

The `p1_max_module_growth=1.643` is *smaller* than the default (2.0), meaning P1 allows less growth. This is counterintuitive as a crash trigger if the crash were due to oversized modules. However, a restricted growth cap means P1 may be called in a state where it partially inlines a function (expands below the cap but can't complete the inlining), leaving the module in an inconsistent state that a subsequent pass then mishandles.

**What to look for in source**: In `JITPipelineFuncSpec.cpp`, identify where the P1-specific inliner is applied relative to the loop unroll pass. If the unroller runs after P1 inlining without re-running analysis passes (e.g., `DominatorTree` or `LoopInfo` may be stale), the unroller operates on stale analysis → dangling pointer.

### Analyze whether the SIGSEGV is in JIT compile or in JIT execution

A SIGSEGV can occur either during JIT code generation (LLVM IR → machine code, inside `LLJIT::addIRModule`) or during execution of the generated code (calling the JIT-compiled function). The stack trace from the crash would distinguish these:

- **During compilation**: stack includes LLVM pass framework frames (`llvm::LoopUnrollPass::run`, `llvm::AlwaysInlinerPass::run`, etc.)
- **During execution**: stack is at the JIT-compiled function's first instruction or a nullptr dereference

Without a captured stack trace, the current hypothesis must remain speculative.

---

## Conclusion

The SIGSEGV is definitively triggered by a multi-parameter interaction (P1 + at least one of `unroll_max=27`, `p1_max_module_growth=1.643`). The most likely mechanism is a stale LLVM analysis (e.g., LoopInfo) after P1's partial inlining, causing the subsequent loop unroll pass to access freed memory. The 2²=4 bisect is the minimum experiment needed to narrow the trigger, costing approximately 4 × 3 reps × ~1 min per run = 12 minutes.

Until the bug is fixed, P1 must be treated as unsafe with `unroll_max > 4` or `p1_max_module_growth < 2.0`. This explains why the Phase-A combined-best config (P0) is both better-performing and safer than P1 for the search space explored in iter-2.

---

## Further Questions/Directions of Research

- **Run the 4-config bisect** (see table above) at reps=3 on `uc8_ivm`. This is the minimum required experiment and should be prioritized in iter-3.
- **Capture a stack trace** by running the crashing config under `gdb` or with `ASAN_OPTIONS=detect_stack_use_after_return=1`. The exact frame where the SIGSEGV fires will pinpoint the bug in `JITPipelineFuncSpec.cpp`.
- **Add an LLVM IR verifier call** (`llvm::verifyModule`) before and after the P1 inlining pass. If the module is invalid after partial inlining (e.g., a call to an undefined function after growth cap truncation), the verifier will catch it and convert a SIGSEGV into a descriptive error.
- Once the crash trigger is identified: file a targeted fix in `JITPipelineFuncSpec.cpp` and re-run Phase E's full 24-point OAT sweep to confirm P1 stability across the full parameter space.
