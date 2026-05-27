# Q9: What is the practical break-even call count for each use case?

## Important Context

Break-even call count = jit_ns / (unspec_ns − spec_ns): the number of calls at which the JIT overhead is amortized by the per-call execution speedup. A `None` / negative denominator means spec_ns ≥ unspec_ns — specialization never pays off at any call count.

Data from `v_optim_breakeven` (study `uc_optim_iter2_20260521`, best Optuna trial per kernel):

| Kernel          | jit_ms | spec_ms | unspec_ms | break-even |
|-----------------|--------|---------|-----------|------------|
| generic_sort    | ~51    | ~282    | ~754      | **0.1**    |
| multi_key_sort  | ~54    | ~694    | ~968      | **0.2**    |
| struct_sort     | ~50    | ~674    | ~1050     | **0.1**    |
| box_filter      | ~50    | ~381    | ~1102     | **0.1**    |
| edge_detection  | ~48    | ~136    | ~208      | **0.7**    |
| separable_gauss | ~55    | ~300    | ~585      | **0.2**    |
| apply_row_delta | ~42    | ~18     | ~30       | **3.3**    |
| batch_delta     | ~42    | ~62     | ~12       | **None**   |
| multi_agg_delta | ~41    | ~20     | ~13       | **None**   |
| grouped_count   | ~43    | ~12     | ~28       | **2.6**    |
| grouped_sum     | ~43    | ~12     | ~28       | **2.6**    |
| grouped_minmax  | ~44    | ~12     | ~28       | **2.7**    |
| column_scan     | ~2367 | ~8     | ~8.3      | **~800**   |
| count_matching  | ~2367 | ~8     | ~8.3      | **~800**   |
| multi_predicate | ~55    | ~54     | ~16       | **None**   |
| email_match     | ~42    | ~95     | ~116      | **2.0**    |
| url_match       | ~42    | ~96     | ~116      | **2.0**    |
| multi_pattern   | ~42    | ~96     | ~115      | **2.0**    |

Note: uc1_sql kernels show a bimodal split — `count_matching_rows` has a ~2357 ms JIT overhead (large module, `large_module_max` path), while `multi_predicate` has spec_ms > unspec_ms.

---

## Investigation

### Analyze kernels where specialization never pays off

Three kernels have `break_even = None` (spec_ms ≥ unspec_ms under the best Optuna config):

**batch_delta** (`uc8_ivm`): spec_ms ≈ 62 ms, unspec_ms ≈ 12 ms. The specialized version is **5× slower** than unspecialized. The JIT overhead of ~42 ms means even the most conservative call-count argument fails — the first call already costs 104 ms vs 12 ms unspecialized.

Root cause: `batch_delta` applies a delta to a batch of rows. The function is likely already AOT-vectorized; baking in the batch size or delta type doesn't eliminate any loop overhead because the inner loop is a simple memory copy / addition that the AOT compiler handles optimally.

**multi_agg_delta** (`uc8_ivm`): spec_ms ≈ 20 ms, unspec_ms ≈ 13 ms. The specialized version is **55 % slower**. Similar issue: the multi-aggregate kernel has a fixed computation structure where specializing the aggregation functions doesn't help — the overhead comes from the specializer adding calling-convention wrappers or extra inlining that the code generator can't undo.

**multi_predicate** (`uc1_sql`): spec_ms ≈ 54 ms, unspec_ms ≈ 16 ms. The specialized version is **3.4× slower**. The unspecialized predicate runs very fast (16 ms), and specialization with a concrete predicate value should help — but something in the JIT compilation path is producing slow code. This may be the same issue as batch_delta: the unspecialized version benefits from SIMD predicate evaluation that the JIT disrupts.

**What to look for**: For all three "never pays off" kernels, collect `jit_analysis` pass traces to see the post-specialization instruction count vs unspecialized. If the JIT IR has more instructions than the AOT IR for the same computation, the specialization is counterproductive.

### Analyze break-even distribution and practical implications

**Group 1: Break-even < 1 call** (sort, conv): `generic_sort`, `box_filter`, `struct_sort`, `separable_gaussian`, `multi_key_sort`. These kernels benefit so much from specialization that even the first call pays for the JIT overhead.
- Break-even of 0.1 means: after 1 call, you've already recovered 10× the JIT cost.
- For any practical workload where the kernel is called ≥ 1 time, specialization is always beneficial.

**Group 2: Break-even 2–4 calls** (IVM, groupby, DFA): `apply_row_delta`, `grouped_count/sum/minmax`, `email_match`, `url_match`, `multi_pattern_match`. Specialization pays off after a few calls.
- For streaming workloads where a function is called O(1000)× per session, this is trivially satisfied.
- For one-shot workloads, the 3.3× break-even for `apply_row_delta` means 3 calls suffice — still very practical.

**Group 3: Break-even ~800 calls** (count_matching, column_scan): These kernels have very large JIT overhead (~2357 ms) due to the `large_module_max` parameter selecting a larger JIT module path. At 2357 ms JIT overhead with only ~0.3 ms per-call speedup, it takes ~800 calls to recoup. This is a configuration artifact — these kernels with a more targeted JIT module (smaller module, faster JIT) would have much lower break-even.

### Analyze how break-even changes between P0 and P1 for uc14_sort

From the ablation data for generic_sort:

| Config  | jit_ms | spec_ms | unspec_ms | break-even |
|---------|--------|---------|-----------|------------|
| P0 best | ~51    | ~282    | ~754      | ~0.11      |
| P1 agg  | ~48    | ~234    | ~754      | ~0.09      |

P1 reduces spec_ms by 17 % (282 → 234 ms), improving break-even from 0.11 to 0.09 calls. The JIT overhead also decreases slightly under P1 (~48 vs 51 ms, since P1's inliner is structurally different). The net effect: P1 is marginally better for break-even on generic_sort.

**This directly answers spec SQ5**: For `uc14_sort`, the break-even count is already < 1 call under both P0 and P1 — the pipeline choice doesn't change the practical conclusion (specialization always pays off after even a single call).

### Analyze a rule-of-thumb for specialization decision

Based on the empirical data, a practical rule can be derived:

1. **If the kernel has function-pointer or vtable calls in the hot path** (sort comparators, polymorphic filters): expect break-even < 1 call and spec_ms / unspec_ms < 0.5. Always specialize.

2. **If the kernel has scalar loop computation with constants that become fixed** (conv, groupby, DFA): expect break-even 1–5 calls and speedup 1.2–2.5×. Specialize if the kernel is called ≥ 5 times per session.

3. **If the kernel is already AOT-vectorized with no runtime-variable branch** (batch_delta, multi_agg_delta, multi_predicate with simple constant predicates): expect spec_ms ≥ unspec_ms. **Do not specialize**.

The distinguishing feature is whether there is a "specializable operation" in the hot path — an indirect call, a branch on a constant, or a computation that the JIT can constant-fold away. If no such operation exists, the JIT overhead is a net loss.

---

## Conclusion

Break-even call counts span the full range from < 1 (sort/conv kernels where one call suffices) to Never (kernels where specialization is counterproductive). The distribution splits into three clear groups:
- **Always beneficial** (break-even < 1): sort, conv
- **Beneficial with a few calls** (break-even 2–5): groupby, IVM apply-row-delta, DFA matching
- **Never beneficial** (spec ≥ unspec): IVM batch-delta, multi-agg-delta, SQL multi-predicate

The pipeline choice (P0 vs P1) has minimal effect on break-even: P1 slightly reduces spec_ms for sort (improving break-even from 0.11 → 0.09) but makes other groups worse. The break-even criterion is dominated by the structural specialization gain, not pipeline tuning.

---

## Further Questions/Directions of Research

- The `count_matching_rows` and `column_scan` kernels with ~800 break-even are a configuration artifact. Re-measure with a more targeted JIT module (using function-indexed blob loading to include only the target function's dependencies). The break-even should drop to < 1.
- For `batch_delta` and `multi_agg_delta`: investigate whether a different specialization interface (e.g., specializing the element count rather than the delta type) would change the profitability profile.
- Implement the rule-of-thumb as an automatic skip condition in the specializer: if the post-prune JIT module has no indirect calls and the target function's instruction count is ≤ N (configurable), skip specialization and use the unspecialized function.
- For the thesis: present the break-even distribution as a key empirical finding — it directly answers SQ5 ("under what conditions does specialization pay off?") with concrete numbers.
