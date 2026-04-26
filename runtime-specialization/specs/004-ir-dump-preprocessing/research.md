# Research: IR Dump Preprocessing (Spec 004)

## Current Implementation Gap

`IRDumpingPass.cpp` currently satisfies:
- **FR-001** ✅ Registered at `OptimizerLastEP` (PassPlugin.cpp line 39–42)
- **FR-002** ✅ Idempotency guard (`kDataName` global check, line 61–63)
- **FR-010** ✅ Function name collection (lines 66–71) — but see Decision 2 below
- **FR-011** ✅ Bitcode serialization + `[N x i8]` constant global
- **FR-012** ✅ Constructor calls `register_blob_v2`
- **FR-013** ✅ Constructor has `InternalLinkage` + `__clangRS_register_blob_` prefix

**Missing (all of FR-003 through FR-007)**:
- No `llvm.global_ctors`/`llvm.global_dtors` erasure before serialization
- No zero-sized global removal before serialization
- No linkage transformations before serialization
- No compile-time GlobalDCE
- No post-DCE `InternalLinkage` assignment on target functions

---

## Decision 1: Preprocessing insertion point — before `WriteBitcodeToFile`

**Decision**: All preprocessing (FR-003 through FR-007) runs between function-name collection and the `WriteBitcodeToFile` call. `getOrCreateIRDumpGlobals` (which creates `PtrGV`/`LenGV`) is moved to AFTER preprocessing.

**Rationale**: GlobalDCE would erase `PtrGV`/`LenGV` if they existed during DCE (they are internal globals with no callers at that point — the constructor that references them is created after serialization). Creating them after DCE avoids the spurious erasure. The blob then contains the preprocessed module + `PtrGV`/`LenGV` with null/zero initializers (same as today, but with a preprocessed module body).

**Alternatives considered**: Running preprocessing on a clone of the module → rejected; doubles memory use, adds complexity, and the current module is discarded after serialization anyway.

---

## Decision 2: FuncNames restricted to originally-externally-visible functions

**Decision**: Change the FR-010 collection filter from `!isDeclaration() && !starts_with("__clangRS")` to additionally exclude `InternalLinkage` and `PrivateLinkage` functions.

**Rationale**: FR-010 requires FuncNames to "accurately reflect the set of functions available in the preprocessed blob" AND to be collected "before any linkage transformations". These two constraints are simultaneously satisfied only if every collected function is guaranteed to survive compile-time GlobalDCE. Only originally-externally-visible functions (ExternalLinkage / WeakODRLinkage / LinkOnceODR) are DCE roots and survive unconditionally. An InternalLinkage static helper that is dead at compile time would be in FuncNames but absent from the blob after DCE — causing `getTargetFunction` to find a name in `FuncToBlobIdx` but fail to locate the function in the blob module.

**Consequence**: InternalLinkage functions (C `static` helpers) no longer appear in FuncNames and therefore cannot be individually targeted by `callSpecialized`. This is correct: they are implementation details, not public specialization targets. They remain in the blob as callees of the exported targets and are compiled by the JIT transitively.

---

## Decision 3: DCE roots — ExternalLinkage target functions + WeakODR vtable functions/globals

**Decision**: Before running GlobalDCE, the module is prepared so that:
1. Vtable functions (transitively referenced from constant global initializers) → `WeakODRLinkage`
2. Constant globals with initializers (vtables, RTTI) → `WeakODRLinkage`
3. Non-constant non-internal globals → `AvailableExternallyLinkage` (Comdat cleared first — see Decision 3b)
4. `llvm.global_ctors`/`llvm.global_dtors` → erased
5. Zero-sized globals → replaced with PoisonValue + erased
6. Target functions remain at ExternalLinkage (= DCE root)

GlobalDCE then prunes everything not reachable from these roots.

---

## Decision 3b: Comdat globals — clear Comdat before AvailableExternally

**Decision**: For non-constant, non-internal globals that carry a Comdat (e.g., guard variables `_ZGVZ*` and local static values `_ZZN*E` for inline functions like `Options::Default()`), clear the Comdat with `G.setComdat(nullptr)` BEFORE setting `AvailableExternallyLinkage`.

**Rationale**: The LLVM IR verifier rejects `AvailableExternallyLinkage` on a global that still carries a Comdat (error: "Declaration may not be in a Comdat!"), because `isDeclarationForLinker()` returns true for `AvailableExternally` globals. Simply skipping Comdat globals (keeping them WeakODR) causes a different failure: the JIT loads them as WeakODR definitions, but conflicts with host definitions that are not in `.dynsym` → "Missing definitions in module" JIT error. Clearing the Comdat first then setting AvailableExternally makes these globals unreferenced AvailableExternally declarations; GlobalDCE prunes them entirely since AvailableExternally is not a DCE root.

**Consequence**: Guard variables and function-local static values from unreachable inline functions (e.g., `Options::Default()`) are absent from the blob. The JIT resolves the host's copies via `DynamicLibrarySearchGenerator` if they're referenced by a surviving function.

**Alternatives considered**: Skip Comdat globals entirely (keep WeakODR) → JIT "Missing definitions" crash. Handle in `prepareModuleForJIT` at JIT time → contradicts spec 003's goal of a fully simplified `prepareModuleForJIT`.

**Rationale**: The `__clangRS_register_blob_*` constructor (if present from a prior partial run) has its only entry point via `llvm.global_ctors`. Once ctors are erased, the constructor has no callers and gets DCE'd. Vtable functions need WeakODR before DCE so they are not accidentally pruned (DevirtualizeConstantVtableCallsPass needs them at JIT-optimization time). Target functions keep ExternalLinkage through DCE, then get InternalLinkage post-DCE.

---

## Decision 4: Running GlobalDCE from within an LLVM module pass

**Decision**: Create a nested `ModulePassManager` and run `GlobalDCEPass` using the outer pass's `ModuleAnalysisManager`:
```cpp
ModulePassManager MPM;
MPM.addPass(GlobalDCEPass());
MPM.run(M, AM);
```

**Rationale**: This is the standard in-tree LLVM idiom for running a sub-pass from within another pass. It reuses the existing analysis infrastructure and correctly invalidates analyses after DCE.

**Include needed**: `#include "llvm/Transforms/IPO/GlobalDCE.h"` and `#include "llvm/IR/PassManager.h"` (already transitively included; add explicitly for clarity).

---

## Decision 5: Post-DCE InternalLinkage assignment scope

**Decision**: After DCE, iterate the collected `DCERoots` set (originally-externally-visible target functions that survived) and set them to `InternalLinkage`. Do NOT iterate all module functions to find others to set; only the roots are changed. Functions reachable transitively from roots that had InternalLinkage in the original module keep their original InternalLinkage unchanged.

**Rationale**: Spec FR-005 targets functions that "were originally externally visible". InternalLinkage transitive callees were already internal and need no change. Only the DCE roots change from ExternalLinkage → InternalLinkage.

---

## Decision 6: `RuntimeSpecializeableIR_ptr` and `RuntimeSpecializeableIR_len` in the preprocessed blob

**Decision**: These two internal globals (created by `getOrCreateIRDumpGlobals`, moved to after preprocessing) will appear in the serialized blob with null/zero initializers. This is acceptable.

**Rationale**: They are harmless dead globals in the JIT context. The JIT optimization pipeline runs `GlobalDCEPass` as its first prune step, which will remove them from the cloned module. Their presence does not affect correctness or performance of specialization.

---

## FR-007 Analysis: `__clangRS` functions in the blob

**FR-007** requires no `__clangRS`-prefixed function in the serialized blob. In practice this is trivially satisfied: the `__clangRS_register_blob_*` constructor is added to the module AFTER `WriteBitcodeToFile` returns. DCE also handles the defensive case where a `__clangRS` function exists in the source module (e.g., from a partial prior invocation prevented by the idempotency check). Both guarantees hold.
