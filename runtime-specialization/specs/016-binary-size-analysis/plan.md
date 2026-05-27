# Implementation Plan: Binary Size Impact Analysis

**Branch**: `015-binary-size-analysis` | **Date**: 2026-05-27 | **Spec**: [spec.md](spec.md)

## Summary

Add two LIT smoke tests (`binary-size-small.cpp`, `binary-size-large.cpp`) that compile a representative kernel with and without `-fpass-plugin`, measure binary sizes with `wc -c`, measure CRS library text section with `size -A`, and cross-check the IR dump size against `getModuleStats().BitcodeSizeBytes`. No CMake changes to the benchmark suite.

## Technical Context

**Language/Version**: C++17 LIT test (existing format)
**Dependencies**: `%clangxx`, `%llvmshlibdir`, `%shlibext` (from `test/lit.cfg.py`); `wc`, `size`, `awk` (GNU coreutils/binutils)
**No new CMake targets**: tests auto-discovered by `check-smoke-runtime-specializer` suite
**No DuckDB integration**: numbers read directly from test output

## Constitution Check

| Principle | Status | Notes |
|-----------|--------|-------|
| Correctness & Safety | ✅ PASS | Read-only measurement; no binary modification |
| LLVM Coding Standards | ✅ PASS | Follows existing LIT test pattern from `ir-dump-preprocessing-blob-stats.cpp` |
| Test-First Validation | ✅ PASS | SC-004: cross-check BitcodeSizeBytes vs file-size diff |
| Performance Measurement | ✅ PASS | All four metrics printed and captured |
| Minimal Public API | ✅ PASS | No C++ API changes; uses existing `getModuleStats()` |
| Backwards Compatibility | ✅ PASS | Additive only; no existing files modified |
| DuckDB Schema | N/A | No DuckDB involvement |

## Measurement Strategy

Each test compiles the source **twice**:

| Variant | RUN command | Measures |
|---|---|---|
| **full** | `%clangxx -O3 -fpass-plugin=... %s -o %t.full.exe` | code + JIT infra + IR dump |
| **no-embed** | `%clangxx -O3 %s -o %t.noEmbed.exe` | code + JIT infra only |

From these:
- **IR dump overhead** = `wc -c %t.full.exe` − `wc -c %t.noEmbed.exe`
- **JIT infra size** = `.text` section of `libClangRuntimeSpecializer.so` (via `size -A`)
- **Cross-check** = `BitcodeSizeBytes` from `getModuleStats()` ≈ IR dump overhead (within 25%)

## Kernel Selection

**Small** (`binary-size-small.cpp`): a single tight loop with integer comparison — `count_above(data, n, threshold)`. Expected blob: ~10–50 KB. Closest to `mypow` in `ir-dump-preprocessing-blob-stats.cpp`.

**Large** (`binary-size-large.cpp`): several interconnected floating-point kernels (dot product, matrix-vector multiply, matrix multiply). Multiple function bodies in one TU produce a substantially larger blob. Expected blob: 3–10× larger than small.

Both files are fully self-contained (no `#include` beyond `ClangRuntimeSpecializer.h` and `<cstdio>`).

## LIT Test Pattern

Both tests follow the same structure. Output is collected into `%t.sizes`, then FileCheck validates it:

```
// RUN: %clangxx -O3 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext \
// RUN:   %s -o %t.full.exe
// RUN: %clangxx -O3 %s -o %t.noEmbed.exe
// RUN: wc -c < %t.full.exe    | awk '{printf "full_bytes: %d\n",     $1}' >  %t.sizes
// RUN: wc -c < %t.noEmbed.exe | awk '{printf "no_embed_bytes: %d\n", $1}' >> %t.sizes
// RUN: size -A %llvmshlibdir/libClangRuntimeSpecializer%shlibext \
// RUN:   | awk '/^\.text /{sum+=$2} END{printf "crs_lib_text_bytes: %d\n", sum}' >> %t.sizes
// RUN: %t.full.exe >> %t.sizes
// RUN: FileCheck %s < %t.sizes

// CHECK: full_bytes: {{[1-9][0-9]+}}
// CHECK: no_embed_bytes: {{[1-9][0-9]+}}
// CHECK: crs_lib_text_bytes: {{[1-9][0-9]+}}
// CHECK: BitcodeSizeBytes: {{[1-9][0-9]+}}
```

Program body (same in both):
```cpp
int main() {
    clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    auto stats = clangRuntimeSpecializer::ClangRuntimeSpecializer::getModuleStats();
    std::printf("BitcodeSizeBytes: %zu\n", stats.BitcodeSizeBytes);
    return 0;
}
```

## Files to Create

| File | Purpose |
|---|---|
| `test/smoke/binary-size-small.cpp` | Small kernel (1 function, simple loop) |
| `test/smoke/binary-size-large.cpp` | Large kernel (4+ functions, FP math) |

## Validation

```bash
# Run just the two new tests verbosely
llvm-lit test/smoke/binary-size-small.cpp test/smoke/binary-size-large.cpp -v

# Read the captured sizes file to see all four metrics
cat /tmp/lit-*/output/binary-size-small*/sizes  # path varies by LIT tmp dir

# Manual check:
# - full_bytes > no_embed_bytes  (IR dump was embedded)
# - crs_lib_text_bytes > 0       (JIT infrastructure measured)
# - BitcodeSizeBytes approx (full_bytes - no_embed_bytes) within 25%
# - BitcodeSizeBytes(large) >= 2 * BitcodeSizeBytes(small)

# Run full suite to confirm no regressions
ninja check-smoke-runtime-specializer
```

## Notes

- `size -A <shared_lib>.so` reports section sizes before dynamic linking. The actual linked contribution after dead-stripping may be smaller; this is documented in the thesis as an upper bound.
- The `.text` section of `libClangRuntimeSpecializer.so` covers the JIT engine code written in this project. The linked LLVM ORC/JIT code comes from `libLLVM.so` and is harder to isolate; it is not separately measured in these tests.
