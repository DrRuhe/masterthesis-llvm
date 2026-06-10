# TPC-H Module-Size Scope Statement

## Module Statistics

- **Blob size**: 4,078 KB (4,176,428 bytes)
- **Function count**: 2,017 non-declaration functions
- **Instruction count**: 255,342 LLVM IR instructions

## Analysis

TPC-H (sqlite3VdbeExec) has 255,342 instructions / 2,017 functions / 4,078 KB blob.
UC MEDIUM modules average ~21k instructions.
JIT overhead for TPC-H exceeds 2 minutes under P0 (measured 2026-05-31).

**Conclusion**: module-size limit for practical specialization is ~21k instructions;
TPC-H exceeds this threshold by a factor of approximately **12×** (255,342 / 21,000 ≈ 12.2).
Documented as RQ6 failure case.

## Root Cause

The sqlite3VdbeExec module is 12× larger than the typical UC MEDIUM module because
sqlite3VdbeExec is a monolithic interpreter loop for SQLite's virtual machine.
It handles all SQLite opcodes in a single function body, leading to O(n_opcodes)
IR size. Runtime specialization is only practical for modules where JIT compilation
completes within a few hundred milliseconds — a threshold exceeded by this module.

## Practical Guidance

Specialization is recommended when:
1. Module instruction count ≤ ~21k (based on UC MEDIUM empirical data)
2. JIT overhead (< ~200ms) is amortizable over the expected number of calls

For TPC-H/sqlite3VdbeExec, options for future work include:
- Extracting a query-specific submodule before embedding IR (out of scope for this thesis)
- Using profile-guided dead-code elimination to reduce module size before embedding
