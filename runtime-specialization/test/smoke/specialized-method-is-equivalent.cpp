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


  int addAndSum(int x) {
	std::fprintf(stderr, "[addAndSum] was called!\n");
	value += x;
    return value;
  }

};

int main(int argc, char** argv) {

  A instance(12);
  A instance2(12);
  int x1 = argc, x2 = argc;
  auto comp = [&]() {
    if (instance != instance2) throw clangRuntimeSpecializer::ClangRuntimeSpecializerChangesBehaviorError("Results differ");
  };

  // Use lambda-based equivalence testing since method pointers aren't supported by the new API
  auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
  using R = int;
  R ResOrig = instance.addAndSum(x1);
  auto lambdaSpec = [&instance2, x2]() mutable -> R { return instance2.addAndSum(x2); };
  auto spec = RS->specializeLambda<R>(lambdaSpec);
  R ResSpec = spec();
  if (ResOrig != ResSpec)
    throw clangRuntimeSpecializer::ClangRuntimeSpecializerChangesBehaviorError("Comparison failed: return values differ");
  comp();

  return 0;
}
