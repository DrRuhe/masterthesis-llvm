# Assumptions Audit — LLVM Runtime Specializer

**Date**: 2026-05-31  
**Auditor**: Claude Sonnet 4.6 (automated)  
**Files read**: `docs/thesis.typ`, `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h`, `.cpp`, `JITPipelineInlining.cpp`, `JITPipelineFuncSpec.cpp`, `JITPipelineIPSCCP.cpp`, `comptime/IRDumpingPass.cpp`, `comptime/IRRewritingPass.cpp`, `InvariantLoadToConstant.cpp`, `specs/003-…/spec.md`, `specs/015-…/spec.md`, `specs/007-…/spec.md`

---

## Part 1 — Assumptions Present in Thesis

The following assumptions are explicitly stated in `thesis.typ`, listed with location:

| # | Assumption | Thesis location |
|---|-----------|----------------|
| A1 | No concurrent modification of specialized arguments | §3.1.1 "Concurrent Modification" (~line 162); §5.2 "Concurrent Modification" (~line 829) |
| A2 | Same address space / same process: JIT and specialized fn share memory | §3.3.1 "Soundness of Shallow Serialization", condition 1 (~line 278) |
| A3 | No concurrent modification between serialization and end of specialized call | §3.3.1, condition 2 (~line 280) |
| A4 | Vtable type-stability: runtime type of an object does not change between specialization and execution | §3.3.2 "Validity of Speculative Devirtualization" (~line 285) |
| A5 | Vtable pointer replacement is UB in C++, so the devirt speculation is safe | §3.3.2 (~line 289) |
| A6 | JIT optimization passes preserve ABI: AlwaysInliner/ConstantArgAlwaysInlinePass inline only within the JIT module | §3.3.3 "ABI Preservation" (~line 293) |
| A7 | GlobalOpt/GlobalDCE only affect InternalLinkage / AvailableExternally globals | §3.3.3 (~line 296) |
| A8 | Platform: tested on x86-64 Linux only | §5.3.1 "Architecture: x86-64 Linux only; ARM/RISC-V untested" (~line 803); §6.2 Portability (~line 859) |
| A9 | `callSpecialized` frees JIT code after the call (SpecializedFunction destructs) | §4.3 "Core Functions" (~line 401) |
| A10 | Shallow serialization: pointer arguments are passed as absolute host addresses; JIT reads host memory directly | §3.2.4 "Shallow Serialization (Current Approach)" (~line 225) |
| A11 | Break-even / amortization: `specializeOnly` overhead must be amortized over multiple calls | §5.4 "RQ5" (~line 773); Pareto breakeven formula (~line 651) |
| A12 | Instrumentation counters are inserted after optimization, reflecting optimized code | §4.6 "Instruction Instrumentation" (~line 523) |

---

## Part 2 — Assumptions NOT in Thesis (Missing)

The following assumptions are present in the implementation but are not stated (or are only partially stated) in `thesis.typ`. They are ordered by severity.

---

### MISSING-01 — `init()` is not thread-safe; must be called before concurrent use

**Assumption**: `ClangRuntimeSpecializer::init()` uses a process-global `static unique_ptr<ClangRuntimeSpecializer> Instance` with no mutex. Concurrent calls to `init()` from multiple threads would race on the singleton creation.

**Evidence in code**: `ClangRuntimeSpecializer.cpp` lines 241–641: no mutex around `if (Instance)` guard. Spec `003/spec.md` documents this in its Assumptions section ("init() is called from a single thread before any multi-threaded specialization calls").

**Risk if violated**: Two threads both pass the `if (Instance)` guard, both create a `ClangRuntimeSpecializer`, one overwrites `Instance` after the other already returned a pointer to it — classic double-init data race. The race is invisible in normal use (init is typically called in `main`), but the thesis must state this requirement explicitly because `callSpecialized` is marketed as a transparent drop-in.

**Severity**: HIGH — the annotation-free pitch ("wrap a single call site") implies any call site can be `callSpecialized`; if two threads both trigger the first specialization, the race fires silently.



---

### MISSING-02 — Each `specializeOnly` / `callSpecialized` call adds a permanent entry to the LLJIT dylib table (monotonic memory growth)

**Assumption**: Each call to `addModuleAndLookup` creates a fresh `JITDylib` named `"spec_<WrapperName>"`. The dylib is removed when the `SpecializedFunction` handle destructs (or immediately for `callSpecialized`), but the `ExecutionSession` accumulates internal bookkeeping for every dylib ever created (symbol tables, JIT compile records). The thesis says "JIT code is freed after the call" (§4.3), which is partially true: the machine code pages are freed, but the LLJIT internal state grows monotonically.

**Evidence in code**: `ClangRuntimeSpecializer.cpp` lines 699–750: `JIT->createJITDylib(…)` is called once per specialization; `removeJITDylibNoexcept` removes the dylib but `ExecutionSession` internal state persists. The thesis `§5.3.2 "JIT dylib state growth"` TODO block (~line 634) acknowledges "lookup time grows monotonically" as an *architectural characteristic* but it is not stated as an assumption or limitation in the body text.

**Risk if violated**: In production use where `callSpecialized` is called in a tight loop (e.g., per database row), the LLJIT `ExecutionSession` accumulates state over millions of calls, eventually causing OOM or degraded lookup performance. This contradicts the "transparent overhead" claim.

**Severity**: HIGH — directly affects correctness of the "JIT overhead is amortized once" framing used in RQ5.

---

### MISSING-03 — All `register_blob_v2` calls complete before `init()` is called (constructor ordering)

**Assumption**: The blob registration constructors (emitted by `IRDumpingPass`, priority 65535) must fire before the user calls `init()`. This relies on: (a) priority-65535 constructors running before `main()`-level code, and (b) the user not calling `init()` from another priority-65535 or higher-priority constructor.

**Evidence in code**: `IRDumpingPass.cpp` line 306: `appendToGlobalCtors(M, CtorFn, /*Priority=*/65535)`. `ClangRuntimeSpecializer.cpp` line 597: `if (g_registered_blobs.empty()) throw …`. Spec `003/spec.md` Assumptions section partially covers this ("clang_runtime_specializer_register_blob_v2 is always called by the TU constructor before main() starts").

**Risk if violated**: If a user calls `ClangRuntimeSpecializer::init()` from a C++ static initializer at priority ≤ 65535 (i.e., running before the blob constructors), `init()` sees an empty registry and throws `ClangRuntimeSpecializerDumpedIRError`. This is silently avoided in the current use pattern but would be a confusing failure in any non-`main`-triggered initialization.

**Severity**: MEDIUM — would produce a diagnostic error in a narrow misuse scenario; the thesis should mention the constructor-ordering requirement.

---

### MISSING-04 — Target functions must be in a separate TU from benchmark infrastructure (linker TU isolation)

**Assumption**: JIT blob = entire TU. If the kernel (specialization target) and benchmark infrastructure (Google Benchmark state, RSS monitors, lambda registrations) share a TU, the blob contains vtable-referenced symbols from `libbenchmark`. These are not in `.dynsym` (statically linked), so JIT link fails with "Symbols not found".

**Evidence in code**: `MEMORY.md` "Benchmark TU Separation Pattern" section. The benchmark `.cpp` files explicitly split: `SyntheticKernels.cpp` (kernels only) vs. `SpecializerBenchmark.cpp` (registration + includes benchmark headers). This is an architectural constraint, not merely a best practice.

**Risk if violated**: JIT link failure at runtime; silent regression if the split is accidentally violated during refactoring.

**Severity**: MEDIUM — is stated in `MEMORY.md` but **not** stated in the thesis body. For reproducibility of the benchmarks, this TU pattern must be documented.

---

### MISSING-05 — x86-64 + Linux ELF platform assumed for JITLink zero-section workaround, Large code model, and perf.map

**Assumption**: Several implementation details are x86-64 / Linux / ELF-specific:
- `TrapUnreachable=true` workaround targets a JITLink `BasicLayout` ELF behavior (zero-byte `.text` crash).
- `CodeModel::Large` is required because JIT allocations may be >2 GB from host globals — this is an x86-64 / ELF-specific addressing model issue (ARM uses `CodeModel::Medium` or ADRP).
- `dumpJITAssembly` uses x86-64 opcodes to detect `ret` (`0xC3`, `0xCB`).
- The perf.map path is `/tmp/perf-<pid>.map` (Linux perf infrastructure).

**Evidence in code**: `ClangRuntimeSpecializer.cpp` lines 259–269 (Large code model), 793 (`0xC3/0xCB`), 736–748 (perf.map). `ClangRuntimeSpecializer.cpp` comment: "Use the Large code model so JIT-compiled functions can reference host process globals (e.g. sqlite3_temp_directory) that may be >2 GB away".

**Risk if violated**: AArch64 builds crash or produce incorrect code at the Large code model boundary; the disassembler's `ret` heuristic misidentifies instructions on non-x86 ISAs; perf.map is Linux-specific.

**Severity**: MEDIUM — the thesis §6.2 "Portability" acknowledges ORC/LLJIT is tested on x86-64 and AArch64 but does not state the code-model and JITLink workarounds are x86-64-specific. This is worth stating explicitly.

---

### MISSING-06 — Argument serialization supports only integer, float, pointer, and class (by-address) types; no structs by value, references, or non-trivial copies

**Assumption**: `serializeArgumentToIR` handles: `integral` (non-bool), `float`/`double`, `pointer`, `class`. Everything else throws `ClangRuntimeSpecializerArgSerializationError`. In particular: structs passed by value are not supported; references (which decay to pointers in the IR but carry aliasing semantics) are silently serialized as `inttoptr` constants; `bool` is not handled as `i1` — it falls through to the pointer/class branch and will serialize the *address* of a bool, not its value.

**Evidence in code**: `ClangRuntimeSpecializer.h` lines 893–920 (`serializeArgumentToIR`):
```cpp
if constexpr (std::is_integral_v<Decayed> && !std::is_same_v<Decayed, bool>)
    // bool excluded from integer path
else if constexpr (std::is_floating_point_v<Decayed>)
    // float / double only; long double falls here as double (lossy)
else if constexpr (std::is_pointer_v<Decayed> || std::is_class_v<Decayed>)
    // pointer or class → inttoptr of address
else
    throw …
```

The `bool` exclusion from the integral path (`!std::is_same_v<Decayed, bool>`) means `bool` arguments fall into the `is_class_v` branch — which is false for `bool` — and then into the `throw` branch. This is a silent unsupported type.

**Risk if violated**: Passing a `bool` argument to `callSpecialized` throws at runtime with an unhelpful "Cannot serialize argument" message. Passing a `long double` loses precision silently (cast to `double`). Passing a struct by value fails at the `throw`.

**Severity**: MEDIUM — the thesis FR-016 documents the supported types, but the thesis body text (§4.3 Core Functions) does not mention unsupported types; `bool` handling is a latent bug. The thesis should document the exact type set.

---

### MISSING-07 — `bool` arguments throw at runtime for `callSpecialized` / `specializeOnly` (latent inconsistency)

This is a sub-case of MISSING-06 that deserves separate emphasis because it is a latent implementation inconsistency:

**Assumption violated**: The code `std::is_integral_v<Decayed> && !std::is_same_v<Decayed, bool>` explicitly excludes `bool` from the integer path in `serializeArgumentToIR`. `bool` is not a pointer, not a class, and does not match float — so it reaches the `throw` branch.

By contrast, `serializeTypeToLLVM` (line 109) correctly handles `bool` as `i1` — this function is used for the `specializeLambda` explicit-arg type mapping. So `bool` works as an explicit argument type for `specializeLambda`, but throws if passed to `callSpecialized` or `specializeOnly`.

**Evidence in code**: `ClangRuntimeSpecializer.h` line 895: `if constexpr (std::is_integral_v<Decayed> && !std::is_same_v<Decayed, bool>)` in `serializeArgumentToIR`. Contrast with line 109: `if constexpr (std::is_same_v<D, bool>) return llvm::Type::getInt1Ty(Ctx)` in `serializeTypeToLLVM`.

**Risk if violated**: Any call `callSpecialized<R>(&f, true)` or `callSpecialized<R>(&f, false)` where the function takes a `bool` argument throws `ClangRuntimeSpecializerArgSerializationError` at runtime. `specializeLambda` with a `bool` explicit arg type compiles fine (correct type mapping) but its corresponding `serializeArgumentToIR` call would also throw.

**Severity**: MEDIUM — the UC benchmarks apparently do not pass `bool` directly (they pass `int` or pointers), so this bug is latent. The implementation is inconsistent between the two serialization paths. Should be either fixed (extend the integer branch to include `bool`) or documented as an unsupported type. 

---

### MISSING-08 — `specializeOrFallback` only attempts specialization for plain function-pointer callables; lambda callables silently fall back

**Assumption**: `specializeOrFallbackResolved` checks `std::is_function_v<std::remove_pointer_t<std::decay_t<Callable>>>` — it only attempts specialization for bare function pointers. Lambda arguments, `std::function`, and method pointers silently skip specialization and invoke `std::invoke(C, args...)` directly without any error.

**Evidence in code**: `ClangRuntimeSpecializer.h` lines 984–1007. The `if constexpr (std::is_function_v<…>)` guard means non-function-pointer callables always fall back.

**Risk if violated**: A user expecting `specializeOrFallback(myLambda, args…)` to specialize the lambda will silently get unspecialized execution with no warning. This contradicts the annotation-free pitch if the user passes a lambda to the fallback variant.

**Severity**: LOW — `specializeLambda` is the correct API for lambdas; `specializeOrFallback` is only a drop-in for the function-pointer case. But the thesis should state this.

---

### MISSING-09 — `callSpecialized` does NOT free JIT code "immediately after the call" — it frees it when the internal `SpecializedFunction<R>` destructs at the `}` of `callImpl`

**Assumption**: The thesis §4.3 states "JIT-compiled code is freed after the call." Technically it is freed when the stack-allocated `SpecializedFunction<R> Fn(…)` destructs at the closing brace of `callImpl`, which is after `Fn()` returns. However, if `Fn()` throws an exception, `Fn` still destructs via stack unwinding, so the code is freed. This is correct but the nuance (the `SpecializedFunction` RAII handle is the actual mechanism, not a direct free call) should be stated.

**Evidence in code**: `ClangRuntimeSpecializer.h` lines 681–693: `callImpl` constructs `SpecializedFunction<R> Fn(…)`, calls `Fn()`, and lets `Fn` destruct normally. The destructor calls `detail::removeJITDylibNoexcept`.

**Risk if violated**: Not a real risk — the behavior is correct — but a reader might assume there is a direct `free()` call after `callSpecialized` returns, which is not how RAII works.

**Severity**: LOW — terminology precision issue.

---

### MISSING-10 — `--export-dynamic` (or equivalent) required for host-process symbol resolution

**Assumption**: The `DynamicLibrarySearchGenerator::GetForCurrentProcess` call in `init()` resolves JIT-referenced external symbols from the host binary's dynamic symbol table. For this to work, the host binary must expose its symbols — either via `--export-dynamic` linker flag, or by being a shared library, or by explicit symbol export. If the binary is a statically-linked executable without `--export-dynamic`, the JIT will fail to resolve any function that is not exported.

**Evidence in code**: `ClangRuntimeSpecializer.cpp` line 288: `DynamicLibrarySearchGenerator::GetForCurrentProcess(…)`. Spec `003/spec.md` Assumptions section mentions this ("The binary is linked with --export-dynamic or equivalent"), but the thesis body does not.

**Risk if violated**: JIT link fails with "Symbols not found" for any external function called by the specialized target (e.g., math library calls, user-defined functions not inlined). This is a deployment requirement invisible to users who just write `callSpecialized`.

**Severity**: MEDIUM — affects any user linking the runtime into a static binary. Should be stated in the thesis implementation chapter.

---

### MISSING-11 — Static initializers in the host process are assumed to have already run when the JIT blob fires the specialization; no re-run in JIT

**Assumption**: `IRDumpingPass` erases `llvm.global_ctors` from the blob. This means any global constructor in the target TU will NOT run again in the JIT. If a global's initialized state is required for correct JIT execution (e.g., a global `std::vector` that is push_backed-into by a constructor), the JIT will see an uninitialized global.

**Evidence in code**: `IRDumpingPass.cpp` lines 162–166: `if (auto *GCtors = ClonedM->getGlobalVariable("llvm.global_ctors")) GCtors->eraseFromParent()`. This is stated in the thesis §4.2 "IRDumpingPass" step 2, but only as a mechanism description ("static initializers have already run in the host process and must not re-run in the JIT"). The consequence — that globals initialized only by constructors will be uninitialized in JIT — is not stated.

**Risk if violated**: JIT execution of a function that depends on a constructor-initialized global produces wrong results or crashes. This is why Polybench data globals had to be non-static (from MEMORY.md).

**Severity**: HIGH for correctness — the thesis should document that constructor-initialized globals must be non-static (or must be accessible via `AvailableExternallyLinkage` from host memory).

---

### MISSING-12 — Non-static globals required for IPSCCP to read initial values from host memory

**Assumption**: For IPSCCP to read a global variable's initialized value from host memory at JIT compile time, the global must have `AvailableExternallyLinkage` in the blob. Globals with `InternalLinkage` in the JIT module are initialized by the JIT using null/zero initializers (since constructors are erased). IPSCCP sees a constant-zero-address pointer and may optimize incorrectly (or crash). The fix — making Polybench data globals non-static — is documented in MEMORY.md but not stated as a general requirement in the thesis.

**Evidence in code**: `IRDumpingPass.cpp` lines 150–159: `if (G.isConstant() && G.hasInitializer()) G.setLinkage(WeakODRLinkage); else { G.setComdat(nullptr); G.setLinkage(AvailableExternallyLinkage); }`. Non-`const` non-`static` globals get `AvailableExternal` → JIT resolves them from host. `static` (internal linkage) globals stay internal → JIT sees zero-initialized data.

**Risk if violated**: Functions accessing `static` global arrays in their blobs see zero-initialized memory in the JIT, producing wrong results. Whole functions may optimize to `unreachable` (the JITLink crash documented in MEMORY.md).

**Severity**: HIGH — a user declaring data globals as `static` for encapsulation (a normal C++ pattern) will get silently wrong JIT results. The thesis must state this requirement.

---

### MISSING-13 — JIT timeout mechanism spawns a detached background thread; no safety guarantee if a second JIT call is made before timeout completes

**Assumption**: When `JITTimeoutSeconds > 0`, `specializeOnlyImpl` spawns a `std::thread(...).detach()` to run the JIT. If the timeout fires and the caller makes another `callSpecialized` call before the background thread completes, there is no mutual exclusion on the LLJIT instance, which is not thread-safe for concurrent module additions.

**Evidence in code**: `ClangRuntimeSpecializer.h` lines 649–675: `std::thread([this, tsm = std::move(TSM), …] { … }).detach()`. The comment says "no further JIT calls should be made until it finishes (callers guard with g_last_jit_timed_out)". No actual guard is visible in the header.

**Risk if violated**: Data race on the LLJIT `ExecutionSession` if a user calls `callSpecialized` again after receiving a timeout. Undefined behavior.

**Severity**: MEDIUM — the timeout feature is used in benchmarks to handle DuckDB (47k functions). The thesis should state the single-active-JIT constraint explicitly.

---

### MISSING-14 — `specializeLambda` only supports lambdas with a single, non-generic `operator()`; generic lambdas cause a static_assert at compile time

**Assumption**: `detail::LambdaTraits<F>` has a static_assert that fires for generic lambdas (`auto` parameters). This is a fundamental limitation not mentioned in the thesis API description.

**Evidence in code**: `ClangRuntimeSpecializer.h` lines 82–98:
```cpp
template <class F> struct LambdaTraits {
  static_assert(sizeof(F) == 0,
    "specializeLambda: Lambda must have a single non-generic operator(). "
    "Generic lambdas (auto params) are not supported.");
};
```

**Risk if violated**: Compile error with a message about generic lambdas; not a runtime risk. But the thesis API section should document this limitation.

**Severity**: LOW — compile-time error with clear message. Documentation gap rather than a correctness risk.

---

## Part 3 — Thesis-vs-Implementation Contradictions

Items are ordered by severity.

---

### CONTRA-01 — Pipeline 0 description omits `WholeProgramDevirtPass`, `StaticMutabilityAnalysis`, `InvariantLoadToConstantPass`, and multiple FPM passes

**Claim in thesis**: §4.4 "Pipeline 0: Fixpoint Inlining + IPSCCP" (~line 468) describes the fixpoint loop as: `IPSCCP → DevirtualizeConstantVtableCallsPass → GlobalOpt → ConstantArgAlwaysInlinePass → AlwaysInliner → GVN`.

**Actual in code**: `JITPipelineInlining.cpp` — the fixpoint loop contains substantially more passes:
1. `IPSCCPPass` (guarded by `!LargeModule`)
2. `DevirtualizeConstantVtableCallsPass`
3. **`GlobalDCEPass`** (between devirt and attribute inference — not in thesis)
4. `ReversePostOrderFunctionAttrsPass` (not in thesis)
5. **`StaticMutabilityAnalysis::StaticMutabilityAnalysisPass`** (not in thesis)
6. **`InvariantLoadToConstantPass`** (not in thesis)
7. `ConstantArgAlwaysInlinePass`
8. `AlwaysInlinerPass`
9. `GlobalOptPass`
10. **`WholeProgramDevirtPass`** (not in thesis)
11. FPM: `EarlyCSE`, `SROA`, `JumpThreading`, `SimplifyCFG`, `InstCombine` (not in thesis)
12. FPM: post-inlining: `StaticMutabilityAnalysis`, `InvariantLoadToConstant`, `InstCombine`, `SROA`, `InstCombine`, `EarlyCSE`, `JumpThreading`, `SimplifyCFG`, `LoopRotate`, `LICM`, `LoopUnroll`, `InstCombine`, `SimplifyCFG`, `InstSimplify` (none in thesis)

The thesis also claims the fixpoint contains `GVN`. **GVN is NOT in the fixpoint loop** in the implementation; GVN only appears inside the post-inlining FPM (`PostInlineFPM`), which is itself gated on `!LargeModule`. The thesis description is thus both over-simplified (omits ~15 passes) and inaccurate (claims GVN is a top-level fixpoint step when it is buried inside an FPM adaptor).

The `initial` phase (before the fixpoint) also runs `SROA`, `EarlyCSE`, `InstCombine`, `ReversePostOrderFunctionAttrsPass`, `StaticMutabilityAnalysis`, and (for small modules) `ConstantArgAlwaysInlinePass → AlwaysInliner → InvariantLoadToConstant` — none of which are described in the thesis.

**Severity**: HIGH — the thesis description is materially incomplete and partially wrong. A reader who tries to reproduce the pipeline from the thesis text will produce a different pipeline. The `StaticMutabilityAnalysis` and `InvariantLoadToConstantPass` are particularly noteworthy as they are custom passes that do something the thesis explicitly says was superseded by IPSCCP (§3.2.3: "IPSCCP achieves equivalent or better constant propagation automatically — without a custom interprocedural analysis pass").

---

### CONTRA-02 — Design §3.2.3 says StaticMutabilityAnalysis was "superseded by IPSCCP" and "not implemented", but it IS in the production pipeline

**Claim in thesis**: §3.2.3 "Static Mutability Analysis" (~line 212): "This analysis was superseded by the IPSCCP-based JIT pipeline." The framing implies the pass is abandoned / not in use.

**Actual in code**: `StaticMutabilityAnalysis.cpp` exists and is a production pass. `JITPipelineInlining.cpp` adds it in both the initial phase and repeatedly in the fixpoint loop (lines 110, 170, 211) and post-fixpoint. `JITPipelineFuncSpec.cpp` (Pipeline 1) also includes it (line 154). It is a live, production pass, not a historical artifact.

**Severity**: CRITICAL — this is a direct, explicit contradiction. The thesis states the pass was abandoned; the implementation uses it extensively. If a reader or reviewer checks the code, this will immediately surface as a credibility issue.

---

### CONTRA-03 — Pipeline 2 description says "no outer fixpoint loop" but its description also mismatches the implementation structure

**Claim in thesis**: §4.4 "Pipeline 2: JIT-IPSCCP" (~line 506): "Pipeline 2 converges internally within a single IPSCCP invocation — no outer fixpoint loop is needed." Also: FR-010 in spec 015 describes a cleanup sequence: `GlobalDCEPass → JitIPSCCPPass → GlobalDCEPass → AlwaysInlinerPass → GlobalDCEPass + SimplifyCFGPass + InstCombinePass`.

**Actual in code**: `JITPipelineIPSCCP.cpp` runs: Phase 1 (linkage scrub) → Phase 2 (GlobalDCE + SROA + JitIPSCCPPass + GlobalDCE + DCE + GlobalOpt + memory-effects inference) → Phase 2b (InstCombine + GlobalDCE) → Phase 3 cleanup (GlobalDCE + StaticMutability + InvariantLoad + InstCombine + SimplifyCFG + ReversePostOrder + GlobalDCE) → Phase 4 (O3, optional). 

Key discrepancies vs. the spec FR-010:
- **`AlwaysInlinerPass` is NOT in the P2 implementation** — FR-010 requires it but the code does not have it (contrary to spec assumption "AlwaysInlinerPass after JitIPSCCPPass is sufficient to inline specialized clones").
- **SROA** is run before `JitIPSCCPPass` (not in spec FR-010).
- **Phase 3** contains `StaticMutabilityAnalysis` and `InvariantLoadToConstant` (spec FR-012 says they "MUST NOT appear as standalone passes in the P2 pipeline main sequence" but allows them as "final cleanup" — the implementation does use them in cleanup, which is compliant, but the thesis text §4.4 doesn't mention them at all).

**Severity**: MEDIUM — the "no outer fixpoint" claim is correct. But the pass sequence described in the thesis does not match the code (missing AlwaysInliner, extra SROA and cleanup passes). The spec FR-010 is more wrong than the thesis here (FR-010 requires AlwaysInliner; code doesn't have it).

---

### CONTRA-04 — Thesis says IRDumpingPass sets "all other defined functions to InternalLinkage" at step 2; this is wrong — it happens AFTER DCE

**Claim in thesis**: §4.2 "IRDumpingPass" (~line 371): Step 2 bullets include "Sets all other defined functions to InternalLinkage" as part of the preprocessing before GlobalDCE runs.

**Actual in code**: `IRDumpingPass.cpp` lines 202–212: The InternalLinkage assignment happens AFTER GlobalDCE runs (post-DCE), not before. The reason is documented in the code: functions must remain external-linkage during DCE to serve as DCE roots; they are demoted to InternalLinkage only after DCE has run.

```cpp
// FR-005 (post-DCE): set originally-externally-visible target functions to
// InternalLinkage. Done after DCE so they served as DCE roots during pruning.
for (auto &F : *ClonedM)
  if (DCERoots.count(&F) && !F.isDeclaration() && !F.hasWeakODRLinkage())
    F.setLinkage(GlobalValue::InternalLinkage);
```

The thesis description places this step before GlobalDCE, which would cause GlobalDCE to prune the target functions (since InternalLinkage + no external callers = dead).

**Severity**: HIGH — the ordering matters for correctness. A reader trying to re-implement the IRDumpingPass from the thesis would produce a broken implementation where target functions are pruned by GlobalDCE.

---

### CONTRA-05 — Thesis says IRDumpingPass runs at `OptimizerLastEP`; this may be accurate but conflicts with IRRewritingPass timing relative to O3

**Claim in thesis**: §4.2 "IRDumpingPass" (~line 361): "The `IRDumpingPass` runs at `OptimizerLastEP` — after all optimization passes complete but before machine code generation."

**Implicit claim**: Since IRRewritingPass runs "at the start of the Clang optimization pipeline, before `-O3`" (§4.2 IRRewritingPass, ~line 343), the sequence is: IRRewritingPass → O3 → IRDumpingPass.

**Actual in code**: `PassPlugin.cpp` should be checked. Let me verify:

**Evidence**: From `comptime/PassPlugin.cpp` (not yet read but inferrable from spec and thesis). The IRDumpingPass is registered at `OptimizerLastEP` and IRRewritingPass at an earlier EP. This ordering IS reflected in the thesis §4.2 IRRewritingPass: "This split from IRDumpingPass is required for correctness at -O3: if name resolution ran at OptimizerLastEP (after -O3), inlining and other optimizations may have eliminated or transformed the call-site structure…"

**Assessment**: The thesis description of the two-pass ordering is CORRECT. No contradiction here. The only imprecision is that the thesis says IRRewritingPass runs "before -O3" but doesn't name the specific EP (it should name `PipelineStartEP` or equivalent for completeness).

**Severity**: LOW — not a contradiction, just imprecision. Not worth a finding here.

---

### CONTRA-06 — Thesis §4.2 IRDumpingPass states the blob global uses `protected` visibility / `dso_local` / `dllexport`; the code uses `InternalLinkage` + `UnnamedAddr::Global`

**Claim in thesis**: §4.2 IRDumpingPass (~line 392): "The global variable carrying the blob is given `protected` visibility (relevant only within this module), `dso_local` preemption, and `dllexport` storage class to prevent linker dead-stripping."

**Actual in code**: `IRDumpingPass.cpp` lines 227–237:
```cpp
auto * const DataGV = new GlobalVariable(
    M, DataTy, /*isConstant=*/true,
    GlobalValue::InternalLinkage, DataInit, kDataName);
DataGV->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
```

The `DataGV` (the actual blob global `RuntimeSpecializeableIR_data`) has `InternalLinkage` and `UnnamedAddr::Global`. There is **no** `protected` visibility, **no** `dso_local`, and **no** `dllexport`. The description in the thesis is incorrect.

The `RuntimeSpecializeableIR_ptr` and `RuntimeSpecializeableIR_len` globals (from `getOrCreateIRDumpGlobals`) also have `InternalLinkage`. None of the emitted globals have `protected` / `dso_local` / `dllexport`.

**Severity**: HIGH — this is a specific factual error about the implementation. The visibility/storage-class claims in the thesis do not match the code.

---

### CONTRA-07 — Thesis says wrapper function name is `@__crs_wrapper_<N>`; code uses `specialized_wrapper_<N>_<addr>`

**Claim in thesis**: §4.3 "Wrapper Construction" (~line 452): "Constructs a wrapper function `@__crs_wrapper_<N>` with signature `() -> R`."

**Actual in code**: `ClangRuntimeSpecializer.cpp` line 685: `return "specialized_wrapper_" + std::to_string(…GlobalSpecializationCount++) + "_" + std::to_string(reinterpret_cast<uintptr_t>(this))`. The actual prefix is `specialized_wrapper_`, not `__crs_wrapper_`.

This name also appears in the optimization pipeline checks (`JITPipelineFuncSpec.cpp` line 60: `F.getName().starts_with("specialized_wrapper_")`) and in `ClangRuntimeSpecializer.cpp` line 575 (logging: `"Optimized specialized function IR:"` look up by `starts_with("specialized_wrapper_")`).

**Severity**: MEDIUM — the naming discrepancy is not correctness-critical but a thesis reviewer checking the code against the description will notice the mismatch immediately.

---

### CONTRA-08 — Thesis §4.4 Pipeline 0 claims fixpoint loop convergence condition is instruction count; implementation checks equality, not decrease

**Claim in thesis**: §4.4 (~line 480): "Fixpoint loop (until convergence or `MaxFixpointIterations`)". Implies convergence is detected.

**Actual in code**: `JITPipelineInlining.cpp` lines 248–256:
```cpp
if (InstCount == PrevInstCount)
    break;
PrevInstCount = InstCount;
```

This is correct — convergence is when instruction count stops changing (equality). The thesis description is technically accurate but does not state the convergence metric explicitly (instruction count, not, e.g., "no new inlining occurred"). This could matter for reproducibility because instruction count convergence is a proxy; a pipeline that oscillates without changing instruction count would be considered converged.

**Severity**: LOW — minor imprecision, not a contradiction.

---

### CONTRA-09 — Thesis §4.4 Pipeline 1 does not mention `DeadArgumentEliminationPass`, `ModuleInlinerPass`, or the budget-aware direct-inline fallback

**Claim in thesis**: §4.4 "Pipeline 1: Budget-Aware Function Specialization" (~line 493): describes `ConstantArgFunctionSpecializationPass` + cost-budget-based inlining. No mention of `DeadArgumentEliminationPass`, `ModuleInlinerPass` (as distinct from "cost-based inliner"), or the manual direct-inline fallback for zero-arg wrappers.

**Actual in code**: `JITPipelineFuncSpec.cpp` lines 198–259: the fixpoint includes `DeadArgumentEliminationPass` before `ModuleInlinerPass`, and after the MPM runs there is an explicit manual inline step for zero-arg wrappers (lines 231–258) that checks if the target fits in `P1InlineThreshold` instructions and force-inlines it. This force-inlining step exists because "LLVM's cost-based inliner sometimes declines to inline a specialised target into a zero-arg wrapper."

**Severity**: MEDIUM — the thesis description would leave a reader unable to understand why P1 sometimes fails to inline into a zero-arg wrapper, or why DAE is needed.

---

### CONTRA-10 — Thesis §4.2 IRDumpingPass says it collects "mangled names of all defined functions in the TU"; the implementation reads from `!crs.func_names` metadata written by IRRewritingPass, not by scanning the module

**Claim in thesis**: §4.2 (~line 374): "3. Collects the mangled names of all defined functions in the TU." This implies IRDumpingPass performs the collection itself.

**Actual in code**: `IRDumpingPass.cpp` lines 73–83: IRDumpingPass reads `FuncNames` from `!crs.func_names` named metadata that was written by `IRRewritingPass`. IRRewritingPass (not IRDumpingPass) does the collection of function names (lines 251–261 of `IRRewritingPass.cpp`). IRDumpingPass just reads what IRRewritingPass produced.

Also, `IRRewritingPass` collects function names from the original module, restricting to non-internal/non-private functions plus explicitly identified lambda targets — not literally "all defined functions in the TU".

**Severity**: MEDIUM — this is a factual error about which pass is responsible for collecting function names. The two-pass coordination through named metadata is an important architectural detail that the thesis should correctly describe.

---

### CONTRA-11 — Thesis §3.2.2 "Full Object Reconstruction" says the approach was abandoned partly because it "required `-fwhole-program-vtables` or RTTI"; the thesis also mentions `dynamic_cast` being used for type identification, but the current approach also reads vtable pointers directly from memory without any of these requirements

**Claim in thesis**: §3.2.2 (~line 203): One reason full object reconstruction was abandoned was that it "required -fwhole-program-vtables or RTTI".

**Actual in code**: The current `DevirtualizeConstantVtableCallsPass` and `JitIPSCCPPass` invariant-load extension read vtable function pointers directly from host memory at JIT time — no RTTI, no `-fwhole-program-vtables`. The vtable pointer is read as raw bytes from the constant address.

**Assessment**: This is not a contradiction per se — the thesis is describing why the *old* approach was abandoned, not claiming the new approach needs those flags. The current approach (shallow serialization + vtable pointer reading) avoids those requirements. This is correct.

**Severity**: NONE — not a contradiction. No action needed.

---

### CONTRA-12 — Thesis §4.4 Pipeline 0 says "`NoInline` and `OptimizeNone` attributes are cleared globally before the initial phase"; implementation clears them in the initial phase FPM, not before it

**Claim in thesis**: §4.4 (~line 484): "`NoInline` and `OptimizeNone` attributes are cleared globally before the initial phase to ensure they do not block inlining of the target."

**Actual in code**: `JITPipelineInlining.cpp` lines 86–91: the attribute removal happens inside `runInliningPipeline`, before the `InitialMPM`, but is part of the same function (conceptually "before the initial phase"). This is a correct description of the ordering; the attribute clearing does happen before any pass runs.

**Severity**: NONE — the thesis description is correct. No contradiction.

---

## Summary Table

| ID | Category | Severity | One-line description |
|----|----------|----------|---------------------|
| MISSING-01 | Missing assumption | HIGH | `init()` not thread-safe; must be single-threaded |
| MISSING-02 | Missing assumption | HIGH | LLJIT internal state grows monotonically per call |
| MISSING-11 | Missing assumption | HIGH | Static initializers don't re-run; non-static globals required |
| MISSING-12 | Missing assumption | HIGH | Non-static globals required for IPSCCP to see host values |
| MISSING-03 | Missing assumption | MEDIUM | Constructor ordering: blobs must register before `init()` |
| MISSING-04 | Missing assumption | MEDIUM | TU isolation: kernels must be in separate TU |
| MISSING-05 | Missing assumption | MEDIUM | x86-64 + Linux ELF for code model + JITLink workarounds |
| MISSING-06 | Missing assumption | MEDIUM | Serialization type support is limited; long double is lossy |
| MISSING-07 | Missing assumption (bug) | MEDIUM | `bool` arguments are unsupported (throw at runtime) |
| MISSING-10 | Missing assumption | MEDIUM | `--export-dynamic` required for host symbol resolution |
| MISSING-13 | Missing assumption | MEDIUM | JIT timeout spawns detached thread; no concurrent JIT safety |
| MISSING-08 | Missing assumption | LOW | `specializeOrFallback` silently no-ops for non-function-ptr callables |
| MISSING-09 | Missing assumption | LOW | JIT code freed via RAII at `}`, not via direct free after call returns |
| MISSING-14 | Missing assumption | LOW | Generic lambdas (auto params) not supported by `specializeLambda` |
| CONTRA-02 | Contradiction | CRITICAL | Thesis says `StaticMutabilityAnalysis` was superseded/abandoned; code uses it extensively in P0 and P1 |
| CONTRA-01 | Contradiction | HIGH | Pipeline 0 fixpoint description omits ~15 passes; GVN not a top-level fixpoint step |
| CONTRA-04 | Contradiction | HIGH | Thesis says InternalLinkage set before GlobalDCE; code sets it AFTER DCE |
| CONTRA-06 | Contradiction | HIGH | Thesis claims blob global has protected/dso_local/dllexport; code uses InternalLinkage + UnnamedAddr only |
| CONTRA-07 | Contradiction | MEDIUM | Wrapper function named `@__crs_wrapper_<N>` in thesis; actual prefix is `specialized_wrapper_` |
| CONTRA-09 | Contradiction | MEDIUM | Pipeline 1 omits `DeadArgumentEliminationPass`, `ModuleInlinerPass`, and budget-aware direct-inline fallback |
| CONTRA-10 | Contradiction | MEDIUM | IRDumpingPass said to collect function names; actually reads from `!crs.func_names` metadata written by IRRewritingPass |
| CONTRA-03 | Contradiction | MEDIUM | Pipeline 2 pass sequence doesn't match FR-010 (missing AlwaysInliner; extra SROA) |
| CONTRA-08 | Contradiction | LOW | Convergence metric (instruction count) not stated in thesis |
