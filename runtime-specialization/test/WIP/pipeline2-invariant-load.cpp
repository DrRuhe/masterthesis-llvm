// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: CRS_DEFAULT_PIPELINE=2 %t.exe | FileCheck %s --check-prefix=EXE --dump-input=always

// Verify that Pipeline 2 (JIT-IPSCCP) resolves invariant loads from
// host-resident read-only memory and propagates them as constants.

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

struct Data {
    int read_only;
    int mutable_field;
};

extern "C" int test_invariant_load(Data* d);
int test_invariant_load(Data* d) {
    return d->read_only;
}

int main() {
    clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(
        clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Debug);
    Data d = {10, 20};

    int res = clangRuntimeSpecializer::specializeOrFallback(test_invariant_load, &d);

    // EXE: DEBUG: Optimized specialized function IR:
    // EXE-NOT: invariant.load
    // EXE: Result: 10
    std::printf("Result: %d\n", res);
    return 0;
}
