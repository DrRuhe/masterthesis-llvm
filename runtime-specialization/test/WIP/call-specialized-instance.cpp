// RUN: %clangxx -g -O0 -emit-llvm -c %s -o %t.bc
// RUN: opt -S %t.bc --strip-debug -o -
// RUN: opt --verify-debuginfo-preserve -load-pass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext -passes='runtime-specialization-IR-dumping' %t.bc -o %t.opt.bc
// RUN: %clangxx -g %t.opt.bc -o %t.exe
// RUN: %t.exe 1




#include "ClangRuntimeSpecializer.h"
#include <cstdio>

// PRE-DUMP-NOT: RuntimeSpecializeableIR_ptr
// POST-DUMP: RuntimeSpecializeableIR_ptr

class A {
  int value;

public:
  A(int val) : value(val) {}

  int getMod2() const __asm__("A::getMod2") {
    int result = value % 2;
    std::fprintf(stderr, "[getMod2] return value: %d\n", result);
    return result;
  }

  int add(int a, int b) const __asm__("A::add")
  {
    std::fprintf(stderr, "[add] args: a=%d, b=%d, value=%d\n", a, b, value);
    int result = value + a + b;
    std::fprintf(stderr, "[add] return value: %d\n", result);
    return result;
  }
};

int main(int argc, char** argv) {

  A instance(argc);

  auto* RuntimeSpecializer = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
  if (!RuntimeSpecializer)
  {
    throw std::runtime_error("[ClangRuntimeSpecializer] could not init! ");
  }
  // TODO Could the reason why getMod2 is not found be that its removed by dead code analysis? YES
  // We can add a FileCheck
  int r1 = RuntimeSpecializer->template call_specialized<int>("A::getMod2", instance);
  int r2 = RuntimeSpecializer->template call_specialized<int>("A::add", instance, 7, 11);

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