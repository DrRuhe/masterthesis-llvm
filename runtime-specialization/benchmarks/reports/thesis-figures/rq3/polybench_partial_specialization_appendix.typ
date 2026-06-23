== PolyBench Partial-Specialization Rules <polybench-partial-specialization-appendix>
The partial-specialization choices follow three rules derived from the PolyBench kernel descriptions.#footnote[#link("https://github.com/MatthiasJReisinger/PolyBenchC-4.2.1/blob/master/polybench.pdf")[PolyBench 4.2.1 benchmark descriptions]]

1. Keep partial specialization only when the benchmark-visible inputs admit a believable split between stable spatial/layout state and changing work amount.
2. Revert to full specialization when the visible scalar inputs jointly define one pure problem instance, especially for dense linear-algebra kernels whose dimensions are mathematically coupled.
3. For convolution-like benchmarks, specialize a small kernel/configuration object only if the benchmark API actually exposes one as runtime state.

All PolyBench kernels not listed in the table below are therefore specialized fully.

#table(
  columns: (1.1fr, 2.4fr, 1.2fr),
  align: (left, left, left),
  [Name], [Description], [What to specialize],
  [`correlation`], [Computes Pearson correlation coefficients for an `N x M` data matrix and produces an `M x M` symmetric correlation matrix. This models a fixed feature schema with changing batch size.], [`M` only; keep `N` runtime-variable],
  [`covariance`], [Computes the covariance matrix of an `N x M` data matrix, again with a fixed attribute layout and changing number of rows. This is the same stable-schema / variable-batch-size scenario as `correlation`.], [`M` only; keep `N` runtime-variable],
  [`adi`], [Alternating-direction implicit heat diffusion over a 2D grid. A fixed spatial grid with a varying number of simulated time steps is a believable repeated-call scenario.], [`N` only; keep `TSTEPS` runtime-variable],
  [`fdtd_2d`], [A simplified 2D finite-difference time-domain electromagnetic simulation. The spatial mesh can stay fixed while the simulation horizon changes.], [`NX`, `NY`; keep `TMAX` runtime-variable],
  [`heat_3d`], [Iterates the heat equation over a 3D volume. The natural split is a fixed spatial domain with a variable iteration horizon.], [`N` only; keep `TSTEPS` runtime-variable],
  [`jacobi_1d`], [A 1D Jacobi-style stencil using a 3-point update pattern. The believable dynamic input is the number of time steps, not the domain size.], [`N` only; keep `TSTEPS` runtime-variable],
  [`jacobi_2d`], [A 2D Jacobi-style stencil using a 5-point update pattern. As with the other stencil kernels, the grid is stable while the iteration count changes.], [`N` only; keep `TSTEPS` runtime-variable],
  [`seidel_2d`], [A 2D Gauss-Seidel-style stencil using a 9-point update pattern. The fixed grid / variable iteration-horizon split remains the defensible partial-specialization scenario.], [`N` only; keep `TSTEPS` runtime-variable],
)
