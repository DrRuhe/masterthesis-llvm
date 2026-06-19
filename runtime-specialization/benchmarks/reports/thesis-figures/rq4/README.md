# Best UC Pipeline JSONs

This directory contains the exported per-kernel UC pipeline selections that can
be reused by the benchmark harness.

## What these JSONs are for

The UC benchmark harness can load a JSON file with one pipeline configuration
per UC kernel. When enabled, the harness applies that kernel-level pipeline to
all benchmark variants of the same kernel, including different sizes and
abstraction levels.

This is intended for running later benchmark suites with the previously
selected "best" pipeline instead of manually setting a single global default.

## How to generate the JSON

Run the exporter from the `llvm/runtime-specialization` repo root:

```bash
python3 benchmarks/reporting/export_best_uc_pipelines.py \
  --output benchmarks/reports/thesis-figures/rq4/best_uc_pipelines.json
```

By default this writes three files:

- `best_uc_pipelines.json`
  The main artifact. This currently uses the amortized objective `U / S` and is
  the recommended file for running benchmarks across sizes.
- `best_uc_pipelines_amortized.json`
  Explicit copy of the amortized artifact.
- `best_uc_pipelines_first_call.json`
  Alternative ranking using the first-call objective `U / (S + J)`.

Useful optional flags:

- `--db PATH`
  Use a different DuckDB database. Default: `benchmarks/benchmarks.duckdb`
- `--study-name NAME`
  Export from a different Optuna study. Default:
  `uc_optim_iter3_20260601`
- `--metric amortized|first_call|both`
  Control which objective is exported. Default: `both`

## Where the JSON is typically saved

The standard location is this directory:

```text
benchmarks/reports/thesis-figures/rq4/
```

The benchmark and thesis integration both already use this location, so it is
best to keep the generated files here unless you have a reason to stage them
elsewhere.

## Which JSON should I use?

For normal UC benchmark runs, use:

```text
benchmarks/reports/thesis-figures/rq4/best_uc_pipelines.json
```

This is the amortized `U / S` selection and is the recommended default because
the JSON is later reused across benchmark sizes, while the one-time JIT term in
`U / (S + J)` is much more size-dependent.

Use `best_uc_pipelines_first_call.json` only when you explicitly want to study
the first-call objective.

## How to run benchmarks using the JSON

Build `AllBenchmarks` in your usual build directory, then pass the JSON either
through the dedicated CLI flag or through the environment variable.

Preferred form:

```bash
/path/to/AllBenchmarks \
  --uc-pipeline-config=/absolute/or/relative/path/to/best_uc_pipelines.json \
  --benchmark_filter='BM_g:uc1_sql;.*'
```

Equivalent environment-variable form:

```bash
CRS_UC_PIPELINE_CONFIG_JSON=/absolute/or/relative/path/to/best_uc_pipelines.json \
  /path/to/AllBenchmarks \
  --benchmark_filter='BM_g:uc1_sql;.*'
```

Example using the standard artifact location:

```bash
/path/to/AllBenchmarks \
  --uc-pipeline-config=benchmarks/reports/thesis-figures/rq4/best_uc_pipelines.json \
  --benchmark_filter='BM_g:uc1_sql;n:count_matching_rows;.*'
```

## What the loader matches on

- The JSON must contain exactly one entry for each of the 18 UC kernels.
- The loader matches by kernel name (`;n:<kernel>;`) in the benchmark name.
- The selected pipeline is then reused for all sizes and abstractions of that
  kernel.
- Non-UC benchmarks ignore this JSON.

## Common failure modes

- Wrong path: the harness fails if the JSON file cannot be read.
- Wrong schema: the harness expects a top-level `entries` array with an
  `options` object per kernel.
- Missing kernel coverage: the harness rejects JSONs that do not cover all 18
  expected UC kernels exactly once.
- Using the first-call JSON accidentally: this is valid, but usually not what
  you want for cross-size benchmark campaigns.
