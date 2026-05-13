# Phase 0: Research — Use-Case Benchmark Suite

## Lambda Factory Pattern

**Decision**: Each kernel TU exposes a `create_*_specialized()` factory function that
calls `specializeLambda<R>(lambda)` internally. The lambda captures the fixed parameters
and calls the kernel function from the same TU.

**Rationale**: `specializeLambdaImpl` clones `TargetFunc->getParent()` — the entire
blob module. Co-locating the kernel function and the lambda's `operator()` in the same
kernel TU ensures both are in the cloned module. `ConstantArgAlwaysInlinePass` then marks
the kernel call (which receives constant-captured args) as `alwaysinline`, and
`AlwaysInlinerPass` inlines the kernel body (it's defined in the same module, not just
declared). This gives the same full constant-propagation depth as `specializeOnly`.

**Alternatives considered**:
- Lambda in benchmark TU calling kernel externally → kernel body not available in JIT
  module → no inlining → limited optimization; also risks JIT crash from
  `benchmark::State` WeakODR vtables surviving early GlobalDCE.
- Header-only inline kernel functions → conflicts with FR-001 (kernel must be in `.cpp`).
- Cross-blob merging → not implemented; would require changes to `specializeOnlyImpl`.

## DFA Construction for UC7

**Decision**: Programmatically-built DFA for email-address recognition.
A builder function fills `g_dfa_table` once at binary startup (via a static initializer
object or `__attribute__((constructor))`):
- N_STATES ≈ 14–16 states; N_CHARS = 128 (ASCII)
- State 0 = start; accept state flagged via bitmask
- High bytes (≥ 128) map to a dead/reject state
- Table stored as a non-`static` global `int g_dfa_table[N_STATES * N_CHARS]`
- Builder uses no external regex library; encodes the email-pattern state machine directly

**Rationale**: Avoids a regex library dependency in the kernel TU (which must stay clean);
programmatic construction is easier to maintain than a raw numeric table initializer,
while still keeping the kernel TU dependency-free.

**Alternatives considered**:
- Hand-coded numeric table → brittle; hard to read and verify correct state transitions.
- Runtime regex library → introduces libc++/libstdc++ symbols into the kernel blob,
  risking "Symbols not found" JIT errors.

## `constexpr` Constants vs. Captured Runtime Values

**Decision**: `DFA_N_STATES`, `DFA_N_CHARS`, `DFA_START`, `DFA_ACCEPT` are `constexpr`
in the kernel header. The compiler propagates them at compile time, so they appear as IR
constants before the IRDumpingPass runs. The JIT does not need to bake them through the
closure mechanism. Only `g_dfa_table` (a non-`static` global) is passed as a constant
pointer by the JIT (its address is stable at runtime).

**For use cases where constants are truly runtime-determined** (e.g., the threshold in
UC1, the kernel coefficients address in UC2), the factory function accepts them as
parameters and the lambda captures them. This is the general pattern.

## UC14: Comparator Pointer Specialization

**Decision**: `int64_asc_cmp(const void*, const void*)` is defined as a non-`static`,
non-`inline` external function in `UC14Kernels.cpp`. The lambda captures its address as
`comparator`. After the JIT bakes in `comparator = &int64_asc_cmp`, the call
`comparator(a, b)` inside `generic_sort` has a constant callee. `ConstantArgAlwaysInlinePass`
marks it `alwaysinline`; `AlwaysInlinerPass` inlines `int64_asc_cmp` (defined in same
module) into the sort body.

**Why this is interesting for the thesis**: Demonstrates that function-pointer
specialization (a traditionally non-trivial JIT pattern) is a natural consequence of
the lambda-capture mechanism, with no special handling required.

## Dataset Sizes and Reuse

**Decision**: All hot-loop datasets are allocated once at benchmark binary startup and
reused across iterations. UC14 arrays are re-shuffled between iterations (using
`std::shuffle` with a fixed seed) to ensure each `generic_sort` call is a real sort.
UC12 `out_buckets` is zeroed between iterations (via `std::fill`) to maintain
aggregation correctness.

## No DuckDB Schema Changes

All six benchmarks emit output in the existing Google Benchmark JSON format with the
`BM_g:<group>;n:<name>;t:<phase>;` naming convention. `record_benchmark.py` ingests
them unchanged.

## Benchmark Helper Usage

**Decision**: Use manually named `BENCHMARK` lambdas with `->Name(...)` rather than
`benchmarkJITOverhead<funcName>`. The existing `benchmarkJITOverhead` helper
internally calls `specializeOnly(funcName, ...)` which no longer exists under spec 010.
Writing the three phases manually is simpler than creating new generic helpers and keeps
each benchmark's control flow explicit.

The three-phase naming pattern (`t:unspecialized`, `t:jit_overhead`, `t:specialized_exec`)
is preserved — `record_benchmark.py` depends on the `t:` tag, not on any particular
C++ helper function.
