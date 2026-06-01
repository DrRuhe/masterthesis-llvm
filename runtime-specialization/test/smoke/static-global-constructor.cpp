// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --dump-input=always
// RUN: CRS_DEFAULT_PIPELINE=1 %t.exe | FileCheck %s --dump-input=always

// Scenario C: constructor-initialized static array.
// Previously this would crash (JITLink setMutableContent assertion) or silently
// return zeros because IRDumpingPass erases llvm.global_ctors, leaving the JIT's
// copy zero-initialized while IPSCCP folds all loads to zero.
// After spec 017, the JIT resolves the array to host memory and reads real values.

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

static double g_data[8];

struct DataInit {
    DataInit() {
        for (int i = 0; i < 8; i++)
            g_data[i] = i * 1.5;
    }
} g_data_init;

extern "C" double read_data(int i);
double read_data(int i) { return g_data[i]; }

int main() {
    clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(
        clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Info);
    clangRuntimeSpecializer::ClangRuntimeSpecializer::init();

    int idx = 3;
    clangRuntimeSpecializer::assertSpecializedIsEquivalent(
        read_data, std::tie(idx), std::tie(idx), []{});
    // CHECK: Successfully specialized! No differences could be observed.

    // Also verify the raw value is correct (g_data[3] = 3 * 1.5 = 4.5)
    double val = clangRuntimeSpecializer::specializeOrFallback(read_data, idx);
    std::printf("data[3]=%.1f\n", val);
    // CHECK: data[3]=4.5

    return 0;
}
