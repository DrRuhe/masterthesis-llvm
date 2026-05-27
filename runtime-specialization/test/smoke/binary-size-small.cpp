// Binary size measurement — small kernel (spec 015).
//
// Compiles this file twice: once with -fpass-plugin (IR blob embedded) and
// once without (JIT infrastructure only, no blob).  Reports:
//   full_bytes        — binary with IR dump + JIT infra
//   no_embed_bytes    — binary with JIT infra only
//   crs_lib_text_bytes — .text section of libClangRuntimeSpecializer.so
//   BitcodeSizeBytes  — blob size from getModuleStats() (cross-check)
//
// IR dump overhead = full_bytes - no_embed_bytes
// These two values should agree with BitcodeSizeBytes within ~25% (the diff
// also includes registration globals and the constructor calling register_blob).

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

// Small kernel: count array elements strictly above a threshold.
extern "C" int count_above(const int *data, int n, int threshold)
    __asm__("count_above");

int count_above(const int *data, int n, int threshold) {
  int count = 0;
  for (int i = 0; i < n; i++)
    count += (data[i] > threshold) ? 1 : 0;
  return count;
}

int main() {
  clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
  auto stats = clangRuntimeSpecializer::ClangRuntimeSpecializer::getModuleStats();
  std::printf("BitcodeSizeBytes: %zu\n", stats.BitcodeSizeBytes);
  return 0;
}
