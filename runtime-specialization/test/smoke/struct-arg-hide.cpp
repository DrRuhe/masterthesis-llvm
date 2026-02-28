// RUN: %clangxx -g -O0 -emit-llvm -c %s -o %t.bc
// RUN: opt --verify-debuginfo-preserve -load-pass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext -passes='runtime-specialization-IR-dumping' %t.bc -o %t.opt.bc
// RUN: %clangxx -g %t.opt.bc -o %t.exe
// RUN: %t.exe 1 | FileCheck %s --check-prefix=EXE

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
    MyStruct s = { argc, 10, 20, 30, 40 };
    MyStruct* hidden_s = hide(&s);
    
    clangRuntimeSpecializer::assertSpecializedMethodIsEquivalent<Fn_process_struct>(process_struct, *hidden_s);
    return 0;
}




// EXE: [ClangRuntimeSpecializer] Optimized IR for specialized_wrapper_1
// EXE: define noundef i32 @specialized_wrapper_1
// EXE: entry:
// EXE:   ret i32 102