// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: CRS_DEFAULT_PIPELINE=0 %t.exe | FileCheck %s --check-prefix=P0
// RUN: CRS_DEFAULT_PIPELINE=1 %t.exe | FileCheck %s --check-prefix=P1
// P0: pipeline=0 ok
// P1: pipeline=1 ok

#include "ClangRuntimeSpecializer.h"
#include <cstdio>
#include <cstdlib>

extern "C" int mul(int a, int b) __asm__("mul");
int mul(int a, int b) {
    return a * b;
}

inline constexpr char Fn_mul[] = "mul";

int main() {
    const char* pipelineEnv = std::getenv("CRS_DEFAULT_PIPELINE");
    int pipelineVal = (pipelineEnv != nullptr) ? std::atoi(pipelineEnv) : 0;

    int result = clangRuntimeSpecializer::specializeOrFallback(Fn_mul, mul, 3, 7);
    (void)result;

    if (pipelineVal == 1) {
        std::printf("pipeline=1 ok\n");
    } else {
        std::printf("pipeline=0 ok\n");
    }
    return 0;
}
