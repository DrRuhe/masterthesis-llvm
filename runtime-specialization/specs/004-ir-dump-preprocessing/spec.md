# Feature Specification: IR Dump Preprocessing

**Feature Branch**: `004-ir-dump-preprocessing`  
**Created**: 2026-04-26  
**Status**: Draft  
**Input**: User description: "Compile-time preprocessing of the dumped IR: capture IR at the right moment, preprocess linkages and run GlobalDCE before serialization to reduce blob size and JIT overhead."

## User Scenarios & Testing *(mandatory)*

### User Story 1 — Compile with Plugin, Get Preprocessed Blob (Priority: P1)

A developer compiles a translation unit with the runtime specialization plugin enabled. After all optimization passes complete (but before machine code is emitted), the plugin captures the IR, applies linkage and structural transformations, runs dead code elimination, and serializes the resulting preprocessed module as an embedded bitcode blob. The blob is smaller than the raw post-optimization IR because unreachable code — including the runtime specializer's own infrastructure — has been pruned at compile time.

**Why this priority**: The preprocessed blob is the foundational input for all JIT specialization. Every JIT call reads from this blob. If the blob is bloated, every JIT call pays unnecessary parsing and optimization overhead.

**Independent Test**: Compile a TU with the plugin; extract the embedded blob; parse and inspect it. Verify that functions not reachable from any exportable target are absent, that `llvm.global_ctors`/`llvm.global_dtors` are absent, and that the blob is smaller than serializing the raw module without preprocessing.

**Acceptance Scenarios**:

1. **Given** a TU compiled with the IR-dumping plugin, **When** the resulting binary's embedded blob is parsed, **Then** `llvm.global_ctors` and `llvm.global_dtors` are absent from the module.
2. **Given** a TU compiled with the IR-dumping plugin, **When** the blob is parsed, **Then** no function with a `__clangRS` prefix is present (the runtime specializer's own constructor and helper functions have been pruned).
3. **Given** a TU that defines functions `foo` (an exported specialization target) and `bar` (used only by `__clangRS_register_blob_...`), **When** the blob is parsed, **Then** `foo` is present and `bar` is absent.
4. **Given** a TU compiled with the plugin at any optimization level (`-O0`, `-O1`, `-O2`, `-O3`), **When** the blob is parsed, **Then** the structural invariants (no global_ctors, no `__clangRS` functions, appropriate linkages) hold regardless of level.

---

### User Story 2 — JIT Specialization Loads and Uses a Pre-Preprocessed Blob (Priority: P1)

When `init()` is called at JIT time, each registered blob is parsed into a module. Because the blob was already preprocessed at compile time, the JIT no longer needs to apply most linkage transformations before optimization. The per-call `prepareModuleForJIT` step is reduced to setting the newly created wrapper function to `ExternalLinkage`. All other module-level preparation (function linkages, vtable global linkages, zero-sized global removal) was completed when the blob was produced.

**Why this priority**: This directly reduces per-call JIT latency. The bulk of `prepareModuleForJIT` work was previously paid on every specialization call; moving it to compile time makes JIT calls faster and the optimization pipeline cheaper to run.

**Independent Test**: Profile a specialization call with and without compile-time preprocessing (by toggling whether the blob was preprocessed). The post-preprocessing variant should show a measurable reduction in time spent in `prepareModuleForJIT` and the initial GlobalDCE stage.

**Acceptance Scenarios**:

1. **Given** a binary compiled with a preprocessed blob, **When** `callSpecialized<R>(funcName, args...)` runs, **Then** it produces the same result as an equivalent call with a raw (unpreprocessed) blob.
2. **Given** a preprocessed blob, **When** `specializeOnly<R>(funcName, args...)` runs, **Then** the JIT optimization pipeline terminates in the same number of fixpoint iterations as with an unpreprocessed blob (correctness invariant: preprocessing must not alter the specializable semantics of the module).
3. **Given** a preprocessed blob where all non-wrapper functions already have `InternalLinkage` or `WeakODRLinkage`, **When** `prepareModuleForJIT` runs, **Then** the only linkage modification it makes is to the newly constructed wrapper function.

---

### User Story 3 — Plugin Runs at the Right Moment in the Compilation Pipeline (Priority: P2)

A developer relies on the IR blob accurately reflecting a fully optimized but not-yet-lowered view of the TU. The plugin must run after all LLVM IR optimization passes complete (so that specialization sees the optimized control flow, inlined functions, and devirtualized calls from the host optimizer) but before machine code is emitted (so that IR structure is still available).

**Why this priority**: The timing determines what information the JIT specializer sees. Running too early (before host optimizations) produces a blob with more IR noise; running after machine code emission is not possible (IR is gone by then). The current `registerOptimizerLastEPCallback` hook is the correct and only viable insertion point.

**Independent Test**: Compile a TU with a function that the host optimizer inlines into a caller; verify that the blob for the caller's TU reflects the inlined version (not the pre-inline version). Confirm the plugin does not run during the backend lowering phase.

**Acceptance Scenarios**:

1. **Given** the plugin is loaded via `-fpass-plugin`, **When** a TU is compiled, **Then** the IR-dumping pass runs exactly once per module, at the `OptimizerLastEP` extension point, after all `ModulePassManager` optimization passes.
2. **Given** a TU compiled with `-O0`, **When** the blob is inspected, **Then** it reflects the un-optimized IR (modulo preprocessing transforms), consistent with the optimizer running no optimization passes before the plugin fires.
3. **Given** the same TU compiled multiple times without source changes, **When** the blobs are compared, **Then** they are byte-identical (the pass is deterministic).

---

### Edge Cases

- What happens when the plugin fires on a TU that defines no non-`__clangRS` functions (e.g., a pure C++ header-only helper TU)? → The function name list is empty; the blob is still registered with `nfuncs=0`; no GlobalDCE roots exist beyond vtable globals; most IR is likely pruned. This is valid and harmless.
- What happens when a function is both a specialization target and is called from within the same TU (internal call graph)? → That function is a GlobalDCE root (externally visible before linkage transformation); its callees are also kept alive as reachable nodes. Linkage is then set to InternalLinkage for JIT consumption.
- What happens if `llvm.global_ctors` contains constructors other than `__clangRS_register_blob_*` (e.g., C++ static initializers from the user's code)? → All constructors are erased, not just the `__clangRS` one. The assumption is that all static initialization has already run in the host process before JIT begins; the JIT module is only used for function specialization, not static initialization.
- What happens on incremental/PCH compilation where the same module is processed multiple times? → The pass guards against double-execution by checking for the presence of the `RuntimeSpecializeableIR_data` global at entry; if already present, the pass returns immediately.
- What if compile-time GlobalDCE removes a function that would have been reachable from the JIT wrapper? → This is a correctness violation. The compile-time DCE MUST NOT remove any function that could be a direct or transitive callee of any exported specialization target. The DCE roots must include all functions that were originally externally visible before linkage transformation.

## Requirements *(mandatory)*

### Functional Requirements

**IR Capture Timing**

- **FR-001**: The IR-dumping pass MUST register at the `OptimizerLastEP` extension point so that it runs after all LLVM IR optimization passes complete but before machine code generation begins.
- **FR-002**: The pass MUST be idempotent: if the `RuntimeSpecializeableIR_data` global is already present in the module, the pass MUST return immediately without modifying the module.

**Compile-Time Module Preprocessing**

- **FR-003**: Before serialization, the pass MUST erase `llvm.global_ctors` and `llvm.global_dtors` from the module.
- **FR-004**: Before serialization, the pass MUST remove zero-sized globals (those with a zero-element struct type `{}`) by replacing all their uses with `PoisonValue` and erasing them.
- **FR-005**: Before serialization, the pass MUST apply linkage transformations to the module. The order of operations MUST ensure that GlobalDCE pruning does not remove functions that are reachable from externally visible specialization targets. Specifically, the following linkage invariants MUST hold in the serialized bitcode:
  - Functions that are specialization targets (non-declaration, non-`__clangRS` prefix) and were originally externally visible: `InternalLinkage` (set AFTER DCE prunes unreachable code).
  - Functions that are transitively reachable from vtable constant initializers: `WeakODRLinkage` (set BEFORE DCE to prevent accidental pruning).
  - Constant globals with initializers (vtable arrays, RTTI): `WeakODRLinkage`.
  - Non-constant, non-internal globals (other than `llvm.global_ctors`/`dtors`): `AvailableExternallyLinkage`.
  - Internal/private globals (other than zero-sized ones): unchanged.
- **FR-006**: Before serialization, the pass MUST run GlobalDCE on the preprocessed module to remove unreachable functions and globals. Functions that were externally visible before the linkage transformation MUST be treated as DCE roots for this pass.
- **FR-007**: After compile-time GlobalDCE, no function with a `__clangRS` prefix MUST remain in the serialized module (those functions become unreachable once `llvm.global_ctors` is erased in FR-003).

**JIT-Time Module Preparation (Reduced Scope)**

- **FR-008**: Because the blob is preprocessed at compile time (FR-003 through FR-007), `prepareModuleForJIT` at JIT time MUST only set the newly constructed wrapper function to `ExternalLinkage`. No other linkage transformations, global removal, or structural modifications are required or permitted (see spec 003 FR-019).

**Function Name Collection**

- **FR-010**: Before serialization (and before any linkage transformations), the pass MUST collect the names of all non-declaration functions whose names do not begin with `__clangRS`. This list is used for v2 blob registration and MUST accurately reflect the set of functions available in the preprocessed blob.

**Blob Serialization and Registration**

- **FR-011**: The preprocessed module MUST be serialized to LLVM bitcode and embedded as an `[N x i8]` constant global (`RuntimeSpecializeableIR_data`) with `InternalLinkage` and `UnnamedAddr::Global`.
- **FR-012**: The blob MUST be registered at program startup via a module constructor (priority 65535) that calls `clang_runtime_specializer_register_blob_v2(ptr, len, funcs, nfuncs)`, where `funcs` is the array of function name strings collected in FR-010.
- **FR-013**: The module constructor function MUST have `InternalLinkage` and a name prefixed with `__clangRS_register_blob_` to prevent cross-TU symbol collisions.

### Key Entities

- **IRDumpingPass**: The LLVM module pass that runs at `OptimizerLastEP`. Responsible for module preprocessing (FR-003 through FR-007), bitcode serialization (FR-011), and constructor injection (FR-012 through FR-013).
- **PreprocessedBlob**: The bitcode that results from applying FR-003 through FR-007 to a post-optimization module. Invariants: no `llvm.global_ctors`, no `__clangRS` functions, vtable globals WeakODR, other functions InternalLinkage.
- **BlobEntry**: Runtime record of one registered blob. Fields: byte pointer, byte length, list of function names. Populated at program startup by the injected constructor.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: A binary compiled with the preprocessed blob produces results numerically identical to an equivalent binary compiled without preprocessing, for all benchmark kernels and smoke tests.
- **SC-002**: The preprocessed blob is measurably smaller than the raw post-optimization bitcode for any TU that includes runtime specializer headers (size reduction ≥ 1 function removed from the blob).
- **SC-003**: Per-call JIT overhead (time from `callSpecialized` entry to the first call of the specialized function) is reduced when using a preprocessed blob compared to an unpreprocessed blob of the same TU, under identical JIT pipeline options.
- **SC-004**: All existing smoke tests (`ninja check-smoke-runtime-specializer`) pass without regression after enabling compile-time preprocessing.
- **SC-005**: The compile-time GlobalDCE does not incorrectly remove any function that is a direct or indirect callee of a specialization target, verified by running the full benchmark suite and confirming no "function not found in JIT module" errors.

## Assumptions

- All C++ static initialization in the compiled TUs runs in the host process before `init()` is called. The JIT module is used solely for function specialization; erasing `llvm.global_ctors` does not cause observable side effects.
- The compile-time preprocessing is applied by the same pass that serializes the blob. There is no separate offline preprocessing step.
- "Functions that were originally externally visible" (FR-006) means functions that had `ExternalLinkage`, `WeakODRLinkage`, `LinkOnceODRLinkage`, or similar non-internal linkage in the module as it appeared at `OptimizerLastEP`, before any linkage transformations are applied.
- There is no versioning or backward-compatibility mechanism in scope for this feature. If a preprocessed blob is used with an older runtime that does not understand the preprocessing invariants, behavior is undefined.
- The preprocessing does not modify function bodies. Only metadata (linkage, global_ctors, zero-sized globals) is changed. The specialized function's semantic behavior is identical whether the blob was preprocessed or not.
