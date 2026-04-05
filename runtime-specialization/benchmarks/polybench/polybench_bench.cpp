#include <benchmark/benchmark.h>
#include "ClangRuntimeSpecializerBenchmark.h"
#include <cstring>

namespace CRS = clangRuntimeSpecializer;

// ── Macros ────────────────────────────────────────────────────────────────────

#define POLYBENCH_IMPL_1(K) \
  static void BM_unspecialized_##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0)); \
    CRS::benchmarkUnspecialized<Fn_##K>(S, kernel_##K, A); } \
  static void BM_jit_overhead_##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0)); \
    CRS::benchmarkJITOverhead<Fn_##K>(S, kernel_##K, A, A); } \
  static void BM_specialized_exec_##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0)); \
    CRS::benchmarkSpecializedExec<Fn_##K>(S, kernel_##K, A); }

#define POLYBENCH_IMPL_2(K) \
  static void BM_unspecialized_##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1)); \
    CRS::benchmarkUnspecialized<Fn_##K>(S, kernel_##K, A); } \
  static void BM_jit_overhead_##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1)); \
    CRS::benchmarkJITOverhead<Fn_##K>(S, kernel_##K, A, A); } \
  static void BM_specialized_exec_##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1)); \
    CRS::benchmarkSpecializedExec<Fn_##K>(S, kernel_##K, A); }

#define POLYBENCH_IMPL_3(K) \
  static void BM_unspecialized_##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1), (int)S.range(2)); \
    CRS::benchmarkUnspecialized<Fn_##K>(S, kernel_##K, A); } \
  static void BM_jit_overhead_##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1), (int)S.range(2)); \
    CRS::benchmarkJITOverhead<Fn_##K>(S, kernel_##K, A, A); } \
  static void BM_specialized_exec_##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1), (int)S.range(2)); \
    CRS::benchmarkSpecializedExec<Fn_##K>(S, kernel_##K, A); }

#define POLYBENCH_IMPL_4(K) \
  static void BM_unspecialized_##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1), (int)S.range(2), (int)S.range(3)); \
    CRS::benchmarkUnspecialized<Fn_##K>(S, kernel_##K, A); } \
  static void BM_jit_overhead_##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1), (int)S.range(2), (int)S.range(3)); \
    CRS::benchmarkJITOverhead<Fn_##K>(S, kernel_##K, A, A); } \
  static void BM_specialized_exec_##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1), (int)S.range(2), (int)S.range(3)); \
    CRS::benchmarkSpecializedExec<Fn_##K>(S, kernel_##K, A); }

#define POLYBENCH_IMPL_5(K) \
  static void BM_unspecialized_##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1), (int)S.range(2), (int)S.range(3), (int)S.range(4)); \
    CRS::benchmarkUnspecialized<Fn_##K>(S, kernel_##K, A); } \
  static void BM_jit_overhead_##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1), (int)S.range(2), (int)S.range(3), (int)S.range(4)); \
    CRS::benchmarkJITOverhead<Fn_##K>(S, kernel_##K, A, A); } \
  static void BM_specialized_exec_##K(benchmark::State& S) { \
    auto A = std::make_tuple((int)S.range(0), (int)S.range(1), (int)S.range(2), (int)S.range(3), (int)S.range(4)); \
    CRS::benchmarkSpecializedExec<Fn_##K>(S, kernel_##K, A); }

#define POLYBENCH_BENCHMARK(K, ...) \
    BENCHMARK(BM_unspecialized_##K)__VA_ARGS__; \
    BENCHMARK(BM_jit_overhead_##K)__VA_ARGS__; \
    BENCHMARK(BM_specialized_exec_##K)__VA_ARGS__


// ── Kernel Stubs ──────────────────────────────────────────────────────────────

// ── correlation ───────────────────────────────────────────────────────────────
// TODO: declare global arrays for correlation (EXTRALARGE: M=2600, N=3000)

extern "C" void kernel_correlation(int M, int N) __asm__("kernel_correlation");
extern "C" void kernel_correlation(int M, int N) {
    (void)M; (void)N;
    // TODO: implement correlation
}
inline constexpr char Fn_correlation[] = "kernel_correlation";
POLYBENCH_IMPL_2(correlation)

// ── covariance ────────────────────────────────────────────────────────────────
// TODO: declare global arrays for covariance (EXTRALARGE: M=2600, N=3000)

extern "C" void kernel_covariance(int M, int N) __asm__("kernel_covariance");
extern "C" void kernel_covariance(int M, int N) {
    (void)M; (void)N;
    // TODO: implement covariance
}
inline constexpr char Fn_covariance[] = "kernel_covariance";
POLYBENCH_IMPL_2(covariance)

// ── 2mm ───────────────────────────────────────────────────────────────────────
// TODO: declare global arrays for 2mm (EXTRALARGE: NI=1600, NJ=1800, NK=2200, NL=2400)

extern "C" void kernel_2mm(int NI, int NJ, int NK, int NL) __asm__("kernel_2mm");
extern "C" void kernel_2mm(int NI, int NJ, int NK, int NL) {
    (void)NI; (void)NJ; (void)NK; (void)NL;
    // TODO: implement 2mm
}
inline constexpr char Fn_2mm[] = "kernel_2mm";
POLYBENCH_IMPL_4(2mm)

// ── 3mm ───────────────────────────────────────────────────────────────────────
// TODO: declare global arrays for 3mm (EXTRALARGE: NI=1600, NJ=1800, NK=2000, NL=2200, NM=2400)

extern "C" void kernel_3mm(int NI, int NJ, int NK, int NL, int NM) __asm__("kernel_3mm");
extern "C" void kernel_3mm(int NI, int NJ, int NK, int NL, int NM) {
    (void)NI; (void)NJ; (void)NK; (void)NL; (void)NM;
    // TODO: implement 3mm
}
inline constexpr char Fn_3mm[] = "kernel_3mm";
POLYBENCH_IMPL_5(3mm)

// ── atax ──────────────────────────────────────────────────────────────────────
// TODO: declare global arrays for atax (EXTRALARGE: M=1800, N=2200)

extern "C" void kernel_atax(int M, int N) __asm__("kernel_atax");
extern "C" void kernel_atax(int M, int N) {
    (void)M; (void)N;
    // TODO: implement atax
}
inline constexpr char Fn_atax[] = "kernel_atax";
POLYBENCH_IMPL_2(atax)

// ── bicg ──────────────────────────────────────────────────────────────────────
// TODO: declare global arrays for bicg (EXTRALARGE: M=1800, N=2200)

extern "C" void kernel_bicg(int M, int N) __asm__("kernel_bicg");
extern "C" void kernel_bicg(int M, int N) {
    (void)M; (void)N;
    // TODO: implement bicg
}
inline constexpr char Fn_bicg[] = "kernel_bicg";
POLYBENCH_IMPL_2(bicg)

// ── doitgen ───────────────────────────────────────────────────────────────────
// TODO: declare global arrays for doitgen (EXTRALARGE: NQ=220, NR=250, NP=270)

extern "C" void kernel_doitgen(int NQ, int NR, int NP) __asm__("kernel_doitgen");
extern "C" void kernel_doitgen(int NQ, int NR, int NP) {
    (void)NQ; (void)NR; (void)NP;
    // TODO: implement doitgen
}
inline constexpr char Fn_doitgen[] = "kernel_doitgen";
POLYBENCH_IMPL_3(doitgen)

// ── mvt ───────────────────────────────────────────────────────────────────────
// TODO: declare global arrays for mvt (EXTRALARGE: N=4000)

extern "C" void kernel_mvt(int N) __asm__("kernel_mvt");
extern "C" void kernel_mvt(int N) {
    (void)N;
    // TODO: implement mvt
}
inline constexpr char Fn_mvt[] = "kernel_mvt";
POLYBENCH_IMPL_1(mvt)

// ── gemm ──────────────────────────────────────────────────────────────────────
// Kernel: C = alpha*A*B + beta*C (triple-nested loop)
// Specialising on (NI, NJ, NK) lets the JIT replace all loop bounds with
// constants, enabling aggressive unrolling and vectorisation.
// alpha=1.5, beta=0.0 hardcoded; beta=0 also exposes dead-store elimination.

static constexpr int MAX_GEMM_NI = 2000, MAX_GEMM_NJ = 2300, MAX_GEMM_NK = 2600;
static double g_gemm_A[MAX_GEMM_NI][MAX_GEMM_NK];
static double g_gemm_B[MAX_GEMM_NK][MAX_GEMM_NJ];
static double g_gemm_C[MAX_GEMM_NI][MAX_GEMM_NJ];

static const struct GemmInit {
    GemmInit() {
        for (int i = 0; i < MAX_GEMM_NI; i++)
            for (int k = 0; k < MAX_GEMM_NK; k++)
                g_gemm_A[i][k] = (double)(i * MAX_GEMM_NK + k) /
                                  (MAX_GEMM_NI * MAX_GEMM_NK);
        for (int k = 0; k < MAX_GEMM_NK; k++)
            for (int j = 0; j < MAX_GEMM_NJ; j++)
                g_gemm_B[k][j] = (double)(k * MAX_GEMM_NJ + j) /
                                  (MAX_GEMM_NK * MAX_GEMM_NJ);
    }
} g_gemm_init;

extern "C" void kernel_gemm(int NI, int NJ, int NK) __asm__("kernel_gemm");
extern "C" void kernel_gemm(int NI, int NJ, int NK) {
    static constexpr double alpha = 1.5, beta = 0.0;
    for (int i = 0; i < NI; i++)
        for (int j = 0; j < NJ; j++) {
            g_gemm_C[i][j] *= beta;
            for (int k = 0; k < NK; k++)
                g_gemm_C[i][j] += alpha * g_gemm_A[i][k] * g_gemm_B[k][j];
        }
}
inline constexpr char Fn_gemm[] = "kernel_gemm";
POLYBENCH_IMPL_3(gemm)

// ── gemver ────────────────────────────────────────────────────────────────────
// TODO: declare global arrays for gemver (EXTRALARGE: N=4000)

extern "C" void kernel_gemver(int N) __asm__("kernel_gemver");
extern "C" void kernel_gemver(int N) {
    (void)N;
    // TODO: implement gemver
}
inline constexpr char Fn_gemver[] = "kernel_gemver";
POLYBENCH_IMPL_1(gemver)

// ── gesummv ───────────────────────────────────────────────────────────────────
// TODO: declare global arrays for gesummv (EXTRALARGE: N=2800)

extern "C" void kernel_gesummv(int N) __asm__("kernel_gesummv");
extern "C" void kernel_gesummv(int N) {
    (void)N;
    // TODO: implement gesummv
}
inline constexpr char Fn_gesummv[] = "kernel_gesummv";
POLYBENCH_IMPL_1(gesummv)

// ── symm ──────────────────────────────────────────────────────────────────────
// TODO: declare global arrays for symm (EXTRALARGE: M=2000, N=2600)

extern "C" void kernel_symm(int M, int N) __asm__("kernel_symm");
extern "C" void kernel_symm(int M, int N) {
    (void)M; (void)N;
    // TODO: implement symm
}
inline constexpr char Fn_symm[] = "kernel_symm";
POLYBENCH_IMPL_2(symm)

// ── syr2k ─────────────────────────────────────────────────────────────────────
// TODO: declare global arrays for syr2k (EXTRALARGE: M=2000, N=2600)

extern "C" void kernel_syr2k(int M, int N) __asm__("kernel_syr2k");
extern "C" void kernel_syr2k(int M, int N) {
    (void)M; (void)N;
    // TODO: implement syr2k
}
inline constexpr char Fn_syr2k[] = "kernel_syr2k";
POLYBENCH_IMPL_2(syr2k)

// ── syrk ──────────────────────────────────────────────────────────────────────
// TODO: declare global arrays for syrk (EXTRALARGE: M=2000, N=2600)

extern "C" void kernel_syrk(int M, int N) __asm__("kernel_syrk");
extern "C" void kernel_syrk(int M, int N) {
    (void)M; (void)N;
    // TODO: implement syrk
}
inline constexpr char Fn_syrk[] = "kernel_syrk";
POLYBENCH_IMPL_2(syrk)

// ── trmm ──────────────────────────────────────────────────────────────────────
// TODO: declare global arrays for trmm (EXTRALARGE: M=2000, N=2600)

extern "C" void kernel_trmm(int M, int N) __asm__("kernel_trmm");
extern "C" void kernel_trmm(int M, int N) {
    (void)M; (void)N;
    // TODO: implement trmm
}
inline constexpr char Fn_trmm[] = "kernel_trmm";
POLYBENCH_IMPL_2(trmm)

// ── cholesky ──────────────────────────────────────────────────────────────────
// TODO: declare global arrays for cholesky (EXTRALARGE: N=4000)

extern "C" void kernel_cholesky(int N) __asm__("kernel_cholesky");
extern "C" void kernel_cholesky(int N) {
    (void)N;
    // TODO: implement cholesky
}
inline constexpr char Fn_cholesky[] = "kernel_cholesky";
POLYBENCH_IMPL_1(cholesky)

// ── durbin ────────────────────────────────────────────────────────────────────
// TODO: declare global arrays for durbin (EXTRALARGE: N=4000)

extern "C" void kernel_durbin(int N) __asm__("kernel_durbin");
extern "C" void kernel_durbin(int N) {
    (void)N;
    // TODO: implement durbin
}
inline constexpr char Fn_durbin[] = "kernel_durbin";
POLYBENCH_IMPL_1(durbin)

// ── gramschmidt ───────────────────────────────────────────────────────────────
// TODO: declare global arrays for gramschmidt (EXTRALARGE: M=2000, N=2600)

extern "C" void kernel_gramschmidt(int M, int N) __asm__("kernel_gramschmidt");
extern "C" void kernel_gramschmidt(int M, int N) {
    (void)M; (void)N;
    // TODO: implement gramschmidt
}
inline constexpr char Fn_gramschmidt[] = "kernel_gramschmidt";
POLYBENCH_IMPL_2(gramschmidt)

// ── lu ────────────────────────────────────────────────────────────────────────
// TODO: declare global arrays for lu (EXTRALARGE: N=4000)

extern "C" void kernel_lu(int N) __asm__("kernel_lu");
extern "C" void kernel_lu(int N) {
    (void)N;
    // TODO: implement lu
}
inline constexpr char Fn_lu[] = "kernel_lu";
POLYBENCH_IMPL_1(lu)

// ── ludcmp ────────────────────────────────────────────────────────────────────
// TODO: declare global arrays for ludcmp (EXTRALARGE: N=4000)

extern "C" void kernel_ludcmp(int N) __asm__("kernel_ludcmp");
extern "C" void kernel_ludcmp(int N) {
    (void)N;
    // TODO: implement ludcmp
}
inline constexpr char Fn_ludcmp[] = "kernel_ludcmp";
POLYBENCH_IMPL_1(ludcmp)

// ── trisolv ───────────────────────────────────────────────────────────────────
// TODO: declare global arrays for trisolv (EXTRALARGE: N=4000)

extern "C" void kernel_trisolv(int N) __asm__("kernel_trisolv");
extern "C" void kernel_trisolv(int N) {
    (void)N;
    // TODO: implement trisolv
}
inline constexpr char Fn_trisolv[] = "kernel_trisolv";
POLYBENCH_IMPL_1(trisolv)

// ── deriche ───────────────────────────────────────────────────────────────────
// TODO: declare global arrays for deriche (EXTRALARGE: W=7680, H=4320)
// Note: deriche uses float arrays internally.

extern "C" void kernel_deriche(int W, int H) __asm__("kernel_deriche");
extern "C" void kernel_deriche(int W, int H) {
    (void)W; (void)H;
    // TODO: implement deriche
}
inline constexpr char Fn_deriche[] = "kernel_deriche";
POLYBENCH_IMPL_2(deriche)

// ── floyd-warshall ────────────────────────────────────────────────────────────
// TODO: declare global arrays for floyd-warshall (EXTRALARGE: N=5600)
// Note: floyd-warshall uses int arrays internally.

extern "C" void kernel_floyd_warshall(int N) __asm__("kernel_floyd_warshall");
extern "C" void kernel_floyd_warshall(int N) {
    (void)N;
    // TODO: implement floyd-warshall
}
inline constexpr char Fn_floyd_warshall[] = "kernel_floyd_warshall";
POLYBENCH_IMPL_1(floyd_warshall)

// ── nussinov ──────────────────────────────────────────────────────────────────
// TODO: declare global arrays for nussinov (EXTRALARGE: N=5500)
// Note: nussinov uses int arrays internally.

extern "C" void kernel_nussinov(int N) __asm__("kernel_nussinov");
extern "C" void kernel_nussinov(int N) {
    (void)N;
    // TODO: implement nussinov
}
inline constexpr char Fn_nussinov[] = "kernel_nussinov";
POLYBENCH_IMPL_1(nussinov)

// ── adi ───────────────────────────────────────────────────────────────────────
// TODO: declare global arrays for adi (EXTRALARGE: TSTEPS=1000, N=2000)

extern "C" void kernel_adi(int TSTEPS, int N) __asm__("kernel_adi");
extern "C" void kernel_adi(int TSTEPS, int N) {
    (void)TSTEPS; (void)N;
    // TODO: implement adi
}
inline constexpr char Fn_adi[] = "kernel_adi";
POLYBENCH_IMPL_2(adi)

// ── fdtd-2d ───────────────────────────────────────────────────────────────────
// TODO: declare global arrays for fdtd-2d (EXTRALARGE: TMAX=1000, NX=2000, NY=2600)

extern "C" void kernel_fdtd_2d(int TMAX, int NX, int NY) __asm__("kernel_fdtd_2d");
extern "C" void kernel_fdtd_2d(int TMAX, int NX, int NY) {
    (void)TMAX; (void)NX; (void)NY;
    // TODO: implement fdtd-2d
}
inline constexpr char Fn_fdtd_2d[] = "kernel_fdtd_2d";
POLYBENCH_IMPL_3(fdtd_2d)

// ── heat-3d ───────────────────────────────────────────────────────────────────
// TODO: declare global arrays for heat-3d (EXTRALARGE: TSTEPS=1000, N=200)

extern "C" void kernel_heat_3d(int TSTEPS, int N) __asm__("kernel_heat_3d");
extern "C" void kernel_heat_3d(int TSTEPS, int N) {
    (void)TSTEPS; (void)N;
    // TODO: implement heat-3d
}
inline constexpr char Fn_heat_3d[] = "kernel_heat_3d";
POLYBENCH_IMPL_2(heat_3d)

// ── jacobi-1d ─────────────────────────────────────────────────────────────────
// Kernel: TSTEPS time-steps of a 1D Jacobi stencil over N points.
// Specialising on (TSTEPS, N) lets the JIT replace both loop bounds with
// constants, enabling outer-loop unrolling and inner-loop vectorisation.

static constexpr int MAX_JAC1D_N = 4000;
static double g_jac1d_A[MAX_JAC1D_N];
static double g_jac1d_B[MAX_JAC1D_N];

static const struct Jac1dInit {
    Jac1dInit() {
        for (int i = 0; i < MAX_JAC1D_N; i++)
            g_jac1d_A[i] = (double)i / MAX_JAC1D_N;
    }
} g_jac1d_init;

extern "C" void kernel_jacobi_1d(int TSTEPS, int N) __asm__("kernel_jacobi_1d");
extern "C" void kernel_jacobi_1d(int TSTEPS, int N) {
    for (int t = 0; t < TSTEPS; t++) {
        for (int i = 1; i < N - 1; i++)
            g_jac1d_B[i] = 0.33333 * (g_jac1d_A[i-1] + g_jac1d_A[i] + g_jac1d_A[i+1]);
        for (int i = 1; i < N - 1; i++)
            g_jac1d_A[i] = 0.33333 * (g_jac1d_B[i-1] + g_jac1d_B[i] + g_jac1d_B[i+1]);
    }
}
inline constexpr char Fn_jacobi_1d[] = "kernel_jacobi_1d";
POLYBENCH_IMPL_2(jacobi_1d)

// ── jacobi-2d ─────────────────────────────────────────────────────────────────
// TODO: declare global arrays for jacobi-2d (EXTRALARGE: TSTEPS=1000, N=2800)

extern "C" void kernel_jacobi_2d(int TSTEPS, int N) __asm__("kernel_jacobi_2d");
extern "C" void kernel_jacobi_2d(int TSTEPS, int N) {
    (void)TSTEPS; (void)N;
    // TODO: implement jacobi-2d
}
inline constexpr char Fn_jacobi_2d[] = "kernel_jacobi_2d";
POLYBENCH_IMPL_2(jacobi_2d)

// ── seidel-2d ─────────────────────────────────────────────────────────────────
// TODO: declare global arrays for seidel-2d (EXTRALARGE: TSTEPS=1000, N=4000)

extern "C" void kernel_seidel_2d(int TSTEPS, int N) __asm__("kernel_seidel_2d");
extern "C" void kernel_seidel_2d(int TSTEPS, int N) {
    (void)TSTEPS; (void)N;
    // TODO: implement seidel-2d
}
inline constexpr char Fn_seidel_2d[] = "kernel_seidel_2d";
POLYBENCH_IMPL_2(seidel_2d)


// ── Benchmark Registrations ───────────────────────────────────────────────────

POLYBENCH_BENCHMARK(correlation,
    ->Args({28, 32})        // MINI
    ->Args({80, 100})       // SMALL
    ->Args({240, 260})      // MEDIUM
    ->Args({1200, 1400})    // LARGE
    ->Args({2600, 3000})    // EXTRALARGE
);

POLYBENCH_BENCHMARK(covariance,
    ->Args({28, 32})        // MINI
    ->Args({80, 100})       // SMALL
    ->Args({240, 260})      // MEDIUM
    ->Args({1200, 1400})    // LARGE
    ->Args({2600, 3000})    // EXTRALARGE
);

POLYBENCH_BENCHMARK(2mm,
    ->Args({16, 18, 22, 24})           // MINI
    ->Args({40, 50, 70, 80})           // SMALL
    ->Args({180, 190, 210, 220})       // MEDIUM
    ->Args({800, 900, 1100, 1200})     // LARGE
    ->Args({1600, 1800, 2200, 2400})   // EXTRALARGE
);

POLYBENCH_BENCHMARK(3mm,
    ->Args({16, 18, 20, 22, 24})           // MINI
    ->Args({40, 50, 60, 70, 80})           // SMALL
    ->Args({180, 190, 200, 210, 220})      // MEDIUM
    ->Args({800, 900, 1000, 1100, 1200})   // LARGE
    ->Args({1600, 1800, 2000, 2200, 2400}) // EXTRALARGE
);

POLYBENCH_BENCHMARK(atax,
    ->Args({38, 42})        // MINI
    ->Args({116, 124})      // SMALL
    ->Args({390, 410})      // MEDIUM
    ->Args({1900, 2100})    // LARGE
    ->Args({1800, 2200})    // EXTRALARGE
);

POLYBENCH_BENCHMARK(bicg,
    ->Args({38, 42})        // MINI
    ->Args({116, 124})      // SMALL
    ->Args({390, 410})      // MEDIUM
    ->Args({1900, 2100})    // LARGE
    ->Args({1800, 2200})    // EXTRALARGE
);

POLYBENCH_BENCHMARK(doitgen,
    ->Args({8, 10, 12})       // MINI
    ->Args({20, 25, 30})      // SMALL
    ->Args({40, 50, 60})      // MEDIUM
    ->Args({140, 150, 160})   // LARGE
    ->Args({220, 250, 270})   // EXTRALARGE
);

POLYBENCH_BENCHMARK(mvt,
    ->Arg(40)     // MINI
    ->Arg(120)    // SMALL
    ->Arg(400)    // MEDIUM
    ->Arg(2000)   // LARGE
    ->Arg(4000)   // EXTRALARGE
);

POLYBENCH_BENCHMARK(gemm,
    ->Args({20, 25, 30})          // MINI
    ->Args({60, 70, 80})          // SMALL
    ->Args({200, 220, 240})       // MEDIUM
    ->Args({1000, 1100, 1200})    // LARGE
    ->Args({2000, 2300, 2600})    // EXTRALARGE
);

POLYBENCH_BENCHMARK(gemver,
    ->Arg(40)     // MINI
    ->Arg(120)    // SMALL
    ->Arg(400)    // MEDIUM
    ->Arg(2000)   // LARGE
    ->Arg(4000)   // EXTRALARGE
);

POLYBENCH_BENCHMARK(gesummv,
    ->Arg(30)     // MINI
    ->Arg(90)     // SMALL
    ->Arg(250)    // MEDIUM
    ->Arg(1300)   // LARGE
    ->Arg(2800)   // EXTRALARGE
);

POLYBENCH_BENCHMARK(symm,
    ->Args({20, 30})        // MINI
    ->Args({60, 80})        // SMALL
    ->Args({200, 240})      // MEDIUM
    ->Args({1000, 1200})    // LARGE
    ->Args({2000, 2600})    // EXTRALARGE
);

POLYBENCH_BENCHMARK(syr2k,
    ->Args({20, 30})        // MINI
    ->Args({60, 80})        // SMALL
    ->Args({200, 240})      // MEDIUM
    ->Args({1000, 1200})    // LARGE
    ->Args({2000, 2600})    // EXTRALARGE
);

POLYBENCH_BENCHMARK(syrk,
    ->Args({20, 30})        // MINI
    ->Args({60, 80})        // SMALL
    ->Args({200, 240})      // MEDIUM
    ->Args({1000, 1200})    // LARGE
    ->Args({2000, 2600})    // EXTRALARGE
);

POLYBENCH_BENCHMARK(trmm,
    ->Args({20, 30})        // MINI
    ->Args({60, 80})        // SMALL
    ->Args({200, 240})      // MEDIUM
    ->Args({1000, 1200})    // LARGE
    ->Args({2000, 2600})    // EXTRALARGE
);

POLYBENCH_BENCHMARK(cholesky,
    ->Arg(40)     // MINI
    ->Arg(120)    // SMALL
    ->Arg(400)    // MEDIUM
    ->Arg(2000)   // LARGE
    ->Arg(4000)   // EXTRALARGE
);

POLYBENCH_BENCHMARK(durbin,
    ->Arg(40)     // MINI
    ->Arg(120)    // SMALL
    ->Arg(400)    // MEDIUM
    ->Arg(2000)   // LARGE
    ->Arg(4000)   // EXTRALARGE
);

POLYBENCH_BENCHMARK(gramschmidt,
    ->Args({20, 30})        // MINI
    ->Args({60, 80})        // SMALL
    ->Args({200, 240})      // MEDIUM
    ->Args({1000, 1200})    // LARGE
    ->Args({2000, 2600})    // EXTRALARGE
);

POLYBENCH_BENCHMARK(lu,
    ->Arg(40)     // MINI
    ->Arg(120)    // SMALL
    ->Arg(400)    // MEDIUM
    ->Arg(2000)   // LARGE
    ->Arg(4000)   // EXTRALARGE
);

POLYBENCH_BENCHMARK(ludcmp,
    ->Arg(40)     // MINI
    ->Arg(120)    // SMALL
    ->Arg(400)    // MEDIUM
    ->Arg(2000)   // LARGE
    ->Arg(4000)   // EXTRALARGE
);

POLYBENCH_BENCHMARK(trisolv,
    ->Arg(40)     // MINI
    ->Arg(120)    // SMALL
    ->Arg(400)    // MEDIUM
    ->Arg(2000)   // LARGE
    ->Arg(4000)   // EXTRALARGE
);

POLYBENCH_BENCHMARK(deriche,
    ->Args({64, 64})        // MINI
    ->Args({192, 128})      // SMALL
    ->Args({720, 480})      // MEDIUM
    ->Args({4096, 2160})    // LARGE
    ->Args({7680, 4320})    // EXTRALARGE
);

POLYBENCH_BENCHMARK(floyd_warshall,
    ->Arg(60)     // MINI
    ->Arg(180)    // SMALL
    ->Arg(500)    // MEDIUM
    ->Arg(2800)   // LARGE
    ->Arg(5600)   // EXTRALARGE
);

POLYBENCH_BENCHMARK(nussinov,
    ->Arg(60)     // MINI
    ->Arg(180)    // SMALL
    ->Arg(500)    // MEDIUM
    ->Arg(2500)   // LARGE
    ->Arg(5500)   // EXTRALARGE
);

POLYBENCH_BENCHMARK(adi,
    ->Args({20, 20})        // MINI
    ->Args({40, 60})        // SMALL
    ->Args({100, 200})      // MEDIUM
    ->Args({500, 1000})     // LARGE
    ->Args({1000, 2000})    // EXTRALARGE
);

POLYBENCH_BENCHMARK(fdtd_2d,
    ->Args({20, 20, 30})        // MINI
    ->Args({40, 60, 80})        // SMALL
    ->Args({100, 200, 240})     // MEDIUM
    ->Args({500, 1000, 1200})   // LARGE
    ->Args({1000, 2000, 2600})  // EXTRALARGE
);

POLYBENCH_BENCHMARK(heat_3d,
    ->Args({20, 10})      // MINI
    ->Args({40, 20})      // SMALL
    ->Args({100, 40})     // MEDIUM
    ->Args({500, 120})    // LARGE
    ->Args({1000, 200})   // EXTRALARGE
);

POLYBENCH_BENCHMARK(jacobi_1d,
    ->Args({20, 30})        // MINI
    ->Args({40, 120})       // SMALL
    ->Args({100, 400})      // MEDIUM
    ->Args({500, 2000})     // LARGE
    ->Args({1000, 4000})    // EXTRALARGE
);

POLYBENCH_BENCHMARK(jacobi_2d,
    ->Args({20, 30})        // MINI
    ->Args({40, 90})        // SMALL
    ->Args({100, 250})      // MEDIUM
    ->Args({500, 1300})     // LARGE
    ->Args({1000, 2800})    // EXTRALARGE
);

POLYBENCH_BENCHMARK(seidel_2d,
    ->Args({20, 40})        // MINI
    ->Args({40, 120})       // SMALL
    ->Args({100, 400})      // MEDIUM
    ->Args({500, 2000})     // LARGE
    ->Args({1000, 4000})    // EXTRALARGE
);


// ── IR Embedding Trigger ──────────────────────────────────────────────────────

volatile bool g_polybench_dummy_trigger = false;

extern "C" __attribute__((used)) void polybench_dummy_registration() {
    auto* RS = CRS::ClangRuntimeSpecializer::init();
    if (g_polybench_dummy_trigger) {
        RS->callSpecialized<Fn_correlation, void>(0, 0);
        RS->callSpecialized<Fn_covariance, void>(0, 0);
        RS->callSpecialized<Fn_2mm, void>(0, 0, 0, 0);
        RS->callSpecialized<Fn_3mm, void>(0, 0, 0, 0, 0);
        RS->callSpecialized<Fn_atax, void>(0, 0);
        RS->callSpecialized<Fn_bicg, void>(0, 0);
        RS->callSpecialized<Fn_doitgen, void>(0, 0, 0);
        RS->callSpecialized<Fn_mvt, void>(0);
        RS->callSpecialized<Fn_gemm, void>(0, 0, 0);
        RS->callSpecialized<Fn_gemver, void>(0);
        RS->callSpecialized<Fn_gesummv, void>(0);
        RS->callSpecialized<Fn_symm, void>(0, 0);
        RS->callSpecialized<Fn_syr2k, void>(0, 0);
        RS->callSpecialized<Fn_syrk, void>(0, 0);
        RS->callSpecialized<Fn_trmm, void>(0, 0);
        RS->callSpecialized<Fn_cholesky, void>(0);
        RS->callSpecialized<Fn_durbin, void>(0);
        RS->callSpecialized<Fn_gramschmidt, void>(0, 0);
        RS->callSpecialized<Fn_lu, void>(0);
        RS->callSpecialized<Fn_ludcmp, void>(0);
        RS->callSpecialized<Fn_trisolv, void>(0);
        RS->callSpecialized<Fn_deriche, void>(0, 0);
        RS->callSpecialized<Fn_floyd_warshall, void>(0);
        RS->callSpecialized<Fn_nussinov, void>(0);
        RS->callSpecialized<Fn_adi, void>(0, 0);
        RS->callSpecialized<Fn_fdtd_2d, void>(0, 0, 0);
        RS->callSpecialized<Fn_heat_3d, void>(0, 0);
        RS->callSpecialized<Fn_jacobi_1d, void>(0, 0);
        RS->callSpecialized<Fn_jacobi_2d, void>(0, 0);
        RS->callSpecialized<Fn_seidel_2d, void>(0, 0);
    }
}

BENCHMARK_MAIN();
