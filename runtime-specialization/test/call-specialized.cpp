// REQUIRES: shell
// RUN: %clang++ -O0 -emit-llvm -c -I%S/../runtime/ClangRuntimeSpecializer %s -o %t.bc
// RUN: opt -load-pass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext -passes='runtime-specialization-IR-dumping' %t.bc -o %t.opt.bc
// RUN: %clang++ %t.opt.bc -o %t.exe %llvmshlibdir/libClangRuntimeSpecializer%shlibext -Wl,-rpath,%llvmshlibdir
// RUN: %t.exe 2>&1 | FileCheck %s

#include "ClangRuntimeSpecializer.h"

// CHECK: [ClangRuntimeSpecializer] RuntimeSpecializeableIR_ptr=0x{{[0-9a-fA-F]+}} RuntimeSpecializeableIR_len={{[1-9][0-9]*}}

class A {
  int value;

public:
  A(int val) : value(val) {}

  int getMod2() const {
    return value % 2;
  }

  int add(int a, int b) const {
    return value + a + b;
  }
};

int main(int argc, char** argv) {
  A instance(argc);

  int r1 = clangRuntimeSpecializer::call_specialized(&A::getMod2, instance);
  int r2 = clangRuntimeSpecializer::call_specialized(&A::add, instance, 7, 11);
  return r1 + r2;
}