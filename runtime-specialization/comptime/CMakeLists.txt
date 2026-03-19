add_llvm_pass_plugin(LLVMRuntimeSpecializationComptimePlugin
  PassPlugin.cpp
  IRDumpingPass.cpp
        # TODO we should refactor this into a shared module
  ../runtime/ClangRuntimeSpecializer/VTableConstantFolding.cpp
  ../runtime/ClangRuntimeSpecializer/StaticMutabilityAnalysis.cpp
  )
