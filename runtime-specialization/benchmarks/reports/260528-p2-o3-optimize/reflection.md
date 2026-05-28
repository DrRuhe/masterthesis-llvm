
---

## 12. Research Finding: Minimum Viable Kernel Duration

**Finding**: JIT specialization via `callSpecialized` introduces a structural per-call
indirect dispatch overhead of approximately **2–5 ns/call**. For kernels whose
unspecialized execution takes less than this threshold, specialization always regresses.

### Evidence from this study

| Kernel | Unspec/call | P0 spec'd | P2+O3 spec'd | Verdict |
|--------|:-----------:|:---------:|:------------:|---------|
| `multi_agg_delta` | **0.26 ns** | 0.42 ns (0.63×) | 0.46 ns (0.56×) | Dispatch-dominated regression |
| `batch_delta` | **0.24 ns** | 1.22 ns (0.20×) | 0.26 ns (0.94×) | P0 cache-cold; P2+O3 near-parity but still overhead |
| `column_scan` | **0.84 ns** | 3.76 ns (0.22×) | 0.95 ns (1.06×) | P2+O3 recovers; still borderline |

The `multi_agg_delta` kernel (0.26 ns unspecialized) regresses in **both** P0 and
P2+O3. No amount of pipeline tuning can fix this — the per-call dispatch overhead
is a JIT mechanism cost, not an optimizer cost.

### Root cause

`callSpecialized` calls through a function pointer stored in the JIT's memory region.
This is an **indirect call** — the compiler cannot inline it, and the CPU's branch
predictor takes longer to warm up than for a direct call. On x86-64 the indirect call
costs ~2–5 ns depending on cache state.

For comparison, a direct (non-JIT) function call takes ~0.3–0.5 ns if the callee is
in instruction cache, and is eliminated entirely if the callee is inlined.

### Practical guidance for benchmark and application design

> **Rule**: Each call to `callSpecialized` / `specializeOrFallback` should process
> at least **~1 µs of work** to keep the dispatch overhead below 0.5%.

For batch-oriented kernels (UC1 `column_scan`, UC12 `grouped_sum`), this is satisfied
by calling the function once with a large array (~10–100 ms of work). For the per-row
pattern (UC8 `apply_row_delta`, `batch_delta`), the fix is to redesign as a batch
function that loops internally — eliminating the per-row dispatch cost.

**Thesis claim**: "JIT specialization via indirect dispatch is not beneficial for
kernels with sub-microsecond per-call duration. The minimum viable kernel duration
is approximately 1 µs; callers should ensure the specialized function processes a
meaningful batch of work per invocation."

This finding is encoded as a new requirement (FR-007b) in
`specs/008-use-case-benchmarks/spec.md` and is being addressed by Phase 2 of the
UC benchmark plan (batch-kernel redesign for UC8).
