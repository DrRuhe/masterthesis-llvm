// RUN: %clangxx -g -O0 -emit-llvm -c %s -o %t.bc
// RUN: opt --verify-debuginfo-preserve -load-pass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext -passes='runtime-specialization-IR-dumping' %t.bc -o %t.opt.bc
// RUN: %clangxx -g %t.opt.bc -o %t.exe
// RUN: %t.exe 1 | FileCheck %s --check-prefix=EXE




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

  // EXE: INFO: [callSpecialized] Specializing call to: A::getMod2
  // EXE: DEBUG: [callSpecialized] Arg Serialized to: @__specialization_global_specialized_instance
  // EXE-SAME: %class.A { i32 2 }

  // EXE: DEBUG: [IRTransform] Optimized specialized function IR:
  // EXE: entry:
  // EXE:   ret i32 0
  // EXE: }
  int r1 = clangRuntimeSpecializer::specializeMethodOrFallback<Fn_A_getMod2>(&A::getMod2, instance);

  // EXE: INFO: [callSpecialized] Specializing call to: A::add
  // EXE: DEBUG: [callSpecialized] Arg Serialized to: @__specialization_global_specialized_instance
  // EXE-SAME: %class.A { i32 2 }

  // EXE: DEBUG: [IRTransform] Optimized specialized function IR:
  // EXE: entry:
  // EXE:   ret i32 20
  // EXE: }
  int r2 = clangRuntimeSpecializer::specializeMethodOrFallback<Fn_A_add>(&A::add, instance,7, 11);


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





