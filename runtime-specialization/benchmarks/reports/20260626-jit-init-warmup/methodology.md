# JIT Init Warmup Provenance

## Goal

Provide a citation-ready number for the one-time runtime-specializer startup overhead referenced by the size-scaling discussion in `docs/thesis.typ`.

## Isolated Rerun

On 2026-06-26, an isolated rerun of `BM_g:synthetic;n:jit_init;t:jit_overhead;` from `AllBenchmarks` was first attempted with:

```bash
/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/llvm/build/release/tools/runtime-specialization/benchmarks/AllBenchmarks \
  --benchmark_filter='BM_g:synthetic;n:jit_init;t:jit_overhead;' \
  --benchmark_out=benchmarks/reports/20260626-jit-init-warmup/probe.json \
  --benchmark_out_format=json
```

That first rerun aborted with `clangRuntimeSpecializer::ClangRuntimeSpecializerDumpedIRError` before producing a usable measurement.
The failure was traced to two benchmark-infrastructure issues:

- `BM_jit_init_warmup` lived in `AllBenchmarks_main.cpp`, which is not plugin-rewritten, so its original `specializeOnly()` call never received a resolved target name.
- The IR metadata used for blob ownership indexed specialization call-site names but did not reliably publish the defining TU's externally visible kernel definitions, which caused `mypow_bench` lookup failures in the isolated benchmark.

After fixing both issues and rebuilding `AllBenchmarks`, the isolated rerun succeeded. The successful artifacts live in `benchmarks/reports/20260626-jit-init-warmup-fixed2/`.

## Measured Startup Cost

The successful single-iteration isolated run reported:

```text
BM_g:synthetic;n:jit_init;t:jit_overhead;/iterations:1/manual_time
real_time = 3206265409 ns
cpu_time  = 3205416466 ns
```

This gives a thesis-facing one-time startup overhead of **3.206 s**, which is the number that should replace the earlier fallback estimate of "roughly 3.0 s".

## Relationship To The Earlier Estimate

The earlier fallback estimate from the first-call study remains directionally correct:

- derived estimate: `3032.502429 ms`
- direct isolated rerun: `3206.265409 ms`

The isolated rerun is the better citation because it measures the startup warmup directly instead of inferring it from a contaminated first-call data point.

## Fallback Evidence Used

Before the fix, the startup cost had to be inferred from the thesis-grade UC first-call study plus the earlier warmup investigation:

- `benchmarks/reports/thesis-figures/rq1/rq1_first_call_quadrants.csv`
  - `default, uc1_sql, count_matching_rows, low, SMALL`: `jit_ms = 3094.720258`
  - `default, uc1_sql, count_matching_rows, low, MEDIUM`: `jit_ms = 62.217829`
- `benchmarks/reports/260531-09-31-optimize-pipeline/reflection.md`
  - documents the same artifact qualitatively as the one-time startup cost paid by the first JIT benchmark

## Derived Estimate

Using the same kernel and pipeline, the difference between the contaminated first `SMALL` run and the later steady-state `MEDIUM` run is:

```text
3094.720258 ms - 62.217829 ms = 3032.502429 ms
```

That fallback derivation yielded **roughly 3.0 s**. It has now been superseded by the direct isolated rerun above.

## Interpretation

The direct isolated number should be read as:

- a process-level one-time startup cost for the first specialization in that study configuration,
- not as the steady-state per-kernel `jit_overhead`,
- and not as a precision claim beyond the order of magnitude.

The thesis text should therefore phrase this as an approximate startup overhead, not as an exact benchmark constant.
