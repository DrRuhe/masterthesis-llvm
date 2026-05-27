// Binary size measurement — large kernel (spec 015).
//
// Same structure as binary-size-small.cpp but with four floating-point kernels
// in the TU (dot product, matrix-vector multiply, matrix multiply, convolution).
// More functions → larger IR dump blob, demonstrating that the embedding overhead
// scales with TU complexity.  BitcodeSizeBytes should be at least 2× the value
// from binary-size-small.cpp (SC-003).

// RUN: %clangxx -O3 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext \
// RUN:   %s -o %t.full.exe
// RUN: %clangxx -O3 %s -o %t.noEmbed.exe
// RUN: wc -c < %t.full.exe    | awk '{printf "full_bytes: %d\n",     $1}' >  %t.sizes
// RUN: wc -c < %t.noEmbed.exe | awk '{printf "no_embed_bytes: %d\n", $1}' >> %t.sizes
// RUN: size -A %llvmshlibdir/libClangRuntimeSpecializer%shlibext \
// RUN:   | awk '/^\.text /{sum+=$2} END{printf "crs_lib_text_bytes: %d\n", sum}' >> %t.sizes
// RUN: %t.full.exe >> %t.sizes
// RUN: FileCheck %s < %t.sizes

// CHECK: full_bytes: {{[1-9][0-9]+}}
// CHECK: no_embed_bytes: {{[1-9][0-9]+}}
// CHECK: crs_lib_text_bytes: {{[1-9][0-9]+}}
// CHECK: BitcodeSizeBytes: {{[1-9][0-9]+}}

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

// Kernel 1: dot product
extern "C" double vec_dot(const double *a, const double *b, int n)
    __asm__("vec_dot");

double vec_dot(const double *a, const double *b, int n) {
  double acc = 0.0;
  for (int i = 0; i < n; i++)
    acc += a[i] * b[i];
  return acc;
}

// Kernel 2: matrix-vector multiply  y = A * x
extern "C" void mat_vec(const double *A, const double *x, double *y, int m,
                        int n) __asm__("mat_vec");

void mat_vec(const double *A, const double *x, double *y, int m, int n) {
  for (int i = 0; i < m; i++) {
    double acc = 0.0;
    for (int j = 0; j < n; j++)
      acc += A[i * n + j] * x[j];
    y[i] = acc;
  }
}

// Kernel 3: dense matrix multiply  C += alpha * A * B
extern "C" void gemm(const double *A, const double *B, double *C, int m, int n,
                     int k, double alpha) __asm__("gemm");

void gemm(const double *A, const double *B, double *C, int m, int n, int k,
          double alpha) {
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      double acc = 0.0;
      for (int l = 0; l < k; l++)
        acc += A[i * k + l] * B[l * n + j];
      C[i * n + j] += alpha * acc;
    }
  }
}

// Kernel 4: 1-D convolution  out[i] = sum_j in[i+j] * kernel[j]
extern "C" void conv1d(const double *in, const double *kernel, double *out,
                       int n, int klen) __asm__("conv1d");

void conv1d(const double *in, const double *kernel, double *out, int n,
            int klen) {
  for (int i = 0; i < n - klen + 1; i++) {
    double acc = 0.0;
    for (int j = 0; j < klen; j++)
      acc += in[i + j] * kernel[j];
    out[i] = acc;
  }
}

int main() {
  clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
  auto stats = clangRuntimeSpecializer::ClangRuntimeSpecializer::getModuleStats();
  std::printf("BitcodeSizeBytes: %zu\n", stats.BitcodeSizeBytes);
  return 0;
}
