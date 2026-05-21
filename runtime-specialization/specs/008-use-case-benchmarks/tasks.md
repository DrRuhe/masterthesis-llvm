# Tasks: Use-Case Benchmark Suite

**Input**: Design documents from `specs/008-use-case-benchmarks/`
**Plan**: `specs/008-use-case-benchmarks/plan.md`
**Spec**: `specs/008-use-case-benchmarks/spec.md`

## Execution Model

**All tasks are designed to run in subagents.** Each task description is self-contained:
it names every file to create or edit, the exact API to use, the build command to run,
and the acceptance criteria to verify. After passing all quality gates the subagent MUST
commit with `git commit`.

**Build root**: `llvm/llvm/build/release` for timing; `llvm/llvm/build/debug` for
equivalence/sanitizer checks.

**Core API**: Use `specializeLambda<R>(lambda)` (spec 010 — no `funcName` string, no
`__asm__` attribute). See `specs/008-use-case-benchmarks/plan.md` "Factory Function API"
section for the canonical code pattern.

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Create the directory skeleton and wire the new benchmark group into the
existing CMake build so that Phase 3 tasks can proceed in parallel.

- [x] T001 Create `benchmarks/use-cases/` directory skeleton, parent CMakeLists.txt
  (with six `add_subdirectory` calls), and wire `add_subdirectory(use-cases)` into
  `benchmarks/CMakeLists.txt`

**Checkpoint**: `benchmarks/use-cases/CMakeLists.txt` exists; `ninja -C llvm/llvm/build/release ClangRuntimeSpecializer` still builds cleanly (no regressions).

---

## Phase 2: Foundational (Blocking Prerequisite)

**Purpose**: The `use-cases/CMakeLists.txt` must define the shared variables
(`CLANG_EXE`, `PLUGIN_LIB`, `CXX_FLAGS_LIST`) and the `AllBenchmarks` integration
plumbing before any per-use-case target is registered.

- [x] T002 Implement `benchmarks/use-cases/CMakeLists.txt` with shared CMake variables,
  six `add_subdirectory` stubs, and `AllBenchmarks` object-file accumulation pattern
  (PARENT_SCOPE exports); edit `benchmarks/CMakeLists.txt` to consume the exported
  object-file variables and link them into the `AllBenchmarks` target

**⚠️ CRITICAL**: T003–T008 (all use-case tasks) MUST wait for T001 + T002.

**Checkpoint**: The CMake build succeeds end-to-end after T002 even with empty
per-use-case `CMakeLists.txt` stubs (`add_subdirectory` calls to empty dirs are OK if
each subdir has a minimal `CMakeLists.txt` placeholder).

---

## Phase 3: User Story 1 — Implement All Six Use-Case Benchmarks (Priority: P1) 🎯 MVP

**Goal**: All six benchmark binaries compile, produce valid Google Benchmark JSON output
with three phases each, and pass `assertSpecializedLambdaIsEquivalent`.

**Independent Test**: For each binary `<UC>`, run:
```bash
ninja -C llvm/llvm/build/release <UC>  # must succeed
./<UC> --benchmark_filter=".*"         # must produce ≥ 3 benchmark rows
record_benchmark.py --db /tmp/uc_test.duckdb ./<UC>  # must insert rows without error
```

**Subagent note**: Each T003–T008 task is fully independent (different directories and
files). Run them in parallel. Each task ends with `git commit`.

### UC1 — SQL Predicate Evaluation

- [x] T003 [P] [US1] Implement UC1 SQL predicate benchmark in
  `benchmarks/use-cases/UC1SqlPredicate/` (UC1Kernels.h, UC1Kernels.cpp,
  UC1Benchmark.cpp, CMakeLists.txt)

  **Files to create**:

  `UC1Kernels.h`:
  - Include `ClangRuntimeSpecializer.h`
  - Declare `int64_t count_matching_rows(const uint8_t* rows, int64_t n_rows, int row_stride, int col_offset, double threshold)` — marks `row_stride`, `col_offset`, `threshold` as `// specialization constant`
  - `using SQLSpecialized = clangRuntimeSpecializer::SpecializedLambda<int64_t, const uint8_t*, int64_t>;`
  - Declare `SQLSpecialized create_sql_specialized(int row_stride, int col_offset, double threshold);`
  - Declare `void validate_sql_specialized(int row_stride, int col_offset, double threshold);`

  `UC1Kernels.cpp` (compiled with `-fpass-plugin`):
  - Include `UC1Kernels.h` only (no benchmark headers)
  - `count_matching_rows`: iterate `n_rows` records of `row_stride` bytes, read `double` at `col_offset`, count where value > threshold
  - `create_sql_specialized(row_stride, col_offset, threshold)`: create instance, call `RS->specializeLambda<int64_t>(lambda)` where lambda captures `(row_stride, col_offset, threshold)` and calls `count_matching_rows(rows, n_rows, row_stride, col_offset, threshold)` — **both lambda and `count_matching_rows` are in this TU**
  - `validate_sql_specialized(...)`: call `assertSpecializedLambdaIsEquivalent` with a small reference dataset; throw on mismatch

  `UC1Benchmark.cpp` (compiled with `-fpass-plugin`):
  - Include `UC1Kernels.h` and `ClangRuntimeSpecializerBenchmark.h`
  - Allocate `g_rows` dataset: `std::vector<uint8_t>` of 10M rows × ROW_STRIDE=16 bytes, random double values at COL_OFFSET=8
  - Three benchmarks with manual `->Name(...)`:
    - `BM_g:uc1_sql;n:predicate;t:unspecialized;` — calls `count_matching_rows` directly in loop
    - `BM_g:uc1_sql;n:predicate;t:jit_overhead;` — calls `create_sql_specialized(16, 8, 0.5)` with `->Iterations(1)`, DoNotOptimize the result
    - `BM_g:uc1_sql;n:predicate;t:specialized_exec;` — calls `create_sql_specialized(16, 8, 0.5)` once before loop, then calls `spec(g_rows.data(), N_ROWS)` in loop
  - `main`: calls `validate_sql_specialized(16, 8, 0.5)` before `benchmark::RunSpecifiedBenchmarks()`

  `CMakeLists.txt`:
  - Two custom commands (kernel .o with plugin, benchmark .o with plugin)
  - `add_benchmark(UC1SqlPredicate PARTIAL_SOURCES_INTENDED ${KERNELS_OBJ} ${BENCH_OBJ} ...)`
  - `target_link_libraries(UC1SqlPredicate PRIVATE ClangRuntimeSpecializer benchmark)`
  - `set_target_properties(UC1SqlPredicate PROPERTIES ENABLE_EXPORTS ON)`
  - `-Wl,--export-dynamic`
  - `set(UC1_KERNELS_OBJ ${KERNELS_OBJ} PARENT_SCOPE)` and `set(UC1_BENCH_OBJ ${BENCH_OBJ} PARENT_SCOPE)`

  **Quality gates**:
  1. `ninja -C llvm/llvm/build/release UC1SqlPredicate` exits 0
  2. Binary runs and produces ≥ 3 benchmark rows (unspecialized, jit_overhead, specialized_exec)
  3. `validate_sql_specialized` does not throw (printed in output before benchmarks)
  4. `record_benchmark.py --db /tmp/uc1.duckdb ./UC1SqlPredicate` inserts rows
  
  **Commit**: `git add benchmarks/use-cases/UC1SqlPredicate/ && git commit -m "feat(uc1): add SQL predicate benchmark with lambda factory"`

---

### UC2 — Image Convolution with Fixed Kernel

- [x] T004 [P] [US1] Implement UC2 convolution benchmark in
  `benchmarks/use-cases/UC2Convolution/` (UC2Kernels.h, UC2Kernels.cpp,
  UC2Benchmark.cpp, CMakeLists.txt)

  **Files to create**:

  `UC2Kernels.h`:
  - `extern float g_kernel_coeffs[5];` — the 1D Gaussian coefficients (non-static)
  - Declare `void convolve2d(const float* src, float* dst, int width, int height, const float* kernel_coeffs, int ksize)` — marks `kernel_coeffs`, `ksize`, `width`, `height` as `// specialization constant`
  - `using ConvSpecialized = clangRuntimeSpecializer::SpecializedLambda<void, const float*, float*>;`
  - Declare `ConvSpecialized create_conv_specialized();`
  - Declare `void validate_conv_specialized();`

  `UC2Kernels.cpp` (compiled with plugin):
  - `g_kernel_coeffs[5]` = 5-tap 1D Gaussian: `{0.0625f, 0.25f, 0.375f, 0.25f, 0.0625f}`
  - `convolve2d`: **separable two-pass** — horizontal 1D pass into a temp buffer, then vertical 1D pass from temp buffer into dst. Kernel has `ksize` taps; each pass iterates over the image with boundary clamping
  - `create_conv_specialized()`: lambda captures `(g_kernel_coeffs, ksize=5, width=1920, height=1080)`, calls `convolve2d` in same TU; `RS->specializeLambda<void>(lambda)`
  - `validate_conv_specialized()`: compare specialized vs reference output for a small 64×64 test image

  `UC2Benchmark.cpp`:
  - `g_src`: `std::vector<float>(1920*1080)` random values; `g_dst`: same size, zeroed
  - Three benchmarks: `BM_g:uc2_conv;n:gaussian5x5;t:unspecialized/jit_overhead/specialized_exec;`
  - Unspecialized calls `convolve2d(src, dst, 1920, 1080, g_kernel_coeffs, 5)` directly

  **Quality gates**: same pattern as T003 — build, run, validate, record.

  **Commit**: `feat(uc2): add convolution benchmark with separable lambda factory`

---

### UC7 — DFA Regex Matching with Fixed Pattern

- [x] T005 [P] [US1] Implement UC7 DFA regex benchmark in
  `benchmarks/use-cases/UC7DfaRegex/` (UC7Kernels.h, UC7Kernels.cpp,
  UC7Benchmark.cpp, CMakeLists.cpp)

  **Files to create**:

  `UC7Kernels.h`:
  - `static constexpr int DFA_N_STATES`, `DFA_N_CHARS = 128`, `DFA_START = 0`, `DFA_ACCEPT_BIT = <accept_state_index>`
  - `extern int g_dfa_table[];` — filled at startup
  - Declare `int64_t dfa_match(const char*, int64_t, const int*, int, int, int, int)`
  - `using DFASpecialized = SpecializedLambda<int64_t, const char*, int64_t>;`
  - Declare `DFASpecialized create_dfa_specialized();`
  - Declare `void validate_dfa_specialized();`

  `UC7Kernels.cpp` (compiled with plugin):
  - `g_dfa_table[DFA_N_STATES * DFA_N_CHARS]` — zero-initialized global
  - DFA builder: a `struct DFABuilder { DFABuilder() { build_email_dfa(g_dfa_table, DFA_N_STATES, DFA_N_CHARS); } } g_dfa_builder;` where `build_email_dfa` fills the table programmatically encoding the email-address recognizer (`[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}`). High bytes → reject state.
  - `dfa_match`: the DFA loop — `int state = start_state; for each char c: state = table[state * n_chars + c]; if accept → count match, reset`
  - `create_dfa_specialized()`: lambda with no captures (uses `g_dfa_table` global, `constexpr` params), calls `dfa_match` in same TU; `RS->specializeLambda<int64_t>(lambda)`
  - `validate_dfa_specialized()`: test with known email strings and non-email strings

  `UC7Benchmark.cpp`:
  - `g_corpus`: `std::vector<char>` ≥ 50 MB synthetic text (mix of email-like and random text), generated once at startup
  - Three benchmarks: `BM_g:uc7_dfa;n:email;t:unspecialized/jit_overhead/specialized_exec;`
  - Unspecialized calls `dfa_match(corpus, len, g_dfa_table, N_STATES, N_CHARS, START, ACCEPT)` directly

  **Quality gates**: build, validate (known email strings matched), benchmark runs, record to DuckDB.

  **Commit**: `feat(uc7): add DFA regex benchmark with programmatic table and lambda factory`

---

### UC8 — Incremental View Maintenance

- [x] T006 [P] [US1] Implement UC8 IVM benchmark in
  `benchmarks/use-cases/UC8IVM/` (UC8Kernels.h, UC8Kernels.cpp,
  UC8Benchmark.cpp, CMakeLists.txt)

  **Files to create**:

  `UC8Kernels.h`:
  - Declare `void apply_row_delta(const uint8_t* row, double* agg_buckets, int n_buckets, int group_col_offset, int value_col_offset, int row_stride)` — marks `n_buckets`, `group_col_offset`, `value_col_offset`, `row_stride` as `// specialization constant`
  - `using IVMSpecialized = SpecializedLambda<void, const uint8_t*, double*>;`
  - Declare `IVMSpecialized create_ivm_specialized(int n_buckets, int group_col_offset, int value_col_offset, int row_stride);`
  - Declare `void validate_ivm_specialized(...);`

  `UC8Kernels.cpp` (compiled with plugin):
  - `apply_row_delta`: read `int32_t group_key` at `group_col_offset`, compute `bucket = group_key % n_buckets`, read `double value` at `value_col_offset`, add to `agg_buckets[bucket]`
  - `create_ivm_specialized(n_buckets, group_col_offset, value_col_offset, row_stride)`: lambda captures all four params, calls `apply_row_delta` in same TU

  `UC8Benchmark.cpp`:
  - `g_deltas`: `std::vector<uint8_t>` of 10M rows × ROW_STRIDE=24 bytes with random int32/double fields
  - `g_buckets`: `std::vector<double>(1024, 0.0)` pre-allocated and reused
  - Three benchmarks: `BM_g:uc8_ivm;n:ivm_sum;t:...;`
  - Unspecialized loop calls `apply_row_delta(row_ptr, buckets, 1024, 4, 8, 24)` per event

  **Commit**: `feat(uc8): add IVM benchmark with lambda factory`

---

### UC12 — Columnar Analytics GROUP BY SUM

- [x] T007 [P] [US1] Implement UC12 GROUP BY benchmark in
  `benchmarks/use-cases/UC12GroupBy/` (UC12Kernels.h, UC12Kernels.cpp,
  UC12Benchmark.cpp, CMakeLists.txt)

  **Files to create**:

  `UC12Kernels.h`:
  - Declare `void grouped_sum(const uint8_t* rows, int64_t n_rows, int row_stride, int key_offset, int value_offset, double* out_buckets, int n_buckets)` — marks `row_stride`, `key_offset`, `value_offset`, `n_buckets` as `// specialization constant`
  - `using GroupBySpecialized = SpecializedLambda<void, const uint8_t*, int64_t, double*>;`
  - Declare `GroupBySpecialized create_groupby_specialized(int row_stride, int key_offset, int value_offset, int n_buckets);`
  - Declare `void validate_groupby_specialized(...);`

  `UC12Kernels.cpp` (compiled with plugin):
  - `grouped_sum`: iterate `n_rows` records, extract `int32_t key` at `key_offset`, `double value` at `value_offset`, add to `out_buckets[key % n_buckets]`
  - `create_groupby_specialized(...)`: lambda captures 4 fixed params, calls `grouped_sum` in same TU

  `UC12Benchmark.cpp`:
  - `g_rows12`: 10M rows × 24 bytes; `g_buckets12`: 1024 doubles, zeroed between iterations (benchmark TU must zero buckets before each call in the specialized_exec phase)
  - Three benchmarks: `BM_g:uc12_groupby;n:groupby_sum;t:...;`

  **Commit**: `feat(uc12): add GROUP BY SUM benchmark with lambda factory`

---

### UC14 — Sort with Fixed Comparator

- [x] T008 [P] [US1] Implement UC14 sort benchmark in
  `benchmarks/use-cases/UC14Sort/` (UC14Kernels.h, UC14Kernels.cpp,
  UC14Benchmark.cpp, CMakeLists.txt)

  **Files to create**:

  `UC14Kernels.h`:
  - Declare `int int64_asc_cmp(const void* a, const void* b)` — non-`static`, non-`inline` (FR-028)
  - Declare `void generic_sort(void* data, int64_t n_elements, int element_size, int (*comparator)(const void*, const void*))` — marks `comparator`, `element_size` as `// specialization constant`
  - `using SortSpecialized = SpecializedLambda<void, void*, int64_t>;`
  - Declare `SortSpecialized create_sort_specialized(int (*comparator)(const void*, const void*), int element_size);`
  - Declare `void validate_sort_specialized(...);`

  `UC14Kernels.cpp` (compiled with plugin):
  - `int64_asc_cmp(a, b)`: `return (*(int64_t*)a > *(int64_t*)b) - (*(int64_t*)a < *(int64_t*)b)`
  - `generic_sort`: **median-of-three quicksort** — pivot chosen as median of first/middle/last element; partition with two-pointer scheme; recursive on sub-ranges; comparator called at every comparison. Use iterative stack-based version to avoid stack overflow on 1M elements.
  - `create_sort_specialized(comparator, element_size)`: lambda captures `(comparator, element_size)`, calls `generic_sort` in same TU; `RS->specializeLambda<void>(lambda)`

  `UC14Benchmark.cpp`:
  - `g_data`: `std::vector<int64_t>(1'000'000)` with values 0..N-1, shuffled
  - Three benchmarks: `BM_g:uc14_sort;n:sort_int64;t:...;`
  - **jit_overhead**: calls `create_sort_specialized(&int64_asc_cmp, sizeof(int64_t))` with `->Iterations(1)`
  - **specialized_exec**: calls factory once before loop; in the loop body: `state.PauseTiming()`, `std::shuffle(g_data.begin(), g_data.end(), rng)`, `state.ResumeTiming()`, then `spec(g_data.data(), g_data.size())`
  - **unspecialized**: same PauseTiming/shuffle/ResumeTiming pattern around direct `generic_sort` call

  **Commit**: `feat(uc14): add sort benchmark with median-of-3 quicksort and lambda factory`

---

**Phase 3 Checkpoint**: All six binaries build and pass `assertSpecializedLambdaIsEquivalent`. Each produces ≥ 3 benchmark rows in Google Benchmark output.

---

/## Phase 3b: Size Calibration (FR-007)

**Goal**: Calibrate dataset sizes so unspecialized per-call runtime is within 2× of the
target: SMALL ≈ 0.1 s, MEDIUM ≈ 1 s, LARGE ≈ 10 s, EXTRALARGE ≈ 60 s.

- [X] T009b [P] [US1] Calibrate dataset sizes for all six benchmarks.

  **Steps**:
  1. Build and run all six binaries in release mode with the default benchmark filter,
     recording results into a temporary database:
     ```bash
     cd llvm/llvm/build/release
     ninja UC1SqlPredicate UC2Convolution UC7DfaRegex UC8IVM UC12GroupBy UC14Sort
     for bin in UC1SqlPredicate UC2Convolution UC7DfaRegex UC8IVM UC12GroupBy UC14Sort; do
       python3 <repo>/benchmarks/record_benchmark.py \
         --db /tmp/uc_calibration.duckdb ./$bin
     done
     ```

  2. Query measured unspecialized runtimes by size label:
     ```sql
     SELECT kernel, kv_s, real_time_ns / 1e9 AS unspec_s
     FROM v_ns
     WHERE run_id = (SELECT MAX(run_id) FROM runs)
       AND phase = 'unspecialized'
       AND kv_s IS NOT NULL
     ORDER BY kernel, kv_s;
     ```
     (Use `size_scaling.py` or run the query directly via `duckdb /tmp/uc_calibration.duckdb`.)

  3. For each (kernel, size) pair that falls outside the 2× tolerance band
     (SMALL: 0.05–0.20 s, MEDIUM: 0.5–2.0 s, LARGE: 5–20 s, EXTRALARGE: 30–120 s):
     - Compute corrected size: `new_size = current_size × (target_s / measured_s)`
     - Update the corresponding size constant in the `*Benchmark.cpp` file (or the
       size array/`Arg()` call that drives the benchmark)
     - Rebuild the affected binary

  4. Re-run the affected binaries, re-record, and re-query until all sizes satisfy
     the tolerance band.

  5. Update the "Dataset Sizes and Memory" table in `specs/008-use-case-benchmarks/plan.md`
     with the calibrated sizes.

  **Quality gates**:
  - All six binaries still build and pass `assertSpecializedLambdaIsEquivalent` after size changes
  - DuckDB query shows `unspec_s` within the tolerance band for every (kernel, size) pair

  **Commit**: `feat(calibration): calibrate dataset sizes to hit FR-007 runtime targets`

**⚠️ NOTE**: T009 (AllBenchmarks integration) MUST wait for T009b — sizes baked into
AllBenchmarks object files must reflect calibrated values.

---

## Phase 4: AllBenchmarks Integration & Build Verification (US1 continued)

- [x] T009 [US1] Edit `benchmarks/CMakeLists.txt` to integrate all six use-case kernel and
  benchmark objects into the existing `AllBenchmarks` target, following the
  `add_custom_command` + `PARENT_SCOPE` variable pattern used for the TPC-H benchmarks.
  Then verify `ninja -C llvm/llvm/build/release AllBenchmarks` succeeds.

  **Specific changes to `benchmarks/CMakeLists.txt`**:
  - Add six `add_custom_command` blocks for AllBenchmarks recompilation of each use-case kernel obj (with `-DALL_BENCHMARKS_BUILD=1`)
  - Add the twelve resulting `.o` files (6 kernel + 6 benchmark) to `add_benchmark(AllBenchmarks ...)` source list
  - Add `target_include_directories` entries for the use-cases headers

  **Quality gates**:
  1. `ninja -C llvm/llvm/build/release AllBenchmarks` succeeds
  2. `./AllBenchmarks --benchmark_filter=".*uc[0-9].*"` produces ≥ 18 rows (3 phases × 6 use cases)

  **Commit**: `feat(build): integrate all six use-case benchmarks into AllBenchmarks target`

---

## Phase 5: User Story 2 — Confirm Speedup ≥ 10% for Highest-Potential Archetypes (Priority: P2)

**Goal**: Record release-mode results into DuckDB and verify ≥ 10% speedup for at
least two of UC1, UC2, UC7.

**Independent Test**: Query
`SELECT kernel, unspec_ns / spec_ns AS speedup FROM v_optim_best_per_kernel`
for these three archetypes and verify speedup > 1.10 for ≥ 2.

- [x] T010 [US2] Run all six benchmarks in release mode, record to `benchmarks/benchmarks.duckdb`
  via `record_benchmark.py`, and document speedup results; if ≥ 2 archetypes show < 10%
  speedup, open a finding comment in `specs/008-use-case-benchmarks/plan.md` under a
  new "## Speedup Findings" section documenting the actual ratios and any architectural
  reasons (e.g., kernel too small for JIT amortization, limited constant propagation).

  **Commands**:
  ```bash
  cd llvm/llvm/build/release
  ninja UC1SqlPredicate UC2Convolution UC7DfaRegex UC8IVM UC12GroupBy UC14Sort
  for bin in UC1SqlPredicate UC2Convolution UC7DfaRegex UC8IVM UC12GroupBy UC14Sort; do
    python3 <repo>/benchmarks/record_benchmark.py \
      --db <repo>/benchmarks/benchmarks.duckdb ./$bin
  done
  ```

  **Acceptance criteria** (SC-003): ≥ 2 binaries show `specialized_exec_ns / unspecialized_exec_ns ≤ 0.90`. Document all six ratios. Mark task DONE regardless of numeric result (a "no-benefit" finding is a valid research outcome per spec §Edge Cases).

  **Commit**: `feat(results): record use-case benchmark results; document speedup findings`

---

## Phase 6: User Story 3 — Spec 007 Pipeline Evaluation Compatibility (Priority: P3)

**Goal**: Verify `optimize_benchmarks.py` can run a 5-trial Optuna study against one
use-case binary without errors.

**Independent Test**: `python3 optimize_benchmarks.py --db /tmp/uc_optim.duckdb --binary ./UC1SqlPredicate --study uc1_default --n-trials 5` completes without exception.

- [ ] T011 [US3] Run `optimize_benchmarks.py` against `UC1SqlPredicate` (5 trials) and
  `UC7DfaRegex` (5 trials) in release mode; verify both studies complete and
  `v_optim_best_per_kernel` has entries for `predicate` and `email` kernels; document
  result in `specs/008-use-case-benchmarks/plan.md` under "## Spec 007 Compatibility".

  **Commit**: `feat(results): verify optimize_benchmarks.py compatibility for UC1 and UC7`

---

## Final Phase: Polish & Cross-Cutting Concerns

- [ ] T012 [P] Run `ninja check-smoke-runtime-specializer` in debug build directory; confirm
  all smoke tests remain green (no regressions from new benchmark code)

- [ ] T013 Run `ninja -C llvm/llvm/build/debug UC1SqlPredicate UC2Convolution UC7DfaRegex
  UC8IVM UC12GroupBy UC14Sort` and verify each binary completes without ASan/UBSan errors
  using `ASAN_OPTIONS=detect_leaks=0` (SC-004)

- [ ] T014 [P] Update `specs/008-use-case-benchmarks/plan.md` "## Speedup Findings" section
  with final measurements; update `docs/thesis.typ` with concise bullet points about the
  new use-case benchmark results and the lambda factory specialization pattern

  **Commit**: `docs: update plan and thesis with use-case benchmark results`

---

---

## Phase 7: Streaming Refactor & EXTRALARGE Calibration (T009c, 2026-05-17)

**Goal**: Refactor UC2, UC7, and UC14 so that all benchmarks with buffer-based kernels
stream through a fixed-size in-memory chunk for EXTRALARGE ≈ 60 s targets.
Tasks T015–T017 are fully independent (different files) — run in parallel.

- [x] T015 [P] Refactor `benchmarks/use-cases/UC2Convolution/UC2Benchmark.cpp` from
  dimension-based to tile-based streaming so that EXTRALARGE reaches ≈ 60 s.

  **Context**: Currently every benchmark function reads `int width = state.range(0);
  int height = state.range(1);` and calls the kernel once with that image size.
  The buffers `g_src` and `g_dst` are `IMG_WIDTH_MAX * IMG_HEIGHT_MAX = 25920² ≈ 5.4 GB
  each`. EXTRALARGE = `Args({25920, 25920})` produces only ~8 s for box_filter — far
  below the 60 s target. The fix: fix the tile to 3840×3840, shrink the buffers, and
  add a streaming loop over `n_tiles = state.range(0)`.

  **Exact changes to `UC2Benchmark.cpp`**:

  1. Replace the three constants at the top of the file:
     ```cpp
     // OLD:
     static constexpr int IMG_WIDTH_MAX  = 25920;
     static constexpr int IMG_HEIGHT_MAX = 25920;
     static constexpr int N_PIXELS_MAX   = IMG_WIDTH_MAX * IMG_HEIGHT_MAX;
     // NEW:
     static constexpr int TILE_W = 3840;
     static constexpr int TILE_H = 3840;
     ```

  2. Change the buffer declarations (and the init struct, if it iterates `N_PIXELS_MAX`):
     ```cpp
     // OLD:
     static std::vector<float> g_src(N_PIXELS_MAX, 0.0f);
     static std::vector<float> g_dst(N_PIXELS_MAX, 0.0f);
     // NEW:
     static std::vector<float> g_src(TILE_W * TILE_H, 0.0f);
     static std::vector<float> g_dst(TILE_W * TILE_H, 0.0f);
     ```
     The `UC2DataInit` constructor iterates `g_src` so it automatically adjusts — no change needed there.

  3. For EVERY benchmark function (there are ~27), replace the two-range parameter reads
     with a single n_tiles read and add a streaming loop:

     **Unspecialized variants** (replace the function body):
     ```cpp
     // OLD:
     static void BM_UC2_unspecialized(benchmark::State& state) {
         int width  = (int)state.range(0);
         int height = (int)state.range(1);
         for (auto _ : state) {
             benchmark::DoNotOptimize(g_src.data());
             convolve2d(g_src.data(), g_dst.data(), width, height, g_kernel_coeffs, 5);
             benchmark::DoNotOptimize(g_dst.data());
         }
     }
     // NEW:
     static void BM_UC2_unspecialized(benchmark::State& state) {
         int64_t n_tiles = state.range(0);
         for (auto _ : state) {
             benchmark::DoNotOptimize(g_src.data());
             for (int64_t t = 0; t < n_tiles; ++t)
                 convolve2d(g_src.data(), g_dst.data(), TILE_W, TILE_H, g_kernel_coeffs, 5);
             benchmark::DoNotOptimize(g_dst.data());
         }
     }
     ```
     Apply the same pattern to all other `*_unspecialized` functions, replacing the kernel
     call inside the loop with the appropriate kernel (box_filter, sobel_edge_detect, etc.)
     with `TILE_W, TILE_H` as width/height.

     **jit_overhead variants** (remove the width/height reads; use fixed TILE):
     ```cpp
     // OLD:
     static void BM_UC2_jit_overhead(benchmark::State& state) {
         int width  = (int)state.range(0);
         int height = (int)state.range(1);
         for (auto _ : state) {
             benchmark::DoNotOptimize(create_conv_specialized(width, height));
         }
     }
     // NEW:
     static void BM_UC2_jit_overhead(benchmark::State& state) {
         for (auto _ : state) {
             benchmark::DoNotOptimize(create_conv_specialized(TILE_W, TILE_H));
         }
     }
     ```
     Apply similarly to all `*_jit_overhead` variants, passing `TILE_W, TILE_H`
     (and any other fixed args like `ksize=2` for box_filter) directly.

     **specialized_exec variants** (streaming loop calling `spec()` n_tiles times):
     ```cpp
     // OLD:
     static void BM_UC2_specialized_exec(benchmark::State& state) {
         int width  = (int)state.range(0);
         int height = (int)state.range(1);
         auto spec = create_conv_specialized(width, height);
         for (auto _ : state) {
             benchmark::DoNotOptimize(g_src.data());
             spec(g_src.data(), g_dst.data());
             benchmark::DoNotOptimize(g_dst.data());
         }
     }
     // NEW:
     static void BM_UC2_specialized_exec(benchmark::State& state) {
         int64_t n_tiles = state.range(0);
         auto spec = create_conv_specialized(TILE_W, TILE_H);
         for (auto _ : state) {
             benchmark::DoNotOptimize(g_src.data());
             for (int64_t t = 0; t < n_tiles; ++t)
                 spec(g_src.data(), g_dst.data());
             benchmark::DoNotOptimize(g_dst.data());
         }
     }
     ```
     Apply similarly to all `*_specialized_exec` variants.

  4. Replace the `UC2_BENCHMARK_SPEC` macro call(s) at the bottom of the file. The
     existing `#ifdef ALL_BENCHMARKS_BUILD` block (if any) and `#else` standalone block:

     **The file currently has a single call at the bottom** (no `#ifdef ALL_BENCHMARKS_BUILD` guard):
     ```cpp
     // OLD:
     UC2_BENCHMARK_SPEC(
         Args({3840, 3840}),
         Args({10000, 10000}),
         Args({25920, 25920}),
         Args({25920, 25920})
     )
     // NEW:
     #ifdef ALL_BENCHMARKS_BUILD
     UC2_BENCHMARK_SPEC(
         Arg(1),
         Arg(6),
         Arg(58),
         Arg(58)
     )
     #else
     UC2_BENCHMARK_SPEC(
         Arg(1),
         Arg(6),
         Arg(58),
         Arg(345)
     )
     #endif
     ```

  **Quality gates**:
  1. `ninja -C /home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/llvm/build/release UC2Convolution` exits 0
  2. `./tools/runtime-specialization/benchmarks/use-cases/UC2Convolution/UC2Convolution --benchmark_filter="g:uc.*s:SMALL;t:unspecialized" --benchmark_min_time=0.1` runs and produces benchmark rows (quick sanity check, do not run full EXTRALARGE)

  **Commit** (from the llvm repo root at `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm`):
  ```bash
  git add runtime-specialization/benchmarks/use-cases/UC2Convolution/UC2Benchmark.cpp
  git commit -m "feat(uc2): add tile-based streaming for EXTRALARGE; reduce buffer from 5.4GB to 115MB

  Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>"
  ```

---

- [x] T016 [P] Refactor `benchmarks/use-cases/UC7DfaRegex/UC7Benchmark.cpp` to stream
  through a 500 MB corpus chunk instead of allocating 15 GB, enabling EXTRALARGE ≈ 60 s.

  **Context**: Currently `CORPUS_MAX = 15000 MB` (standalone) and `g_corpus` is allocated
  to that size, requiring 15 GB RAM. Each benchmark function calls
  `dfa_match(g_corpus.data(), corpus_size, ...)` in one pass. EXTRALARGE = 15000 MB
  only yields ~34.5 s. The fix: reduce `CORPUS_MAX` → `CORPUS_CHUNK = 500 MB`, add a
  streaming loop `for rem = corpus_size; rem > 0; rem -= CORPUS_CHUNK`, and set
  EXTRALARGE = 26000 MB → ~60 s.

  **Exact changes to `UC7Benchmark.cpp`**:

  1. In the `#else` (standalone) block of the `CORPUS_MAX` declaration, change:
     ```cpp
     // OLD (standalone):
     static constexpr int64_t CORPUS_MAX = 15000LL * 1024 * 1024;  // 15 GB for standalone
     // NEW (standalone):
     static constexpr int64_t CORPUS_MAX = 500LL * 1024 * 1024;  // 500 MB streaming chunk
     ```
     The `#ifdef ALL_BENCHMARKS_BUILD` block (`CORPUS_MAX = 1000 MB`) stays unchanged.

  2. For EVERY unspecialized benchmark function (there are ~9 variants: email_low,
     email_tradeoff, email_abstract, url_low, url_tradeoff, url_abstract, multi_low,
     multi_tradeoff, multi_abstract), replace the single call with a streaming loop:
     ```cpp
     // OLD:
     static void BM_UC7_email_low_unspecialized(benchmark::State& state) {
         int64_t corpus_size = state.range(0);
         for (auto _ : state) {
             benchmark::DoNotOptimize(
                 dfa_match(g_corpus.data(), corpus_size,
                           g_dfa_table, DFA_N_STATES, DFA_N_CHARS,
                           DFA_START, DFA_ACCEPT));
         }
     }
     // NEW:
     static void BM_UC7_email_low_unspecialized(benchmark::State& state) {
         int64_t corpus_size = state.range(0);
         for (auto _ : state) {
             int64_t result = 0;
             for (int64_t rem = corpus_size; rem > 0; rem -= CORPUS_MAX)
                 result += dfa_match(g_corpus.data(), std::min(rem, CORPUS_MAX),
                                     g_dfa_table, DFA_N_STATES, DFA_N_CHARS,
                                     DFA_START, DFA_ACCEPT);
             benchmark::DoNotOptimize(result);
         }
     }
     ```
     Apply the same pattern to all `*_unspecialized` functions (url variants use `URL_ACCEPT`,
     multi variants use `multi_accept_dfa_match` or whatever the multi-pattern call looks like).

  3. For EVERY specialized_exec benchmark function, add the same streaming loop calling `spec()`:
     ```cpp
     // OLD:
     static void BM_UC7_email_low_specialized_exec(benchmark::State& state) {
         int64_t corpus_size = state.range(0);
         auto spec = create_dfa_specialized();
         for (auto _ : state) {
             benchmark::DoNotOptimize(spec(g_corpus.data(), corpus_size));
         }
     }
     // NEW:
     static void BM_UC7_email_low_specialized_exec(benchmark::State& state) {
         int64_t corpus_size = state.range(0);
         auto spec = create_dfa_specialized();
         for (auto _ : state) {
             int64_t result = 0;
             for (int64_t rem = corpus_size; rem > 0; rem -= CORPUS_MAX)
                 result += spec(g_corpus.data(), std::min(rem, CORPUS_MAX));
             benchmark::DoNotOptimize(result);
         }
     }
     ```
     The `*_jit_overhead` functions do NOT need streaming (they don't process data).

  4. Change the EXTRALARGE value in the standalone `#else` block from `15000 MB` to `26000 MB`:
     ```cpp
     // OLD:
     UC7_BENCHMARK_SPEC(
         Arg(44LL * 1024 * 1024),
         Arg(440LL * 1024 * 1024),
         Arg(4350LL * 1024 * 1024),
         Arg(15000LL * 1024 * 1024)
     )
     // NEW:
     UC7_BENCHMARK_SPEC(
         Arg(44LL * 1024 * 1024),
         Arg(440LL * 1024 * 1024),
         Arg(4350LL * 1024 * 1024),
         Arg(26000LL * 1024 * 1024)
     )
     ```

  **Quality gates**:
  1. `ninja -C /home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/llvm/build/release UC7DfaRegex` exits 0
  2. Binary runs with `--benchmark_filter="g:uc.*s:SMALL;t:unspecialized" --benchmark_min_time=0.1` and produces rows

  **Commit** (from `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm`):
  ```bash
  git add runtime-specialization/benchmarks/use-cases/UC7DfaRegex/UC7Benchmark.cpp
  git commit -m "feat(uc7): add corpus streaming loop; reduce allocation from 15GB to 500MB; EXTRALARGE=26GB

  Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>"
  ```

---

- [x] T017 [P] Increase `N_SORT_MAX` and `N_STRUCT_MAX` in
  `benchmarks/use-cases/UC14Sort/UC14Benchmark.cpp` to enable EXTRALARGE ≈ 34–66 s.

  **Context**: Currently `N_SORT_MAX = 200'000'000` (generic_sort, int64_t, 8B) and
  `N_STRUCT_MAX = 50'000'000` (struct/multi-key sort, 16B). EXTRALARGE = 200M for
  generic_sort gives ~20–33 s (below [30, 120] for fast variants). EXTRALARGE = 50M for
  struct/multi gives only ~6–8 s. The fix: increase N_SORT_MAX to 400M (6.4 GB total
  allocations) and N_STRUCT_MAX to 250M (8 GB), and update the EXTRALARGE macro args.
  In-place quicksort cannot stream, so only buffer enlargement is possible.

  **Exact changes to `UC14Benchmark.cpp`**:

  1. Change `N_SORT_MAX`:
     ```cpp
     // OLD:
     static constexpr int64_t N_SORT_MAX = 200'000'000;
     // NEW:
     static constexpr int64_t N_SORT_MAX = 400'000'000;
     ```

  2. Change `N_STRUCT_MAX`:
     ```cpp
     // OLD:
     static constexpr int64_t N_STRUCT_MAX = 50'000'000;
     // NEW:
     static constexpr int64_t N_STRUCT_MAX = 250'000'000;
     ```

  3. Update the EXTRALARGE argument in every generic_sort macro call (there are 3:
     `UC14_BENCHMARK_SPEC`, `UC14_GENERIC_TRADEOFF_SPEC`, `UC14_GENERIC_ABSTRACT_SPEC`):
     ```cpp
     // OLD:
     UC14_BENCHMARK_SPEC(
         Arg(900'000),
         Arg(8'000'000),
         Arg(70'000'000),
         Arg(200'000'000)   // ← EXTRALARGE
     )
     // NEW:
     UC14_BENCHMARK_SPEC(
         Arg(900'000),
         Arg(8'000'000),
         Arg(70'000'000),
         Arg(400'000'000)   // ← EXTRALARGE
     )
     ```
     Apply identically to `UC14_GENERIC_TRADEOFF_SPEC` and `UC14_GENERIC_ABSTRACT_SPEC`.

  4. Update the EXTRALARGE argument in every struct/multi-key sort macro call (there are 6:
     3 struct variants + 3 multi-key variants). Currently EXTRALARGE = 50'000'000
     (same as LARGE). Change to 250'000'000:
     ```cpp
     // OLD:
     UC14_STRUCT_LOW_SPEC(
         Arg(1'000'000),
         Arg(8'500'000),
         Arg(50'000'000),
         Arg(50'000'000)  // ← EXTRALARGE (same as LARGE, wrong)
     )
     // NEW:
     UC14_STRUCT_LOW_SPEC(
         Arg(1'000'000),
         Arg(8'500'000),
         Arg(50'000'000),
         Arg(250'000'000)  // ← EXTRALARGE
     )
     ```
     Apply identically to UC14_STRUCT_TRADEOFF_SPEC, UC14_STRUCT_ABSTRACT_SPEC,
     UC14_MULTI_LOW_SPEC, UC14_MULTI_TRADEOFF_SPEC, UC14_MULTI_ABSTRACT_SPEC.

  **Quality gates**:
  1. `ninja -C /home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm/llvm/build/release UC14Sort` exits 0
  2. Binary runs with `--benchmark_filter="g:uc.*s:SMALL;t:unspecialized" --benchmark_min_time=0.1` and produces rows

  **Commit** (from `/home/Jakob.Gerhardt/CLionProjects/Masterarbeit/llvm`):
  ```bash
  git add runtime-specialization/benchmarks/use-cases/UC14Sort/UC14Benchmark.cpp
  git commit -m "feat(uc14): increase N_SORT_MAX to 400M and N_STRUCT_MAX to 250M for EXTRALARGE≈60s

  Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>"
  ```

---

**Phase 7 Checkpoint**: All three modified binaries build and run with SMALL filter.
T015/T016/T017 each committed independently.

---

## Dependencies & Execution Order

### Phase Dependencies

```
T001 → T002 → T003–T008 (parallel) → T009b → T009 → T010 → T011 → T012–T014
```

- **T001 (Setup)**: No dependencies — start immediately
- **T002 (Foundational)**: Depends on T001
- **T003–T008 (US1, use cases)**: All depend on T002; run in parallel
- **T009b (Size calibration)**: Depends on T003–T008 all complete; calibrates sizes to FR-007 targets
- **T009 (AllBenchmarks)**: Depends on T009b (must use calibrated sizes)
- **T010 (US2, record results)**: Depends on T009
- **T011 (US3, Optuna)**: Depends on T009; can run in parallel with T010
- **T012–T014 (Polish)**: Depend on T010 + T011

### Parallel Opportunities

```bash
# After T002 completes, launch all six use cases in parallel:
Agent: "Implement UC1SqlPredicate benchmark (T003)"
Agent: "Implement UC2Convolution benchmark (T004)"
Agent: "Implement UC7DfaRegex benchmark (T005)"
Agent: "Implement UC8IVM benchmark (T006)"
Agent: "Implement UC12GroupBy benchmark (T007)"
Agent: "Implement UC14Sort benchmark (T008)"

# After T003–T008 complete, run T009b (size calibration) before T009
Agent: "Calibrate dataset sizes to FR-007 runtime targets (T009b)"

# After T009, run in parallel:
Agent: "Record benchmarks and document speedups (T010)"
Agent: "Verify optimize_benchmarks.py compatibility (T011)"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete T001 + T002 (Setup + Foundational)
2. Complete T003–T008 in parallel (six use cases)
3. Complete T009b (size calibration — measure and adjust sizes to FR-007 targets)
4. Complete T009 (AllBenchmarks integration — using calibrated sizes)
5. **STOP and VALIDATE**: each binary runs and passes equivalence check
6. Advance to T010 if recordings needed for thesis

### Incremental Delivery

1. T001 + T002 → Foundation ready
2. T003 alone → UC1 SQL predicate working → validate independently
3. T004–T008 in parallel → all six working
4. T009 → AllBenchmarks green
5. T010 + T011 → results recorded and pipeline compatibility confirmed

---

## Quality Gate Summary

| Gate | Command | Criterion |
|------|---------|-----------|
| Build (per use case) | `ninja -C .../release <target>` | Exit 0 |
| Equivalence | `assertSpecializedLambdaIsEquivalent` in binary startup | No exception |
| Benchmark output | `./<binary> --benchmark_filter=".*"` | ≥ 3 rows, all three phases |
| DuckDB ingestion | `record_benchmark.py --db ... ./<binary>` | Rows inserted, no schema error |
| AllBenchmarks | `ninja -C .../release AllBenchmarks` + `./AllBenchmarks --benchmark_filter="uc"` | ≥ 18 rows |
| No regressions | `ninja check-smoke-runtime-specializer` (debug) | All smoke tests green |
| Speedup (SC-003) | DuckDB query on `v_optim_best_per_kernel` | ≥ 2 of 6 show ≥ 10% speedup |
| Optuna compat (SC-006) | `optimize_benchmarks.py --n-trials 5` | Completes, `v_optim_best_per_kernel` row created |

---

## Notes

- `[P]` tasks touch different directories — safe to run in parallel subagents
- `[US1]` maps to User Story 1 (all six use case implementations)
- `[US2]` maps to User Story 2 (speedup confirmation)
- `[US3]` maps to User Story 3 (Spec 007 integration)
- Every subagent MUST commit after passing quality gates — do not batch multiple tasks into one commit
- A "no-benefit" speedup result is a valid thesis finding; do not attempt to "fix" it by changing kernel implementations
- The `create_*_specialized()` factory function MUST be in the kernel TU (not benchmark TU) for JIT optimization to work — see plan.md "Lambda Factory Pattern" section
