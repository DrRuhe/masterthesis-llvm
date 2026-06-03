# Pipeline Overview: P0 (Inlining), P1 (FuncSpec), P2 (JitIPSCCP)

**Date**: 2026-06-02  
**Status**: Three production pipelines. Default is P2 (`CRS_DEFAULT_PIPELINE=2`).

---

## Architecture Overview

The JIT specializer selects one of three transform pipelines at specialization time
via `Options::OptimizationPipelineToUse` (0/1/2), registered in `JITPipelineRegistry.cpp`.

| Pipeline | ID | File | Inlining strategy |
|---|---|---|---|
| P0 Inlining | 0 | `JITPipelineInlining.cpp` | Forced `alwaysinline` — unconstrained growth |
| P1 FuncSpec | 1 | `JITPipelineFuncSpec.cpp` | `ConstantArgFunctionSpecializationPass` + cost-based `ModuleInliner` |
| P2 JitIPSCCP | 2 | `JITPipelineIPSCCP.cpp` | Fork of IPSCCP + interprocedural function specialization |

All three pipelines receive the same `PipelineRunArgs` struct, which carries:
`Module&`, `Options&`, cross-registered `MAM`/`FAM`/`CGAM`/`LAM`, a `PassBuilder&`,
and two mutable trace fields (`CurrentGroup`, `CurrentFixpointIter`) for pass-trace
recording.

---

## Shared Pre-Processing (before any pipeline runs)

In `specializeOnlyImpl` (`ClangRuntimeSpecializer.cpp`), before the pipeline is
invoked:

1. Clone the blob module (or parse a fresh bitcode copy in debug builds per spec-005).
2. Build `LLJITBuilder` with `TrapUnreachable=true` (prevents 0-byte `.text` crash).
3. Create `specialized_wrapper_<N>_<addr>` — sets `ExternalLinkage`, `alwaysinline`,
   calls into the target function with the serialized constant arguments baked in as
   IR constants.
4. **Early GlobalDCE** (if `EnableEarlyPrune`): prunes the module to functions
   transitively reachable from the wrapper before the pipeline runs.  Example: 61 fns
   → 6 fns on synthetic benchmarks.  Vtable functions set to `WeakODRLinkage` by
   `IRDumpingPass` at compile time survive DCE.

---

## Pipeline 0 — Inlining (`JITPipelineInlining.cpp`)

### Pre-fixpoint: attribute scrub
Strip `NoInline`/`OptimizeNone` from all non-declaration functions so the
`AlwaysInlinerPass` can act on `alwaysinline` call-site annotations.

### Initial phase (`CurrentGroup = "initial"`)
1. FPM: `SROAPass` → `EarlyCSEPass` → `InstCombinePass` — normalize alloca/load/store
2. `ReversePostOrderFunctionAttrsPass` — infer `readonly`/`readnone`/etc.
3. FPM: `StaticMutabilityAnalysisPass` — tag loads from never-written fields with
   `!invariant.load`
4. *(if `!LargeModule`)*: `ConstantArgAlwaysInlinePass` — mark call sites with ≥1
   constant arg as `alwaysinline` on the call instruction (skips functions already
   `alwaysinline` on their definition)
5. *(if `!LargeModule`)*: `AlwaysInlinerPass(InsertLifetimeIntrinsics=true)`
6. *(if `!LargeModule`)*: FPM `InvariantLoadToConstantPass` — read host memory at
   `!invariant.load`-tagged addresses, replace loads with `Constant*`

### Fixpoint loop (`CurrentGroup = "fixpoint"`, convergence on instruction count)
1. *(if `!LargeModule`)*: `IPSCCPPass(AllowFuncSpec=false)` — interprocedural SCC
2. `DevirtualizeConstantVtableCallsPass` — resolves vtable pointer chains via dlsym +
   host-memory dereference → direct calls (module-level, no ValueHandle hazard, runs
   unconditionally)
3. `GlobalDCEPass` — prune dead functions after devirt/IPSCCP
4. `ReversePostOrderFunctionAttrsPass`
5. FPM: `StaticMutabilityAnalysisPass` → `InvariantLoadToConstantPass` — pick up new
   invariant loads exposed by inlining
6. `ConstantArgAlwaysInlinePass` → `AlwaysInlinerPass` — forced inlining of newly
   constant-arg sites
7. `GlobalOptPass` — trivial global optimizations
8. `WholeProgramDevirtPass` — whole-program virtual dispatch optimization
9. *(if `!LargeModule`)*: PreInline FPM: `EarlyCSE(MemSSA)` + `SROA` +
   `JumpThreadingPass` + `SimplifyCFG` + `InstCombine`
10. *(if `!LargeModule`)*: PostInline FPM: `StaticMutability` + `InvariantLoadToConst`
    + `InstCombine` + `SROA` + `InstCombine` + `EarlyCSE(MemSSA)` +
    `JumpThreading` + `SimplifyCFG` + LoopPM(`LoopRotate` + `LICM`) +
    `LoopUnroll(Opts.LoopUnrollCount)` + `InstCombine` + `SimplifyCFG` +
    `InstSimplify`

### Postfix (`CurrentGroup = "postfix"`)
- `GlobalDCEPass`

### Final (`CurrentGroup = "final"`, if `EnableO3Final`)
- `buildPerModuleDefaultPipeline(O3)`

---

## Pipeline 1 — FuncSpec (`JITPipelineFuncSpec.cpp`)

### Phase 1: Linkage scrub (one-time, before fixpoint)
BFS from `specialized_wrapper_*` callees → collect transitive callee set:
- Convert `AvailableExternallyLinkage` → `InternalLinkage` for reachable functions
  (so the JIT compiles their bodies; `WeakODR` functions stay `WeakODR` — downgrading
  them causes a JITLink "Missing definitions" error)
- Strip `AlwaysInline` from all functions and all call-site attributes (P1 uses the
  cost-based inliner, not forced inlining — leaving `alwaysinline` on the wrapper
  blocks the cost-based inliner from inlining callees INTO the wrapper)

### Phase 2: Fixpoint loop (`CurrentGroup = "fixpoint"`, convergence on instruction count OR module size cap)
Terminates early if `instructions > P1MaxModuleGrowth × StartingInsts`.

1. `ConstantArgFunctionSpecializationPass(MaxGroups, TargetFuncName)` — clone the
   target function with constant arguments baked into the clone's body (in-place
   specialization)
2. *(if `!LargeModule`)*: Folding FPM: `SROA` + `EarlyCSE(MemSSA)` +
   `StaticMutabilityAnalysisPass` + `InvariantLoadToConstantPass` +
   `InstCombine` + `SimplifyCFG` + LoopPM(`LoopRotate`) +
   `LoopUnroll(partial=false, Opts.LoopUnrollCount)` + `GVNPass` +
   `InstCombine` + `SimplifyCFG` + `DCEPass` — folds baked-in constants so the
   cost model sees a small clone body
3. *(if `!LargeModule`)*: `IPSCCPPass(AllowFuncSpec=false)`
4. `ReversePostOrderFunctionAttrsPass`
5. `GlobalOptPass`
6. `GlobalDCEPass`
7. `DeadArgumentEliminationPass` — prunes unused formal parameters left by in-place
   specialization
8. `ModuleInlinerPass(P1InlineThreshold)` — cost-based inlining (NOT forced — this
   is P1's budget mechanism)
9. `ReversePostOrderFunctionAttrsPass`
10. *(if `!LargeModule`)*: Post-cleanup FPM: `SROA` + `InstCombine` + `SimplifyCFG`
11. `GlobalDCEPass`
12. Budget-aware zero-arg wrapper direct inline: if target size ≤ `P1InlineThreshold`
    AND wrapper has no arguments, directly calls `llvm::InlineFunction` (bypasses cost
    model for the `specializeOnly` path)

### Phase 3: Final (`CurrentGroup = "final"`)
*(No full O3 — O3's SCC inliner ignores the budget and reproduces code-size explosion)*
- *(if `!LargeModule`)*: FPM: `InstCombine` + `SimplifyCFG`
- `ReversePostOrderFunctionAttrsPass`
- `GlobalDCEPass`

---

## Pipeline 2 — JitIPSCCP (`JITPipelineIPSCCP.cpp`)

P2 is the current default (`CRS_DEFAULT_PIPELINE=2`). It uses a **custom fork of
LLVM's IPSCCP solver** (`JitSCCP/`) that adds JIT-time host-memory constant folding
and interprocedural function specialization.

### Phase 1: Linkage scrub (identical to P1)
BFS from `specialized_wrapper_*` callees:
- `AvailableExternallyLinkage` → `InternalLinkage` for reachable functions
- Strip `AlwaysInline` from all functions and call sites

### Phase 2: Main pass (`CurrentGroup = "initial"`)
1. `GlobalDCEPass` — early prune
2. FPM: `SROAPass(ModifyCFG)` — promote stack-allocated vtable pointers to SSA so
   the solver sees them as direct constants (not loads from alloca addresses)
3. FPM: `StaticMutabilityAnalysisPass` — annotate invariant loads with
   `!invariant.load` before the solver runs (runs unconditionally, including large
   modules — skipping would leave loads un-annotated, breaking P2 specialization)
4. `JitIPSCCPPass(P2FuncSpec options)` — the core:
   - Standard IPSCCP lattice propagation on all functions simultaneously
   - **Host-memory constant folding**: loads tagged `!invariant.load` from
     inttoptr-constant pointers are folded to their current host-memory values
   - **Interprocedural function specialization**: when a call site passes constant
     arguments to a function ≥ `P2MinFunctionSize` instructions and fewer than
     `P2MaxClones` specializations exist, clones the function with constants baked in
     and re-propagates (up to `P2FuncSpecMaxIters` specialization iterations)
   - Controlled by: `ForceSpecialization`, `SpecializeOnAddress`,
     `SpecializeLiteralConstant`
5. `GlobalDCEPass`
6. FPM: `DCEPass` — removes dead instructions left after IPSCCP constant
   materialization (loads replaced with constants but not erased by solver)
7. `GlobalOptPass`
8. Manual `memory(none)` annotation loop: iterates internal functions, marks
   memory-effect-free ones as `memory(none)` + `willreturn` + `nounwind` (compensates
   for ReversePostOrderFunctionAttrs not reliably inferring this in the JIT CGSCC
   context)

### Phase 2b: Dead-call elimination
- FPM: `InstCombinePass` — eliminates calls to the now-`memory(none)` callees
- `GlobalDCEPass`

### Phase 3: Cleanup (`CurrentGroup = "final"`)
- `GlobalDCEPass`
- *(if `!LargeModule`)*: FPM: `StaticMutabilityAnalysisPass` +
  `InvariantLoadToConstantPass` + `InstCombinePass` + `SimplifyCFGPass`
  — fold any remaining `!invariant.load` values to IR immediates so O3 sees
  literal constants when it inlines
- `ReversePostOrderFunctionAttrsPass`
- `GlobalDCEPass`

### Phase 4: O3 (`CurrentGroup = "final_o3"`, if `EnableO3Final`)
- `buildPerModuleDefaultPipeline(O3)` — O3's cost-model inliner uses the
  constant-argument bonus to aggressively inline IPSCCP-specialised callees, then
  vectorizes and unrolls the merged body

---

## Pass Presence Matrix

| Pass | P0 | P1 | P2 |
|------|----|----|-----|
| Early `GlobalDCE` (pre-pipeline prune) | ✓ (shared) | ✓ (shared) | ✓ (shared) |
| `StaticMutabilityAnalysisPass` | ✓ initial + fixpoint ×2 | ✓ folding FPM (!LM) | ✓ Phase 2 (ungated) |
| `InvariantLoadToConstantPass` | ✓ initial (!LM) + fixpoint ×2 (!LM) | ✓ folding FPM (!LM) | ✓ Phase 3 (!LM) |
| `DevirtualizeConstantVtableCallsPass` (IR rewrite) | ✓ fixpoint (ungated) | **absent** | **absent** |
| Lattice-based indirect-call resolution | **absent** | **absent** | ✓ inside `JitIPSCCPPass` solver |
| `WholeProgramDevirtPass` | ✓ fixpoint | **absent** | **absent** |
| `ConstantArgAlwaysInlinePass` | ✓ initial (!LM) + fixpoint | **absent** | **absent** |
| `AlwaysInlinerPass` | ✓ initial (!LM) + fixpoint | **absent** | **absent** |
| `IPSCCPPass` (stock) | ✓ fixpoint (!LM) | ✓ fixpoint (!LM) | **replaced by JitIPSCCPPass** |
| `JitIPSCCPPass` (custom fork) | **absent** | **absent** | ✓ Phase 2 |
| `ConstantArgFunctionSpecializationPass` | **absent** | ✓ fixpoint | **absent** |
| `ModuleInlinerPass` (cost-based) | **absent** | ✓ fixpoint | **absent** |
| `GlobalDCEPass` | ✓ fixpoint + postfix | ✓ fixpoint + final | ✓ Phases 2/2b/3 |
| `GlobalOptPass` | ✓ fixpoint | ✓ fixpoint | ✓ Phase 2 |
| `ReversePostOrderFunctionAttrsPass` | ✓ initial + fixpoint | ✓ fixpoint + final | ✓ Phase 3 |
| `DeadArgumentEliminationPass` | **absent** | ✓ fixpoint | **absent** |
| `O3 final` | ✓ (if flag) | **absent** (O3 ignored budget) | ✓ (if flag) |
| Pre-/Post-inlining FPMs (SROA, EarlyCSE, etc.) | ✓ !LM | ✓ !LM | ✓ !LM (Phase 3 only) |
| `GVNPass` | **absent** | ✓ folding FPM (!LM) | **absent** |

*LM = LargeModule (module instruction count exceeds `LargeModuleInstrThreshold` after prune)*

---

## `LargeModule` Threshold and its Guards

`LargeModuleInstrThreshold` (default **10,000 instructions**; env `CRS_DEFAULT_LARGE_MODULE_INSTR_THRESHOLD`) is measured after the early-prune `GlobalDCE`.

When `LargeModule = true` the following are skipped in all pipelines:
- All `createModuleToFunctionPassAdaptor` wrapping (FPM adaptors) except the
  unconditional passes listed above
- `IPSCCPPass` (stock)
- `LoopUnroll`, `GVN`, `JumpThreading`, `LICM`, `SROA` (all FPM-only)

**Why**: LLVM ValueHandle accumulation (`AssertingVH`/`WeakTrackingVH`) in
`LLVMContext::pImpl->ValueHandles`. Passes like IPSCCP, JumpThreading, and
LazyValueInfo register handles per call. In a fixpoint loop the DenseMap rehashes;
freed bucket arrays leave stale `PrevPtr` chain-heads. Debug-mode assertion `"No name
entry found!"` fires on the next insert. Release builds use plain pointers (no
assertion) but still suffer quadratic slowdown on very large modules (e.g., DuckDB
47K functions).

Spec-005 workaround: debug builds create a fresh `LLVMContext` per
`specializeOnlyImpl` call (bitcode round-trip). Release builds use `CloneModule` +
shared `TSCtx` and rely on the `LargeModule` guard.

---

## Options / Configurable Knobs

### Shared knobs (all pipelines)
| Option | Default | Env var |
|--------|---------|---------|
| `MaxFixpointIterations` | **16** | `CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS` |
| `LoopUnrollCount` | **44** | `CRS_DEFAULT_LOOP_UNROLL_COUNT` |
| `LargeModuleInstrThreshold` | **10,000** | `CRS_DEFAULT_LARGE_MODULE_INSTR_THRESHOLD` |
| `EnableEarlyPrune` | `true` | `CRS_DEFAULT_EARLY_PRUNE` |
| `EnableO3Final` | `true` | `CRS_DEFAULT_O3_FINAL` |
| `OptimizationPipelineToUse` | **2** (P2) | `CRS_DEFAULT_PIPELINE` |
| `FuncSpecMaxGroups` | 0 (unlimited) | `CRS_DEFAULT_FUNC_SPEC_MAX_GROUPS` |

### Pipeline 1 knobs
| Option | Default | Env var |
|--------|---------|---------|
| `P1InlineThreshold` | 225 (LLVM O3 default) | `CRS_DEFAULT_P1_INLINE_THRESHOLD` |
| `P1MaxModuleGrowth` | 2.0× | `CRS_DEFAULT_P1_MAX_MODULE_GROWTH` |

### Pipeline 2 knobs (`JitFunctionSpecializationOptions`)
| Option | Default | Env var |
|--------|---------|---------|
| `P2MinFunctionSize` | **18** instrs | `CRS_P2_MIN_FUNC_SIZE` |
| `P2MaxClones` | **18** | `CRS_P2_MAX_CLONES` |
| `P2FuncSpecMaxIters` | **2** | `CRS_P2_FUNC_SPEC_ITERS` |
| `P2ForceSpecialization` | `true` | `CRS_P2_FORCE_SPEC` |
| `P2SpecializeOnAddress` | `true` | `CRS_P2_SPEC_ON_ADDR` |
| `P2SpecializeLiteralConstant` | `true` | `CRS_P2_SPEC_LITERAL` |

*Default values above are from the most recent Optuna optimization run
(`uc_optim_iter3_20260601`). The `CRS_DEFAULT_*` env vars in the source are set to
these optimized values as compile-time defaults.*

---

## JitIPSCCP Solver Internals (P2 only)

The `JitSCCP/` directory contains a fork of LLVM's IPSCCP solver (copied from
4fecb930be12 on 2026-05-27). Modifications from upstream:

### Host-memory constant folding (`JitSCCPSolver::visitLoadInst`)
- **Path A (inttoptr constants)**: if a load's pointer operand is an `inttoptr` of a
  literal integer AND the pointed-to memory is annotated `!invariant.load`
  (by `StaticMutabilityAnalysisPass`), dereferences the host address and folds the
  load to the current value.
- **Path B (propagated constants)**: if the SCCP lattice has resolved a pointer to a
  constant AND the load has `!invariant.load`, folds via host-memory dereference.
- Both paths gate on `!invariant.load` to prevent folding stores to the same field
  (the A-1 correctness fix from the 2026-05-31 review).

### Interprocedural function specialization (`JitFunctionSpecialization.cpp`)
When a call site's arguments are constants in the lattice AND the callee has ≥
`P2MinFunctionSize` instructions AND fewer than `P2MaxClones` specializations exist:
1. Clone the callee with constants substituted for the matching formal parameters
2. Re-run the SCCP propagation on the clone
3. Repeat up to `P2FuncSpecMaxIters` times
`ForceSpecialization=true`: specializes even when the call's inline cost is below
threshold (not gated on the inliner cost model).

---

## Optuna Benchmark Results (uc_optim_iter3_20260601, 2026-06-01)

50 Optuna trials across P0, P1, P2 with shared hyperparameters.

**Best trial (ID 28 — P0)**:
- `pipeline=0`, `fixpoint_max=16`, `unroll_max=62`, `large_module_max=3`
- `obj_combined_ns=172,325,082` (JIT: 57.5ms, exec: 62.7ms)

**Parameter importance** (fANOVA):

| Parameter | Importance |
|-----------|-----------|
| `pipeline` | **0.781** |
| `large_module_max` | 0.175 |
| `o3_final` | 0.017 |
| `unroll_max` | 0.014 |
| `early_prune` | 0.007 |
| `fixpoint_max` | 0.006 |

Pipeline selection dominates importance by a wide margin. The `large_module_max`
importance is because P2's IPSCCP cannot run on large modules (LargeModule guard) —
lowering the threshold from 10,000 to a small value (e.g., 3) forces conservative
mode universally, which happens to be competitive for the UC workload.

---

## Known Limitations and Remaining Gaps

### P1 vs P0 gaps
| Capability | P0 | P1 |
|---|---|---|
| Vtable devirtualization | ✓ `DevirtualizeConstantVtableCalls` | **absent** |
| `WholeProgramDevirtPass` | ✓ | **absent** |
| `ConstantArgAlwaysInlinePass` | ✓ | **absent** (cost model used instead) |
| `AlwaysInlinerPass` | ✓ | **absent** |
| `InvariantLoadToConst` on large modules | ✓ (ungated) | ✗ (inside `!LargeModule` FPM) |
| Module growth bound | none | `P1MaxModuleGrowth` |
| SIGSEGV boundary | stable | known: `unroll_max ≥ 27 + p1_max_module_growth ≤ 1.64` |

### P2 vs P0 gaps
| Capability | P0 | P2 |
|---|---|---|
| Vtable devirtualization (IR rewrite via dlsym) | ✓ `DevirtualizeConstantVtableCallsPass` per iteration | **absent** — IR call remains indirect |
| Vtable devirtualization (lattice-based) | **absent** | ✓ `JitSCCPSolver::handleCallArguments` — indirect call treated as direct when callee operand is a constant in the lattice |
| `WholeProgramDevirtPass` | ✓ | **absent** |
| `ConstantArgAlwaysInlinePass` | ✓ | **absent** |
| `AlwaysInlinerPass` | ✓ | **absent** (O3 inlines via cost model after IPSCCP) |
| Outer fixpoint loop | ✓ up to `MaxFixpointIterations` module-level iterations | **absent** — solver iterates internally but the module is not re-fed |
| Specialization mechanism | forced inline on constant-arg call sites | function clone + lattice re-propagation |

### Correctness note (A-1, 2026-05-31 review)
P2's `JitIPSCCPPass` required `StaticMutabilityAnalysisPass` to run before the solver
to gate host-memory folding on `!invariant.load`. Without the annotation, fields
written by the function could be folded to stale pre-execution host values. Fixed by
adding the annotation pass unconditionally before `JitIPSCCPPass`.

---

## Why P0 Wins the Optuna Benchmark

The UC benchmark suite consists of synthetic kernels (lambdas, virtual method
dispatch, Polybench-style loops). P0's forced-inlining approach works well for this
workload:

1. The kernels are small enough that forced inlining does not cause code-size
   explosion (early GlobalDCE + the small module size after pruning keeps the module
   manageable).
2. P0's `DevirtualizeConstantVtableCallsPass` in the fixpoint resolves virtual
   dispatch, enabling the `AlwaysInliner` to absorb the virtual callee.
3. P2's IPSCCP specialization propagates constants across function boundaries but
   does not devirtualize — indirect calls are opaque to the solver's lattice.
4. P2's single-pass structure means it cannot iterate the way P0's fixpoint does.

For use cases with large modules (TPC-H, DuckDB), P2 is the only practical pipeline
since it does not rely on `AlwaysInlinerPass` which would cause unbounded module
growth.
