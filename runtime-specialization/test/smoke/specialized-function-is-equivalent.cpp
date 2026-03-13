// RUN: %clangxx -g -O0 -emit-llvm -c %s -o %t.bc
// Test that there is no RuntimeSpecializableIR_ptr yet:
// RUN: opt -S %t.bc -o - | FileCheck %s --check-prefix=PRE-DUMP
// RUN: opt --verify-debuginfo-preserve -load-pass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext -passes='runtime-specialization-IR-dumping' %t.bc -o %t.opt.bc
// Test that there is a RuntimeSpecializableIR_ptr now:
// RUN: opt -S %t.opt.bc -o - | FileCheck %s --check-prefix=POST-DUMP
// RUN: %clangxx -g %t.opt.bc -o %t.exe
// RUN: %t.exe 2 | FileCheck %s --check-prefix=EXE --dump-input=always




#include "ClangRuntimeSpecializer.h"
#include <cstdio>

// PRE-DUMP-NOT: RuntimeSpecializeableIR_ptr
// POST-DUMP: RuntimeSpecializeableIR_ptr


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


