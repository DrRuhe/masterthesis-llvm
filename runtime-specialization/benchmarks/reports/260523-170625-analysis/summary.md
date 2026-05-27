# Analysis Report — 20260523-170625-analysis

| # | Benchmark | Status | Elapsed (s) | Artifacts |
|---|-----------|--------|-------------|-----------|
| 1 | `BM_g:uc1_sql;n:count_matching_rows;a:low;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 3 |
| 2 | `BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 5 |
| 3 | `BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 5 |
| 4 | `BM_g:uc1_sql;n:multi_predicate;a:low;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 5 |
| 5 | `BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 5 |
| 6 | `BM_g:uc1_sql;n:multi_predicate;a:abstract;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 5 |
| 7 | `BM_g:uc1_sql;n:column_scan;a:low;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 5 |
| 8 | `BM_g:uc1_sql;n:column_scan;a:tradeoff;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 5 |
| 9 | `BM_g:uc1_sql;n:column_scan;a:abstract;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 5 |
| 10 | `BM_g:uc2_conv;n:separable_gaussian;a:low;s:MEDIUM;t:jit_analysis;/6/iterations:1/manual_time` | ❌ fail | 0.1 | 3 |
| 11 | `BM_g:uc2_conv;n:box_filter;a:low;s:MEDIUM;t:jit_analysis;/6/iterations:1/manual_time` | ❌ fail | 0.1 | 5 |
| 12 | `BM_g:uc2_conv;n:edge_detection;a:low;s:MEDIUM;t:jit_analysis;/6/iterations:1/manual_time` | ❌ fail | 0.1 | 5 |
| 13 | `BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:MEDIUM;t:jit_analysis;/6/iterations:1/manual_time` | ❌ fail | 0.1 | 5 |
| 14 | `BM_g:uc2_conv;n:box_filter;a:tradeoff;s:MEDIUM;t:jit_analysis;/6/iterations:1/manual_time` | ❌ fail | 0.1 | 5 |
| 15 | `BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:MEDIUM;t:jit_analysis;/6/iterations:1/manual_time` | ❌ fail | 0.1 | 5 |
| 16 | `BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:MEDIUM;t:jit_analysis;/6/iterations:1/manual_time` | ❌ fail | 0.1 | 5 |
| 17 | `BM_g:uc2_conv;n:box_filter;a:abstract;s:MEDIUM;t:jit_analysis;/6/iterations:1/manual_time` | ❌ fail | 0.1 | 5 |
| 18 | `BM_g:uc2_conv;n:edge_detection;a:abstract;s:MEDIUM;t:jit_analysis;/6/iterations:1/manual_time` | ❌ fail | 0.1 | 1 |
| 19 | `BM_g:uc7_dfa;n:email_match;a:low;s:MEDIUM;t:jit_analysis;/52428800/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 20 | `BM_g:uc7_dfa;n:email_match;a:tradeoff;s:MEDIUM;t:jit_analysis;/52428800/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 21 | `BM_g:uc7_dfa;n:email_match;a:abstract;s:MEDIUM;t:jit_analysis;/52428800/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 22 | `BM_g:uc7_dfa;n:url_match;a:low;s:MEDIUM;t:jit_analysis;/52428800/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 23 | `BM_g:uc7_dfa;n:url_match;a:tradeoff;s:MEDIUM;t:jit_analysis;/52428800/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 24 | `BM_g:uc7_dfa;n:url_match;a:abstract;s:MEDIUM;t:jit_analysis;/52428800/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 25 | `BM_g:uc7_dfa;n:multi_pattern_match;a:low;s:MEDIUM;t:jit_analysis;/52428800/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 26 | `BM_g:uc7_dfa;n:multi_pattern_match;a:tradeoff;s:MEDIUM;t:jit_analysis;/52428800/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 27 | `BM_g:uc7_dfa;n:multi_pattern_match;a:abstract;s:MEDIUM;t:jit_analysis;/52428800/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 28 | `BM_g:uc8_ivm;n:apply_row_delta;a:low;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 29 | `BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 30 | `BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 31 | `BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 32 | `BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 33 | `BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 34 | `BM_g:uc8_ivm;n:batch_delta;a:low;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 35 | `BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 36 | `BM_g:uc8_ivm;n:batch_delta;a:abstract;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 37 | `BM_g:uc12_groupby;n:grouped_sum;a:low;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 38 | `BM_g:uc12_groupby;n:grouped_sum;a:tradeoff;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 39 | `BM_g:uc12_groupby;n:grouped_sum;a:abstract;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 40 | `BM_g:uc12_groupby;n:grouped_count;a:low;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 41 | `BM_g:uc12_groupby;n:grouped_count;a:tradeoff;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 42 | `BM_g:uc12_groupby;n:grouped_count;a:abstract;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 43 | `BM_g:uc12_groupby;n:grouped_minmax;a:low;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 44 | `BM_g:uc12_groupby;n:grouped_minmax;a:tradeoff;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 45 | `BM_g:uc12_groupby;n:grouped_minmax;a:abstract;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 46 | `BM_g:uc14_sort;n:generic_sort;a:low;s:MEDIUM;t:jit_analysis;/8000000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 47 | `BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:MEDIUM;t:jit_analysis;/8000000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 48 | `BM_g:uc14_sort;n:generic_sort;a:abstract;s:MEDIUM;t:jit_analysis;/8000000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 49 | `BM_g:uc14_sort;n:struct_sort;a:low;s:MEDIUM;t:jit_analysis;/8500000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 50 | `BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:MEDIUM;t:jit_analysis;/8500000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 51 | `BM_g:uc14_sort;n:struct_sort;a:abstract;s:MEDIUM;t:jit_analysis;/8500000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 52 | `BM_g:uc14_sort;n:multi_key_sort;a:low;s:MEDIUM;t:jit_analysis;/6500000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 53 | `BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:MEDIUM;t:jit_analysis;/6500000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |
| 54 | `BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:MEDIUM;t:jit_analysis;/6500000/iterations:1/manual_time` | ❌ fail | 0.1 | 0 |

## Failures

- **BM_g:uc1_sql;n:count_matching_rows;a:low;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc1_sql;n:count_matching_rows;a:tradeoff;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc1_sql;n:count_matching_rows;a:abstract;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc1_sql;n:multi_predicate;a:low;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc1_sql;n:multi_predicate;a:tradeoff;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc1_sql;n:multi_predicate;a:abstract;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc1_sql;n:column_scan;a:low;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc1_sql;n:column_scan;a:tradeoff;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc1_sql;n:column_scan;a:abstract;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc2_conv;n:separable_gaussian;a:low;s:MEDIUM;t:jit_analysis;/6/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc2_conv;n:box_filter;a:low;s:MEDIUM;t:jit_analysis;/6/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc2_conv;n:edge_detection;a:low;s:MEDIUM;t:jit_analysis;/6/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc2_conv;n:separable_gaussian;a:tradeoff;s:MEDIUM;t:jit_analysis;/6/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc2_conv;n:box_filter;a:tradeoff;s:MEDIUM;t:jit_analysis;/6/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc2_conv;n:edge_detection;a:tradeoff;s:MEDIUM;t:jit_analysis;/6/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc2_conv;n:separable_gaussian;a:abstract;s:MEDIUM;t:jit_analysis;/6/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc2_conv;n:box_filter;a:abstract;s:MEDIUM;t:jit_analysis;/6/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc2_conv;n:edge_detection;a:abstract;s:MEDIUM;t:jit_analysis;/6/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc7_dfa;n:email_match;a:low;s:MEDIUM;t:jit_analysis;/52428800/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc7_dfa;n:email_match;a:tradeoff;s:MEDIUM;t:jit_analysis;/52428800/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc7_dfa;n:email_match;a:abstract;s:MEDIUM;t:jit_analysis;/52428800/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc7_dfa;n:url_match;a:low;s:MEDIUM;t:jit_analysis;/52428800/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc7_dfa;n:url_match;a:tradeoff;s:MEDIUM;t:jit_analysis;/52428800/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc7_dfa;n:url_match;a:abstract;s:MEDIUM;t:jit_analysis;/52428800/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc7_dfa;n:multi_pattern_match;a:low;s:MEDIUM;t:jit_analysis;/52428800/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc7_dfa;n:multi_pattern_match;a:tradeoff;s:MEDIUM;t:jit_analysis;/52428800/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc7_dfa;n:multi_pattern_match;a:abstract;s:MEDIUM;t:jit_analysis;/52428800/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc8_ivm;n:apply_row_delta;a:low;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc8_ivm;n:apply_row_delta;a:tradeoff;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc8_ivm;n:apply_row_delta;a:abstract;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc8_ivm;n:multi_agg_delta;a:tradeoff;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc8_ivm;n:multi_agg_delta;a:abstract;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc8_ivm;n:batch_delta;a:low;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc8_ivm;n:batch_delta;a:tradeoff;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc8_ivm;n:batch_delta;a:abstract;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc12_groupby;n:grouped_sum;a:low;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc12_groupby;n:grouped_sum;a:tradeoff;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc12_groupby;n:grouped_sum;a:abstract;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc12_groupby;n:grouped_count;a:low;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc12_groupby;n:grouped_count;a:tradeoff;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc12_groupby;n:grouped_count;a:abstract;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc12_groupby;n:grouped_minmax;a:low;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc12_groupby;n:grouped_minmax;a:tradeoff;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc12_groupby;n:grouped_minmax;a:abstract;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc14_sort;n:generic_sort;a:low;s:MEDIUM;t:jit_analysis;/8000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc14_sort;n:generic_sort;a:tradeoff;s:MEDIUM;t:jit_analysis;/8000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc14_sort;n:generic_sort;a:abstract;s:MEDIUM;t:jit_analysis;/8000000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc14_sort;n:struct_sort;a:low;s:MEDIUM;t:jit_analysis;/8500000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc14_sort;n:struct_sort;a:tradeoff;s:MEDIUM;t:jit_analysis;/8500000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc14_sort;n:struct_sort;a:abstract;s:MEDIUM;t:jit_analysis;/8500000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc14_sort;n:multi_key_sort;a:low;s:MEDIUM;t:jit_analysis;/6500000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc14_sort;n:multi_key_sort;a:tradeoff;s:MEDIUM;t:jit_analysis;/6500000/iterations:1/manual_time**: benchmark process exited with code -11
- **BM_g:uc14_sort;n:multi_key_sort;a:abstract;s:MEDIUM;t:jit_analysis;/6500000/iterations:1/manual_time**: benchmark process exited with code -11
