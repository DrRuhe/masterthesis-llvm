#include "ClangRuntimeSpecializer.h"

#include <cstdio>
#include <cstdint>

extern "C" void clang_runtime_specializer_link_anchor() {}

// Weak-Refs auf die von der Compiler-Pipeline erzeugten Globals.
// Wenn nicht vorhanden: bleiben sie 0/null.
extern "C" {
  __attribute__((weak)) void* RuntimeSpecializeableIR_ptr;
  __attribute__((weak)) std::uint64_t RuntimeSpecializeableIR_len;
}

namespace clangRuntimeSpecializer::detail {

void maybe_log_irdump() {
  static bool Printed = false;
  if (Printed) return;
  Printed = true;

  std::fprintf(stderr,
               "[ClangRuntimeSpecializer] RuntimeSpecializeableIR_ptr=%p RuntimeSpecializeableIR_len=%llu\n",
               RuntimeSpecializeableIR_ptr,
               static_cast<unsigned long long>(RuntimeSpecializeableIR_len));
}

} // namespace clangRuntimeSpecializer::detail