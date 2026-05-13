// Test: specializeOnly with a runtime-variable function pointer (FR-018).
// The IRDumpingPass must emit a fatal_error at compile time when the function
// pointer is not a compile-time constant.
//
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
