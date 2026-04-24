add_library(ClangRuntimeSpecializer SHARED
        ClangRuntimeSpecializer.cpp
        JITPipelineInlining.cpp
        JITPipelineFuncSpec.cpp
        JITPipelineRegistry.cpp
        DevirtualizeConstantVtableCalls.cpp
        StaticMutabilityAnalysis.cpp
        InvariantLoadToConstant.cpp
        ConstantArgFunctionSpecializationPass.cpp
)

target_include_directories(ClangRuntimeSpecializer
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Optional, aber meist sinnvoll für DSOs
set_target_properties(ClangRuntimeSpecializer PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        POSITION_INDEPENDENT_CODE ON
)

if(NOT LLVM_ENABLE_RTTI)
    target_compile_options(ClangRuntimeSpecializer PRIVATE -fno-rtti)
endif()

llvm_config(ClangRuntimeSpecializer USE_SHARED
        core
        bitreader
        support
        orcjit
        passes
        scalaropts
        instcombine
        analysis
        transformutils
        x86codegen
        x86desc
        x86info
        mcdisassembler
        x86disassembler
)

# llvm_config links LLVM privately; consumers that use inline LLVM APIs from
# ClangRuntimeSpecializer.h also need libLLVM.so transitively.
target_link_libraries(ClangRuntimeSpecializer PUBLIC LLVM)