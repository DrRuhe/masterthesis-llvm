// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --check-prefix=EXE --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>
#include <vector>
#include <tuple>

struct Ctx { int base; };

extern "C" int add_with_ctx2(const Ctx* ctx, int x, bool negate) __asm__("add_with_ctx2");
int add_with_ctx2(const Ctx* ctx, int x, bool negate) {
  return negate ? (ctx->base - x) : (ctx->base + x);
}

int main() {
  clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(
      clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Info);

  Ctx ctx{10};
  auto lambda_correct = [ctx](int x, bool negate) -> int {
    return add_with_ctx2(&ctx, x, negate);
  };

  using InputTuple = std::tuple<int, bool>;
  std::vector<InputTuple> inputs = {
      {5, false},
      {5, true},
      {0, false},
  };

  // EXE: INFO: assertSpecializedLambdaIsEquivalent passed
  clangRuntimeSpecializer::assertSpecializedLambdaIsEquivalent<int>(lambda_correct, inputs);

  // EXE-NOT: FAIL

  return 0;
}
