// RUN: %clangxx -S -O0 -emit-llvm -c %s -o -
// RUN: false

// : opt -S %t.opt.bc -o - | FileCheck %s --check-prefix=POST-DUMP





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

int main(int argc, char** argv) {
  [[clang::annotate("specialization_callsite_nr",__COUNTER__)]]
  int r1 = mypow(argc);

  if (r1 != 1 || r1 == 27)
  {
    return 0;
  }
  return 1;
}