#include "ClangRuntimeSpecializer.h"

#include <cstdio>
#include <dlfcn.h>
#include <utility>

#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/IRTransformLayer.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/IPO/ModuleInliner.h"
#include "llvm/Analysis/InlineCost.h"


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


  // TODO convert this initializer to throw exceptions when a fault occurs instead of returning nullptr.
  ClangRuntimeSpecializer* ClangRuntimeSpecializer::init() {
    if (Instance) {
      return Instance.get();
    }

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    RuntimeSpecializableData data = read_runtime_specializable_data();
    if (!data.Ptr || data.Len == 0) {
      return nullptr;
    }

    Instance.reset(new ClangRuntimeSpecializer);

    auto JITExp = llvm::orc::LLJITBuilder().create();
    if (!JITExp) {
      llvm::errs() << "[ClangRuntimeSpecializer] Failed to create JIT: "
                   << JITExp.takeError() << "\n";
      return nullptr;
    }
    Instance->JIT = std::move(*JITExp);

    // Install an IR transform to run optimizations and log the optimized IR of the
    // specialized wrapper function before compilation.
    Instance->JIT->getIRTransformLayer().setTransform(
        [](llvm::orc::ThreadSafeModule TSM, llvm::orc::MaterializationResponsibility &R)
            -> llvm::Expected<llvm::orc::ThreadSafeModule> {
          TSM.withModuleDo([&](llvm::Module &M) {
            llvm::PassBuilder PB;
            llvm::LoopAnalysisManager LAM;
            llvm::FunctionAnalysisManager FAM;
            llvm::CGSCCAnalysisManager CGAM;
            llvm::ModuleAnalysisManager MAM;

            PB.registerModuleAnalyses(MAM);
            PB.registerCGSCCAnalyses(CGAM);
            PB.registerFunctionAnalyses(FAM);
            PB.registerLoopAnalyses(LAM);
            PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

            // Enable very aggressive inlining prior to the regular O3 pipeline.
            // We first run AlwaysInliner to respect any alwaysinline hints, then
            // a ModuleInlinerPass configured with extremely high thresholds and
            // relaxed deferral/recursion settings to inline as much as possible.
            {
              // Ensure that internal functions can be inlined by making them linkonce_odr
              // or similar if they were just internal. Actually, for JIT it should be fine,
              // but let's make sure the target functions are not marked as "noinline".
              for (auto &F : M) {
                if (!F.isDeclaration()) {
                  F.removeFnAttr(llvm::Attribute::NoInline);
                  F.removeFnAttr(llvm::Attribute::OptimizeNone);
                }
              }

              llvm::ModulePassManager AggressiveMPM;

              // Respect alwaysinline attributes.
              AggressiveMPM.addPass(llvm::AlwaysInlinerPass(/*InsertLifetimeIntrinsics=*/true));

              // Configure aggressive inline parameters.
              llvm::InlineParams IP = llvm::getInlineParams();
              IP.DefaultThreshold = 100000; // very high budget
              IP.HintThreshold = 100000;
              IP.ColdThreshold = 100000;
              IP.OptSizeThreshold = 100000;
              IP.OptMinSizeThreshold = 100000;
              IP.HotCallSiteThreshold = 100000;
              IP.LocallyHotCallSiteThreshold = 100000;
              IP.ColdCallSiteThreshold = 100000;
              IP.ComputeFullInlineCost = true;
              IP.EnableDeferral = false;      // do not defer, inline eagerly
              IP.AllowRecursiveCall = true;   // allow recursive inlining when profitable

              AggressiveMPM.addPass(llvm::ModuleInlinerPass(IP));
              AggressiveMPM.run(M, MAM);
            }

            // After aggressive inlining, run the regular O3 pipeline to clean up
            // and perform further optimizations on the now inlined code.
            {
              llvm::ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);
              MPM.run(M, MAM);
            }

            // Log the optimized IR for any specialized wrapper functions in this module.
            for (auto &F : M) {
              if (!F.isDeclaration() && F.getName().starts_with("specialized_wrapper_")) {
                llvm::errs() << "[ClangRuntimeSpecializer] Optimized IR for " << F.getName() << ":\n";
                F.print(llvm::errs());
                llvm::errs() << "\n";

                // Check if the target function is still called.
                for (auto &BB : F) {
                  for (auto &I : BB) {
                    if (auto *CB = llvm::dyn_cast<llvm::CallBase>(&I)) {
                      if (auto *Callee = CB->getCalledFunction()) {
                        llvm::errs() << "[ClangRuntimeSpecializer] Still calling: " << Callee->getName();
                        if (Callee->isDeclaration())
                          llvm::errs() << " (declaration only)\n";
                        else
                          llvm::errs() << " (definition present)\n";
                      }
                    }
                  }
                }
              }
            }
          });
          return std::move(TSM);
        });

    Instance->Module = parse_module_from_runtime_data(data, Instance->Context);

    return Instance.get();
  }

  ClangRuntimeSpecializer::ClangRuntimeSpecializer(){}

  ClangRuntimeSpecializer::~ClangRuntimeSpecializer() = default;

} // namespace clangRuntimeSpecializer