// Regression test: callSpecialized through a helper function-parameter.
// This exercises the same forwarding pattern as benchmark helpers.
//
// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --check-prefix=EXE --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

int mul2(int x) { return x * 2; }

template <class F>
__attribute__((noinline))
int callThroughHelper(F func, int v) {
  return clangRuntimeSpecializer::callSpecialized<int>(func, v);
}

int main() {
  try {
    int result = callThroughHelper(&mul2, 21);
    // EXE: result=42
    std::printf("result=%d\n", result);
  } catch (const clangRuntimeSpecializer::ClangRuntimeSpecializerDumpedIRError &e) {
    std::printf("unexpected DumpedIRError: %s\n", e.what());
  }
  // EXE-NOT: unexpected DumpedIRError
  return 0;
}

