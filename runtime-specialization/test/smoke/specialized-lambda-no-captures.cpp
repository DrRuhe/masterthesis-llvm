// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --check-prefix=EXE --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

// Kernel: only uses explicit args; unused_closure param is ignored.
extern "C" int triple_with_closure(void* /*unused_closure*/, int x) __asm__("triple_with_closure");
int triple_with_closure(void* /*unused_closure*/, int x) {
  return x * 3;
}

int main() {
  clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(
      clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Info);

  // Captureless lambda; closure is empty — the pass still discovers the lambda's
  // operator() as the specialization target.
  auto lambda = [](int x) -> int {
    return triple_with_closure(nullptr, x);
  };

  // EXE: INFO: Specializing lambda call to: _Z
  auto spec = clangRuntimeSpecializer::specializeLambda<int>(lambda);

  // EXE: result=12
  printf("result=%d\n", spec(4));

  // EXE: result2=21
  printf("result2=%d\n", spec(7));

  if (spec(4) != lambda(4))
    printf("FAIL: mismatch for 4\n");
  if (spec(7) != lambda(7))
    printf("FAIL: mismatch for 7\n");

  // EXE-NOT: FAIL

  return 0;
}
