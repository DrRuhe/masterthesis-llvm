# Details: SQLite Specialization Failure Analysis

## Background: The Dispatch-Loop Hypothesis

`sqlite3VdbeExec` is a ~5000-line function built around a dispatch loop:

```c
for (pc=p->pc; ...; pc++) {
  pOp = &aOp[pc];
  switch (pOp->opcode) {
    case OP_Noop: ...
    case OP_ResultRow: ...
    // ~160 cases
  }
}
```

The specialization hypothesis (stated in `tpch_bench.cpp`) is:
> Specializing `sqlite3VdbeExec` for a known `Vdbe*` makes the entire opcode
> dispatch switch constant-foldable: IPSCCP sees `p->aOp[pc].opcode` as known
> constants and eliminates the switch, inlining only relevant handlers.

For this to work, IPSCCP would need to know the value of `p->aOp[pc].opcode`
for each iteration. Since `p` is a constant pointer, `p->aOp` is a fixed
memory address (loadable via `!invariant.load`). But `pc` is a *dynamic*
loop variable — it changes every iteration. Therefore `p->aOp[pc].opcode`
requires a runtime load at a *variable* offset, which SCCP cannot resolve
to a constant. The switch cannot be folded without loop unrolling, which
the LLVM O3 backend does only for small trip counts.

## Candidate Root Causes

### RC-1: Dynamic `pc` prevents constant folding of dispatch switch
Even with a constant `p`, `pc` varies every iteration.
`p->aOp[pc].opcode` is a load at a variable GEP offset → not a constant.
SCCP cannot fold the switch. This is a fundamental limit, independent of
pipeline configuration.

### RC-2: `InvariantLoadToConstantPass` skipped for large modules
As of spec-005, this pass was guarded on `!IsLargeModule`.
SQLite is always classified as large (21 functions / 178 instructions after
GlobalDCE prune, but the threshold is 3 instructions).
If still guarded, the pass that converts host-memory-readable invariant loads
to literal IR constants never runs → no constant substitution in the IR.

### RC-3: Too few !invariant.load annotations on mutable Vdbe fields
`StaticMutabilityAnalysis` marks a load invariant only when its source pointer
is never written. Vdbe fields that are mutated during execution (e.g., `pc`,
`nChange`, `cacheCtr`) will NOT be marked invariant. If the fields relevant to
the dispatch switch (e.g., `aOp` pointer, individual `VdbeOp.opcode` fields)
are considered mutable due to conservative alias analysis, no annotations land.

### RC-4: Specialized and original code are structurally identical
The pipeline produces no meaningful IR transformation, and the specialized
function is identical (or near-identical) to the unspecialized one.
The small overhead comes from JIT compilation + indirect call overhead.

## Pipeline Stages of Interest

| Stage | File | Relevant for |
|-------|------|-------------|
| StaticMutabilityAnalysis | `StaticMutabilityAnalysis.cpp` | RC-2, RC-3 |
| JitSCCPSolver::visitLoadInst | `JitSCCPSolver.cpp` | RC-1, RC-3 |
| InvariantLoadToConstantPass | `InvariantLoadToConstant.cpp` | RC-2 |
| ConstantArgFunctionSpecializationPass | `ConstantArgFunctionSpecializationPass.cpp` | RC-3 |
| IsLargeModule guard | `JITPipelineIPSCCP.cpp` | RC-2 |

## Instrumentation Points Added

| File | What was added |
|------|---------------|
| `StaticMutabilityAnalysis.cpp` | Count annotated loads per function, emit `[CRS-STAT] StaticMutability:` |
| `JitIPSCCPPass.cpp` | Post-solve count of invariant loads and constant-lattice loads, emit `[CRS-STAT] SCCP:` |
| `InvariantLoadToConstant.cpp` | Count successful host-memory replacements, emit `[CRS-STAT] InvariantLoadToConst:` |
| `ConstantArgFunctionSpecializationPass.cpp` | Log `Groups.size()` per function (P1 pipeline only) |

## Collected Data

### Q1 (TPC-H Query 1) and simple_scan (identical results)

Debug build, `CRS_PASS_TRACE_DIR`+`CRS_CHROME_TRACE_DIR` set, `jit_analysis` phase:

```
[CRS-STAT] StaticMutability: fn=_ZN16RSSMemoryManager4StopERN9benchmark13MemoryManager6ResultE annotated_invariant_loads=1
[CRS-STAT] SCCP: module=RuntimeSpecializeableIR invariant_load_total=1 folded_to_constant=0
```

- **0 loads** annotated in any sqlite3 function
- **0 loads** folded to constants by IPSCCP
- `InvariantLoadToConstantPass` never ran (large-module guard active)
- `ConstantArgFunctionSpecializationPass` not in P2 pipeline

The 1 annotated load is in a benchmark utility function (`RSSMemoryManager::Stop`) that is
in the JIT module but not on the sqlite3VdbeExec call path.

## ASM Comparison Notes

Release build, Q1, `CRS_ASM_DUMP_DIR` enabled:

| Version | Lines | Content |
|---------|-------|---------|
| Original (`sqlite3VdbeExec` in ELF via `objdump`) | 9187 | Full dispatch loop, 160+ opcode cases |
| Specialized (JIT dump, `sqlite3VdbeExec__specialized.asm`) | 513 | 7 real instructions + zero-byte padding |

The 7 real instructions in the specialized function:
```asm
leaq    -7(%rip), %rax
movabsq $-8768, %rcx
addq    %rax, %rcx
movabsq $48, %rax
movabsq $95072674024776, %rdi   ; constant Vdbe* address
jmpq    *(%rcx,%rax)             ; tail-call to original function via GOT
```

This is a PLT-style thunk. The entire `sqlite3VdbeExec` body is unmodified.

## Pass-Trace Data

Release build, Q1, `CRS_PASS_TRACE_DIR`:

| Group   | Pass                                  | Fns       | Instrs     |
|---------|---------------------------------------|-----------|------------|
| prune   | GlobalDCEPass                         | 216 → 21  | 15215 → 178|
| initial | clangRuntimeSpecializer::JitIPSCCPPass| 21 → 21   | 178 → 178  |
| final   | (GlobalDCE, ReversePostOrderFuncAttrs)| 21 → 21   | 178 → 178  |
| final_o3| (full O3 pipeline)                    | 21 → 20   | 178 → ~180 |

`JitIPSCCPPass` produces zero instruction changes. O3 inlines one small helper (fn count
goes 21→20) but the instruction count barely changes. No `InvariantLoadToConstantPass`
entry appears in the "final" group — confirmed skipped due to IsLargeModule guard.

## Large-Module Guard Finding

In `JITPipelineIPSCCP.cpp` at line 149:
```cpp
// Gated on !LargeModule to match spec-005 safety rule.
if (!Args.IsLargeModule) {
    FPM.addPass(StaticMutabilityAnalysis::StaticMutabilityAnalysisPass());
    FPM.addPass(InvariantLoadToConstantPass());
    FPM.addPass(llvm::InstCombinePass());
    FPM.addPass(llvm::SimplifyCFGPass());
    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(FPM)));
}
```

`IsLargeModule` is true when `InstructionCountAfterPrune > LargeModuleInstrThreshold`.
For sqlite3, post-prune count is 178, the default threshold is 3 → always large → this
block is always skipped.

Note: The FIRST `StaticMutabilityAnalysis` run (Phase 2, "initial" group) IS unconditional
and runs correctly. It is only the SECOND run in Phase 3 ("final" group) that is skipped.
Since the first run annotates 0 loads anyway, the second skip has no practical effect.
