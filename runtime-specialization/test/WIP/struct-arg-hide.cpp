// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe 1 | FileCheck %s --check-prefix=EXE --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

struct MyStruct {
    int x;
    int y;
    int z;
    int w;
    int a;
};

extern "C" int process_struct(MyStruct s) __asm__("process_struct");
int process_struct(MyStruct s) {
    return s.x + s.y + s.z + s.w + s.a;
}

inline constexpr char Fn_process_struct[] = "process_struct";

__attribute__((noinline)) MyStruct* hide(MyStruct* s) {
    return s;
}

int main(int argc, char** argv) {
    clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Debug);
    MyStruct a = { argc, 10, 20, 30, 40 };
    MyStruct* hidden_a = hide(&a);

    MyStruct b = { argc, 10, 20, 30, 40 };
    MyStruct* hidden_b = hide(&b);


    // EXE: define noundef i32 @specialized_wrapper
    // EXE: entry:
    // EXE:   ret i32 102
    auto comp = [&]() {}; // no comparison needed, as no changes done anyways.
    clangRuntimeSpecializer::assertSpecializedFunctionIsEquivalent<Fn_process_struct>(process_struct, std::tie(*hidden_a), std::tie(*hidden_b), comp);
    return 0;
}




