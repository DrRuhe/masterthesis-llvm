// RUN: %clangxx -O0 -g %s -o %t.without_dump.exe
// RUN: %t.without_dump.exe 1 2
// RUN: %clangxx -g -O0 -emit-llvm -c %s -o %t.bc
// Test that there is no RuntimeSpecializableIR_ptr yet:
// RUN: opt -S %t.bc -o - | FileCheck %s --check-prefix=PRE-DUMP
// RUN: opt --verify-debuginfo-preserve -load-pass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext -passes='runtime-specialization-IR-dumping' %t.bc -o %t.opt.bc
// Test that there is a RuntimeSpecializableIR_ptr now:
// RUN: opt -S %t.opt.bc -o - | FileCheck %s --check-prefix=POST-DUMP
// RUN: %clangxx -g %t.opt.bc -o %t.exe
// RUN: %t.exe 1




#include "ClangRuntimeSpecializer.h"
#include <cstdio>

// PRE-DUMP-NOT: RuntimeSpecializeableIR_ptr
// POST-DUMP: RuntimeSpecializeableIR_ptr

class A {
  int value;

public:
  A(int val) : value(val) {}

  int getMod2() const __asm__("A::getMod2") {
    int result = value % 2;
    std::fprintf(stderr, "[getMod2] return value: %d\n", result);
    return result;
  }

  int add(int a, int b) const __asm__("A::add")
  {
    std::fprintf(stderr, "[add] args: a=%d, b=%d, value=%d\n", a, b, value);
    int result = value + a + b;
    std::fprintf(stderr, "[add] return value: %d\n", result);
    return result;
  }
};

int main(int argc, char** argv) {

  A instance(argc);

  int r1 = SPECIALIZE_METHOD(A::getMod2, instance);
  int r2 = SPECIALIZE_METHOD(A::add, instance, 7, 11);

  if (r1 != 1)
  {
    return 0;
  } else if (r2 != 20)
  {
    return 0;
  }
  return 1;
}