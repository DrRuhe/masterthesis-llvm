add_llvm_pass_plugin(LLVMRuntimeSpecializationComptimePlugin
  PassPlugin.cpp
  IRRewritingPass.cpp
  IRDumpingPass.cpp
        # TODO we should refactor this into a shared module
  ../runtime/ClangRuntimeSpecializer/DevirtualizeConstantVtableCalls.cpp
  ../runtime/ClangRuntimeSpecializer/StaticMutabilityAnalysis.cpp
  )
