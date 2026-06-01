// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: CRS_DEFAULT_PIPELINE=2 %t.exe | FileCheck %s --dump-input=always

// Scenario E: P2 invariant-load constant folding through a static array.
// table[0] = 10 is a read-only access at a constant index. After spec 017 replaces
// the global with inttoptr(HOST_ADDR), StaticMutabilityAnalysis marks the load
// !invariant.load. P2's IPSCCP then folds it to the constant 10 at JIT compile time.

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

static const int g_table[4] = {10, 20, 30, 40};

extern "C" int read_table_zero(int unused);
int read_table_zero(int unused) { return g_table[0]; }

int main() {
    clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(
        clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Debug);
    clangRuntimeSpecializer::ClangRuntimeSpecializer::init();

    int arg = 0;
    int result = clangRuntimeSpecializer::specializeOrFallback(read_table_zero, arg);

    // P2 should fold g_table[0] to the constant 10 at JIT time.
    // CHECK: Optimized specialized function IR:
    // CHECK: ret i32 10

    std::printf("result=%d\n", result);
    // CHECK: result=10

    return 0;
}
