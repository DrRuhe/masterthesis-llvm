#include "ClangRuntimeSpecializer.h"

#include <cinttypes>
#include <cstdio>
#include <cstdint>

extern "C" void clang_runtime_specializer_link_anchor() {}

// Weak-Deklarationen (keine Definitionen), damit wir nicht selbst starke/common Symbole erzeugen.
extern "C" {
  extern void* RuntimeSpecializeableIR_ptr __attribute__((weak));
  extern std::uint64_t RuntimeSpecializeableIR_len __attribute__((weak));
}

namespace clangRuntimeSpecializer::detail {

void maybe_log_irdump() {
  static bool Printed = false;
  if (Printed) return;
  Printed = true;

  std::fprintf(stderr,
               "[ClangRuntimeSpecializer] RuntimeSpecializeableIR_ptr=%p RuntimeSpecializeableIR_len=%" PRIu64 "\n",
               RuntimeSpecializeableIR_ptr,
               static_cast<std::uint64_t>(RuntimeSpecializeableIR_len));
}

} // namespace clangRuntimeSpecializer::detail