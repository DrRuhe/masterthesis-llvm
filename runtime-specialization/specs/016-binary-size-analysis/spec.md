# Feature Specification: Binary Size Impact Analysis

**Feature Branch**: `015-binary-size-analysis`
**Created**: 2026-05-27
**Status**: Draft
**Input**: Measure the binary size overhead that JIT integration adds, broken down into IR dump contribution and JIT infrastructure contribution, using LIT tests on representative kernel samples.

## Overview

Adding runtime specialization to a binary increases its size through two mechanisms:

- **IR dump**: `IRDumpingPass` embeds a preprocessed bitcode copy of each compiled TU as a byte array (`RuntimeSpecializeableIR_data`). Size scales with target TU complexity.
- **JIT infrastructure**: Linking `ClangRuntimeSpecializer` (and its LLVM ORC/JIT dependencies) adds the JIT compilation engine. This is a fixed cost independent of target complexity.

Two representative LIT tests (small and large kernel) provide the measurements needed for the thesis without requiring changes to the full benchmark suite or CMake infrastructure.

---

## User Scenarios & Testing *(mandatory)*

### User Story 1 — Researcher obtains binary size breakdown for the thesis (Priority: P1)

A researcher runs two LIT tests and reads the printed metrics: full binary size, no-embed binary size (same binary but without IR dump), CRS library text section size, and bitcode blob size from the runtime API.

**Why this priority**: Core thesis result — needed to answer "what is the practical binary size cost of adding runtime specialization?"

**Independent Test**: `ninja check-smoke-runtime-specializer` passes both new tests; the captured output contains all four numeric metrics per test.

**Acceptance Scenarios**:

1. **Given** the LIT tests run, **When** the output file is inspected, **Then** `full_bytes > no_embed_bytes > 0` and `BitcodeSizeBytes > 0`.
2. **Given** the file-size difference `full_bytes − no_embed_bytes`, **When** compared to `BitcodeSizeBytes`, **Then** they agree within 25% (the file-size diff also includes registration globals and function name string arrays embedded alongside the blob).

---

### User Story 2 — IR dump size differs between small and large kernels (Priority: P2)

A researcher confirms that IR dump overhead scales with kernel complexity, not uniformly.

**Why this priority**: Supports the thesis argument that the IR dump is a proportional cost — not a fixed overhead.

**Independent Test**: Inspect `BitcodeSizeBytes` from both tests; the large-kernel value must be significantly higher.

**Acceptance Scenarios**:

1. **Given** both tests run, **When** their `BitcodeSizeBytes` values are compared, **Then** the large kernel produces a blob at least 2× the size of the small kernel's blob.

---

### Edge Cases

- The `full_bytes − no_embed_bytes` difference includes not just the raw bitcode blob but also registration globals (`RuntimeSpecializeableIR_ptr`, `RuntimeSpecializeableIR_len`, `RuntimeSpecializeableIR_funcs`) and the constructor that calls `clang_runtime_specializer_register_blob_v2`. This is documented as "IR embedding overhead" and will be slightly larger than `BitcodeSizeBytes` alone.
- `size -A` on the shared lib reports sections in the library before dynamic linking; this is a reasonable proxy for the linked contribution, slightly overestimating the actual contribution after linker dead-stripping.

---

## Requirements *(mandatory)*

**FR-001**: Two LIT test files MUST be added to `test/smoke/`: `binary-size-small.cpp` and `binary-size-large.cpp`.

**FR-002**: Each test MUST compile the same kernel source twice: once with `-fpass-plugin` (full, with IR dump) and once without (no-embed, JIT infra only).

**FR-003**: Each test MUST print `full_bytes`, `no_embed_bytes`, and `crs_lib_text_bytes` to a captured output file.

**FR-004**: Each test's compiled program (full variant) MUST call `getModuleStats()` and print `BitcodeSizeBytes`.

**FR-005**: FileCheck MUST verify all four metrics are present and contain at least one nonzero digit.

**FR-006**: Both tests MUST be auto-discovered by the existing `check-smoke-runtime-specializer` suite without CMake changes.

---

## Success Criteria *(mandatory)*

**SC-001**: Both tests pass under `ninja check-smoke-runtime-specializer`.

**SC-002**: `full_bytes > no_embed_bytes` in both tests.

**SC-003**: `BitcodeSizeBytes` for the large kernel is at least 2× the value for the small kernel.

**SC-004**: `full_bytes − no_embed_bytes` and `BitcodeSizeBytes` agree within 25%.

---

## Assumptions

- Benchmark binaries link `ClangRuntimeSpecializer` as a shared library (as configured in `test/lit.cfg.py` via `%clangxx`).
- `size` and `wc` (GNU coreutils/binutils) are available in the build environment.
- Analysis is performed post-build; no runtime performance measurements are needed.
- The two test files are self-contained (no external kernel headers needed).
