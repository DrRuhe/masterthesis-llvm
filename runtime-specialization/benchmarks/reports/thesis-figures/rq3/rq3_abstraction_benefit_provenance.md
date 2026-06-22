# RQ3 Abstraction Benefit Plot Provenance

- Source database: `benchmarks/benchmarks.duckdb`
- Study selector: exact `corpus_uc_first_call_quadrants_20260620_160209` plus any `corpus_uc_first_call_quadrants_20260620_160209__*` shards
- Matched study names: `corpus_uc_first_call_quadrants_20260620_160209`
- Config slice: `default`
- Size slice: `MEDIUM`
- Abstraction levels: `low`, `tradeoff`, `abstract`
- Phases: `unspecialized`, `specialized_exec`
- Kernel count: 18
- Distinct run count for this slice: 5
- Aggregation: median `real_time` per `(kernel, abstraction, phase)` across the matched runs
- Normalization: each point is divided by the same kernel's `low` + `unspecialized` median runtime

## Best-Practice Status

- `best_practice_full=True`: 5 runs

## Files

- `rq3_abstraction_benefit.csv`: normalized per-kernel plotting data
- `rq3_abstraction_benefit_aggregate.png`: thesis aggregate plot with points, boxplots, and geomeans
- `rq3_abstraction_benefit_per_kernel.png`: 18-kernel exploration grid
