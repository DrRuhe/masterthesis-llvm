set(LLVM_LINK_COMPONENTS)

# List of all kernel sources for this UC (one per variant × level)
set(UC2_KERNEL_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/UC2BoxFilterLowKernels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UC2BoxFilterTradeoffKernels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UC2BoxFilterAbstractKernels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UC2EdgeDetectionLowKernels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UC2EdgeDetectionTradeoffKernels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UC2EdgeDetectionAbstractKernels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UC2SeparableGaussianLowKernels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UC2SeparableGaussianTradeoffKernels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UC2SeparableGaussianAbstractKernels.cpp
)

set(_KERN_OBJS "")
foreach(KERN_SRC IN LISTS UC2_KERNEL_SRCS)
    get_filename_component(KERN_STEM "${KERN_SRC}" NAME_WE)
    set(_KERN_OBJ "${CMAKE_CURRENT_BINARY_DIR}/${KERN_STEM}.o")
    add_custom_command(OUTPUT ${_KERN_OBJ}
        COMMAND ${CLANG_EXE} -x c++ -g -O3 ${CXX_FLAGS_LIST}
                -I${CMAKE_CURRENT_SOURCE_DIR}
                -I${CMAKE_CURRENT_SOURCE_DIR}/../../../runtime/ClangRuntimeSpecializer
                -I${LLVM_MAIN_INCLUDE_DIR}
                -I${LLVM_BINARY_DIR}/include
                -fpass-plugin=${PLUGIN_LIB}
                -fexceptions -frtti
                -c ${KERN_SRC} -o ${_KERN_OBJ}
        DEPENDS ${KERN_SRC} ${CMAKE_CURRENT_SOURCE_DIR}/UC2Kernels.h
                LLVMRuntimeSpecializationComptimePlugin clang
        COMMENT "Compiling ${KERN_STEM} with plugin")
    list(APPEND _KERN_OBJS ${_KERN_OBJ})
endforeach()

set(BENCH_OBJ "${CMAKE_CURRENT_BINARY_DIR}/UC2Benchmark.o")
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
            -c ${CMAKE_CURRENT_SOURCE_DIR}/UC2Benchmark.cpp
            -o ${BENCH_OBJ}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/UC2Benchmark.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/UC2Kernels.h
            LLVMRuntimeSpecializationComptimePlugin clang
    COMMENT "Compiling UC2Benchmark with plugin")

set(IDE_STUB "${CMAKE_CURRENT_BINARY_DIR}/UC2_ide_stub.cpp")
file(WRITE "${IDE_STUB}" "")

set_source_files_properties(UC2Benchmark.cpp PROPERTIES HEADER_FILE_ONLY ON)

add_benchmark(UC2Convolution PARTIAL_SOURCES_INTENDED
    ${_KERN_OBJS} ${BENCH_OBJ} UC2Benchmark.cpp ${IDE_STUB})

target_include_directories(UC2Convolution PRIVATE
    ${LLVM_THIRD_PARTY_DIR}/benchmark/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../runtime/ClangRuntimeSpecializer
    ${LLVM_MAIN_INCLUDE_DIR}
    ${LLVM_BINARY_DIR}/include
)

target_link_libraries(UC2Convolution PRIVATE ClangRuntimeSpecializer benchmark)
set_target_properties(UC2Convolution PROPERTIES ENABLE_EXPORTS ON)
if(NOT APPLE AND NOT MSVC)
    target_link_options(UC2Convolution PRIVATE "-Wl,--export-dynamic")
endif()

# Export to parent (use-cases/) scope
set(UC2_KERNELS_OBJS "${_KERN_OBJS}"             PARENT_SCOPE)
set(UC2_KERNELS_SRCS "${UC2_KERNEL_SRCS}"       PARENT_SCOPE)
set(UC2_SRC_DIR      "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)
set(UC2_BENCH_SRC    "${CMAKE_CURRENT_SOURCE_DIR}/UC2Benchmark.cpp" PARENT_SCOPE)
set(UC2_BENCH_OBJ    "${BENCH_OBJ}"              PARENT_SCOPE)
