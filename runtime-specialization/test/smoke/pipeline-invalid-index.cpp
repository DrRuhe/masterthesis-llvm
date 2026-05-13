// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: CRS_DEFAULT_PIPELINE=99 %t.exe 2>&1 | FileCheck %s --check-prefix=WARN
// WARN: WARNING
// WARN: pipeline
// WARN: done

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

extern "C" int addone(int x);
int addone(int x) {
    return x + 1;
}

int main() {
    int result = clangRuntimeSpecializer::specializeOrFallback(addone, 42);
    (void)result;
    std::printf("done\n");
    return 0;
}
