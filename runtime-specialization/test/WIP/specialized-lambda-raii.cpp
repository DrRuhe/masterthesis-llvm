// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --check-prefix=EXE --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

struct Ctx { int val; };

// Simple identity kernel: first param = closure ptr (Ctx*); second = int input.
extern "C" int identity(const Ctx* ctx, int x) __asm__("identity");
int identity(const Ctx* ctx, int x) {
  (void)ctx;
  return x;
}

int main() {
  clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(
      clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Error);

  int sum = 0;
  for (int i = 0; i < 100; ++i) {
    Ctx ctx{i};
    auto lambda = [ctx](int x) -> int {
      return identity(&ctx, x);
    };
    // Construct SpecializedLambda, call it, then let it go out of scope (RAII).
    auto spec = clangRuntimeSpecializer::specializeLambda<int>("identity", lambda);
    sum += spec(i);
    // spec destroyed here → JITDylib released
  }

  (void)sum;
  // EXE: RAII loop done
  printf("RAII loop done\n");

  return 0;
}
