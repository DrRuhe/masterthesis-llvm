# Implementation Plan: Eliminate Manual ASM Names from Specialization API

**Branch**: `010-eliminate-asm-names` | **Date**: 2026-05-10 | **Spec**: `specs/010-eliminate-asm-names/spec.md`

---

## Summary

Remove `funcName` string parameters from `specializeLambda`, `specializeOnly`, `callSpecialized`, and all helper functions. The IR-dumping compile-time pass detects each call site, resolves the target function identity from the IR at compile time, and rewrites the call to an internal `*Resolved` variant that carries the embedded name as a private string constant. At runtime the specializer uses this constant — no `__asm__` attribute, no manual registration.

**Current state (2026-05-13)**:
- Phase 1–4 complete: lambda + funcptr specialization without funcName work end-to-end; 35/35 smoke tests pass.
- Phase 5–6 (old API removal, benchmarks, polish) not yet started.

---

## Technical Context

**Language**: C++17, LLVM 18 (opaque pointers, Itanium ABI, clang 18)  
**Primary changed files**:
- `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h` — public API + internal resolved overloads  
- `comptime/IRDumpingPass.cpp` — compile-time call-site rewriting pass  
- `test/smoke/`, `test/WIP/` — smoke tests (promoted from WIP)  
- `benchmarks/` — callers updated when old funcName overloads are removed  

**Testing**: `ninja check-wip-runtime-specializer` (WIP loop), `ninja check-smoke-runtime-specializer` (gate before commit). Always run via ninja targets — never `llvm-lit` directly.  
**Build**: `ninja LLVMRuntimeSpecializationComptimePlugin ClangRuntimeSpecializer` from `llvm/llvm/build/debug`.

---

## Constitution Check

| Gate | Status | Notes |
|------|--------|-------|
| Research motivation documented | ✅ | Simplifies API, removes error-prone string literals, enables arbitrary lambda bodies |
| Backwards compatibility | ✅ OK to break | No-legacy-support policy; old funcName overloads will be removed |
| Tests before promotion | ✅ | All new tests go through WIP → smoke workflow |
| Smoke suite green | ✅ | 41/41 pass before Phase 4 work |

---

## Implementation-Informed Design (Lessons from Phase 3)

This section replaces the original research.md findings where experience proved them wrong or incomplete.

### Lambda `operator()` Discovery — Revised

**Research.md said**: alloca-tracing to get closure struct type.  
**What actually works**: Extract `$_N` (e.g. `$_0`, `$_12`) from the **mangled name of the `specializeLambda<R, Lambda>` callee** — the Lambda template arg is encoded there. Then find a non-declaration function whose demangled name contains `"operator()"` AND whose mangled name contains the same `$_N` token. This is robust under opaque pointers and works for captureless lambdas (whose closure struct is `class.anon`, carrying no useful type info).

### Lambda Argument Index — Revised

**Research.md said**: alloca-trace the lambda arg to find its index.  
**What actually works**: Read it from the callee's mangled name:
- If callee name contains `"23ClangRuntimeSpecializer16specializeLambda"` → member function → `LambdaArgIdx = 2` (sret=0, this=1, lambda=2)
- Otherwise → free function → `LambdaArgIdx = 1` (sret=0, lambda=1)  
No alloca-tracing needed.

### Function Pointer Argument Location

For `specializeOnly(F* func, args...)` / `callSpecialized(F* func, args...)`, scan all args with `dyn_cast<Function>(arg->stripPointerCasts())`. The first argument that resolves to a `Function*` is the target. This automatically handles sret/this offsets without index arithmetic.

**Critical**: Skip (do NOT error) if no `Function*` arg is found — this means the call is the old string-based overload, which the pass should not touch. A compile-time fatal error is only appropriate for the new funcptr overloads when the pointer is NOT a compile-time constant. Currently, non-constant pointers can be silently deferred to a runtime `ClangRuntimeSpecializerDumpedIRError`.

### CallBase vs CallInst

Always use `CallBase*` (the common base of `CallInst` and `InvokeInst`). The `always_inline` attribute on helpers like `assertSpecializedLambdaIsEquivalent` causes inlining into EH-cleanup contexts, producing `InvokeInst` nodes that `dyn_cast<CallInst>` misses. When rewriting, create `InvokeInst` (preserving `getNormalDest()`/`getUnwindDest()`) if the original was an invoke, else create `CallInst`.

### forceLambdaOpEmit — Required

If the user never directly calls `lambda(...)` (only uses `spec(args...)`), clang does NOT emit the lambda's `operator()` in the TU, and the pass cannot discover it. The fix: in the user-facing `specializeLambda<R>(lambda)` overload body, take a `volatile` pointer-to-member-function of `operator()`. The `volatile` prevents dead-code elimination, forcing clang to emit the definition.

---

## Remaining Work — Minimal Complexity Breakdown

### Phase 4: specializeOnly / callSpecialized with Function Pointer

**Goal**: `specializeOnly<R>(&myFunc, args...)` works without funcName.

**Files to change**:
1. `comptime/IRDumpingPass.cpp` — fix the Phase 0b detection + Phase N2 rewriting
2. `test/WIP/speconly-funcptr.cpp` — already written, needs to pass
3. `test/WIP/zero-arg-lambda-equals-speconly.cpp` — already written, needs to pass

**IRDumpingPass changes needed** (the skeleton is in Phase 0b / Phase N2 but has a critical bug):

**BUG**: `findFunctionPtrArg` currently calls `report_fatal_error` when no `Function*` found, which breaks existing smoke tests that use `specializeOnly("funcName", args...)`. Fix: change to `continue` (skip the site) when no `Function*` is found, since the absence means it's the old string-based overload.

**How `findSpecOnlyResolvedInBody` works**: scan the callee body for a `CallInst` to a function whose name contains `"specializeOnlyResolved"` OR `"callSpecializedResolved"`. The new funcptr overloads call the resolved variant with `nullptr` as the name arg, which forces template instantiation of the resolved variant and makes it visible in the module.

**Key invariant**: The SFINAE constraint `std::enable_if_t<std::is_function_v<F>, int>` on the funcptr overloads ensures they do NOT match `const char*` or other pointer types — the IR will have two distinct mangled instantiations.

**Phase 4 test sequence**:
1. Fix the `report_fatal_error` → `continue` bug in Phase 0b
2. Run `ninja check-smoke-runtime-specializer` to confirm existing 41 tests still pass
3. Run `ninja check-wip-runtime-specializer` for the two new tests; iterate on IRDumpingPass until both pass
4. Promote the two tests to `test/smoke/`

### Phase 5: Helper Function Updates and Old API Removal

**Minimal approach** (do NOT do all at once — risk of cascading failures):

**Step A — Update helpers** (without removing old overloads yet):
- `assertSpecializedIsEquivalent(F, normalArgs, specArgs, comp)` — change to take callable `F` directly; internally call `RS->callSpecialized<R>(&F_as_fnptr, ...)` but this requires F to be a plain function. For the helper use-case where F is a plain function, `&F` works.
- `compareFunctionInstructionCounts(F, args...)` — same pattern.
- `specializeOrFallback(C, args...)` — `C` is a callable, not a function pointer. The internal `callSpecialized` call uses the new funcptr form `RS->callSpecialized<R>(&C_address_if_fn, args...)`. Since `C` can be a lambda, this requires that `C` is not a lambda (or that it's passed as a function pointer). **Simplest approach**: keep `specializeOrFallback` internal as-is but remove `funcName` from its PUBLIC signature; route through `callSpecialized<R>(&C, args...)` only when `C` is a pointer to a plain function (detected at compile time via `std::is_function_v<std::remove_pointer_t<C>>`).

**Step B — Remove old funcName overloads** (after all callers updated):
- Remove `specializeOnly(const char* funcName, ...)` member and free functions
- Remove `callSpecialized(const char* funcName, ...)` member and free functions
- Do NOT remove `specializeLambda(const char* funcName, ...)` until after smoke tests confirm the new path works
- Run `ninja check-smoke-runtime-specializer`; fix regressions one by one

**Step C — Update benchmarks**:
- `benchmarks/ClangRuntimeSpecializerBenchmark.h`: `benchmarkJITOverhead` etc. use the old string form → update to `callSpecialized<R>(&funcName_as_pointer, ...)`
- Update actual benchmark `.cpp` files under `benchmarks/polybench/`, `benchmarks/tpch/`

### Phase 6: Error Paths and Polish

**Missing-plugin error tests**: The runtime behavior when compiled without the plugin is already handled (the `Resolved` variants check for `nullptr` resolvedName and throw `ClangRuntimeSpecializerDumpedIRError`). The tests just need to be written and the error message verified.

**Old API cleanup**: After all callers are migrated and smoke tests pass, delete the old funcName overloads. One file, one pass, one verify.

---

## File Change Map

```
runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h
├── [DONE] specializeLambda<R>(lambda [, opts])  — noinline, forceLambdaOpEmit, →Resolved
├── [DONE] specializeLambdaResolved<R>(name, lambda [, opts])  — public, →specializeLambdaImpl
├── [DONE] specializeOnlyResolved<R>(name, F*, args...)  — private, nullptr check
├── [DONE] callSpecializedResolved<R>(name, F*, args...)  — private, nullptr check
├── [DONE] specializeOnly<R>(F* func, args...) member  — noinline, →specializeOnlyResolved(nullptr,...)
├── [DONE] callSpecialized<R>(F* func, args...) member  — noinline, →callSpecializedResolved(nullptr,...)
├── [DONE] free specializeOnly<R>(F* func, ...)  — delegates to RS->specializeOnlyResolved
├── [DONE] free callSpecialized<R>(F* func, ...)  — delegates to RS->callSpecializedResolved
├── [TODO] assertSpecializedIsEquivalent(F, ...)  — remove funcName
├── [TODO] compareFunctionInstructionCounts(F, ...)  — remove funcName
├── [TODO] specializeOrFallback(C, ...)  — remove funcName
└── [TODO] Delete old string-based overloads (Phase 5B)

comptime/IRDumpingPass.cpp
├── [DONE] SpecLambdaSite + Phase 0 lambda detection  — $\_N extraction
├── [DONE] Phase N lambda rewriting  — CallBase, InvokeInst awareness
├── [DONE] SpecFuncPtrSite + Phase 0b funcptr detection skeleton
├── [BUG] Phase 0b: change report_fatal_error → continue when no Function* found
├── [DONE] Phase N2 funcptr rewriting skeleton
└── [VERIFY] findSpecOnlyResolvedInBody works for both specializeOnly and callSpecialized

test/smoke/  (41 tests, all passing)
├── [DONE] specialized-lambda-{basic,void,equivalence,no-captures,raii}.cpp
├── [DONE] specialized-lambda-{complex,no-captures,zero-arg}-no-funcname.cpp
└── [TODO] speconly-funcptr.cpp, zero-arg-lambda-equals-speconly.cpp (after Phase 4)

test/WIP/  (2 remaining functional tests)
├── speconly-funcptr.cpp       [written, needs to pass]
└── zero-arg-lambda-equals-speconly.cpp  [written, needs to pass]
```

---

## Execution Order (Minimal Risk)

1. **Fix the Phase 0b bug** (skip non-funcptr specializeOnly calls instead of erroring) → verify smoke tests still 41/41
2. **Iterate WIP tests** until `speconly-funcptr.cpp` and `zero-arg-lambda-equals-speconly.cpp` pass
3. **Promote** those two tests to smoke → verify 43/43 pass
4. **Update helpers** (`assertSpecializedIsEquivalent`, `compareFunctionInstructionCounts`, `specializeOrFallback`) — remove funcName from each; update their existing smoke test callers
5. **Remove old string overloads** from the header — run smoke suite; fix regressions
6. **Update benchmarks** — compile-check only (benchmarks are run separately)
7. **Write missing-plugin error tests** → promote to smoke
8. **Final smoke sweep** both debug and release

---

## Critical Implementation Notes

- **Do not touch the lambda rewriting logic** — it works, 41 tests prove it. Only add the funcptr path.
- **SiteIdx is shared** between Phase N and Phase N2 so `@__crs_resolved_name_K` indices are unique across both lambda and funcptr sites.
- **The `findSpecOnlyResolvedInBody` function** must search for BOTH `"specializeOnlyResolved"` and `"callSpecializedResolved"` in the callee body, since the user-facing `callSpecialized(F* func, ...)` overload calls `callSpecializedResolved(nullptr, func, ...)`.
- **Zero-arg lambda unification** (FR-005): `specializeLambdaImpl` already has the `if constexpr (sizeof...(ExplicitArgs) == 0)` branch calling `specializeOnlyImpl`. Verify this in the test.
- **SC-005 (identical pass traces)**: the zero-arg lambda routes through `specializeOnlyImpl(&lambda, ...)` while `specializeOnly(&func, 42)` routes through `specializeOnlyImpl(&func, 42)`. These are NOT identical JIT wrappers because the targets differ (lambda's operator() vs computeFoo). The test should be written to compare the PASS SEQUENCE only, not instruction counts, or compare two structurally identical computations.
