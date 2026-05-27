# Plan: Add Vtable Devirtualization to Pipeline 1

## Goal

Make Pipeline 1 capable of devirtualizing virtual calls and propagating constants through virtual dispatch paths, matching the capabilities P0 already has.

The three missing ingredients in P1 are:
1. `DevirtualizeConstantVtableCallsPass` — JIT-time vtable resolution
2. `WholeProgramDevirtPass` — whole-program virtual call optimization
3. `InvariantLoadToConstantPass` running unconditionally (currently gated behind `!LargeModule`)

---

## File to Modify

`runtime/ClangRuntimeSpecializer/JITPipelineFuncSpec.cpp`

---

## Change 1: Add `DevirtualizeConstantVtableCallsPass` to the P1 fixpoint loop

### Where to insert

In the fixpoint loop, **after `ConstantArgFunctionSpecializationPass` and the folding FPM**, and **before `IPSCCPPass`**. The pass must run before IPSCCP so that any direct calls it creates can be propagated by IPSCCP in the same iteration.

### Suggested placement (between steps 2 and 3 of the current loop)

```cpp
// After folding FPM block (or after ConstantArgFunctionSpecializationPass
// if LargeModule and the FPM block was skipped):

// Devirtualize indirect calls whose vtable pointer is resolvable at JIT time.
MPM.addPass(DevirtualizeConstantVtableCallsPass());
// Prune functions made dead by devirt before inlining cost analysis.
MPM.addPass(llvm::GlobalDCEPass());
MPM.addPass(llvm::ReversePostOrderFunctionAttrsPass());
```

### Why before IPSCCP
`DevirtualizeConstantVtableCallsPass` turns an indirect call (opaque to IPSCCP) into a direct call. IPSCCP can then see the call target and propagate constants across it. Running devirt after IPSCCP wastes one iteration before the effect reaches the inliner.

### Why no `!LargeModule` guard
The pass is a module-level pass with no FunctionPassManager adaptor. It does not register `AssertingVH` handles in the `LLVMContext`. The `LargeModule` hazard applies specifically to passes that go through `createModuleToFunctionPassAdaptor` or use `LazyValueInfo`/`AssumptionCache`. `DevirtualizeConstantVtableCallsPass` iterates call instructions directly and is safe to run on any size module.

---

## Change 2: Add `WholeProgramDevirtPass` to the P1 fixpoint loop

### Where to insert

After `GlobalOptPass` and `GlobalDCEPass`, mirroring the P0 order:

```cpp
MPM.addPass(llvm::GlobalOptPass());
MPM.addPass(llvm::GlobalDCEPass());
MPM.addPass(llvm::WholeProgramDevirtPass());   // <-- add here
```

### Caveat
`WholeProgramDevirtPass` requires type metadata (`!vcall_visibility`, `!type`) to be present in the module. If the blobs were compiled without `-fwhole-program-vtables` / `-fvisibility-ms-compat`, the pass will be a no-op but harmless. Verify with a test that uses `clang -fwhole-program-vtables`.

---

## Change 3: Move `InvariantLoadToConstantPass` out of the `!LargeModule` block

### Current structure in P1 fixpoint

```cpp
if (!LargeModule) {
    llvm::FunctionPassManager FPM;
    FPM.addPass(llvm::SROAPass(...));
    FPM.addPass(llvm::EarlyCSEPass(...));
    FPM.addPass(StaticMutabilityAnalysis::StaticMutabilityAnalysisPass());
    FPM.addPass(InvariantLoadToConstantPass());   // <-- buried here
    // ... rest of folding FPM
    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(FPM)));
}
```

### Problem
`StaticMutabilityAnalysisPass` (also inside this block) emits `!invariant.load` metadata. If `LargeModule=true`, both the analysis pass and the consuming pass are skipped, so no constant materialization happens at all.

### Suggested fix
Move just `StaticMutabilityAnalysisPass` and `InvariantLoadToConstantPass` outside the `!LargeModule` guard, as a separate ungated FPM before the main folding FPM:

```cpp
// Always run: mark invariant loads and materialize their values.
{
    llvm::FunctionPassManager InvFPM;
    InvFPM.addPass(StaticMutabilityAnalysis::StaticMutabilityAnalysisPass());
    InvFPM.addPass(InvariantLoadToConstantPass());
    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(InvFPM)));
}

// Heavy folding — only for small/medium modules.
if (!LargeModule) {
    llvm::FunctionPassManager FPM;
    FPM.addPass(llvm::SROAPass(...));
    FPM.addPass(llvm::EarlyCSEPass(...));
    FPM.addPass(llvm::InstCombinePass());
    // ... (StaticMutabilityAnalysis + InvariantLoadToConstant removed here)
    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(FPM)));
}
```

### Safety note
`InvariantLoadToConstantPass` uses `mincore(2)` to validate page readability before any dereference. It is safe to run on large modules — it will simply iterate more functions but produces no allocation proportional to function count (no ValueHandles registered). The only performance concern is iteration time, which is linear and bounded by the number of load instructions.

---

## Recommended insertion order in the P1 fixpoint (complete view)

```
1. ConstantArgFunctionSpecializationPass
2. [always] StaticMutabilityAnalysis + InvariantLoadToConstant (ungated FPM)
3. [!LargeModule] Heavy folding FPM (SROA, EarlyCSE, InstCombine, LoopUnroll, GVN, ...)
4. DevirtualizeConstantVtableCallsPass          ← NEW
5. GlobalDCEPass                                 ← after devirt
6. ReversePostOrderFunctionAttrsPass             ← after devirt
7. [!LargeModule] IPSCCPPass
8. ReversePostOrderFunctionAttrsPass
9. GlobalOptPass
10. GlobalDCEPass
11. WholeProgramDevirtPass                       ← NEW
12. DeadArgumentEliminationPass
13. ModuleInlinerPass(P1InlineThreshold)
14. ReversePostOrderFunctionAttrsPass
15. [!LargeModule] Post-cleanup FPM (SROA, InstCombine, SimplifyCFG)
16. GlobalDCEPass
```

---

## Expected Impact on Failing Tests

Tests in `test/smoke/virtual-methods.cpp` and `virtual-methods-simple.cpp` currently have no P1 `RUN:` line because P1 cannot devirtualize. After Change 1, `DevirtualizeConstantVtableCallsPass` runs in P1's fixpoint. A P1 `RUN:` + `EXE-NOT: load ptr, ptr %vtable` line can be added to those tests to verify that vtable loads are eliminated.

For lambda-capture tests (`invariant-load-to-constant.cpp`, `static-analysis-robust.cpp`), Change 3 ensures the pass runs even when the module is classified as large.

---

## Risk Assessment

| Change | Risk | Mitigation |
|--------|------|-----------|
| Add `DevirtualizeConstantVtableCallsPass` | Low — module-level, no ValueHandle hazard | Run smoke tests; check for unexpected symbol-not-found in JIT link |
| Add `WholeProgramDevirtPass` | Low-medium — may be no-op without type metadata | Verify pass has no-op behavior when metadata absent |
| Move `InvariantLoadToConstantPass` out of `!LargeModule` | Low — pass is safe on large modules; linear overhead | Measure JIT time on large-module benchmarks (DuckDB) |

All three changes are additive (no existing passes removed or reordered) and are local to `JITPipelineFuncSpec.cpp`.
