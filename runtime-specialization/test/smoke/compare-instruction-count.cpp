// RUN: %clangxx -g -O0 -emit-llvm -c %s -o %t.bc
// RUN: opt --verify-debuginfo-preserve -load-pass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext -passes='runtime-specialization-IR-dumping' %t.bc -o %t.opt.bc
// RUN: %clangxx -g %t.opt.bc -o %t.exe
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
    // CHECK: {{34 *1 *total *-33, -97%}}
    // CHECK-EMPTY:
    // CHECK: {{11 *0 *other *-11, -100%}}
    // CHECK: {{10 *0 *arith *-10, -100%}}
    // CHECK: {{6 *0 *branch *-6, -100%}}
    // CHECK: {{6 *0 *cmp *-6, -100%}}
    // CHECK: {{1 *1 *ret *0, 0%}}
    // CHECK: {{0 *0 *call *0, 0%}}
    // CHECK: {{0 *0 *load *0, 0%}}
    // CHECK: {{0 *0 *store *0, 0%}}
    clangRuntimeSpecializer::compareFunctionInstructionCounts<Fn_mypow>(mypow, 3, 5);
    return 0;
}
