// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --check-prefix=EXE --dump-input=always
// RUN: CRS_DEFAULT_PIPELINE=1 %t.exe | FileCheck %s --check-prefix=EXE --dump-input=always
// RUN: %clangxx -g -O3 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.o3.exe
// RUN: %t.o3.exe | FileCheck %s --check-prefix=EXE --dump-input=always
// RUN: CRS_DEFAULT_PIPELINE=1 %t.o3.exe | FileCheck %s --check-prefix=EXE --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

// Zero-explicit-arg lambda: all inputs are captured; the specialized function takes
// no runtime parameters. The SpecializedLambda<int> returned here is callable as spec().
// Internally this routes through specializeOnlyImpl (the if-constexpr NArgs==0 branch),
// unifying with the specializeOnly code path.

int main() {
  clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(
      clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Info);

  const int base = 42;
  const int scale = 3;
  auto lambda = [base, scale]() -> int { return base * scale; };

  // Zero-arg lambda routes through specializeOnlyImpl, which logs "Specializing call to:".
  // EXE: INFO: Specializing call to: _Z
  auto spec = clangRuntimeSpecializer::specializeLambda<int>(lambda);

  // 42 * 3 = 126
  // EXE: result=126
  printf("result=%d\n", spec());

  if (spec() != lambda())
    printf("FAIL: mismatch\n");

  // EXE-NOT: FAIL

  return 0;
}
