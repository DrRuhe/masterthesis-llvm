// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --check-prefix=EXE --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>
#include <vector>
#include <tuple>

struct Ctx { int base; };

// Positive path: correct kernel.
extern "C" int add_with_ctx2(const Ctx* ctx, int x, bool negate) __asm__("add_with_ctx2");
int add_with_ctx2(const Ctx* ctx, int x, bool negate) {
  return negate ? (ctx->base - x) : (ctx->base + x);
}

// Wrong kernel: same first-arg layout but computes off-by-one.
// The lambda in the negative path calls add_with_ctx2 (correct), but we ask
// assertSpecializedLambdaIsEquivalent to specialize wrong_kernel instead.
// The JIT specializes wrong_kernel(baked_ctx, x, negate) → mismatches lambda output.
extern "C" int wrong_kernel(const Ctx* ctx, int x, bool negate) __asm__("wrong_kernel");
int wrong_kernel(const Ctx* ctx, int x, bool negate) {
  return negate ? (ctx->base - x) : (ctx->base + x + 1); // off-by-one
}

int main() {
  clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(
      clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Info);

  Ctx ctx{10};

  // --- Positive path: lambda calls add_with_ctx2; we specialize add_with_ctx2. ---
  auto lambda_correct = [ctx](int x, bool negate) -> int {
    return add_with_ctx2(&ctx, x, negate);
  };

  using InputTuple = std::tuple<int, bool>;
  std::vector<InputTuple> inputs = {
      {5, false},
      {5, true},
      {0, false},
  };

  // EXE: INFO: assertSpecializedLambdaIsEquivalent passed for: add_with_ctx2
  clangRuntimeSpecializer::assertSpecializedLambdaIsEquivalent<int>(
      "add_with_ctx2", lambda_correct, inputs);

  // --- Negative path: lambda calls add_with_ctx2 (correct), but we ask to specialize
  // wrong_kernel. The JIT specializes wrong_kernel, which returns off-by-one results —
  // mismatching what the lambda computes via add_with_ctx2. Exception must be thrown. ---
  auto lambda_neg = [ctx](int x, bool negate) -> int {
    return add_with_ctx2(&ctx, x, negate); // correct computation
  };

  try {
    clangRuntimeSpecializer::assertSpecializedLambdaIsEquivalent<int>(
        "wrong_kernel", lambda_neg, inputs);
    printf("FAIL: expected exception not thrown\n");
  } catch (const clangRuntimeSpecializer::ClangRuntimeSpecializerChangesBehaviorError&) {
    // EXE: negative-path-caught
    printf("negative-path-caught\n");
  }

  // EXE-NOT: FAIL

  return 0;
}
