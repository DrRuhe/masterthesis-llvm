#include "ClangRuntimeSpecializer.h"

#include <algorithm>
#include <cstdio>
#include <unistd.h>
#include <utility>
#include <vector>

#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/Linker/Linker.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/IRTransformLayer.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/IPO/ModuleInliner.h"
#include "llvm/Transforms/IPO/SCCP.h"
#include "llvm/Transforms/IPO/GlobalOpt.h"
#include "llvm/Transforms/IPO/GlobalDCE.h"
#include "llvm/Transforms/IPO/WholeProgramDevirt.h"
#include "llvm/Transforms/Scalar/SROA.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Scalar/InstSimplifyPass.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/LICM.h"
#include "llvm/Transforms/Scalar/LoopUnrollPass.h"
#include "llvm/Transforms/Scalar/LoopRotation.h"
#include "llvm/Transforms/Scalar/JumpThreading.h"
#include "llvm/Transforms/Scalar/CorrelatedValuePropagation.h"
#include "llvm/Transforms/Scalar/EarlyCSE.h"
#include "DevirtualizeConstantVtableCalls.h"
#include "StaticMutabilityAnalysis.h"
#include "InvariantLoadToConstant.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/IPO/FunctionAttrs.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/ExecutionEngine/JITEventListener.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/TargetParser/Host.h"

extern "C" void clang_runtime_specializer_link_anchor() {}

// ---------------------------------------------------------------------------
// Multi-TU IR blob registry
// Each TU compiled with the plugin injects a constructor that calls
// clang_runtime_specializer_register_blob(), enabling multiple TUs to coexist
// in a single binary.
// ---------------------------------------------------------------------------

namespace {
  struct BlobEntry { const void* Ptr; std::uint64_t Len; };
  std::vector<BlobEntry> g_registered_blobs;
} // namespace

extern "C" void clang_runtime_specializer_register_blob(const void* ptr, std::uint64_t len) {
  g_registered_blobs.push_back({ptr, len});
}

namespace {

  std::unique_ptr<llvm::Module> load_and_merge_blobs(llvm::LLVMContext& ctx) {
    if (g_registered_blobs.empty()) {
      throw clangRuntimeSpecializer::ClangRuntimeSpecializerDumpedIRError(
          "No IR blobs registered. Was the binary compiled with the plugin?");
    }

    std::unique_ptr<llvm::Module> Merged;
    for (const auto& blob : g_registered_blobs) {
      const llvm::StringRef Bytes(reinterpret_cast<const char*>(blob.Ptr), blob.Len);
      const llvm::MemoryBufferRef Buffer(Bytes, "RuntimeSpecializeableIR");
      llvm::Expected<std::unique_ptr<llvm::Module>> M =
          llvm::parseBitcodeFile(Buffer, ctx);
      if (!M) {
        const std::string Err = llvm::toString(M.takeError());
        throw clangRuntimeSpecializer::ClangRuntimeSpecializerDumpedIRError(
            "Failed to parse bitcode: " + Err);
      }
      if (!Merged) {
        Merged = std::move(*M);
      } else {
        if (llvm::Linker::linkModules(*Merged, std::move(*M))) {
          throw clangRuntimeSpecializer::ClangRuntimeSpecializerDumpedIRError(
              "Failed to link IR modules from multiple TUs");
        }
      }
    }
    return Merged;
  }

} // namespace

extern "C" {
    uint64_t g_inst_count = 0;
    uint64_t g_load_count = 0;
    uint64_t g_store_count = 0;
    uint64_t g_call_count = 0;
    uint64_t g_arith_count = 0;
    uint64_t g_cmp_count = 0;
    uint64_t g_branch_count = 0;
    uint64_t g_ret_count = 0;
    uint64_t g_other_count = 0;
}

namespace clangRuntimeSpecializer {

  static ClangRuntimeSpecializer::LogLevel CurrentLogLevel = ClangRuntimeSpecializer::LogLevel::Debug;

  void ClangRuntimeSpecializer::setLogLevel(const LogLevel Level) {
      CurrentLogLevel = Level;
  }

  ClangRuntimeSpecializer::LogLevel ClangRuntimeSpecializer::getLogLevel() {
      return CurrentLogLevel;
  }

  __attribute__((always_inline))
  void ClangRuntimeSpecializer::log(const LogLevel Level, const char* const FuncName, const llvm::Twine Message)
  {
    if (static_cast<int>(getLogLevel()) >= static_cast<int>(Level)) {
        log(Level, FuncName, Message.str().c_str());
    }
  }

  __attribute__((always_inline))
  void ClangRuntimeSpecializer::log(const LogLevel Level, const char* const FuncName, const char* const Message) {
      if (static_cast<int>(getLogLevel()) >= static_cast<int>(Level)) {
        const char* LevelStr = "UNKNOWN";
        switch (Level) {
          case LogLevel::None: LevelStr = "NONE"; break;
          case LogLevel::Error: LevelStr = "ERROR"; break;
          case LogLevel::Info: LevelStr = "INFO"; break;
          case LogLevel::Debug: LevelStr = "DEBUG"; break;
        }
        std::fprintf(stdout, "%s: [%s] %s\n", LevelStr, FuncName, Message);
      }
  }

  static std::unique_ptr<ClangRuntimeSpecializer> Instance;

  void ClangRuntimeSpecializer::resetCounters() {
    g_inst_count = 0;
    g_load_count = 0;
    g_store_count = 0;
    g_call_count = 0;
    g_arith_count = 0;
    g_cmp_count = 0;
    g_branch_count = 0;
    g_ret_count = 0;
    g_other_count = 0;
  }

  ClangRuntimeSpecializer::InstructionCounts ClangRuntimeSpecializer::getCurrentCounters() {
    return { g_inst_count, g_load_count, g_store_count, g_call_count, g_arith_count, g_cmp_count, g_branch_count, g_ret_count, g_other_count };
  }

  void ClangRuntimeSpecializer::printCounters() {
    auto Counts = getCurrentCounters();
    std::fprintf(stdout, "[Instruction Stats]\n");
    std::fprintf(stdout, "Total Instructions: %lu\n", Counts.Total);
    std::fprintf(stdout, "  Loads: %lu\n", Counts.Loads);
    std::fprintf(stdout, "  Stores: %lu\n", Counts.Stores);
    std::fprintf(stdout, "  Calls: %lu\n", Counts.Calls);
    std::fprintf(stdout, "  Arith: %lu\n", Counts.Arith);
    std::fprintf(stdout, "  Cmp: %lu\n", Counts.Cmp);
    std::fprintf(stdout, "  Branch: %lu\n", Counts.Branches);
    std::fprintf(stdout, "  Ret: %lu\n", Counts.Returns);
    std::fprintf(stdout, "  Other: %lu\n", Counts.Other);
  }

  void ClangRuntimeSpecializer::printComparisonTable(const char* const funcName, const InstructionCounts& Before, const InstructionCounts& After) {
      std::fprintf(stdout, "Comparing instruction counts from specializing %s:\n", funcName);
      std::fprintf(stdout, "%10s %10s %-15s %s\n", "before", "after", "instruction", "change");

      struct Row {
          const char* Name;
          uint64_t B;
          uint64_t A;
      };

      const auto printRow = [](const Row& r) {
          const int64_t Diff = static_cast<int64_t>(r.A) - static_cast<int64_t>(r.B);
          const double Percent = (r.B == 0) ? (r.A == 0 ? 0.0 : 100.0) : (static_cast<double>(std::abs(Diff)) / r.B) * 100.0;
          if (Diff > 0) {
              std::fprintf(stdout, "%10lu %10lu %-15s +%ld, %.0f%%\n", r.B, r.A, r.Name, Diff, Percent);
          } else if (Diff < 0) {
              std::fprintf(stdout, "%10lu %10lu %-15s %ld, -%.0f%%\n", r.B, r.A, r.Name, Diff, Percent);
          } else {
              std::fprintf(stdout, "%10lu %10lu %-15s 0, 0%%\n", r.B, r.A, r.Name);
          }
      };

      printRow({"total", Before.Total, After.Total});
      std::fprintf(stdout, "\n");

      std::vector<Row> IndividualRows = {
          {"load", Before.Loads, After.Loads},
          {"store", Before.Stores, After.Stores},
          {"call", Before.Calls, After.Calls},
          {"arith", Before.Arith, After.Arith},
          {"cmp", Before.Cmp, After.Cmp},
          {"branch", Before.Branches, After.Branches},
          {"ret", Before.Returns, After.Returns},
          {"other", Before.Other, After.Other}
      };

      std::sort(IndividualRows.begin(), IndividualRows.end(), [](const Row& a, const Row& b) {
          if (a.B != b.B) return a.B > b.B;
          return std::string(a.Name) < std::string(b.Name);
      });

      for (const auto& r : IndividualRows) {
          printRow(r);
      }
  }

  ClangRuntimeSpecializer* ClangRuntimeSpecializer::init() {
    if (Instance) {
      return Instance.get();
    }
    
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetDisassembler();

    Instance.reset(new ClangRuntimeSpecializer);

    // Build the LLJIT with TrapUnreachable=true so that IR `unreachable`
    // instructions always emit a ud2 trap instead of 0 machine bytes.
    // Without this, a fully-optimised wrapper whose body reduces to
    // `unreachable` (e.g. because internal globals are still null-initialised
    // in the JIT clone) produces a zero-byte .text section.  JITLink's
    // BasicLayout::apply() then calls setMutableContent({nullptr,0}), which
    // asserts that the data pointer is non-null.
    auto JTMBOrErr = llvm::orc::JITTargetMachineBuilder::detectHost();
    if (!JTMBOrErr) {
      const std::string ErrMsg = llvm::toString(JTMBOrErr.takeError());
      throw ClangRuntimeSpecializerError("Failed to detect host for JIT: " + ErrMsg);
    }
    JTMBOrErr->getOptions().TrapUnreachable = true;

    auto JITExp = llvm::orc::LLJITBuilder()
        .setJITTargetMachineBuilder(std::move(*JTMBOrErr))
        .create();
    if (!JITExp) {
      const std::string ErrMsg = llvm::toString(JITExp.takeError());
      throw ClangRuntimeSpecializerError("Failed to create JIT: " + ErrMsg);
    }
    Instance->JIT = std::move(*JITExp);

#if LLVM_USE_PERF
    if (auto *RTDyldLayer = llvm::dyn_cast<llvm::orc::RTDyldObjectLinkingLayer>(
            &Instance->JIT->getObjLinkingLayer())) {
      if (auto *Listener = llvm::JITEventListener::createPerfJITEventListener())
        RTDyldLayer->registerJITEventListener(*Listener);
    }
#endif

    Instance->JIT->getMainJITDylib().addGenerator(
        llvm::cantFail(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
            Instance->JIT->getDataLayout().getGlobalPrefix())));

    // Explicitly export instrumentation counters to the JIT.
    if (Instance->isInstructionInstrumentationEnabled()) {
      auto &JD = Instance->JIT->getMainJITDylib();
      llvm::orc::SymbolMap Symbols;
      Symbols[Instance->JIT->mangleAndIntern("g_inst_count")] = { llvm::orc::ExecutorAddr::fromPtr(&g_inst_count), llvm::JITSymbolFlags::Exported };
      Symbols[Instance->JIT->mangleAndIntern("g_load_count")] = { llvm::orc::ExecutorAddr::fromPtr(&g_load_count), llvm::JITSymbolFlags::Exported };
      Symbols[Instance->JIT->mangleAndIntern("g_store_count")] = { llvm::orc::ExecutorAddr::fromPtr(&g_store_count), llvm::JITSymbolFlags::Exported };
      Symbols[Instance->JIT->mangleAndIntern("g_call_count")] = { llvm::orc::ExecutorAddr::fromPtr(&g_call_count), llvm::JITSymbolFlags::Exported };
      Symbols[Instance->JIT->mangleAndIntern("g_arith_count")] = { llvm::orc::ExecutorAddr::fromPtr(&g_arith_count), llvm::JITSymbolFlags::Exported };
      Symbols[Instance->JIT->mangleAndIntern("g_cmp_count")] = { llvm::orc::ExecutorAddr::fromPtr(&g_cmp_count), llvm::JITSymbolFlags::Exported };
      Symbols[Instance->JIT->mangleAndIntern("g_branch_count")] = { llvm::orc::ExecutorAddr::fromPtr(&g_branch_count), llvm::JITSymbolFlags::Exported };
      Symbols[Instance->JIT->mangleAndIntern("g_ret_count")] = { llvm::orc::ExecutorAddr::fromPtr(&g_ret_count), llvm::JITSymbolFlags::Exported };
      Symbols[Instance->JIT->mangleAndIntern("g_other_count")] = { llvm::orc::ExecutorAddr::fromPtr(&g_other_count), llvm::JITSymbolFlags::Exported };
      cantFail(JD.define(llvm::orc::absoluteSymbols(Symbols)));
    }
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

            bool Optimize = true;
            for (auto &F : M) {
                if (F.hasFnAttribute("force-no-optimize")) {
                    Optimize = false;
                    break;
                }
            }

            // Strip debug info by default to reduce JIT overhead and code size.
            // Only keep debug info if explicitly requested via setKeepDebugInfo(true).
            if (!Instance->shouldKeepDebugInfo()) {
              // llvm::StripDebugInfo removes most debug info, but sometimes leaves
              // some behind in modules that already have it.
              bool DebugInfoStripped = llvm::StripDebugInfo(M);
              
              // More aggressive stripping:
              // 1. Remove all debug-related named metadata
              const char *DebugMetadataNames[] = {
                  "llvm.dbg.cu", "llvm.module.flags", "llvm.ident"};
              for (const char *Name : DebugMetadataNames) {
                if (llvm::NamedMDNode *NMD = M.getNamedMetadata(Name)) {
                  M.eraseNamedMetadata(NMD);
                }
              }
              
              // 2. Remove all debug intrinsics from all functions
              for (auto &F : M) {
                  std::vector<llvm::Instruction*> ToErase;
                  for (auto &BB : F) {
                      for (auto &I : BB) {
                          if (llvm::isa<llvm::DbgInfoIntrinsic>(&I) || llvm::isa<llvm::DbgLabelInst>(&I)) {
                              ToErase.push_back(&I);
                          } else {
                              // Also clear any debug location attached to instructions
                              I.setDebugLoc(llvm::DebugLoc());
                          }
                      }
                  }
                  for (auto *I : ToErase) {
                      I->eraseFromParent();
                  }
                  // Clear any debug metadata attached to the function itself
                  F.setSubprogram(nullptr);
                  // Remove other debug-related metadata attachments
                  F.setMetadata(llvm::LLVMContext::MD_dbg, nullptr);
              }
              
              if (DebugInfoStripped) {
                log(LogLevel::Debug, "IRTransform", "Debug info stripped from JIT module");
              }
            }

            if (Optimize) {
              // FIXPOINT ITERATION: Runtime specialization requires aggressive devirtualization
              // and inlining. We iterate with a carefully ordered pipeline:
              // 1. IPSCCP -> GlobalOpt -> GlobalDCE (interprocedural constant propagation & devirt)
              // 2. Pre-inlining passes: EarlyCSE, SROA, JumpThreading, CVP, InstCombine
              // 3. ModuleInliner (aggressive inlining)
              // 4. Post-inlining passes: GVN (KEY!), EarlyCSE, JumpThreading, CVP, loop opts
              // The CRITICAL insight: GVN must run AFTER inlining to propagate vtable pointer
              // constants through the inlined code, enabling complete devirtualization.
              // This ensures virtual calls like Filter::next and Scan::next are fully devirtualized
              // and inlined, eliminating all vtable lookups.

              // Prepare all functions for aggressive inlining
              for (auto &F : M) {
                if (!F.isDeclaration()) {
                  F.removeFnAttr(llvm::Attribute::NoInline);
                  F.removeFnAttr(llvm::Attribute::OptimizeNone);
                  F.addFnAttr(llvm::Attribute::AlwaysInline);
                }
              }

              // Initial pass: Always inline marked functions
              {
                llvm::ModulePassManager InitialMPM;

                // 3a. Cleanup alloca/store/load before analysis
                llvm::FunctionPassManager FPM;
                FPM.addPass(llvm::SROAPass(llvm::SROAOptions::ModifyCFG));
                FPM.addPass(llvm::EarlyCSEPass());
                FPM.addPass(llvm::InstCombinePass());
                InitialMPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(FPM)));

                // 3b. Attribute inference - deduces readonly, readnone, etc.
                InitialMPM.addPass(llvm::ReversePostOrderFunctionAttrsPass());

                // 3c. Static Mutability Analysis to infer read-only fields
                InitialMPM.addPass(llvm::createModuleToFunctionPassAdaptor(StaticMutabilityAnalysis::StaticMutabilityAnalysisPass()));
                
                // 3d. Replace invariant loads with constants from host memory
                InitialMPM.addPass(llvm::createModuleToFunctionPassAdaptor(InvariantLoadToConstantPass()));

                InitialMPM.addPass(llvm::AlwaysInlinerPass(/*InsertLifetimeIntrinsics=*/true));
                InitialMPM.run(M, MAM);
              }


              if (Instance->printsFixpointIterations())
              {
                for (auto &F : M) {
                  if (!F.isDeclaration() && F.getName().starts_with("specialized_wrapper_")) {
                    log(LogLevel::Debug, "IRTransform", [&] {
                        return llvm::formatv("Initial specialized function IR:\n{0}", printLLVM(&F)).str();
                    });
                  }
                }
              }

              // Fixpoint iteration: Run until no more changes occur
              // We count instructions to detect convergence
              constexpr int MaxFixpointIterations = 10;
              size_t PrevInstCount = 0;

              for (int Iteration = 0; Iteration < MaxFixpointIterations; ++Iteration) {
                if (Instance->printsFixpointIterations()) log(LogLevel::Debug, "IRTransform",
                    llvm::formatv("Fixpoint iteration {0}", Iteration).str());

                llvm::ModulePassManager FixpointMPM;

                // 1. Interprocedural Sparse Conditional Constant Propagation
                // This propagates constants across function boundaries and can turn
                // indirect calls into direct calls
                FixpointMPM.addPass(llvm::IPSCCPPass());

                // 1b. Devirtualize indirect calls through constant vtable pointers
                FixpointMPM.addPass(DevirtualizeConstantVtableCallsPass());

                // 1b2. Infer attributes again after optimistic resolution
                FixpointMPM.addPass(llvm::ReversePostOrderFunctionAttrsPass());

                // 1c. Static Mutability Analysis to infer read-only fields
                FixpointMPM.addPass(llvm::createModuleToFunctionPassAdaptor(StaticMutabilityAnalysis::StaticMutabilityAnalysisPass()));

                // 1d. Replace invariant loads with constants from host memory
                FixpointMPM.addPass(llvm::createModuleToFunctionPassAdaptor(InvariantLoadToConstantPass()));

                // 1e. Inline devirtualized calls
                FixpointMPM.addPass(llvm::AlwaysInlinerPass());

                // 2. Global optimizations - includes devirtualization
                // GlobalOpt can devirtualize calls when it knows the concrete type
                FixpointMPM.addPass(llvm::GlobalOptPass());

                // 2b. Whole-program devirtualization - attempts to devirtualize based on
                // vtable information. This can eliminate virtual calls when the set of
                // possible callees is known.
                FixpointMPM.addPass(llvm::WholeProgramDevirtPass());

                // 3. Pre-inlining function-level optimizations
                llvm::FunctionPassManager PreInlineFPM;

                // EarlyCSE with MemorySSA - eliminate redundant loads early
                PreInlineFPM.addPass(llvm::EarlyCSEPass(/*UseMemorySSA=*/true));

                // SROA - breaks down aggregates into scalars
                PreInlineFPM.addPass(llvm::SROAPass(llvm::SROAOptions::ModifyCFG));

                // JumpThreading - thread control flow based on known values
                PreInlineFPM.addPass(llvm::JumpThreadingPass());

                // CorrelatedValuePropagation - propagate value constraints
                PreInlineFPM.addPass(llvm::CorrelatedValuePropagationPass());

                // SimplifyCFG - cleanup control flow
                PreInlineFPM.addPass(llvm::SimplifyCFGPass());

                // InstCombine - combine instructions to expose more devirtualization
                PreInlineFPM.addPass(llvm::InstCombinePass());

                FixpointMPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(PreInlineFPM)));

                // 5. Aggressive inlining - inline everything possible
                llvm::InlineParams IP = llvm::getInlineParams();
                IP.DefaultThreshold = 500000;
                IP.HintThreshold = 500000;
                IP.ColdThreshold = 500000;
                IP.OptSizeThreshold = 500000;
                IP.OptMinSizeThreshold = 500000;
                IP.HotCallSiteThreshold = 500000;
                IP.LocallyHotCallSiteThreshold = 500000;
                IP.ColdCallSiteThreshold = 500000;
                IP.ComputeFullInlineCost = false;  // Faster
                IP.EnableDeferral = false;
                IP.AllowRecursiveCall = true;

                FixpointMPM.addPass(llvm::ModuleInlinerPass(IP));

                // 6. CRITICAL: Post-inlining optimizations
                // GVN can NOW see through inlined code and propagate vtable pointer constants!
                llvm::FunctionPassManager PostInlineFPM;

                // GVN - propagate constants through inlined code (KEY for devirtualization!)
                PostInlineFPM.addPass(llvm::GVNPass());

                // Static Mutability Analysis to infer read-only fields
                PostInlineFPM.addPass(StaticMutabilityAnalysis::StaticMutabilityAnalysisPass());

                // Replace invariant loads with constants from host memory
                PostInlineFPM.addPass(InvariantLoadToConstantPass());

                // InstCombine - fold loads of constant vtable pointers
                PostInlineFPM.addPass(llvm::InstCombinePass());

                // SROA again - eliminate redundant alloca/store/load patterns
                PostInlineFPM.addPass(llvm::SROAPass(llvm::SROAOptions::ModifyCFG));

                // InstCombine again after SROA
                PostInlineFPM.addPass(llvm::InstCombinePass());

                // EarlyCSE - cleanup redundant loads
                PostInlineFPM.addPass(llvm::EarlyCSEPass(/*UseMemorySSA=*/true));

                // JumpThreading - may find new opportunities
                PostInlineFPM.addPass(llvm::JumpThreadingPass());

                // CorrelatedValuePropagation
                PostInlineFPM.addPass(llvm::CorrelatedValuePropagationPass());

                // SimplifyCFG before loop optimization
                PostInlineFPM.addPass(llvm::SimplifyCFGPass());

                // Loop optimizations: rotate and LICM (requires MemorySSA)
                llvm::LoopPassManager LPM;
                LPM.addPass(llvm::LoopRotatePass());
                LPM.addPass(llvm::LICMPass(llvm::LICMOptions()));
                PostInlineFPM.addPass(llvm::createFunctionToLoopPassAdaptor(std::move(LPM), /*UseMemorySSA=*/true));

                // Aggressive loop unrolling (runs on functions, not loops)
                llvm::LoopUnrollOptions UnrollOpts;
                UnrollOpts.setPartial(true);
                UnrollOpts.setRuntime(true);
                UnrollOpts.setUpperBound(true);
                UnrollOpts.setFullUnrollMaxCount(128);
                PostInlineFPM.addPass(llvm::LoopUnrollPass(UnrollOpts));

                // Post-unroll cleanup
                PostInlineFPM.addPass(llvm::InstCombinePass());
                PostInlineFPM.addPass(llvm::SimplifyCFGPass());
                PostInlineFPM.addPass(llvm::InstSimplifyPass());

                FixpointMPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(PostInlineFPM)));

                // Run the fixpoint iteration pass pipeline
                FixpointMPM.run(M, MAM);

                if (Instance->printsFixpointIterations())
                {
                  for (auto &F : M) {
                    if (!F.isDeclaration() && F.getName().starts_with("specialized_wrapper_")) {
                      log(LogLevel::Debug, "IRTransform", [&] {
                          return llvm::formatv("Specialized function IR after iteration {0}:\n{1}", Iteration, printLLVM(&F)).str();
                      });
                    }
                  }
                }


                // Count instructions to check for convergence
                size_t InstCount = 0;
                for (auto &F : M) {
                  for (auto &BB : F) {
                    InstCount += BB.size();
                  }
                }

                if (Instance->printsFixpointIterations()) log(LogLevel::Debug, "IRTransform",
                    llvm::formatv("Instruction count: {0}", InstCount).str());

                if (InstCount == PrevInstCount) {
                  if (Instance->printsFixpointIterations()) log(LogLevel::Debug, "IRTransform",
                      llvm::formatv("Fixpoint reached after {0} iterations", Iteration + 1).str());
                  break;
                }

                PrevInstCount = InstCount;
              }

              // Run GlobalDCE after all fixpoint iterations complete.
              // Must not run during the fixpoint loop because virtual function implementations
              // (e.g., Scan::next, Filter::next) may have no direct callers yet but are still
              // needed as devirtualization targets in subsequent iterations.
              {
                llvm::ModulePassManager PostFixpointMPM;
                PostFixpointMPM.addPass(llvm::GlobalDCEPass());
                PostFixpointMPM.run(M, MAM);
              }

              // Final O3 pass for cleanup and additional optimizations
              {
                llvm::ModulePassManager FinalMPM = PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);
                FinalMPM.run(M, MAM);
              }
            }

            // After optimization, perform dynamic instruction counting instrumentation if enabled.
            bool Instrument = Instance->isInstructionInstrumentationEnabled();
            if (!Instrument) {
                for (auto &F : M) {
                    if (F.hasFnAttribute("force-instrument")) {
                        Instrument = true;
                        break;
                    }
                }
            }

            if (Instrument) {
              for (auto &F : M) {
                if (F.isDeclaration()) continue;
                // Instrument only the specialized wrapper or everything that was inlined into it.
                // For simplicity, let's instrument all functions in the module.
                for (auto &BB : F) {
                  uint64_t BBInstCount = 0;
                  uint64_t BBLoadCount = 0;
                  uint64_t BBStoreCount = 0;
                  uint64_t BBCallCount = 0;
                  uint64_t BBArithCount = 0;
                  uint64_t BBCmpCount = 0;
                  uint64_t BBBranchCount = 0;
                  uint64_t BBRetCount = 0;
                  uint64_t BBOtherCount = 0;
                  for (auto &I : BB) {
                    BBInstCount++;
                    if (llvm::isa<llvm::LoadInst>(&I)) BBLoadCount++;
                    else if (llvm::isa<llvm::StoreInst>(&I)) BBStoreCount++;
                    else if (llvm::isa<llvm::CallBase>(&I)) BBCallCount++;
                    else if (llvm::isa<llvm::BinaryOperator>(&I) || llvm::isa<llvm::UnaryOperator>(&I)) BBArithCount++;
                    else if (llvm::isa<llvm::CmpInst>(&I)) BBCmpCount++;
                    else if (llvm::isa<llvm::ReturnInst>(&I)) BBRetCount++;
                    else if (I.isTerminator()) BBBranchCount++;
                    else BBOtherCount++;
                  }

                  if (BBInstCount > 0) {
                    llvm::IRBuilder<> Builder(&*BB.getFirstInsertionPt());
                    auto CreateIncrement = [&](const char* Name, uint64_t Count) {
                      if (Count == 0) return;
                      auto *CounterGV = M.getGlobalVariable(Name, true);
                      if (!CounterGV) {
                        CounterGV = new llvm::GlobalVariable(M, llvm::Type::getInt64Ty(M.getContext()), false,
                                                             llvm::GlobalValue::ExternalLinkage, nullptr, Name);
                      }
                      Builder.CreateAtomicRMW(llvm::AtomicRMWInst::Add, CounterGV,
                                             Builder.getInt64(Count), llvm::MaybeAlign(),
                                             llvm::AtomicOrdering::Monotonic);
                    };

                    CreateIncrement("g_inst_count", BBInstCount);
                    CreateIncrement("g_load_count", BBLoadCount);
                    CreateIncrement("g_store_count", BBStoreCount);
                    CreateIncrement("g_call_count", BBCallCount);
                    CreateIncrement("g_arith_count", BBArithCount);
                    CreateIncrement("g_cmp_count", BBCmpCount);
                    CreateIncrement("g_branch_count", BBBranchCount);
                    CreateIncrement("g_ret_count", BBRetCount);
                    CreateIncrement("g_other_count", BBOtherCount);
                  }
                }
              }
            }

            for (auto &F : M) {
              if (!F.isDeclaration() && F.getName().starts_with("specialized_wrapper_")) {
                log(LogLevel::Debug, "IRTransform", [&] {
                    return llvm::formatv("Optimized specialized function IR:\n{0}", printLLVM(&F)).str();
                });
              }
            }
          });
          return std::move(TSM);
        });

    Instance->TSCtx.withContextDo([&](llvm::LLVMContext *Ctx) {
        Instance->Module = load_and_merge_blobs(*Ctx);
    });

    return Instance.get();
  }

  ClangRuntimeSpecializer::ClangRuntimeSpecializer() : TSCtx(std::make_unique<llvm::LLVMContext>()) {}

  void ClangRuntimeSpecializer::checkInitialization(const char* const funcName) const {
    if (funcName == nullptr) {
      throw ClangRuntimeSpecializerError("funcName was null!");
    }
    if (Module == nullptr) {
      throw ClangRuntimeSpecializerError("Module was null!");
    }
    if (!JIT) {
      throw ClangRuntimeSpecializerError("JIT was not initialized!");
    }
  }

  llvm::Function* ClangRuntimeSpecializer::getTargetFunction(const char* const funcName) const {
    std::string FuncNameStr(funcName);
    if (!FuncNameStr.empty() && FuncNameStr.front() == '&') {
      FuncNameStr.erase(0, 1);
    }
    llvm::Function * const TargetFunc = Module->getFunction(FuncNameStr);
    if (TargetFunc == nullptr) {
      throw ClangRuntimeSpecializerDumpedIRError((llvm::Twine("Could not find function: ") + funcName + " (It might be optimized out already by dead-code-elimination?)").str());
    }
    return TargetFunc;
  }

  void ClangRuntimeSpecializer::validateArgs(llvm::Function* const TargetFunc, const size_t NumArgs) const {
    if (NumArgs != TargetFunc->arg_size())
    {
      throw ClangRuntimeSpecializerError((llvm::Twine("The number of args are incompatible! numArgs: ") + llvm::Twine(NumArgs) + ", TargetFunc->arg_size(): " + llvm::Twine(TargetFunc->arg_size())).str());
    }
  }

  std::string ClangRuntimeSpecializer::createUniqueWrapperName() const {
    return "specialized_wrapper_" + std::to_string(const_cast<ClangRuntimeSpecializer*>(this)->GlobalSpecializationCount++) + "_" + std::to_string(reinterpret_cast<uintptr_t>(this));
  }

  void ClangRuntimeSpecializer::prepareModuleForJIT(llvm::Module& M, const std::string& WrapperName) const {
    bool Optimize = true;
    for (auto &F : M) {
        if (F.hasFnAttribute("force-no-optimize")) {
            Optimize = false;
            break;
        }
    }

    // Every function except the specialized wrapper should have available_externally linkage
    // if it has a definition. This allows the JIT inliner to see the bodies but won't
    // produce a definition in the resulting object file, as we want to use the host's version
    // if it's not inlined.
    // Collect functions referenced from vtable constants.  These are virtual-method
    // implementations that DevirtualizeConstantVtablePass may call in a future fixpoint
    // iteration even when they currently have no direct callers, so they must not be
    // deleted by the inliner's dead-function removal.
    llvm::SmallPtrSet<llvm::Function *, 16> VTableFunctions;
    if (Optimize) {
      llvm::SmallVector<llvm::Constant *, 32> WorkList;
      llvm::SmallPtrSet<llvm::Constant *, 32> Visited;
      for (auto &G : M.globals()) {
        if (G.isConstant() && G.hasInitializer()) {
          auto *Init = G.getInitializer();
          if (Visited.insert(Init).second)
            WorkList.push_back(Init);
        }
      }
      while (!WorkList.empty()) {
        auto *C = WorkList.pop_back_val();
        if (auto *F = llvm::dyn_cast<llvm::Function>(C)) {
          VTableFunctions.insert(F);
        } else {
          for (unsigned I = 0, E = C->getNumOperands(); I != E; ++I) {
            if (auto *Op = llvm::dyn_cast<llvm::Constant>(C->getOperand(I)))
              if (Visited.insert(Op).second)
                WorkList.push_back(Op);
          }
        }
      }
    }

    for (auto &F : M) {
      if (F.getName() == WrapperName) {
         F.setLinkage(llvm::GlobalValue::ExternalLinkage);
         continue;
      }
      if (!F.isDeclaration()) {
        if (!Optimize) {
          // Baseline / instrumentation path: compile all functions so instrumented
          // bodies run instead of the host's uninstrumented versions.
          F.setLinkage(llvm::GlobalValue::InternalLinkage);
        } else if (VTableFunctions.count(&F)) {
          // Virtual-method implementations referenced from vtables need WeakODR so
          // the ModuleInlinerPass does not delete their bodies between fixpoint
          // iterations — DevirtualizeConstantVtablePass looks them up by name.
          F.setLinkage(llvm::GlobalValue::WeakODRLinkage);
        } else {
          // All other functions: body available for inlining but canonical definition
          // lives in the host process (--export-dynamic).  The JIT will not compile
          // a new copy; if not inlined it resolves the symbol from the host.
          F.setLinkage(llvm::GlobalValue::AvailableExternallyLinkage);
        }
      }
    }
    
    // Also convert global variables to available_externally or declarations.
    // Special care for constant strings and other internal globals.
    for (auto &G : M.globals()) {
      if (!G.isDeclaration()) {
        // If it's a constant string or similar internal, we might want to keep it
        // as private/internal if we can't find it in the host.
        // However, available_externally for globals usually works if they are
        // indeed available. For JIT, internal globals might NOT be available.
        if (G.hasInternalLinkage() || G.hasPrivateLinkage()) {
          continue; // Keep internal/private globals as is.
        }
        G.setLinkage(llvm::GlobalValue::AvailableExternallyLinkage);
      }
    }

    // Remove global constructors/destructors. The host process already ran them
    // at startup; the JIT only needs to compile the specialized wrapper, not
    // re-initialize the entire translation unit.
    if (auto *GCtors = M.getGlobalVariable("llvm.global_ctors"))
      GCtors->eraseFromParent();
    if (auto *GDtors = M.getGlobalVariable("llvm.global_dtors"))
      GDtors->eraseFromParent();

    // Remove zero-sized globals (e.g., empty C++ init structs of type `{}`) before JIT
    // compilation. The ELF backend can emit a SHT_PROGBITS section with sh_size=0 for
    // such globals. JITLink's BasicLayout::apply() then calls setMutableContent with a
    // null pointer (malloc(0) may return nullptr), triggering an assertion failure.
    // These globals are always dead after llvm.global_ctors is erased above, so removing
    // them here is safe; GlobalDCE in the IR transform will clean up any remaining
    // references in dead constructor functions.
    {
      const auto &DL = M.getDataLayout();
      llvm::SmallVector<llvm::GlobalVariable *, 16> ZeroSized;
      for (auto &G : M.globals()) {
        if (!G.isDeclaration()) {
          uint64_t Sz = DL.getTypeAllocSize(G.getValueType());
          if (Sz == 0)
            ZeroSized.push_back(&G);
        }
      }
      for (auto *G : ZeroSized) {
        G->replaceAllUsesWith(llvm::PoisonValue::get(G->getType()));
        G->eraseFromParent();
      }
    }
  }

  uintptr_t ClangRuntimeSpecializer::addModuleAndLookup(llvm::orc::ThreadSafeModule TSM,
                                                         const std::string& WrapperName,
                                                         const std::string& OrigFuncName) {
    // Create a fresh JITDylib for this specialization.  Each specialization gets
    // its own isolated symbol namespace so WeakODR symbols from different
    // specialization calls (e.g. repeated benchmark iterations) never conflict.
    // The new dylib inherits the LLJIT default link order (main dylib + host process
    // symbols + instrumentation counters) automatically.
    auto DylibOrErr = JIT->createJITDylib("spec_" + WrapperName);
    if (!DylibOrErr) {
      const std::string ErrMsg = llvm::toString(DylibOrErr.takeError());
      throw ClangRuntimeSpecializerError("Failed to create JITDylib: " + ErrMsg);
    }
    auto &Dylib = *DylibOrErr;

    // The DynamicLibrarySearchGenerator and instrumentation counter symbols were
    // added to the main JITDylib (not to DefaultLinks), so the fresh dylib needs
    // to fall through to it for host-process and instrumentation symbols.
    Dylib.addToLinkOrder(JIT->getMainJITDylib());

    if (auto Err = JIT->addIRModule(Dylib, std::move(TSM))) {
      const std::string ErrMsg = llvm::toString(std::move(Err));
      throw ClangRuntimeSpecializerError("Failed to add module to JIT: " + ErrMsg);
    }

    auto SpecializedFn = JIT->lookup(Dylib, WrapperName);
    if (!SpecializedFn) {
      const std::string ErrMsg = llvm::toString(SpecializedFn.takeError());
      throw ClangRuntimeSpecializerError("Failed to lookup wrapper: " + ErrMsg);
    }
    uintptr_t Addr = SpecializedFn->getValue();

    uint64_t FuncSize = dumpJITAssembly(OrigFuncName, Addr);

#if LLVM_USE_PERF
    // Write a perf.map entry so perf script can resolve this JIT symbol.
    // Format: <start_hex> <size_hex> <name>  (no 0x prefix, space-separated)
    {
      char MapPath[64];
      std::snprintf(MapPath, sizeof(MapPath), "/tmp/perf-%d.map", (int)getpid());
      if (FILE *F = std::fopen(MapPath, "a")) {
        std::fprintf(F, "%lx %lx %s\n",
                     (unsigned long)Addr,
                     (unsigned long)(FuncSize ? FuncSize : 0x1000),
                     WrapperName.c_str());
        std::fclose(F);
      }
    }
#endif

    return Addr;
  }

  uint64_t ClangRuntimeSpecializer::dumpJITAssembly(const std::string& OrigFuncName, uintptr_t Addr) {
    const char* DumpDir = std::getenv("CRS_ASM_DUMP_DIR");
    if (!DumpDir) return 0;

    llvm::Triple T = JIT->getTargetTriple();
    std::string TripleStr = T.getTriple();
    std::string Err;
    const llvm::Target* TheTarget = llvm::TargetRegistry::lookupTarget(TripleStr, Err);
    if (!TheTarget) return 0;

    std::unique_ptr<llvm::MCRegisterInfo>  MRI(TheTarget->createMCRegInfo(TripleStr));
    llvm::MCTargetOptions MCOpts;
    std::unique_ptr<llvm::MCAsmInfo>       MAI(TheTarget->createMCAsmInfo(*MRI, TripleStr, MCOpts));
    std::unique_ptr<llvm::MCInstrInfo>     MII(TheTarget->createMCInstrInfo());
    std::unique_ptr<llvm::MCSubtargetInfo> STI(TheTarget->createMCSubtargetInfo(
        TripleStr, llvm::sys::getHostCPUName(), ""));
    llvm::MCContext Ctx(T, MAI.get(), MRI.get(), STI.get());
    std::unique_ptr<llvm::MCDisassembler> DisAsm(TheTarget->createMCDisassembler(*STI, Ctx));
    std::unique_ptr<llvm::MCInstPrinter>  IP(TheTarget->createMCInstPrinter(
        T, MAI->getAssemblerDialect(), *MAI, *MII, *MRI));
    if (!DisAsm || !IP) return 0;

    std::string AsmText;
    llvm::raw_string_ostream OS(AsmText);
    const uint8_t* Bytes = reinterpret_cast<const uint8_t*>(Addr);
    uint64_t PC = 0;
    for (int I = 0; I < 512; ++I) {
      llvm::MCInst Inst;
      uint64_t Size;
      auto S = DisAsm->getInstruction(Inst, Size,
          llvm::ArrayRef<uint8_t>(Bytes + PC, 64), Addr + PC, llvm::nulls());
      if (S != llvm::MCDisassembler::Success) break;
      IP->printInst(&Inst, Addr + PC, "", *STI, OS);
      OS << "\n";
      bool IsRet = (Bytes[PC] == 0xC3 || Bytes[PC] == 0xCB);
      PC += Size;
      if (IsRet) break;
    }

    // Strip leading '&' if present (function name annotation convention)
    std::string CleanName = OrigFuncName;
    if (!CleanName.empty() && CleanName.front() == '&')
      CleanName.erase(0, 1);

    std::string Path = std::string(DumpDir) + "/" + CleanName + "__specialized.asm";
    if (FILE* F = std::fopen(Path.c_str(), "w")) {
      std::fputs(AsmText.c_str(), F);
      std::fclose(F);
    }

    return PC;
  }

  void ClangRuntimeSpecializer::encourageInlining(llvm::Function* const F) {
    if (!F) return;
    F->removeFnAttr(llvm::Attribute::NoInline);
    F->removeFnAttr(llvm::Attribute::OptimizeNone);
    F->addFnAttr(llvm::Attribute::AlwaysInline);
  }


  ClangRuntimeSpecializer::~ClangRuntimeSpecializer() = default;

} // namespace clangRuntimeSpecializer
