// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --dump-input=always
// RUN: CRS_DEFAULT_PIPELINE=1 %t.exe | FileCheck %s --dump-input=always

// Scenario B: modify a static global after init(), then specialize.
// add_to_base(x) returns g_base + x. After setting g_base=7 and specializing
// with x=3, the result must be 10 (not 3, which would happen with g_base=0).

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

static int g_base = 0;

extern "C" int add_to_base(int x);
int add_to_base(int x) { return g_base + x; }

int main() {
    clangRuntimeSpecializer::ClangRuntimeSpecializer::init();

    g_base = 7;

    int arg = 3;
    int result = clangRuntimeSpecializer::specializeOrFallback(add_to_base, arg);
    std::printf("result=%d\n", result);
    // CHECK: result=10

    return 0;
}
