# PolyBench Partial-Specialization Decisions

This file records the intended specialization split for each PolyBench kernel. The common rule is:

- specialize stable runtime state that plausibly remains fixed for one processing session, such as data-buffer identities, coefficient tables, matrix layouts, or a subset of dimensional parameters;
- keep at least one request-varying problem-size or iteration parameter explicit whenever that parameter represents per-call work variation in the modeled scenario.

For unary kernels, the benchmark-visible size parameter remains the per-call varying input and the specialized subset is carried by the lambda closure through stable runtime state such as array identities and auxiliary buffer layout. This is still a proper partial-specialization model relative to the underlying imported kernel, whose true runtime interface includes both scalar parameters and data pointers.

| Kernel | Specialized inputs / state | Runtime-variable inputs | Repeated-call scenario represented |
| --- | --- | --- | --- |
| `correlation` | `m`, data/corr/mean/stddev buffers | `n` | Same feature layout reused across batches while row count varies per batch. |
| `covariance` | `m`, data/cov/mean buffers | `n` | Same feature layout and output buffer reused while incoming sample count changes. |
| `2mm` | `ni`, `nj`, `nk`, all matrix buffers | `nl` | Fixed left-hand pipeline and intermediate layout, varying output width per request. |
| `3mm` | `ni`, `nj`, `nk`, `nl`, all matrix buffers | `nm` | Fixed upstream matrix chain with varying final reduction/output extent. |
| `atax` | matrix/vector buffers, `m` | `n` | Fixed row layout with varying active column count per request. |
| `bicg` | matrix/vector buffers, `m` | `n` | Fixed row-side layout and work buffers, varying column-side extent. |
| `doitgen` | tensor/coefficient buffers, `nq`, `nr` | `np` | Fixed outer tensor grid while the innermost transform width varies between calls. |
| `mvt` | matrix/vector buffers | `n` | Same allocated matrix/vector session, varying active prefix length. |
| `gemm` | matrix buffers, `ni`, `nj` | `nk` | Fixed output tile geometry while the reduction depth varies by request. |
| `gemver` | matrix/vector buffers | `n` | Same update buffers reused across requests with varying active problem size. |
| `gesummv` | matrix/vector buffers | `n` | Same coefficient matrices reused while active vector length changes. |
| `symm` | matrix buffers, `m` | `n` | Fixed left symmetric operand shape, varying output width. |
| `syr2k` | matrix buffers, `m` | `n` | Fixed update width/layout, varying accumulation extent. |
| `syrk` | matrix buffers, `m` | `n` | Fixed update width/layout, varying accumulation extent. |
| `trmm` | matrix buffers, `m` | `n` | Fixed triangular transform shape, varying output width. |
| `cholesky` | matrix buffer | `n` | Same factorization workspace reused with varying active matrix prefix. |
| `durbin` | `r`/`y` buffers | `n` | Same signal workspace reused while the active sequence length changes. |
| `gramschmidt` | matrix buffers, `m` | `n` | Fixed row layout with varying orthogonalization width. |
| `lu` | matrix buffer | `n` | Same factorization buffer reused while active matrix size changes. |
| `ludcmp` | matrix/vector buffers | `n` | Same factorization/solve workspace reused while active matrix size changes. |
| `trisolv` | matrix/vector buffers | `n` | Same triangular-solve workspace reused while active prefix length changes. |
| `deriche` | image/work buffers, `w` | `h` | Fixed image row stride and filter workspace, varying image height. |
| `floyd_warshall` | path matrix buffer | `n` | Same path matrix allocation reused while active graph size changes. |
| `nussinov` | sequence/table buffers | `n` | Same DP workspace reused while active sequence length changes. |
| `adi` | work buffers, `n` | `tsteps` | Fixed grid geometry reused while the number of simulation time steps varies. |
| `fdtd_2d` | field buffers, `nx`, `ny` | `tmax` | Fixed field layout reused while the simulated time horizon varies. |
| `heat_3d` | 3D buffers, `n` | `tsteps` | Fixed volume geometry reused while the number of diffusion steps varies. |
| `jacobi_1d` | work buffers, `n` | `tsteps` | Fixed vector layout reused while the iteration count changes. |
| `jacobi_2d` | work buffers, `n` | `tsteps` | Fixed grid layout reused while the iteration count changes. |
| `seidel_2d` | grid buffer, `n` | `tsteps` | Fixed grid layout reused while the iteration count changes. |

## Reporting / Appendix Intent

- The appendix section in `docs/thesis.typ` should summarize this same table in prose/list form.
- The reporting script should be able to emit an appendix-ready Typst snippet from the same decision ledger to avoid hand-maintaining a second copy of the kernel rationale.
- The conditional summary table for amortized speedup and `$U_p$` should only be emitted when the PolyBench curves are sufficiently size-stable; otherwise the reporting script should write a short provenance note explaining why the table was skipped.
