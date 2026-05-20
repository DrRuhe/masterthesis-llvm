# Feature Specification: Benchmark Dynamic Buffer Allocation

**Feature Branch**: `WIP-specialization/013-dynamic-buffer-alloc`  
**Created**: 2026-05-17  
**Status**: Draft  

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Run full benchmark suite without OOM (Priority: P1)

A researcher runs the combined benchmark binary (`AllBenchmarks`) on a machine with 16–32 GB RAM. Currently the binary requires ~23 GB of memory at peak because all benchmark datasets are allocated at process startup and held simultaneously. After this change, each benchmark allocates its buffers just before it runs and frees them immediately after, so the peak memory at any point in time is determined by the single largest individual benchmark's buffers rather than their sum.

**Why this priority**: The benchmark suite is currently unusable on any machine with less than 24 GB free RAM. Reducing peak memory is a prerequisite for running benchmarks in CI or on standard developer hardware.

**Independent Test**: Run `AllBenchmarks` (or any individual benchmark binary) under `valgrind --tool=massif` or by observing `/proc/self/status` VmRSS. Peak RSS must be well below the sum of all individual benchmark buffer sizes.

**Acceptance Scenarios**:

1. **Given** the combined `AllBenchmarks` binary, **When** it executes all registered benchmarks sequentially, **Then** peak RSS never exceeds 8 GB (versus the previous ~23 GB).
2. **Given** any individual benchmark binary (e.g. UC14), **When** it runs, **Then** buffers are absent before the first benchmark starts timing and freed after the last benchmark for that TU finishes.
3. **Given** a benchmark with multiple size variants (e.g. SMALL, MEDIUM, LARGE), **When** each variant runs in sequence, **Then** the same pre-allocated buffer is reused across iterations without reallocation per iteration.

---

### User Story 2 - Timed benchmark runs are unaffected (Priority: P2)

Benchmark timing numbers remain valid: no buffer allocation or initialization work happens inside the timed measurement window.

**Why this priority**: Correctness of benchmark measurements is the primary purpose of the benchmark suite. Any change that accidentally folds allocation time into measured results would invalidate all data.

**Independent Test**: Run a single benchmark (e.g. `BM_UC14_unspecialized`) with `--benchmark_filter` and verify the reported times match pre-change baselines.

**Acceptance Scenarios**:

1. **Given** a benchmark whose timed loop calls a kernel, **When** the benchmark is registered with setup/teardown, **Then** buffer allocation happens exclusively outside `state.ResumeTiming()` windows.
2. **Given** the UC14 sort benchmark where each iteration shuffles data before sorting, **When** the benchmark runs, **Then** only the shuffle and sort are timed, not buffer allocation.

---

### User Story 3 - Polybench kernels allocate and free per-benchmark (Priority: P3)

Polybench kernels each have multi-gigabyte arrays (e.g. `new double[3000][2600]` = ~60 MB per array, 30 kernels with 2–7 arrays each ≈ several GB total). These are currently allocated at startup via static constructors. They should be allocated before the benchmark group for a kernel runs and freed after.

**Why this priority**: Polybench alone likely contributes several GB to the startup allocation; freeing between kernels reclaims memory for the next kernel's run.

**Independent Test**: Run only polybench benchmarks; observe that VmRSS increases when a kernel group starts and decreases after it finishes.

**Acceptance Scenarios**:

1. **Given** the polybench `correlation` benchmark arrays, **When** no polybench benchmark is currently running, **Then** the arrays are not allocated (pointers are null).
2. **Given** back-to-back polybench kernel groups, **When** the first kernel finishes and the second begins, **Then** the first kernel's arrays have been freed before the second's are allocated.

---

### Edge Cases

- What happens when a benchmark is filtered out (via `--benchmark_filter`) and its setup is never called? Teardown must not free unallocated memory (guard with null-check).
- What happens when the same buffer is shared by multiple benchmark functions within the same group (e.g. UC14's `g_reference_data` is used by both `BM_UC14_unspecialized` and `BM_UC14_specialized_exec`)? Allocation must happen once per group, not once per benchmark function.
- What happens if a benchmark terminates early (e.g. timeout)? Teardown must still free the buffer to avoid leaks.
- What if allocation fails (out of memory)? The benchmark should report a clear error rather than crashing with a null-pointer dereference.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: All large benchmark datasets (UC1, UC2, UC7, UC8, UC12, UC14, polybench) MUST be allocated dynamically before their benchmark group runs, not at process startup.
- **FR-002**: Each benchmark group MUST free its allocated buffers after the group finishes, returning the memory to the OS.
- **FR-003**: Buffer allocation and initialization MUST NOT occur inside any timed measurement window (i.e., not between `state.ResumeTiming()` and `state.PauseTiming()`).
- **FR-004**: The existing benchmark registration structure (benchmark names, `state.range()` size variants, number of iterations, output counters) MUST remain unchanged.
- **FR-005**: DuckDB and SQLite benchmarks, which use database connections rather than raw buffers, MUST follow the same lifecycle pattern: open connection in setup, close in teardown.
- **FR-006**: The `AllBenchmarks` combined binary MUST be buildable and runnable with peak RSS under 8 GB across the full suite.
- **FR-007**: Individual benchmark binaries (UC-only, polybench-only) MUST continue to work correctly with the same timing characteristics as before.
- **FR-008**: Null-pointer guards MUST be present in teardown callbacks so that benchmarks skipped by `--benchmark_filter` do not crash on teardown.

### Key Entities

- **Benchmark group**: A set of benchmark functions sharing the same dataset (e.g. all UC14 functions share `g_reference_data` and `g_sort_data`). One setup/teardown pair per group.
- **Setup callback**: A function invoked by the benchmark framework before the first iteration of a benchmark group that allocates and initializes buffers.
- **Teardown callback**: A function invoked after the last iteration of a benchmark group that frees the allocated buffers.
- **Buffer**: A heap-allocated contiguous array (typically `std::vector` or raw pointer) holding benchmark input or output data.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Peak RSS of the combined `AllBenchmarks` binary drops from ~23 GB to under 8 GB when running all benchmarks sequentially.
- **SC-002**: No benchmark's reported wall-clock or CPU time changes by more than 5% relative to baseline, confirming allocation is not included in the timed window.
- **SC-003**: All existing benchmark smoke tests continue to pass without modification (`ninja check-smoke-runtime-specializer`).
- **SC-004**: The benchmark suite completes successfully on a machine with 16 GB RAM (previously required >23 GB).
- **SC-005**: Every allocated buffer has a corresponding free call: no memory leaks detectable by running with a leak-checker on the benchmark binary with a size-filtered single benchmark.

## Assumptions

- The benchmark framework supports per-benchmark Setup/Teardown callbacks (Google Benchmark ≥1.7 provides `->Setup(fn)->Teardown(fn)` on `benchmark::internal::Benchmark*`).
- Benchmarks within a group that share a buffer are registered such that their setup/teardown fires exactly once per group (not once per benchmark function). The Google Benchmark `->Setup`/`->Teardown` API fires per benchmark instance, so grouping is achieved by sharing a single global pointer guarded by a reference count or by registering a single setup for the first benchmark and teardown for the last.
- DuckDB and SQLite benchmarks' "buffers" are database connections/files, which are already opened lazily; this feature covers ensuring they are closed after use.
- Polybench arrays declared with `new double[N][M]` will be converted to raw pointer globals (initialized to nullptr) with explicit allocation in setup and `delete[]` in teardown.
- The `SpecializerBenchmark.cpp` (synthetic kernel benchmarks) uses only small stack-local data and does not need changes.
- The `DBOperatorsBenchmark.cpp` (virtual dispatch benchmarks) uses only lightweight operator objects and does not need changes.
