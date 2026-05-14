set(LLVM_LINK_COMPONENTS)

# Define UC7 kernel source files (absolute paths required for AllBenchmarks re-link)
set(UC7_KERNEL_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/UC7EmailMatchLowKernels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UC7EmailMatchTradeoffKernels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UC7EmailMatchAbstractKernels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UC7UrlMatchLowKernels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UC7UrlMatchTradeoffKernels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UC7UrlMatchAbstractKernels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UC7MultiPatternMatchLowKernels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UC7MultiPatternMatchTradeoffKernels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UC7MultiPatternMatchAbstractKernels.cpp
)

# Compile each kernel source to object file
set(UC7_KERNELS_OBJS)
foreach(KERNEL_SRC ${UC7_KERNEL_SRCS})
    get_filename_component(KERN_STEM "${KERNEL_SRC}" NAME_WE)
    set(KERNEL_OBJ "${CMAKE_CURRENT_BINARY_DIR}/${KERN_STEM}.o")
    list(APPEND UC7_KERNELS_OBJS "${KERNEL_OBJ}")

    add_custom_command(OUTPUT ${KERNEL_OBJ}
        COMMAND ${CLANG_EXE} -x c++ -g -O0 ${CXX_FLAGS_LIST}
                -I${CMAKE_CURRENT_SOURCE_DIR}
                -I${CMAKE_CURRENT_SOURCE_DIR}/../../../runtime/ClangRuntimeSpecializer
                -I${LLVM_MAIN_INCLUDE_DIR}
                -I${LLVM_BINARY_DIR}/include
                -fpass-plugin=${PLUGIN_LIB}
                -fexceptions -frtti
                -c ${KERNEL_SRC}
                -o ${KERNEL_OBJ}
        DEPENDS ${KERNEL_SRC}
                LLVMRuntimeSpecializationComptimePlugin clang
        COMMENT "Compiling ${KERN_STEM} with plugin")
endforeach()

set(BENCH_OBJ "${CMAKE_CURRENT_BINARY_DIR}/UC7Benchmark.o")
add_custom_command(OUTPUT ${BENCH_OBJ}
    COMMAND ${CLANG_EXE} -x c++ -g -O3 ${CXX_FLAGS_LIST}
            -I${CMAKE_CURRENT_SOURCE_DIR}
            -I${CMAKE_CURRENT_SOURCE_DIR}/../../
            -I${CMAKE_CURRENT_SOURCE_DIR}/../../../runtime/ClangRuntimeSpecializer
            -I${LLVM_MAIN_INCLUDE_DIR}
            -I${LLVM_BINARY_DIR}/include
            -I${LLVM_THIRD_PARTY_DIR}/benchmark/include
            -fpass-plugin=${PLUGIN_LIB}
            -fexceptions -frtti
            -c ${CMAKE_CURRENT_SOURCE_DIR}/UC7Benchmark.cpp
            -o ${BENCH_OBJ}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/UC7Benchmark.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/UC7Kernels.h
            LLVMRuntimeSpecializationComptimePlugin clang
    COMMENT "Compiling UC7Benchmark with plugin")

set(IDE_STUB "${CMAKE_CURRENT_BINARY_DIR}/UC7_ide_stub.cpp")
file(WRITE "${IDE_STUB}" "")

set_source_files_properties(${UC7_KERNEL_SRCS} UC7Benchmark.cpp PROPERTIES HEADER_FILE_ONLY ON)

add_benchmark(UC7DfaRegex PARTIAL_SOURCES_INTENDED
    ${UC7_KERNELS_OBJS} ${BENCH_OBJ} ${UC7_KERNEL_SRCS} UC7Benchmark.cpp ${IDE_STUB})

target_include_directories(UC7DfaRegex PRIVATE
    ${LLVM_THIRD_PARTY_DIR}/benchmark/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../runtime/ClangRuntimeSpecializer
    ${LLVM_MAIN_INCLUDE_DIR}
    ${LLVM_BINARY_DIR}/include
)

target_link_libraries(UC7DfaRegex PRIVATE ClangRuntimeSpecializer benchmark)
set_target_properties(UC7DfaRegex PROPERTIES ENABLE_EXPORTS ON)
if(NOT APPLE AND NOT MSVC)
    target_link_options(UC7DfaRegex PRIVATE "-Wl,--export-dynamic")
endif()

# Export to use-cases/CMakeLists.txt scope for AllBenchmarks re-linking
set(UC7_KERNELS_OBJS "${UC7_KERNELS_OBJS}"                         PARENT_SCOPE)
set(UC7_BENCH_OBJ    "${BENCH_OBJ}"                                PARENT_SCOPE)
set(UC7_SRC_DIR      "${CMAKE_CURRENT_SOURCE_DIR}"                 PARENT_SCOPE)
set(UC7_KERNELS_SRCS "${UC7_KERNEL_SRCS}"                          PARENT_SCOPE)
set(UC7_BENCH_SRC    "${CMAKE_CURRENT_SOURCE_DIR}/UC7Benchmark.cpp" PARENT_SCOPE)
