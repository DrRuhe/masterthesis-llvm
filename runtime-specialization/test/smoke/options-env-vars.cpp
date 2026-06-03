// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: CRS_DEFAULT_MAX_FIXPOINT_ITERATIONS=2 CRS_DEFAULT_LOOP_UNROLL_COUNT=16 CRS_DEFAULT_P2_LARGE_MODULE_INSTR_THRESHOLD=50000 %t.exe | FileCheck %s --check-prefix=EXE
// EXE: env-vars ok

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

extern "C" int square(int x);
int square(int x) {
    return x * x;
}

int main() {
    int result = clangRuntimeSpecializer::specializeOrFallback(square, 5);
    (void)result;
    std::printf("env-vars ok\n");
    return 0;
}
