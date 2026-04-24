# Quickstart: Adding a New JIT Pipeline

This guide walks through adding a third (or later) JIT compilation pipeline after spec 002 is implemented.

## Prerequisites

Spec 002 infrastructure is in place:
- `runtime/ClangRuntimeSpecializer/JITPipeline.h` defines `PipelineRunArgs`, `PipelineEntry`, and `getRegisteredPipelines()`.
- `runtime/ClangRuntimeSpecializer/JITPipelineRegistry.cpp` contains the static pipeline array.

## Steps

### 1. Create the pipeline implementation file

```
runtime/ClangRuntimeSpecializer/JITPipelineYourStrategy.cpp
```

Implement the pipeline function:

```cpp
#include "JITPipeline.h"
// include any pass headers needed

namespace clangRuntimeSpecializer {

llvm::Error runYourStrategyPipeline(PipelineRunArgs& Args) {
    // Args.Mod        — the module to transform
    // Args.Opts       — the active Options (read only)
    // Args.PB         — pre-built PassBuilder (use to create analysis managers)
    // Args.PIC        — pre-populated PassInstrumentationCallbacks (register your MPM against it)
    // Args.CurrentGroup      — set this before each logical phase for trace labels
    // Args.CurrentFixpointIter — set this inside fixpoint loops; reset to -1 after
    // Args.IsLargeModule — true if module exceeds Opts.LargeModuleInstrThreshold

    Args.CurrentGroup = "your-phase";
    llvm::ModulePassManager MPM;
    // ... add passes ...
    MPM.run(Args.Mod, /* MAM */);
    return llvm::Error::success();
}

} // namespace clangRuntimeSpecializer
```

### 2. Register the pipeline

Edit `JITPipelineRegistry.cpp` — add one line to the static array and include the new header:

```cpp
#include "JITPipelineYourStrategy.h"  // add

static const PipelineEntry kPipelines[] = {
    {"inlining",       runInliningPipeline},
    {"func-spec",      runFuncSpecPipeline},
    {"your-strategy",  runYourStrategyPipeline},  // add — its index is 2
};
```

### 3. Add new Options fields (if needed)

If the pipeline needs tunable parameters:

1. Add the field to `Options` in `ClangRuntimeSpecializer.h`:
   ```cpp
   int YourNewParam = 0;
   ```
2. Add a fluent builder:
   ```cpp
   Options& withYourNewParam(int V) { YourNewParam = V; return *this; }
   ```
3. Add the env var read in `Options::Default()`:
   ```cpp
   static const int kYourNewParam = (int)_envOr("CRS_DEFAULT_YOUR_NEW_PARAM", 0.0);
   O.YourNewParam = kYourNewParam;
   ```
4. Add a cross-pipeline incompatibility warning in `specializeOnlyImpl()` if the field is ignored by other pipelines.
5. Add the parameter to the built-in default descriptor in `optimize_benchmarks.py` and update `contracts/search-space-descriptor.md`.

### 4. Update CMakeLists.txt

Add the new source file to `runtime/ClangRuntimeSpecializer/CMakeLists.txt`:

```cmake
add_llvm_library(ClangRuntimeSpecializer SHARED
  ClangRuntimeSpecializer.cpp
  JITPipelineInlining.cpp
  JITPipelineFuncSpec.cpp
  JITPipelineYourStrategy.cpp   # add
  JITPipelineRegistry.cpp
  ...
)
```

### 5. Write a WIP test

```cpp
// test/WIP/pipeline-your-strategy.cpp
// RUN: %clangxx ... %s -o %t.exe
// RUN: CRS_DEFAULT_PIPELINE=2 %t.exe 1 | FileCheck %s
// CHECK: ...expected output...
```

Run: `ninja check-wip-runtime-specializer`

### 6. Update the descriptor

To make the new pipeline searchable by `optimize_benchmarks.py`, add it to the `pipeline` parameter's `choices` list in the built-in default descriptor (in `optimize_benchmarks.py`):

```python
{"name": "pipeline", "env_var": "CRS_DEFAULT_PIPELINE", "type": "categorical", "choices": [0, 1, 2]},
```

No other code changes to `optimize_benchmarks.py` are required.

## Files touched summary

| File | Change |
|---|---|
| `JITPipelineYourStrategy.cpp` | NEW — entire pipeline |
| `JITPipelineRegistry.cpp` | +1 include +1 array entry |
| `CMakeLists.txt` | +1 source file |
| `ClangRuntimeSpecializer.h` | +fields +env var read (only if new Options needed) |
| `ClangRuntimeSpecializer.cpp` | +warning for incompatible fields (only if new Options needed) |
| `optimize_benchmarks.py` | +pipeline index to `choices` list |
| `test/WIP/pipeline-your-strategy.cpp` | NEW |

Existing pipeline files (`JITPipelineInlining.cpp`, `JITPipelineFuncSpec.cpp`) are **not modified**.
