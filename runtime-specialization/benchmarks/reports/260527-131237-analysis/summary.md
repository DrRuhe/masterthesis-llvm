# Analysis Report — 260527-131237-analysis

| # | Benchmark | Status | Elapsed (s) | Artifacts |
|---|-----------|--------|-------------|-----------|
| 1 | `BM_g:uc1_sql;n:count_matching_rows;a:low;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ✅ pass | 1.0 | 3 |
| 2 | `BM_g:uc1_sql;n:multi_predicate;a:low;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ✅ pass | 1.0 | 5 |
| 3 | `BM_g:uc1_sql;n:column_scan;a:low;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ✅ pass | 1.0 | 5 |
| 4 | `BM_g:uc2_conv;n:separable_gaussian;a:low;s:MEDIUM;t:jit_analysis;/6/iterations:1/manual_time` | ✅ pass | 1.0 | 3 |
| 5 | `BM_g:uc2_conv;n:box_filter;a:low;s:MEDIUM;t:jit_analysis;/6/iterations:1/manual_time` | ✅ pass | 1.0 | 5 |
| 6 | `BM_g:uc2_conv;n:edge_detection;a:low;s:MEDIUM;t:jit_analysis;/6/iterations:1/manual_time` | ✅ pass | 1.0 | 5 |
| 7 | `BM_g:uc7_dfa;n:email_match;a:low;s:MEDIUM;t:jit_analysis;/52428800/iterations:1/manual_time` | ✅ pass | 1.0 | 3 |
| 8 | `BM_g:uc7_dfa;n:url_match;a:low;s:MEDIUM;t:jit_analysis;/52428800/iterations:1/manual_time` | ✅ pass | 1.0 | 5 |
| 9 | `BM_g:uc7_dfa;n:multi_pattern_match;a:low;s:MEDIUM;t:jit_analysis;/52428800/iterations:1/manual_time` | ✅ pass | 1.0 | 5 |
| 10 | `BM_g:uc8_ivm;n:apply_row_delta;a:low;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ✅ pass | 1.0 | 3 |
| 11 | `BM_g:uc8_ivm;n:multi_agg_delta;a:low;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ✅ pass | 1.0 | 5 |
| 12 | `BM_g:uc8_ivm;n:batch_delta;a:low;s:MEDIUM;t:jit_analysis;/10000000/iterations:1/manual_time` | ✅ pass | 1.0 | 5 |
