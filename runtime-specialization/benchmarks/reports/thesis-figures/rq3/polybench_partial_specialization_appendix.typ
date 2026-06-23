== PolyBench Kernel Summaries and Specialization Decisions <polybench-partial-specialization-appendix>
The summaries below paraphrase the official PolyBench documentation.#footnote[#link("https://github.com/MatthiasJReisinger/PolyBenchC-4.2.1/blob/master/polybench.pdf")[PolyBench 4.2.1 benchmark descriptions]]
They serve two purposes: first, to clarify what each kernel computes and why that computation matters; second, to justify whether a partial-specialization scenario is believable for the benchmark-visible API.

=== Data Mining

`correlation` computes the Pearson correlation coefficients of an `N x M` data matrix and produces an `M x M` symmetric correlation matrix.
This is useful in statistics and feature-analysis workloads where one wants to measure relationships between attributes.
We keep partial specialization here: `M` models a stable feature schema, while `N` models the number of rows in one batch.

`covariance` computes the covariance matrix of an `N x M` data matrix and produces an `M x M` symmetric covariance matrix.
This is useful in multivariate statistics as a basic measure of linear dependence between attributes.
We keep partial specialization here for the same reason as `correlation`: fixed attribute layout `M`, varying batch size `N`.

=== BLAS Routines

`gemm` is generalized matrix multiplication, computing `C_out = alpha * A * B + beta * C`.
It is the canonical dense linear-algebra primitive and is useful across scientific computing, simulation, and machine-learning style workloads.
We revert to full specialization because the exposed dimensions `NI`, `NJ`, and `NK` jointly define one coupled GEMM problem instance.

`gemver` performs multiple BLAS-like rank updates and matrix-vector multiplies, producing updated matrix/vector results.
It is useful as a richer dense linear-algebra kernel than plain GEMM because it combines dependent matrix and vector stages.
We revert to full specialization because only one benchmark-visible size parameter `N` remains after the buffers are captured.

`gesummv` computes `y = alpha * A x + beta * B x`.
It is useful as a compact fused matrix-vector benchmark representative of BLAS-style linear algebra.
We revert to full specialization because there is only one benchmark-visible size parameter `N`.

`symm` performs symmetric matrix-matrix multiplication `C_out = alpha * A * B + beta * C`.
It is useful in dense linear algebra when one operand has symmetric structure that an optimized implementation can exploit.
We revert to full specialization because the exposed dimensions `M` and `N` are just the coupled shape of one pure matrix problem.

`syrk` performs a symmetric rank-k update `C_out = alpha * A A^T + beta * C`.
It is useful in structured linear-algebra workloads such as covariance-like updates and blocked factorizations.
We revert to full specialization because the exposed dimensions are mathematically coupled and do not form a believable stable/dynamic split.

`syr2k` performs a symmetric rank-2k update `C_out = alpha * A B^T + alpha * B A^T + beta * C`.
It is useful as another structured BLAS kernel that updates symmetric matrices efficiently.
We revert to full specialization for the same reason as `syrk`: the exposed size parameters jointly define one pure problem instance.

`trmm` multiplies a triangular matrix by another matrix, storing the result in place of `B`.
It is useful in triangular-solve and factorization pipelines where triangular operands arise naturally.
We revert to full specialization because the visible dimensions are the full problem shape, not a separate runtime kernel object.

=== Linear Algebra Kernels

`2mm` performs two chained matrix multiplications and computes `E = alpha * A * B * C + beta * D`.
It is useful as a representative multi-stage dense linear-algebra pipeline with intermediates.
We revert to full specialization because the four exposed dimensions are tightly coupled and together define the legal matrix chain.

`3mm` performs three chained matrix multiplications and computes `G = (A * B) * (C * D)`.
It is useful as another representative dense kernel with more intermediates and more shape coupling.
We revert to full specialization because the five exposed dimensions are all part of one coupled pure matrix problem.

`atax` computes `A^T * (A * x)`.
It is useful in numerical linear algebra and least-squares style workloads where repeated operator applications appear.
We revert to full specialization because `M` and `N` together define the matrix shape; varying only one would be artificial.

`bicg` is the kernel of BiCGSTAB and computes both `q = A p` and `s = A^T r`.
It is useful as a representative iterative-solver linear-algebra kernel.
We revert to full specialization because the row and column dimensions are coupled parts of one operator shape.

`doitgen` is a tensor contraction kernel derived from MADNESS-style scientific simulation code.
It is useful as a representative higher-dimensional scientific array kernel.
We revert to full specialization because the PolyBench documentation itself notes a dimensional consistency constraint, making a partial size split especially hard to justify.

`mvt` performs two matrix-vector multiplications, one with `A` and one with `A^T`, updating `x1` and `x2`.
It is useful as a compact benchmark for dense matrix-vector work.
We revert to full specialization because only one benchmark-visible size parameter `N` exists.

=== Linear Algebra Solvers

`cholesky` computes the Cholesky decomposition `A = L L^T` for a positive-definite matrix.
It is useful in numerical linear algebra, Gaussian-model computations, and least-squares methods.
We revert to full specialization because only one benchmark-visible size parameter `N` exists.

`durbin` solves a special Toeplitz/Yule-Walker system.
It is useful in signal-processing and time-series/autoregressive settings.
We revert to full specialization because only one benchmark-visible size parameter `N` exists.

`gramschmidt` computes a QR decomposition with modified Gram-Schmidt.
It is useful in numerical linear algebra and least-squares factorization.
We revert to full specialization because `M` and `N` together define the factorization shape and preconditions.

`lu` computes an LU decomposition without pivoting.
It is useful as a classic direct-solver building block.
We revert to full specialization because only one benchmark-visible size parameter `N` exists.

`ludcmp` solves `A x = b` using LU decomposition followed by forward and backward substitution.
It is useful as a more complete direct-solver workflow than `lu` alone.
We revert to full specialization because only one benchmark-visible size parameter `N` exists.

`trisolv` solves a lower-triangular system `L x = b` by forward substitution.
It is useful in direct-solver pipelines and factorization back-solves.
We revert to full specialization because only one benchmark-visible size parameter `N` exists.

=== Medley

`deriche` implements the Deriche recursive filter, which can be used for smoothing and edge detection via horizontal and vertical image passes.
It is useful in low-level image-processing and computer-vision pipelines.
We revert to full specialization because the benchmark wrapper already fixes the filter coefficients; the remaining width and height parameters are simply the full image shape, not a separate small convolution kernel object.

`floyd-warshall` computes all-pairs shortest path lengths in a weighted graph.
It is useful in graph analytics, routing, and dynamic-programming-based optimization.
We revert to full specialization because only one benchmark-visible size parameter `N` exists.

`nussinov` is a dynamic-programming algorithm for RNA folding prediction.
It is useful in bioinformatics for predicting secondary structure.
We revert to full specialization because only one benchmark-visible size parameter `N` exists.

=== Stencils

`adi` models alternating-direction implicit heat diffusion over a 2D grid.
It is useful in PDE solvers because it splits a 2D diffusion step into cheaper 1D sub-problems.
We keep partial specialization here: the grid size `N` can stay fixed while the number of simulated time steps `TSTEPS` changes by workload or stopping criterion.

`fdtd_2d` models a simplified 2D finite-difference time-domain electromagnetic simulation.
It is useful in computational electromagnetics and wave-propagation studies.
We keep partial specialization here: the spatial mesh (`NX`, `NY`) is stable, while the simulation horizon `TMAX` is the natural runtime-varying work amount.

`heat_3d` iterates the heat equation over a 3D space.
It is useful as a representative diffusion stencil and as a benchmark for spatial-locality optimizations.
We keep partial specialization here: the 3D volume size `N` can stay fixed while `TSTEPS` varies.

`jacobi_1d` is a 1D Jacobi-style stencil using a 3-point update pattern.
It is useful as a simple iterative stencil benchmark and as a minimal representative of time-stepped local averaging.
We keep partial specialization here: fixed domain size `N`, varying iteration count `TSTEPS`.

`jacobi_2d` is a 2D Jacobi-style stencil using a 5-point update pattern.
It is useful in iterative PDE solvers and stencil-optimization studies.
We keep partial specialization here for the same reason as `jacobi_1d`: fixed spatial domain, variable iteration horizon.

`seidel_2d` is a 2D Gauss-Seidel-style stencil using a 9-point update pattern.
It is useful when same-step updates and their convergence behavior matter.
We keep partial specialization here: the fixed grid size `N` is stable state, while `TSTEPS` is the believable runtime-varying work amount.
