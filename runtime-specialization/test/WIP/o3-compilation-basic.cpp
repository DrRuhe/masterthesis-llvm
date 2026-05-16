// RUN: %clangxx -g -O3 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.o3.exe
// RUN: %t.o3.exe | FileCheck %s --check-prefix=EXE --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

struct Ctx { int multiplier; };

extern "C" int multiply(const Ctx* ctx, int x) __asm__("multiply");
int multiply(const Ctx* ctx, int x) {
  return ctx->multiplier * x;
}

int main() {
  clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(
      clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Info);

  Ctx ctx{7};
  auto lambda = [ctx](int x) -> int {
    return multiply(&ctx, x);
  };

  // EXE: INFO: Specializing lambda call to: _Z
  auto spec = clangRuntimeSpecializer::specializeLambda<int>(lambda);

  // EXE: result=35
  printf("result=%d\n", spec(5));

  if (spec(5) != lambda(5))
    printf("FAIL: result mismatch\n");

  // EXE-NOT: FAIL

  return 0;
}
