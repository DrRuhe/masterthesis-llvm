// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe 1 | FileCheck %s --check-prefix=EXE --dump-input=always




#include "ClangRuntimeSpecializer.h"
#include <cstdio>


class A {
  int value;

public:
  A(int val) : value(val) {}

  int getMod2() const __asm__("A::getMod2") {
    int result = value % 2;
    return result;
  }

  int add(int a, int b) const __asm__("A::add")
  {
    int result = value + a + b;
    return result;
  }
};

inline constexpr char Fn_A_getMod2[] = "A::getMod2";
inline constexpr char Fn_A_add[] = "A::add";

int main(int argc, char** argv) {
  clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Debug);


  A instance(argc);

  // EXE: INFO: Specializing call to: A::getMod2
  // EXE: DEBUG: Serializing value of type pointer or class
  // EXE: DEBUG: Arg Serialized to: ptr inttoptr (i64 {{[0-9]+}} to ptr)

  // EXE: DEBUG: Optimized specialized function IR:
  // EXE: entry:
  // EXE:   ret i32 0
  // EXE: }
  int r1 = clangRuntimeSpecializer::specializeOrFallback(Fn_A_getMod2, &A::getMod2, instance);

  // EXE: INFO: Specializing call to: A::add
  // EXE: DEBUG: Serializing value of type pointer or class
  // EXE: DEBUG: Arg Serialized to: ptr inttoptr (i64 {{[0-9]+}} to ptr)
  // EXE: DEBUG: Serializing value of type i32
  // EXE: DEBUG: Arg Serialized to: i32 7
  // EXE: DEBUG: Serializing value of type i32
  // EXE: DEBUG: Arg Serialized to: i32 11

  // EXE: DEBUG: Optimized specialized function IR:
  // EXE: entry:
  // EXE:   ret i32 20
  // EXE: }
  int r2 = clangRuntimeSpecializer::specializeOrFallback(Fn_A_add, &A::add, instance,7, 11);


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





