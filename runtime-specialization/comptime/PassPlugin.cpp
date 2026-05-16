#include "RuntimeSpecializerPass.h"
#include "../runtime/ClangRuntimeSpecializer/StaticMutabilityAnalysis.h"
#include "../runtime/ClangRuntimeSpecializer/DevirtualizeConstantVtableCalls.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "RuntimeSpecializer", LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
                // 1. Registrierung für RuntimeSpecializerPass
                PB.registerPipelineParsingCallback(
                    [](StringRef Name, ModulePassManager &MPM,
                       ArrayRef<PassBuilder::PipelineElement>) {

                      if (Name == "runtime-specialization-IR-rewriting") {
                        MPM.addPass(IRRewritingPass());
                        return true;
                      }
                      if (Name == "runtime-specialization-IR-dumping") {
                        MPM.addPass(IRDumpingPass());
                        return true;
                      }
                      if (Name == "devirt-constant-vtable-calls") {
                        MPM.addPass(clangRuntimeSpecializer::DevirtualizeConstantVtableCallsPass());
                        return true;
                      }
                      if (Name == "static-mutability-analysis") {
                        MPM.addPass(createModuleToFunctionPassAdaptor(clangRuntimeSpecializer::StaticMutabilityAnalysis::StaticMutabilityAnalysisPass()));
                        return true;
                      }
                      return false;
                    });

                // IRRewritingPass runs at pipeline start (before any -O3 optimisations)
                // so call-site detection sees unmodified IR shapes.
                PB.registerPipelineStartEPCallback(
                    [](ModulePassManager &MPM, OptimizationLevel Level) {
                      MPM.addPass(IRRewritingPass());
                    });

                // IRDumpingPass runs after all optimisations to clone and serialise
                // the post-optimisation module into a bitcode blob.
                PB.registerOptimizerLastEPCallback(
                    [](ModulePassManager &MPM, OptimizationLevel Level, ThinOrFullLTOPhase Phase) {
                      MPM.addPass(IRDumpingPass());
                    });
    }};
}