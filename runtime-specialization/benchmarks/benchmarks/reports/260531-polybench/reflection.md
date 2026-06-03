# PolyBench Evaluation — P0 vs P2+O3 (2026-05-31)

**Binary**: `PolyBenchBenchmark` (dedicated binary — AllBenchmarks throws DumpedIRError for PolyBench)  
**Sizes**: SMALL + MEDIUM (LARGE/EXTRALARGE excluded — too slow)  
**Pipeline**: P0 (default, large_module_threshold=10000) vs P2+O3 (force_spec=1, spec_on_addr=1, spec_literal=1)

## Key Findings

### JIT overhead: P0 ≈ P2+O3

Both pipelines produce nearly identical JIT times for PolyBench:
- P0: ~190–220ms per kernel
- P2+O3: ~190–230ms per kernel

The blob per kernel is ~2.276 MB, 197 functions, 21k instructions. P2's JitIPSCCPPass on 197 functions
takes negligible extra time vs P0's fixpoint loop. No large-module penalty.

### Exec speedups: mostly flat at MEDIUM size

Most PolyBench kernels show < 1.0× speedup (regression) at MEDIUM size because:
- Unspecialized execution is already very fast (often < 10ms at MEDIUM)
- JIT overhead (~200ms) cannot be amortized in a single call
- Static compiler already vectorizes PolyBench with -O3

### P2 wins on stencil kernels

Two stencil kernels show genuine speedup with P2 that P0 misses:
| Kernel | P0 | P2+O3 |
|--------|:---:|:---:|
| `heat_3d` | 0.97× | **1.16×** |
| `seidel_2d` | 1.02× | **1.18×** |

Reason: these kernels have loop bounds stored as invariant constants (problem dimensions).
P2's SCCP propagates these as literal IR immediates, enabling the O3 loop optimizer to
see constant trip counts and apply more aggressive loop transformations.

### P0 wins: `deriche` (1.22×), `floyd_warshall` (1.07×)

These benefit from P0's inlining of the recursive/iterative kernel structure, which P2
doesn't replicate. `deriche` uses a data-dependent algorithm where knowing constants
early in the pipeline enables branch elimination that P2 doesn't achieve.

## Thesis implications

- PolyBench with MEDIUM size is **not a fair evaluation** for runtime specialization.
  The JIT overhead (200ms) dominates because MEDIUM problem sizes are too small.
  LARGE or EXTRALARGE sizes would amortize JIT overhead but take 10× longer to run.
  
- A better PolyBench evaluation strategy: use LARGE/EXTRALARGE with `specializeOrFallback`
  (amortize JIT over multiple calls). The break-even analysis shows that at 200ms JIT /
  ~5ms exec speedup, break-even requires ~40 calls.

- P2+O3 is not worse than P0 for PolyBench on JIT time — the concern about P2 running
  JitIPSCCPPass on large modules does not apply here (2.2MB blobs are small).

## Failure: AllBenchmarks binary

PolyBench throws `ClangRuntimeSpecializerDumpedIRError` when run via AllBenchmarks.
Root cause: blob lookup fails in the multi-TU binary context. Always use the dedicated
`PolyBenchBenchmark` binary for polybench measurements.
