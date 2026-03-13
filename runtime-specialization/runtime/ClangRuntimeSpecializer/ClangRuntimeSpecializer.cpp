#include "ClangRuntimeSpecializer.h"

#include <algorithm>
#include <cstdio>
#include <dlfcn.h>
#include <utility>
#include <vector>

#include "llvm/Bitcode/BitcodeReader.h"
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
#include "VTableConstantFolding.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Analysis/InlineCost.h"

extern "C" void clang_runtime_specializer_link_anchor() {}

namespace {

  struct RuntimeSpecializableData {
    const void* Ptr;
    std::uint64_t Len;
  };

  RuntimeSpecializableData read_runtime_specializable_data() {
    void* ptrSym = dlsym(RTLD_DEFAULT, "RuntimeSpecializeableIR_ptr");
    void* lenSym = dlsym(RTLD_DEFAULT, "RuntimeSpecializeableIR_len");

    if (!ptrSym || !lenSym) {
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
      throw clangRuntimeSpecializer::ClangRuntimeSpecializerDumpedIRError("Failed to parse bitcode: " + Err);
    }

    return std::move(*M);
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

  void ClangRuntimeSpecializer::setLogLevel(LogLevel Level) {
      CurrentLogLevel = Level;
  }

  ClangRuntimeSpecializer::LogLevel ClangRuntimeSpecializer::getLogLevel() {
      return CurrentLogLevel;
  }

  __attribute__((always_inline))
  void ClangRuntimeSpecializer::log(LogLevel Level, const char* FuncName, const llvm::Twine Message)
  {
    if (static_cast<int>(getLogLevel()) >= static_cast<int>(Level)) {
        log(Level, FuncName, Message.str().c_str());
    }
  }

  __attribute__((always_inline))
  void ClangRuntimeSpecializer::log(LogLevel Level, const char* FuncName, const char* Message) {
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

  void ClangRuntimeSpecializer::printComparisonTable(const char* funcName, const InstructionCounts& Before, const InstructionCounts& After) {
      std::fprintf(stdout, "Comparing instruction counts from specializing %s:\n", funcName);
      std::fprintf(stdout, "%10s %10s %-15s %s\n", "before", "after", "instruction", "change");

      struct Row {
          const char* Name;
          uint64_t B;
          uint64_t A;
      };

      auto printRow = [](const Row& r) {
          int64_t Diff = static_cast<int64_t>(r.A) - static_cast<int64_t>(r.B);
          double Percent = (r.B == 0) ? (r.A == 0 ? 0.0 : 100.0) : (static_cast<double>(std::abs(Diff)) / r.B) * 100.0;
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

    RuntimeSpecializableData data = read_runtime_specializable_data();
    if (!data.Ptr || data.Len == 0) {
      std::fprintf(stderr, "No dumped IR found in the executable.\n");
      std::abort();
    }

    Instance.reset(new ClangRuntimeSpecializer);

    auto JITExp = llvm::orc::LLJITBuilder().create();
    if (!JITExp) {
      std::string ErrMsg = llvm::toString(JITExp.takeError());
      throw ClangRuntimeSpecializerError("Failed to create JIT: " + ErrMsg);
    }
    Instance->JIT = std::move(*JITExp);

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
              bool DebugInfoStripped = llvm::StripDebugInfo(M);
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

                // 2. Global optimizations - includes devirtualization
                // GlobalOpt can devirtualize calls when it knows the concrete type
                FixpointMPM.addPass(llvm::GlobalOptPass());

                // 2b. Whole-program devirtualization - attempts to devirtualize based on
                // vtable information. This can eliminate virtual calls when the set of
                // possible callees is known.
                FixpointMPM.addPass(llvm::WholeProgramDevirtPass());

                // 3. Dead code elimination - removes unreachable vtable entries
                FixpointMPM.addPass(llvm::GlobalDCEPass());

                // 4. Pre-inlining function-level optimizations
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

                // CUSTOM: VTable constant folding - replace vtable loads with constants
                // This is our custom pass that specifically handles the pattern:
                //   %obj = alloca; store @vtable, %obj; load %obj
                // It replaces the loads with the known constant vtable pointer.
                PostInlineFPM.addPass(llvm::VTableConstantFoldingPass());

                // InstCombine immediately after - fold loads of constant vtable pointers
                // and devirtualize the now-direct function pointer calls
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

    Instance->Module = parse_module_from_runtime_data(data, Instance->Context);

    return Instance.get();
  }

  ClangRuntimeSpecializer::ClangRuntimeSpecializer() {}

  void ClangRuntimeSpecializer::checkInitialization(const char* funcName) const {
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

  llvm::Function* ClangRuntimeSpecializer::getTargetFunction(const char* funcName) const {
    std::string FuncNameStr(funcName);
    if (!FuncNameStr.empty() && FuncNameStr.front() == '&') {
      FuncNameStr.erase(0, 1);
    }
    llvm::Function *TargetFunc = Module->getFunction(FuncNameStr);
    if (TargetFunc == nullptr) {
      throw ClangRuntimeSpecializerDumpedIRError((llvm::Twine("Could not find function: ") + funcName + " (It might be optimized out already by dead-code-elimination?)").str());
    }
    return TargetFunc;
  }

  void ClangRuntimeSpecializer::validateArgs(llvm::Function* TargetFunc, llvm::CallBase* CallSite, size_t NumArgs) const {
    unsigned CallArgCount = CallSite->arg_size();
    unsigned SkipArgs = 1; // 'this'
    if (CallArgCount != SkipArgs + static_cast<unsigned>(NumArgs))
    {
      throw ClangRuntimeSpecializerError((llvm::Twine("Unexpected callsite arg count. callArgCount=") + llvm::Twine(CallArgCount) + ", numArgs=" + llvm::Twine(NumArgs) + " (expected " + llvm::Twine(static_cast<unsigned>(NumArgs) + 1) + ")").str());
    }

    if (NumArgs != TargetFunc->arg_size())
    {
      throw ClangRuntimeSpecializerError((llvm::Twine("The number of args are incompatible! numArgs: ") + llvm::Twine(NumArgs) + ", TargetFunc->arg_size(): " + llvm::Twine(TargetFunc->arg_size())).str());
    }
  }

  std::string ClangRuntimeSpecializer::createUniqueWrapperName() {
    return "specialized_wrapper_" + std::to_string(++GlobalSpecializationCount) + "_" + std::to_string(reinterpret_cast<uintptr_t>(this));
  }

  void ClangRuntimeSpecializer::prepareModuleForJIT(llvm::Module& M, const std::string& WrapperName) {
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
    // If we are in baseline mode (no optimize), we want to make sure the target functions
    // are compiled and instrumented, so we use internal linkage.
    // Exception: Functions that are unlikely to be available externally (e.g., with custom
    // asm names, constructors/destructors, or member functions) should use internal linkage.
    for (auto &F : M) {
      if (F.getName() == WrapperName) {
         F.setLinkage(llvm::GlobalValue::ExternalLinkage);
         continue;
      }
      if (!F.isDeclaration()) {
         // Check if this function should be kept internal for the JIT
         bool KeepInternal = !Optimize;

         if (Optimize) {
             // Keep constructors, destructors, and functions with custom asm names internal
             // as they may not be exported from the host executable
             llvm::StringRef FName = F.getName();
             if (FName.contains("C1E") || FName.contains("C2E") ||  // Constructors
                 FName.contains("D1E") || FName.contains("D2E") ||  // Destructors
                 FName.contains("D0E") ||                           // Deleting destructor
                 !FName.starts_with("_Z")) {                         // Non-mangled (custom asm name)
                 KeepInternal = true;
             }
         }

         if (KeepInternal) {
             F.setLinkage(llvm::GlobalValue::InternalLinkage);
         } else {
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
  }

  uintptr_t ClangRuntimeSpecializer::addModuleAndLookup(llvm::orc::ThreadSafeModule TSM, const std::string& WrapperName) {
    if (auto Err = JIT->addIRModule(std::move(TSM))) {
      std::string ErrMsg = llvm::toString(std::move(Err));
      throw ClangRuntimeSpecializerError("Failed to add module to JIT: " + ErrMsg);
    }

    auto SpecializedFn = JIT->lookup(WrapperName);
    if (!SpecializedFn) {
      std::string ErrMsg = llvm::toString(SpecializedFn.takeError());
      throw ClangRuntimeSpecializerError("Failed to lookup wrapper: " + ErrMsg);
    }
    return SpecializedFn->getValue();
  }

  void ClangRuntimeSpecializer::encourageInlining(llvm::Function* F) {
    if (!F) return;
    F->removeFnAttr(llvm::Attribute::NoInline);
    F->removeFnAttr(llvm::Attribute::OptimizeNone);
    F->addFnAttr(llvm::Attribute::AlwaysInline);
  }

  static void fixupPointersInAlloca(llvm::Value* AllocaPtr, llvm::Constant* Initializer,
                                   const std::map<llvm::GlobalVariable*, llvm::AllocaInst*>& GVToAlloca,
                                   llvm::IRBuilder<>& Builder) {
    auto* Ty = Initializer->getType();
    if (auto* STy = llvm::dyn_cast<llvm::StructType>(Ty)) {
        for (unsigned i = 0; i < STy->getNumElements(); ++i) {
            llvm::Constant* Elem = Initializer->getAggregateElement(i);
            if (!Elem) continue;
            if (Elem->getType()->isPointerTy()) {
                auto* Stripped = Elem->stripPointerCasts();
                if (auto* GV = llvm::dyn_cast<llvm::GlobalVariable>(Stripped)) {
                    auto It = GVToAlloca.find(GV);
                    if (It != GVToAlloca.end()) {
                        llvm::Value* ElemPtr = Builder.CreateStructGEP(STy, AllocaPtr, i);
                        llvm::Value* NewAddr = It->second;
                        if (NewAddr->getType() != Elem->getType()) {
                            NewAddr = Builder.CreateBitCast(NewAddr, Elem->getType());
                        }
                        Builder.CreateStore(NewAddr, ElemPtr);
                    }
                }
            } else if (Elem->getType()->isAggregateType()) {
                llvm::Value* NestedAllocaPtr = Builder.CreateStructGEP(STy, AllocaPtr, i);
                fixupPointersInAlloca(NestedAllocaPtr, Elem, GVToAlloca, Builder);
            }
        }
    } else if (auto* ATy = llvm::dyn_cast<llvm::ArrayType>(Ty)) {
        for (unsigned i = 0; i < ATy->getNumElements(); ++i) {
             llvm::Constant* Elem = Initializer->getAggregateElement(i);
             if (!Elem) continue;
             if (Elem->getType()->isPointerTy()) {
                 auto* Stripped = Elem->stripPointerCasts();
                 if (auto* GV = llvm::dyn_cast<llvm::GlobalVariable>(Stripped)) {
                     auto It = GVToAlloca.find(GV);
                     if (It != GVToAlloca.end()) {
                         llvm::Value* ElemPtr = Builder.CreateConstGEP2_32(ATy, AllocaPtr, 0, i);
                         llvm::Value* NewAddr = It->second;
                         if (NewAddr->getType() != Elem->getType()) {
                             NewAddr = Builder.CreateBitCast(NewAddr, Elem->getType());
                         }
                         Builder.CreateStore(NewAddr, ElemPtr);
                     }
                 }
             } else if (Elem->getType()->isAggregateType()) {
                 llvm::Value* NestedAllocaPtr = Builder.CreateConstGEP2_32(ATy, AllocaPtr, 0, i);
                 fixupPointersInAlloca(NestedAllocaPtr, Elem, GVToAlloca, Builder);
             }
        }
    }
  }

  llvm::Function* ClangRuntimeSpecializer::buildWrapperIR(llvm::Module& M, const std::string& WrapperName, llvm::Function* TargetFunc,
                                                         llvm::ArrayRef<llvm::Constant*> SpecializedArgs, llvm::ArrayRef<WriteBack> WriteBacks,
                                                         const bool ForceInstrument, const bool Optimize) {
    llvm::LLVMContext& Ctx = M.getContext();
    llvm::FunctionType* FTy= llvm::FunctionType::get(TargetFunc->getReturnType(), false);


    llvm::Function* NewFunc = llvm::Function::Create(FTy, llvm::Function::ExternalLinkage, WrapperName, M);
    if (ForceInstrument) {
        NewFunc->addFnAttr("force-instrument");
    }
    if (!Optimize) {
        NewFunc->addFnAttr("force-no-optimize");
    }

    llvm::BasicBlock* Entry = llvm::BasicBlock::Create(Ctx, "entry", NewFunc);
    llvm::IRBuilder<> Builder(Entry);

    std::map<llvm::GlobalVariable*, llvm::AllocaInst*> GVToAlloca;
    std::vector<llvm::GlobalVariable*> Templates;
    for (auto &GV : M.globals()) {
        if (GV.getName().starts_with("__specialization_global_")) {
            auto* Alloca = Builder.CreateAlloca(GV.getValueType(), nullptr, GV.getName().str() + ".stack");
            GVToAlloca[&GV] = Alloca;
            Templates.push_back(&GV);
        }
    }

    // Initialize allocas and fixup internal pointers
    for (auto const& [GV, Alloca] : GVToAlloca) {
        Builder.CreateStore(GV->getInitializer(), Alloca);
    }
    for (auto const& [GV, Alloca] : GVToAlloca) {
        fixupPointersInAlloca(Alloca, GV->getInitializer(), GVToAlloca, Builder);
    }

    std::vector<llvm::Value*> CallArgs;
    for (auto* C : SpecializedArgs) {
        if (auto* BitCast = llvm::dyn_cast<llvm::ConstantExpr>(C)) {
            if (BitCast->getOpcode() == llvm::Instruction::BitCast) {
                if (auto* GV = llvm::dyn_cast<llvm::GlobalVariable>(BitCast->getOperand(0))) {
                    auto It = GVToAlloca.find(GV);
                    if (It != GVToAlloca.end()) {
                        CallArgs.push_back(Builder.CreateBitCast(It->second, BitCast->getType()));
                        continue;
                    }
                }
            }
        }
        if (auto* GV = llvm::dyn_cast<llvm::GlobalVariable>(C)) {
            auto It = GVToAlloca.find(GV);
            if (It != GVToAlloca.end()) {
                CallArgs.push_back(It->second);
                continue;
            }
        }
        CallArgs.push_back(C);
    }

    auto *CallInst = Builder.CreateCall(TargetFunc->getFunctionType(), TargetFunc, CallArgs);
    CallInst->setAttributes(TargetFunc->getAttributes());
    CallInst->addFnAttr(llvm::Attribute::AlwaysInline);

    for (const auto& WB : WriteBacks) {
        llvm::Type* Ty = llvm::Type::getInt64Ty(Ctx);
        llvm::Constant* OriginalPtrVal = llvm::ConstantInt::get(Ty, reinterpret_cast<uintptr_t>(WB.OriginalPtr));
        llvm::Value* OriginalPtr = Builder.CreateIntToPtr(OriginalPtrVal, Builder.getPtrTy());
        
        llvm::Value* Source = WB.Source;
        if (auto* GV = llvm::dyn_cast<llvm::GlobalVariable>(Source)) {
            auto It = GVToAlloca.find(GV);
            if (It != GVToAlloca.end()) {
                Source = It->second;
            }
        }
        Builder.CreateMemCpy(OriginalPtr, llvm::MaybeAlign(), Source, llvm::MaybeAlign(), WB.Size);
    }

    if (TargetFunc->getReturnType()->isVoidTy()) {
        Builder.CreateRetVoid();
    } else {
        Builder.CreateRet(CallInst);
    }

    // Clean up template globals
    for (auto* GV : Templates) {
        GV->replaceAllUsesWith(llvm::UndefValue::get(GV->getType()));
        GV->eraseFromParent();
    }

    return NewFunc;
}

  llvm::CallBase* ClangRuntimeSpecializer::findCallSpecializedFunctionInModule(const char* FunctionName, const char* UID) const {
    auto readCStringFromGlobal = [&](llvm::GlobalVariable *GV) -> std::string {
      if (!GV) return {};
      if (auto *CDA = llvm::dyn_cast<llvm::ConstantDataArray>(GV->getInitializer())) {
        if (CDA->isCString()) return CDA->getAsCString().str();
      }
      return {};
    };

    auto getAnnotationValueForFunction = [&](const llvm::Function &F, llvm::StringRef Key) -> std::optional<std::string> {
      llvm::GlobalVariable *AnnGV = Module->getGlobalVariable("llvm.global.annotations");
      if (!AnnGV || !AnnGV->hasInitializer()) return std::nullopt;
      auto *CA = llvm::dyn_cast<llvm::ConstantArray>(AnnGV->getInitializer());
      if (!CA) return std::nullopt;
      for (unsigned i = 0; i < CA->getNumOperands(); ++i) {
        auto *Elt = llvm::dyn_cast<llvm::ConstantStruct>(CA->getOperand(i));
        if (!Elt || Elt->getNumOperands() < 4) continue;
        // 0: ptr to annotated global (function), 1: ptr to anno string, 4: extra args (optional)
        llvm::Value *Op0 = Elt->getOperand(0);
        llvm::Value *Op1 = Elt->getOperand(1);
        llvm::Value *Op4 = (Elt->getNumOperands() >= 5) ? Elt->getOperand(4) : nullptr;

        if (auto *Op0C = llvm::dyn_cast<llvm::Constant>(Op0)) {
          if (auto *Target = Op0C->stripPointerCasts()) {
            if (Target == &F) {
              // Extract key
              std::string KeyStr;
              if (auto *Op1C = llvm::dyn_cast<llvm::Constant>(Op1)) {
                if (auto *KeyGV = llvm::dyn_cast<llvm::GlobalVariable>(Op1C->stripPointerCasts())) {
                  KeyStr = readCStringFromGlobal(KeyGV);
                }
              }

              if (KeyStr == Key) {
                // Try to extract first argument string from args struct if present
                if (Op4) {
                  if (auto *Op4C = llvm::dyn_cast<llvm::Constant>(Op4)) {
                    if (auto *ArgsGV = llvm::dyn_cast<llvm::GlobalVariable>(Op4C->stripPointerCasts())) {
                      if (auto *ArgsInit = llvm::dyn_cast<llvm::ConstantStruct>(ArgsGV->getInitializer())) {
                        if (ArgsInit->getNumOperands() >= 1) {
                          if (auto *Arg0C = llvm::dyn_cast<llvm::Constant>(ArgsInit->getOperand(0))) {
                            if (auto *StrGV = llvm::dyn_cast<llvm::GlobalVariable>(Arg0C->stripPointerCasts())) {
                              std::string V = readCStringFromGlobal(StrGV);
                              if (!V.empty()) return V;
                            }
                          }
                        }
                      }
                    }
                  }
                }
                // No value available
                return std::string();
              }
            }
          }
        }
      }
      return std::nullopt;
    };

    for (auto &F : *Module) {
      std::string FName = F.getName().str();
      if (FName.find(FunctionName) != std::string::npos) {
        auto AnnoVal = getAnnotationValueForFunction(F, CALL_SPECIALIZED_FUNC_NAME_ANNOTATION_NAME);
        if (!AnnoVal) continue;

        if (*AnnoVal == UID) {
          log(LogLevel::Info, "findCallSpecializedFunctionInModule",
              llvm::formatv("Found Function {0} responsible for specializing {1}", FName, UID));
          for (auto &U : F.uses()) {
            if (auto *CB = llvm::dyn_cast<llvm::CallBase>(U.getUser())) {
              if (CB->getCalledFunction() == &F) {
                return CB;
              }
            }
          }
        }
      }
    }

    throw ClangRuntimeSpecializerDumpedIRError("Could not find callsite for UID: " + std::string(UID));
  }

  // Helper function to identify concrete type from vtable pointer
  llvm::StructType* ClangRuntimeSpecializer::identifyPolymorphicType(llvm::Module& M, const void* ObjectPtr) {
    // Read the vtable pointer from the object (first pointer in memory layout)
    const void* const* VTablePtrLoc = static_cast<const void* const*>(ObjectPtr);
    const void* VTablePtr = *VTablePtrLoc;

    log(LogLevel::Debug, "identifyPolymorphicType", [&] {
        return llvm::formatv("Identifying type for vtable pointer: {0}", VTablePtr).str();
    });

    // Iterate through all global variables to find matching vtables
    for (llvm::GlobalVariable& GV : M.globals()) {
      if (!GV.hasName()) continue;

      llvm::StringRef Name = GV.getName();

      // Look for vtable symbols (mangled names starting with _ZTV)
      if (Name.starts_with("_ZTV")) {
        // Get the runtime address of this vtable
        void* GVAddr = dlsym(RTLD_DEFAULT, Name.str().c_str());
        if (!GVAddr) continue;

        // Vtables have offset +16 bytes (past type_info pointer)
        const void* VTableStart = static_cast<const char*>(GVAddr) + 16;

        if (VTableStart == VTablePtr) {
          // Found matching vtable! Extract the class name from mangled name
          // _ZTV<len><name> -> extract the class name
          std::string MangledName = Name.str().substr(4); // Skip "_ZTV"

          log(LogLevel::Debug, "identifyPolymorphicType", [&] {
              return llvm::formatv("Found matching vtable: {0}", Name).str();
          });

          // Try to find corresponding struct type in module
          // Try different naming conventions: class.<name>, struct.<name>, <name>
          for (llvm::StructType* STy : M.getIdentifiedStructTypes()) {
            llvm::StringRef StructName = STy->getName();

            // Match if the struct name contains the class name from vtable
            if (StructName.contains(MangledName) ||
                (StructName.starts_with("class.") && Name.contains(StructName.substr(6))) ||
                (StructName.starts_with("struct.") && Name.contains(StructName.substr(7)))) {
              log(LogLevel::Debug, "identifyPolymorphicType", [&] {
                  return llvm::formatv("Identified concrete type: {0}", printLLVM(STy)).str();
              });
              return STy;
            }
          }
        }
      }
    }

    log(LogLevel::Debug, "identifyPolymorphicType", [&] {
        return "Could not identify concrete type from vtable pointer";
    });

    return nullptr;
  }

  llvm::Constant* ClangRuntimeSpecializer::serializeValueToIR(llvm::Module& M, llvm::Type* Type, const void* ValuePtr) {
    log(LogLevel::Debug, "serializeValueToIR", [&] {
        return llvm::formatv("Serializing value of type {0}", printLLVM(Type)).str();
    });

    llvm::Constant* Result = nullptr;
    llvm::LLVMContext& Context = M.getContext();
    if (Type->isIntegerTy()) {
      unsigned BitWidth = Type->getIntegerBitWidth();
      if (BitWidth <= 64) {
        uint64_t Val = 0;
        std::memcpy(&Val, ValuePtr, (BitWidth + 7) / 8);
        Result = llvm::ConstantInt::get(Type, Val);
      } else {
        throw ClangRuntimeSpecializerArgSerializationError("Integers > 64 bits are not supported yet.");
      }
    } else if (Type->isFloatTy()) {
      float Val;
      std::memcpy(&Val, ValuePtr, sizeof(float));
      Result = llvm::ConstantFP::get(Context, llvm::APFloat(Val));
    } else if (Type->isDoubleTy()) {
      double Val;
      std::memcpy(&Val, ValuePtr, sizeof(double));
      Result = llvm::ConstantFP::get(Context, llvm::APFloat(Val));
    } else if (Type->isPointerTy()) {


      // Read the pointer value
      uintptr_t Val;
      std::memcpy(&Val, ValuePtr, sizeof(uintptr_t));
      const void* PointedToPtr = reinterpret_cast<const void*>(Val);

      // Check if this is a null pointer
      if (Val == 0) {
        Result = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(Type));
      } else {
        // Check if this pointer points to a known vtable
        for (llvm::GlobalVariable& GV : M.globals()) {
          if (GV.getName().starts_with("_ZTV")) {
            void* GVAddr = dlsym(RTLD_DEFAULT, GV.getName().str().c_str());
            if (GVAddr) {
              const void* VTableStart = static_cast<const char*>(GVAddr) + 16;
              if (VTableStart == PointedToPtr) {
                log(LogLevel::Debug, "serializeValueToIR", [&] {
                  return llvm::formatv("Identified vtable pointer for {0}", GV.getName()).str();
                });
                // Robustly create a GEP to the vtable entry (offset 16 bytes)
                // We use a byte-wise GEP because vtable types can vary (struct vs array)
                llvm::Type* I8 = llvm::Type::getInt8Ty(Context);
                llvm::Type* I64 = llvm::Type::getInt64Ty(Context);
                llvm::Constant* Offset = llvm::ConstantInt::get(I64, 16);
                llvm::Constant* GEP = llvm::ConstantExpr::getGetElementPtr(
                    I8, &GV, Offset);
                Result = llvm::ConstantExpr::getBitCast(GEP, Type);
                break;
              }
            }
          }
        }

        if (!Result) {
          // Try to identify if this pointer points to a polymorphic object
          // by attempting to read a vtable pointer from it
          llvm::StructType* ConcreteType = nullptr;
          bool IsPolymorphic = false;

        // Attempt to read the first 8 bytes to see if it looks like a valid vtable pointer
        // This is a heuristic - we check if the value looks like a code/data pointer
        try {
          const void* PotentialVTable = *static_cast<const void* const*>(PointedToPtr);

          // Check if this address is plausible (not null, not obviously invalid)
          uintptr_t VTableAddr = reinterpret_cast<uintptr_t>(PotentialVTable);
          if (VTableAddr > 0x1000 && VTableAddr < 0x7fffffffffff) {
            // Try to identify the concrete type from the vtable
            ConcreteType = identifyPolymorphicType(M, PointedToPtr);
            if (ConcreteType) {
              IsPolymorphic = true;

              log(LogLevel::Debug, "serializeValueToIR", [&] {
                  return llvm::formatv("Pointer field points to polymorphic object of type: {0}",
                                       printLLVM(ConcreteType)).str();
              });
            }
          }
        } catch (...) {
          // If we fail to read memory, it's not a valid polymorphic object
          IsPolymorphic = false;
        }

        if (IsPolymorphic && ConcreteType) {
          // Recursively serialize the pointed-to polymorphic object
          llvm::Constant* SerializedObject = serializeValueToIR(M, ConcreteType, PointedToPtr);

          if (!SerializedObject) {
            throw ClangRuntimeSpecializerArgSerializationError("Failed to serialize nested polymorphic object");
          }

          // Create a global variable for the nested object
          auto *GV = new llvm::GlobalVariable(M, ConcreteType, true, // isConstant = true
                                              llvm::GlobalValue::InternalLinkage,
                                              SerializedObject, "__specialization_global_nested_polymorphic_object");

          // Return a pointer to the global, bitcast if necessary
          Result = llvm::ConstantExpr::getBitCast(GV, Type);
        } else {
          // Fall back to opaque pointer serialization
          llvm::Type* Ty = llvm::Type::getInt64Ty(Context);
          llvm::Constant* IntVal = llvm::ConstantInt::get(Ty, static_cast<uint64_t>(Val));
          Result = llvm::ConstantExpr::getIntToPtr(IntVal, Type);
        }
      }
      }
    } else if (Type->isStructTy()) {
      llvm::StructType* STy = llvm::cast<llvm::StructType>(Type);
      const llvm::DataLayout& DL = M.getDataLayout();
      const llvm::StructLayout* SL = DL.getStructLayout(STy);

      std::vector<llvm::Constant*> Elements;
      for (unsigned i = 0; i < STy->getNumElements(); ++i) {
        llvm::Type* ElemTy = STy->getElementType(i);
        uint64_t Offset = SL->getElementOffset(i);
        const void* ElemPtr = static_cast<const char*>(ValuePtr) + Offset;

        llvm::Constant* ElemVal = serializeValueToIR(M, ElemTy, ElemPtr);
        if (ElemVal) {
          Elements.push_back(ElemVal);
        } else {
          throw ClangRuntimeSpecializerArgSerializationError("Failed to serialize struct element " + std::to_string(i));
        }
      }
      Result = llvm::ConstantStruct::get(STy, Elements);
    } else if (Type->isArrayTy()) {
      llvm::ArrayType* ATy = llvm::cast<llvm::ArrayType>(Type);
      llvm::Type* ElemTy = ATy->getElementType();
      const llvm::DataLayout& DL = M.getDataLayout();
      uint64_t ElemSize = DL.getTypeAllocSize(ElemTy);

      std::vector<llvm::Constant*> Elements;
      for (uint64_t i = 0; i < ATy->getNumElements(); ++i) {
        const void* ElemPtr = static_cast<const char*>(ValuePtr) + (i * ElemSize);
        llvm::Constant* ElemVal = serializeValueToIR(M, ElemTy, ElemPtr);
        if (ElemVal) {
          Elements.push_back(ElemVal);
        } else {
          throw ClangRuntimeSpecializerArgSerializationError("Failed to serialize array element " + std::to_string(i));
        }
      }
      Result = llvm::ConstantArray::get(ATy, Elements);
    } else {
      throw ClangRuntimeSpecializerArgSerializationError("Unsupported type for serialization: " + printLLVM(Type));
    }

    if (Result) {
        log(LogLevel::Debug, "serializeValueToIR", [&] {
            return llvm::formatv("Serialized to LLVM value: {0}", printLLVM(Result)).str();
        });
    }

    return Result;
  }

  ClangRuntimeSpecializer::~ClangRuntimeSpecializer() = default;

} // namespace clangRuntimeSpecializer
