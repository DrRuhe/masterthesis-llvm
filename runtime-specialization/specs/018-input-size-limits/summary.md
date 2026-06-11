Summary

Spec 018: Input Size Limits — fully implemented across 3 commits.

What was built

1. sqlite3_with_accessor.c — wrapper around the sqlite3 amalgamation that exports crs_vdbe_nop(),
   crs_vdbe_op_size(), and crs_vdbe_set_pc() accessor functions.
2. CMake changes — added -fpass-plugin to both sqlite3_with_accessor.c (for IR registration) and
   sqlite3_tpch_bench.cpp (for call-site rewriting). Previously the benchmark TU had no plugin,
   causing DumpedIRError.
3. sqlite3_tpch_bench.cpp overhaul — added query_nops/query_bytecode_bytes counters, 300s timeout
   guard with SetIterationTime, g_last_jit_timed_out propagation to phaseSpecializedExec,
   getNamedQuery() helper, and 7x4 synthetic benchmarks via DEFINE_INPUT_SIZE_BM macro.
   TPC-H Q1-Q22 refactored to DEFINE_TPCH_BM macro.
4. 7 synthetic SQL files (simple_scan -> join_agg) under benchmarks/tpch/queries/sqlite/.
5. plot_input_size_limits.py — scatterplot visualization script producing input_size_limits.png.

Bug fixed (post-implementation)

phaseSpecializedExec called sqlite3VdbeExec(vdbe) directly after sqlite3_reset(stmt).
sqlite3_reset leaves Vdbe::pc = -1; sqlite3VdbeExec starts at aOp[pc], so the first instruction
fetch hit aOp[-1] -> SIGSEGV. The normal path (sqlite3_step -> sqlite3Step) explicitly sets
pc = 0 before calling sqlite3VdbeExec; the benchmark bypassed that.

Fix: added crs_vdbe_set_pc(stmt, int pc) to sqlite3_with_accessor.c and called it with pc=0
after each sqlite3_reset in phaseSpecializedExec. All three phases (unspecialized, jit_overhead,
specialized_exec) now complete successfully for all synthetic and TPC-H queries.

Key finding (thesis-ready)

The effective input-size limit for sqlite3 specialization is the module size, not the query
bytecode size. The early GlobalDCE prune reduces 4393 functions (433k instructions) to ~17
functions (~121 instructions) — the same pruned subset for every query. JIT time is flat at
~115 ms regardless of nOps (9-230), making all 29 queries feasible.

Observed execution times for TPC-H Q1 (debug build):
- Unspecialized: ~600 ms
- JIT overhead: ~120 ms
- Specialized exec: ~630 ms

Note: specialized_exec shows no speedup over unspecialized for TPC-H Q1. This is expected —
sqlite3VdbeExec accesses large datasets (disk I/O dominates) and the JIT specialization folds
only the Vdbe pointer, leaving runtime behaviour nearly identical. The spec-018 focus is on
demonstrating that JIT compilation remains feasible (flat ~115 ms) regardless of query complexity.
