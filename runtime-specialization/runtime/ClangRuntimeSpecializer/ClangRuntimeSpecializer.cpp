#include "ClangRuntimeSpecializer.h"

#include <cinttypes>
#include <cstdio>
#include <dlfcn.h>

extern "C" void clang_runtime_specializer_link_anchor() {}

// Weak-Deklarationen (keine Definitionen), damit wir nicht selbst starke/common Symbole erzeugen.
extern "C" {
  extern void* RuntimeSpecializeableIR_ptr __attribute__((weak));
  extern std::uint64_t RuntimeSpecializeableIR_len __attribute__((weak));
}

namespace clangRuntimeSpecializer::detail {

    void maybe_log_irdump()
    {
        // Robust on ELF/Linux: don't try to "probe" weak object symbols via &sym.
        // In PIC code &sym may refer to a GOT slot even when the symbol is undefined.
        // Instead, query the dynamic loader and only read if the variable exists.
        void *ptrSym = dlsym(RTLD_DEFAULT, "RuntimeSpecializeableIR_ptr");
        void *lenSym = dlsym(RTLD_DEFAULT, "RuntimeSpecializeableIR_len");

        if (!ptrSym || !lenSym) {
            std::fprintf(stderr,
                         "[ClangRuntimeSpecializer] RuntimeSpecializeableIR symbols not present\n");
            return;
        }

        auto *PtrVar = reinterpret_cast<void * const *>(ptrSym);
        auto *LenVar = reinterpret_cast<const std::uint64_t *>(lenSym);

        std::fprintf(stderr,
                     "[ClangRuntimeSpecializer] RuntimeSpecializeableIR_ptr=%p RuntimeSpecializeableIR_len=%" PRIu64 "\n",
                     *PtrVar,
                     static_cast<std::uint64_t>(*LenVar));
    }
} // namespace clangRuntimeSpecializer::detail
