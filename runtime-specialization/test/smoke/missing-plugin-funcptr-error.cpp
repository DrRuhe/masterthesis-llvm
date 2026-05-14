// Test: specializeOnly<int>(&myFunc, 42) compiled WITHOUT the IRDumpingPass plugin.
// The runtime should detect that no IR blobs were registered (i.e. plugin was absent)
// and throw ClangRuntimeSpecializerDumpedIRError.
//
// RUN: %clangxx -g -O0 %s -o %t.exe
// RUN: %t.exe 2>&1 | FileCheck %s --check-prefix=EXE --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

int myFunc(int x) { return x * 2; }

int main() {
  try {
    auto spec = clangRuntimeSpecializer::specializeOnly<int>(&myFunc, 42);
    printf("no exception\n");
  } catch (const clangRuntimeSpecializer::ClangRuntimeSpecializerDumpedIRError& e) {
    // EXE: ClangRuntimeSpecializerDumpedIRError
    printf("ClangRuntimeSpecializerDumpedIRError: %s\n", e.what());
  } catch (const std::exception& e) {
    printf("unexpected exception: %s\n", e.what());
  }
  return 0;
}
