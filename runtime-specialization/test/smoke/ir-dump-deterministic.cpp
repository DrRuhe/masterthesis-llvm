// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t1.exe
// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t2.exe
// RUN: %t1.exe > %t1.out
// RUN: %t2.exe > %t2.out
// RUN: diff %t1.out %t2.out

// Verify that IRDumpingPass is deterministic: compiling the same TU twice
// produces byte-identical blobs. The blob stats (FunctionCount,
// InstructionCount, BitcodeSizeBytes) must be identical across both
// compilations, confirming US3 acceptance scenario 3.

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
    std::printf("InstructionCount: %zu\n", stats.InstructionCount);
    std::printf("BitcodeSizeBytes: %zu\n", stats.BitcodeSizeBytes);
    return 0;
}
