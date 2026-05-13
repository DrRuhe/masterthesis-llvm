// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --check-prefix=EXE --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

// Capture-free lambda: the closure is empty (zero-size struct by C++ standard).
// The specializer still serializes &lambda as the sole baked argument (it is unused
// by the body since there are no captures). This tests the degenerate partial
// specialization case with no constants actually baked in.

int main() {
  clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(
      clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Info);

  auto lambda = [](int x) -> int { return x * x + 1; };

  // EXE: INFO: Specializing lambda call to: _Z
  auto spec = clangRuntimeSpecializer::specializeLambda<int>(lambda);

  // EXE: result_5=26
  printf("result_5=%d\n", spec(5));

  // EXE: result_7=50
  printf("result_7=%d\n", spec(7));

  if (spec(5) != lambda(5))
    printf("FAIL: mismatch for 5\n");
  if (spec(7) != lambda(7))
    printf("FAIL: mismatch for 7\n");
  if (spec(0) != lambda(0))
    printf("FAIL: mismatch for 0\n");

  // EXE-NOT: FAIL

  return 0;
}
