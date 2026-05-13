// RUN: %clangxx -g -O0 %s -o %t.exe
// RUN: %t.exe 2>&1 | FileCheck %s --check-prefix=EXE --dump-input=always

// Test that specializeLambda throws ClangRuntimeSpecializerDumpedIRError when
// the TU was not compiled with the IRDumpingPass plugin (resolvedName == nullptr).

#include "ClangRuntimeSpecializer.h"
#include <cstdio>
#include <stdexcept>

int main() {
  auto lambda = [](int x) -> int { return x * 2; };

  try {
    auto spec = clangRuntimeSpecializer::specializeLambda<int>(lambda);
    printf("FAIL: expected exception was not thrown\n");
  } catch (const clangRuntimeSpecializer::ClangRuntimeSpecializerDumpedIRError& e) {
    // EXE: ClangRuntimeSpecializerDumpedIRError: {{No IR blobs registered|TU was not compiled with the IRDumpingPass plugin}}
    printf("ClangRuntimeSpecializerDumpedIRError: %s\n", e.what());
  } catch (const std::exception& e) {
    printf("unexpected exception: %s\n", e.what());
  }

  return 0;
}
