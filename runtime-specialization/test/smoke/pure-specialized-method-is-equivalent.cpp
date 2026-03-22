// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe 1 2>&1 | FileCheck %s --check-prefix=EXE --dump-input=always




#include "ClangRuntimeSpecializer.h"
#include <cstdio>

// PRE-DUMP-NOT: RuntimeSpecializeableIR_ptr
// POST-DUMP: RuntimeSpecializeableIR_ptr

class A {
  int value;

public:
  A(int val) : value(val) {}

  bool operator==(const A& other) const {
    return value == other.value;
  }
  bool operator!=(const A& other) const {
    return !(*this == other);
  }

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
  // EXE: DEBUG: [serializeValueToIR] Serializing value of type pointer or class
  // EXE: DEBUG: [callSpecialized] Arg Serialized to: ptr inttoptr (i64 {{[0-9]+}} to ptr)
  // EXE: DEBUG: [IRTransform] Optimized specialized function IR:
  // EXE: entry:
  // EXE:   ret i32 0
  // EXE: }
  // EXE: INFO: [assertSpecializedMethodIsEquivalent] Successfully specialized A::getMod2! No differences could be observed.
  clangRuntimeSpecializer::assertSpecializedMethodIsEquivalent<Fn_A_getMod2>(&A::getMod2, instance);

  // EXE: INFO: [callSpecialized] Specializing call to: A::add
  // EXE: DEBUG: [serializeValueToIR] Serializing value of type pointer or class
  // EXE: DEBUG: [callSpecialized] Arg Serialized to: ptr inttoptr (i64 {{[0-9]+}} to ptr)
  // EXE: DEBUG: [serializeValueToIR] Serializing value of type i32
  // EXE: DEBUG: [callSpecialized] Arg Serialized to: i32 7
  // EXE: DEBUG: [serializeValueToIR] Serializing value of type i32
  // EXE: DEBUG: [callSpecialized] Arg Serialized to: i32 11
  // EXE: DEBUG: [IRTransform] Optimized specialized function IR:
  // EXE: entry:
  // EXE:   ret i32 20
  // EXE: }
  // EXE: INFO: [assertSpecializedMethodIsEquivalent] Successfully specialized A::add! No differences could be observed.
  clangRuntimeSpecializer::assertSpecializedMethodIsEquivalent<Fn_A_add>(&A::add, instance,7, 11);
  return 0;
}


