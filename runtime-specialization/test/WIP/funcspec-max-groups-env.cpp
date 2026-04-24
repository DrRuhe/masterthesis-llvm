// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: CRS_DEFAULT_FUNC_SPEC_MAX_GROUPS=2 CRS_DEFAULT_PIPELINE=1 %t.exe | FileCheck %s --check-prefix=OK
// RUN: CRS_DEFAULT_FUNC_SPEC_MAX_GROUPS=5 CRS_DEFAULT_PIPELINE=0 %t.exe 2>&1 | FileCheck %s --check-prefix=WARN2
// OK: funcspec ok
// WARN2: WARNING
// WARN2: FuncSpecMaxGroups

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

extern "C" int triple(int x) __asm__("triple");
int triple(int x) {
    return x * 3;
}

inline constexpr char Fn_triple[] = "triple";

int main() {
    int result = clangRuntimeSpecializer::specializeOrFallback(Fn_triple, triple, 4);
    (void)result;
    std::printf("funcspec ok\n");
    return 0;
}
