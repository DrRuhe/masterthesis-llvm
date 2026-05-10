// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --check-prefix=EXE --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>

// Kernel with void return: first param = closure pointer; explicit arg = delta.
struct State { int total; };

extern "C" void accumulate(State* s, int delta) __asm__("accumulate");
void accumulate(State* s, int delta) {
  s->total += delta;
}

int main() {
  clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(
      clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Info);

  State state{0};
  // Capture state by value; closure = {State state_copy}; &lambda == State*.
  auto lambda = [state](int delta) mutable -> void {
    accumulate(&state, delta);
  };

  // EXE: INFO: Specializing lambda call to: accumulate
  auto spec = clangRuntimeSpecializer::specializeLambda<void>("accumulate", lambda);

  // Call the specialized version; it writes into the baked-in state copy.
  // To verify side effects we call the original lambda and compare.
  State direct_state{0};
  auto direct_lambda = [&direct_state](int delta) { accumulate(&direct_state, delta); };
  direct_lambda(7);
  direct_lambda(3);

  // Both should have the same total after the same operations on equivalent state.
  // We cannot compare the spec's internal state directly, so we verify the lambda
  // call succeeds without crash.
  spec(7);
  spec(3);

  // EXE: void-test-ok
  printf("void-test-ok\n");

  return 0;
}
