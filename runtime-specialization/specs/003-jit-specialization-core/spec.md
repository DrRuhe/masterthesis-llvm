# Feature Specification: JIT Specialization Core

**Feature Branch**: `003-jit-specialization-core`  
**Created**: 2026-04-24  
**Status**: Draft  
**Input**: User description: "Extract a specification for the process of runtime specialization from ClangRuntimeSpecializer.h + ClangRuntimeSpecializer.cpp. Accurately reflect the state of the current implementation."

## User Scenarios & Testing *(mandatory)*

### User Story 1 — Specialize and Call a Function in One Step (Priority: P1)

A developer wants to JIT-compile a function with one set of argument values baked in as compile-time constants and immediately execute the result. They call `callSpecialized<R>(funcName, args...)` and receive the return value. The entire JIT process — module lookup, cloning, wrapper construction, optimization, compilation, linking — happens transparently in one call. The compiled code is freed at the end of the call.

**Why this priority**: This is the primary research use case: transparent specialization at a call site. Everything else in the system exists to support this path.

**Independent Test**: Compile a function with the IR-dumping plugin, call `callSpecialized<int>("myfunc", 42)`, verify the result equals the unspecialized result, and verify JIT compilation succeeded by observing log output.

**Acceptance Scenarios**:

1. **Given** a binary compiled with the IR-dumping plugin and `init()` called, **When** `callSpecialized<int>(funcName, args...)` is invoked, **Then** the function is JIT-compiled with those arguments as IR constants and the return value equals the unspecialized result.
2. **Given** a function with no registered IR blob, **When** `callSpecialized` is called for that function name, **Then** a `ClangRuntimeSpecializerDumpedIRError` is thrown with a diagnostic naming the missing function.
3. **Given** `callSpecialized` is called with the wrong number of arguments for the target function, **Then** a `ClangRuntimeSpecializerError` is thrown.
4. **Given** two successive calls to `callSpecialized` for the same function with the same args, **Then** each call independently re-JIT-compiles and the first call's compiled code does not interfere with the second.

---

### User Story 2 — Specialize and Retain a Callable for Later Use (Priority: P1)

A developer wants to specialize a function once and then call the resulting specialized version repeatedly without re-paying JIT overhead on each call. They call `specializeOnly<R>(funcName, args...)`, receive a `SpecializedFunction<R>` RAII handle, and invoke it as many times as needed via `operator()` or `call()`. When the handle goes out of scope its destructor automatically frees all JIT-compiled machine code.

**Why this priority**: Enables the key benchmark pattern — measure JIT overhead once, then measure specialized execution overhead separately — which is critical to the research measurement methodology.

**Independent Test**: Call `specializeOnly<int>(funcName, 42)`, invoke the returned handle 100 times, verify results equal the unspecialized result on every call, then let the handle destruct and verify no crash or memory leak.

**Acceptance Scenarios**:

1. **Given** a successful `specializeOnly<R>(funcName, args...)` call, **When** the returned `SpecializedFunction<R>` is called via `operator()`, **Then** it returns the same value as the unspecialized function with those arguments.
2. **Given** a `SpecializedFunction<R>` in scope, **When** it is destroyed (goes out of scope or explicitly reassigned), **Then** the associated JITDylib is removed and compiled machine code is freed.
3. **Given** a move of `SpecializedFunction<R>` via move-constructor or move-assignment, **Then** the new owner manages the dylib lifetime; the original is left in a null/empty state and its destructor is a no-op.
4. **Given** a default-constructed `SpecializedFunction<R>`, **When** `operator bool` is called, **Then** it returns `false` and no attempt is made to invoke a null function pointer.
5. **Given** `specializeOnly<R>(funcName, opts, args...)` is called with an `Options` value, **Then** the JIT compilation uses those specific options rather than the instance's current default options.

---

### User Story 3 — Multiple TUs Registered at Startup (Priority: P1)

A binary links multiple translation units, each compiled with the IR-dumping plugin. Each TU's constructor calls `clang_runtime_specializer_register_blob_v2` at startup, registering its bitcode blob and the names of functions it contains. When `init()` is called, all blobs are parsed independently into separate module objects, and a per-function index is built so that any registered function can be looked up and specialized regardless of which TU it came from.

**Why this priority**: The multi-TU scenario is the realistic deployment (e.g., benchmark suite + library), and correct per-blob cloning (rather than cloning a merged module) is load-bearing for JIT performance.

**Independent Test**: Link a binary with two TUs each compiled with the plugin; call `callSpecialized` for a function from TU1 and then for a function from TU2; verify both succeed and produce correct results; verify `getModuleStats().FunctionCount` sums functions across both blobs.

**Acceptance Scenarios**:

1. **Given** N TUs each registering a blob at startup via `register_blob_v2`, **When** `init()` is called, **Then** exactly N blob modules are parsed, each as a separate `llvm::Module` object sharing one `LLVMContext`.
2. **Given** a function defined in TU2, **When** `callSpecialized` is called for that function, **Then** only TU2's module is cloned (not TU1's), and TU1's module is unmodified.
3. **Given** two functions from different TUs are specialized in sequence, **Then** each specialization operates on a fresh clone of its own blob module and the two clones are independent.
4. **Given** a binary where no blob is registered (compiled without the plugin), **When** `init()` is called, **Then** a `ClangRuntimeSpecializerDumpedIRError` is thrown.

---

### User Story 4 — Per-Call Options Override (Priority: P2)

A researcher benchmarks multiple JIT pipeline configurations against the same workload. For each configuration they call `setOptions(opts)` once to configure the instance default, or pass `opts` directly as the second argument to `callSpecialized` / `specializeOnly` for a one-shot per-call override. The active options during a JIT compilation are the ones passed to `specializeOnlyImpl`, not necessarily the current instance default.

**Why this priority**: Required for the optimization sweep workflow in `optimize_benchmarks.py`; without per-call option overrides, benchmarking different configurations requires separate process invocations.

**Independent Test**: Call `specializeOnly<int>(funcName, Options::O3Only(), 42)` and `specializeOnly<int>(funcName, Options::Aggressive(), 42)` back-to-back; verify both return correct results; verify `getLastTransformStats().FunctionCountAfterPrune` differs between the two calls in accordance with the configured pipeline behavior.

**Acceptance Scenarios**:

1. **Given** `setOptions(opts)` was called with `Options A`, **When** `callSpecialized<R>(funcName, args...)` (no explicit opts) is invoked, **Then** the JIT uses Options A.
2. **Given** `setOptions(opts)` configured Options A, **When** `callSpecialized<R>(funcName, Options B, args...)` is invoked, **Then** the JIT uses Options B and the instance's stored Options A is unmodified.
3. **Given** the two-arg overload is used with an `Options` value as the second argument, **Then** the SFINAE `FirstArgIsNotOptions` trait correctly routes to the with-opts overload (not the no-opts overload).

---

### User Story 5 — Module Preparation for JIT Linkage (Priority: P2)

Before the cloned module is handed to the LLJIT, its symbol linkages must be set so that (a) the specialized wrapper is the unique exported entry point, (b) the JIT optimizer can still inline any function body it needs, and (c) definitions whose canonical copy lives in the host process are not duplicated. This preparation must also remove global constructors/destructors (already run by the host) and zero-sized globals (which cause JITLink assertion failures).

**Why this priority**: Incorrect linkage preparation causes either linkage errors (duplicate symbols) or missed optimization opportunities (bodies not visible to the inliner), both of which break the specialization correctness or performance guarantees.

**Independent Test**: After `prepareModuleForJIT` runs on a cloned module, verify: the wrapper function has `ExternalLinkage`; all other non-declaration functions have `AvailableExternally` or `WeakODR` linkage; `llvm.global_ctors` is absent; constant globals with initializers (vtables) have `WeakODR` linkage.

**Acceptance Scenarios**:

1. **Given** a cloned module with a wrapper function and other function definitions, **When** `prepareModuleForJIT` runs, **Then** the wrapper has `ExternalLinkage` and every other defined function has `AvailableExternallyLinkage` (optimize path) or `InternalLinkage` (no-optimize path).
2. **Given** a module with vtable globals (constant globals with initializers), **When** `prepareModuleForJIT` runs, **Then** those globals have `WeakODRLinkage` (so GlobalDCE keeps them as roots).
3. **Given** a module with `llvm.global_ctors` / `llvm.global_dtors`, **When** `prepareModuleForJIT` runs, **Then** both are erased from the module.
4. **Given** a module with zero-sized globals (e.g., empty C++ init structs), **When** `prepareModuleForJIT` runs, **Then** those globals are replaced with poison and erased to prevent JITLink assertion failures.
5. **Given** functions that are referenced from vtable constant initializers, **When** `prepareModuleForJIT` runs in optimize mode, **Then** those functions receive `WeakODRLinkage` rather than `AvailableExternally` so the vtable-devirtualization pass can look them up.

---

### Edge Cases

- What happens when the same function name is registered by two different TUs? → The second registration overwrites the first in `FuncToBlobIdx`; the second TU's definition is used. (Current behavior; documented assumption.) [CLARIFICATION NEEDED: what if we error? is this a common pattern? e.g. could symbols of common libraries collide? The current behavior might cause silent failures when there are different functions with the same name]
- What happens when `callSpecialized` is called with a pointer argument that is null at JIT time? → The null address is serialized as `inttoptr(i64 0, ptr)` and baked in as a constant; the resulting specialized code will dereference null and crash at runtime if the function does so.
- What happens when `init()` is called concurrently from multiple threads? → The singleton guard is not thread-safe; `init()` must be called from a single thread before multi-threaded use.
- What happens when `SpecializedFunction<R>::operator()` is called on a moved-from handle? → Undefined behavior (null function pointer call). The `bool` operator should be checked first.
- What if the target function has a void return type? → `callSpecialized<void>` builds a `CreateRetVoid` in the wrapper and `specializeOnly<void>` returns a `SpecializedFunction<void>` whose `call()` invocation works correctly.

---

## Requirements *(mandatory)*

### Functional Requirements

**Singleton Lifecycle**

- **FR-001**: `init()` MUST be a process-wide singleton: the first call initializes the JIT engine and parses all registered blobs; subsequent calls return the same instance without re-initialization.
- **FR-002**: `init()` MUST initialize the host LLVM backend (native target + AsmPrinter + Disassembler) before creating the LLJIT instance.
- **FR-003**: The LLJIT instance MUST be created with `TrapUnreachable = true` to prevent zero-byte `.text` section crashes in JITLink when an optimized wrapper reduces to `unreachable`.
- **FR-004**: The LLJIT instance MUST use the Large code model so JIT-compiled functions can reference host-process globals located more than 2 GB from the JIT allocation.
- **FR-005**: On successful `init()`, the main JITDylib MUST have a `DynamicLibrarySearchGenerator` for the current process so that host-process symbols are resolvable from JIT-compiled code.
- **FR-006**: Instrumentation counter globals (`g_inst_count`, `g_load_count`, etc.) MUST be registered as explicit named symbols in the main JITDylib during `init()`, independently of the `--export-dynamic` state of the host binary.

**Blob Registration and Module Loading**

- **FR-007**: The runtime MUST support multiple IR blobs registered via `clang_runtime_specializer_register_blob_v2(ptr, len, funcs, nfuncs)` before `init()` is called. Each blob is stored with its byte range and the list of function names it defines.
- **FR-008**: During `init()`, each registered blob MUST be parsed independently into its own `llvm::Module` object. All blob modules MUST share a single `ThreadSafeContext` (`LLVMContext`) to avoid per-module context overhead.
- **FR-009**: A `FuncToBlobIdx` map MUST be populated from the pre-registered function name lists, mapping each function name to its blob module index. This map is the sole dispatch mechanism for `getTargetFunction`.
- **FR-010**: If no blobs are registered at `init()` time, `init()` MUST throw `ClangRuntimeSpecializerDumpedIRError` with a message indicating the binary was not compiled with the plugin.

**Function Lookup**

- **FR-011**: `getTargetFunction(funcName)` MUST strip a leading `&` from `funcName` (if present) before looking it up, to support `&function_name` address-of syntax used in some callers.
- **FR-012**: If a function name is not found in `FuncToBlobIdx`, `getTargetFunction` MUST throw `ClangRuntimeSpecializerDumpedIRError` naming the missing function.

**Specialization Wrapper Construction**

- **FR-013**: For each specialization call, the system MUST clone only the blob module that contains the target function. It MUST NOT clone a merged multi-TU module.
- **FR-014**: The target function in the cloned module MUST have `NoInline` and `OptimizeNone` attributes removed, and `AlwaysInline` added, before wrapper construction (`encourageInlining`).
- **FR-015**: A fresh wrapper function MUST be created in the cloned module. Its signature is `R()` (no parameters, return type matching the target function). Its name is `"specialized_wrapper_<N>_<addr>"` where `N` is a monotonically increasing per-instance counter.
- **FR-016**: Runtime arguments MUST be serialized to LLVM IR constants via `serializeArgumentsToIR` and passed as the sole call arguments to the target function. Serialization rules:
  - Integer types (non-bool) → `ConstantInt` of the corresponding bit-width
  - Floating-point types → `ConstantFP` (float → `float`, others → `double`)
  - Pointer and class types → `inttoptr(i64 <address>, ptr)` opaque pointer constant
  - Any other type → `ClangRuntimeSpecializerArgSerializationError`
- **FR-017**: The call instruction to the target function MUST carry an `AlwaysInline` attribute to force inlining during JIT optimization.
- **FR-018**: Argument count MUST be validated against the IR function arity before wrapper construction; a mismatch MUST throw `ClangRuntimeSpecializerError`.

**Module Preparation for JIT**

- **FR-019**: `prepareModuleForJIT` MUST set the following linkages in the cloned module before it is submitted to the JIT:
  - Wrapper function: `ExternalLinkage`
  - VTable functions (functions referenced transitively from constant global initializers): `WeakODRLinkage` (optimize path) or `InternalLinkage` (no-optimize path)
  - All other defined functions: `AvailableExternallyLinkage` (optimize path) or `InternalLinkage` (no-optimize path)
  - Internal/private globals: unchanged
  - Constant globals with initializers (vtables, RTTI): `WeakODRLinkage`
  - Non-constant non-internal globals: `AvailableExternallyLinkage`
- **FR-020**: `prepareModuleForJIT` MUST erase `llvm.global_ctors` and `llvm.global_dtors` from the cloned module.
- **FR-021**: `prepareModuleForJIT` MUST remove zero-sized globals (type `{}`) by replacing all uses with `PoisonValue` and erasing them, to prevent JITLink `setMutableContent` assertion failures.

**JITDylib Isolation**

- **FR-022**: Each specialization call MUST create a fresh, uniquely-named JITDylib (`"spec_<WrapperName>"`). The JITDylib is named after the wrapper to guarantee uniqueness per call.
- **FR-023**: Each per-call JITDylib MUST have the main JITDylib added to its link order so that host-process symbols and instrumentation counters are resolvable.
- **FR-024**: After `addIRModule` and `lookup`, the `JITResult` returned MUST contain both the resolved function address and a raw pointer to the owning JITDylib.

**SpecializedFunction RAII Wrapper**

- **FR-025**: `SpecializedFunction<R>` MUST be move-only (copy-constructor and copy-assignment deleted).
- **FR-026**: On destruction of a non-empty `SpecializedFunction<R>`, `detail::removeJITDylibNoexcept(ES, Dylib)` MUST be called to release the JITDylib and free the compiled machine code.
- **FR-027**: After a move, the moved-from `SpecializedFunction<R>` MUST be in the null/empty state (function pointer, dylib pointer, and ES pointer all null) so its destructor is a no-op.
- **FR-028**: `operator bool` MUST return `true` if and only if the function pointer is non-null.
- **FR-029**: `callSpecialized<R>` (the variant that does not return a `SpecializedFunction`) MUST construct a `SpecializedFunction<R>` internally, call it, and then allow it to destruct (freeing the dylib) before returning the result.

**Options and Per-Call Configuration**

- **FR-030**: `setOptions(Options)` MUST store the options as the instance-level default used by the no-opts overloads of `callSpecialized` / `specializeOnly`.
- **FR-031**: The with-opts overloads MUST use the provided `Options` value for the current call only; the instance-level default MUST NOT be modified.
- **FR-032**: `CurrentCallOptions` MUST be set from `specializeOnlyImpl` before the JIT IR transform layer runs, so the transform lambda can read it.
- **FR-033**: The `FirstArgIsNotOptions` SFINAE trait MUST prevent ambiguity when `Options` is the first call argument, routing correctly to the with-opts overload.

**Diagnostics and Observability**

- **FR-034**: `getModuleStats()` MUST return the aggregate function count, instruction count, and bitcode size across all `BlobModules`. `getLastTransformStats()` MUST return the stats snapshot from the most recent IR transform pass. Both MUST return zero-initialized structs if `init()` has not been called.
- **FR-035**: `getLastPassTrace()` MUST return a copy of the per-pass records from the most recent transform, populated by `PassInstrumentationCallbacks` in the IR transform layer. Only module-level passes are recorded (function/loop-level passes are skipped).

### Key Entities

- **BlobEntry**: One registered IR blob. Attributes: raw byte pointer, byte length, list of function names defined in the blob (populated by v2 API). Stored in `g_registered_blobs` (process-global).
- **BlobModule**: The `llvm::Module` parsed from one `BlobEntry`. One `BlobModule` per `BlobEntry`, all sharing a single `LLVMContext` via `TSCtx`. Owned by `ClangRuntimeSpecializer::BlobModules`.
- **FuncToBlobIdx**: A map from function name string to blob module index. Built once during `init()` from the `BlobEntry.FuncNames` lists.
- **SpecializationWrapper**: The synthetic `R()` function inserted into a cloned blob module. Its body is a call to the target function with serialized constant arguments. Its name is `"specialized_wrapper_<N>_<addr>"`.
- **SpecializedFunction\<R\>**: RAII handle returned by `specializeOnly`. Owns a `JITDylib*` and `ExecutionSession*`; frees the dylib on destruction. Move-only.
- **JITResult**: Internal struct `{uintptr_t Addr, JITDylib* Dylib}` used to propagate the compiled function address and dylib ownership from `addModuleAndLookup` back to the calling template.
- **Options**: Configuration struct controlling JIT pipeline behavior. Holds pipeline selector, fixpoint iteration count, loop unroll bound, large-module threshold, early-prune toggle, O3-final toggle, and debug flags. Readable from ENV vars via `Options::Default()`.
- **PassRecord**: One per-pass observability record: pass name, pass group, fixpoint iteration, function/instruction/BB counts before and after, wall time, and IR-changed flag.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: A function JIT-specialized via `callSpecialized` with constant arguments produces a result numerically identical to the unspecialized function for all tested argument values (correctness).
- **SC-002**: `specializeOnly` returns a `SpecializedFunction<R>` that can be invoked 1 000 consecutive times without crash, memory leak, or result deviation from the unspecialized baseline.
- **SC-003**: When N > 1 TUs are linked, specializing a function from each TU succeeds; the number of parsed `BlobModules` equals N; cloning one TU's blob does not alter any other TU's blob module.
- **SC-004**: A `SpecializedFunction<R>` that goes out of scope causes no heap leak: all JIT-compiled machine code pages are freed (verified by RSS comparison or valgrind suppression-free output).
- **SC-005**: `prepareModuleForJIT` produces a module in which no function other than the wrapper has `ExternalLinkage`, verified by iterating the module after the call.
- **SC-006**: After `init()` on a binary compiled without the plugin, a `ClangRuntimeSpecializerDumpedIRError` is thrown within 100 ms.
- **SC-007**: All existing smoke tests (`ninja check-smoke-runtime-specializer`) continue to pass after any change to the specialization core, with no new failures introduced.

## Assumptions

- `init()` is called from a single thread before any multi-threaded specialization calls. Thread-safety of `init()` itself is out of scope.
- The binary is linked with `--export-dynamic` (or equivalent) so that host-process symbols referenced by `AvailableExternallyLinkage` functions in the JIT module are resolvable at JIT link time.
- Argument serialization handles only integer, floating-point, pointer, and class (by address) types. Serialization of structs by value, arrays, references, or non-trivial class types is out of scope.
- `clang_runtime_specializer_register_blob_v2` is always called by the TU constructor before `main()` starts, as injected by the IR-dumping plugin. The v1 API (`register_blob` without function names) is not specified here.
- The `TSCtx` LLVMContext is shared across all blob modules; no blob module outlives the `ClangRuntimeSpecializer` instance.
- JIT optimization pipeline behavior (which passes run, in which order, fixpoint convergence) is covered in separate specifications and is deliberately excluded here.
- Debug info stripping behavior (controlled by `Options::KeepDebugInfo`) is part of the IR transform layer, not the module preparation or wrapper construction step.
- The Large code model is always set; there is no per-call override for the code model.
