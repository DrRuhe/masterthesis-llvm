// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --check-prefix=EXE --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

// Plain C++ function — no __asm__ annotation needed.
int computeFoo(int base, int x) {
  return base + x;
}

int main() {
  clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(
      clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Info);

  // specializeOnly with function pointer — IRDumpingPass extracts name at compile time.
  // EXE: INFO: Specializing call to:
  auto spec = clangRuntimeSpecializer::specializeOnly<int>(&computeFoo, 10, 5);

  // EXE: result=15
  printf("result=%d\n", spec());

  if (spec() != computeFoo(10, 5))
    printf("FAIL: result mismatch\n");

  // EXE-NOT: FAIL

  return 0;
}
