// Regression test: function-pointer specialization through a helper parameter.
// Before the fix, IRRewritingPass missed this pattern and left resolvedName=null,
// which triggered ClangRuntimeSpecializerDumpedIRError at runtime.
//
// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --check-prefix=EXE --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

int add2(int a, int b) { return a + b; }

template <class F>
__attribute__((noinline))
int runThroughHelper(F func) {
  auto spec = clangRuntimeSpecializer::specializeOnly<int>(func, 10, 5);
  return spec();
}

int main() {
  try {
    int result = runThroughHelper(&add2);
    // EXE: result=15
    std::printf("result=%d\n", result);
  } catch (const clangRuntimeSpecializer::ClangRuntimeSpecializerDumpedIRError &e) {
    std::printf("unexpected DumpedIRError: %s\n", e.what());
  }
  // EXE-NOT: unexpected DumpedIRError
  return 0;
}

