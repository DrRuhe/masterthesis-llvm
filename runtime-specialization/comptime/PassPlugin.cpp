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


                // 2. Automatisches Einhängen des Finalizers nach allen Optimierungen
                PB.registerOptimizerLastEPCallback(
                    [](ModulePassManager &MPM, OptimizationLevel Level, ThinOrFullLTOPhase Phase) {
                      MPM.addPass(IRDumpingPass());
                    });
    }};
}