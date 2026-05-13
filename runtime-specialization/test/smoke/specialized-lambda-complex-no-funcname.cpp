// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --check-prefix=EXE --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

// Tests the primary motivation for spec 010: a lambda with complex inline logic
// (loop + conditional branching) that cannot be expressed as a single extern-C kernel.
// The IRDumpingPass must discover the lambda's operator() mangled name and embed it.

struct Obj { int value; };

int main() {
  clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(
      clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Info);

  Obj obj{10};
  auto lambda = [&obj](int x, bool y) -> int {
    int acc = 0;
    for (int i = 0; i < x; i++) {
      acc += y ? (obj.value * (i + 1)) : (obj.value - (i + 1));
    }
    return acc;
  };

  // EXE: INFO: Specializing lambda call to: _Z
  auto spec = clangRuntimeSpecializer::specializeLambda<int>(lambda);

  // obj.value=10, x=3, y=true: 10*1 + 10*2 + 10*3 = 60
  // EXE: result_true=60
  printf("result_true=%d\n", spec(3, true));

  // obj.value=10, x=3, y=false: (10-1) + (10-2) + (10-3) = 24
  // EXE: result_false=24
  printf("result_false=%d\n", spec(3, false));

  if (spec(3, true) != lambda(3, true))
    printf("FAIL: mismatch for (3, true)\n");
  if (spec(3, false) != lambda(3, false))
    printf("FAIL: mismatch for (3, false)\n");
  if (spec(0, true) != lambda(0, true))
    printf("FAIL: mismatch for (0, true)\n");

  // EXE-NOT: FAIL

  return 0;
}
