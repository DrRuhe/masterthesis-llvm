// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>
#include <cassert>

// A trivial function used as the specialization target.
extern "C" int trivial_add(int a, int b) __asm__("trivial_add");
int trivial_add(int a, int b) { return a + b; }

inline constexpr char Fn_trivial_add[] = "trivial_add";

int main() {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();

    // CHECK: timeout_test: no_timeout=1
    // Specialization with generous timeout should succeed and return a valid function.
    auto opts_ok = clangRuntimeSpecializer::ClangRuntimeSpecializer::Options::Default()
                       .withJITTimeoutSeconds(60);
    auto fn_ok = RS->specializeOnly<int>(Fn_trivial_add, opts_ok, 3, 4);
    bool no_timeout = (bool)fn_ok;
    printf("timeout_test: no_timeout=%d\n", (int)no_timeout);
    assert(no_timeout && "specialization with 60s timeout should not time out");
    assert(fn_ok() == 7 && "specialized function should return correct result");

    // CHECK: timeout_test: default_no_timeout=1
    // Default options (JITTimeoutSeconds=0) mean no timeout — must always succeed.
    auto fn_default = RS->specializeOnly<int>(Fn_trivial_add, 5, 6);
    printf("timeout_test: default_no_timeout=%d\n", (int)(bool)fn_default);
    assert((bool)fn_default && "default options should produce a valid function");

    return 0;
}
