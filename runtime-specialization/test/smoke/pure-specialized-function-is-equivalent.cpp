// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe 2 | FileCheck %s --check-prefix=EXE --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>


/// computes 3^x
extern "C" int mypow(int x);
int mypow(int x)
{
  int result = 1;
  for (int i = 0; i < x; i++)
  {
    result = result * 3;
  }

  return result;
}


int main(int argc, char** argv)
{
  clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Debug);

  // EXE: INFO: Specializing call to: mypow
  // EXE: DEBUG: Serializing value of type i32
  // EXE: DEBUG: Arg Serialized to: i32 2
  // EXE: DEBUG: Optimized specialized function IR:
  // EXE: define noundef i32 @specialized_wrapper
  // EXE: entry:
  // EXE:   ret i32 9
  // EXE: }
  // EXE: INFO: Successfully specialized! No differences could be observed.
  int argc_copy = argc;
  auto comp = [&]() {
      if (argc != argc_copy) throw clangRuntimeSpecializer::ClangRuntimeSpecializerChangesBehaviorError("Arguments differ");
  };
  clangRuntimeSpecializer::assertSpecializedIsEquivalent(mypow, std::tie(argc), std::tie(argc_copy), comp);

  return 0;
}


