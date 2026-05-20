# Research: Benchmark Dynamic Buffer Allocation

## Memory Audit

| Benchmark File | Buffers | Approx Size |
|---|---|---|
| UC14Sort/UC14Benchmark.cpp | g_reference_data (int64×400M), g_sort_data (int64×400M), g_struct_reference (SortRecord×250M), g_struct_data (SortRecord×250M) | ~14.4 GB |
| UC8IVM/UC8Benchmark.cpp | g_deltas (uint8×50M×24B) | ~1.2 GB |
| UC12GroupBy/UC12Benchmark.cpp | g_rows12 (uint8×50M×24B) | ~1.2 GB |
| UC1SqlPredicate/UC1Benchmark.cpp | g_rows (uint8×50M×16B), g_out_indices (int32×50M) | ~1.0 GB |
| UC7DfaRegex/UC7Benchmark.cpp | g_corpus (char×500MB or 1GB in AllBenchmarks) | ~1.0 GB |
| polybench/polybench_bench.cpp | 30 kernels × 2–7 arrays each (e.g. gemm: 3 arrays ≈ 180MB; correlation: 2 arrays ≈ 120MB; etc.) | ~3–5 GB |
| UC2Convolution/UC2Benchmark.cpp | g_src (float×3840×3840), g_dst (float×3840×3840) | ~115 MB |
| DBOperatorsBenchmark.cpp | Operator objects only | negligible |
| tpch/ | DB connections (lazy) | negligible |

**Total**: ~23 GB, consistent with reported peak.

## Decision: Google Benchmark Setup/Teardown API

**Decision**: Use `->Setup(fn)->Teardown(fn)` on each `BENCHMARK(...)` registration.

**API signature**: `void fn(const benchmark::State&)` — receives current state (size ranges available via `state.range(0)` etc., though not needed for buffer allocation decisions since buffers are sized to the maximum needed).

**Rationale**: This is the standard Google Benchmark mechanism for per-benchmark lifecycle. The setup fires once before the benchmark's iterations begin; teardown fires once after all iterations finish. Both are outside the measured timing window by design.

**Alternative considered**: BENCHMARK_F with fixture classes. Rejected because it would require rewriting all 200+ benchmark registrations and all benchmark function signatures (from free functions to member functions), which is a much larger diff with higher regression risk.

## Decision: Reference-Counting for Shared Buffers

**Decision**: Each benchmark file defines a single `static int g_xxx_refcount = 0` and one setup/teardown pair. Setup increments refcount and allocates on first call (when refcount was 0). Teardown decrements and frees on last call (when refcount reaches 0), then calls `shrink_to_fit()` or sets raw pointers to `nullptr`.

**Rationale**: Multiple benchmark registrations in the same file share the same dataset. Reference counting ensures the dataset is allocated once (on the first benchmark's setup) and freed once (on the last benchmark's teardown), without reallocating between registrations. This works because Google Benchmark runs benchmarks sequentially by default.

**Alternative considered**: Allocate/free per registration (no ref-count). Rejected because initialization for UC14 alone involves filling 3.2 GB with random data; doing this 50+ times would make the test suite extremely slow even though it's outside the timed window.

**Alternative considered**: Lazy initialization inside the benchmark body with `PauseTiming`. Rejected because it adds complexity inside benchmark functions and risks accidentally drifting into the timed window.

## Decision: Polybench — Static Constructor Replacement

**Decision**: Remove each `static struct XxxInit { XxxInit() { ... } }` constructor. Replace with `static void pb_setup_K(const benchmark::State&)` and `static void pb_teardown_K(const benchmark::State&)` functions defined in the per-kernel block (where size constants like N, M, NI etc. are still in scope). Extend `POLYBENCH_BENCHMARK_SPEC` macro to append `->Setup(pb_setup_##K)->Teardown(pb_teardown_##K)` to every registration line.

**Rationale**: The per-kernel block already has the size constants in scope; setup/teardown can use them for `new double[N][M]()`. This is a mechanical substitution of the existing constructor pattern.

**Note**: Kernels with only stack-allocated arrays (durbin, jacobi-1d) need no-op setup/teardown or can skip them with macro guards.

## Decision: UC Benchmark Buffer Groups

**Decision**: One setup/teardown pair per UC benchmark file, managing all large buffers in that file. A single `g_xxx_refcount` guards the group.

**Rationale**: Simplicity. Finer-grained grouping (e.g., separating int64 and struct buffers in UC14) would save memory between sub-groups but requires knowing benchmark execution order and adds complexity. The primary goal is reducing 23 GB → < 8 GB; per-file grouping is sufficient.

## Decision: `std::vector` Release Pattern

**Decision**: `vec.clear(); vec.shrink_to_fit();` to release memory. For raw pointer arrays (polybench): `delete[] ptr; ptr = nullptr;`.

**Rationale**: `clear()` alone does not release capacity. `shrink_to_fit()` is a non-binding hint that in practice releases capacity to the allocator on all major implementations (GCC, Clang, libc++). The OS may not reclaim pages until malloc trims, but RSS will drop after the allocator releases the pages. An alternative is to assign an empty vector: `vec = std::vector<T>();`, which also works.

## Resolved: Google Benchmark Version

The environment is pinned via `flake.nix`. Google Benchmark 1.7.0+ is required for Setup/Teardown. Nix provides a recent version of `benchmark`. The `->Setup(fn)->Teardown(fn)` API was added in Google Benchmark 1.7.0 (2022). This API is confirmed available.
