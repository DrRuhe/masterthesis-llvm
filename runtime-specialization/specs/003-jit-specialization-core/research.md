# Research: JIT Specialization Core (Spec 003)

## Status

Spec 003 was extracted from the existing implementation; all functional requirements except FR-019 through FR-021 are already satisfied. This document records the decisions behind the only remaining delta: the `prepareModuleForJIT` simplification.

---

## Decision 1: Spec 004 as hard prerequisite for the prepareModuleForJIT simplification

**Decision**: `prepareModuleForJIT` can only be simplified to a single ExternalLinkage assignment after spec 004 (IR Dump Preprocessing) is fully implemented and all blobs in the repo are produced by the new plugin.

**Rationale**: The current `prepareModuleForJIT` does all linkage fixup (vtable WeakODR, InternalLinkage for functions, AvailableExternally for globals, global_ctors erasure, zero-sized global removal) at JIT time because blobs were produced by the old plugin with no preprocessing. Once spec 004 ships, the blob already has all those invariants baked in at compile time — there is nothing left for JIT-time linkage fixup except giving the wrapper `ExternalLinkage`.

**Alternatives considered**:
- Keep the full JIT-time linkage code as an idempotent pass alongside preprocessing → rejected; the constitution forbids backwards-compatibility code paths for internal versions.
- Apply simplification before spec 004 ships → rejected; blobs currently lack the preprocessing invariants; removing the linkage fixup from JIT-time would silently produce incorrect modules.

---

## Decision 2: Remove the `force-no-optimize` branch from `prepareModuleForJIT`

**Decision**: The `Optimize`/`force-no-optimize` detection block inside `prepareModuleForJIT` becomes dead code after simplification and MUST be removed with the linkage loop.

**Rationale**: The `force-no-optimize` flag only affects which linkage to assign to vtable functions (WeakODR vs InternalLinkage). Both branches are eliminated when the linkage loop is removed. The flag is still consulted by the IR transform layer independently of `prepareModuleForJIT`.

---

## Decision 3: Update the stale comment at line 449 of ClangRuntimeSpecializer.cpp

**Decision**: The comment "prepareModuleForJIT already gave the wrapper ExternalLinkage and vtable functions WeakODRLinkage" must be updated to attribute vtable WeakODR to compile-time preprocessing (IRDumpingPass), not to `prepareModuleForJIT`.

**Rationale**: After simplification `prepareModuleForJIT` no longer sets vtable function linkage; the comment would be misleading and incorrect.

---

## Decision 4: No new API changes required

**Decision**: Spec 003's FR-001 through FR-018 and FR-022 through FR-035 are already fully implemented. No new public API surface, no new types, no new method signatures are introduced by this implementation.

**Rationale**: The spec was extracted from the existing implementation. The only change is internal to `prepareModuleForJIT`.
