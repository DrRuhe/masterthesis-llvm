# Report: Why SQLite Specialization Produces No Speedup

**Date**: 2026-06-11  
**Spec**: 019-sqlite-specialization-analysis  
**Context**: Spec 018 showed that all TPC-H queries on SQLite (`sqlite3VdbeExec`) produce
no execution speedup from JIT specialization (unspecialized ≈ 600 ms, specialized ≈ 630 ms
for Q1, debug build). This report investigates why.

---

## Executive Summary

The JIT-specialized function for `sqlite3VdbeExec` is a **thin trampoline** that pre-loads
the constant `Vdbe*` pointer into the `%rdi` register and tail-jumps to the completely
unmodified original function. Zero IPSCCP transformations occur inside the function body.
The dispatch switch (`switch(pOp->opcode)`) is fully preserved with all 160+ cases.

The primary cause is that **`StaticMutabilityAnalysis` annotates zero loads in
`sqlite3VdbeExec` as `!invariant.load`**. Without annotations, the IPSCCP solver
never attempts to fold any loads. The secondary causes (large-module guard, dynamic `pc`)
would independently block useful specialization even if the annotations were present.

---

## Sub-Question 1: How many loads are identified as `!invariant.load`?

**Answer: Zero loads in any sqlite3 function.**

**Evidence** (from instrumented run with `CRS_LOG` instrumentation, debug build,
Q1 and `simple_scan` — identical results for both):

```
[CRS-STAT] StaticMutability: fn=_ZN16RSSMemoryManager4StopERN9...  annotated_invariant_loads=1
[CRS-STAT] SCCP: module=RuntimeSpecializeableIR  invariant_load_total=1  folded_to_constant=0
```

The JIT module processes 21 functions after GlobalDCE prune (down from 216 reachable
functions). Of the 21, only **one** load is annotated as `!invariant.load`, and it belongs
to a benchmark utility (`RSSMemoryManager::Stop`) that is not part of the sqlite3 dispatch
path. No load inside `sqlite3VdbeExec` or any of its 20 callees is annotated.

**Why zero annotations?** `StaticMutabilityAnalysis::runCaptureAnalysis()` tracks GEP
paths from pointer arguments. It marks a pointer as `Escaped = true` when:

1. It is passed to a callee without `doesNotCapture` attribute, **or**
2. It is the source of a GEP with a variable (non-constant) index.

`sqlite3VdbeExec` passes `p` (Vdbe\*) or GEPs of `p` to many callees that do not have
`nocapture`/`readonly` attributes. This marks `p`'s entry in the PointerMap as
`Escaped = true`. In Phase C of the analysis, the check:

```cpp
if (FieldEscaped) continue;
```

skips every load derived from an escaped ancestor. Since `p` itself is escaped, ALL loads
from Vdbe fields are skipped and receive no `!invariant.load` annotation.

Additionally, the central access pattern `p->aOp[pc].opcode` (where `pc` is a dynamic
loop variable) produces a variable-index GEP, which also triggers `Escaped = true` for
the GEP source.

---

## Sub-Question 2: How many constants can the JIT actually inline?

**Answer: Zero.**

**Evidence from SCCP instrumentation**:
```
invariant_load_total=1  folded_to_constant=0
```

There is 1 annotated load visible to the IPSCCP solver, but it is not folded (the pointer
is not a constant the solver can resolve). For the sqlite3 functions, the IPSCCP solver
has nothing to work with.

**Pass trace** (release build, Q1, P2 pipeline, `CRS_PASS_TRACE_DIR` output):

| Group   | Pass                                  | Fns before → after | Instrs before → after |
|---------|---------------------------------------|--------------------|-----------------------|
| prune   | GlobalDCEPass                         | 216 → 21           | 15215 → 178           |
| initial | clangRuntimeSpecializer::JitIPSCCPPass| 21 → 21            | **178 → 178**         |
| final   | GlobalDCEPass                         | 21 → 21            | 178 → 178             |
| final_o3| (entire O3 pipeline)                  | 21 → 20            | 178 → ~180            |

`JitIPSCCPPass` makes **zero instruction changes**. The `InvariantLoadToConstantPass` never
runs: it is gated on `!Args.IsLargeModule` in `JITPipelineIPSCCP.cpp` (line 149), and
sqlite3's post-prune module (178 instructions) exceeds the default 3-instruction threshold.
O3 makes minor changes (inlines one helper), but the function count goes down by 1, not
the 160-handler collapse the hypothesis predicted.

---

## Sub-Question 3: Is the specialized code structurally different from unspecialized?

**Answer: No. The specialized "function" is a 7-instruction trampoline.**

**Evidence from ASM comparison** (release build, Q1, `CRS_ASM_DUMP_DIR` output):

| Version            | Size          | Content                                |
|--------------------|---------------|----------------------------------------|
| Original (ELF)     | **9187 lines**| Full dispatch loop, all 160+ cases    |
| Specialized (JIT)  | **513 lines** | 7 meaningful instructions + padding   |

The specialized ASM (first 7 lines, stripped of padding):
```asm
<sqlite3VdbeExec>:
    leaq    -7(%rip), %rax
    movabsq $-8768, %rcx
    addq    %rax, %rcx
    movabsq $48, %rax
    movabsq $95072674024776, %rdi   ; ← hardcoded Vdbe* address
    jmpq    *(%rcx,%rax)             ; ← indirect jump to original sqlite3VdbeExec
```

This is a **PLT-style thunk**: pre-load the constant `Vdbe*` into `%rdi` (first argument
register on System V AMD64 ABI), then tail-call the original unmodified `sqlite3VdbeExec`
via an indirect jump through the GOT. The dispatch switch, loop structure, and all 160+
opcode handlers are entirely unchanged in the body that actually executes.

The 513-line size is the JIT-compiled wrapper plus x86 alignment padding (the `addb`
instructions are the encoding of zero bytes). The remaining ~506 lines are padding noise.

---

## Root Cause Conclusion

The four candidate root causes, evaluated:

### RC-1: Dynamic `pc` prevents constant folding of dispatch switch
**Confirmed as a structural barrier.**  
Even if `p` were a known constant and `!invariant.load` annotations were present,
`p->aOp[pc].opcode` requires a load at a *variable* GEP offset (`pc` changes every
iteration). IPSCCP cannot fold a switch whose discriminant comes from a variable-index
array access. This would require loop unrolling at the level of the bytecode program
itself — LLVM does not do this for 100+ iteration programs.

### RC-2: `InvariantLoadToConstantPass` skipped for large modules
**Confirmed as a secondary contributing factor.**  
The pass that performs direct host-memory reads to convert `!invariant.load` loads to
literal IR constants never executes for sqlite3 (178 instructions >> 3-instruction
threshold at `JITPipelineIPSCCP.cpp:149`). However, since there are zero annotated
loads anyway (RC-3), this guard has no additional effect in practice.

### RC-3: Zero `!invariant.load` annotations due to conservative escape analysis
**Confirmed as the primary immediate cause.**  
`StaticMutabilityAnalysis` marks the `Vdbe*` argument as escaped because
`sqlite3VdbeExec` passes it to callees lacking `nocapture`/`readonly` attributes.
This causes all loads from Vdbe fields to be skipped. With zero annotations, the
IPSCCP solver has no constant load to fold, and the entire specialization pipeline
is a no-op (confirmed: `JitIPSCCPPass` changes 0 instructions).

### RC-4: Specialized and original code are structurally identical
**Confirmed as the observed consequence.**  
The "specialized" function is a 7-instruction trampoline calling the unmodified original.
The dispatch switch is completely intact. Execution time of the specialized function
includes the trampoline overhead + the full original function cost, explaining the slight
regression (630 ms vs 600 ms).

---

## Thesis Implications

For the thesis evaluation of specialization limits (RQ6):

- The sqlite3/TPC-H case is a **genuine failure case** where the specialization
  mechanism is fundamentally inapplicable, not merely inefficient.
- The root cause is not module size per se (the module is feasible after GlobalDCE prune),
  but the **semantic mismatch** between the specialization hypothesis and reality:
  the hypothesis assumed that a constant `Vdbe*` would make `aOp[pc].opcode` constant,
  which is impossible because `pc` is a dynamic variable.
- Even if all escape-analysis limitations were fixed (e.g., by manually annotating
  `sqlite3VdbeExec` callees with `nocapture`), constant folding of the dispatch switch
  would still require knowing `pc` at JIT compile time — which depends on the bytecode
  execution sequence, not just the address of the `Vdbe` struct.
- This is a fundamental difference from the successful use cases (e.g., DuckDB, corpus UCs)
  where the specialized argument directly controls a static branch or constant-foldable
  expression without an additional dynamic index (`pc`) intervening.

---

## Data Files

- Pass trace (release, Q1): `benchmarks/reports/260611-1528-sqlite-analysis/pass_trace/`
- Specialized ASM (Q1): `benchmarks/reports/260611-1528-sqlite-analysis/asm/sqlite3VdbeExec__specialized.asm`
- Original ASM (Q1): `benchmarks/reports/260611-1528-sqlite-analysis/asm/sqlite3VdbeExec__original.asm`
- Instrumentation logs: captured in `stderr.txt` in the same report dir; debug-build
  stderr from `./TPCHBenchmark` execution with CRS_PASS_TRACE_DIR and CRS_CHROME_TRACE_DIR set.
