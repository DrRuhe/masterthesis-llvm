// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --dump-input=always

// Scenario D: function writes to a static global. The write must be visible in
// the host process after the specialized call returns (write goes to host memory
// via the inttoptr constant, not to a dead JIT-local zero copy).

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

static int g_accum = 0;

extern "C" int accumulate(int v);
int accumulate(int v) {
    g_accum += v;
    return g_accum;
}

int main() {
    clangRuntimeSpecializer::ClangRuntimeSpecializer::init();

    int v = 5;
    int result = clangRuntimeSpecializer::specializeOrFallback(accumulate, v);
    std::printf("result=%d host_accum=%d\n", result, g_accum);
    // CHECK: result=5 host_accum=5

    return 0;
}
