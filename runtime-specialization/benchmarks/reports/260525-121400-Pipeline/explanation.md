# Pipeline Walkthrough: P0 (Inlining) vs P1 (FuncSpec)

## Overview

The JIT specializer has two distinct transform pipelines selected at specialization time:

- **Pipeline 0** (`JITPipelineInlining.cpp`): always-inline driven, full module inlining into the wrapper. Can produce arbitrarily large output. Used when the inlining budget is unconstrained.
- **Pipeline 1** (`JITPipelineFuncSpec.cpp`): budget-aware function cloning + cost-based inlining. Limits module growth to `P1MaxModuleGrowth × starting size` (default 2×). Used for large or expensive-to-inline modules.

---

## Pipeline 0 (Inlining) — Step-by-Step

### Early Prune *(if `EnableEarlyPrune`)*
- `GlobalDCEPass` — strips unreachable functions before any analysis (61→6 fn example)

### Initial Phase
1. FPM: `SROA` → `EarlyCSE` → `InstCombine` — normalize allocas/loads/stores
2. `ReversePostOrderFunctionAttrsPass` — infer `readonly`/`readnone`
3. FPM: `StaticMutabilityAnalysisPass` — tags read-only fields with `!invariant.load` metadata
4. FPM: `InvariantLoadToConstantPass` — reads host memory at tagged load addresses, replaces with `Constant*`
5. `ConstantArgAlwaysInlinePass` — marks any call site with ≥1 constant arg as `alwaysinline` *(skipped if LargeModule)*
6. `AlwaysInlinerPass` — performs forced inlining of all `alwaysinline` sites *(skipped if LargeModule)*

### Fixpoint Loop *(exits when instruction count converges)*
1. `IPSCCPPass(AllowFuncSpec=false)` — interprocedural sparse conditional constant propagation *(skipped if LargeModule)*
2. `DevirtualizeConstantVtableCallsPass` — resolves vtable pointer chains to direct calls via dlsym + host-memory dereferencing
3. `GlobalDCEPass` — prune dead code after devirt/IPSCCP
4. `ReversePostOrderFunctionAttrsPass`
5. FPM: `StaticMutabilityAnalysisPass` → `InvariantLoadToConstantPass` — pick up new invariant loads exposed by inlining
6. `ConstantArgAlwaysInlinePass` → `AlwaysInlinerPass` — force-inline the newly-constant-arg sites
7. `GlobalOptPass` + `WholeProgramDevirtPass`
8. Pre-inlining FPM: `EarlyCSE` + `SROA` + `JumpThreading` + `SimplifyCFG` + `InstCombine` *(skipped if LargeModule)*
9. Post-inlining FPM: `StaticMutability` + `InvariantLoadToConstant` + `InstCombine` + `SROA` + `EarlyCSE` + `JumpThreading` + `SimplifyCFG` + `LoopRotate` + `LICM` + `LoopUnroll` + `InstCombine` + `SimplifyCFG` + `InstSimplify` *(all skipped if LargeModule)*

### Postfix
- `GlobalDCEPass`

### Final *(if `EnableO3Final`)*
- `buildPerModuleDefaultPipeline(O3)`

---

## Pipeline 1 (FuncSpec) — Step-by-Step

### Linkage Scrub *(one-time, before fixpoint)*
- BFS from the wrapper's callee (target function) → collect all transitive callees
- Convert `AvailableExternalLinkage` → `InternalLinkage` so the JIT compiles those bodies
- Strip `AlwaysInline` from all functions and all call sites (P1 uses cost-based inliner, not forced inlining)

### Fixpoint Loop *(exits on instruction-count convergence or when module exceeds `P1MaxModuleGrowth × starting size`)*
1. `ConstantArgFunctionSpecializationPass` — clones the target function with constant arguments baked into the clone body
2. Folding FPM *(skipped if LargeModule)*: `SROA` + `EarlyCSE` + `StaticMutabilityAnalysis` + `InvariantLoadToConstantPass` + `InstCombine` + `SimplifyCFG` + `LoopRotate` + `LoopUnroll(setPartial=false)` + `GVN` + `InstCombine` + `SimplifyCFG` + `DCE`
   — folds baked-in constants so the cost model sees a small clone body
3. `IPSCCPPass(AllowFuncSpec=false)` *(skipped if LargeModule)*
4. `ReversePostOrderFunctionAttrsPass` + `GlobalOptPass` + `GlobalDCEPass`
5. `DeadArgumentEliminationPass`
6. `ModuleInlinerPass(P1InlineThreshold)` — **cost-based** inlining (not forced)
7. `ReversePostOrderFunctionAttrsPass`
8. Post-cleanup FPM *(skipped if LargeModule)*: `SROA` + `InstCombine` + `SimplifyCFG`
9. `GlobalDCEPass`
10. Zero-arg wrapper special case: if `sizeof(target) ≤ P1InlineThreshold` and wrapper has no args, directly calls `llvm::InlineFunction` bypassing the cost model

### Final Phase
- FPM: `InstCombine` + `SimplifyCFG` *(skipped if LargeModule)*
- `ReversePostOrderFunctionAttrsPass` + `GlobalDCEPass`

---

## Pass Presence Matrix

| Pass | P0 | P1 | Notes |
|------|----|----|-------|
| Early `GlobalDCE` (prune) | ✓ | — | P0 only |
| `StaticMutabilityAnalysisPass` | ✓ initial + fixpoint | ✓ fixpoint, `!LargeModule` | P1 gated |
| `InvariantLoadToConstantPass` | ✓ initial + fixpoint ×2 | ✓ fixpoint, **`!LargeModule`** | P1 gated |
| `DevirtualizeConstantVtableCallsPass` | ✓ fixpoint | **absent** | **P1 missing entirely** |
| `WholeProgramDevirtPass` | ✓ fixpoint | **absent** | **P1 missing** |
| `ConstantArgAlwaysInlinePass` | ✓ initial + fixpoint | **absent** | P1 uses cost model |
| `AlwaysInlinerPass` | ✓ initial + fixpoint | **absent** | P1 uses cost model |
| `IPSCCPPass` | ✓ `!LargeModule` | ✓ `!LargeModule` | Both gated |
| `GlobalOptPass` | ✓ fixpoint | ✓ fixpoint | |
| `ConstantArgFunctionSpecializationPass` | — | ✓ fixpoint | P1 only |
| `ModuleInlinerPass` (cost-based) | — | ✓ fixpoint | P1 only |

---

## Restrictions and Limits Per Pipeline

### Pipeline 0 Restrictions
1. **Module explosion**: always-inline has no growth cap. SQLite grows 234K → 3M+ instructions.
2. **LargeModule disables all FPM passes and IPSCCP**: on large modules, no function-level optimization runs. Only `DevirtualizeConstantVtable`, `GlobalDCE`, `ReversePostOrderFunctionAttrs`, `GlobalOpt`, `WholeProgramDevirt` execute.
3. **LargeModule disables initial `ConstantArgAlwaysInline`**: no constant-arg inlining before the fixpoint, so no constant propagation scaffolding is set up.

### Pipeline 1 Restrictions
1. **No vtable devirtualization**: `DevirtualizeConstantVtableCallsPass` and `WholeProgramDevirtPass` are completely absent. Virtual dispatch is never resolved, so the cost-based inliner cannot inline virtual callees.
2. **`InvariantLoadToConstantPass` gated behind `!LargeModule`**: invariant loads (lambda captures, read-only struct fields) are never materialized as constants for large-module runs, even though `StaticMutabilityAnalysisPass` emits the metadata.
3. **No `ConstantArgAlwaysInlinePass`**: constant propagation from baked-in clone arguments does not cascade transitively through callee call sites. The cost model may not inline callee chains that P0 would always-inline.
4. **Cost-model limits**: `ModuleInlinerPass` uses a threshold (`P1InlineThreshold`). Clones larger than the threshold are not inlined into the wrapper even when fully specialized.
5. **Growth cap**: fixpoint terminates early if `instructions > P1MaxModuleGrowth × StartingInsts`. Can prematurely stop specialization.

---

## Problems Causing Suboptimal Specialization in P1

### Problem 1: No vtable devirtualization → indirect virtual calls never inlined
`DevirtualizeConstantVtableCallsPass` resolves vtable pointer chains to direct calls by reading the vtable at JIT time via `dlsym` + pointer dereference. Without it in P1, virtual dispatch paths are opaque to the cost model. Even when the object pointer is a compile-time constant whose vtable slot can be read directly, P1 never attempts the resolution. The `ModuleInlinerPass` sees an indirect call and cannot inline it.

`WholeProgramDevirtPass` is also absent, blocking whole-program virtual call optimization.

### Problem 2: `InvariantLoadToConstantPass` gated behind `!LargeModule`
`StaticMutabilityAnalysisPass` runs ungated in the P0 fixpoint but is gated in P1. The result is that `!invariant.load` metadata is emitted (the analysis runs) but the pass that consumes it (`InvariantLoadToConstantPass`) is skipped for large modules. Lambda captures and read-only struct fields remain as load instructions. The cost model sees non-constant arguments and inlining is inhibited.

### Problem 3: No `ConstantArgAlwaysInlinePass` → constant propagation doesn't cascade
After `ConstantArgFunctionSpecializationPass` bakes constants into a clone, those constants may produce new constant-arg call sites deeper in the callee graph. P1 has no forced-inlining mechanism for these sites — only the cost model. Deep call chains don't fully specialize; constants only propagate as far as the cost threshold allows.

### Problem 4: IPSCCP gated on both pipelines
`GlobalOptPass` provides weak substitution but cannot replace IPSCCP for interprocedural constant propagation. Large-module P1 runs have no meaningful cross-function constant propagation at all.

---

## Why Is the LargeModule Workaround Needed?

### Root Cause: LLVM ValueHandle Accumulation in Shared LLVMContext

**Issue A — Cross-iteration ValueHandle corruption (debug builds only)**

Passes like IPSCCP, JumpThreading, and LazyValueInfo register `AssertingVH`/`WeakTrackingVH` handles in `LLVMContext::pImpl->ValueHandles` (a `DenseMap`). Across fixpoint iterations, this map grows, is rehashed, and old buckets are freed. Stale `PrevPtr` chain-heads trigger the debug assertion `"No name entry found!"` on the next insert. Release builds use plain pointers with no assertions and are unaffected.

The spec 005 fix already creates a fresh `LLVMContext` per call in debug builds via bitcode round-trip. The `LargeModule` guard is kept as belt-and-suspenders and to address Issue B.

**Issue B — Performance: ValueHandle growth within a single IPSCCP run on very large modules**

On a 47K-function module (DuckDB), IPSCCP creates `AssumptionCache` WeakVH handles for every function. Within one fixpoint pass run, `pImpl->ValueHandles` grows proportional to function count. This causes quadratic-ish slowdown from DenseMap rehashes, affecting both debug and release builds.

**Why a scalable fix is hard**

The `AnalysisManager` invalidation API does not flush `ValueHandles` — those live in `LLVMContext`, not in an analysis. Principled fixes:
1. Fresh context per fixpoint iteration (already done per-call in debug; too expensive in release for large modules)
2. Eager cleanup of analysis handles (not exposed by LLVM's PassManager API)
3. Skip offending passes above a size threshold (current approach)

A truly scalable design would require `AnalysisManager` invalidation to also evict associated `LLVMContext` state — which is not how LLVM's pass infrastructure is designed today.
