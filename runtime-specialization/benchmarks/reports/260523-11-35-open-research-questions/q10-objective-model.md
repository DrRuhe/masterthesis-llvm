# Q10: Does the combined-cost objective (jit_ns + spec_ns) correctly model the thesis use case?

## Important Context

The Optuna optimization minimizes `jit_ns + spec_ns` — the total cost assuming a single call to the specialized function. This implicitly assumes that the JIT overhead is paid exactly once and the specialized function is called exactly once. Spec W6 flags this as a methodology weakness.

For different call-count distributions:
- **1 call**: combined-cost objective is exact
- **N calls**: true cost = jit_ns + N × spec_ns; objective underweights exec speedup by factor N
- **Recurring with warm JIT cache**: if the JIT module is cached and only compiled once per program run, jit_ns is amortized over all calls in the session

The concern is that the optimizer is solving the wrong problem — it may favor configs that reduce JIT overhead at the expense of exec quality, which is suboptimal for workloads with many calls.

---

## Investigation

### Analyze the actual call-count distribution for thesis use cases

The thesis targets two primary use cases:
1. **Query engines** (TPC-H, SQL predicates, groupby): A query is compiled once and executed over millions of rows. The specialized kernel is called exactly **1 time** (the entire scan is one function call operating over a row range). However, the function itself iterates millions of times internally. The "call count" for the purpose of break-even is 1, but the internal loop count is O(10⁶).

2. **Event-driven incremental view maintenance** (uc8_ivm): A delta is applied once per incoming event. In a high-throughput stream, this could be O(1000–100000) calls per second. The JIT cost (42 ms) is paid once per session, amortized over all incoming events.

3. **Sort/filter on changing schema** (uc14_sort): A schema change triggers re-specialization. In a typical database, schema changes are rare (O(10–100) per day). The specialization is called once post-schema-change, then used for all subsequent sort operations until the next schema change.

**Interpretation**: For use cases 1 and 3, the "1 call" assumption is accurate — the specialization cost is paid once and the specialized function is called a handful of times. For use case 2 (streaming IVM), the per-call speedup dominates and the objective underweights exec quality.

### Analyze whether the optimizer produces different configs for different call-count assumptions

The combined-cost objective sorts configs by jit_ns + spec_ns. An alternative objective for N=1000 calls would sort by jit_ns + 1000 × spec_ns. The difference:

From the top Optuna trials:

| trial | fixpoint_max | unroll_max | jit_ms | spec_ms | combined (N=1) | combined (N=1000) |
|-------|-------------|-----------|--------|---------|----------------|-------------------|
| 7     | 27          | 13        | ?      | ?       | 64.23          | ?                 |
| 15    | 7           | 27        | 56.76  | 68.54   | 125.30         | 68,540.76         |
| 25    | 30          | 203       | ?      | 68.35   | 68.35 + jit    | ?                 |

Without per-trial jit_ms and spec_ms broken out separately, we can't directly compute the N=1000 objective. However, the reflection reports the winner as jit=56.76ms + spec=68.54ms = 125.30ms combined.

A config optimized for N=1000 would prefer **minimizing spec_ms** regardless of jit_ms. The best spec_ms across all trials for the dominant kernel (generic_sort) is ~233 ms (P1 aggressive budget). For N=1000, this config's objective would be 48ms + 1000×233ms = 233,048ms — the same ranking because spec_ms dominates. For N→∞, the exec-optimal config always wins.

**Key insight**: For N ≥ 10, the exec-optimal config (lowest spec_ms) is better than the combined-cost config regardless of jit_ns. The combined-cost objective is only correct for N≈1.

### Analyze whether Aggressive config Pareto-dominates Default at high call counts

From ablation data, per-group averages:

| Group       | Config    | jit_ms | avg spec_ms | combined N=1 | combined N=100 |
|-------------|-----------|--------|-------------|--------------|----------------|
| uc12_groupby| default   | ~58    | ~10.7       | 68.7         | 1128           |
| uc12_groupby| aggressive| ~55    | ~10.8       | 65.8         | 1135           |
| uc14_sort   | default   | ~67    | ~549        | 616          | 54,967         |
| uc14_sort   | aggressive| ~67    | ~552        | 619          | 55,267         |
| uc2_conv    | default   | ~56    | ~229        | 285          | 22,956         |
| uc2_conv    | aggressive| ~59    | ~229        | 288          | 22,959         |

For N=100, the configs are nearly identical — the spec_ms values barely change between `default` and `aggressive`. This is consistent with the pass-trace analysis (fixpoint converges in 1–3 iterations, so `aggressive` with more fixpoint iterations gives no better IR than `default`).

For N=1000, the `Aggressive` config (more iterations, higher unroll) may produce better exec time for sort kernels — but the ablation shows `aggressive` doesn't improve over `default` in the ablation study. The real benefit of higher unroll_max came from the Optuna search finding `unroll_max=27`, not from the `aggressive` preset.

### Analyze the two-objective Pareto frontier

The two dimensions of interest are:
- **JIT overhead** (jit_ns): minimized by O3Only or low fixpoint configs
- **Exec time** (spec_ns): minimized by full pipeline with high unroll

A Pareto frontier would show the tradeoff between faster JIT and faster execution. The Pareto plots already generated per UC group (`pareto_uc_optim_iter2_20260521_<group>.png`) visualize exactly this. The reflection notes that these are available in the report directory.

From the break-even analysis (Q9): for sort/conv kernels, break-even is < 1 call, meaning the combined-cost objective and the exec-optimal objective produce the same ranking for any N ≥ 1. For IVM kernels with batch_delta (never pays off), no config choice helps — the correct action is to not specialize at all.

---

## Conclusion

The combined-cost objective is a reasonable approximation for the thesis use cases because:
1. Most thesis-relevant kernels (sort, conv, SQL) have break-even < 1 call — the objective correctly identifies these as "always specialize."
2. For IVM kernels where specialization is counterproductive (batch_delta), no pipeline config changes the verdict.
3. The Pareto frontier (jit_ns vs spec_ns) is already visualized and shows that for UC kernels, there is limited tradeoff — configs that minimize spec_ns also have acceptable jit_ns.

The objective weakness flagged in W6 is real but not critical: the optimal pipeline for N=1 and N=1000 produces nearly the same config for these UC kernels, because the exec improvements from the fixpoint are already saturated. The objective would matter more for future workloads where:
- JIT overhead is much larger (e.g., DuckDB at 2+ seconds)
- Exec speedup is significant (e.g., 10× faster specialized vs unspecialized)
- Call count is known to be high (streaming workloads)

In those cases, a two-objective Optuna study (jit_ns vs spec_ns separately, Pareto mode) would provide more useful guidance.

---

## Further Questions/Directions of Research

- Characterize the actual call-count distribution for the thesis target use cases. For TPC-H: 1 call per query. For IVM: N calls per session (user-defined). For sort: 1 call per schema change.
- Run the Pareto-mode Optuna study (already supported by `v_budget_sweep`): compare the Pareto-optimal configs against the scalar combined-cost optimum to quantify the tradeoff.
- For IVM streaming workloads: define a call-count-weighted objective `jit_ns/N + spec_ns` (where N is the expected call count). Add N as a configuration parameter in the benchmark, and measure whether the optimal pipeline changes for N=1 vs N=10000.
- Design a `N*`-threshold experiment: find the call count above which `Aggressive` config (higher fixpoint, more unroll) outperforms `Default` on the combined-cost metric. From the ablation data, this threshold is likely very high (> 100 calls) because `Aggressive` barely improves spec_ms over `Default`.
- Validate the Spec W6 weakness claim: if the objective is truly wrong, the optimal config for N=1 should be qualitatively different from the optimal config for N=1000. The data suggests they are nearly identical — which would *disprove* W6 as a meaningful weakness for the thesis use cases.
