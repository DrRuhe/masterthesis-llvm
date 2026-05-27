# IPSCCP Configuration Variations for the Specializing JIT

## Cost Model Quick Reference

All parameters are `cl::opt` globals in `FunctionSpecialization.cpp` — not exposed through any runtime API.

| Parameter | Default | Effect |
|-----------|---------|--------|
| `MinFunctionSize` | 500 | Skip entire function if < 500 instructions (pre-gate, checked before profitability) |
| `MaxClones` | 3 | Hard global budget: at most 3 clones total across the entire module |
| `MaxCodeSizeGrowth` | 3× | Reject if total function growth exceeds 3× original |
| `MinCodeSizeSavings` | 20% | Reject if code-size savings < 20% of function size |
| `MinLatencySavings` | 20% | Reject if latency savings < 20% of function size |
| `MinInliningBonus` | 300% | Accept early (bypass code/latency checks) if inlining bonus > 300% of func size |
| `FuncSpecMaxIters` | 10 | Max iterations of IPSCCP's internal specialization loop |
| `ForceSpecialization` | false | Bypass ALL profitability checks; still gated by MinFunctionSize and MaxClones |
| `SpecializeOnAddress` | false | Allow specialization on addresses of global values |
| `SpecializeLiteralConstant` | true | Allow specialization on integer/float/struct constants (not just pointers) |

**Decision tree per specialization candidate:**
1. `MinFunctionSize < 500` → skip function entirely (hard gate, before any scoring)
2. `ForceSpecialization=true` → accept unconditionally
3. `InliningBonus > 300% FuncSize` → accept (early exit)
4. `CodeSizeSavings < 20% FuncSize` → reject
5. `LatencySavings < 20% FuncSize` → reject
6. `(TotalGrowth + SpecSize) / FuncSize > 3` → reject
7. Otherwise → accept

**The fundamental problem for our JIT use case:** our target functions are small (often <500 instructions after GlobalDCE), and IPSCCP's profitability model assumes large, expensive-to-call functions. The MinFunctionSize=500 gate alone disqualifies most JIT targets before any scoring.

---

## Variation 1: Status Quo — `IPSCCP(AllowFuncSpec=false)` inside fixpoint

**What it does:** Interprocedural constant propagation across function boundaries using the SCCP solver. Replaces arguments with proven-constant values, eliminates dead branches and unreachable code. Never clones.

**What it doesn't do:** No function specialization. Our `ConstantArgFunctionSpecializationPass` handles cloning.

**Interaction with our passes:**
- Runs after our custom pass creates clones → propagates baked-in constants across callee boundaries
- Sees constants introduced by `InvariantLoadToConstantPass` and `DevirtualizeConstantVtableCallsPass` from the previous iteration
- Each outer fixpoint iteration gives IPSCCP fresh material

**Verdict:** Correct and necessary. The two passes (our custom + IPSCCP-no-spec) are complementary, not redundant.

---

## Variation 2: `IPSCCP(AllowFuncSpec=true)` with Defaults

**What happens:** The `FunctionSpecializer` is activated. It scans all functions for specialization candidates.

**In practice on our JIT modules:**
- `MinFunctionSize=500` immediately blocks most targets — after early GlobalDCE, kernel functions typically have 20–200 instructions
- `MaxClones=3` is a module-wide budget; with 3 clones shared across all functions, specialization is extremely scarce
- `MinInliningBonus=300%` is rarely met unless the constant argument is a function pointer enabling direct call conversion
- **Net effect:** Almost no specializations happen. Our custom `ConstantArgFunctionSpecializationPass` does far more work

**Verdict:** Worse than status quo for our use case. Defaults are calibrated for AOT compilation of large production codebases, not JIT specialization of small kernel functions.

---

## Variation 3: `IPSCCP(AllowFuncSpec=true, ForceSpecialization=true)`

**What changes:** Profitability checks are bypassed. Any function with a constant argument is specialized.

**Still blocked by:**
- `MinFunctionSize=500` — the function size gate is checked before `IsProfitable()`, so `ForceSpecialization` does not bypass it
- `MaxClones=3` — module-wide budget still applies

**In practice:**
- Functions with ≥500 instructions are fully specialized on any constant argument
- Functions <500 instructions (most of ours) are still silently skipped
- The 3-clone budget is consumed quickly; once exhausted, no further specializations occur in the entire module

**Verdict:** Partially useful for large target functions (e.g. a full kernel loop body), but fails on small functions which are the common case post-prune.

---

## Variation 4: Aggressive JIT-Tuned IPSCCP — `AllowFuncSpec=true, ForceSpecialization=true, MinFunctionSize=1, MaxClones=∞`

**What this would do:** IPSCCP becomes equivalent in power to our `ConstantArgFunctionSpecializationPass` but with the SCCP solver's lattice inference, inlining bonus calculation, and iterative refinement on top.

**Key advantage over our custom pass:**
- IPSCCP can specialize on values the solver has *inferred* to be constant (not just literal `Constant*` in the IR) — enabling specialization on values that become constant through control flow
- Built-in iterative refinement: new specializations expose new constants for the next round
- Can specialize on function pointer arguments (inlining bonus path)

**Key problem: no runtime API exists for these parameters.** All are `cl::opt` globals in an anonymous namespace in `FunctionSpecialization.cpp`. Options for reaching them:
- `llvm::cl::ParseCommandLineOptions()` before JIT init (hacky, global, affects all passes)
- Modify LLVM to accept an `FunctionSpecializationOptions` struct (mirroring how `InlineParams` works for the inliner — the right long-term approach)
- Set `cl::opt` values directly at startup via `llvm::StringMap<cl::Option*>` lookup through `cl::getRegisteredOptions()`

**ValueHandle hazard:** With `AllowFuncSpec=true`, `FunctionSpecializer::run()` calls `Solver.solveWhileResolvedUndefsIn()` which creates many `AssumptionCache` WeakVH handles. This is the same hazard as plain IPSCCP but worse (more solver iterations). The `LargeModule` guard must remain, or we need fresh-context-per-iteration.

**Verdict:** The right long-term direction. Requires a small LLVM modification to expose an options struct. When available, this could replace `ConstantArgFunctionSpecializationPass` entirely while being more powerful.

---

## Variation 5: Move IPSCCP Outside the Fixpoint Loop

**Proposed structure:**
```
[before fixpoint]
  IPSCCP(AllowFuncSpec=true, FuncSpecMaxIters=10) → specialize once, fully converged

[fixpoint loop]
  DevirtualizeConstantVtableCallsPass
  InvariantLoadToConstantPass
  ConstantArgAlwaysInlinePass + AlwaysInlinerPass (P0) / ModuleInlinerPass (P1)
  GlobalOpt + WholeProgramDevirt
  IPSCCP(AllowFuncSpec=false, single call) → propagate new constants from each round
```

**Argument for:** IPSCCP already iterates internally (up to FuncSpecMaxIters=10). Running it in our outer fixpoint loop is redundant — on each outer iteration, IPSCCP re-runs from scratch and re-creates all ValueHandle entries from the previous iteration's values.

**Argument against:** Each outer fixpoint iteration produces *new* constants that IPSCCP hasn't seen:
- `DevirtualizeConstantVtableCallsPass` turns indirect calls into direct calls → new specialization opportunities
- `InvariantLoadToConstantPass` materializes invariant loads → new constant arguments
- `AlwaysInlinerPass` inlines functions → new constants visible after inlining

If IPSCCP runs only once before the loop, it misses all of these. The constants it sees are only those present in the original IR.

**Hybrid compromise:** Run IPSCCP(AllowFuncSpec=true, FuncSpecMaxIters=1) inside the outer loop. The outer loop drives convergence; IPSCCP does one pass of specialization per outer iteration instead of 10. This:
- Reduces ValueHandle accumulation (fewer internal SCCP solver iterations)
- Lets our outer loop handle convergence detection
- Still sees new constants from devirt/inlining each outer round

**Verdict:** Don't move IPSCCP fully outside. Use `FuncSpecMaxIters=1` inside the loop to eliminate internal redundant re-running. Outer fixpoint drives convergence.

---

## Variation 6: IPSCCP with Invariant-Load Awareness (SCCP Extension)

**The idea:** Extend IPSCCP's `visitLoadInst` in `SCCPSolver` to handle loads marked with `!invariant.load` metadata. When the solver encounters such a load whose pointer resolves to a compile-time constant address, it reads the host memory value and marks the load's lattice cell as a constant — without emitting any IR change (the solver just tracks it as a known constant for propagation purposes).

**Current state:** `getCandidateConstant()` in `FunctionSpecialization.cpp` already accepts values that the SCCP solver has proven constant (`Solver.getConstantOrNull(V)`). If the solver treats invariant loads as constants, specialization would natively propagate through lambda captures and read-only struct fields.

**What this enables:**
- `StaticMutabilityAnalysisPass` becomes optional — IPSCCP can infer the invariant.load metadata itself when it finds `!invariant.load` on a load instruction
- `InvariantLoadToConstantPass` becomes redundant for the purposes of constant propagation through the solver — IPSCCP natively knows the value
- Function specialization can specialize on lambda-captured values without needing our pre-passes to materialize them first

**Implementation sketch:**
In `SCCPSolver::visitLoadInst()` (in `llvm/lib/Transforms/Scalar/SCCP.cpp`), add:
```cpp
if (I.hasMetadata(LLVMContext::MD_invariant_load)) {
    if (Constant *C = tryResolveInvariantLoad(&I)) {
        markConstant(ValueState[&I], &I, C);
        return;
    }
}
```
Where `tryResolveInvariantLoad` contains the resolution logic from `InvariantLoadToConstantPass` (pointer chain resolution, `mincore` check, host memory read).

**Risks:**
- The SCCP solver is a core LLVM component; modifying it requires care to not break the lattice invariants
- `mincore`/host-memory reads in the solver make IPSCCP non-pure (side-effects via syscall) — this is already the case for our custom pass but unusual for SCCP
- The solver runs speculatively on unreachable blocks; a bad pointer dereference on speculative code would be a bug. Need to gate on block executability (already tracked by the solver)

**Verdict:** High-value, medium-difficulty LLVM extension. The resolver logic already exists in `InvariantLoadToConstantPass.cpp` and can be factored into a shared utility. This is the right direction if we want to consolidate our JIT-specific passes into the IPSCCP solver.

---

## Interaction Analysis: How Variations Compose

```
State entering fixpoint:
  Module with runtime constants as function arguments (literal Constant* or via specialization clones)

P0 fixpoint iteration flow (current):
  IPSCCP(AllowFuncSpec=false) → propagate
  DevirtualizeConstantVtable → turn virtual calls direct
  GlobalDCE
  StaticMutability + InvariantLoadToConstant → materialize invariant loads
  ConstantArgAlwaysInline + AlwaysInliner → inline constant-arg sites
  GlobalOpt + WholeProgramDevirt

Key dependency chains:
  (A) DevirtualizeConstantVtable → AlwaysInliner → new constants visible → IPSCCP next round
  (B) InvariantLoadToConstant → new constants in args → IPSCCP next round
  (C) AlwaysInliner inline → exposes callee constants → DevirtualizeConstantVtable next round
  (D) IPSCCP propagates → branch elimination → devirt targets become unambiguous
```

If Variation 6 (invariant-load IPSCCP) is implemented, chain (B) collapses into IPSCCP itself:
```
  IPSCCP(AllowFuncSpec=true/false, InvariantLoadAware) → propagate + materialize invariant loads
  DevirtualizeConstantVtable → turn virtual calls direct  
  GlobalDCE
  ConstantArgAlwaysInline / ModuleInliner → inline
  GlobalOpt + WholeProgramDevirt
```
`StaticMutabilityAnalysisPass` and `InvariantLoadToConstantPass` become pre-passes that are optional (they help the inliner directly; IPSCCP uses them via solver lattice).

---

## Recommendation

### Immediate (no LLVM changes)

Set `FuncSpecMaxIters=1` when calling IPSCCP inside our outer fixpoint loop. The outer loop already tracks convergence via instruction count; IPSCCP's internal 10-iteration loop is redundant and multiplies ValueHandle accumulation by 10×. This is achievable today via `cl::getRegisteredOptions()` to look up and set the opt at startup:

```cpp
// At JIT init time (one-time):
auto &OptMap = llvm::cl::getRegisteredOptions();
if (auto It = OptMap.find("funcspec-max-iters"); It != OptMap.end())
    It->second->setNumAdditionalVals(0);  // or use Option's setDefault
```

Or more cleanly: contribute a patch to expose `FuncSpecMaxIters` as a field in `IPSCCPOptions`.

Keep the current `ConstantArgFunctionSpecializationPass` — it is faster, has no cost-model overhead, and avoids the MinFunctionSize=500 gate.

### Medium Term (small LLVM patch)

Add a `FunctionSpecializationOptions` struct to `FunctionSpecialization.h` (analogous to `InlineParams`) and thread it through `runIPSCCP()`. Expose at minimum:
- `MinFunctionSize` (set to 1 for JIT)
- `MaxClones` (set to unlimited)
- `FuncSpecMaxIters` (set to 1 — outer loop drives convergence)
- `ForceSpecialization` (optional: bypass profitability checks for JIT context)

With this, switch Pipeline 0 and Pipeline 1 to use `IPSCCP(AllowFuncSpec=true)` with JIT-tuned options inside the fixpoint loop. `ConstantArgFunctionSpecializationPass` can be retired.

### Long Term (moderate LLVM patch)

Extend `SCCPSolver::visitLoadInst` to resolve `!invariant.load` loads by reading host memory (reuse the resolution logic from `InvariantLoadToConstantPass` as a shared utility). This makes IPSCCP natively aware of lambda captures and read-only struct fields, propagating them through the lattice and enabling specialization on them directly.

With this in place:
- `InvariantLoadToConstantPass` becomes a standalone "IR rewriter" pass (still useful to replace the load instructions with constants for the inliner, since IPSCCP only tracks lattice values internally)
- `StaticMutabilityAnalysisPass` can be retired (IPSCCP infers invariance from `!invariant.load` metadata that the compiler already emits for genuinely read-only fields)
- The pipeline simplifies significantly: IPSCCP(AllowFuncSpec=true, InvariantLoadAware) + DevirtualizeConstantVtable + Inliner covers most of what the current multi-pass fixpoint does

### Recommended pipeline target state (medium term)

```
[P0 fixpoint loop]
  IPSCCP(AllowFuncSpec=true, MinFunctionSize=1, MaxClones=∞, FuncSpecMaxIters=1)
  DevirtualizeConstantVtableCallsPass
  GlobalDCE + ReversePostOrderFunctionAttrs
  InvariantLoadToConstantPass (still useful: replaces loads in IR so inliner sees constants)
  AlwaysInlinerPass + GlobalOpt + WholeProgramDevirt
  [FPMs as today]

[P1 fixpoint loop]
  IPSCCP(AllowFuncSpec=true, MinFunctionSize=1, MaxClones=∞, FuncSpecMaxIters=1)
  DevirtualizeConstantVtableCallsPass (currently absent — add this)
  InvariantLoadToConstantPass
  ModuleInlinerPass(P1InlineThreshold)
  GlobalOpt + GlobalDCE
```

`ConstantArgFunctionSpecializationPass` is retired; IPSCCP with JIT-tuned options covers its role.
