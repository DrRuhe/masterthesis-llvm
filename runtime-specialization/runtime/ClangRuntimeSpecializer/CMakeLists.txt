add_library(ClangRuntimeSpecializer SHARED
        ClangRuntimeSpecializer.cpp
        VTableConstantFolding.cpp
        VTableEscapeAnalysis.cpp
        StaticMutabilityAnalysis.cpp
        InvariantLoadToConstant.cpp
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

target_link_libraries(ClangRuntimeSpecializer
        PUBLIC
        LLVMCore
        LLVMBitReader
        LLVMSupport
        LLVMOrcJIT
        LLVMPasses
        LLVMScalarOpts
        LLVMInstCombine
        LLVMAnalysis
        LLVMTransformUtils
        LLVMX86CodeGen
        LLVMX86Desc
        LLVMX86Info
)