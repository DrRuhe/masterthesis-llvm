// RUN: %clangxx -g -O0 -emit-llvm -c %s -o %t.bc
// RUN: opt --verify-debuginfo-preserve -load-pass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext -passes='runtime-specialization-IR-dumping' %t.bc -o %t.opt.bc
// RUN: %clangxx -g %t.opt.bc -o %t.exe
// RUN: %t.exe 1




#include "ClangRuntimeSpecializer.h"
#include <cstdio>


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


  int addAndSum(int x) __asm__("A::addAndSum") {
	std::fprintf(stderr, "[addAndSum] was called!\n");
	value += x;
    return value;
  }

};

inline constexpr char Fn_A_addAndSum[] = "A::addAndSum";

int main(int argc, char** argv) {

  A instance(12);

  clangRuntimeSpecializer::assertSpecializedMethodIsEquivalent<Fn_A_addAndSum>(&A::addAndSum, instance,argc);
  return 0;
}