// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe 1 | FileCheck %s --check-prefix=EXE --dump-input=always
// RUN: %clangxx -g -O3 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.o3.exe
// RUN: %t.o3.exe 1 | FileCheck %s --check-prefix=EXE --dump-input=always




#include "ClangRuntimeSpecializer.h"
#include <cstdio>


class A {
  int value;

public:
  A(int val) : value(val) {}

  int getMod2() const {
    int result = value % 2;
    return result;
  }

  int add(int a, int b) const
  {
    int result = value + a + b;
    return result;
  }
};

int main(int argc, char** argv) {
  clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Debug);


  A instance(argc);

  // EXE: DEBUG: Optimized specialized function IR:
  // EXE: entry:
  // EXE:   ret i32 0
  // EXE: }
  int r1 = [&]() -> int {
    auto lambda1 = [&instance]() -> int { return instance.getMod2(); };
    auto spec = clangRuntimeSpecializer::ClangRuntimeSpecializer::init()->specializeLambda<int>(lambda1);
    return spec();
  }();

  // EXE: DEBUG: Optimized specialized function IR:
  // EXE: entry:
  // EXE:   ret i32 20
  // EXE: }
  int r2 = [&]() -> int {
    auto lambda2 = [&instance]() -> int { return instance.add(7, 11); };
    auto spec = clangRuntimeSpecializer::ClangRuntimeSpecializer::init()->specializeLambda<int>(lambda2);
    return spec();
  }();


  int r3 = instance.getMod2();
  int r4 = instance.add(7,11);

  if (r1 != r3 || r2 != r4)
  {
    return 1;
  } else
  {
    return 0;
  }
}
