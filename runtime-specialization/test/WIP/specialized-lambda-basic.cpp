// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --check-prefix=EXE --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

// Kernel: first param = pointer to lambda closure (Ctx struct); remaining = explicit args.
// The lambda [ctx](int x, bool negate) captures Ctx by value; closure layout = {Ctx ctx}.
// &lambda == pointer to that Ctx copy, so kernel receives Ctx*.
struct Ctx { int base; };

extern "C" int add_with_ctx(const Ctx* ctx, int x, bool negate) __asm__("add_with_ctx");
int add_with_ctx(const Ctx* ctx, int x, bool negate) {
  return negate ? (ctx->base - x) : (ctx->base + x);
}

int main() {
  clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(
      clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Info);

  Ctx ctx{10};
  // Capture ctx by value; closure = {Ctx ctx_copy}; &lambda == Ctx*.
  auto lambda = [ctx](int x, bool negate) -> int {
    return add_with_ctx(&ctx, x, negate);
  };

  // EXE: INFO: Specializing lambda call to: add_with_ctx
  auto spec = clangRuntimeSpecializer::specializeLambda<int>("add_with_ctx", lambda);

  // EXE: result_false=15
  printf("result_false=%d\n", spec(5, false));

  // EXE: result_true=5
  printf("result_true=%d\n", spec(5, true));

  // Verify equivalence vs direct lambda calls.
  if (spec(5, false) != lambda(5, false))
    printf("FAIL: result mismatch for (5, false)\n");
  if (spec(5, true) != lambda(5, true))
    printf("FAIL: result mismatch for (5, true)\n");
  if (spec(0, false) != lambda(0, false))
    printf("FAIL: result mismatch for (0, false)\n");

  // EXE-NOT: FAIL

  return 0;
}
