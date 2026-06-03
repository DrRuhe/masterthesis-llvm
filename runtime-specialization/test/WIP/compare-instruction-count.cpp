// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: CRS_DEFAULT_PIPELINE=0 %t.exe | FileCheck %s --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

extern "C" int mypow(int x, int y);
int mypow(int x, int y) {
    int result = 1;
    for (int i = 0; i < y; i++) {
        result *= x;
    }
    return result;
}

int main() {
    // CHECK: Comparing instruction counts from specializing
    // CHECK: {{before *after *instruction *change}}
    // CHECK: {{82 *1 *total *-81, -99%}}
    // CHECK-EMPTY:
    // CHECK: {{28 *0 *load *-28, -100%}}
    // CHECK: {{17 *0 *branch *-17, -100%}}
    // CHECK: {{14 *0 *store *-14, -100%}}
    // CHECK: {{10 *0 *arith *-10, -100%}}
    // CHECK: {{6 *0 *cmp *-6, -100%}}
    // CHECK: {{4 *0 *other *-4, -100%}}
    // CHECK: {{2 *1 *ret *-1, -50%}}
    // CHECK: {{1 *0 *call *-1, -100%}}
    clangRuntimeSpecializer::compareFunctionInstructionCounts(mypow, 3, 5);
    return 0;
}
