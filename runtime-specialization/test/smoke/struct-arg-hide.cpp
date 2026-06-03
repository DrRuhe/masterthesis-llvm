// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: CRS_DEFAULT_PIPELINE=0 %t.exe 1 | FileCheck %s --check-prefix=EXE --dump-input=always
// RUN: CRS_DEFAULT_PIPELINE=1 %t.exe 1 | FileCheck %s --check-prefix=P1-EXE --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

struct MyStruct {
    int x;
    int y;
    int z;
    int w;
    int a;
};

extern "C" int process_struct(MyStruct s);
int process_struct(MyStruct s) {
    return s.x + s.y + s.z + s.w + s.a;
}

__attribute__((noinline)) MyStruct* hide(MyStruct* s) {
    return s;
}

int main(int argc, char** argv) {
    clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Debug);
    MyStruct a = { argc, 10, 20, 30, 40 };
    MyStruct* hidden_a = hide(&a);

    MyStruct b = { argc, 10, 20, 30, 40 };
    MyStruct* hidden_b = hide(&b);


    // EXE: define {{.*}}i32 @specialized_wrapper
    // EXE: entry:
    // EXE:   ret i32 102
    //
    // P1 produces the same result but without P0's inferred noundef return
    // attribute (P1 pipeline doesn't run the full O3 attribute-inference
    // chain — see [[feedback-pipeline1-budget-inlining]]).
    // P1-EXE: define i32 @specialized_wrapper
    // P1-EXE: entry:
    // P1-EXE:   ret i32 102
    auto comp = [&]() {}; // no comparison needed, as no changes done anyways.
    clangRuntimeSpecializer::assertSpecializedIsEquivalent(process_struct, std::tie(*hidden_a), std::tie(*hidden_b), comp);
    return 0;
}


