// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

struct Data {
    int read_only;
    int mutable_field;
};

extern "C" int test_invariant_load(Data* d) __asm__("test_invariant_load");
int test_invariant_load(Data* d) {
    // This load should be marked invariant and then replaced by a constant (10)
    return d->read_only;
}

inline constexpr char Fn_test_invariant_load[] = "test_invariant_load";

int main() {
    clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Debug);
    Data d = {10, 20};
    
    int res = clangRuntimeSpecializer::specializeFunctionOrFallback<Fn_test_invariant_load>(test_invariant_load, &d);
    
    // Check that the specialized IR has been optimized to a single return of constant 10
    // CHECK: Optimized specialized function IR:
    // CHECK: define {{.*}} i32 @specialized_wrapper_{{.*}}()
    // CHECK-NEXT: entry:
    // CHECK-NEXT: ret i32 10

    std::printf("Result: %d\n", res);
    // CHECK: Result: 10
    
    return 0;
}
