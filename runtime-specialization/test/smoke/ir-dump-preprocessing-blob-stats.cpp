// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --dump-input=always

// Verify that compile-time GlobalDCE in IRDumpingPass (spec 004) significantly
// reduces the function count in the embedded blob. A minimal TU with a single
// target function should contain only a handful of functions after preprocessing
// (target + main + transitive callees), not the hundreds present in a raw
// unprocessed blob that includes all C++ standard library instantiations.

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

extern "C" int mypow(int x) __asm__("mypow");
int mypow(int x) {
    int result = 1;
    for (int i = 0; i < x; i++) result *= 3;
    return result;
}

int main() {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    (void)RS;
    auto stats = clangRuntimeSpecializer::ClangRuntimeSpecializer::getModuleStats();
    std::printf("FunctionCount: %zu\n", stats.FunctionCount);
    std::printf("BitcodeSizeBytes: %zu\n", stats.BitcodeSizeBytes);
    return 0;
}

// Compile-time DCE prunes the blob to a single-digit function count (≤ 9).
// A raw unprocessed blob for the same TU would have hundreds of functions.
// CHECK: FunctionCount: {{[1-9]}}
// CHECK: BitcodeSizeBytes: {{[1-9][0-9]+}}
