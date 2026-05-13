// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --check-prefix=EXE --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

int computeFoo(int x) {
  return x * x + 1;
}

int main() {
  clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(
      clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Info);

  const int val = 7;

  // Path A: 0-arg lambda capturing val — internally routes through specializeLambdaImpl
  // (0-arg branch calls specializeOnlyImpl with &lambda as the single baked arg).
  auto lambda = [val]() -> int { return computeFoo(val); };
  auto specA = clangRuntimeSpecializer::specializeLambda<int>(lambda);

  // Path B: specializeOnly with function pointer — routes through specializeOnlyImpl directly.
  auto specB = clangRuntimeSpecializer::specializeOnly<int>(&computeFoo, val);

  int rA = specA();
  int rB = specB();

  // EXE: result_a=50
  printf("result_a=%d\n", rA);

  // EXE: result_b=50
  printf("result_b=%d\n", rB);

  if (rA != rB)
    printf("FAIL: A and B produced different results\n");

  if (rA != computeFoo(val))
    printf("FAIL: A result differs from direct call\n");

  // EXE-NOT: FAIL

  return 0;
}
