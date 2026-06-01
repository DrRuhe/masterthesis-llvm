# Plan 017: InternalLinkage Global Variable Resolution

- [x] Verify all 22 smoke tests pass before any changes (`ninja check-smoke-runtime-specializer`)
- [x] In `comptime/IRDumpingPass.cpp`: after GlobalDCE on the clone, collect `InternalLinkage` GlobalVariables (exclude `_ZTV*`, `llvm.*`, zero-sized); for each, get host address from the original module's corresponding global
- [x] In `comptime/IRDumpingPass.cpp`: emit `RuntimeSpecializeableIR_global_names` and `RuntimeSpecializeableIR_global_addrs` globals into the original module (same pattern as `RuntimeSpecializeableIR_funcs`)
- [x] In `comptime/IRDumpingPass.cpp`: add `clang_runtime_specializer_register_blob_v3` function type + callee; extend constructor `CreateCall` to pass the two new arrays and their count
- [x] In `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.cpp`: add `std::vector<std::pair<std::string, void*>> InternalGlobals` to `BlobEntry`; implement `register_blob_v3`
- [x] In `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h`: add `std::vector<std::unordered_map<std::string, void*>> BlobInternalGlobals` member to class
- [x] In `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.cpp` (`init()`): after populating `BlobModules`, populate `BlobInternalGlobals` from `g_registered_blobs[i].InternalGlobals`
- [x] In `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h` (`specializeOnlyImpl`): after clone step, before `prepareModuleForJIT`, add inttoptr-patching loop via `make_early_inc_range`; look up `blobIdx` from `FuncToBlobIdx`
- [ ] Write `test/smoke/static-global-basic.cpp`: target reads `static int x = 42`; verifies `assertSpecializedIsEquivalent` (SC-001/FR-006)
- [ ] Write `test/smoke/static-global-modified.cpp`: increment static to 7 post-init; `specializeOrFallback`; FileCheck verifies result == 7 (SC-002)
- [ ] Write `test/smoke/static-global-constructor.cpp`: constructor fills `static double data[8]` with `i * 1.5`; target reads `data[idx]`; `assertSpecializedIsEquivalent` (SC-001)
- [ ] Write `test/smoke/static-global-readwrite.cpp`: target does `accum += v`; after call verify `accum` == `v` in host (SC-003)
- [ ] Write `test/smoke/static-global-p2-fold.cpp`: `static const int table[4]`; target reads `table[0]`; P2 run; FileCheck verifies `ret i32 10` (SC-004)
- [ ] [complex] Debug any issues in the debug-mode bitcode round-trip path (global names must survive round-trip; verify patching works in `#ifndef NDEBUG` branch)
- [ ] Verify SC-001–SC-005: `ninja check-smoke-runtime-specializer`; all 5 new tests pass, no regressions
