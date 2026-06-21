# Details 021: Specializeable Architectures Taxonomy

## Scope Decision

This section classifies architectural patterns by **technical support only**.
It is not a per-pattern cost/benefit analysis. The intended thesis move is:

1. Identify whether CRS supports specializing a given architectural pattern.
2. Use that classification to explain why some kernels do or do not show speedup.

This means:
- A pattern may count as "supported" even if a particular benchmark built from
  that pattern does not yield a strong speedup.
- Outlier kernels should only be assigned to "unsupported" categories when the
  inspected local evidence actually supports that claim.
- If an outlier simply lacks an optimizer-visible specialization opportunity,
  the later thesis draft must say so plainly instead of forcing it into an
  unsupported mechanism claim.

## Canonical-Section Decision

`Specializeable Architectures` should become the canonical thesis section for
what CRS supports and does not support. The current SQLite/TPC-H explanation in
`docs/thesis.typ` under `@rq-limits` is therefore not separate theory; it is
source material for this taxonomy.

Implication for the later drafting pass:
- Move or condense the SQLite/TPC-H reasoning into this section.
- Leave `@rq-limits` with only the pipeline-selection consequence plus a short
  cross-reference back to `@specializeable-architectures`.
- Avoid maintaining two separate explanations of the same unsupported pattern.

## Current Taxonomy Ledger

These are the current headings that survived initial evidence inspection. Each
entry has concrete local evidence and is safe to carry into a later drafting
pass.

| Heading | Status | Why it survives | Primary local evidence |
|---|---|---|---|
| `Flat Batch Kernels with Fixed Layout or Threshold Parameters` | supported | UC kernels repeatedly bake row stride, field offsets, bucket counts, thresholds, and element sizes into one hot loop body. | `specs/008-use-case-benchmarks/spec.md`, `benchmarks/use-cases/UC1SqlPredicate/UC1CountMatchingRowsLowKernels.cpp`, `UC1MultiPredicateLowKernels.cpp`, `UC1ColumnScanLowKernels.cpp` |
| `Immutable Lookup Tables or Coefficient Arrays` | supported | UC2 and UC7 capture a stable coefficient/table pointer and then run a large loop against that fixed structure. | `benchmarks/use-cases/UC2Convolution/UC2Kernels.h`, `benchmarks/use-cases/UC7DfaRegex/UC7EmailMatchTradeoffKernels.cpp`, `UC7MultiPatternMatchTradeoffKernels.cpp` |
| `Nested Function Calls` | supported | The runtime is explicitly built to clone one blob, build a wrapper, and inline helper chains inside that blob; pass traces show `ModuleInlinerPass` firing on benchmark kernels. | `specs/003-jit-specialization-core/spec.md`, `benchmarks/reports/260523-173549-analysis/BM_g_uc2_conv_n_edge_detection_a_tradeoff_s_MEDIUM_t_jit_analysis__6_iterations_1_manual_time/BM_g_uc2_conv_n_edge_detection_a_tradeoff_s_MEDIUM_t_jit_analysis__pass_trace.json`, `benchmarks/reports/260523-173549-analysis/BM_g_uc14_sort_n_generic_sort_a_tradeoff_s_MEDIUM_t_jit_analysis__8000000_iterations_1_manual_time/BM_g_uc14_sort_n_generic_sort_a_tradeoff_s_MEDIUM_t_jit_analysis__pass_trace.json` |
| `By-Value Captured Helper Objects / Policy Structs` | supported | Tradeoff-tier kernels reconstruct helper objects inside the specialized lambda so their fields become constants without relying on virtual dispatch. | `specs/011-uc-benchmark-abstraction-variants/plan.md`, `benchmarks/use-cases/UC1SqlPredicate/UC1MultiPredicateTradeoffKernels.cpp`, `benchmarks/use-cases/UC14Sort/UC14MultiKeySortTradeoffKernels.cpp` |
| `Function-Pointer Callbacks` | supported | The repository has explicit regression coverage for forwarded function pointers and a benchmark family where comparator callbacks are the specialization target. | `test/smoke/call-specialized-forwarded-funcptr.cpp`, `test/smoke/speconly-forwarded-funcptr.cpp`, `test/smoke/speconly-std-apply-funcptr.cpp`, `benchmarks/use-cases/UC14Sort/UC14GenericSortTradeoffKernels.cpp` |
| `Vtable Devirtualization` | supported | The runtime has a dedicated devirtualization pass, smoke tests assert vtable loads disappear, and the abstract benchmark tier is built around this mechanism. | `test/smoke/virtual-methods.cpp`, `specs/015-pipeline2-jit-ipsccp/spec.md`, `runtime/ClangRuntimeSpecializer/DevirtualizeConstantVtableCalls.cpp`, UC abstract kernels in `benchmarks/use-cases/UC1SqlPredicate/`, `benchmarks/use-cases/UC7DfaRegex/`, and `benchmarks/use-cases/UC8IVM/` |
| `Shared Mutable State Object Threaded Through the Call Graph` | unsupported | SQLite shows that once the execution context escapes broadly, the current analysis cannot expose stable field values for specialization. | `specs/019-sqlite-specialization-analysis/spec.md`, `specs/019-sqlite-specialization-analysis/details.md`, `specs/019-sqlite-specialization-analysis/report.md` |
| `Interpreter Dispatch Loop over a Dynamic Program Counter` | unsupported | SQLite also fails because the opcode stream is addressed via a changing `pc`, so the dispatch path cannot collapse to one fixed specialized trace. | `specs/019-sqlite-specialization-analysis/details.md`, `specs/019-sqlite-specialization-analysis/report.md`, `docs/thesis.typ` around `@rq-limits` |
| `Opaque External Callees / Cross-Blob Boundaries` | unsupported but secondary | Local design notes explicitly state that the current specializer only sees bodies in the cloned blob, so wrappers that cross blob boundaries cannot inline into the real work. | `specs/005-tpch-duckdb-benchmarks/research.md`, `specs/004-ir-dump-preprocessing/research.md` |

## Heading Consolidation Decisions

- Keep `Interpreter Dispatch Loop over a Dynamic Program Counter` separate from
  `Shared Mutable State Object...` because SQLite needs both reasons:
  one explains why loads are not exposed as constants, the other explains why a
  fixed context pointer still does not imply a fixed execution path.
- Keep `Opaque External Callees / Cross-Blob Boundaries` only as a secondary
  unsupported heading or thesis footnote.
  It has concrete local design evidence, but not as much thesis-weight as the
  SQLite entry.
- Fold `Escaping Pointer State Hidden Behind Conservative Mutability` into the
  `Shared Mutable State Object...` heading instead of keeping it separate.
  The evidence is real, but it is the mechanism inside the SQLite failure case,
  not a clearer reader-facing architecture category on its own.
- Fold possible headings such as `Abstract Result Collectors` and
  `Schema-Specialized Aggregation` into stronger parent entries.
  `column_scan`'s collector logic is better explained by
  `Vtable Devirtualization` plus `Flat Batch Kernels...`, while grouped
  aggregation kernels are another instance of fixed-layout batch loops rather
  than a separate architectural family.

## Evidence Sources Already Identified

### Supported-pattern evidence

- **Flat batch kernels with fixed layout/scalar constants**
  - `specs/008-use-case-benchmarks/spec.md`
    - fixed specialization constants for UC1, UC4, UC5, UC6
  - `docs/thesis.typ` lines `952-1070`
    - current evaluation text already describes these families in thesis prose
  - `benchmarks/use-cases/UC1SqlPredicate/UC1CountMatchingRowsLowKernels.cpp`
  - `benchmarks/use-cases/UC1SqlPredicate/UC1MultiPredicateLowKernels.cpp`
  - `benchmarks/use-cases/UC1SqlPredicate/UC1ColumnScanLowKernels.cpp`

- **Immutable lookup tables / coefficient arrays**
  - `specs/008-use-case-benchmarks/spec.md`
    - UC2 `kernel_coeffs`, UC3 DFA table, UC7/UC2 benchmark rationale
  - `benchmarks/use-cases/UC2Convolution/UC2Kernels.h`
  - `benchmarks/use-cases/UC7DfaRegex/UC7EmailMatchTradeoffKernels.cpp`
  - `benchmarks/use-cases/UC7DfaRegex/UC7MultiPatternMatchTradeoffKernels.cpp`

- **Nested/helper-call support**
  - `specs/003-jit-specialization-core/spec.md`
    - wrapper construction and `AlwaysInline` requirements
  - `specs/008-use-case-benchmarks/research.md`
    - rationale that constant-captured arguments plus same-module helper bodies
      enable inlining into the specialized wrapper
  - `benchmarks/reports/260523-173549-analysis/.../pass_trace.json`
    - concrete `ModuleInlinerPass` changes for kernels such as UC2
      `edge_detection` and UC14 `generic_sort`

- **Function-pointer support**
  - `specs/008-use-case-benchmarks/spec.md`
    - UC14 comparator specialization is a first-class benchmark requirement
  - `specs/008-use-case-benchmarks/research.md`
    - comparator captured as constant function pointer, then inlined
  - `specs/TODO.md` RQ1-002
    - forwarded funcptr fix and smoke-test list
  - `test/smoke/call-specialized-forwarded-funcptr.cpp`
  - `test/smoke/speconly-forwarded-funcptr.cpp`
  - `test/smoke/speconly-std-apply-funcptr.cpp`

- **Vtable devirtualization support**
  - `test/smoke/virtual-methods.cpp`
    - explicit `EXE-NOT: load ptr, ptr %vtable`
  - `specs/015-pipeline2-jit-ipsccp/spec.md`
    - vtable devirtualization as a named scenario and success criterion
  - `runtime/ClangRuntimeSpecializer/DevirtualizeConstantVtableCalls.cpp`
    - dedicated pass for constant-vtable calls
  - `benchmarks/use-cases/UC1SqlPredicate/UC1CountMatchingRowsAbstractKernels.cpp`
  - `benchmarks/use-cases/UC1SqlPredicate/UC1ColumnScanAbstractKernels.cpp`
  - `benchmarks/use-cases/UC1SqlPredicate/UC1MultiPredicateAbstractKernels.cpp`
  - `benchmarks/use-cases/UC7DfaRegex/*AbstractKernels.cpp`
  - `benchmarks/use-cases/UC8IVM/*AbstractKernels.cpp`

- **By-value captured helper objects / policy structs**
  - `specs/011-uc-benchmark-abstraction-variants/plan.md`
    - repeated guidance that concrete objects are captured by value so fields or
      vtable pointers become JIT constants
  - `benchmarks/use-cases/UC7DfaRegex/UC7EmailMatchTradeoffKernels.cpp`
  - `benchmarks/use-cases/UC14Sort/UC14GenericSortTradeoffKernels.cpp`
  - `benchmarks/use-cases/UC14Sort/UC14MultiKeySortTradeoffKernels.cpp`

### Unsupported-pattern evidence

- **Shared mutable state object / interpreter**
  - `specs/019-sqlite-specialization-analysis/spec.md`
  - `specs/019-sqlite-specialization-analysis/details.md`
  - `specs/019-sqlite-specialization-analysis/report.md`
  - `docs/thesis.typ` lines around `1560-1585`

- **Potential opaque-body / cross-blob limitation**
  - `specs/005-tpch-duckdb-benchmarks/research.md`
    - explicit note that the JIT cannot inline across blobs in the current
      single-blob specialization path
  - `specs/004-ir-dump-preprocessing/research.md`
    - only functions surviving in one preprocessed blob are visible for deeper
      JIT simplification

- **Outlier kernel context**
  - `docs/thesis.typ` lines around `962-964`, `1170-1173`, `1240-1248`
  - `specs/TODO.md` RQ1-004 and RQ6-001
  - `specs/008-use-case-benchmarks/spec.md` “Minimum Viable Kernel Duration”
  - `benchmarks/reports/260531-2115-Review/methodology.md`
  - `benchmarks/reports/260610-corpus-final/unroll_impact.txt`

## Initial Outlier-Kernel Classification Hypotheses

These are hypotheses to test during collection, not conclusions.

| Kernel | Initial hypothesis | Why this is not final yet |
|---|---|---|
| `count_matching_rows` | appears to instantiate the supported `Flat Batch Kernels with Fixed Layout or Threshold Parameters` pattern rather than an unsupported one | The inspected low-level kernel is a direct counted loop with captured `row_stride`, `col_offset`, and `threshold`; weak speedup alone does not prove unsupportedness |
| `multi_predicate` | appears to be another flat batch/layout-constant kernel, but with more specialized constants and a richer predicate condition | Need to inspect tradeoff/abstract variants before deciding whether it belongs under a helper-object or vtable heading instead |
| `column_scan` | low-level variant appears to be a flat batch/layout-constant kernel plus output-buffer writes; abstract variant may additionally exercise vtable-based collector patterns | One kernel family may touch different headings at different abstraction tiers, so the later thesis draft may need to classify the family by its dominant pattern and mention secondary ones |

## Decision Rules for the Later Draft

- Prefer headings that correspond to mechanisms the implementation or tests name
  directly, such as helper-call inlining, function-pointer specialization, and
  vtable devirtualization.
- Prefer supported headings that recur across multiple benchmark families over
  one-off micro-mechanisms, since this subsection is meant to be the canonical
  support reference.
- Prefer unsupported headings that already have a root-cause report, not just a
  weak performance observation.
- Do not claim an outlier kernel belongs to an unsupported architecture unless a
  repository artifact shows the missing mechanism directly.
- If a kernel’s result is better explained by "no optimizer-visible supported
  pattern present" than by "unsupported pattern present", write that instead.
- If a kernel clearly instantiates only simple supported patterns, the later
  draft should say that CRS supports the pattern technically, but that this
  alone does not guarantee a large simplification opportunity.
- If the SQLite unsupported story already needs two distinct reasons (shared
  mutable context and dynamic opcode dispatch), decide during collection whether
  separate headings improve clarity or just duplicate the same root cause.

## Expected Deliverable Shape

The later thesis draft should be able to derive a subsection structure like:

```text
=== Flat Batch Kernels with Fixed Layout or Threshold Parameters
=== Immutable Lookup Tables or Coefficient Arrays
=== Nested Function Calls
=== By-Value Captured Helper Objects
=== Function-Pointer Callbacks
=== Vtable Devirtualization
=== Unsupported: Shared Mutable State Objects
=== Unsupported: Interpreter Dispatch over Dynamic State
```

or, if evidence is weaker than expected:

```text
=== Supported Patterns
==== Flat Batch Kernels with Fixed Layout or Threshold Parameters
==== Immutable Lookup Tables or Coefficient Arrays
==== Nested Function Calls
==== By-Value Captured Helper Objects
==== Function-Pointer Callbacks
==== Vtable Devirtualization
=== Unsupported Patterns
==== Shared Mutable State Objects in Interpreter Loops
```

The second form is safer if some entries collapse during evidence collection.
