#include "ClangRuntimeSpecializer.h"

#include <cstdio>
#include <dlfcn.h>
#include <utility>

#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/MemoryBuffer.h"


extern "C" void clang_runtime_specializer_link_anchor() {}

// Weak-Deklarationen (keine Definitionen), damit wir nicht selbst starke/common Symbole erzeugen.
extern "C" {
  extern void* RuntimeSpecializeableIR_ptr __attribute__((weak));
  extern std::uint64_t RuntimeSpecializeableIR_len __attribute__((weak));
}



namespace {

  struct RuntimeSpecializableData {
    const void* Ptr;
    std::uint64_t Len;
  };

  RuntimeSpecializableData read_runtime_specializable_data() {
    // Robust on ELF/Linux: don't try to "probe" weak object symbols via &sym.
    // In PIC code &sym may refer to a GOT slot even when the symbol is undefined.
    // Instead, query the dynamic loader and only read if the variable exists.
    void* ptrSym = dlsym(RTLD_DEFAULT, "RuntimeSpecializeableIR_ptr");
    void* lenSym = dlsym(RTLD_DEFAULT, "RuntimeSpecializeableIR_len");

    if (!ptrSym || !lenSym) {
      std::fprintf(stderr,
                   "[ClangRuntimeSpecializer] RuntimeSpecializeableIR symbols not present. "
                   "Ensure the IR is dumped and pass -Wl,--export-dynamic\n");
      return {nullptr, 0};
    }

    auto* PtrVar = reinterpret_cast<void* const*>(ptrSym);
    auto* LenVar = reinterpret_cast<const std::uint64_t*>(lenSym);

    return { *PtrVar, static_cast<std::uint64_t>(*LenVar) };
  }

  std::unique_ptr<llvm::Module> parse_module_from_runtime_data(const RuntimeSpecializableData& data,
                                                               llvm::LLVMContext& ctx) {
    llvm::StringRef Bytes(reinterpret_cast<const char*>(data.Ptr), data.Len);
    llvm::MemoryBufferRef Buffer(Bytes, "RuntimeSpecializeableIR");

    llvm::Expected<std::unique_ptr<llvm::Module>> M =
        llvm::parseBitcodeFile(Buffer, ctx);

    if (!M) {
      std::string Err = llvm::toString(M.takeError());
      std::fprintf(stderr,
                   "[ClangRuntimeSpecializer] Failed to parse bitcode: %s\n",
                   Err.c_str());
      return nullptr;
    }

    return std::move(*M);
  }

} // namespace

namespace clangRuntimeSpecializer {

  static std::unique_ptr<ClangRuntimeSpecializer> Instance;

  ClangRuntimeSpecializer* ClangRuntimeSpecializer::init() {
    if (Instance) {
      return Instance.get();
    }

    RuntimeSpecializableData data = read_runtime_specializable_data();
    if (!data.Ptr || data.Len == 0) {
      return nullptr;
    }

    llvm::LLVMContext ctx;
    std::unique_ptr<llvm::Module> mod = parse_module_from_runtime_data(data, ctx);
    if (!mod) {
      return nullptr;
    }

    Instance.reset(new ClangRuntimeSpecializer(std::move(mod)));
    return Instance.get();
  }

  ClangRuntimeSpecializer::ClangRuntimeSpecializer(std::unique_ptr<llvm::Module> mod)
      : Module(std::move(mod)), Builder(Context) {}

  ClangRuntimeSpecializer::~ClangRuntimeSpecializer() = default;

} // namespace clangRuntimeSpecializer