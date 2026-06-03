// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --check-prefix=DEFAULT
// RUN: CRS_DEFAULT_PIPELINE=0 %t.exe | FileCheck %s --check-prefix=P0
// RUN: CRS_DEFAULT_PIPELINE=1 %t.exe | FileCheck %s --check-prefix=P1
// RUN: CRS_DEFAULT_PIPELINE=2 %t.exe | FileCheck %s --check-prefix=P2
// DEFAULT: pipeline=2 ok
// P0: pipeline=0 ok
// P1: pipeline=1 ok
// P2: pipeline=2 ok

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

extern "C" int mul(int a, int b);
int mul(int a, int b) {
    return a * b;
}

int main() {
    auto defaultOpts = clangRuntimeSpecializer::ClangRuntimeSpecializer::Options::Default();
    auto rawOpts = clangRuntimeSpecializer::ClangRuntimeSpecializer::Options{};

    int result = clangRuntimeSpecializer::specializeOrFallback(mul, 3, 7);
    (void)result;

    if (defaultOpts.OptimizationPipelineToUse == 0) {
        std::printf("pipeline=0 ok\n");
    } else if (defaultOpts.OptimizationPipelineToUse == 1) {
        std::printf("pipeline=1 ok\n");
    } else if (defaultOpts.OptimizationPipelineToUse == 2 &&
               rawOpts.OptimizationPipelineToUse == 2) {
        std::printf("pipeline=2 ok\n");
    } else {
        std::printf("pipeline=%d raw=%d unexpected\n",
                    defaultOpts.OptimizationPipelineToUse,
                    rawOpts.OptimizationPipelineToUse);
    }
    return 0;
}
