set(LLVM_LINK_COMPONENTS)

set(KERNELS_OBJ "${CMAKE_CURRENT_BINARY_DIR}/UC14Kernels.o")
add_custom_command(OUTPUT ${KERNELS_OBJ}
    COMMAND ${CLANG_EXE} -x c++ -g -O0 ${CXX_FLAGS_LIST}
            -I${CMAKE_CURRENT_SOURCE_DIR}
            -I${CMAKE_CURRENT_SOURCE_DIR}/../../../runtime/ClangRuntimeSpecializer
            -I${LLVM_MAIN_INCLUDE_DIR}
            -I${LLVM_BINARY_DIR}/include
            -fpass-plugin=${PLUGIN_LIB}
            -fexceptions -frtti
            -c ${CMAKE_CURRENT_SOURCE_DIR}/UC14Kernels.cpp
            -o ${KERNELS_OBJ}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/UC14Kernels.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/UC14Kernels.h
            LLVMRuntimeSpecializationComptimePlugin clang
    COMMENT "Compiling UC14Kernels with plugin")

set(BENCH_OBJ "${CMAKE_CURRENT_BINARY_DIR}/UC14Benchmark.o")
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
            -c ${CMAKE_CURRENT_SOURCE_DIR}/UC14Benchmark.cpp
            -o ${BENCH_OBJ}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/UC14Benchmark.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/UC14Kernels.h
            LLVMRuntimeSpecializationComptimePlugin clang
    COMMENT "Compiling UC14Benchmark with plugin")

set(IDE_STUB "${CMAKE_CURRENT_BINARY_DIR}/UC14_ide_stub.cpp")
file(WRITE "${IDE_STUB}" "")

set_source_files_properties(UC14Kernels.cpp UC14Benchmark.cpp PROPERTIES HEADER_FILE_ONLY ON)

add_benchmark(UC14Sort PARTIAL_SOURCES_INTENDED
    ${KERNELS_OBJ} ${BENCH_OBJ} UC14Kernels.cpp UC14Benchmark.cpp ${IDE_STUB})

target_include_directories(UC14Sort PRIVATE
    ${LLVM_THIRD_PARTY_DIR}/benchmark/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../runtime/ClangRuntimeSpecializer
    ${LLVM_MAIN_INCLUDE_DIR}
    ${LLVM_BINARY_DIR}/include
)

target_link_libraries(UC14Sort PRIVATE ClangRuntimeSpecializer benchmark)
set_target_properties(UC14Sort PROPERTIES ENABLE_EXPORTS ON)
if(NOT APPLE AND NOT MSVC)
    target_link_options(UC14Sort PRIVATE "-Wl,--export-dynamic")
endif()

set(UC14_KERNELS_OBJ "${KERNELS_OBJ}"                                PARENT_SCOPE)
set(UC14_BENCH_OBJ   "${BENCH_OBJ}"                                  PARENT_SCOPE)
set(UC14_SRC_DIR     "${CMAKE_CURRENT_SOURCE_DIR}"                   PARENT_SCOPE)
set(UC14_KERNELS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/UC14Kernels.cpp"   PARENT_SCOPE)
set(UC14_BENCH_SRC   "${CMAKE_CURRENT_SOURCE_DIR}/UC14Benchmark.cpp" PARENT_SCOPE)
