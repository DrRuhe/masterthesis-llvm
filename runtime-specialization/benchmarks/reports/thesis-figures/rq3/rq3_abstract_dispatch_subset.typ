// Abstract-tier kernels where specialization fixes one concrete operator implementation.
// Source: benchmarks/reports/thesis-figures/rq3/rq3_abstraction_detail.csv
#table(
  columns: (1.5fr, 1fr, 1fr, 1fr),
  table.header([Kernel], [Low], [Tradeoff], [Abstract]),
  [apply_row_delta], [2.984x], [2.895x], [2.349x],
  [grouped_count], [3.209x], [2.996x], [2.784x],
  [grouped_sum], [2.865x], [2.405x], [2.859x],
  [multi_agg_delta], [2.698x], [2.264x], [4.063x],
  [Geomean], [2.933x], [2.621x], [2.952x],
)
