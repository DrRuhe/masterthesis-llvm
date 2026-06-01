// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --dump-input=always
// RUN: CRS_DEFAULT_PIPELINE=1 %t.exe | FileCheck %s --dump-input=always
// RUN: %clangxx -g -O3 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.o3.exe
// RUN: %t.o3.exe | FileCheck %s --dump-input=always

// Scenario A: compile-time initialized static — verifies the JIT resolves the
// host's actual value (42) instead of the zero-initialized shadow copy.
// Scenario F: function with no static globals is also tested (no regression).

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

static int g_value = 42;

// Takes a dummy arg so assertSpecializedIsEquivalent can be used with a non-empty tuple.
extern "C" int read_static(int unused);
int read_static(int unused) { return g_value; }

extern "C" int add_arg(int x);
int add_arg(int x) { return x + 1; }  // no static globals — regression check

int main() {
    clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(
        clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Info);
    clangRuntimeSpecializer::ClangRuntimeSpecializer::init();

    // Scenario A
    int dummy = 0;
    clangRuntimeSpecializer::assertSpecializedIsEquivalent(
        read_static, std::tie(dummy), std::tie(dummy), []{});
    // CHECK: Successfully specialized! No differences could be observed.

    // Scenario F
    int arg = 5;
    clangRuntimeSpecializer::assertSpecializedIsEquivalent(
        add_arg, std::tie(arg), std::tie(arg), []{});
    // CHECK: Successfully specialized! No differences could be observed.

    return 0;
}
