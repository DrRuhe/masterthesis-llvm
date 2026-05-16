// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe 1 2>&1 | FileCheck %s --check-prefix=EXE --dump-input=always
// RUN: %clangxx -g -O3 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.o3.exe
// RUN: %t.o3.exe 1 2>&1 | FileCheck %s --check-prefix=EXE --dump-input=always




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

  int getMod2() const {
    int result = value % 2;
    return result;
  }

  int add(int a, int b) const
  {
    int result = value + a + b;
    return result;
  }
};

int main(int argc, char** argv) {

  clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Debug);

  A instance(argc);
  A instance2(argc);

  // EXE: INFO: Successfully specialized! No differences could be observed.
  {
    auto comp = [&]() {
      if (instance.value != instance2.value) throw clangRuntimeSpecializer::ClangRuntimeSpecializerChangesBehaviorError("Results differ");
    };
    auto normalArgs = std::tie(instance);
    auto specArgs   = std::tie(instance2);
    auto InvokeNormal = [](A& inst) { return inst.getMod2(); };
    using R = decltype(InvokeNormal(instance));
    R ResOrig = InvokeNormal(instance);
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    auto lambda1 = [&instance2]() -> R { return instance2.getMod2(); };
    auto spec = RS->specializeLambda<R>(lambda1);
    R ResSpec = spec();
    if (ResOrig != ResSpec)
      throw clangRuntimeSpecializer::ClangRuntimeSpecializerChangesBehaviorError("Comparison failed: return values differ");
    comp();
    CRS_LOG(Info, "Successfully specialized! No differences could be observed.");
  }

  // EXE: INFO: Successfully specialized! No differences could be observed.
  {
    A instance3(argc);
    A instance4(argc);
    int a1 = 7, b1 = 11, a2 = 7, b2 = 11;
    auto comp2 = [&]() {
      if (instance3.value != instance4.value || a1 != a2 || b1 != b2) throw clangRuntimeSpecializer::ClangRuntimeSpecializerChangesBehaviorError("Results differ");
    };
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    using R = int;
    R ResOrig = instance3.add(a1, b1);
    auto lambda2 = [&instance4, a2, b2]() -> R { return instance4.add(a2, b2); };
    auto spec = RS->specializeLambda<R>(lambda2);
    R ResSpec = spec();
    if (ResOrig != ResSpec)
      throw clangRuntimeSpecializer::ClangRuntimeSpecializerChangesBehaviorError("Comparison failed: return values differ");
    comp2();
    CRS_LOG(Info, "Successfully specialized! No differences could be observed.");
  }

  return 0;
}


