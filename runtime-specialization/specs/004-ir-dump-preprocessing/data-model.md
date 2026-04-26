# Data Model: IR Dump Preprocessing (Spec 004)

All entities are purely compile-time (within `IRDumpingPass::run()`). There are no new runtime data structures.

## Compile-Time Entities

| Entity | Type | Created When | Role |
|--------|------|--------------|------|
| `FuncNames` | `SmallVector<std::string, 64>` | Top of `run()`, before transforms | Names of originally-externally-visible non-`__clangRS` functions; used for v2 registration and as DCE roots |
| `DCERoots` | `SmallPtrSet<Function*, 64>` | Top of `run()`, same loop as FuncNames | Raw pointers to the DCE root functions; used post-DCE to assign InternalLinkage |
| `VTableFunctions` | `SmallPtrSet<Function*, 16>` | Preprocessing phase, BFS over constant globals | Functions reachable from vtable/RTTI constant initializers; get WeakODRLinkage pre-DCE |

## Preprocessing State Machine

```
Module at OptimizerLastEP
       │
       ▼
[Collect FuncNames + DCERoots]  ← FR-010: before any transforms
       │
       ▼
[Collect VTableFunctions via BFS over constant global initializers]
       │
       ▼
[VTableFunctions → WeakODRLinkage]          ┐
[Constant globals with initializers → WeakODR] │ FR-005 (pre-DCE)
[Non-constant non-internal globals → AvailableExternally] ┘
       │
       ▼
[Erase llvm.global_ctors / llvm.global_dtors]  ← FR-003
       │
       ▼
[Remove zero-sized globals (PoisonValue + erase)]  ← FR-004
       │
       ▼
[GlobalDCE via nested ModulePassManager]  ← FR-006
  Roots: ExternalLinkage fns (DCERoots) + WeakODR fns/globals
  Pruned: __clangRS fns (no ctors), dead internal fns
       │
       ▼
[DCERoots → InternalLinkage]  ← FR-005 (post-DCE)
       │
       ▼
[getOrCreateIRDumpGlobals → PtrGV (null ptr), LenGV (0 i64)]
       │
       ▼
[WriteBitcodeToFile → preprocessed module]  ← FR-011
       │
       ▼
[Create RuntimeSpecializeableIR_data constant global]  ← FR-011
[Update PtrGV/LenGV initializers to point to data global]
[Build FuncsGV array from FuncNames]
[Create __clangRS_register_blob_* constructor]  ← FR-013
[appendToGlobalCtors priority=65535]  ← FR-012
```

## Preprocessed Blob Invariants

After serialization the blob satisfies these invariants (verified by JIT-time callers and tests):

| Invariant | Linkage | Set by |
|-----------|---------|--------|
| `llvm.global_ctors`/`llvm.global_dtors` | absent | FR-003 |
| Zero-sized globals | absent | FR-004 |
| Vtable/RTTI constant globals | `WeakODRLinkage` | FR-005 |
| Vtable method functions | `WeakODRLinkage` | FR-005 |
| Specialization target functions | `InternalLinkage` | FR-005 post-DCE |
| Non-constant non-internal globals | `AvailableExternallyLinkage` | FR-005 |
| Internal/private globals | unchanged | FR-005 |
| `__clangRS`-prefixed functions | absent | FR-007 (DCE) |
