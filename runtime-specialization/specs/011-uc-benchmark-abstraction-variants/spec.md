# Feature Specification: Use-Case Benchmark Abstraction Variants

**Feature Branch**: `011-uc-benchmark-abstraction-variants`
**Created**: 2026-05-14
**Status**: Draft
**Input**: Expand use-case benchmarks with abstraction-level variants (low-level / tradeoff / abstract) and multiple benchmark variants per use-case.

## Overview

The existing use-case benchmarks (UC1–UC14) are all written at a low abstraction level — raw loops, byte-level memory access, function pointers. This feature adds two additional abstraction tiers to each use-case and broadens each use-case to at least three benchmark variants. The goal is to measure whether real-world, idiomatic C++ code (using standard abstractions like `std::function`, virtual dispatch, iterators, and policy classes) benefits *more* from runtime specialization than hand-written low-level code, and to quantify that difference across the abstraction spectrum.

Each abstraction level represents a coding style commonly found in production C++ codebases:

- **Low-level**: Hand-optimized, minimal abstraction. Represents performance-critical inner loops. This is where the current benchmarks live.
- **Tradeoff**: Uses standard C++ idioms (templates, `std::function`, simple class hierarchies) that balance maintainability with performance. Represents typical library or engine code.
- **Abstract**: Maximally extensible, object-oriented design with virtual dispatch, policy injection, and runtime configurability. Represents framework or plugin-based code where maintainability is paramount.

---

## Clarifications

### Session 2026-05-14

- Q: UC7 variant 3 (`multi_pattern_match`) currently short-circuits by returning after the first pattern match. What should the non-short-circuiting replacement behavior be? → A: Count all matches (scan entire buffer, return total count of all pattern occurrences).

---

## User Scenarios & Testing *(mandatory)*

### User Story 1 — Researcher measures specialization gain across abstraction levels (Priority: P1)

A researcher running the benchmark suite wants to compare JIT specialization speedup at all three abstraction levels for each use-case variant. They run a single benchmark binary, filter by use-case, and record per-abstraction-level speedup ratios.

**Why this priority**: This is the central research question. Without all three abstraction levels, the core hypothesis ("does higher abstraction benefit more from specialization?") cannot be tested.

**Independent Test**: Run `--benchmark_filter=".*uc1_sql.*"` and observe results for low/tradeoff/abstract tiers for each variant.

**Acceptance Scenarios**:

1. **Given** the benchmark binary is built, **When** filtered to a single use-case, **Then** output includes rows for all three abstraction levels and all variants for that use-case.
2. **Given** a specialization benchmark runs, **When** the abstract-tier kernel is used, **Then** the JIT speedup ratio equals or exceeds the low-level tier speedup for the same variant.

---

### User Story 2 — Researcher compares specialization benefit across use-case variants (Priority: P2)

A researcher wants to understand which *aspect* of a use-case benefits most from specialization (e.g., for SQL: is it the threshold comparison, the memory layout, or the column count that drives speedup?).

**Why this priority**: Multiple variants per use-case expose whether specialization gain is specific to one parameter or generalizes across the use-case.

**Independent Test**: Filter by `uc1_sql` and compare speedup ratios across `count_matching_rows`, `multi_predicate`, and `column_scan` variants.

**Acceptance Scenarios**:

1. **Given** UC1 SQL benchmarks run, **When** results are collected for all 3 variants, **Then** each variant produces a distinct speedup ratio, showing parameter-sensitivity.
2. **Given** any use-case runs, **When** the JIT analysis tier is recorded, **Then** each variant has a different function/instruction count after pruning.

---

### User Story 3 — Benchmark results are importable into the existing DuckDB reporting pipeline (Priority: P3)

All new benchmarks follow the existing naming convention so that `record_benchmark.py` imports them without schema changes.

**Why this priority**: The reporting infrastructure already handles `g:`, `n:`, `s:`, `t:` tags. New benchmarks must not require pipeline changes.

**Independent Test**: Run `record_benchmark.py` on the output of the new benchmarks and verify all rows appear in the `benchmarks` table.

**Acceptance Scenarios**:

1. **Given** a new benchmark runs, **When** its JSON output is ingested by `record_benchmark.py`, **Then** all rows are inserted without error.
2. **Given** the DuckDB schema, **When** querying by `group_name LIKE 'uc%'`, **Then** all new benchmark variants appear.

---

### Edge Cases

- Abstract-tier kernels with virtual dispatch must not be inlined by the compiler at compile time (use `__attribute__((noinline))` on virtual call sites if needed).
- Tradeoff-tier lambdas that capture `std::function` must still be specializable — verify the lambda factory pattern works.
- Benchmarks with the same data shapes across abstraction levels must produce identical numerical results to validate correctness.
- **No benchmark kernel may short-circuit** (terminate before processing its entire input). All kernels must process their full input dataset and return an aggregate result (count, sum, sorted array, etc.). In particular, `multi_pattern_match` must scan the entire buffer and count all matches — it must not return after the first match.

---

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: Each existing use-case (UC1, UC2, UC7, UC8, UC12, UC14) MUST have at least 3 named variants covering distinct aspects of the use-case.
- **FR-002**: Each variant MUST be implemented at all three abstraction levels: low-level, tradeoff, and abstract.
- **FR-003**: Each abstraction level MUST be in its own translation unit (or clearly separated header) so the JIT module blob is level-specific.
- **FR-004**: Benchmark names MUST follow the existing `BM_g:<group>;n:<name>;s:<size>;t:<type>;...` convention with a new `a:<level>` tag for abstraction level (low/tradeoff/abstract).
- **FR-005**: Each variant MUST produce a validation function confirming numerical equivalence across abstraction levels.
- **FR-006**: All new kernels MUST be free of static globals (following the non-static rule established in spec 008).
- **FR-007**: The spec MUST include a documented rationale for each use-case variant and each abstraction-level implementation choice.
- **FR-008**: Abstract-tier implementations MUST use runtime polymorphism (virtual dispatch or `std::function`) to represent real-world extensibility patterns.
- **FR-009**: Tradeoff-tier implementations MUST use compile-time polymorphism (templates, policy classes, or `std::function` with known callables at specialization time).
- **FR-010**: The existing low-level benchmarks MUST be re-tagged (or aliased) with `a:low` so comparison queries work uniformly.
- **FR-011**: Every kernel MUST process its entire input without early exit. No kernel may return a result after the first matching element; all kernels MUST compute an aggregate over the full dataset (count, sum, sorted array, convolved image, etc.).

### Key Entities

- **Variant**: A named benchmark scenario within a use-case, covering a distinct functional aspect (e.g., `count_matching_rows`, `multi_predicate`, `column_scan` for UC1).
- **Abstraction Level**: One of {low, tradeoff, abstract}. Determines the C++ coding style used to implement the variant kernel.
- **Kernel TU**: A separate `.cpp` file containing the kernel function and its lambda factory. One TU per (variant × abstraction-level) pair, or combined per use-case with level-specific headers.
- **Benchmark Group**: The `g:` tag value used in benchmark names (e.g., `uc1_sql`).

---

## Use-Cases

### UC1: SQL Predicate Filter

Represents database query execution engines that evaluate row predicates over columnar or row-store data. This pattern appears in every SQL engine, stream processor, and OLAP system. The specialization opportunity is that the predicate (threshold, column offsets, row stride) is fixed per query but varies across queries.

---

#### Variant 1: `count_matching_rows` (single numeric predicate)

Scan a row store and count rows where a single numeric column exceeds a threshold. This is the simplest aggregation predicate — a hot path in every OLAP query.

##### Low-level
Raw byte loop with `__builtin_memcpy` to extract a `double` from a fixed byte offset in a flat row buffer. `row_stride`, `col_offset`, and `threshold` are specialization constants. Represents what a query engine's JIT-compiled inner loop looks like. Already implemented as the current UC1 benchmark.

##### Tradeoff
A templated `RowScanner<ColOffset, RowStride>` class with a `scan(const uint8_t* rows, int64_t n, double threshold)` method. The column offset and stride are template parameters (compile-time constants at specialization time), but the threshold is a runtime argument that becomes a JIT constant. Represents a type-safe, reusable scanner utility that a library would expose.

##### Abstract
A `Predicate` base class with a `virtual bool test(const uint8_t* row) const = 0` interface. A `ThresholdPredicate` subclass captures `col_offset`, `row_stride`, and `threshold` as constructor arguments. A `scan(const uint8_t* rows, int64_t n, const Predicate& pred)` free function iterates and delegates. Represents a plugin-based query predicate system where predicates are registered at runtime.

---

#### Variant 2: `multi_predicate` (AND of two predicates)

Evaluate two threshold predicates on different columns simultaneously and count rows satisfying both. Real SQL WHERE clauses almost always combine multiple conditions; this variant measures specialization benefit when the optimizer must constant-fold two independent predicates.

##### Low-level
Loop extracting two `double` fields via `__builtin_memcpy` with distinct `col_offset_a` / `col_offset_b`, comparing against `threshold_a` / `threshold_b`. All four constants are specialization inputs.

##### Tradeoff
A `BinaryPredicate<PredA, PredB>` policy class template that composes two column predicates via `&&`. Instantiated with `ColumnPredicate<ColOffsetA>` and `ColumnPredicate<ColOffsetB>` — template parameters carrying constant offsets. The compound predicate is passed to a generic `scan()` template. Demonstrates policy-based composition, common in expression template libraries.

##### Abstract
Two `ThresholdPredicate` objects composed by an `AndPredicate` combinator (also a `Predicate` subclass) that holds two `std::unique_ptr<Predicate>` children. The scan loop calls `and_pred.test(row)` which delegates to both children. Represents a query plan tree where predicate nodes are composed at runtime.

---

#### Variant 3: `column_scan` (projection + count)

Scan and collect (not just count) matching row indices into an output buffer. This exercises write-path specialization, where the output stride and index type are constants. Appears in database index scans and columnar projection operators.

##### Low-level
Loop that writes matching row indices as `int32_t` to a pre-allocated output array. `row_stride`, `col_offset`, `threshold`, and the output element size are specialization constants.

##### Tradeoff
A `ProjectingScanner` class template parameterized on `RowStride` and `ColOffset`. The `scan_to(const uint8_t* rows, int64_t n, double threshold, int32_t* out) -> int64_t` method returns the count of matches written. Ownership and buffer management handled by the caller.

##### Abstract
A `ResultCollector` interface with `virtual void emit(int64_t row_idx) = 0`. A `BufferCollector` implementation writes to a `std::vector<int64_t>`. The scan function accepts a `ResultCollector&` and calls `emit()` for each match. Models the observer pattern used in streaming query result delivery.

---

### UC2: 2D Convolution (Image Processing)

Represents image processing pipelines, CNNs, and signal processing frameworks. The convolution kernel (filter coefficients and size) is fixed per operation but changes between operations. Runtime specialization can fold the coefficients into the generated code, enabling strength reduction and loop unrolling.

---

#### Variant 1: `separable_gaussian` (fixed 5-tap kernel, both axes)

Apply a separable 2D Gaussian filter. Already implemented as the current UC2 benchmark. Both horizontal and vertical passes use the same fixed coefficients.

##### Low-level
Two nested loops — horizontal then vertical — with explicit bounds clamping via `std::max/std::min`. Kernel coefficients and `ksize` are specialization constants. Already implemented.

##### Tradeoff
A `SeparableFilter<KSize>` class template where `KSize` is a compile-time constant. The `apply(const float* src, float* dst, int width, int height, const float* coeffs)` method performs both passes. The compiler can unroll the inner `ksize` loop when `KSize` is a template parameter.

##### Abstract
A `FilterKernel` base class with `virtual float weight(int offset) const = 0` and `virtual int radius() const = 0`. A `GaussianKernel` subclass stores coefficients in a `std::vector<float>`. A `Convolver` class holds a `std::shared_ptr<FilterKernel>` and exposes `apply(const float* src, float* dst, int width, int height)`. Models extensible image processing pipelines where filter types are pluggable.

---

#### Variant 2: `box_filter` (uniform average, variable radius)

Apply a box (uniform average) filter with a runtime-configurable radius. Box filters are used in fast blur, integral images, and downsampling pipelines. The radius determines loop bounds — a prime specialization target for loop trip-count optimization.

##### Low-level
Single-pass 2D box filter with a nested loop bounded by `2*radius+1`. `radius`, `width`, and `height` are specialization constants. Output is the average of the `(2r+1)²` neighborhood.

##### Tradeoff
A `BoxFilter` class with a `radius` data member set at construction. The `apply(const float* src, float* dst, int width, int height)` method performs the average. When specialized, `radius` folds into constant loop bounds.

##### Abstract
A `SpatialFilter` interface with `virtual void apply(const float* src, float* dst, int width, int height) = 0`. A `BoxFilter` implementation inherits it, with `radius` injected via constructor. A `FilterPipeline` holds a `std::vector<std::unique_ptr<SpatialFilter>>` and applies them in sequence. Models OpenCV-style filter pipelines.

---

#### Variant 3: `edge_detection` (Sobel gradient, fixed 3×3 kernel)

Compute per-pixel gradient magnitude using the Sobel operator (3×3 horizontal and vertical kernels). Sobel is the canonical image processing kernel beyond Gaussian — it has fixed non-separable coefficients and combines two passes. Specializing both kernels and the image dimensions eliminates all parameter-dependent branches.

##### Low-level
Two separate 3×3 convolution passes (Gx, Gy) with hardcoded coefficient arrays. `width` and `height` are specialization constants. Gradient magnitude computed as `sqrt(Gx² + Gy²)`.

##### Tradeoff
A `SobelFilter` struct with static constexpr coefficient arrays for Gx and Gy. A `detect_edges(const float* src, float* dst, int width, int height)` free function instantiated as a template on width/height or inlined after specialization.

##### Abstract
A `GradientDetector` interface with `virtual void detect(const float* src, float* dst, int width, int height) = 0`. A `SobelDetector` implements it with the fixed Sobel coefficients. Client code holds a `GradientDetector*` and calls `detect()` — the virtual call is devirtualized by the JIT specializer when the concrete type is known at specialization time.

---

### UC7: DFA Regex (Pattern Matching)

Represents pattern matching in lexers, network packet classifiers, and log parsers. The DFA state table is fixed per compiled pattern but differs between patterns. Specializing the table dimensions and transitions enables constant folding of the state machine.

---

#### Variant 1: `email_match` (email address validation DFA)

Match strings against a DFA encoding the email address pattern. Already implemented as UC7. The 10-state, 128-character DFA table is the specialization constant.

##### Low-level
Single loop: `state = dfa_table[state * n_chars + (unsigned char)c]`. `n_states`, `n_chars`, `start_state`, `accept_state`, and the table pointer are all specialization constants. Already implemented.

##### Tradeoff
A `DFAMatcher<NStates, NChars>` class template with a `match(const char* s, int64_t len) const` method. The `NStates` and `NChars` template parameters allow the compiler to optimize the table indexing arithmetic. The table is passed as a constructor argument.

##### Abstract
A `Matcher` interface with `virtual bool match(const char* s, int64_t len) const = 0`. A `DFAMatcher` implementation holds the table in a `std::vector<int>` and stores `n_states`, `n_chars`, `start_state`, `accept_state` as fields. A `MatcherRegistry` holds named `std::unique_ptr<Matcher>` instances by pattern name. Models a regex engine with pluggable backends.

---

#### Variant 2: `url_match` (URL scheme + host validation DFA)

Match strings against a DFA encoding a URL pattern (scheme `://` host). URL validation is a ubiquitous use-case in web frameworks, proxies, and sanitizers. The DFA has more states than email (richer grammar) but the same specialization structure.

##### Low-level
Same loop structure as email_match, but with a different DFA table (more states for scheme, colon-slash-slash, host segments). Specialization constants include the larger table.

##### Tradeoff
Reuses `DFAMatcher<NStates, NChars>` template. The URL DFA table is built by a `URLDFABuilder` struct at initialization, analogous to `DFABuilder` for email.

##### Abstract
Reuses the `Matcher` / `DFAMatcher` class hierarchy. A `URLMatcher` factory function constructs the DFA and returns a `std::unique_ptr<Matcher>`. Demonstrates that the abstract layer adds no new code — only configuration changes.

---

#### Variant 3: `multi_pattern_match` (scan entire buffer for N patterns, return total match count)

Scan a byte buffer for any of N fixed string patterns using a combined DFA (Aho-Corasick-style multi-pattern automaton). This represents network intrusion detection, log parsing, and content filtering — all of which compile a fixed set of patterns into one automaton and then stream data through it. The kernel processes the **entire buffer without early exit** and returns the total count of all pattern occurrences found.

##### Low-level
Single loop over all input bytes; state = table lookup. The combined table encodes all N patterns. `n_states`, `n_chars`, and all accept states (passed as an array) are specialization constants. On each accept-state hit, increments a counter and continues scanning (no early exit). Returns the total match count.

##### Tradeoff
A `MultiPatternMatcher<NStates, NChars, NPatterns>` template. Accept-state checking is a compile-time-sized array lookup. Specialization folds `NPatterns` and enables branch prediction. Returns total match count after processing the full buffer.

##### Abstract
A `PatternSet` interface with `virtual int64_t match_all_count(const char* buf, int64_t len) const = 0`. A `DFAPatternSet` implementation uses the combined table and scans the full buffer. A `StreamScanner` holds a `PatternSet*` and exposes a `scan_all(const char* buf, int64_t len) -> int64_t` method returning the total match count.

---

### UC8: Incremental View Materialization (IVM)

Represents incremental aggregation in stream processors (Flink, Kafka Streams), CDC pipelines, and OLAP materialized view maintenance. Each incoming row is a delta that must be folded into pre-aggregated buckets. The row layout and bucket count are fixed per view but vary across views.

---

#### Variant 1: `apply_row_delta` (single-column sum per bucket)

Apply one row delta into sum buckets. Already implemented as UC8. `n_buckets`, `group_col_offset`, `value_col_offset`, `row_stride` are specialization constants.

##### Low-level
Extract group key and value from raw bytes via `__builtin_memcpy`, compute bucket index, accumulate. Already implemented.

##### Tradeoff
An `IVMUpdater<NBuckets, GroupOffset, ValueOffset, RowStride>` class template. The `update(const uint8_t* row, double* buckets)` method performs the extraction and accumulation. Template parameters fold into compile-time constants.

##### Abstract
An `Aggregator` interface with `virtual void apply(const uint8_t* row, double* state) = 0`. A `SumAggregator` implementation holds the offsets and bucket count as constructor arguments. A `ViewMaintainer` holds an `Aggregator*` and calls `apply()` per row. Models a streaming aggregation operator in a query execution engine.

---

#### Variant 2: `multi_agg_delta` (sum + count per bucket)

Apply one row delta into both sum and count buckets simultaneously. Count-distinct and average queries require both; this variant measures specialization benefit when two accumulation targets must be updated per row.

##### Low-level
Extract group key and value; update both `sum_buckets[bucket]` and `count_buckets[bucket]`. Six specialization constants: `n_buckets`, `group_col_offset`, `value_col_offset`, `row_stride`, plus pointers to both arrays.

##### Tradeoff
An `IVMDualUpdater` struct with `sum_buckets` and `count_buckets` as member pointers. The `update(const uint8_t* row)` method updates both. Constructor takes all constant parameters.

##### Abstract
A `MultiAggregator` that holds a `std::vector<std::unique_ptr<Aggregator>>` and calls `apply()` on each for every row. A `SumAggregator` and `CountAggregator` are registered at view-creation time. Models composable aggregation operators in a streaming framework.

---

#### Variant 3: `batch_delta` (apply N rows from a buffer)

Apply an entire batch of rows in one call rather than one row at a time. Batch-oriented deltas are the norm in CDC pipelines (Debezium, Maxwell) where a transaction produces many rows. The batch size is a specialization constant enabling loop-bound optimization.

##### Low-level
Outer loop over `n_rows`, inner extraction and accumulation. `n_rows`, `row_stride`, `group_col_offset`, `value_col_offset`, and `n_buckets` are all specialization constants.

##### Tradeoff
A `BatchIVMUpdater<BatchSize, NBuckets, GroupOffset, ValueOffset, RowStride>` template. `BatchSize` enables the compiler to unroll or vectorize the outer loop.

##### Abstract
A `BatchProcessor` interface with `virtual void process_batch(const uint8_t* rows, int64_t n_rows, double* buckets) = 0`. A `SumBatchProcessor` implements it with configurable constants. A `PipelineStage` wraps a `BatchProcessor*` and calls it from a processing loop.

---

### UC12: Group-By Aggregation

Represents bulk grouped aggregation in OLAP query engines (columnar sum/average per group), pandas-style groupby operations, and Spark shuffle reduce. The row layout and number of output groups are fixed per query.

---

#### Variant 1: `grouped_sum` (sum of one column per group key)

Compute grouped sum over a row batch. Already implemented as UC12. `row_stride`, `key_offset`, `value_offset`, `n_buckets` are specialization constants.

##### Low-level
Row loop with per-row extraction and bucket accumulation. Already implemented.

##### Tradeoff
A `GroupedAggregator<RowStride, KeyOffset, ValueOffset>` class template. The `aggregate(const uint8_t* rows, int64_t n_rows, double* out, int n_buckets)` method performs the grouped sum. Template parameters fold layout constants.

##### Abstract
An `AggregationOperator` base class with `virtual void aggregate(const uint8_t* rows, int64_t n_rows, double* out_buckets, int n_buckets) = 0`. A `SumOperator` subclass holds the offsets as fields. A `QueryPlan` holds a `std::unique_ptr<AggregationOperator>` and drives execution. Models a vectorized query execution engine operator.

---

#### Variant 2: `grouped_count` (count rows per group key)

Count rows per group without reading a value column. This is the `COUNT(*)` GROUP BY pattern — the most common grouped query, with a slightly different specialization profile (no value extraction, only group key).

##### Low-level
Loop extracts `int32_t` group key via `__builtin_memcpy`, increments `count_buckets[bucket]`. Specialization constants: `row_stride`, `key_offset`, `n_buckets`.

##### Tradeoff
A `GroupCounter<RowStride, KeyOffset>` class template with `count(const uint8_t* rows, int64_t n_rows, int64_t* out, int n_buckets)`.

##### Abstract
Reuses `AggregationOperator`. A `CountOperator` subclass stores only `key_offset` and `row_stride`. Demonstrates that the abstract layer handles both `sum` and `count` variants with zero structural change.

---

#### Variant 3: `grouped_minmax` (min and max per group)

Compute both minimum and maximum of a value column per group in a single pass. Two-accumulator groupby is common in time-series queries (open/close prices, sensor ranges). Specialization can eliminate the double extraction by folding offsets.

##### Low-level
Row loop extracts value; updates `min_buckets[bucket]` and `max_buckets[bucket]`. Specialization constants: `row_stride`, `key_offset`, `value_offset`, `n_buckets`.

##### Tradeoff
A `MinMaxAggregator<RowStride, KeyOffset, ValueOffset>` template with dual output arrays.

##### Abstract
An `AggregationOperator` subclass `MinMaxOperator` that writes to two output buffers. The virtual interface gains a `virtual void aggregate_dual(const uint8_t* rows, int64_t n_rows, double* out_min, double* out_max, int n_buckets) = 0` extension point.

---

### UC14: Generic Sort

Represents generic sorting in standard library implementations, database ORDER BY operators, and in-memory index construction. The comparator and element size are fixed per sort operation but vary between operations. Specializing the comparator enables function-pointer inlining and element-size loop optimization.

---

#### Variant 1: `generic_sort` (int64 ascending, function pointer comparator)

Sort an array of `int64_t` using a function pointer comparator with a generic byte-copying swap. Already implemented as UC14. `comparator` and `element_size` are specialization constants.

##### Low-level
Iterative median-of-three quicksort with explicit stack and byte-level swap via `memcpy`. Already implemented.

##### Tradeoff
A `GenericSorter<ElementSize>` class template. The `sort(void* data, int64_t n, int (*cmp)(const void*, const void*))` method uses the compile-time `ElementSize` to inline `memcpy` sizes. The comparator is still passed as a function pointer but gets inlined by specialization.

##### Abstract
A `Comparator` interface with `virtual int compare(const void* a, const void* b) const = 0`. An `Int64AscComparator` implements it. A `Sorter` class holds a `Comparator*` and exposes `sort(void* data, int64_t n, int element_size)`. Models the `java.util.Comparator` pattern, common in generic algorithm libraries.

---

#### Variant 2: `struct_sort` (sort structs by one field, runtime field offset)

Sort an array of fixed-size structs by a single `double` field at a runtime-known byte offset. Represents ORDER BY on a specific column in a row store — the field offset and struct size are fixed per query.

##### Low-level
Quicksort with a custom comparator that reads `double` via `__builtin_memcpy` at `field_offset`. `element_size` and `field_offset` are specialization constants inlined into the comparator.

##### Tradeoff
A `FieldComparator<FieldOffset, FieldType>` template. The `compare(const void* a, const void* b)` method extracts the field at `FieldOffset` and compares. Passed to a generic sort as a static function.

##### Abstract
A `FieldExtractor` interface with `virtual double extract(const void* record) const = 0`. A `ByteOffsetExtractor` implements it with `field_offset` as a constructor argument. A `FieldComparatorAdapter` wraps a `FieldExtractor*` and implements the `Comparator` interface. Demonstrates the adapter pattern used in ORM-style data access layers.

---

#### Variant 3: `multi_key_sort` (sort by primary + secondary field)

Sort by two fields lexicographically (primary ascending, secondary descending). Multi-key ORDER BY is ubiquitous in database queries. Both field offsets and types are specialization constants; the sort direction is also a constant.

##### Low-level
Comparator function reads two fields and performs a two-level comparison. Four specialization constants: `key1_offset`, `key2_offset`, `element_size`, plus direction flags.

##### Tradeoff
A `MultiKeyComparator<Key1Offset, Key2Offset, Key1Asc, Key2Asc>` template with a static `compare()` method.

##### Abstract
A `CompositeComparator` that holds a `std::vector<std::pair<std::unique_ptr<FieldExtractor>, bool>>` (extractor + ascending flag). The `compare(const void* a, const void* b)` method iterates extractors in order. Models a query planner's ORDER BY node with arbitrary sort keys.

---

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: All 6 use-cases have at least 3 variants, each with 3 abstraction levels = at least 54 benchmark scenarios (excluding size variants).
- **SC-002**: For at least 4 of 6 use-cases, the abstract-tier variant shows a higher specialization speedup ratio than the low-level variant for the same benchmark scenario.
- **SC-003**: All new benchmarks are importable by `record_benchmark.py` without schema modifications.
- **SC-004**: All validation functions pass (numerical equivalence across abstraction levels verified).
- **SC-005**: JIT compile time does not regress for low-level tiers compared to the pre-existing benchmarks (within 10% tolerance).
- **SC-006**: The spec documentation section (this file) contains a rationale for every use-case variant and every abstraction-level implementation choice.

---

## Assumptions

- Each (variant × abstraction-level) pair is implemented in its own kernel `.cpp` file or a clearly separated section so the JIT blob is specific to that level.
- Template-based tradeoff-tier kernels are still specializable because the template is instantiated in the same TU as the lambda factory — the JIT sees a concrete function, not a template.
- Virtual dispatch in the abstract tier is not devirtualized at compile time (enforced via separate TU for the vtable) so the JIT specializer must perform devirtualization at runtime to show speedup.
- The existing `a:low` benchmarks reuse current UC kernel implementations with an added `a:low` tag; no functional changes to existing code.
- Data sizes and benchmark parameters match the existing conventions (SMALL/MEDIUM/LARGE/EXTRALARGE) for comparability.
- The `DBOperatorsBenchmark` (already abstract-tier style) serves as a reference implementation showing the expected pattern for abstract-tier kernels.
