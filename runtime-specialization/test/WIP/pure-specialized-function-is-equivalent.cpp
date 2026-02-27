// RUN: %clangxx -g -O0 -emit-llvm -c %s -o %t.bc
// Test that there is no RuntimeSpecializableIR_ptr yet:
// RUN: opt -S %t.bc -o - | FileCheck %s --check-prefix=PRE-DUMP
// RUN: opt --verify-debuginfo-preserve -load-pass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext -passes='runtime-specialization-IR-dumping' %t.bc -o %t.opt.bc
// Test that there is a RuntimeSpecializableIR_ptr now:
// RUN: opt -S %t.opt.bc -o - | FileCheck %s --check-prefix=POST-DUMP
// RUN: %clangxx -g %t.opt.bc -o %t.exe
// RUN: %t.exe 2




#include "ClangRuntimeSpecializer.h"
#include <cstdio>

// PRE-DUMP-NOT: RuntimeSpecializeableIR_ptr
// POST-DUMP: RuntimeSpecializeableIR_ptr


/// computes 3^x
extern "C" int mypow(int x) __asm__("mypow");
int mypow(int x)
{
  int result = 1;
  for (int i = 0; i < x; i++)
  {
    result = result * 3;
  }

  return result;
}


inline constexpr char Fn_mypow[] = "mypow";

int main(int argc, char** argv)
{
  clangRuntimeSpecializer::assertSpecializedFunctionIsEquivalent<Fn_mypow>(mypow, argc);

  return 0;
}