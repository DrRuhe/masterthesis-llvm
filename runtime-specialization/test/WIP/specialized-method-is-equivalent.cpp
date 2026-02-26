// RUN: %clangxx -g -O0 -emit-llvm -c %s -o %t.bc
// RUN: opt --verify-debuginfo-preserve -load-pass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext -passes='runtime-specialization-IR-dumping' %t.bc -o %t.opt.bc
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

  bool operator==(const A& other) const {
    return value == other.value;
  }
  bool operator!=(const A& other) const {
    return !(*this == other);
  }

  int getMod2() const __asm__("A::getMod2") {
    int result = value % 2;
    return result;
  }

  int add(int a, int b) const __asm__("A::add")
  {
    int result = value + a + b;
    return result;
  }
};

inline constexpr char Fn_A_getMod2[] = "A::getMod2";
inline constexpr char Fn_A_add[] = "A::add";

int main(int argc, char** argv) {

  A instance(argc);

  clangRuntimeSpecializer::assertSpecializedMethodIsEquivalent<Fn_A_getMod2>(&A::getMod2, instance);
  clangRuntimeSpecializer::assertSpecializedMethodIsEquivalent<Fn_A_add>(&A::add, instance,7, 11);
  return 0;
}