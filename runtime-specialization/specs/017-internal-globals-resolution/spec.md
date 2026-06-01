# Spec 017: InternalLinkage Global Variable Resolution

## Overview

Static file-scope globals (`static T x = ...;`) have `InternalLinkage` in LLVM IR.
`IRDumpingPass` does not export them to the binary's dynamic symbol table, so
`DynamicLibrarySearchGenerator` cannot resolve them at JIT time. The JIT allocates
its own zero-initialized copy. Any target function that reads from such a global sees
zeros instead of the actual host value. When the only stores to the global (e.g., a
C++ constructor) are erased by `IRDumpingPass`, IPSCCP concludes the global is
always-zero and collapses the function body — producing either wrong results or a
JITLink crash (the polybench pattern).

The fix: at compile time, `IRDumpingPass` captures the host address of each
`InternalLinkage` global and registers it alongside the blob. At JIT time,
`specializeOnlyImpl` replaces each such global with `inttoptr(HOST_ADDR to ptr)` —
a constant pointer into host memory. Loads then read actual live data. If
`StaticMutabilityAnalysis` determines the load is non-mutating, it adds
`!invariant.load` and P2's IPSCCP folds the value to a constant at specialization
time. No value snapshot is taken at init time; globals are treated as live pointers.

This is an implementation gap, not an architectural limitation. The mechanism already
exists for non-static globals (`AvailableExternallyLinkage` + `DynamicLibrarySearchGenerator`);
this extends it to `InternalLinkage` globals via explicit address registration.

## User Scenarios

**Scenario A — Compile-time initialized static**

Given a target function that reads `static int x = 42;`,
When the function is specialized (any pipeline),
Then the specialized function returns 42, matching the unspecialized result.

**Scenario B — Modified static, then specialize**

Given a target function that reads `static int counter;`,
When `counter` is set to 7 after CRS `init()` but before `specializeOnly`,
Then the specialized function returns 7 (value at specialization time for P2,
value at call time for P0).

**Scenario C — Constructor-initialized static array** *(previously crashing)*

Given a target function that reads `static double data[N]` filled by a C++
constructor at binary startup,
When the function is specialized,
Then the specialized function reads the constructor-filled values, not zeros, and
does not crash.

**Scenario D — Read-write static global**

Given a target function that both reads and writes `static int accum`,
When the function is specialized with a constant argument `v`,
Then writes to `accum` are visible in the host process after the call returns.

**Scenario E — P2 invariant-load constant folding**

Given a target function that reads `static const int table[N]` at a constant index,
When the function is specialized with `CRS_DEFAULT_PIPELINE=2`,
Then the IR dump shows the load replaced by a literal constant (`ret i32 <N>`).

**Scenario F — No static globals (regression)**

Given a target function that uses no static globals,
When the function is specialized,
Then behavior is identical to before this change.

## Functional Requirements

**FR-001** — After `GlobalDCE` on the preprocessed clone, `IRDumpingPass` collects
all `GlobalVariable`s with `InternalLinkage` remaining in the clone. For each, the
corresponding global in the original module provides the host address (they share
names). Excluded: vtable globals (`_ZTV*`), zero-sized types, LLVM-reserved names
(`llvm.*`), and the blob globals themselves.

**FR-002** — `IRDumpingPass` emits a `[N x ptr]` names array
(`RuntimeSpecializeableIR_global_names`) and a `[N x ptr]` addrs array
(`RuntimeSpecializeableIR_global_addrs`) into the original module, with the same
`InternalLinkage` + `UnnamedAddr::Global` conventions as the existing funcs array.
Addresses are emitted as `ConstantExpr::getIntToPtr(ConstantInt::get(Int64Ty, addr), PtrTy)`.

**FR-003** — The constructor in `IRDumpingPass` calls a new
`clang_runtime_specializer_register_blob_v3` with the blob ptr+len, funcs array,
and the two new globals arrays plus their count.

**FR-004** — `register_blob_v3` populates a new
`std::vector<std::pair<std::string, void*>> InternalGlobals` field on `BlobEntry`.
`register_blob_v2` remains unchanged (FR-007).

**FR-005** — During `init()`, after populating `BlobModules`, the per-blob
`InternalGlobals` is copied into a new class member
`std::vector<std::unordered_map<std::string, void*>> BlobInternalGlobals`,
indexed to match `BlobModules`.

**FR-006** — In `specializeOnlyImpl`, immediately after the module clone step and
before `prepareModuleForJIT`, walk `InternalLinkage GlobalVariable`s via
`make_early_inc_range`. For each global whose name appears in
`BlobInternalGlobals[blobIdx]`: create `inttoptr(HOST_ADDR to ptr)`,
`replaceAllUsesWith`, `eraseFromParent`.

**FR-007** — Backwards compatibility: `register_blob_v2` is unchanged. TUs compiled
without the updated pass produce v2 blobs; `BlobInternalGlobals[i]` is empty for
those blobs; the patching loop is a no-op.

**FR-008** — The patching in FR-006 applies to both release-mode (`CloneModule`)
and debug-mode (bitcode round-trip) paths. The map uses string names as keys, which
survive bitcode round-trips.

## Success Criteria

**SC-001** — Scenario C no longer crashes and `assertSpecializedIsEquivalent`
passes for a constructor-initialized static array.

**SC-002** — Scenario B: `specializeOrFallback` returns the modified value (7),
not the pre-modification value (0).

**SC-003** — Scenario D: after the specialized call, the host-visible value of
the static global reflects the write performed inside the function.

**SC-004** — Scenario E (P2): IR dump shows `ret i32 <literal>` for a
constant-index access into a static const array.

**SC-005** — All existing 22 smoke tests continue to pass
(`ninja check-smoke-runtime-specializer`).

## Edge Cases

**EC-001 — Same static name in two TUs**: `BlobInternalGlobals` is per-blob, so
each blob's `count` is resolved to its own TU's address. No cross-TU collision.

**EC-002 — Global DCE'd from blob**: Not present in the clone after GlobalDCE →
not collected → not registered → not patched. Correct: the function never loads it.

**EC-003 — Genuinely zero static**: `static int x;` that is always zero. After
patching, loads read from the host's zero copy — still zero. Slight overhead
(runtime load vs. constant 0) is acceptable.

**EC-004 — v2-registered blobs**: `BlobInternalGlobals[i]` is empty; patching loop
is a no-op. No regression.

**EC-005 — Static pointer to heap**: `static Node* head = nullptr;` updated at
runtime. After patching, `head` is an inttoptr to the `head` variable in host
memory. Loading it dereferences the live pointer value — correct.
