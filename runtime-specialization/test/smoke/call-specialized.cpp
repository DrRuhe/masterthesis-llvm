// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
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

  std::fprintf(stderr, "[add] return value: %d\n", result);
  return result;
}


inline constexpr char Fn_mypow[] = "mypow";

int main(int argc, char** argv)
{
  clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Debug);

  int r1 = clangRuntimeSpecializer::specializeOrFallback(Fn_mypow, mypow, argc);


  if (r1 != 1 || r1 == 27)
  {
    return 0;
  }
  return 1;
}