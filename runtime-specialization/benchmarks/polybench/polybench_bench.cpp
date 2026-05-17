#include <iostream>
#include <benchmark/benchmark.h>
#include "ClangRuntimeSpecializerBenchmark.h"

namespace CRS = clangRuntimeSpecializer;

// ── PolyBench runtime stubs ───────────────────────────────────────────────────
// The renamed PolyBench main() functions call polybench_alloc_data / _free_data
// to manage heap arrays; they are never executed but must link.
extern "C" void* polybench_alloc_data(unsigned long long int n, int elt_size) {
    return std::malloc((std::size_t)n * (std::size_t)elt_size);
}
extern "C" void polybench_free_data(void* p) {
    std::free(p);
}

// ── Macros ────────────────────────────────────────────────────────────────────

#define POLYBENCH_IMPL_1(K) \
  static void BM_unspecialized____##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0)); \
    CRS::benchmarkUnspecialized(S, kernel_##K, A); } \
  static void BM_jit_overhead_____##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0)); \
    CRS::benchmarkJITOverhead(S, kernel_##K, A, A); } \
  static void BM_specialized_exec_##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0)); \
    CRS::benchmarkSpecializedExec(S, kernel_##K, A); } \
  static void BM_jit_analysis____##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0)); \
    CRS::benchmarkJITAnalysis(S, kernel_##K, A); }

#define POLYBENCH_IMPL_2(K) \
  static void BM_unspecialized____##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1)); \
    CRS::benchmarkUnspecialized(S, kernel_##K, A); } \
  static void BM_jit_overhead_____##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1)); \
    CRS::benchmarkJITOverhead(S, kernel_##K, A, A); } \
  static void BM_specialized_exec_##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1)); \
    CRS::benchmarkSpecializedExec(S, kernel_##K, A); } \
  static void BM_jit_analysis____##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1)); \
    CRS::benchmarkJITAnalysis(S, kernel_##K, A); }

#define POLYBENCH_IMPL_3(K) \
  static void BM_unspecialized____##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1), (int)S.range(2)); \
    CRS::benchmarkUnspecialized(S, kernel_##K, A); } \
  static void BM_jit_overhead_____##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1), (int)S.range(2)); \
    CRS::benchmarkJITOverhead(S, kernel_##K, A, A); } \
  static void BM_specialized_exec_##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1), (int)S.range(2)); \
    CRS::benchmarkSpecializedExec(S, kernel_##K, A); } \
  static void BM_jit_analysis____##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1), (int)S.range(2)); \
    CRS::benchmarkJITAnalysis(S, kernel_##K, A); }

#define POLYBENCH_IMPL_4(K) \
  static void BM_unspecialized____##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1), (int)S.range(2), (int)S.range(3)); \
    CRS::benchmarkUnspecialized(S, kernel_##K, A); } \
  static void BM_jit_overhead_____##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1), (int)S.range(2), (int)S.range(3)); \
    CRS::benchmarkJITOverhead(S, kernel_##K, A, A); } \
  static void BM_specialized_exec_##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1), (int)S.range(2), (int)S.range(3)); \
    CRS::benchmarkSpecializedExec(S, kernel_##K, A); } \
  static void BM_jit_analysis____##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1), (int)S.range(2), (int)S.range(3)); \
    CRS::benchmarkJITAnalysis(S, kernel_##K, A); }

#define POLYBENCH_IMPL_5(K) \
  static void BM_unspecialized____##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1), (int)S.range(2), (int)S.range(3), (int)S.range(4)); \
    CRS::benchmarkUnspecialized(S, kernel_##K, A); } \
  static void BM_jit_overhead_____##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1), (int)S.range(2), (int)S.range(3), (int)S.range(4)); \
    CRS::benchmarkJITOverhead(S, kernel_##K, A, A); } \
  static void BM_specialized_exec_##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1), (int)S.range(2), (int)S.range(3), (int)S.range(4)); \
    CRS::benchmarkSpecializedExec(S, kernel_##K, A); } \
  static void BM_jit_analysis____##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1), (int)S.range(2), (int)S.range(3), (int)S.range(4)); \
    CRS::benchmarkJITAnalysis(S, kernel_##K, A); }




// ── PolyBench Kernel Implementations ─────────────────────────────────────────
// Strategy: for each kernel, reset all PB macros, rename symbols to avoid
// conflicts, #include the .c, then declare arrays (all 2-D/3-D arrays are
// heap-allocated via constructors to keep BSS small) and define the wrapper.

#define EXTRALARGE_DATASET

// ── correlation ───────────────────────────────────────────────────────────────
#define POLYBENCH_KERNEL      correlation
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/datamining/correlation/correlation.c"
#define kernel_correlation    POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_correlation
// After include: M=2600, N=3000, DATA_TYPE=double
double (*g_corr_data)[M] = nullptr; // [N][M]
double (*g_corr_corr)[M] = nullptr; // [M][M]
double g_corr_mean[M];
double g_corr_stddev[M];
static struct CorrelationInit {
    CorrelationInit() {
        g_corr_data = new double[N][M]();
        g_corr_corr = new double[M][M]();
    }
} _corr_arrinit;
extern "C" void kernel_correlation(int m, int n) __asm__("kernel_correlation");
extern "C" void kernel_correlation(int m, int n) {
    __pb_correlation_kernel(m, n, (double)n,
        g_corr_data, g_corr_corr, g_corr_mean, g_corr_stddev);
}
POLYBENCH_IMPL_2(correlation)

// ── covariance ────────────────────────────────────────────────────────────────
#define POLYBENCH_KERNEL      covariance
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/datamining/covariance/covariance.c"
#define kernel_covariance     POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_covariance
// After include: M=2600, N=3000, DATA_TYPE=double
double (*g_cov_data)[M] = nullptr; // [N][M]
double (*g_cov_cov)[M]  = nullptr; // [M][M]
double g_cov_mean[M];
static struct CovarianceInit {
    CovarianceInit() {
        g_cov_data = new double[N][M]();
        g_cov_cov  = new double[M][M]();
    }
} _cov_arrinit;
extern "C" void kernel_covariance(int m, int n) __asm__("kernel_covariance");
extern "C" void kernel_covariance(int m, int n) {
    __pb_covariance_kernel(m, n, (double)n,
        g_cov_data, g_cov_cov, g_cov_mean);
}
POLYBENCH_IMPL_2(covariance)

// ── 2mm ───────────────────────────────────────────────────────────────────────
#define POLYBENCH_KERNEL      2mm
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/linear-algebra/kernels/2mm/2mm.c"
#define kernel_2mm            POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_2mm
// After include: NI=1600, NJ=1800, NK=2200, NL=2400, DATA_TYPE=double
double (*g_2mm_tmp)[NJ] = nullptr; // [NI][NJ]
double (*g_2mm_A)[NK]   = nullptr; // [NI][NK]
double (*g_2mm_B)[NJ]   = nullptr; // [NK][NJ]
double (*g_2mm_C)[NL]   = nullptr; // [NJ][NL]
double (*g_2mm_D)[NL]   = nullptr; // [NI][NL]
static struct Mm2Init {
    Mm2Init() {
        g_2mm_tmp = new double[NI][NJ]();
        g_2mm_A   = new double[NI][NK]();
        g_2mm_B   = new double[NK][NJ]();
        g_2mm_C   = new double[NJ][NL]();
        g_2mm_D   = new double[NI][NL]();
    }
} _2mm_arrinit;
extern "C" void kernel_2mm(int ni, int nj, int nk, int nl) __asm__("kernel_2mm");
extern "C" void kernel_2mm(int ni, int nj, int nk, int nl) {
    __pb_2mm_kernel(ni, nj, nk, nl, 1.5, 1.2,
        g_2mm_tmp, g_2mm_A, g_2mm_B, g_2mm_C, g_2mm_D);
}
POLYBENCH_IMPL_4(2mm)

// ── 3mm ───────────────────────────────────────────────────────────────────────
#define POLYBENCH_KERNEL      3mm
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/linear-algebra/kernels/3mm/3mm.c"
#define kernel_3mm            POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_3mm
// After include: NI=1600, NJ=1800, NK=2000, NL=2200, NM=2400, DATA_TYPE=double
double (*g_3mm_E)[NJ] = nullptr; // [NI][NJ]
double (*g_3mm_A)[NK] = nullptr; // [NI][NK]
double (*g_3mm_B)[NJ] = nullptr; // [NK][NJ]
double (*g_3mm_F)[NL] = nullptr; // [NJ][NL]
double (*g_3mm_C)[NM] = nullptr; // [NJ][NM]
double (*g_3mm_D)[NL] = nullptr; // [NM][NL]
double (*g_3mm_G)[NL] = nullptr; // [NI][NL]
static struct Mm3Init {
    Mm3Init() {
        g_3mm_E = new double[NI][NJ]();
        g_3mm_A = new double[NI][NK]();
        g_3mm_B = new double[NK][NJ]();
        g_3mm_F = new double[NJ][NL]();
        g_3mm_C = new double[NJ][NM]();
        g_3mm_D = new double[NM][NL]();
        g_3mm_G = new double[NI][NL]();
    }
} _3mm_arrinit;
extern "C" void kernel_3mm(int ni, int nj, int nk, int nl, int nm) __asm__("kernel_3mm");
extern "C" void kernel_3mm(int ni, int nj, int nk, int nl, int nm) {
    __pb_3mm_kernel(ni, nj, nk, nl, nm,
        g_3mm_E, g_3mm_A, g_3mm_B, g_3mm_F, g_3mm_C, g_3mm_D, g_3mm_G);
}
POLYBENCH_IMPL_5(3mm)

// ── atax ──────────────────────────────────────────────────────────────────────
#define POLYBENCH_KERNEL      atax
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/linear-algebra/kernels/atax/atax.c"
#define kernel_atax           POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_atax
// After include: M=1800, N=2200, DATA_TYPE=double
// LARGE dataset uses m=1900 > M=1800; size arrays for max m across all 5 benchmark sizes.
static constexpr int kAtaxMaxM = 1900;
double (*g_atax_A)[N] = nullptr; // [kAtaxMaxM][N]
double g_atax_x[N];
double g_atax_y[N];
double g_atax_tmp[kAtaxMaxM]; // was [M=1800]; LARGE uses m=1900
static struct AtaxInit {
    AtaxInit() { g_atax_A = new double[kAtaxMaxM][N](); }
} _atax_arrinit;
extern "C" void kernel_atax(int m, int n) __asm__("kernel_atax");
extern "C" void kernel_atax(int m, int n) {
    __pb_atax_kernel(m, n, g_atax_A, g_atax_x, g_atax_y, g_atax_tmp);
}
POLYBENCH_IMPL_2(atax)

// ── bicg ──────────────────────────────────────────────────────────────────────
#define POLYBENCH_KERNEL      bicg
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/linear-algebra/kernels/bicg/bicg.c"
#define kernel_bicg           POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_bicg
// After include: M=1800, N=2200, DATA_TYPE=double
// LARGE dataset uses m=1900 > M=1800; size arrays for max m across all 5 benchmark sizes.
// g_bicg_A stride stays M=1800 to match the compiled kernel's type; allocation is N*M which
// covers all accesses for LARGE (max index 2099*1800+1899 < 2200*1800).
static constexpr int kBicgMaxM = 1900;
double (*g_bicg_A)[M] = nullptr; // [N][M]; stride=M=1800 matches compiled kernel
double g_bicg_s[kBicgMaxM];     // was [M=1800]; LARGE uses m=1900
double g_bicg_q[N];
double g_bicg_p[kBicgMaxM];     // was [M=1800]; LARGE uses m=1900
double g_bicg_r[N];
static struct BicgInit {
    BicgInit() { g_bicg_A = new double[N][M](); }
} _bicg_arrinit;
extern "C" void kernel_bicg(int m, int n) __asm__("kernel_bicg");
extern "C" void kernel_bicg(int m, int n) {
    __pb_bicg_kernel(m, n, g_bicg_A, g_bicg_s, g_bicg_q, g_bicg_p, g_bicg_r);
}
POLYBENCH_IMPL_2(bicg)

// ── doitgen ───────────────────────────────────────────────────────────────────
// Spec params: (NQ, NR, NP); kernel takes (nr, nq, np).
// kernel_doitgen is NOT static in the original source.
#define POLYBENCH_KERNEL      doitgen
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/linear-algebra/kernels/doitgen/doitgen.c"
#define kernel_doitgen        POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_doitgen
// After include: NR=250, NQ=220, NP=270, DATA_TYPE=double
double (*g_doitgen_A)[NQ][NP] = nullptr; // [NR][NQ][NP]
double (*g_doitgen_C4)[NP]    = nullptr; // [NP][NP]
double g_doitgen_sum[NP];
static struct DoitgenInit {
    DoitgenInit() {
        g_doitgen_A  = new double[NR][NQ][NP]();
        g_doitgen_C4 = new double[NP][NP]();
    }
} _doitgen_arrinit;
// Wrapper receives (nq, nr, np) matching spec order; kernel takes (nr, nq, np).
extern "C" void kernel_doitgen(int nq, int nr, int np) __asm__("kernel_doitgen");
extern "C" void kernel_doitgen(int nq, int nr, int np) {
    __pb_doitgen_kernel(nr, nq, np, g_doitgen_A, g_doitgen_C4, g_doitgen_sum);
}
POLYBENCH_IMPL_3(doitgen)

// ── mvt ───────────────────────────────────────────────────────────────────────
#define POLYBENCH_KERNEL      mvt
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/linear-algebra/kernels/mvt/mvt.c"
#define kernel_mvt            POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_mvt
// After include: N=4000, DATA_TYPE=double
double g_mvt_x1[N];
double g_mvt_x2[N];
double g_mvt_y1[N];
double g_mvt_y2[N];
double (*g_mvt_A)[N] = nullptr; // [N][N]
static struct MvtInit {
    MvtInit() { g_mvt_A = new double[N][N](); }
} _mvt_arrinit;
extern "C" void kernel_mvt(int n) __asm__("kernel_mvt");
extern "C" void kernel_mvt(int n) {
    __pb_mvt_kernel(n, g_mvt_x1, g_mvt_x2, g_mvt_y1, g_mvt_y2, g_mvt_A);
}
POLYBENCH_IMPL_1(mvt)

// ── gemm ──────────────────────────────────────────────────────────────────────
#define POLYBENCH_KERNEL      gemm
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/linear-algebra/blas/gemm/gemm.c"
#define kernel_gemm           POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_gemm
// After include: NI=2000, NJ=2300, NK=2600, DATA_TYPE=double
double (*g_gemm_C)[NJ] = nullptr; // [NI][NJ]
double (*g_gemm_A)[NK] = nullptr; // [NI][NK]
double (*g_gemm_B)[NJ] = nullptr; // [NK][NJ]
static struct GemmInit {
    GemmInit() {
        g_gemm_C = new double[NI][NJ]();
        g_gemm_A = new double[NI][NK]();
        g_gemm_B = new double[NK][NJ]();
    }
} _gemm_arrinit;
extern "C" void kernel_gemm(int ni, int nj, int nk) __asm__("kernel_gemm");
extern "C" void kernel_gemm(int ni, int nj, int nk) {
    __pb_gemm_kernel(ni, nj, nk, 1.5, 1.2, g_gemm_C, g_gemm_A, g_gemm_B);
}
POLYBENCH_IMPL_3(gemm)

// ── gemver ────────────────────────────────────────────────────────────────────
#define POLYBENCH_KERNEL      gemver
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/linear-algebra/blas/gemver/gemver.c"
#define kernel_gemver         POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_gemver
// After include: N=4000, DATA_TYPE=double
double (*g_gemver_A)[N] = nullptr; // [N][N]
double g_gemver_u1[N];
double g_gemver_v1[N];
double g_gemver_u2[N];
double g_gemver_v2[N];
double g_gemver_w[N];
double g_gemver_x[N];
double g_gemver_y[N];
double g_gemver_z[N];
static struct GemverInit {
    GemverInit() { g_gemver_A = new double[N][N](); }
} _gemver_arrinit;
extern "C" void kernel_gemver(int n) __asm__("kernel_gemver");
extern "C" void kernel_gemver(int n) {
    __pb_gemver_kernel(n, 1.5, 1.2,
        g_gemver_A, g_gemver_u1, g_gemver_v1, g_gemver_u2, g_gemver_v2,
        g_gemver_w, g_gemver_x, g_gemver_y, g_gemver_z);
}
POLYBENCH_IMPL_1(gemver)

// ── gesummv ───────────────────────────────────────────────────────────────────
#define POLYBENCH_KERNEL      gesummv
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/linear-algebra/blas/gesummv/gesummv.c"
#define kernel_gesummv        POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_gesummv
// After include: N=2800, DATA_TYPE=double
double (*g_gesummv_A)[N] = nullptr; // [N][N]
double (*g_gesummv_B)[N] = nullptr; // [N][N]
double g_gesummv_tmp[N];
double g_gesummv_x[N];
double g_gesummv_y[N];
static struct GesummvInit {
    GesummvInit() {
        g_gesummv_A = new double[N][N]();
        g_gesummv_B = new double[N][N]();
    }
} _gesummv_arrinit;
extern "C" void kernel_gesummv(int n) __asm__("kernel_gesummv");
extern "C" void kernel_gesummv(int n) {
    __pb_gesummv_kernel(n, 1.5, 1.2,
        g_gesummv_A, g_gesummv_B, g_gesummv_tmp, g_gesummv_x, g_gesummv_y);
}
POLYBENCH_IMPL_1(gesummv)

// ── symm ──────────────────────────────────────────────────────────────────────
#define POLYBENCH_KERNEL      symm
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/linear-algebra/blas/symm/symm.c"
#define kernel_symm           POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_symm
// After include: M=2000, N=2600, DATA_TYPE=double
double (*g_symm_C)[N] = nullptr; // [M][N]
double (*g_symm_A)[M] = nullptr; // [M][M]
double (*g_symm_B)[N] = nullptr; // [M][N]
static struct SymmInit {
    SymmInit() {
        g_symm_C = new double[M][N]();
        g_symm_A = new double[M][M]();
        g_symm_B = new double[M][N]();
    }
} _symm_arrinit;
extern "C" void kernel_symm(int m, int n) __asm__("kernel_symm");
extern "C" void kernel_symm(int m, int n) {
    __pb_symm_kernel(m, n, 1.5, 1.2, g_symm_C, g_symm_A, g_symm_B);
}
POLYBENCH_IMPL_2(symm)

// ── syr2k ─────────────────────────────────────────────────────────────────────
// Spec params (M, N); kernel takes (n=N, m=M).
#define POLYBENCH_KERNEL      syr2k
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/linear-algebra/blas/syr2k/syr2k.c"
#define kernel_syr2k          POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_syr2k
// After include: N=2600, M=2000, DATA_TYPE=double
double (*g_syr2k_C)[N] = nullptr; // [N][N]
double (*g_syr2k_A)[M] = nullptr; // [N][M]
double (*g_syr2k_B)[M] = nullptr; // [N][M]
static struct Syr2kInit {
    Syr2kInit() {
        g_syr2k_C = new double[N][N]();
        g_syr2k_A = new double[N][M]();
        g_syr2k_B = new double[N][M]();
    }
} _syr2k_arrinit;
// Wrapper receives (spec_M, spec_N); kernel takes (n=spec_N, m=spec_M).
extern "C" void kernel_syr2k(int spec_m, int spec_n) __asm__("kernel_syr2k");
extern "C" void kernel_syr2k(int spec_m, int spec_n) {
    __pb_syr2k_kernel(spec_n, spec_m, 1.5, 1.2,
        g_syr2k_C, g_syr2k_A, g_syr2k_B);
}
POLYBENCH_IMPL_2(syr2k)

// ── syrk ──────────────────────────────────────────────────────────────────────
// Spec params (M, N); kernel takes (n=N, m=M).
#define POLYBENCH_KERNEL      syrk
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/linear-algebra/blas/syrk/syrk.c"
#define kernel_syrk           POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_syrk
// After include: N=2600, M=2000, DATA_TYPE=double
double (*g_syrk_C)[N] = nullptr; // [N][N]
double (*g_syrk_A)[M] = nullptr; // [N][M]
static struct SyrkInit {
    SyrkInit() {
        g_syrk_C = new double[N][N]();
        g_syrk_A = new double[N][M]();
    }
} _syrk_arrinit;
// Wrapper receives (spec_M, spec_N); kernel takes (n=spec_N, m=spec_M).
extern "C" void kernel_syrk(int spec_m, int spec_n) __asm__("kernel_syrk");
extern "C" void kernel_syrk(int spec_m, int spec_n) {
    __pb_syrk_kernel(spec_n, spec_m, 1.5, 1.2, g_syrk_C, g_syrk_A);
}
POLYBENCH_IMPL_2(syrk)

// ── trmm ──────────────────────────────────────────────────────────────────────
#define POLYBENCH_KERNEL      trmm
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/linear-algebra/blas/trmm/trmm.c"
#define kernel_trmm           POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_trmm
// After include: M=2000, N=2600, DATA_TYPE=double
double (*g_trmm_A)[M] = nullptr; // [M][M]
double (*g_trmm_B)[N] = nullptr; // [M][N]
static struct TrmmInit {
    TrmmInit() {
        g_trmm_A = new double[M][M]();
        g_trmm_B = new double[M][N]();
    }
} _trmm_arrinit;
extern "C" void kernel_trmm(int m, int n) __asm__("kernel_trmm");
extern "C" void kernel_trmm(int m, int n) {
    __pb_trmm_kernel(m, n, 1.5, g_trmm_A, g_trmm_B);
}
POLYBENCH_IMPL_2(trmm)

// ── cholesky ──────────────────────────────────────────────────────────────────
#define POLYBENCH_KERNEL      cholesky
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/linear-algebra/solvers/cholesky/cholesky.c"
#define kernel_cholesky       POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_cholesky
// After include: N=4000, DATA_TYPE=double
double (*g_cholesky_A)[N] = nullptr; // [N][N]
static struct CholeskyInit {
    CholeskyInit() { g_cholesky_A = new double[N][N](); }
} _cholesky_arrinit;
extern "C" void kernel_cholesky(int n) __asm__("kernel_cholesky");
extern "C" void kernel_cholesky(int n) {
    __pb_cholesky_kernel(n, g_cholesky_A);
}
POLYBENCH_IMPL_1(cholesky)

// ── durbin ────────────────────────────────────────────────────────────────────
#define POLYBENCH_KERNEL      durbin
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/linear-algebra/solvers/durbin/durbin.c"
#define kernel_durbin         POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_durbin
// After include: N=4000, DATA_TYPE=double
double g_durbin_r[N];
double g_durbin_y[N];
extern "C" void kernel_durbin(int n) __asm__("kernel_durbin");
extern "C" void kernel_durbin(int n) {
    __pb_durbin_kernel(n, g_durbin_r, g_durbin_y);
}
POLYBENCH_IMPL_1(durbin)

// ── gramschmidt ───────────────────────────────────────────────────────────────
#define POLYBENCH_KERNEL      gramschmidt
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/linear-algebra/solvers/gramschmidt/gramschmidt.c"
#define kernel_gramschmidt    POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_gramschmidt
// After include: M=2000, N=2600, DATA_TYPE=double
double (*g_gramschmidt_A)[N] = nullptr; // [M][N]
double (*g_gramschmidt_R)[N] = nullptr; // [N][N]
double (*g_gramschmidt_Q)[N] = nullptr; // [M][N]
static struct GramschmidtInit {
    GramschmidtInit() {
        g_gramschmidt_A = new double[M][N]();
        g_gramschmidt_R = new double[N][N]();
        g_gramschmidt_Q = new double[M][N]();
    }
} _gramschmidt_arrinit;
extern "C" void kernel_gramschmidt(int m, int n) __asm__("kernel_gramschmidt");
extern "C" void kernel_gramschmidt(int m, int n) {
    __pb_gramschmidt_kernel(m, n,
        g_gramschmidt_A, g_gramschmidt_R, g_gramschmidt_Q);
}
POLYBENCH_IMPL_2(gramschmidt)

// ── lu ────────────────────────────────────────────────────────────────────────
#define POLYBENCH_KERNEL      lu
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/linear-algebra/solvers/lu/lu.c"
#define kernel_lu             POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_lu
// After include: N=4000, DATA_TYPE=double
double (*g_lu_A)[N] = nullptr; // [N][N]
static struct LuInit {
    LuInit() { g_lu_A = new double[N][N](); }
} _lu_arrinit;
extern "C" void kernel_lu(int n) __asm__("kernel_lu");
extern "C" void kernel_lu(int n) {
    __pb_lu_kernel(n, g_lu_A);
}
POLYBENCH_IMPL_1(lu)

// ── ludcmp ────────────────────────────────────────────────────────────────────
#define POLYBENCH_KERNEL      ludcmp
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/linear-algebra/solvers/ludcmp/ludcmp.c"
#define kernel_ludcmp         POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_ludcmp
// After include: N=4000, DATA_TYPE=double
double (*g_ludcmp_A)[N] = nullptr; // [N][N]
double g_ludcmp_b[N];
double g_ludcmp_x[N];
double g_ludcmp_y[N];
static struct LudcmpInit {
    LudcmpInit() { g_ludcmp_A = new double[N][N](); }
} _ludcmp_arrinit;
extern "C" void kernel_ludcmp(int n) __asm__("kernel_ludcmp");
extern "C" void kernel_ludcmp(int n) {
    __pb_ludcmp_kernel(n, g_ludcmp_A, g_ludcmp_b, g_ludcmp_x, g_ludcmp_y);
}
POLYBENCH_IMPL_1(ludcmp)

// ── trisolv ───────────────────────────────────────────────────────────────────
#define POLYBENCH_KERNEL      trisolv
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/linear-algebra/solvers/trisolv/trisolv.c"
#define kernel_trisolv        POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_trisolv
// After include: N=4000, DATA_TYPE=double
double (*g_trisolv_L)[N] = nullptr; // [N][N]
double g_trisolv_x[N];
double g_trisolv_b[N];
static struct TrisolveInit {
    TrisolveInit() { g_trisolv_L = new double[N][N](); }
} _trisolv_arrinit;
extern "C" void kernel_trisolv(int n) __asm__("kernel_trisolv");
extern "C" void kernel_trisolv(int n) {
    __pb_trisolv_kernel(n, g_trisolv_L, g_trisolv_x, g_trisolv_b);
}
POLYBENCH_IMPL_1(trisolv)

// ── deriche ───────────────────────────────────────────────────────────────────
// Uses float (DATA_TYPE_IS_FLOAT per deriche.h default).
#define POLYBENCH_KERNEL      deriche
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/medley/deriche/deriche.c"
#define kernel_deriche        POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_deriche
// After include: W=7680, H=4320, DATA_TYPE=float
float (*g_deriche_imgIn)[H]  = nullptr; // [W][H]
float (*g_deriche_imgOut)[H] = nullptr; // [W][H]
float (*g_deriche_y1)[H]     = nullptr; // [W][H]
float (*g_deriche_y2)[H]     = nullptr; // [W][H]
static struct DericheInit {
    DericheInit() {
        g_deriche_imgIn  = new float[W][H]();
        g_deriche_imgOut = new float[W][H]();
        g_deriche_y1     = new float[W][H]();
        g_deriche_y2     = new float[W][H]();
    }
} _deriche_arrinit;
extern "C" void kernel_deriche(int w, int h) __asm__("kernel_deriche");
extern "C" void kernel_deriche(int w, int h) {
    __pb_deriche_kernel(w, h, 0.25f,
        g_deriche_imgIn, g_deriche_imgOut, g_deriche_y1, g_deriche_y2);
}
POLYBENCH_IMPL_2(deriche)

// ── floyd-warshall ────────────────────────────────────────────────────────────
// Uses int (DATA_TYPE_IS_INT per floyd-warshall.h default).
// Note: fw is used as the abbreviated kernel token to avoid the hyphen in "floyd-warshall".
#define POLYBENCH_KERNEL      fw
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/medley/floyd-warshall/floyd-warshall.c"
#define kernel_floyd_warshall POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_floyd_warshall
// After include: N=5600, DATA_TYPE=int
int (*g_fw_path)[N] = nullptr; // [N][N]
static struct FwInit {
    FwInit() { g_fw_path = new int[N][N](); }
} _fw_arrinit;
extern "C" void kernel_floyd_warshall(int n) __asm__("kernel_floyd_warshall");
extern "C" void kernel_floyd_warshall(int n) {
    __pb_fw_kernel(n, g_fw_path);
}
POLYBENCH_IMPL_1(floyd_warshall)

// ── nussinov ──────────────────────────────────────────────────────────────────
// Uses int (DATA_TYPE_IS_INT per nussinov.h default).
// nussinov.c defines: typedef char base; #define match() #define max_score()
#define POLYBENCH_KERNEL      nussinov
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/medley/nussinov/nussinov.c"
#define kernel_nussinov       POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_nussinov
#undef match
#undef max_score
// After include: N=5500, DATA_TYPE=int; typedef char base is visible.
static char g_nussinov_seq[N];
int (*g_nussinov_table)[N] = nullptr; // [N][N]
static struct NussinovInit {
    NussinovInit() { g_nussinov_table = new int[N][N](); }
} _nussinov_arrinit;
extern "C" void kernel_nussinov(int n) __asm__("kernel_nussinov");
extern "C" void kernel_nussinov(int n) {
    __pb_nussinov_kernel(n, g_nussinov_seq, g_nussinov_table);
}
POLYBENCH_IMPL_1(nussinov)

// ── adi ───────────────────────────────────────────────────────────────────────
#define POLYBENCH_KERNEL      adi
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/stencils/adi/adi.c"
#define kernel_adi            POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_adi
// After include: TSTEPS=1000, N=2000, DATA_TYPE=double
double (*g_adi_u)[N] = nullptr; // [N][N]
double (*g_adi_v)[N] = nullptr; // [N][N]
double (*g_adi_p)[N] = nullptr; // [N][N]
double (*g_adi_q)[N] = nullptr; // [N][N]
static struct AdiInit {
    AdiInit() {
        g_adi_u = new double[N][N]();
        g_adi_v = new double[N][N]();
        g_adi_p = new double[N][N]();
        g_adi_q = new double[N][N]();
    }
} _adi_arrinit;
extern "C" void kernel_adi(int tsteps, int n) __asm__("kernel_adi");
extern "C" void kernel_adi(int tsteps, int n) {
    __pb_adi_kernel(tsteps, n, g_adi_u, g_adi_v, g_adi_p, g_adi_q);
}
POLYBENCH_IMPL_2(adi)

// ── fdtd-2d ───────────────────────────────────────────────────────────────────
// Note: fdtd2d is used as abbreviated token; kernel fn in C source is kernel_fdtd_2d.
#define POLYBENCH_KERNEL      fdtd2d
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/stencils/fdtd-2d/fdtd-2d.c"
#define kernel_fdtd_2d        POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_fdtd_2d
// After include: TMAX=1000, NX=2000, NY=2600, DATA_TYPE=double
double (*g_fdtd2d_ex)[NY] = nullptr; // [NX][NY]
double (*g_fdtd2d_ey)[NY] = nullptr; // [NX][NY]
double (*g_fdtd2d_hz)[NY] = nullptr; // [NX][NY]
double g_fdtd2d_fict[TMAX];
static struct Fdtd2dInit {
    Fdtd2dInit() {
        g_fdtd2d_ex = new double[NX][NY]();
        g_fdtd2d_ey = new double[NX][NY]();
        g_fdtd2d_hz = new double[NX][NY]();
    }
} _fdtd2d_arrinit;
extern "C" void kernel_fdtd_2d(int tmax, int nx, int ny) __asm__("kernel_fdtd_2d");
extern "C" void kernel_fdtd_2d(int tmax, int nx, int ny) {
    __pb_fdtd2d_kernel(tmax, nx, ny,
        g_fdtd2d_ex, g_fdtd2d_ey, g_fdtd2d_hz, g_fdtd2d_fict);
}
POLYBENCH_IMPL_3(fdtd_2d)

// ── heat-3d ───────────────────────────────────────────────────────────────────
// Note: heat3d is used as abbreviated token; kernel fn in C source is kernel_heat_3d.
#define POLYBENCH_KERNEL      heat3d
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/stencils/heat-3d/heat-3d.c"
#define kernel_heat_3d        POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_heat_3d
// After include: TSTEPS=1000, N=200, DATA_TYPE=double
double (*g_heat3d_A)[N][N] = nullptr; // [N][N][N]
double (*g_heat3d_B)[N][N] = nullptr; // [N][N][N]
static struct Heat3dInit {
    Heat3dInit() {
        g_heat3d_A = new double[N][N][N]();
        g_heat3d_B = new double[N][N][N]();
    }
} _heat3d_arrinit;
extern "C" void kernel_heat_3d(int tsteps, int n) __asm__("kernel_heat_3d");
extern "C" void kernel_heat_3d(int tsteps, int n) {
    __pb_heat3d_kernel(tsteps, n, g_heat3d_A, g_heat3d_B);
}
POLYBENCH_IMPL_2(heat_3d)

// ── jacobi-1d ─────────────────────────────────────────────────────────────────
// Note: jacobi1d is used as abbreviated token; kernel fn in C source is kernel_jacobi_1d.
#define POLYBENCH_KERNEL      jacobi1d
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/stencils/jacobi-1d/jacobi-1d.c"
#define kernel_jacobi_1d      POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_jacobi_1d
// After include: TSTEPS=1000, N=4000, DATA_TYPE=double
double g_jacobi1d_A[N];
double g_jacobi1d_B[N];
extern "C" void kernel_jacobi_1d(int tsteps, int n) __asm__("kernel_jacobi_1d");
extern "C" void kernel_jacobi_1d(int tsteps, int n) {
    __pb_jacobi1d_kernel(tsteps, n, g_jacobi1d_A, g_jacobi1d_B);
}
POLYBENCH_IMPL_2(jacobi_1d)

// ── jacobi-2d ─────────────────────────────────────────────────────────────────
// Note: jacobi2d is used as abbreviated token; kernel fn in C source is kernel_jacobi_2d.
#define POLYBENCH_KERNEL      jacobi2d
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/stencils/jacobi-2d/jacobi-2d.c"
#define kernel_jacobi_2d      POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_jacobi_2d
// After include: TSTEPS=1000, N=2800, DATA_TYPE=double
double (*g_jacobi2d_A)[N] = nullptr; // [N][N]
double (*g_jacobi2d_B)[N] = nullptr; // [N][N]
static struct Jacobi2dInit {
    Jacobi2dInit() {
        g_jacobi2d_A = new double[N][N]();
        g_jacobi2d_B = new double[N][N]();
    }
} _jacobi2d_arrinit;
extern "C" void kernel_jacobi_2d(int tsteps, int n) __asm__("kernel_jacobi_2d");
extern "C" void kernel_jacobi_2d(int tsteps, int n) {
    __pb_jacobi2d_kernel(tsteps, n, g_jacobi2d_A, g_jacobi2d_B);
}
POLYBENCH_IMPL_2(jacobi_2d)

// ── seidel-2d ─────────────────────────────────────────────────────────────────
// Note: seidel2d is used as abbreviated token; kernel fn in C source is kernel_seidel_2d.
#define POLYBENCH_KERNEL      seidel2d
#define POLYBENCH_KERNEL_PATH "PolyBenchC-4.2.1-master/stencils/seidel-2d/seidel-2d.c"
#define kernel_seidel_2d      POLYBENCH_KERNEL_FN_IMPL
#include "polybench_import_kernel.h"
#undef kernel_seidel_2d
// After include: TSTEPS=1000, N=4000, DATA_TYPE=double
double (*g_seidel2d_A)[N] = nullptr; // [N][N]
static struct Seidel2dInit {
    Seidel2dInit() { g_seidel2d_A = new double[N][N](); }
} _seidel2d_arrinit;
extern "C" void kernel_seidel_2d(int tsteps, int n) __asm__("kernel_seidel_2d");
extern "C" void kernel_seidel_2d(int tsteps, int n) {
    __pb_seidel2d_kernel(tsteps, n, g_seidel2d_A);
}
POLYBENCH_IMPL_2(seidel_2d)




// ── Benchmark Registrations ───────────────────────────────────────────────────
#define POLYBENCH_BENCHMARK_SPEC(K, MINI, SMALL, MEDIUM, LARGE, EXTRALARGE, K_SETUP, K_TEARDOWN) \
BENCHMARK(BM_unspecialized____##K)->Name("BM_g:polybench;n:" #K ";s:MINI;t:unspecialized;")->MINI->Setup(K_SETUP)->Teardown(K_TEARDOWN); \
BENCHMARK(BM_unspecialized____##K)->Name("BM_g:polybench;n:" #K ";s:SMALL;t:unspecialized;")->SMALL->Setup(K_SETUP)->Teardown(K_TEARDOWN); \
BENCHMARK(BM_unspecialized____##K)->Name("BM_g:polybench;n:" #K ";s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(K_SETUP)->Teardown(K_TEARDOWN); \
BENCHMARK(BM_unspecialized____##K)->Name("BM_g:polybench;n:" #K ";s:LARGE;t:unspecialized;")->LARGE->Setup(K_SETUP)->Teardown(K_TEARDOWN); \
BENCHMARK(BM_unspecialized____##K)->Name("BM_g:polybench;n:" #K ";s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(K_SETUP)->Teardown(K_TEARDOWN); \
BENCHMARK(BM_jit_overhead_____##K)->Name("BM_g:polybench;n:" #K ";s:MINI;t:jit_overhead;")->MINI->Setup(K_SETUP)->Teardown(K_TEARDOWN); \
BENCHMARK(BM_jit_overhead_____##K)->Name("BM_g:polybench;n:" #K ";s:SMALL;t:jit_overhead;")->SMALL->Setup(K_SETUP)->Teardown(K_TEARDOWN); \
BENCHMARK(BM_jit_overhead_____##K)->Name("BM_g:polybench;n:" #K ";s:MEDIUM;t:jit_overhead;")->MEDIUM->Setup(K_SETUP)->Teardown(K_TEARDOWN); \
BENCHMARK(BM_jit_overhead_____##K)->Name("BM_g:polybench;n:" #K ";s:LARGE;t:jit_overhead;")->LARGE->Setup(K_SETUP)->Teardown(K_TEARDOWN); \
BENCHMARK(BM_jit_overhead_____##K)->Name("BM_g:polybench;n:" #K ";s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Setup(K_SETUP)->Teardown(K_TEARDOWN); \
BENCHMARK(BM_specialized_exec_##K)->Name("BM_g:polybench;n:" #K ";s:MINI;t:specialized_exec;")->MINI->Setup(K_SETUP)->Teardown(K_TEARDOWN); \
BENCHMARK(BM_specialized_exec_##K)->Name("BM_g:polybench;n:" #K ";s:SMALL;t:specialized_exec;")->SMALL->Setup(K_SETUP)->Teardown(K_TEARDOWN); \
BENCHMARK(BM_specialized_exec_##K)->Name("BM_g:polybench;n:" #K ";s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(K_SETUP)->Teardown(K_TEARDOWN); \
BENCHMARK(BM_specialized_exec_##K)->Name("BM_g:polybench;n:" #K ";s:LARGE;t:specialized_exec;")->LARGE->Setup(K_SETUP)->Teardown(K_TEARDOWN); \
BENCHMARK(BM_specialized_exec_##K)->Name("BM_g:polybench;n:" #K ";s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(K_SETUP)->Teardown(K_TEARDOWN); \
BENCHMARK(BM_jit_analysis____##K)->Name("BM_g:polybench;n:" #K ";s:MINI;t:jit_analysis;")->MINI->Iterations(1)->UseManualTime()->Setup(K_SETUP)->Teardown(K_TEARDOWN); \
BENCHMARK(BM_jit_analysis____##K)->Name("BM_g:polybench;n:" #K ";s:SMALL;t:jit_analysis;")->SMALL->Iterations(1)->UseManualTime()->Setup(K_SETUP)->Teardown(K_TEARDOWN); \
BENCHMARK(BM_jit_analysis____##K)->Name("BM_g:polybench;n:" #K ";s:MEDIUM;t:jit_analysis;")->MEDIUM->Iterations(1)->UseManualTime()->Setup(K_SETUP)->Teardown(K_TEARDOWN); \
BENCHMARK(BM_jit_analysis____##K)->Name("BM_g:polybench;n:" #K ";s:LARGE;t:jit_analysis;")->LARGE->Iterations(1)->UseManualTime()->Setup(K_SETUP)->Teardown(K_TEARDOWN); \
BENCHMARK(BM_jit_analysis____##K)->Name("BM_g:polybench;n:" #K ";s:EXTRALARGE;t:jit_analysis;")->EXTRALARGE->Iterations(1)->UseManualTime()->Setup(K_SETUP)->Teardown(K_TEARDOWN);


POLYBENCH_BENCHMARK_SPEC(correlation,Args({28, 32}),Args({80, 100}),Args({240, 260}),Args({1200, 1400}),Args({2600, 3000}),pb_setup_correlation,pb_teardown_correlation)
POLYBENCH_BENCHMARK_SPEC(covariance,Args({28, 32}),Args({80, 100}),Args({240, 260}),Args({1200, 1400}),Args({2600, 3000}),pb_setup_covariance,pb_teardown_covariance)
POLYBENCH_BENCHMARK_SPEC(2mm,Args({16, 18, 22, 24}),Args({40, 50, 70, 80}),Args({180, 190, 210, 220}),Args({800, 900, 1100, 1200}),Args({1600, 1800, 2200, 2400}),pb_setup_mm2,pb_teardown_mm2)
POLYBENCH_BENCHMARK_SPEC(3mm,Args({16, 18, 20, 22, 24}),Args({40, 50, 60, 70, 80}),Args({180, 190, 200, 210, 220}),Args({800, 900, 1000, 1100, 1200}),Args({1600, 1800, 2000, 2200, 2400}),pb_setup_mm3,pb_teardown_mm3)
POLYBENCH_BENCHMARK_SPEC(atax,Args({38, 42}),Args({116, 124}),Args({390, 410}),Args({1900, 2100}),Args({1800, 2200}),pb_setup_atax,pb_teardown_atax)
POLYBENCH_BENCHMARK_SPEC(bicg,Args({38, 42}),Args({116, 124}),Args({390, 410}),Args({1900, 2100}),Args({1800, 2200}),pb_setup_bicg,pb_teardown_bicg)
POLYBENCH_BENCHMARK_SPEC(doitgen,Args({8, 10, 12}),Args({20, 25, 30}),Args({40, 50, 60}),Args({140, 150, 160}),Args({220, 250, 270}),pb_setup_doitgen,pb_teardown_doitgen)
POLYBENCH_BENCHMARK_SPEC(mvt,Arg(40),Arg(120),Arg(400),Arg(2000),Arg(4000),pb_setup_mvt,pb_teardown_mvt)
POLYBENCH_BENCHMARK_SPEC(gemm,Args({20, 25, 30}),Args({60, 70, 80}),Args({200, 220, 240}),Args({1000, 1100, 1200}),Args({2000, 2300, 2600}),pb_setup_gemm,pb_teardown_gemm)
POLYBENCH_BENCHMARK_SPEC(gemver,Arg(40),Arg(120),Arg(400),Arg(2000),Arg(4000),pb_setup_gemver,pb_teardown_gemver)
POLYBENCH_BENCHMARK_SPEC(gesummv,Arg(30),Arg(90),Arg(250),Arg(1300),Arg(2800),pb_setup_gesummv,pb_teardown_gesummv)
POLYBENCH_BENCHMARK_SPEC(symm,Args({20, 30}),Args({60, 80}),Args({200, 240}),Args({1000, 1200}),Args({2000, 2600}),pb_setup_symm,pb_teardown_symm)
POLYBENCH_BENCHMARK_SPEC(syr2k,Args({20, 30}),Args({60, 80}),Args({200, 240}),Args({1000, 1200}),Args({2000, 2600}),pb_setup_syr2k,pb_teardown_syr2k)
POLYBENCH_BENCHMARK_SPEC(syrk,Args({20, 30}),Args({60, 80}),Args({200, 240}),Args({1000, 1200}),Args({2000, 2600}),pb_setup_syrk,pb_teardown_syrk)
POLYBENCH_BENCHMARK_SPEC(trmm,Args({20, 30}),Args({60, 80}),Args({200, 240}),Args({1000, 1200}),Args({2000, 2600}),pb_setup_trmm,pb_teardown_trmm)
POLYBENCH_BENCHMARK_SPEC(cholesky,Arg(40),Arg(120),Arg(400),Arg(2000),Arg(4000),pb_setup_cholesky,pb_teardown_cholesky)
POLYBENCH_BENCHMARK_SPEC(durbin,Arg(40),Arg(120),Arg(400),Arg(2000),Arg(4000),pb_setup_durbin,pb_teardown_durbin)
POLYBENCH_BENCHMARK_SPEC(gramschmidt,Args({20, 30}),Args({60, 80}),Args({200, 240}),Args({1000, 1200}),Args({2000, 2600}),pb_setup_gramschmidt,pb_teardown_gramschmidt)
POLYBENCH_BENCHMARK_SPEC(lu,Arg(40),Arg(120),Arg(400),Arg(2000),Arg(4000),pb_setup_lu,pb_teardown_lu)
POLYBENCH_BENCHMARK_SPEC(ludcmp,Arg(40),Arg(120),Arg(400),Arg(2000),Arg(4000),pb_setup_ludcmp,pb_teardown_ludcmp)
POLYBENCH_BENCHMARK_SPEC(trisolv,Arg(40),Arg(120),Arg(400),Arg(2000),Arg(4000),pb_setup_trisolv,pb_teardown_trisolv)
POLYBENCH_BENCHMARK_SPEC(deriche,Args({64, 64}),Args({192, 128}),Args({720, 480}),Args({4096, 2160}),Args({7680, 4320}),pb_setup_deriche,pb_teardown_deriche)
POLYBENCH_BENCHMARK_SPEC(floyd_warshall,Arg(60),Arg(180),Arg(500),Arg(2800),Arg(5600),pb_setup_fw,pb_teardown_fw)
POLYBENCH_BENCHMARK_SPEC(nussinov,Arg(60),Arg(180),Arg(500),Arg(2500),Arg(5500),pb_setup_nussinov,pb_teardown_nussinov)
POLYBENCH_BENCHMARK_SPEC(adi,Args({20, 20}),Args({40, 60}),Args({100, 200}),Args({500, 1000}),Args({1000, 2000}),pb_setup_adi,pb_teardown_adi)
POLYBENCH_BENCHMARK_SPEC(fdtd_2d,Args({20, 20, 30}),Args({40, 60, 80}),Args({100, 200, 240}),Args({500, 1000, 1200}),Args({1000, 2000, 2600}),pb_setup_fdtd2d,pb_teardown_fdtd2d)
POLYBENCH_BENCHMARK_SPEC(heat_3d,Args({20, 10}),Args({40, 20}),Args({100, 40}),Args({500, 120}),Args({1000, 200}),pb_setup_heat3d,pb_teardown_heat3d)
POLYBENCH_BENCHMARK_SPEC(jacobi_1d,Args({20, 30}),Args({40, 120}),Args({100, 400}),Args({500, 2000}),Args({1000, 4000}),pb_setup_jacobi1d,pb_teardown_jacobi1d)
POLYBENCH_BENCHMARK_SPEC(jacobi_2d,Args({20, 30}),Args({40, 90}),Args({100, 250}),Args({500, 1300}),Args({1000, 2800}),pb_setup_jacobi2d,pb_teardown_jacobi2d)
POLYBENCH_BENCHMARK_SPEC(seidel_2d,Args({20, 40}),Args({40, 120}),Args({100, 400}),Args({500, 2000}),Args({1000, 4000}),pb_setup_seidel2d,pb_teardown_seidel2d)

// ── IR Embedding Trigger ──────────────────────────────────────────────────────
// Dead code that forces the IR-dumping pass to embed all 30 kernel functions.

volatile bool g_polybench_dummy_trigger = false;

extern "C" __attribute__((used)) void polybench_dummy_registration() {
    auto* RS = CRS::ClangRuntimeSpecializer::init();
    if (g_polybench_dummy_trigger) {
        RS->callSpecialized<void>(kernel_correlation, 0, 0);
        RS->callSpecialized<void>(kernel_covariance, 0, 0);
        RS->callSpecialized<void>(kernel_2mm, 0, 0, 0, 0);
        RS->callSpecialized<void>(kernel_3mm, 0, 0, 0, 0, 0);
        RS->callSpecialized<void>(kernel_atax, 0, 0);
        RS->callSpecialized<void>(kernel_bicg, 0, 0);
        RS->callSpecialized<void>(kernel_doitgen, 0, 0, 0);
        RS->callSpecialized<void>(kernel_mvt, 0);
        RS->callSpecialized<void>(kernel_gemm, 0, 0, 0);
        RS->callSpecialized<void>(kernel_gemver, 0);
        RS->callSpecialized<void>(kernel_gesummv, 0);
        RS->callSpecialized<void>(kernel_symm, 0, 0);
        RS->callSpecialized<void>(kernel_syr2k, 0, 0);
        RS->callSpecialized<void>(kernel_syrk, 0, 0);
        RS->callSpecialized<void>(kernel_trmm, 0, 0);
        RS->callSpecialized<void>(kernel_cholesky, 0);
        RS->callSpecialized<void>(kernel_durbin, 0);
        RS->callSpecialized<void>(kernel_gramschmidt, 0, 0);
        RS->callSpecialized<void>(kernel_lu, 0);
        RS->callSpecialized<void>(kernel_ludcmp, 0);
        RS->callSpecialized<void>(kernel_trisolv, 0);
        RS->callSpecialized<void>(kernel_deriche, 0, 0);
        RS->callSpecialized<void>(kernel_floyd_warshall, 0);
        RS->callSpecialized<void>(kernel_nussinov, 0);
        RS->callSpecialized<void>(kernel_adi, 0, 0);
        RS->callSpecialized<void>(kernel_fdtd_2d, 0, 0, 0);
        RS->callSpecialized<void>(kernel_heat_3d, 0, 0);
        RS->callSpecialized<void>(kernel_jacobi_1d, 0, 0);
        RS->callSpecialized<void>(kernel_jacobi_2d, 0, 0);
        RS->callSpecialized<void>(kernel_seidel_2d, 0, 0);
    }
}

#ifndef ALL_BENCHMARKS_BUILD

int main(int argc, char** argv) {
    char arg0_default[] = "benchmark";
    char* args_default = arg0_default;
    if (!argv) {
        argc = 1;
        argv = &args_default;
    }
    static RSSMemoryManager g_rss_mgr;
    ::benchmark::RegisterMemoryManager(&g_rss_mgr);
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;

    // Exclude EXTRALARGE benchmarks by default (too slow for routine runs).
    // "_LARGE" is not a substring of "_EXTRALARGE", so this correctly matches
    // only MINI/SMALL/MEDIUM/LARGE
    if (::benchmark::GetBenchmarkFilter() == "")
    {
        ::benchmark::SetBenchmarkFilter("s:MINI|s:SMALL|s:MEDIUM|s:LARGE");
        std::cout << "Using default benchmark filter: \"" << ::benchmark::GetBenchmarkFilter() <<"\""<< std::endl;
    }

    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();
    return 0;
}
int main(int, char**);
#endif