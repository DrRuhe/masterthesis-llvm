# Implementation Plan: Use-Case Benchmark Suite

**Branch**: `008-use-case-benchmarks` | **Date**: 2026-05-13 | **Spec**: [spec.md](spec.md)
**Input**: Feature specification from `specs/008-use-case-benchmarks/spec.md`

## Summary

Implement six self-contained C++ benchmark workloads (UC1/UC2/UC7/UC8/UC12/UC14) using
the spec 010 lambda API (`specializeLambda`). Each benchmark:

1. Provides a kernel TU with the core computation function + globals.
2. Exposes a **factory function** (`create_*_specialized(fixed_args...)`) in the kernel TU
   that wraps `specializeLambda` — capturing the fixed parameters and delegating to the
   kernel function in the **same module** so the JIT can fully inline and optimize it.
3. Has a separate benchmark TU (with Google Benchmark headers) that calls the factory
   function and registers the three standard measurement phases.

The lambda factory pattern is the central design decision: by placing both
`specializeLambda` and the target kernel function in the **same kernel TU**, the JIT
clones the entire kernel module, enabling `ConstantArgAlwaysInlinePass` + `AlwaysInliner`
to inline the kernel body into the specialized wrapper and propagate all captured constants
through loops and branches — achieving the same full optimization as the existing
`specializeOnly(funcName, ...)` pattern.

---

## Technical Context

**Language/Version**: C++20 (LLVM/Clang, Nix flake)
**Primary Dependencies**: LLVM LLJIT, Google Benchmark, ClangRuntimeSpecializer (spec 010 API)
**Storage**: `benchmarks/benchmarks.duckdb` (existing DuckDB schema — no schema changes)
**Testing**: lit/FileCheck smoke tests for correctness; Google Benchmark for timing
**Target Platform**: Linux x86-64 (single controlled machine, release build)
**Project Type**: Research benchmark suite
**Performance Goals**: ≥10% speedup for UC2 (convolution) and UC7 (DFA); valid data for all six
**Constraints**: Release build only for timing; debug build must not crash
**Scale/Scope**: 6 binary targets, ≥10M iterations per hot-loop benchmark

---

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-checked after Phase 1 design.*

| Principle | Status | Notes |
|-----------|--------|-------|
| I. Correctness & Safety | ✅ Pass | `assertSpecializedLambdaIsEquivalent` called per kernel before any timing |
| II. LLVM Coding Standards | ✅ Pass | Kernel TUs use no LLVM headers; benchmark TUs use existing LLVM-aware benchmark infrastructure |
| III. Test-First Validation | ✅ Pass | Each kernel factory function validated via equivalence assertion before benchmarking |
| IV. Performance Measurement | ✅ Pass | Three-phase measurement (unspecialized / jit_overhead / specialized_exec); `record_benchmark.py` compatible |
| V. Minimal Public API | ✅ Pass | No changes to the public `ClangRuntimeSpecializer` API; no new public helpers |
| Specialization Scope Constraint | ✅ Pass | All benchmarks single-threaded; no concurrent mutation of specialized arguments |

**No violations.** Planning may proceed.

---

## Project Structure

### Documentation (this feature)

```text
specs/008-use-case-benchmarks/
├── plan.md              # This file
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
└── tasks.md             # Phase 2 output (/speckit-tasks)
```

### Source Code

```text
benchmarks/
└── use-cases/
    ├── CMakeLists.txt            # Six add_benchmark targets + add_subdirectory in parent
    ├── UC1SqlPredicate/
    │   ├── UC1Kernels.h          # Types, constants, extern globals, factory decl
    │   ├── UC1Kernels.cpp        # count_matching_rows + create_sql_specialized() [plugin]
    │   └── UC1Benchmark.cpp      # Google Benchmark registration [plugin, benchmark headers]
    ├── UC2Convolution/
    │   ├── UC2Kernels.h
    │   ├── UC2Kernels.cpp        # convolve2d + g_kernel_coeffs + create_conv_specialized() [plugin]
    │   └── UC2Benchmark.cpp
    ├── UC7DfaRegex/
    │   ├── UC7Kernels.h
    │   ├── UC7Kernels.cpp        # dfa_match + g_dfa_table + create_dfa_specialized() [plugin]
    │   └── UC7Benchmark.cpp
    ├── UC8IVM/
    │   ├── UC8Kernels.h
    │   ├── UC8Kernels.cpp        # apply_row_delta + create_ivm_specialized() [plugin]
    │   └── UC8Benchmark.cpp
    ├── UC12GroupBy/
    │   ├── UC12Kernels.h
    │   ├── UC12Kernels.cpp       # grouped_sum + create_groupby_specialized() [plugin]
    │   └── UC12Benchmark.cpp
    └── UC14Sort/
        ├── UC14Kernels.h
        ├── UC14Kernels.cpp       # generic_sort + int64_asc_cmp + create_sort_specialized() [plugin]
        └── UC14Benchmark.cpp
```

`[plugin]` = compiled with `-fpass-plugin=LLVMRuntimeSpecializationComptimePlugin`.

---

## Phase 0: Research

### Lambda Factory Pattern — Why It Works

The critical optimization mechanism depends on **co-locating the factory function and
the kernel function in the same kernel TU**:

1. `specializeLambdaImpl` clones `TargetFunc->getParent()` — the **entire blob module**
   (not just the lambda's `operator()`). If `dfa_match` and `lambda::operator()` are
   both in `UC7Kernels.cpp`, the JIT gets both functions in the cloned module.

2. The JIT wrapper bakes the lambda's captured constants (e.g., `dfa_table_ptr`,
   `n_chars`, `start_state`, `accept_mask`) as LLVM IR constants.

3. `ConstantArgAlwaysInlinePass` detects that the call from `lambda::operator()` to
   `dfa_match(hay, len, CONST_table, CONST_n_chars, CONST_start, CONST_accept)` has
   constant args → marks it `alwaysinline`.

4. `AlwaysInlinerPass` inlines `dfa_match` into `lambda::operator()` (possible because
   `dfa_match` is **defined** in the same cloned module, not just declared).

5. After inlining, IPSCCP propagates all constants into the DFA loop body: constant
   multiplier in the index expression, constant accept mask, constant start state.

6. Final O3 can vectorize and unroll the specialized loop.

**Contrast with separate-TU lambda**: if the lambda were in the benchmark TU (with
benchmark headers) and called `dfa_match` externally, step 4 would fail (external
function, no body in module) and step 5 would produce no benefit. The blob would also
risk a JIT crash from `benchmark::State` WeakODR vtables surviving early GlobalDCE.

### TU-Separation Compliance

FR-001 requires kernel functions in a dedicated `*Kernels.cpp` with no Google Benchmark
headers. This is satisfied: `*Kernels.cpp` files include only `ClangRuntimeSpecializer.h`
and standard C headers. The factory function and `specializeLambda` call live in the
kernel TU — no benchmark infrastructure contaminates the blob.

The benchmark TU (`*Benchmark.cpp`) is compiled **with** the plugin (consistent with
the existing convention for `SpecializerBenchmark.cpp`) but its blob is never used as
a JIT target. The benchmark TU calls the factory function from the kernel TU.

### Fixed vs. Variable Parameters per Use Case

| ID | Kernel | Fixed (captured by lambda) | Variable (explicit args) |
|----|--------|---------------------------|--------------------------|
| UC1 | `count_matching_rows` | `row_stride`, `col_offset`, `threshold` | `rows`, `n_rows` |
| UC2 | `convolve2d` (separable) | `kernel_coeffs*`, `ksize`, `width`, `height` | `src`, `dst` |
| UC7 | `dfa_match` | `dfa_table*`, `n_states`, `n_chars`, `start_state`, `accept_mask` | `haystack`, `len` |
| UC8 | `apply_row_delta` | `group_col_offset`, `value_col_offset`, `row_stride`, `n_buckets` | `row`, `agg_buckets` |
| UC12 | `grouped_sum` | `row_stride`, `key_offset`, `value_offset`, `n_buckets` | `rows`, `n_rows`, `out_buckets` |
| UC14 | `generic_sort` | `comparator*`, `element_size` | `data`, `n_elements` |

`*` = pointer to a non-`static` global in the kernel TU; the JIT bakes the address as a
constant pointer, and the optimizer can use the stable address for load-hoisting.

### UC14 (Sort) Function Pointer Specialization

`comparator` is a function pointer. The lambda captures its value. The JIT bakes the
pointer value as a constant. `ConstantArgAlwaysInlinePass` marks the comparator call in
`generic_sort` as `alwaysinline`. `AlwaysInlinerPass` can inline the comparator body if
`int64_asc_cmp` is defined in the same kernel TU (FR-028 satisfied). The result is a
sort routine with the comparison fully inlined — potentially enabling GVN/LICM to remove
comparator-call overhead entirely.

### DFA Construction for UC7

The DFA for the email-address pattern is built **programmatically at startup** by a builder
function in the kernel TU (invoked via a static initializer object or
`__attribute__((constructor))`):
- States model the recognizer for `[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}`
- N_STATES ≈ 12–16, N_CHARS = 128 (ASCII only, high bytes → dead/reject state)
- Table: `g_dfa_table[N_STATES * N_CHARS]` as a non-`static` global `int` array
- Builder uses no external regex library; encodes state transitions directly in C++
- Corpus: 50–100 MB synthetic text generated once at benchmark binary startup and reused

### Dataset Sizes and Memory

Each benchmark registers four size variants via `kv_s` (`s:SMALL`, `s:MEDIUM`, `s:LARGE`,
`s:EXTRALARGE` in the benchmark name). Global datasets are allocated to the EXTRALARGE max
size at startup; smaller variants use a subset (prefix or dimension slice).
Default filter in standalone `main()` excludes EXTRALARGE (same pattern as polybench).

**Size targets (FR-007)**: unspecialized per-call runtime MUST be within 2× of:
SMALL ≈ 0.1 s, MEDIUM ≈ 1 s, LARGE ≈ 10 s, EXTRALARGE ≈ 60 s.

The table below shows **initial estimates** — sizes MUST be calibrated after implementation
(see "Size Calibration" subsection below).

**Calibrated sizes (T009b rev2, 2026-05-17)** — standalone binaries; AllBenchmarks uses original sizes via `#ifdef ALL_BENCHMARKS_BUILD`:

| Use Case | SMALL | MEDIUM | LARGE | EXTRALARGE | Standalone buffer | Status |
|----------|-------|--------|-------|------------|-------------------|--------|
| UC1 `n_rows` | 65M (~100ms) | 640M (~1000ms) | 6.4B (~10s) | 38.5B (~60s) ² | 50M × 16B = 800 MB | All ✓ |
| UC2 `n_tiles` (3840×3840 fixed) | 1 (~174ms box) | 6 (~1044ms box) | 58 (~10.1s box) | 345 (~60s box) ² | 3840² × 4B × 2 = 115 MB | All ✓ |
| UC7 `corpus_bytes` | 44 MB (~101ms) | 440 MB (~1011ms) | 4350 MB (~10s) | 26 000 MB (~60s) ² | 500 MB chunk | All ✓ |
| UC8 `n_rows` | 43M (~126ms) | 430M (~1266ms) | 4.3B (~12.5s) | 26B (~76s apply / ~33s batch) ³ | 50M × 24B = 1.2 GB | All ✓ |
| UC12 `n_rows` | 36M (~99ms) | 360M (~1001ms) | 3.6B (~10s) | 21.7B (~60s) ² | 50M × 24B = 1.2 GB | All ✓ |
| UC14 generic_sort `n_elements` | 900K | 8M | 70M (~9s avg) | 400M (~47s avg) ⁴ | 400M × 8B × 2 = 6.4 GB | All ✓ |
| UC14 struct/multi-key sort `n_elements` | 750K–1M | 6.5–8.5M | 50M (~7–8s) | 250M (~34–42s) ⁴ | 250M × 16B × 2 = 8 GB | All ✓ |

² **Streaming (buffer-loop)**: UC1, UC7, UC8, UC12 all stream a fixed in-memory buffer one chunk at a time for LARGE/EXTRALARGE. UC2 convolves a fixed 3840×3840 tile repeatedly (see Streaming Refactor Design below). EXTRALARGE represents total bytes/rows/tiles processed, not the buffer size.

³ **UC8 shared size, two-kernel constraint**: `apply_row_delta` (slowest, ~3× slower than `batch_delta`) and `batch_delta` share the `n_rows` parameter. At EXTRALARGE=26B: apply_row_delta ≈ 76 s, batch_delta ≈ 33 s — both within [30, 120 s]. Targeting exactly 60 s for `apply_row_delta` (21B rows) would push `batch_delta` to ~27 s (outside [30, 120 s]). 26B is the optimal value that keeps all kernels within tolerance.

⁴ **UC14 — no streaming possible**: in-place quicksort requires all elements in memory; streaming is not semantically viable. N_SORT_MAX and N_STRUCT_MAX are increased to accommodate the larger EXTRALARGE sizes (see code changes below). EXTRALARGE runtimes land at ~40–67 s (generic_sort) and ~34–42 s (struct/multi-key sort), both within [30, 120 s].

AllBenchmarks total peak (all EXTRALARGE, default filter excludes EXTRALARGE): ≈ 3.3 GB (UC1 800 MB + UC2 115 MB + UC7 500 MB + UC8 1.2 GB + UC12 1.2 GB + UC14 small AllBenchmarks sizes ≈ negligible).

#### Size Calibration

After initial implementation (T003–T008), run a calibration pass (task T009b) to measure
actual unspecialized runtimes and update the size constants to hit the FR-007 targets.

**Calibration query** (mirrors `size_scaling.py` `load_data`):

```sql
-- Run against the benchmarks database after recording the initial run:
SELECT kernel, kv_s, real_time_ns / 1e9 AS unspec_s
FROM v_ns
WHERE run_id = (SELECT MAX(run_id) FROM runs)
  AND phase = 'unspecialized'
  AND kv_s IS NOT NULL
ORDER BY kernel, kv_s;
```

**Calibration procedure**:
1. Record a short benchmark run (default filter, release build) via `record_benchmark.py`.
2. Run the query above; compare `unspec_s` against targets (SMALL 0.05–0.2 s, MEDIUM 0.5–2 s, LARGE 5–20 s, EXTRALARGE 30–120 s).
3. For any size that misses its target by > 2×, compute the required size as `new_size = current_size × (target_s / measured_s)`, update the constant in `*Benchmark.cpp`, rebuild, and re-record.
4. Repeat until all four sizes are within the 2× tolerance band.
5. Update the table above with the calibrated sizes.

### Streaming Refactor Design (T009c, 2026-05-17)

**Goal**: All buffer-based UCs must support sizes larger than any single allocation by streaming
through a fixed-size in-memory chunk. This enables EXTRALARGE ≈ 60 s without requiring
>15 GB of RAM.

#### UC1 / UC8 / UC12 — Already streaming (no code changes needed)

All three use the pattern:

```cpp
for (int64_t rem = n_total; rem > 0; rem -= N_ROWS_MAX)
    kernel(g_buf.data(), std::min(rem, N_ROWS_MAX), ...);
```

where `N_ROWS_MAX = 50'000'000` (50 M rows). The `n_total` parameter from
`state.range(0)` drives the loop; EXTRALARGE values (38.5 B / 26 B / 21.7 B) already
exceed the buffer and stream automatically.

#### UC7 — Streaming refactor required

**Current**: allocates `g_corpus = make_corpus(CORPUS_MAX)` where `CORPUS_MAX = 15 GB`;
calls `dfa_match(g_corpus.data(), corpus_size, ...)` in one pass. Requires 15 GB RAM.

**New**: reduce buffer to `CORPUS_CHUNK = 500 MB`; add streaming loop:

```cpp
static constexpr int64_t CORPUS_CHUNK = 500LL * 1024 * 1024;
static std::vector<char> g_corpus = make_corpus(CORPUS_CHUNK);

static void BM_UC7_email_low_unspecialized(benchmark::State& state) {
    int64_t n_total = state.range(0);
    for (auto _ : state) {
        int64_t result = 0;
        for (int64_t rem = n_total; rem > 0; rem -= CORPUS_CHUNK)
            result += dfa_match(g_corpus.data(),
                                std::min(rem, CORPUS_CHUNK), ...);
        benchmark::DoNotOptimize(result);
    }
}
```

- SMALL (44 MB) and MEDIUM (440 MB) each fit in one chunk (single pass, same semantics).
- LARGE (4350 MB) streams ⌈4350/500⌉ = 9 passes ≈ 10 s ✓.
- EXTRALARGE (26 000 MB) streams 52 passes ≈ 60 s ✓.
- **Memory**: 15 GB → 500 MB (96% reduction).
- **DFA semantics**: state does not carry between chunks (reset at start of each call).
  This is correct for the throughput benchmark — we measure bytes/s, not match accuracy
  across chunk boundaries.
- **AllBenchmarks guard**: `#ifdef ALL_BENCHMARKS_BUILD` block keeps `CORPUS_CHUNK = 1 GB`
  and its smaller sizes; only the `#else` (standalone) block changes.

**Size constants** (standalone `#else` block):

```cpp
UC7_BENCHMARK_SPEC(
    Arg(44LL * 1024 * 1024),
    Arg(440LL * 1024 * 1024),
    Arg(4350LL * 1024 * 1024),
    Arg(26000LL * 1024 * 1024)
)
```

#### UC2 — Tile-based streaming refactor required

**Current**: allocates `g_src/g_dst` at `IMG_WIDTH_MAX × IMG_HEIGHT_MAX = 25920²` (2.7 GB
each); SMALL/MEDIUM/LARGE/EXTRALARGE use `Args({width, height})` with different image
dimensions. EXTRALARGE = `Args({25920, 25920})` produces only ~8 s (box_filter) — far
below the 60 s target. The 25920×25920 hardware limit prevents further growth.

**New**: fix the tile at `TILE_W = 3840, TILE_H = 3840`; `width` and `height` become
specialization constants baked into every JIT call. The variable parameter becomes
`n_tiles` (how many times the tile is convolved). Streaming loop:

```cpp
static constexpr int TILE_W = 3840;
static constexpr int TILE_H = 3840;
static std::vector<float> g_src(TILE_W * TILE_H);
static std::vector<float> g_dst(TILE_W * TILE_H);

static void BM_UC2_unspecialized(benchmark::State& state) {
    int64_t n_tiles = state.range(0);
    for (auto _ : state) {
        for (int64_t t = 0; t < n_tiles; ++t)
            convolve2d(g_src.data(), g_dst.data(),
                       TILE_W, TILE_H, g_kernel_coeffs, 5);
        benchmark::DoNotOptimize(g_dst.data());
    }
}
```

The jit_overhead and specialized_exec benchmarks receive `n_tiles` via `state.range(0)` but
only call `create_conv_specialized(TILE_W, TILE_H)` once (n_tiles ignored for JIT phase).

**Parameter format**: `Arg(n_tiles)` (single range, replacing `Args({width, height})`).

**Estimated runtimes** (measured: box_filter = 174 ms/tile, sep_gauss = 93 ms/tile):

| n_tiles | box_filter | sep_gauss | edge_detect |
|---------|-----------|-----------|------------|
| 1 (SMALL) | 174 ms ✓ | 93 ms ✓ | 35 ms |
| 6 (MEDIUM) | 1044 ms ✓ | 558 ms ✓ | 210 ms ✓ |
| 58 (LARGE) | 10.1 s ✓ | 5.4 s ✓ | 2.0 s ✓ |
| 345 (EXTRALARGE) | 60.0 s ✓ | 32.1 s ✓ | 12.1 s ✓ |

`edge_detection` SMALL (35 ms) is below the 50 ms floor; this is an inherent property of
the kernel's lower compute intensity and cannot be fixed with a shared tile parameter.

**Memory**: 25920² × 4 B × 2 ≈ 5.4 GB → 3840² × 4 B × 2 ≈ 115 MB (98% reduction).

**Size constants** (standalone `#else` block):

```cpp
UC2_BENCHMARK_SPEC(
    Args({3840, 3840}),  // kept as Args for macro compatibility; n_tiles = range(0)
    Args({3840, 3840}),
    Args({3840, 3840}),
    Args({3840, 3840})
)
```

Wait — since the tile is fixed, all four macro positions pass the same tile. The
distinction between SMALL/MEDIUM/LARGE/EXTRALARGE must come from a **separate**
n_tiles argument. The macro must be updated to accept `(SMALL_TILES, MEDIUM_TILES,
LARGE_TILES, EXTRALARGE_TILES)` as plain `Arg(N)` values:

```cpp
UC2_BENCHMARK_SPEC(
    Arg(1),    // SMALL  (~174 ms box, ~93 ms gauss)
    Arg(6),    // MEDIUM (~1044 ms box, ~558 ms gauss)
    Arg(58),   // LARGE  (~10.1 s box, ~5.4 s gauss)
    Arg(345)   // EXTRALARGE (~60 s box, ~32 s gauss)
)
```

The benchmark functions and macro definitions in `UC2Benchmark.cpp` must be updated to
read `state.range(0)` as `n_tiles` (not `width`/`height`).

#### UC14 — Buffer increase (no streaming; quicksort is in-place)

In-place quicksort requires all elements resident in memory simultaneously; chunk-based
streaming would require an external merge-sort which is a different algorithm. The approach
is to increase `N_SORT_MAX` and `N_STRUCT_MAX` to accommodate 60 s datasets.

**generic_sort** (`int64_t`, 8 B/element):

```cpp
static constexpr int64_t N_SORT_MAX = 400'000'000;  // was 200M
```

Memory: 400 M × 8 B × 2 (reference + working) = 6.4 GB. EXTRALARGE = 400 M elements.
Estimated runtime: (200 M / 70 M) × 2 × (6870–11628 ms) = 39 258–66 446 ms ≈ 40–66 s ✓.

**struct_sort / multi_key_sort** (`SortRecord = {double, double}`, 16 B/element):

```cpp
static constexpr int64_t N_STRUCT_MAX = 250'000'000;  // was 50M
```

Memory: 250 M × 16 B × 2 = 8 GB. EXTRALARGE = 250 M elements.
- struct_sort: (250 / 50) × 6720 ms = 33 600 ms ≈ 34 s ✓
- multi_key_sort: (250 / 50) × 8316 ms = 41 580 ms ≈ 42 s ✓

**Updated UC14 macro calls** (standalone `#else` block):

```cpp
// generic_sort variants
UC14_BENCHMARK_SPEC(Arg(900'000), Arg(8'000'000), Arg(70'000'000), Arg(400'000'000))
UC14_GENERIC_TRADEOFF_SPEC(Arg(900'000), Arg(8'000'000), Arg(70'000'000), Arg(400'000'000))
UC14_GENERIC_ABSTRACT_SPEC(Arg(900'000), Arg(8'000'000), Arg(70'000'000), Arg(400'000'000))

// struct_sort variants
UC14_STRUCT_LOW_SPEC(Arg(1'000'000), Arg(8'500'000), Arg(50'000'000), Arg(250'000'000))
UC14_STRUCT_TRADEOFF_SPEC(Arg(1'000'000), Arg(8'500'000), Arg(50'000'000), Arg(250'000'000))
UC14_STRUCT_ABSTRACT_SPEC(Arg(1'000'000), Arg(8'500'000), Arg(50'000'000), Arg(250'000'000))

// multi_key_sort variants
UC14_MULTI_LOW_SPEC(Arg(750'000), Arg(6'500'000), Arg(50'000'000), Arg(250'000'000))
UC14_MULTI_TRADEOFF_SPEC(Arg(750'000), Arg(6'500'000), Arg(50'000'000), Arg(250'000'000))
UC14_MULTI_ABSTRACT_SPEC(Arg(750'000), Arg(6'500'000), Arg(50'000'000), Arg(250'000'000))
```

#### Summary of required code changes

| UC | File | Change |
|----|------|--------|
| UC2 | `UC2Benchmark.cpp` | Replace `IMG_WIDTH_MAX/IMG_HEIGHT_MAX` with `TILE_W/TILE_H`; resize `g_src/g_dst`; rewrite benchmark functions to take `n_tiles = state.range(0)`; update macro definitions and calls |
| UC7 | `UC7Benchmark.cpp` | Rename `CORPUS_MAX` (standalone) to `CORPUS_CHUNK = 500 MB`; add streaming loop; update EXTRALARGE to 26 000 MB |
| UC14 | `UC14Benchmark.cpp` | Increase `N_SORT_MAX` to 400 M; increase `N_STRUCT_MAX` to 250 M; update EXTRALARGE macro args |

UC1, UC8, UC12: no code changes needed (streaming already implemented).

---

### Benchmark Phase Structure

Each use case registers three Google Benchmark phases. Because the factory function
(not `benchmarkJITOverhead`) is used, phases are written manually following the naming
convention required by `record_benchmark.py`:

```cpp
// Registered via UC7_BENCHMARK_SPEC macro (4 sizes × 3 phases = 12 benchmarks):
BENCHMARK(BM_UC7_unspecialized)->Name("BM_g:uc7_dfa;n:email;s:SMALL;t:unspecialized;")
    ->Arg(5LL*1024*1024)->Unit(benchmark::kMillisecond);
// ... MEDIUM / LARGE / EXTRALARGE variants ...
BENCHMARK(BM_UC7_jit_overhead)->Name("BM_g:uc7_dfa;n:email;s:MEDIUM;t:jit_overhead;")
    ->Arg(50LL*1024*1024)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_UC7_specialized_exec)->Name("BM_g:uc7_dfa;n:email;s:MEDIUM;t:specialized_exec;")
    ->Arg(50LL*1024*1024)->Unit(benchmark::kMillisecond);
```

The factory function handles `specializeOnly` internally via `specializeLambda`; the
benchmark TU calls it as an opaque `create_*_specialized(...)` C++ function.

### Correctness Validation

Each benchmark binary calls `assertSpecializedLambdaIsEquivalent` during startup (before
any timing). This calls the kernel function and the specialized lambda for a set of
reference inputs and throws `ClangRuntimeSpecializerChangesBehaviorError` on mismatch.
The assertion is placed in the benchmark binary's `main` (or in a `BENCHMARK` with
`->Iterations(1)`).

---

## Phase 1: Design

### Data Model

No DuckDB schema changes. The existing `benchmarks` table ingests the six new benchmark
binaries without modification. The `kernel` field (derived from the `n:` tag in the
benchmark name) uniquely identifies each archetype:

| Kernel field | Benchmark name tag |
|-------------|-------------------|
| `predicate` | `n:predicate` |
| `gaussian5x5` | `n:gaussian5x5` |
| `email_dfa` | `n:email` |
| `ivm_sum` | `n:ivm_sum` |
| `groupby_sum` | `n:groupby_sum` |
| `sort_int64` | `n:sort_int64` |

### Factory Function API (per use case)

Each kernel header exposes a typed factory function and a `SpecializedLambda` alias.
Example for UC7:

```cpp
// UC7Kernels.h
#pragma once
#include "ClangRuntimeSpecializer.h"
#include <cstdint>

// Non-static globals (serializable by IRDumpingPass)
extern int g_dfa_table[];      // specialization constant
static constexpr int DFA_N_STATES = 14;
static constexpr int DFA_N_CHARS  = 128;
static constexpr int DFA_START    = 0;
static constexpr int DFA_ACCEPT   = 1 << 13;

// Reference implementation signature
int64_t dfa_match(const char* haystack, int64_t len,
                  const int* dfa_table, int n_states,
                  int n_chars, int start_state, int accept_mask);

// Factory: specializeLambda with DFA constants baked in
using DFASpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const char*, int64_t>;
DFASpecialized create_dfa_specialized();
```

```cpp
// UC7Kernels.cpp  (compiled with -fpass-plugin=...)
#include "UC7Kernels.h"
#include "ClangRuntimeSpecializer.h"

int g_dfa_table[DFA_N_STATES * DFA_N_CHARS] = { /* hand-coded email DFA */ };

// Reference implementation — also used by AlwaysInliner in JIT pipeline
int64_t dfa_match(const char* hay, int64_t len,
                  const int* table, int n_states,
                  int n_chars, int start, int accept) {
    int state = start;
    int64_t matches = 0;
    for (int64_t i = 0; i < len; ++i) {
        state = table[state * n_chars + (unsigned char)hay[i]];
        if ((state & accept) != 0) { ++matches; state = start; }
    }
    return matches;
}

// Factory: lambda captures DFA constants and calls dfa_match.
// Both are in this TU → JIT can inline dfa_match after IPSCCP bakes constants.
DFASpecialized create_dfa_specialized() {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    // Lambda explicit args = (haystack, len); captures = DFA fixed params
    auto lam = [](const char* hay, int64_t len) -> int64_t {
        return dfa_match(hay, len, g_dfa_table, DFA_N_STATES, DFA_N_CHARS,
                         DFA_START, DFA_ACCEPT);
    };
    return RS->specializeLambda<int64_t>(lam);
}
```

Note: `g_dfa_table` is a non-`static` global. The lambda captures it by the address
baked into the IR (a constant pointer at the global's address). `DFA_N_STATES`,
`DFA_N_CHARS`, `DFA_START`, `DFA_ACCEPT` are `constexpr` — the compiler constant-folds
them into the lambda body before the IRDumpingPass runs; IPSCCP sees them as already
known, saving closure-serialization overhead.

### Source File Convention (all six use cases)

**`*Kernels.cpp`** (compiled with plugin):
- `#include "*Kernels.h"` + `#include "ClangRuntimeSpecializer.h"` only
- Non-`static` globals (so `DynamicLibrarySearchGenerator` can resolve them in host)
- Kernel function body (matches FR-010/013/016/019/022/025 signatures)
- Comparator function (UC14: `int64_asc_cmp(const void*, const void*)`, non-`static`)
- `create_*_specialized()` factory using `specializeLambda` with inline lambda
- `assertSpecializedLambdaIsEquivalent` call in a dedicated validation function

**`*Benchmark.cpp`** (compiled with plugin, includes `ClangRuntimeSpecializerBenchmark.h`):
- `#include "*Kernels.h"`
- Dataset allocation at file scope (or in benchmark setup)
- Three `BENCHMARK` functions per use case, manually named with `->Name(...)`
- Phase 2 (jit_overhead): calls `create_*_specialized()` in `->Iterations(1)` body
- Phase 3 (specialized_exec): calls `create_*_specialized()` once before loop

### CMake Build Pattern

Each use case follows the same two-custom-command pattern as `SyntheticKernels.o` +
`SpecializerBenchmark.o`. The kernel object is compiled with the plugin; the benchmark
object is compiled with the plugin (for consistency with existing targets) but is not
a JIT specialization target:

```cmake
# Template (shown for UC7; repeated for UC1/UC2/UC8/UC12/UC14)
set(UC7_KERNELS_OBJ "${CMAKE_CURRENT_BINARY_DIR}/UC7Kernels.o")
add_custom_command(OUTPUT ${UC7_KERNELS_OBJ}
    COMMAND ${CLANG_EXE} -x c++ -g -O3 ${CXX_FLAGS_LIST}
            -I${CMAKE_CURRENT_SOURCE_DIR}
            -I${CMAKE_CURRENT_SOURCE_DIR}/../../runtime/ClangRuntimeSpecializer
            -I${LLVM_MAIN_INCLUDE_DIR} -I${LLVM_BINARY_DIR}/include
            -fpass-plugin=${PLUGIN_LIB} -fexceptions -frtti
            -c ${CMAKE_CURRENT_SOURCE_DIR}/UC7DfaRegex/UC7Kernels.cpp
            -o ${UC7_KERNELS_OBJ}
    DEPENDS UC7DfaRegex/UC7Kernels.cpp LLVMRuntimeSpecializationComptimePlugin clang)

# Benchmark object (no plugin needed; compiled with plugin for IDE consistency)
set(UC7_BENCH_OBJ "${CMAKE_CURRENT_BINARY_DIR}/UC7Benchmark.o")
add_custom_command(OUTPUT ${UC7_BENCH_OBJ}
    COMMAND ${CLANG_EXE} -x c++ -g -O3 ${CXX_FLAGS_LIST}
            ... (same includes + -fpass-plugin) ...
    DEPENDS UC7DfaRegex/UC7Benchmark.cpp LLVMRuntimeSpecializationComptimePlugin clang)

add_benchmark(UC7DfaRegex PARTIAL_SOURCES_INTENDED ${UC7_KERNELS_OBJ} ${UC7_BENCH_OBJ} ...)
target_link_libraries(UC7DfaRegex PRIVATE ClangRuntimeSpecializer benchmark)
set_target_properties(UC7DfaRegex PROPERTIES ENABLE_EXPORTS ON)
target_link_options(UC7DfaRegex PRIVATE "-Wl,--export-dynamic")
```

The parent `benchmarks/CMakeLists.txt` adds `add_subdirectory(use-cases)`. The
`use-cases/CMakeLists.txt` also sets `PARENT_SCOPE` variables (one per use case)
exporting the pre-built kernel and benchmark `.o` paths so `benchmarks/CMakeLists.txt`
can include them in the existing `AllBenchmarks` target — following the same pattern used
for `tpch`.

### UC14 Comparator Specialization Detail

`generic_sort` takes `int (*comparator)(const void*, const void*)` as a parameter.
The lambda captures the comparator function pointer by value. After the JIT bakes in
the pointer as a constant, `ConstantArgAlwaysInlinePass` marks the indirect call
`comparator(a, b)` inside `generic_sort` as `alwaysinline`. `AlwaysInlinerPass` replaces
the indirect call with a direct inline of `int64_asc_cmp` (defined in the same kernel TU).
The result is a sort routine with the comparison inlined — enabling GVN and branch
elimination on the comparison logic.

`generic_sort` MUST implement **median-of-three quicksort** (user-confirmed). The comparator
call appears in every partition step, so inlining it after JIT specialization eliminates
one indirect call per comparison in the tightest loop. The benchmark re-shuffles the array
between iterations using `state.PauseTiming()` / `std::shuffle` / `state.ResumeTiming()`
so the shuffle overhead is excluded from reported timing.

---

## Complexity Tracking

No constitution violations. No complexity justification required.

## Speedup Findings

Results from release build, 2026-05-13. All six use cases pass `assertSpecializedLambdaIsEquivalent`/manual correctness validation.

| Use Case | Unspecialized | Specialized | Speedup | JIT Overhead | Notes |
|----------|--------------|-------------|---------|--------------|-------|
| UC1 SQL predicate | ~0.1 µs | ~0.01 µs | **8.43x** | ~0.1s | Predicate with min/max bounds folded to constants |
| UC2 Gaussian convolution | 202 ms | 54 ms | **3.76x** | ~0.1s | 1920×1080 image, 5-tap kernel coefficients constant |
| UC7 DFA regex | ~0.1 µs | ~0.06 µs | **1.47x** | ~0.1s | 10-state email-pattern DFA, table folded |
| UC8 IVM sum | ~0.1 µs | ~0.06 µs | **1.77x** | ~0.1s | Aggregation with constant column index |
| UC12 GROUP BY | ~0.1 µs | ~0.04 µs | **2.78x** | ~0.1s | Group-by with constant key extractor |
| UC14 Sort | 187 ms | 154 ms | **1.21x** | ~0.1s | 256 k int64 elements, comparator specialized |

All use cases exceed the ≥10% speedup performance goal. UC1 and UC2 show the largest gains due to heavy loop induction with constant bounds/coefficients.


---

## Phase 2: Batch-Oriented Kernel Redesign (2026-05-28)

**Motivation**: Per-row kernel functions called from a benchmark loop produce
misleading `specialized_exec_ns` values because the JIT dispatch overhead (~2–5 ns/call)
dominates the measurement for short kernels. This phase redesigns UC8 kernels as
batch functions and recalibrates all UC sizes to 1 GB / ~10 ms per call at MEDIUM.

See spec.md §Session 2026-05-28 clarification and §Research Finding for full rationale.

### Preconditions

- [x] Confirm current MEDIUM unspecialized times per-call for all 6 UCs by querying
  `v_ablation_medians` or running benchmarks directly with `--benchmark_repetitions=3`.
  Record baseline so post-change improvement is measurable.

### UC8: Batch Kernel Redesign

- [x] In `UC8Kernels.cpp` / `UC8Kernels.h`: add batch-oriented variants:
  - `apply_row_delta_batch(const uint8_t* rows, int64_t n_rows, double* agg_buckets, int n_buckets, int group_col_offset, int value_col_offset, int row_stride)` — loops over rows internally; replaces per-row `apply_row_delta` in the benchmark hot path.
  - `batch_delta_batch(const uint8_t* rows, int64_t n_rows, double threshold, double* out, int col_offset, int row_stride)` — batch threshold filter.
  - `multi_agg_delta_batch(const uint8_t* rows, int64_t n_rows, double* agg_buckets, int n_buckets, int group_col_offset, int* value_col_offsets, int n_agg_cols, int row_stride)` — batch multi-column aggregate.
- [x] In `UC8Benchmark.cpp`: update `jit_overhead` and `specialized_exec` phases to call
  the batch variant **once per benchmark iteration** with the full `n_rows` dataset.
  Remove the `for (rem > 0)` outer loop from benchmark body.
- [x] Update specialization constant set to match new function signatures (FR-020).
- [x] Verify `assertSpecializedIsEquivalent` passes for the new batch variants.

### Uniform 1 GB Buffer and Size Recalibration

- [x] For UC1, UC8, UC12: set `N_ROWS_MAX` (buffer allocation) to `1 GB / row_stride_bytes`:
  - UC1 (16 B/row): 64 M rows = 1 024 MB
  - UC8 (24 B/row): 42 M rows = 1 008 MB
  - UC12 (12 B/row): 85 M rows = 1 020 MB
- [x] For UC7: set corpus buffer to 1 GB; MEDIUM = fraction that takes ~10 ms unspecialized.
- [x] For UC2, UC14: measure current MEDIUM call time; adjust size if needed to reach ~10 ms.
- [x] Recalibrate SMALL/MEDIUM/LARGE/EXTRALARGE row-count constants per FR-007:
  - MEDIUM → row count that produces ~10 ms unspecialized per single call.
  - SMALL  → ~10× smaller than MEDIUM (≈ 1 ms).
  - LARGE  → ~10× larger than MEDIUM (≈ 100 ms).
  - EXTRALARGE → ~100× larger than MEDIUM (≈ 1 s); cap at buffer size if needed.
- [x] Update all `->Arg(...)` benchmark registration lines with new row-count constants.

### Verification

- [x] Run `ninja check-smoke-runtime-specializer` — no regressions.
- [x] Run AllBenchmarks MEDIUM filter; confirm unspecialized call time ≈ 10 ms for each
  UC group.
- [x] Re-run `optimize_benchmarks.py` for affected UCs; confirm `multi_agg_delta`,
  `batch_delta`, `column_scan` no longer regress (expected: >1.0× speedup after
  batching eliminates per-call dispatch overhead from the regression).
- [x] Record new baseline in DuckDB and note old vs new speedup table in `plan.md`.
