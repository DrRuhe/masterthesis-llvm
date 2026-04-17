// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

extern "C" int mypow(int x, int y) __asm__("mypow");
int mypow(int x, int y) {
    int result = 1;
    for (int i = 0; i < y; i++) {
        result *= x;
    }
    return result;
}

inline constexpr char Fn_mypow[] = "mypow";

int main() {
    // CHECK: Comparing instruction counts from specializing mypow:
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
    clangRuntimeSpecializer::compareFunctionInstructionCounts(Fn_mypow, mypow, 3, 5);
    return 0;
}
