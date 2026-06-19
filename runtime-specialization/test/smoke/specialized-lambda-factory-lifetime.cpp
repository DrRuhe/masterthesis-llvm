// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --check-prefix=EXE --dump-input=always
// RUN: CRS_DEFAULT_PIPELINE=1 %t.exe | FileCheck %s --check-prefix=EXE --dump-input=always
// RUN: %clangxx -g -O3 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.o3.exe
// RUN: %t.o3.exe | FileCheck %s --check-prefix=EXE --dump-input=always
// RUN: CRS_DEFAULT_PIPELINE=1 %t.o3.exe | FileCheck %s --check-prefix=EXE --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>
#include <utility>

struct ScalarCtx {
  int Base;
  int Scale;
};

extern "C" int apply_scalar_ctx(const ScalarCtx *Ctx, int X)
    __asm__("apply_scalar_ctx");
int apply_scalar_ctx(const ScalarCtx *Ctx, int X) {
  return Ctx->Base + Ctx->Scale * X;
}

struct SmallHelper {
  int Bias;
  int Factor;

  int apply(int X) const { return Bias + Factor * X; }
};

extern "C" int apply_helper(const SmallHelper *Helper, int X)
    __asm__("apply_helper");
int apply_helper(const SmallHelper *Helper, int X) { return Helper->apply(X); }

clangRuntimeSpecializer::SpecializedLambda<int, int>
makeSpecializedScalarFactory() {
  ScalarCtx Ctx{10, 3};
  auto Lam = [Ctx](int X) -> int { return apply_scalar_ctx(&Ctx, X); };
  return clangRuntimeSpecializer::specializeLambda<int>(Lam);
}

clangRuntimeSpecializer::SpecializedLambda<int, int>
makeSpecializedHelperFactory() {
  SmallHelper Helper{7, 5};
  auto Lam = [Helper](int X) -> int { return apply_helper(&Helper, X); };
  return clangRuntimeSpecializer::specializeLambda<int>(Lam);
}

clangRuntimeSpecializer::SpecializedLambda<int, int>
makeSpecializedMovedFactory() {
  SmallHelper Helper{4, 6};
  auto Lam = [Helper](int X) -> int { return apply_helper(&Helper, X); };
  return clangRuntimeSpecializer::specializeLambda<int>(std::move(Lam));
}

clangRuntimeSpecializer::SpecializedLambda<int>
makeSpecializedZeroArgFactory() {
  SmallHelper Helper{9, 4};
  auto Lam = [Helper]() -> int { return apply_helper(&Helper, 2); };
  return clangRuntimeSpecializer::specializeLambda<int>(Lam);
}

int main() {
  clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(
      clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Error);

  auto ScalarSpec = makeSpecializedScalarFactory();
  auto HelperSpec = makeSpecializedHelperFactory();
  auto MovedSpec = makeSpecializedMovedFactory();
  auto ZeroArgSpec = makeSpecializedZeroArgFactory();

  std::printf("scalar=%d\n", ScalarSpec(4));
  std::printf("helper=%d\n", HelperSpec(3));
  std::printf("moved=%d\n", MovedSpec(2));
  std::printf("zero=%d\n", ZeroArgSpec());

  if (ScalarSpec(4) != 22)
    std::printf("FAIL: scalar lifetime mismatch\n");
  if (HelperSpec(3) != 22)
    std::printf("FAIL: helper lifetime mismatch\n");
  if (MovedSpec(2) != 16)
    std::printf("FAIL: moved lifetime mismatch\n");
  if (ZeroArgSpec() != 17)
    std::printf("FAIL: zero-arg lifetime mismatch\n");

  // EXE: scalar=22
  // EXE: helper=22
  // EXE: moved=16
  // EXE: zero=17
  // EXE-NOT: FAIL
  return 0;
}
