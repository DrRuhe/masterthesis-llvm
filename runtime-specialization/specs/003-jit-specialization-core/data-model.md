# Data Model: JIT Specialization Core (Spec 003)

All entities are already implemented. This document records their locations and invariants for reference during implementation of the prepareModuleForJIT simplification.

## Key Entities

| Entity | Location | Notes |
|--------|----------|-------|
| `BlobEntry` | `ClangRuntimeSpecializer.cpp` (process-global `g_registered_blobs`) | Raw byte pointer + length + function name list |
| `BlobModule` | `ClangRuntimeSpecializer::BlobModules` | One per BlobEntry; shares `TSCtx`/LLVMContext |
| `FuncToBlobIdx` | `ClangRuntimeSpecializer::FuncToBlobIdx` | Map from function name → blob module index |
| `SpecializationWrapper` | Created inside `specializeOnlyImpl` | Synthetic `R()` function; name `"specialized_wrapper_<N>_<addr>"` |
| `SpecializedFunction<R>` | `ClangRuntimeSpecializer.h` (public) | RAII; owns JITDylib*; move-only |
| `JITResult` | `ClangRuntimeSpecializer.cpp` (internal) | `{uintptr_t Addr, JITDylib* Dylib}` |
| `Options` | `ClangRuntimeSpecializer.h` (public) | Pipeline config; preset factories; fluent builder |
| `PassRecord` | `ClangRuntimeSpecializer.h` (public) | Per-pass observability record |

## Invariants After prepareModuleForJIT Simplification

After spec 004 ships, blobs arrive at JIT time with these invariants already established:
- `llvm.global_ctors` / `llvm.global_dtors`: absent
- Zero-sized globals: absent
- Non-declaration functions (except future wrapper): `InternalLinkage` or `WeakODRLinkage` (vtable functions)
- Constant globals with initializers (vtables, RTTI): `WeakODRLinkage`
- Non-constant non-internal globals: `AvailableExternallyLinkage`

`prepareModuleForJIT` adds only: wrapper → `ExternalLinkage`.
