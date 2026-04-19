// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

struct Data {
    int read_only;
    int mutable_field;
};

// This function will be marked as readonly by attribute inference
extern "C" __attribute__((pure)) int nested_read_only(__attribute__((noescape)) const int* p) {
    return *p;
}

extern "C" int process_data_robust(Data* d, int x) __asm__("process_data_robust");
int process_data_robust(Data* d, int x) {
    int val1 = d->read_only; // Should be invariant
    
    int dummy = nested_read_only(&d->read_only); // Should not invalidate
    
    d->mutable_field += x; // Should invalidate d->mutable_field but not d->read_only
    
    int val2 = d->read_only; // Should be invariant
    int val3 = d->mutable_field; // Should NOT be invariant
    
    return val1 + val2 + val3 + dummy;
}

inline constexpr char Fn_process_data_robust[] = "process_data_robust";

// CHECK: INFO: Specializing call to: process_data_robust
// CHECK: DEBUG: Optimized specialized function IR:
// CHECK: define {{.*}} i32 @specialized_wrapper_{{.*}}()
// CHECK: load i32, ptr inttoptr (i64 {{.*}} to ptr), align {{.*}}
// CHECK-NOT: !invariant.load

int main() {
    clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Debug);
    Data d = {10, 20};
    
    int res = clangRuntimeSpecializer::specializeFunctionOrFallback(Fn_process_data_robust, process_data_robust, &d, 5);
    
    std::printf("Result: %d\n", res);
    // CHECK: Result: 55
    
    return 0;
}
