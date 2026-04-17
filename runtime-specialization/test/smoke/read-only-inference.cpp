// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

struct Data {
    int read_only;
    int mutable_field;
};

extern "C" int process_data(Data* d, int x) __asm__("process_data");
int process_data(Data* d, int x) {
    d->mutable_field += x;
    return d->read_only + d->mutable_field;
}

inline constexpr char Fn_process_data[] = "process_data";

int main() {
    clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Debug);
    Data d = {10, 20};
    
    // First call: d.read_only should be inferred as read-only.
    // d.mutable_field is clearly modified.
    int res = clangRuntimeSpecializer::specializeFunctionOrFallback(Fn_process_data, process_data, &d, 5);
    
    std::printf("Result: %d, mutable_field: %d\n", res, d.mutable_field);
    
    // CHECK: Result: 35, mutable_field: 25
    if (res != 35 || d.mutable_field != 25) {
        return 1;
    }
    
    return 0;
}
