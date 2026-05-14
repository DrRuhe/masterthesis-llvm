// Test: specializeOnly with a runtime-variable function pointer (FR-018).
// The IRDumpingPass silently skips rewriting when the function pointer is a
// runtime variable (e.g. passed through a template wrapper). Fatal error was
// too aggressive: it broke polybench_bench.cpp benchmark helpers that pass
// function pointers through template parameters. Kept as XFAIL documentation.
//
// XFAIL: *
// RUN: %clangxx -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe 2>&1 | FileCheck %s
// CHECK: IRDumpingPass: specializeOnly/callSpecialized requires a compile-time-constant function pointer

#include "ClangRuntimeSpecializer.h"

int myFunc(int x) { return x * 2; }

// fp is a runtime value (function parameter) — not a compile-time constant.
int specializeWithRuntimePtr(int (*fp)(int), int x) {
    return clangRuntimeSpecializer::specializeOnly<int>(fp, x).call();
}

int main() {
    return specializeWithRuntimePtr(myFunc, 21);
}
