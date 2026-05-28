# Spec 015 — Implementation Plan: Pipeline 2 JIT-IPSCCP

**Branch**: `015-pipeline2-jit-ipsccp` | **Date**: 2026-05-27 | **Spec**: [spec.md](spec.md)

## Summary

Add a third JIT specialization pipeline (P2) backed by a local fork of LLVM's IPSCCP pass extended with JIT-specific constant-propagation capabilities. The fork (`JitIPSCCPPass`) extends the SCCP solver to resolve `!invariant.load` loads by reading host memory and to devirtualize indirect calls whose vtable chain resolves to a module-local function — eliminating the need for a manual fixpoint loop and for `DevirtualizeConstantVtableCallsPass` as a separate pass. P2 pipeline structure: linkage scrub → early GlobalDCE → `JitIPSCCPPass(AllowFuncSpec=true)` → GlobalDCE → AlwaysInliner → cleanup. All cost-model parameters are exposed through `JitFunctionSpecializationOptions` fields backed by `CRS_P2_*` env vars, enabling automated sweep by `optimize.py`. The fork uses a 3-commit git discipline (verbatim copy → compile adjustments → JIT logic) to preserve rebase-ability against upstream LLVM.

## Technical Context

**Language/Version**: C++17, Python 3.11  
**Primary Dependencies**: LLVM (in-tree), Google Benchmark, DuckDB, Optuna  
**Storage**: DuckDB benchmark database (`benchmarks/benchmarks.duckdb`) — no schema changes needed  
**Testing**: `lit` / `FileCheck` via `ninja check-*-runtime-specializer`  
**Target Platform**: Linux x86-64 (NixOS, flake.nix environment)  
**Project Type**: LLVM in-tree compiler library + benchmark suite  
**Performance Goals**: P2 JIT overhead within 2× of P0 on virtual-dispatch use cases; specialization quality at least matching P0 for vtable patterns  
**Constraints**: No outer fixpoint loop in P2; JitSCCP fork must compile cleanly in the in-tree environment without modifying public LLVM headers  
**Scale/Scope**: ~4500 lines of copied LLVM code; 3 new source files in `JitSCCP/`; 1 new pipeline file; optimize.py changes

## Constitution Check

| Principle | Status | Notes |
|-----------|--------|-------|
| I. Correctness & Safety | ✅ PASS | `assertSpecializedIsEquivalent` used in new tests; invariant-load resolver uses `mincore` to guard host reads; graceful fallback on unresolvable pointers |
| II. LLVM Coding Standards | ✅ PASS | All new code in `clangRuntimeSpecializer` namespace; LLVM error types; no raw owning pointers; `CRS_LOG` for logging |
| III. Test-First Validation | ✅ PASS | WIP tests for vtable devirt and invariant-load written before implementation; promoted to smoke on completion |
| IV. Performance Measurement | ✅ PASS | Benchmark runs recorded with `CRS_DEFAULT_PIPELINE=2`; compared against P0 baseline in DuckDB |
| V. Minimal Public API | ✅ PASS | `JitFunctionSpecializationOptions` added to `Options` struct (existing pattern); no new public types exposed |

## Project Structure

### Documentation

```text
specs/015-pipeline2-jit-ipsccp/
├── plan.md           ← this file
├── spec.md
├── research.md       ← Phase 0 (written inline below — no separate agent needed)
└── checklists/
    └── requirements.md
```

### Source Code

```text
runtime/ClangRuntimeSpecializer/
├── JitSCCP/
│   ├── JitSCCPSolver.h          ← renamed copy of SCCPSolver.h
│   ├── JitSCCPSolver.cpp        ← renamed copy of SCCPSolver.cpp + JIT extensions
│   ├── JitFunctionSpecialization.h  ← renamed copy of FunctionSpecialization.h
│   ├── JitFunctionSpecialization.cpp← renamed copy, options-struct-based
│   ├── JitIPSCCPPass.h          ← renamed copy of SCCP.h (IPO)
│   └── JitIPSCCPPass.cpp        ← renamed copy of SCCP.cpp (IPO) + P2 wiring
├── JITPipelineP2.cpp            ← new P2 pipeline implementation
├── JITPipelineRegistry.cpp      ← add P2 entry
├── InvariantLoadToConstant.cpp  ← refactor resolve logic into PointerChainResolver
├── InvariantLoadToConstant.h
└── CMakeLists.txt               ← add JitSCCP/*.cpp + JITPipelineP2.cpp

benchmarks/optimize_benchmarks.py ← conditional pipeline-specific sampling

test/WIP/
├── pipeline2-vtable-devirt.cpp
└── pipeline2-invariant-load.cpp

test/smoke/
├── virtual-methods.cpp          ← add P2 RUN lines
└── virtual-methods-simple.cpp   ← add P2 RUN lines
```

## Research Summary

All unknowns resolved through prior design exploration (see `benchmarks/reports/260525-121400-Pipeline/`):

| Decision | Resolution |
|----------|-----------|
| Outer fixpoint loop needed? | No. SCCP solver's internal worklist handles convergence. FuncSpecMaxIters controls internal iteration. One round of inlining after IPSCCP suffices for common JIT patterns. |
| Subclass vs. copy of SCCPInstVisitor | Copy only. SCCPInstVisitor is a private CRTP class in SCCPSolver.cpp; non-virtual methods; no public header. Cannot subclass. |
| !invariant.load in solver | Extend `visitLoadInst`: when load has `!invariant.load` and pointer resolves to a constant mapped address via pointer-chain resolution, call `markConstant`. Share resolution logic with InvariantLoadToConstantPass via PointerChainResolver utility. |
| Vtable devirt in solver | Extend `visitCallBase`: when callee lattice value is a constant function pointer to a function in the module, treat as a tracked direct call — add to `ArgumentTrackedFunctions`, propagate constant args into callee via existing solver machinery. |
| FunctionSpecializer cost model | Replace all `cl::opt` globals with `JitFunctionSpecializationOptions` struct fields. Default: MinFunctionSize=1, MaxClones=0(unlimited), FuncSpecMaxIters=10, ForceSpecialization=false. |
| ENV var naming | `CRS_P2_MIN_FUNC_SIZE`, `CRS_P2_MAX_CLONES`, `CRS_P2_FUNC_SPEC_ITERS`, `CRS_P2_FORCE_SPEC` via `_envOr`. |
| optimize.py domain | Replace flat `DEFAULT_SEARCH_SPACE` dict with `sample_params(trial)` function using conditional `trial.suggest_*` gated on pipeline value. |
| ODR conflicts | Rename: `SCCPSolver`→`JitSCCPSolver`, `SCCPInstVisitor`→`JitSCCPInstVisitor`, `FunctionSpecializer`→`JitFunctionSpecializer`, `IPSCCPPass`→`JitIPSCCPPass`. All in `clangRuntimeSpecializer` namespace. |

---

## Task List

### Phase 0 — Pre-flight

- [x] Confirm `ninja check-smoke-runtime-specializer` is green on debug build before any change (`/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/llvm/build/debug`)
- [x] Note the exact LLVM commit hash at `HEAD` in `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/` (record in a comment at the top of each copied file for future rebase reference)

### Phase 1 — Refactor PointerChainResolver

- [x] [complex] Extract the pointer-chain resolution logic from `InvariantLoadToConstantPass.cpp` (address resolution, GEP/inttoptr chain traversal, `mincore` page check, host-memory read for int/float/double/pointer types) into a new free function `resolveInvariantLoadToConstant(LoadInst&) -> std::optional<llvm::Constant*>` in a new file `runtime/ClangRuntimeSpecializer/PointerChainResolver.h` (header-only or with a .cpp); update `InvariantLoadToConstantPass.cpp` to call the new function — no behaviour change
- [x] Build debug tree and confirm `ninja check-smoke-runtime-specializer` still passes after the refactor

### Phase 2 — JitSCCP Verbatim Copy (Commit 1)

- [x] Create directory `runtime/ClangRuntimeSpecializer/JitSCCP/`
- [x] Copy verbatim (byte-for-byte): `llvm/lib/Transforms/Utils/SCCPSolver.cpp` → `JitSCCP/JitSCCPSolver.cpp`
- [x] Copy verbatim: `llvm/include/llvm/Transforms/Utils/SCCPSolver.h` → `JitSCCP/JitSCCPSolver.h`
- [x] Copy verbatim: `llvm/lib/Transforms/IPO/FunctionSpecialization.cpp` → `JitSCCP/JitFunctionSpecialization.cpp`
- [x] Copy verbatim: `llvm/include/llvm/Transforms/IPO/FunctionSpecialization.h` → `JitSCCP/JitFunctionSpecialization.h`
- [x] Copy verbatim: `llvm/lib/Transforms/IPO/SCCP.cpp` → `JitSCCP/JitIPSCCPPass.cpp`
- [x] Copy verbatim: `llvm/include/llvm/Transforms/IPO/SCCP.h` → `JitSCCP/JitIPSCCPPass.h`
- [x] Add LLVM commit hash comment at the top of each copied file: `// JIT-SCCP fork — copied verbatim from LLVM <hash> on 2026-05-27`
- [x] Commit as "feat(jit-sccp): verbatim copy of LLVM SCCP/FunctionSpecialization at <hash>" — **this commit must not change any logic**

### Phase 3 — Compile-Only Adjustments (Commit 2)

- [x] In all 6 copied files: update `#include` paths — change `"llvm/Transforms/Utils/SCCPSolver.h"` → `"JitSCCPSolver.h"`, `"llvm/Transforms/IPO/FunctionSpecialization.h"` → `"JitFunctionSpecialization.h"`, `"llvm/Transforms/IPO/SCCP.h"` → `"JitIPSCCPPass.h"`; all other `llvm/*` includes remain unchanged (public LLVM headers)
- [x] In `JitSCCPSolver.h` and `JitSCCPSolver.cpp`: rename `class SCCPSolver` → `class JitSCCPSolver`; rename `class SCCPInstVisitor` → `class JitSCCPInstVisitor`; wrap all declarations in `namespace clangRuntimeSpecializer {}`
- [x] In `JitFunctionSpecialization.h` and `JitFunctionSpecialization.cpp`: rename `class FunctionSpecializer` → `class JitFunctionSpecializer`; wrap in namespace; update all references to `SCCPSolver` → `JitSCCPSolver`
- [x] In `JitIPSCCPPass.h` and `JitIPSCCPPass.cpp`: rename `class IPSCCPPass` → `class JitIPSCCPPass`; wrap in namespace; update references
- [x] Add `JitSCCP/JitSCCPSolver.cpp`, `JitSCCP/JitFunctionSpecialization.cpp`, `JitSCCP/JitIPSCCPPass.cpp` to `CMakeLists.txt` target `ClangRuntimeSpecializer`
- [x] Build debug tree: `ninja -C /home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/llvm/build/debug ClangRuntimeSpecializer` — fix any compilation errors (include path issues, namespace conflicts, LLVM_ABI macro usage) without changing logic
- [x] Commit as "build(jit-sccp): compile-only adjustments — rename, namespace, include paths" — **no logic changes, only compilation adjustments**

### Phase 4 — JitFunctionSpecializationOptions (Commit 3)

- [x] Add `JitFunctionSpecializationOptions` struct to `ClangRuntimeSpecializer.h` (alongside existing `Options`):
  ```cpp
  struct JitFunctionSpecializationOptions {
    unsigned MinFunctionSize = 1;
    unsigned MaxClones       = 0;   // 0 = unlimited
    unsigned FuncSpecMaxIters = 10;
    bool     ForceSpecialization = false;
    bool     SpecializeOnAddress = false;
    bool     SpecializeLiteralConstant = true;
  };
  ```
- [x] Add `JitFunctionSpecializationOptions P2FuncSpec` field to the main `Options` struct
- [x] Add `_envOr` reads in `Options::Default()` for `CRS_P2_MIN_FUNC_SIZE` (default 1), `CRS_P2_MAX_CLONES` (default 0), `CRS_P2_FUNC_SPEC_ITERS` (default 10), `CRS_P2_FORCE_SPEC` (default 0)
- [x] Add fluent builder methods `withP2MinFuncSize(unsigned)`, `withP2MaxClones(unsigned)`, `withP2FuncSpecIters(unsigned)`, `withP2ForceSpec(bool)` to `Options`
- [x] In `JitFunctionSpecialization.h`: replace all `cl::opt` references (`MinFunctionSize`, `MaxClones`, `FuncSpecMaxIters`, `ForceSpecialization`, `SpecializeOnAddress`, `SpecializeLiteralConstant`) with fields read from a `JitFunctionSpecializationOptions` parameter passed to the constructor
- [x] In `JitFunctionSpecialization.cpp`: remove the `cl::opt` global declarations for those parameters; update all usages to read from the constructor-injected struct
- [x] In `JitIPSCCPPass.h/cpp`: update `JitIPSCCPPass` constructor to accept `JitFunctionSpecializationOptions`; pass it through to `JitFunctionSpecializer`
- [x] Build and confirm compilation clean

### Phase 5 — Solver Extensions (Commit 4)

- [x] [complex] In `JitSCCP/JitSCCPSolver.cpp`, add a `#include` for `PointerChainResolver.h`; in `JitSCCPInstVisitor::visitLoadInst`, after the existing constant-pointer load path and before the final `mergeInValue(&I, getValueFromMetadata(&I))` fallback, add: if `I.hasMetadata(LLVMContext::MD_invariant_load)` and `isBlockExecutable(I.getParent())`, call `resolveInvariantLoadToConstant(I)`; if it returns a non-null `Constant*`, call `markConstant(IV, &I, C)` and return. Mark the addition with `// JIT-SCCP extension: !invariant.load host-memory resolution` comment.
- [x] [complex] In `JitSCCPInstVisitor::visitCallBase` (or `handleCallArguments`): after existing argument handling, check if the called value's lattice state is a constant (`SCCPSolver::isConstant(CalleeLattice)`); if so, cast to `Function*` via `dyn_cast<Function>(getConstant(...))`; if the function is present in the module and not already in `ArgumentTrackedFunctions`, add it via `addArgumentTrackedFunction` and `addTrackedFunction`, mark its entry block executable, and call `handleCallArguments` to propagate constants into its parameters. Mark with `// JIT-SCCP extension: vtable indirect-call devirtualization` comment.
- [x] Build debug tree; confirm compilation is clean
- [x] Commit as "feat(jit-sccp): extend solver with !invariant.load resolution and vtable devirt"

### Phase 6 — P2 Pipeline (Commit 5)

- [x] Create `runtime/ClangRuntimeSpecializer/JITPipelineP2.cpp` implementing `runP2Pipeline(PipelineRunArgs& Args)`:
  1. Linkage scrub: BFS from `specialized_wrapper_*` callees → convert `AvailableExternalLinkage` → `InternalLinkage`; strip `AlwaysInline` from all functions and call sites (same logic as P1's linkage scrub in `JITPipelineFuncSpec.cpp`)
  2. Early `GlobalDCEPass`
  3. `JitIPSCCPPass` constructed with `Args.Opts.P2FuncSpec`; run via `MPM.addPass(JitIPSCCPPass(Args.Opts.P2FuncSpec))`
  4. `GlobalDCEPass` + `ReversePostOrderFunctionAttrsPass`
  5. `AlwaysInlinerPass(/*InsertLifetimeIntrinsics=*/true)`
  6. Cleanup: `GlobalDCEPass` + FPM (`InstCombinePass` + `SimplifyCFGPass`)
  7. Optional: `InvariantLoadToConstantPass` (FPM) as a final cleanup to replace any remaining invariant loads in inlined bodies — gated on `!LargeModule`
- [x] Add `#include "JITPipelineP2.h"` declaration header (or forward-declare `runP2Pipeline` in `JITPipeline.h`)
- [x] Add `JITPipelineP2.cpp` to `CMakeLists.txt`
- [x] In `JITPipelineRegistry.cpp`: add `{"jit-ipsccp", runP2Pipeline}` as the third entry (index 2)
- [x] Build debug tree clean

### Phase 7 — WIP Tests

- [x] [complex] Write `test/WIP/pipeline2-vtable-devirt.cpp`: a `Scan`/`Filter` virtual-method pattern (same as `virtual-methods.cpp`) with a `RUN:` line using `CRS_DEFAULT_PIPELINE=2`; FileCheck `EXE-NOT: load ptr, ptr %vtable` to confirm devirt. Include a P2-specific `EXE:` line confirming functional correctness.
- [x] [complex] Write `test/WIP/pipeline2-invariant-load.cpp`: a lambda-capture pattern (same as `invariant-load-to-constant.cpp`) with a `RUN:` line using `CRS_DEFAULT_PIPELINE=2`; FileCheck that the specialized output does not contain the invariant load — it has been resolved to a constant.
- [x] Run `ninja -C /home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/llvm/build/debug check-wip-runtime-specializer`; debug until both WIP tests pass
- [x] [complex] Debug strategy if vtable devirt fails: enable `CRS_LOG` at Debug level; inspect whether `JitSCCPInstVisitor::visitCallBase` extension fires (add a debug log); check if the function is in the module's tracked function set; verify `markConstant` was called for the vtable load; check if `FunctionSpecializer` picked up the resolved function pointer as a constant arg
- [x] [complex] Debug strategy if invariant-load fails: check if `resolveInvariantLoadToConstant` returns a value (add debug log); verify `mincore` succeeds for the pointer; confirm the load instruction has `!invariant.load` metadata in the JIT module

### Phase 8 — Smoke Test Integration

- [x] Add P2 `RUN:` lines to `test/smoke/virtual-methods.cpp`: `// RUN: %clangxx ... CRS_DEFAULT_PIPELINE=2 ... | FileCheck %s --check-prefix=P2-EXE` with `// P2-EXE-NOT: load ptr, ptr %vtable`
- [x] Add P2 `RUN:` lines to `test/smoke/virtual-methods-simple.cpp` (same pattern)
- [x] Add P2 `RUN:` lines to `test/smoke/invariant-load-to-constant.cpp` using existing `EXE` prefix or a `P2-EXE` prefix if FileCheck patterns differ
- [x] Add P2 `RUN:` lines to `test/smoke/call-specialized.cpp` (functional correctness only, no IR FileCheck)
- [x] Add P2 `RUN:` lines to `test/smoke/specialized-lambda-basic.cpp`
- [x] Add P2 `RUN:` lines to `test/smoke/specialized-lambda-equivalence.cpp`
- [x] Promote `test/WIP/pipeline2-vtable-devirt.cpp` → `test/smoke/pipeline2-vtable-devirt.cpp`
- [x] Promote `test/WIP/pipeline2-invariant-load.cpp` → `test/smoke/pipeline2-invariant-load.cpp`
- [x] Run `ninja check-smoke-runtime-specializer`; debug any failures

### Phase 9 — optimize.py Pipeline-Specific Domains

- [x] In `benchmarks/optimize_benchmarks.py`: change `pipeline` parameter from `bool` to `trial.suggest_categorical("pipeline", [0, 1, 2])` (or `suggest_int("pipeline", 0, 2)`)
- [x] [complex] Replace the flat `DEFAULT_SEARCH_SPACE` dict with a `sample_params(trial)` function:
  - Always-active params: `fixpoint_max`, `unroll_max`, `large_module_max`, `early_prune`, `o3_final`
  - `if pipeline == 1`: sample `p1_inline_threshold` (log_int, 50–2000), `p1_max_module_growth` (float, 1.0–5.0)
  - `if pipeline == 2`: sample `p2_min_func_size` (int, 1–100), `p2_max_clones` (int, 0–20), `p2_func_spec_iters` (int, 1–10), `p2_force_spec` (categorical, [0, 1])
  - Inactive pipeline params are not sampled (not added to trial env)
- [x] Update `_build_env(trial)` (or equivalent) to read from the new `sample_params` output and set the corresponding `CRS_*` env vars; ensure absent params produce no env var (not an empty string)
- [x] Verify a test Optuna dry-run: `python -c "import optimize_benchmarks; ..."` samples correct env vars for each pipeline choice

### Phase 10 — Verification

- [x] Run full smoke suite: `ninja -C /home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/llvm/build/debug check-smoke-runtime-specializer`
- [x] Run a quick benchmark comparison: `CRS_DEFAULT_PIPELINE=0` vs `CRS_DEFAULT_PIPELINE=2` on the `virtual_methods` use-case benchmark; record both results in DuckDB; confirm P2 JIT overhead is within 2× of P0 (SC-005)
- [x] Verify SC-003 (verbatim copy): `diff <(cat JitSCCP/JitSCCPSolver.cpp | grep -v '^// JIT-SCCP') <upstream-SCCPSolver.cpp>` produces no output (modulo the added comment header)
- [x] Run `optimize.py` dry-run with `--pipeline=2`; confirm `CRS_P2_*` vars appear and `CRS_DEFAULT_P1_*` vars are absent (SC-004)

### Phase 11 — Fix: propagate !invariant.load values across call boundaries

- [x] [complex] In `JitSCCP/JitSCCPSolver.cpp` `visitLoadInst`: after the existing `!invariant.load` block, add a second JIT-SCCP extension block — when the load's pointer has a constant SCCP lattice value, attempt a host memory read via `resolveInvariantLoadToConstant` (or a new `resolveLoadFromConstPtr` helper in `PointerChainResolver.h`); call `markConstant` on success. This enables the solver's internal worklist to propagate through load chains inside callee bodies after the wrapper's captured-struct pointer has been resolved and propagated into the callee argument.
- [x] In `JITPipelineIPSCCP.cpp` Phase 2: add `SROAPass` as an FPM before `JitIPSCCPPass` so stack-allocated vtable pointers are SSA-promoted before the solver runs — `MPM.addPass(llvm::createModuleToFunctionPassAdaptor(llvm::SROAPass(llvm::SROAOptions::ModifyCFG)))` — add `#include "llvm/Transforms/Scalar/SROA.h"` to includes.
- [x] Update `test/smoke/pipeline2-invariant-load.cpp`: add `// EXE-NOT: call{{.*}}test_invariant_load` and `// EXE: ret i32 10` between the existing `DEBUG: Optimized specialized function IR:` and `Result: 10` checks to assert the target was inlined and the return value folded, not merely that `!invariant.load` is absent from the wrapper.
- [x] Update `test/smoke/pipeline2-vtable-devirt.cpp`: add `// EXE-NOT: call ptr` before the existing `EXE-NOT: load ptr, ptr %vtable` line to assert indirect virtual calls were eliminated from the inlined wrapper IR, not just the vtable loads.
- [x] Run `ninja -C /home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/llvm/build/debug check-smoke-runtime-specializer`; debug until all smoke tests pass.
