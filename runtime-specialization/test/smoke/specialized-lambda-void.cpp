// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --check-prefix=EXE --dump-input=always
// RUN: CRS_DEFAULT_PIPELINE=1 %t.exe | FileCheck %s --check-prefix=EXE --dump-input=always
// RUN: %clangxx -g -O3 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.o3.exe
// RUN: %t.o3.exe | FileCheck %s --check-prefix=EXE --dump-input=always
// RUN: CRS_DEFAULT_PIPELINE=1 %t.o3.exe | FileCheck %s --check-prefix=EXE --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

// Kernel with void return: called from the lambda body.
struct State { int total; };

extern "C" void accumulate(State* s, int delta) __asm__("accumulate");
void accumulate(State* s, int delta) {
  s->total += delta;
}

int main() {
  clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(
      clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Info);

  State state{0};
  // Capture state by value; the lambda's operator() is the specialization target.
  auto lambda = [state](int delta) mutable -> void {
    accumulate(&state, delta);
  };

  // EXE: INFO: Specializing lambda call to: _Z
  auto spec = clangRuntimeSpecializer::specializeLambda<void>(lambda);

  spec(7);
  spec(3);

  // EXE: void-test-ok
  printf("void-test-ok\n");

  return 0;
}
