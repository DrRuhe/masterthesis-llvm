# Spec 014 — Details

## Current state (baseline)

- `runtime/ClangRuntimeSpecializer/JITPipelineFuncSpec.cpp`
  - Lines 19–78: linkage scrub (BFS reachable from wrapper → Internal,
    strip `AlwaysInline` from non-wrapper functions and call sites). Keep.
  - Line 81: `const int MaxFixpointIterations = 0;` — this hardcoded zero
    is the root cause of "P1 doesn't specialize". Change to read
    `Opts.MaxFixpointIterations`.
  - Lines 83–108: fixpoint loop body — currently runs only
    `ConstantArgFunctionSpecializationPass`. No clone-body cleanup, no
    inlining, no GlobalDCE. Replace per the plan below.
  - No final O3 / cleanup pass after the loop. Add a constrained final
    cleanup (not the full O3 pipeline).

- `runtime/ClangRuntimeSpecializer/ConstantArgFunctionSpecializationPass.cpp`
  - Case (A) in-place specialization, lines 90–100: replaces arg uses in F
    directly and strips `NoInline`/`OptimizeNone`. **Must NOT add
    `AlwaysInline`** (see [[feedback-pipeline1-budget-inlining]]).
  - Case (B) clone, lines 102–124: creates clone, sets InternalLinkage,
    strips NoInline/OptimizeNone, redirects call sites. **Must NOT add
    `AlwaysInline`**.

- `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h`
  - `Options::Default()` reads env vars at static init; add the two new
    P1-specific env vars here (`CRS_DEFAULT_P1_INLINE_THRESHOLD`,
    `CRS_DEFAULT_P1_MAX_MODULE_GROWTH`).
  - Add corresponding `Options` fields and fluent setters
    (`withP1InlineThreshold(int)`, `withP1MaxModuleGrowth(double)`).

- `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.cpp`
  - Lines 487–491: warning about `MaxFixpointIterations > 1` having no
    effect on the func-spec pipeline. Remove — this change makes it have
    an effect.

## Pipeline 1 design

### Phase 1: linkage scrub (unchanged)

Keep the existing block (`JITPipelineFuncSpec.cpp:19-78`). BFS from
wrapper → mark reachable functions Internal; strip `AlwaysInline` from
non-wrapper functions and call sites. Wrapper itself keeps its
`alwaysinline` attribute (it's the JIT entry, so this is a no-op).

### Phase 2: fixpoint loop

```cpp
const int MaxFixpointIterations = Opts.MaxFixpointIterations;
const size_t StartingInsts = countInstrs(M);
const size_t Cap = static_cast<size_t>(StartingInsts * Opts.P1MaxModuleGrowth);
size_t PrevInsts = 0;
unsigned MaxGroups = Opts.FuncSpecMaxGroups ? Opts.FuncSpecMaxGroups : 1;

for (int Iter = 0; Iter < MaxFixpointIterations; ++Iter) {
  CurrentGroup = "fixpoint"; CurrentFixpointIter = Iter;

  ModulePassManager MPM;

  // (1) clone the target with constants baked in
  MPM.addPass(ConstantArgFunctionSpecializationPass(MaxGroups, Pipeline1TargetFuncName));

  // (2) intra-clone cleanup — folds the just-substituted constants so the
  // inliner cost model sees a small body. Function-level passes guarded
  // behind !IsLargeModule per the spec 005 ValueHandle workaround.
  if (!IsLargeModule) {
    FunctionPassManager FPM;
    FPM.addPass(SROAPass(SROAOptions::ModifyCFG));
    FPM.addPass(EarlyCSEPass(/*UseMemorySSA=*/true));
    FPM.addPass(InstCombinePass());
    FPM.addPass(SimplifyCFGPass());
    // Loop rotate + bounded unroll. Unroll cap = Opts.LoopUnrollCount (this
    // is itself a budget knob).
    {
      LoopPassManager LPM;
      LPM.addPass(LoopRotatePass());
      FPM.addPass(createFunctionToLoopPassAdaptor(std::move(LPM),
                                                  /*UseMemorySSA=*/true));
    }
    LoopUnrollOptions UO;
    UO.setPartial(false).setRuntime(false).setUpperBound(true)
      .setFullUnrollMaxCount(Opts.LoopUnrollCount);
    FPM.addPass(LoopUnrollPass(UO));
    FPM.addPass(InstCombinePass());
    FPM.addPass(SimplifyCFGPass());
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
  }

  // (3) global cleanup — propagate constants across function boundaries
  // and prune dead originals. Skip on large modules per spec 005.
  if (!IsLargeModule) {
    MPM.addPass(IPSCCPPass(IPSCCPOptions(/*AllowFuncSpec=*/false)));
  }
  MPM.addPass(GlobalOptPass());
  MPM.addPass(GlobalDCEPass());

  // (4) budget-aware inlining of clones into the wrapper.
  // ModuleInlinerWrapperPass with custom InlineParams. Threshold from env.
  InlineParams Params = getInlineParams(Opts.P1InlineThreshold);
  MPM.addPass(ModuleInlinerWrapperPass(Params,
                                       /*MandatoryFirst=*/true,
                                       /*UseCGSCC=*/true));

  // (5) wrap-up: clean residual ops in the wrapper after inlining.
  if (!IsLargeModule) {
    FunctionPassManager Post;
    Post.addPass(SROAPass(SROAOptions::ModifyCFG));
    Post.addPass(InstCombinePass());
    Post.addPass(SimplifyCFGPass());
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(Post)));
  }
  MPM.addPass(GlobalDCEPass());

  MPM.run(M, MAM);

  // Convergence check + soft module-size cap.
  size_t Insts = countInstrs(M);
  if (Insts > Cap) {
    log(LogLevel::Warning,
        ("P1 module size cap exceeded: " + Twine(Insts) +
         " > " + Twine(Cap) + "; terminating fixpoint.").str());
    break;
  }
  if (Insts == PrevInsts) break;
  PrevInsts = Insts;
}
CurrentFixpointIter = -1;
```

### Phase 3: final cleanup (no full O3)

```cpp
CurrentGroup = "final";
ModulePassManager FinalMPM;
if (!IsLargeModule) {
  FunctionPassManager FPM;
  FPM.addPass(InstCombinePass());
  FPM.addPass(SimplifyCFGPass());
  FinalMPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
}
FinalMPM.addPass(GlobalDCEPass());
FinalMPM.run(M, MAM);
```

Note we deliberately do NOT run `PB.buildPerModuleDefaultPipeline(O3)` — O3
contains its own SCC inliner that ignores the P1 budget and would re-create
the explosion problem.

## New `Options` fields and env vars

In `ClangRuntimeSpecializer.h`, inside `struct Options`:

```cpp
int    P1InlineThreshold  = 225;   // env: CRS_DEFAULT_P1_INLINE_THRESHOLD
double P1MaxModuleGrowth  = 2.0;   // env: CRS_DEFAULT_P1_MAX_MODULE_GROWTH
```

Inside `Options::Default()`:

```cpp
static const int    kP1Thresh = (int)_envOr("CRS_DEFAULT_P1_INLINE_THRESHOLD", 225.0);
static const double kP1Growth = _envOr("CRS_DEFAULT_P1_MAX_MODULE_GROWTH", 2.0);
O.P1InlineThreshold = kP1Thresh;
O.P1MaxModuleGrowth = kP1Growth;
```

Fluent setters:

```cpp
Options& withP1InlineThreshold(int N)     { P1InlineThreshold = N; return *this; }
Options& withP1MaxModuleGrowth(double G)  { P1MaxModuleGrowth = G; return *this; }
```

## Test categorization (full list of `test/smoke/`)

### Bucket A — add P1 RUN, reuse existing FileCheck prefix

Output-only or log-only assertions; both pipelines produce the same text.

- `call-specialized.cpp` (no FileCheck currently — just append the RUN)
- `call-specialized-instance.cpp`
- `db-operators-modifies-argument-state.cpp`
- `instruction-count.cpp` — reuse `EXE-10`
- `o3-compilation-basic.cpp` — only O3 build; reuse `EXE`
- `pure-specialized-method-is-equivalent.cpp`
- `specialized-lambda-basic.cpp`
- `specialized-lambda-complex-no-funcname.cpp`
- `specialized-lambda-equivalence.cpp`
- `specialized-lambda-no-captures.cpp`
- `specialized-lambda-no-captures-no-funcname.cpp`
- `specialized-lambda-raii.cpp`
- `specialized-lambda-void.cpp`
- `specialized-lambda-zero-arg-no-funcname.cpp`
- `specialized-method-is-equivalent.cpp`
- `speconly-funcptr.cpp`
- `zero-arg-lambda-equals-speconly.cpp`

### Bucket B — add P1 RUN, may need separate `P1-EXE` prefix

IR-strict tests. Try shared `EXE` first; only split prefixes if FileCheck
fails on P1 output. With the default budget, small kernels should still
fold to `ret <const>` under P1.

- `pure-specialized-function-is-equivalent.cpp` — expects `ret i32 9`
- `specialized-function-is-equivalent.cpp` — expects `store i32 9; ret void`
- `struct-arg-hide.cpp` — expects `ret i32 102`

### Bucket C — leave untouched

Already covered, or depend on P0-only passes, or are pipeline-agnostic.

- **Already have P1 RUN lines**: `pipeline-dispatch.cpp`,
  `funcspec-max-groups-env.cpp`, `pipeline-invalid-index.cpp`
- **Pipeline-agnostic** (compile-time IR-dump / lit `.ll`):
  `ir-dump-deterministic.cpp`, `ir-dump-preprocessing-blob-stats.cpp`,
  `static-mutability-analysis.ll`
- **Plugin / error-path** (no JIT execution): `missing-plugin-funcptr-error.cpp`,
  `missing-plugin-lambda-error.cpp`, `speconly-runtime-funcptr-error.cpp`
- **Unrelated to pipeline**: `options-env-vars.cpp`
- **P0-only passes** (devirt / invariant-load / static-mutability):
  `invariant-load-to-constant.cpp`, `read-only-inference.cpp`,
  `static-analysis-robust.cpp`, `virtual-methods.cpp`,
  `virtual-methods-simple.cpp`

`test/WIP/*` is intentionally out of scope.

## RUN-line pattern to add

For tests with both O0 and O3 builds (most cases):

```c
// RUN: CRS_DEFAULT_PIPELINE=1 %t.exe | FileCheck %s --check-prefix=EXE --dump-input=always
// RUN: CRS_DEFAULT_PIPELINE=1 %t.o3.exe | FileCheck %s --check-prefix=EXE --dump-input=always
```

For tests with only one binary:

```c
// RUN: CRS_DEFAULT_PIPELINE=1 %t.exe | FileCheck %s --check-prefix=EXE
```

For `call-specialized.cpp` (no FileCheck today):

```c
// RUN: CRS_DEFAULT_PIPELINE=1 %t.exe 2
```

## Risks / gotchas

1. **Spec-005 ValueHandle bug**: any `FunctionPassManager` adaptor on
   debug builds for large modules can trip `pImpl->ValueHandles`
   corruption. Mirror P0's `!IsLargeModule` gates around the FPM blocks
   and `IPSCCPPass` (with `AllowFuncSpec=false`, same as P0).
2. **`Pipeline1TargetFuncName` detection failure**: the current heuristic
   walks the wrapper for a unique non-declaration callee. Lambda wrappers
   call `operator()` — should still work. If the wrapper somehow ends up
   with multiple internal callees (e.g. after some other pass), the
   func-spec pass becomes a no-op and the wrapper falls back to calling
   the unspecialized target. That degrades P1 to "correct-but-slow" rather
   than "broken" — acceptable.
3. **Bucket B IR-shape divergence**: P1's cost-based inliner may pick a
   different inlining point than P0's `AlwaysInlinerPass` and produce
   subtly different IR around the `ret`. Mitigation: try shared `EXE`
   prefix first; split to `P0-EXE`/`P1-EXE` and relax P1 checks (e.g.
   only assert the final constant in the `ret`, not the full block
   layout) if needed.
4. **`ModuleInlinerWrapperPass` constructor signature** varies across LLVM
   versions. Validate against the in-tree LLVM headers at
   `llvm/include/llvm/Transforms/IPO/Inliner.h`. If the public API isn't
   ergonomic, fall back to building a manual CGSCC inliner pipeline:
   `MPM.addPass(createModuleToPostOrderCGSCCPassAdaptor(InlinerPass(Params)))`.
5. **`getInlineParams(int Threshold)` factory** is the cleanest way to
   populate `InlineParams` — defined in `llvm/Analysis/InlineCost.h`. It
   sets `DefaultThreshold` and leaves the hint/cold thresholds at their
   library defaults.
6. **No big-kernel test in the smoke suite covers SC-003** directly. We
   don't add one in this spec (would require a synthetic large-kernel
   harness); SC-003 is verified by inspection on the polybench/sqlite
   benchmarks separately.

## Constitution refs

- Spec 005 (debug-mode large-module fix): respect the `!IsLargeModule`
  guards around IPSCCP and FPM passes.
- Spec 002 (JIT pipeline infrastructure): pipeline dispatch and env-var
  override patterns established here.
- `LINKING-ISSUES.md`: documents the sqlite3 static-locals / dlsym
  problems. Out of scope for spec 014 — only matters once P1 is wired up
  against the sqlite benchmark, not for the smoke tests.
