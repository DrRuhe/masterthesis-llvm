// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe 1 2>&1 | FileCheck %s --check-prefix=EXE --dump-input=always




#include "ClangRuntimeSpecializer.h"
#include <cstdio>

// PRE-DUMP-NOT: RuntimeSpecializeableIR_ptr
// POST-DUMP: RuntimeSpecializeableIR_ptr

class A {
public:
  int value;
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




  // EXE: INFO: [callSpecialized] Specializing call to: A::getMod2
  // EXE: DEBUG: [serializeValueToIR] Serializing value of type pointer or class
  // EXE: DEBUG: [callSpecialized] Arg Serialized to: ptr inttoptr (i64 {{[0-9]+}} to ptr)
  // EXE: DEBUG: [IRTransform] Optimized specialized function IR:
  // EXE: entry:
  // EXE:   ret i32 0
  // EXE: }
  // EXE: INFO: [assertSpecializedFunctionIsEquivalent] Successfully specialized A::getMod2! No differences could be observed.
  A instance(argc);
  A instance2(argc);
  auto comp = [&]() {
    if (instance.value != instance2.value) throw clangRuntimeSpecializer::ClangRuntimeSpecializerChangesBehaviorError("Results differ");
  };
  clangRuntimeSpecializer::assertSpecializedFunctionIsEquivalent(Fn_A_getMod2, &A::getMod2, std::tie(instance), std::tie(instance2), comp);

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
  // EXE: INFO: [assertSpecializedFunctionIsEquivalent] Successfully specialized A::add! No differences could be observed.

  A instance3(argc);
  A instance4(argc);
  int a1 = 7, b1 = 11, a2 = 7, b2 = 11;
  auto comp2 = [&]() {
    if (instance3.value != instance4.value || a1 != a2 || b1 != b2) throw clangRuntimeSpecializer::ClangRuntimeSpecializerChangesBehaviorError("Results differ");
  };
  clangRuntimeSpecializer::assertSpecializedFunctionIsEquivalent(Fn_A_add, &A::add, std::tie(instance3, a1, b1), std::tie(instance4, a2, b2), comp2);

  return 0;
}


