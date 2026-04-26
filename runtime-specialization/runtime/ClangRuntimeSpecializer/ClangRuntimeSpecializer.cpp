#include "ClangRuntimeSpecializer.h"
#include "JITPipeline.h"

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
#include "llvm/Transforms/IPO/GlobalDCE.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/PassInstrumentation.h"
#include "llvm/Support/TimeProfiler.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/Config/llvm-config.h"
#include <chrono>
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
  struct BlobEntry {
    const void* Ptr;
    std::uint64_t Len;
    std::vector<std::string> FuncNames; // populated by v2 API; empty for v1 blobs
  };
  std::vector<BlobEntry> g_registered_blobs;
} // namespace

extern "C" void clang_runtime_specializer_register_blob_v2(
    const void* ptr, std::uint64_t len,
    const char* const* funcs, std::uint64_t nfuncs) {
  std::vector<std::string> Names;
  Names.reserve(nfuncs);
  for (std::uint64_t i = 0; i < nfuncs; ++i)
    Names.push_back(funcs[i]);
  g_registered_blobs.push_back({ptr, len, std::move(Names)});
}

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

  namespace detail {
    void removeJITDylibNoexcept(llvm::orc::ExecutionSession* ES,
                                llvm::orc::JITDylib* Dylib) noexcept {
      llvm::consumeError(ES->removeJITDylib(*Dylib));
    }
  }

  static ClangRuntimeSpecializer::LogLevel CurrentLogLevel = ClangRuntimeSpecializer::LogLevel::Debug;
  static ClangRuntimeSpecializer::JITModuleStats g_lastTransformStats;
  static std::vector<ClangRuntimeSpecializer::PassRecord> g_lastPassTrace;

  static size_t countNonDecl(const llvm::Module& M) {
    size_t n = 0;
    for (auto& F : M) if (!F.isDeclaration()) ++n;
    return n;
  }

  static size_t countInstrs(const llvm::Module& M) {
    size_t n = 0;
    for (auto& F : M) for (auto& BB : F) n += BB.size();
    return n;
  }

  void ClangRuntimeSpecializer::setLogLevel(const LogLevel Level) {
      CurrentLogLevel = Level;
  }

  ClangRuntimeSpecializer::LogLevel ClangRuntimeSpecializer::getLogLevel() {
      return CurrentLogLevel;
  }

  void ClangRuntimeSpecializer::log(const LogLevel Level, const llvm::Twine Message)
  {
    if (static_cast<int>(getLogLevel()) >= static_cast<int>(Level)) {
        log(Level, Message.str().c_str());
    }
  }

  void ClangRuntimeSpecializer::log(const LogLevel Level, const char* const Message) {
      if (static_cast<int>(getLogLevel()) >= static_cast<int>(Level)) {
        const char* LevelStr = "UNKNOWN";
        switch (Level) {
          case LogLevel::None: LevelStr = "NONE"; break;
          case LogLevel::Error: LevelStr = "ERROR"; break;
          case LogLevel::Warning: LevelStr = "WARNING"; break;
          case LogLevel::Info: LevelStr = "INFO"; break;
          case LogLevel::Debug: LevelStr = "DEBUG"; break;
        }
        std::fprintf(stdout, "%s: %s\n", LevelStr, Message);
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

  ClangRuntimeSpecializer::JITModuleStats ClangRuntimeSpecializer::getModuleStats() {
    JITModuleStats Stats;
    if (!Instance || Instance->BlobModules.empty()) return Stats;
    for (const auto& BM : Instance->BlobModules) {
      if (!BM) continue;
      for (auto& F : *BM) {
        if (!F.isDeclaration()) {
          ++Stats.FunctionCount;
          for (auto& BB : F) Stats.InstructionCount += BB.size();
        }
      }
    }
    for (const auto& Blob : g_registered_blobs)
      Stats.BitcodeSizeBytes += Blob.Len;
    return Stats;
  }

  ClangRuntimeSpecializer::JITModuleStats ClangRuntimeSpecializer::getLastTransformStats() {
    return g_lastTransformStats;
  }

  std::vector<ClangRuntimeSpecializer::PassRecord> ClangRuntimeSpecializer::getLastPassTrace() {
    return g_lastPassTrace;
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
    // Use the Large code model so JIT-compiled functions can reference host
    // process globals (e.g. sqlite3_temp_directory) that may be >2 GB away from
    // the JIT allocation.  Small/medium models generate RIP+32 fixups that
    // overflow when JIT memory is allocated in the upper address space.
    JTMBOrErr->setCodeModel(llvm::CodeModel::Large);

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

    // Always export instrumentation counters so they are available when per-call
    // options enable instrumentation, without requiring re-initialization.
    {
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
            // Per-pass trace infrastructure using PassInstrumentationCallbacks.
            // Tracks module-level passes only (function/loop-level are skipped via any_cast).
            std::vector<ClangRuntimeSpecializer::PassRecord> TraceRecords;
            std::string CurrentGroup;
            int CurrentFixpointIter = -1;

            std::string PendingName;
            uint64_t PendingFns = 0, PendingInstrs = 0, PendingBBs = 0;
            std::chrono::steady_clock::time_point PendingT0;

            auto countModule = [](const llvm::Module& Mod) -> std::tuple<uint64_t, uint64_t, uint64_t> {
              uint64_t Fns = 0, Instrs = 0, BBs = 0;
              for (const auto& F : Mod) {
                if (F.isDeclaration()) continue;
                ++Fns;
                for (const auto& BB : F) { ++BBs; Instrs += BB.size(); }
              }
              return {Fns, Instrs, BBs};
            };

            llvm::PassInstrumentationCallbacks PIC;
            PIC.registerBeforeNonSkippedPassCallback(
                [&](llvm::StringRef ID, llvm::Any IR) {
                  llvm::timeTraceProfilerBegin(ID, ""); // no-op if profiler not initialized
                  if (auto *MP = llvm::any_cast<const llvm::Module*>(&IR)) {
                    PendingName = std::string(ID);
                    auto [F, I, B] = countModule(**MP);
                    PendingFns = F; PendingInstrs = I; PendingBBs = B;
                    PendingT0 = std::chrono::steady_clock::now();
                  }
                });
            PIC.registerAfterPassCallback(
                [&](llvm::StringRef /*ID*/, llvm::Any IR,
                    const llvm::PreservedAnalyses& PA) {
                  llvm::timeTraceProfilerEnd(); // no-op if profiler not initialized
                  if (auto *MP = llvm::any_cast<const llvm::Module*>(&IR)) {
                    auto [FA, IA, BA] = countModule(**MP);
                    double Ms = std::chrono::duration<double, std::milli>(
                        std::chrono::steady_clock::now() - PendingT0).count();
                    TraceRecords.push_back({
                        PendingName, CurrentGroup, CurrentFixpointIter,
                        PendingFns, FA, PendingInstrs, IA, PendingBBs, BA,
                        Ms, !PA.areAllPreserved()
                    });
                  }
                });
            PIC.registerAfterPassInvalidatedCallback(
                [](llvm::StringRef, const llvm::PreservedAnalyses&) {
                  llvm::timeTraceProfilerEnd(); // balance begin for invalidated passes
                });

            llvm::PassBuilder PB(nullptr, {}, std::nullopt, &PIC);
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
            if (!Instance->CurrentCallOptions.KeepDebugInfo) {
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
                log(LogLevel::Debug, "Debug info stripped from JIT module");
              }
            }

            bool TraceEnabled = Optimize && !Instance->CurrentCallOptions.TimeTraceOutputPath.empty();
            if (TraceEnabled)
                llvm::timeTraceProfilerInitialize(/*TimeTraceGranularity=*/0, "JIT");

            if (Optimize) {
              // ── Diagnostic phase timer (stderr, always flushed) ──────────────
              auto T0_jit = std::chrono::steady_clock::now();
              auto phaseLog = [&](const char* phase, size_t instrs = 0) {
                double sec = std::chrono::duration<double>(
                    std::chrono::steady_clock::now() - T0_jit).count();
                if (instrs)
                  std::fprintf(stderr, "[JIT %.2fs] %s  (%zu instrs)\n", sec, phase, instrs);
                else
                  std::fprintf(stderr, "[JIT %.2fs] %s\n", sec, phase);
                std::fflush(stderr);
              };
              // ────────────────────────────────────────────────────────────────

              // Snapshot pre-prune stats
              g_lastTransformStats.FunctionCount    = countNonDecl(M);
              g_lastTransformStats.InstructionCount = countInstrs(M);
              phaseLog("start", g_lastTransformStats.InstructionCount);

              // Early pruning: remove definitions unreachable from the wrapper + vtable roots.
              // Safe: prepareModuleForJIT set the wrapper to ExternalLinkage; IRDumpingPass
              // (spec 004) set vtable functions to WeakODRLinkage at compile time — both are
              // GlobalDCE roots. AvailableExternally functions that get removed are resolved
              // from the host via DynamicLibrarySearchGenerator.
              if (Instance->CurrentCallOptions.EnableEarlyPrune) {
                CurrentGroup = "prune";
                llvm::ModulePassManager PruneMPM;
                PruneMPM.addPass(llvm::GlobalDCEPass());
                PruneMPM.run(M, MAM);
              }

              // Snapshot post-prune stats (reflects current state whether prune ran or not)
              g_lastTransformStats.FunctionCountAfterPrune    = countNonDecl(M);
              g_lastTransformStats.InstructionCountAfterPrune = countInstrs(M);
              phaseLog("after prune", g_lastTransformStats.InstructionCountAfterPrune);

              // Classify module size after pruning to gate expensive transforms.
              // Large modules (e.g. the sqlite3 amalgamation compiled as a single
              // translation unit) must use conservative inlining and loop-unroll
              // settings to avoid catastrophic IR explosion.
              const bool LargeModule =
                  g_lastTransformStats.InstructionCountAfterPrune > Instance->CurrentCallOptions.LargeModuleInstrThreshold;
              const int Pipeline = Instance->CurrentCallOptions.OptimizationPipelineToUse;

              // Validate pipeline index; clamp to 0 on out-of-range value.
              const int kNumPipelines = static_cast<int>(getRegisteredPipelines().size());
              if (Pipeline < 0 || Pipeline >= kNumPipelines) {
                log(LogLevel::Warning,
                    (llvm::Twine("Invalid OptimizationPipelineToUse value ") +
                     llvm::Twine(Pipeline) +
                     "; valid range is [0, " + llvm::Twine(kNumPipelines - 1) +
                     "]. Clamping pipeline to 0.").str());
              }
              const int ValidPipeline = (Pipeline < 0 || Pipeline >= kNumPipelines) ? 0 : Pipeline;

              // Warn about options that are inapplicable to the selected pipeline.
              if (ValidPipeline == 0 && Instance->CurrentCallOptions.FuncSpecMaxGroups > 0) {
                log(LogLevel::Warning,
                    "FuncSpecMaxGroups is ignored by the inlining pipeline (pipeline 0); "
                    "set OptimizationPipelineToUse=1 to use function-specialization cloning.");
              }
              if (ValidPipeline == 1 && Instance->CurrentCallOptions.MaxFixpointIterations > 1) {
                log(LogLevel::Warning,
                    "MaxFixpointIterations > 1 has no effect on the func-spec pipeline (pipeline 1); "
                    "the fixpoint loop is skipped for this pipeline.");
              }

              // Dispatch to the selected pipeline implementation.
              PipelineRunArgs PipeArgs{
                  M,
                  Instance->CurrentCallOptions,
                  PB,
                  MAM,
                  FAM,
                  CGAM,
                  LAM,
                  PIC,
                  CurrentGroup,
                  CurrentFixpointIter,
                  LargeModule,
              };
              if (auto Err = getRegisteredPipelines()[ValidPipeline].Run(PipeArgs)) {
                // Log but don't propagate — withModuleDo doesn't return Error.
                log(LogLevel::Error,
                    (llvm::Twine("Pipeline error: ") + llvm::toString(std::move(Err))).str());
              }
            }

            // After optimization, perform dynamic instruction counting instrumentation if enabled.
            bool Instrument = Instance->CurrentCallOptions.EnableInstructionInstrumentation;
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
                log(LogLevel::Debug, [&] {
                    return llvm::formatv("Optimized specialized function IR:\n{0}", printLLVM(&F)).str();
                });
              }
            }

            if (TraceEnabled) {
                std::error_code EC;
                llvm::raw_fd_ostream TraceOS(Instance->CurrentCallOptions.TimeTraceOutputPath, EC);
                if (!EC)
                    llvm::timeTraceProfilerWrite(TraceOS);
                llvm::timeTraceProfilerCleanup();
                Instance->CurrentCallOptions.TimeTraceOutputPath.clear();
            }

            g_lastPassTrace = std::move(TraceRecords);
          });
          return std::move(TSM);
        });

    if (g_registered_blobs.empty()) {
      throw ClangRuntimeSpecializerDumpedIRError(
          "No IR blobs registered. Was the binary compiled with the plugin?");
    }

    // Parse each blob into its own module (all sharing TSCtx's LLVMContext).
    Instance->TSCtx.withContextDo([&](llvm::LLVMContext *Ctx) {
      Instance->BlobModules.resize(g_registered_blobs.size());
      for (size_t i = 0; i < g_registered_blobs.size(); ++i) {
        const auto& Blob = g_registered_blobs[i];
        const llvm::StringRef Bytes(reinterpret_cast<const char*>(Blob.Ptr), Blob.Len);
        const llvm::MemoryBufferRef Buffer(Bytes, "RuntimeSpecializeableIR");
        llvm::Expected<std::unique_ptr<llvm::Module>> M = llvm::parseBitcodeFile(Buffer, *Ctx);
        if (!M) {
          const std::string Err = llvm::toString(M.takeError());
          throw ClangRuntimeSpecializerDumpedIRError("Failed to parse bitcode blob " +
                                                     std::to_string(i) + ": " + Err);
        }
        Instance->BlobModules[i] = std::move(*M);
      }
    });

    // Build funcName -> blob index map from the pre-registered name lists.
    for (size_t i = 0; i < g_registered_blobs.size(); ++i) {
      for (const auto& Name : g_registered_blobs[i].FuncNames)
        Instance->FuncToBlobIdx[Name] = i;
    }

    {
      size_t NumBlobs = g_registered_blobs.size();
      size_t TotalBytes = 0;
      for (const auto& Blob : g_registered_blobs) TotalBytes += Blob.Len;
      size_t NumFunctions = 0, NumInstructions = 0;
      for (const auto& BM : Instance->BlobModules) {
        if (BM) {
          NumFunctions += countNonDecl(*BM);
          NumInstructions += countInstrs(*BM);
        }
      }
      std::fprintf(stdout,
          "[CRS init] blobs=%zu  bitcode=%zu KB  functions=%zu  instructions=%zu\n",
          NumBlobs, TotalBytes / 1024, NumFunctions, NumInstructions);
    }

    return Instance.get();
  }

  ClangRuntimeSpecializer::ClangRuntimeSpecializer() : TSCtx(std::make_unique<llvm::LLVMContext>()) {}

  void ClangRuntimeSpecializer::checkInitialization(const char* const funcName) const {
    if (funcName == nullptr) {
      throw ClangRuntimeSpecializerError("funcName was null!");
    }
    if (BlobModules.empty()) {
      throw ClangRuntimeSpecializerError("No blob modules loaded! Was init() called?");
    }
    if (!JIT) {
      throw ClangRuntimeSpecializerError("JIT was not initialized!");
    }
  }

  llvm::Function* ClangRuntimeSpecializer::getTargetFunction(const char* const funcName) {
    std::string FuncNameStr(funcName);
    if (!FuncNameStr.empty() && FuncNameStr.front() == '&') {
      FuncNameStr.erase(0, 1);
    }
    auto It = FuncToBlobIdx.find(FuncNameStr);
    if (It == FuncToBlobIdx.end()) {
      throw ClangRuntimeSpecializerDumpedIRError(
          (llvm::Twine("Could not find function: ") + funcName +
           " (not found in any registered blob — was the binary compiled with the plugin?)").str());
    }
    llvm::Function* TargetFunc = BlobModules[It->second]->getFunction(FuncNameStr);
    if (TargetFunc == nullptr) {
      throw ClangRuntimeSpecializerDumpedIRError(
          (llvm::Twine("Could not find function: ") + funcName + " in its blob module").str());
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
    // All linkage invariants (vtable WeakODR, targets InternalLinkage, globals
    // AvailableExternally, ctors/dtors erased, zero-sized globals removed) are
    // established at compile time by IRDumpingPass (spec 004). The only JIT-time
    // fixup needed is making the wrapper symbol externally visible so JITLink can
    // export it for lookup.
    for (auto &F : M)
      if (F.getName() == WrapperName)
        F.setLinkage(llvm::GlobalValue::ExternalLinkage);
  }

  ClangRuntimeSpecializer::JITResult ClangRuntimeSpecializer::addModuleAndLookup(llvm::orc::ThreadSafeModule TSM,
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

    return {Addr, &Dylib};
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
