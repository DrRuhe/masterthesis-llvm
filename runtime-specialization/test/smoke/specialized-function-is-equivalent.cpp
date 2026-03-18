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

  // EXE: INFO: [callSpecialized] Specializing call to: mypow
  // EXE: DEBUG: [serializeValueToIR] Serializing value of type i32
  // EXE: DEBUG: [callSpecialized] Arg Serialized to: i32 2
  // EXE: DEBUG: [IRTransform] Optimized specialized function IR:
  // EXE: define void @specialized_wrapper
  // EXE: entry:
  // EXE:   store i32 9, ptr inttoptr
  // EXE:   ret void
  // EXE: }
  // EXE: INFO: [assertSpecializedFunctionIsEquivalent] Successfully specialized mypow! No differences could be observed.
  int result = 0;
  clangRuntimeSpecializer::assertSpecializedFunctionIsEquivalent<Fn_mypow>(mypow, &result, argc);

  return 0;
}


