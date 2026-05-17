# Tasks: Benchmark Dynamic Buffer Allocation

**Input**: Design documents from `specs/013-dynamic-buffer-alloc/`
**Prerequisites**: plan.md, spec.md, research.md

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no inter-task dependencies)
- **[Story]**: User story from spec.md (US1 = memory reduction, US2 = timing correctness, US3 = polybench)

---

## Phase 1: UC Benchmark Files — Lazy Buffer Allocation (Priority: P1)

**Goal (US1)**: Replace all startup-time static buffer allocations in the 6 UC benchmark files with on-demand `setup`/`teardown` functions wired to Google Benchmark's `->Setup(fn)->Teardown(fn)` API. Eliminates ~18 GB of startup RSS.

**Independent Test**: Build `AllBenchmarks`, run with `--benchmark_filter=BM_UC14.*SMALL`, observe VmRSS in `/proc/self/status` stays below 16 GB (only UC14 data is live during the run, not all files simultaneously).

**Pattern to apply in every UC file**:
1. Remove the static initializer lambda (`= [] { ... }()`); leave the vector declared empty (`static std::vector<T> g_xxx;`).
2. Add a `static int g_xxx_refcount = 0;` guard.
3. Add `static void setup_xxx(const benchmark::State&)`: increment refcount; if it was 0, allocate and initialize all buffers.
4. Add `static void teardown_xxx(const benchmark::State&)`: decrement refcount; if it reaches 0, call `.clear()` + `.shrink_to_fit()` on every vector.
5. Append `->Setup(setup_xxx)->Teardown(teardown_xxx)` to **every** `BENCHMARK(...)` registration line in the file (including inside macros).

- [X] T001 [P] [US1] Refactor UC1SqlPredicate: replace `g_rows` and `g_out_indices` static init lambdas with `setup_uc1`/`teardown_uc1` + refcount; wire to all BENCHMARK registrations in `benchmarks/use-cases/UC1SqlPredicate/UC1Benchmark.cpp`

- [X] T002 [P] [US1] Refactor UC2Convolution: replace `g_src` and `g_dst` static init struct with `setup_uc2`/`teardown_uc2` + refcount; call `std::generate` for `g_src` init inside setup; wire to all BENCHMARK registrations in `benchmarks/use-cases/UC2Convolution/UC2Benchmark.cpp`

- [X] T003 [P] [US1] Refactor UC7DfaRegex: replace `static std::vector<char> g_corpus = make_corpus(CORPUS_MAX)` with `static std::vector<char> g_corpus;` and call `g_corpus = make_corpus(CORPUS_MAX)` inside `setup_uc7`/`teardown_uc7` + refcount; wire to all BENCHMARK registrations in `benchmarks/use-cases/UC7DfaRegex/UC7Benchmark.cpp`

- [X] T004 [P] [US1] Refactor UC8IVM: replace `g_deltas` static init lambda with `setup_uc8`/`teardown_uc8` + refcount; also clear `g_buckets` and `g_count_buckets` in teardown (or leave as-is since they're small — but clear + shrink for correctness); wire to all BENCHMARK registrations in `benchmarks/use-cases/UC8IVM/UC8Benchmark.cpp`

- [X] T005 [P] [US1] Refactor UC12GroupBy: replace `g_rows12` static init lambda with `setup_uc12`/`teardown_uc12` + refcount; wire to all BENCHMARK registrations in `benchmarks/use-cases/UC12GroupBy/UC12Benchmark.cpp`

- [X] T006 [P] [US1] Refactor UC14Sort: replace `g_reference_data`, `g_sort_data`, `g_struct_reference`, `g_struct_data` static init lambdas with `setup_uc14`/`teardown_uc14` + single refcount managing all four vectors; wire to every `BENCHMARK(...)` registration in all six macros (`UC14_BENCHMARK_SPEC`, `UC14_GENERIC_TRADEOFF_SPEC`, `UC14_GENERIC_ABSTRACT_SPEC`, `UC14_STRUCT_LOW_SPEC`, `UC14_STRUCT_TRADEOFF_SPEC`, `UC14_STRUCT_ABSTRACT_SPEC`) and any standalone registrations in `benchmarks/use-cases/UC14Sort/UC14Benchmark.cpp`

**Checkpoint US1**: Build UC14 standalone binary (`ninja <uc14-target>`) — must compile and link. Run with `--benchmark_filter=BM_UC14_unspecialized.*SMALL` — must complete and report a timing result.

---

## Phase 2: Polybench — Per-Kernel Setup/Teardown (Priority: P3)

**Goal (US3)**: Replace the 28 static constructor objects (`static struct XxxInit { ... } _xxx_arrinit;`) in `polybench_bench.cpp` with per-kernel `pb_setup_K`/`pb_teardown_K` functions. Extend `POLYBENCH_BENCHMARK_SPEC` macro to chain them on every registration. Kernels with no heap arrays (durbin, jacobi-1d) get no-op stubs.

**Independent Test**: Build the polybench binary, run `--benchmark_filter=BM_g:polybench;n:gemm` (the largest kernel by array size), verify VmRSS drops after that group finishes before the next kernel group starts.

**Ordering note**: T007 (macro change) changes the `POLYBENCH_BENCHMARK_SPEC` macro to reference `pb_setup_##K` / `pb_teardown_##K`. Tasks T008–T013 add those functions per kernel. All edits are in the same file; T007 must be applied before T008–T013 so that the compiler can see the function references. The per-kernel tasks (T008–T013) can be done in any order relative to each other once T007 is done.

- [X] T007 [US3] Extend `POLYBENCH_BENCHMARK_SPEC` macro in `benchmarks/polybench/polybench_bench.cpp`: append `->Setup(pb_setup_##K)->Teardown(pb_teardown_##K)` to every one of the 20 `BENCHMARK(...)...;` lines in the macro body (4 benchmark types × 5 sizes); note `jit_analysis` lines already have `->Iterations(1)->UseManualTime()` — add setup/teardown after those too

- [X] T008 [P] [US3] Datamining kernels in `benchmarks/polybench/polybench_bench.cpp`: remove `_corr_arrinit` and `_cov_arrinit` static constructors; add `pb_setup_correlation`/`pb_teardown_correlation` (allocate `g_corr_data[N][M]`, `g_corr_corr[M][M]`; free on teardown) and matching functions for `covariance`; use per-kernel refcount

- [X] T009 [P] [US3] Linear-algebra/kernels in `benchmarks/polybench/polybench_bench.cpp`: remove `_2mm_arrinit`, `_3mm_arrinit`, `_atax_arrinit`, `_bicg_arrinit`, `_doitgen_arrinit`, `_mvt_arrinit` static constructors; add `pb_setup_K`/`pb_teardown_K` for each (2mm, 3mm, atax, bicg, doitgen, mvt) with per-kernel refcounts; allocate/free the arrays listed in the comments (e.g., `new double[NI][NJ]()` etc.)

- [X] T010 [P] [US3] Linear-algebra/blas in `benchmarks/polybench/polybench_bench.cpp`: remove `_gemm_arrinit`, `_gemver_arrinit`, `_gesummv_arrinit`, `_symm_arrinit`, `_syr2k_arrinit`, `_syrk_arrinit`, `_trmm_arrinit`; add `pb_setup_K`/`pb_teardown_K` for gemm, gemver, gesummv, symm, syr2k, syrk, trmm with per-kernel refcounts

- [X] T011 [P] [US3] Linear-algebra/solvers in `benchmarks/polybench/polybench_bench.cpp`: remove `_cholesky_arrinit`, `_gramschmidt_arrinit`, `_lu_arrinit`, `_ludcmp_arrinit`, `_trisolv_arrinit`; add `pb_setup_K`/`pb_teardown_K` for cholesky, gramschmidt, lu, ludcmp, trisolv; add **no-op** stubs `pb_setup_durbin`/`pb_teardown_durbin` (durbin has only stack arrays `g_durbin_r[N]`, `g_durbin_y[N]`)

- [X] T012 [P] [US3] Medley kernels in `benchmarks/polybench/polybench_bench.cpp`: remove `_deriche_arrinit`, `_fw_arrinit`, `_nussinov_arrinit`; add `pb_setup_K`/`pb_teardown_K` for deriche (4 float arrays), floyd_warshall (`g_fw_path[N][N]`), nussinov (`g_nussinov_table[N][N]`) with per-kernel refcounts

- [X] T013 [P] [US3] Stencil kernels in `benchmarks/polybench/polybench_bench.cpp`: remove `_adi_arrinit`, `_fdtd2d_arrinit`, `_heat3d_arrinit`, `_jacobi2d_arrinit`, `_seidel2d_arrinit`; add `pb_setup_K`/`pb_teardown_K` for adi, fdtd_2d, heat_3d, jacobi_2d, seidel_2d with per-kernel refcounts; add **no-op** stubs `pb_setup_jacobi_1d`/`pb_teardown_jacobi_1d` (jacobi-1d has only stack arrays)

**Checkpoint US3**: Build polybench standalone binary — must compile. Run `--benchmark_filter=s:MINI` to exercise all 30 kernels at minimal size; must complete without crash.

---

## Phase 3: Verification — Timing Correctness (Priority: P2)

**Goal (US2)**: Confirm that no allocation work leaked into timed windows. Build and run a spot-check of timing-sensitive benchmarks and verify the smoke test suite stays green.

- [X] T014 [US2] Build release AllBenchmarks binary (`ninja <AllBenchmarks-release-target>` from `llvm/llvm/build/release`) and run `--benchmark_filter=BM_UC14.*SMALL --benchmark_repetitions=3`; verify reported median ms times are within ±10% of pre-change baseline (note in commit message if timing shifts are observed)

- [X] T015 [US2] Run `ninja check-smoke-runtime-specializer` from `llvm/llvm/build/debug`; all tests must pass (this is the hard gate before merging per constitution principle III)

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (US1 — UC files)**: No prerequisites — all 6 tasks start immediately
- **Phase 2 (US3 — Polybench)**: T007 must complete before T008–T013; T008–T013 are independent of each other
- **Phase 3 (US2 — Verification)**: Depends on all of Phase 1 and Phase 2 completing

### User Story Dependencies

- **US1 (P1)**: No dependencies — can start immediately with T001–T006
- **US3 (P3)**: T007 depends on nothing; T008–T013 depend on T007
- **US2 (P2)**: Depends on US1 and US3 completion (needs all changes present to verify)

### Within Each Phase

- T001–T006 are in different files — all six can be implemented in parallel
- T008–T013 are all in `polybench_bench.cpp` — sequential (one developer); or split the file into sections mentally and apply all changes in one editing pass
- T014–T015 are sequential (build then test)

---

## Parallel Execution Examples

### Phase 1 — All UC files in parallel

```
T001: UC1Benchmark.cpp   (800 MB freed)
T002: UC2Benchmark.cpp   (115 MB freed)
T003: UC7Benchmark.cpp   (500 MB freed)
T004: UC8Benchmark.cpp   (1.2 GB freed)
T005: UC12Benchmark.cpp  (1.2 GB freed)
T006: UC14Benchmark.cpp  (14.4 GB freed)  ← largest single task
```

### Phase 2 — Polybench kernel groups in parallel (conceptually)

```
T008: datamining (correlation, covariance)
T009: lin-alg/kernels (2mm, 3mm, atax, bicg, doitgen, mvt)
T010: lin-alg/blas (gemm, gemver, gesummv, symm, syr2k, syrk, trmm)
T011: solvers (cholesky, durbin, gramschmidt, lu, ludcmp, trisolv)
T012: medley (deriche, floyd_warshall, nussinov)
T013: stencils (adi, fdtd_2d, heat_3d, jacobi_1d, jacobi_2d, seidel_2d)
```

(In practice one developer edits the file top-to-bottom; this ordering matches file order.)

---

## Implementation Strategy

### MVP (US1 only — T001–T006)

1. Complete T001–T006 (UC files) — eliminates ~18 GB startup cost
2. Build and run `--benchmark_filter=BM_UC14.*SMALL` to smoke-test
3. **Stop and validate**: peak RSS drops from ~23 GB to ~5 GB (polybench still static-initialized, but that's only ~3–5 GB)
4. This alone brings the suite runnable on 16 GB machines

### Full Delivery

1. Phase 1: T001–T006 (UC files) → commit
2. Phase 2: T007 then T008–T013 (Polybench) → commit
3. Phase 3: T014–T015 (Verification) → confirm and commit

---

## Notes

- [P] tasks operate on different files or independent sections — safe to parallelize
- Setup/teardown run outside timed windows by Google Benchmark design — no timing risk
- `shrink_to_fit()` is non-binding but releases capacity on all relevant stdlibs (libc++, libstdc++)
- Polybench refcounts are per-kernel (20 registrations × 1 kernel = refcount peaks at 20, returns to 0 after all 20 finish)
- UC refcounts are per-file (all benchmark registrations in the file share one refcount)
- Do NOT modify benchmark names, `state.range()` sizes, or output counter names — `record_benchmark.py` depends on exact benchmark name format
