# Data Model: O3 Compilation Mode

## Pass Architecture

### IRRewritingPass (new)
- **Registered at**: `registerPipelineStartCallback` — runs before any -O3 optimisations
- **Input**: Module M in pre-optimisation state (call sites visible, function body shapes intact)
- **Reads**: All non-declaration functions in M for `isSpecializeLambdaUserCall` / `isSpecOnlyFuncPtrUserCall` patterns
- **Writes to M**:
  - `__crs_resolved_name_K` string globals (private, unnamed-addr)
  - Rewrites `specializeLambda(lambda)` → `specializeLambdaResolved(resolvedName, lambda)` call sites
  - Rewrites `callSpecialized(funcPtr, args)` → `callSpecializedResolved(resolvedName, funcPtr, args)` call sites
  - Named metadata `!crs.func_names`: one `!{!"mangledName"}` operand per collected function name
- **State passed to IRDumpingPass**: LLVM named metadata `!crs.func_names` (survives the optimisation pipeline)
- **Guard**: skips module if `!crs.rewriting_done` flag metadata already present (idempotency)

### IRDumpingPass (updated)
- **Registered at**: `registerOptimizerLastEPCallback` — runs after all -O3 optimisations
- **Input**: Module M in post-optimisation state
- **Reads from M**:
  - `!crs.func_names` metadata written by IRRewritingPass (function names for blob index)
  - All live functions (for cloning and serialisation)
- **Writes to M**:
  - `RuntimeSpecializeableIR_data` global (bitcode blob, existing)
  - `RuntimeSpecializeableIR_funcs` global (function name array, existing)
  - `RuntimeSpecializeableIR_ptr` / `_len` globals (existing)
  - `__clangRS_register_blob_<TU>` constructor function (existing)
- **Guard**: skips module if `RuntimeSpecializeableIR_data` global already present (existing, unchanged)

### Module Metadata Schema

```
!crs.func_names = !{!0, !1, ...}
!0 = !{!"_Z10myFunctionPi"}
!1 = !{!"_ZN6Filter8evaluateEPi"}
```

Each operand is an MDString containing the mangled function name. IRDumpingPass reads operands and reconstructs the `FuncNames` vector.

```
!crs.rewriting_done = !{!"1"}
```

Written at the end of IRRewritingPass to prevent double-rewriting if the pass runs twice.

## Source File Layout Changes

```
comptime/
├── IRDumpingPass.cpp        # Stripped: only cloning/serialisation/constructor-emit
├── IRRewritingPass.cpp      # New: call-site detection + rewriting + metadata emit
├── RuntimeSpecializerPass.h # Extended: add IRRewritingPass class declaration
└── PassPlugin.cpp           # Updated: register IRRewritingPass at pipeline start
```

Existing helper functions (`findLambdaProxyFunc`, `findLambdaArgIdx`, `findResolvedFuncInBody`, `findSpecOnlyResolvedInBody`, `getOrCreateIRDumpGlobals`, `isSpecializeLambdaUserCall`, `isSpecOnlyFuncPtrUserCall`, anonymous structs `SpecLambdaSite`, `SpecFuncPtrSite`) move to `IRRewritingPass.cpp` since they are only needed there.

The `getOrCreateIRDumpGlobals` helper stays in `IRDumpingPass.cpp` (only used for blob globals).

## Test File Changes

Each smoke test gains a paired -O3 `RUN:` line:

```
// RUN: %clangxx -g -O0 -fpass-plugin=... %s -o %t.o0.exe
// RUN: %t.o0.exe [args] | FileCheck %s --check-prefix=EXE
// RUN: %clangxx -g -O3 -fpass-plugin=... %s -o %t.o3.exe
// RUN: %t.o3.exe [args] | FileCheck %s --check-prefix=EXE
```

Where EXE patterns are identical at both levels (observable output is the same). Tests that differ at IR level (e.g., `instruction-count.cpp`) need a separate `-O3` prefix only if the *output value* differs.

## CMake Changes

### use-cases/*/CMakeLists.txt and benchmarks/CMakeLists.txt

Kernel TU compilation commands change from:
```cmake
COMMAND ${CLANG_EXE} -x c++ -g -O0 ... -fpass-plugin=${PLUGIN_LIB} ...
```
to:
```cmake
COMMAND ${CLANG_EXE} -x c++ -g -O3 ... -fpass-plugin=${PLUGIN_LIB} ...
```

This change applies to the `foreach(UC IN ITEMS UC1 UC2 UC7 UC8 UC12 UC14)` / kernel loop in the parent `benchmarks/CMakeLists.txt` and in each UC's own `CMakeLists.txt` (for the standalone UC benchmark targets).
