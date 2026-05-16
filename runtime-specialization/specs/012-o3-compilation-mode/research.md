# Research: O3 Compilation Mode

## Root Cause: IRDumpingPass Ordering vs -O3 Pipeline

**Decision**: The core problem is that `IRDumpingPass` is registered via `registerOptimizerLastEPCallback` (`PassPlugin.cpp:35`), meaning it runs **after all -O3 optimisations** have already executed.

The pass's call-site detection traversal (`findLambdaProxyFunc`, `findLambdaArgIdx`, `findResolvedFuncInBody`, `findSpecOnlyResolvedInBody`) relies on unoptimised IR shape:

- It walks the body of each `specializeLambda` instantiation looking for a call to `forceLambdaOpEmit`
- Inside `forceLambdaOpEmit` it looks for a call to `__crs_op_hint(arg0)` to read the proxy pointer
- At -O3, DAE (Dead Argument Elimination) may strip the arguments of `forceLambdaOpEmit` (both args are provably unused), changing the call signature seen by the traversal

The existing workaround (from `benchmarks/CMakeLists.txt`) confirms this: *"Kernel TUs compile at -O0 (prevents DAE/inlining from hiding the specializeLambda call from IRDumpingPass)."*

**Rationale**: The call-site rewriting work (Phases 0/0b/N/N2 in `IRDumpingPass.cpp`) requires the pre-optimisation IR shape. The bitcode serialisation work (cloning, linkage fixup, GlobalDCE, writing) operates correctly on post-optimisation IR. Separating these concerns into two registration points eliminates the ordering problem.

**Alternatives considered**:
- Mark `forceLambdaOpEmit` args with `__attribute__((used))` / escape them — fragile, compiler-specific
- Register the whole pass at pipeline start — bitcode blob would capture pre-optimisation IR, meaning the JIT receives unoptimised code; acceptable but wasteful
- Add `optnone` to all specializer-infrastructure functions — would prevent -O3 from optimising the user-side call site

---

## Current State: What Already Works at -O3

**Decision**: Benchmark compilation is already largely at -O3; only the use-case kernel TUs remain at -O0.

From `benchmarks/CMakeLists.txt`:
- `SyntheticKernels.cpp` → `-O3`
- `SpecializerBenchmark.cpp` → `-O3`
- `DBOperatorsBenchmark.cpp` → `-O3`
- `polybench_bench.cpp` → `-O3`
- TPC-H and DuckDB TPC-H → `-O3`
- UC kernel TUs (`UC1`–`UC14`) → **`-O0`** (the ones needing the fix)
- UC benchmark registration TUs → `-O3` (already correct)

**Rationale**: Benchmark TUs (registration code) don't contain `specializeLambda` call sites that IRDumpingPass needs to detect — only kernel TUs do. So the existing -O3 benchmark code works because the plugin's detection logic doesn't need to run on those TUs.

---

## Fix Strategy: Two-Pass Split

**Decision**: Split `IRDumpingPass` into two co-operating passes registered at different pipeline points.

| Pass | Registration point | Responsibility |
|------|--------------------|----------------|
| `IRRewritingPass` | `registerPipelineStartCallback` (before any optimisation) | Phase 0/0b call-site detection; Phase N/N2 call-site rewriting; function-name collection → module metadata |
| `IRDumpingPass` (updated) | `registerOptimizerLastEPCallback` (after all optimisation) | Read function names from metadata; clone + preprocess + GlobalDCE + serialize → bitcode blob; emit `register_blob_v2` constructor |

State is shared between the two passes via LLVM named metadata (`!crs.func_names` nodes written by `IRRewritingPass` and read by `IRDumpingPass`). This is idiomatic LLVM: named metadata survives the optimisation pipeline intact.

**Rationale**: This is the minimal structural change that fixes the ordering problem without altering the blob content (which is still captured post-optimisation). It requires no changes to the public API or to the runtime.

**Alternatives considered**:
- Single-pass registration at pipeline start: would capture pre-O3 IR in blob; JIT would receive more redundant code → slower JIT compilation
- Module flags / custom attributes to carry state: more complex than named metadata; attributes on functions can be removed by passes

---

## Test Strategy: Per-Test -O3 RUN Lines

**Decision**: For each smoke test that currently has a `-O0 -fpass-plugin=...` RUN line, add a paired `-O3 -fpass-plugin=...` RUN line immediately below it. FileCheck patterns are shared where possible; where -O3 causes structural IR differences, a separate `-O3` check-prefix is used.

**Rationale**: 
- A global lit substitution for the optimisation level would couple all tests together and make failures harder to diagnose
- Separate RUN lines per test keep failures isolated and the intent explicit
- Adding paired lines (not replacing) preserves the -O0 coverage as a regression baseline

Tests that do *not* involve the comptime plugin (`missing-plugin-funcptr-error`, `missing-plugin-lambda-error`) need no -O3 RUN line as they test error-reporting paths that are plugin-independent.

**Alternatives considered**:
- Parameterise via a `%opt_level` substitution in `lit.cfg.py`: adds configuration complexity; doesn't help with FileCheck pattern differences
- Run both levels via a shell loop in the RUN line: unusual pattern in LLVM lit; harder for tooling to parse

---

## Benchmark Kernel TUs: Switch to -O3 Post-Split

**Decision**: After the rewriting pass is moved to pipeline start, UC kernel CMake commands can be changed from `-O0` to `-O3`. This must happen in the same commit as the pass split, not before.

**Rationale**: Changing kernel TUs to -O3 before the pass is fixed will cause call-site detection to fail, breaking the JIT blob. The order of implementation is: (1) split passes, (2) add -O3 tests, (3) switch UC kernel TUs.

---

## No Public API Surface Change

**Decision**: The split is internal to `comptime/`. `ClangRuntimeSpecializer.h`, the benchmark helpers, and all callsites are unchanged.

**Rationale**: Constitution principle V (minimal public API) and backwards-compatibility rule prohibit adding external-facing surface for an internal refactoring.
