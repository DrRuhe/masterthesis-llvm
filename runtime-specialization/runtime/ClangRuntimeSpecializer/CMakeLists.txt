add_library(ClangRuntimeSpecializer SHARED
        ClangRuntimeSpecializer.cpp
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