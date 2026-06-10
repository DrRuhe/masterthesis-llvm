// Regression test: specializeOnly via tuple/index-sequence forwarding helper.
// This mirrors benchmark helper forwarding without lambda capture indirection.
//
// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe | FileCheck %s --check-prefix=EXE --dump-input=always

#include "ClangRuntimeSpecializer.h"
#include <cstdio>
#include <tuple>
#include <type_traits>
#include <utility>

int add3(int a, int b, int c) { return a + b + c; }

template <class R, class F, class Tuple, size_t... I>
__attribute__((noinline))
auto specializeOnlyFromTuple(
    clangRuntimeSpecializer::ClangRuntimeSpecializer* RS,
    F func,
    const clangRuntimeSpecializer::ClangRuntimeSpecializer::Options& opts,
    Tuple& args,
    std::index_sequence<I...>) {
  return RS->template specializeOnly<R>(func, opts, std::get<I>(args)...);
}

template <class F, class Tuple>
__attribute__((noinline))
int runViaTupleHelper(F func, Tuple args) {
  auto *RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
  auto opts = clangRuntimeSpecializer::ClangRuntimeSpecializer::Options::Default();
  using TupleT = std::remove_reference_t<Tuple>;
  auto spec = specializeOnlyFromTuple<int>(
      RS, func, opts, args, std::make_index_sequence<std::tuple_size_v<TupleT>>{});
  return spec();
}

int main() {
  try {
    int result = runViaTupleHelper(&add3, std::make_tuple(10, 20, 12));
    // EXE: result=42
    std::printf("result=%d\n", result);
  } catch (const clangRuntimeSpecializer::ClangRuntimeSpecializerDumpedIRError &e) {
    std::printf("unexpected DumpedIRError: %s\n", e.what());
  }
  // EXE-NOT: unexpected DumpedIRError
  return 0;
}
