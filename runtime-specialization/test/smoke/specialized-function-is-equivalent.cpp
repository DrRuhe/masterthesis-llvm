// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe 2 | FileCheck %s --check-prefix=EXE --dump-input=always




#include "ClangRuntimeSpecializer.h"
#include <cstdio>


/// computes 3^x
extern "C" void mypow(int* result, int x)
{
  *result =  1;
  for (int i = 0; i < x; i++)
  {
    *result = (*result) * 3;
  }
}


inline constexpr char Fn_mypow[] = "mypow";

int main(int argc, char** argv)
{
  clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Debug);

  // EXE: INFO: Specializing call to: mypow
  // EXE: DEBUG: Serializing value of type i32
  // EXE: DEBUG: Arg Serialized to: i32 2
  // EXE: DEBUG: Optimized specialized function IR:
  // EXE: define void @specialized_wrapper
  // EXE: entry:
  // EXE:   store i32 9, ptr inttoptr
  // EXE:   ret void
  // EXE: }
  // EXE: INFO: Successfully specialized mypow! No differences could be observed.
  int result1 = 0;
  int result2 = 0;
  int x1 = argc;
  int x2 = argc;
  auto comp = [&]() {
      if (result1 != result2) throw clangRuntimeSpecializer::ClangRuntimeSpecializerChangesBehaviorError("Results differ");
  };
  int* p1 = &result1;
  int* p2 = &result2;
  clangRuntimeSpecializer::assertSpecializedFunctionIsEquivalent(Fn_mypow, mypow, std::tie(p1, x1), std::tie(p2, x2), comp);

  return 0;
}


