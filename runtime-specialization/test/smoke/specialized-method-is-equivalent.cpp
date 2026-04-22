// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
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
  A instance2(12);
  int x1 = argc, x2 = argc;
  auto comp = [&]() {
    if (instance != instance2) throw clangRuntimeSpecializer::ClangRuntimeSpecializerChangesBehaviorError("Results differ");
  };
  clangRuntimeSpecializer::assertSpecializedIsEquivalent(Fn_A_addAndSum, &A::addAndSum, std::tie(instance, x1), std::tie(instance2, x2), comp);
  return 0;
}