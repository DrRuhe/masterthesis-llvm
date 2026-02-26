// RUN: %clangxx -S -O0 -emit-llvm -c %s -o -
// RUN: false

// THIS IS A WIP TEST TO CHECK HOW AN ATTRIBUTE IS REPRESENTED IN IR, TO SEE:
// is it possible to obtain the callsite by annotating it and then querying the IR for the annotation.
// For that the annotation is passed a counter, ideally, there is a wrapping macro that invokes the counter
// macro and annotates the function call with the counter while passing it to the function itself, so that the
// function knows which annotation UID to look for.




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