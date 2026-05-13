set(LLVM_LINK_COMPONENTS)

set(KERNELS_OBJ "${CMAKE_CURRENT_BINARY_DIR}/UC7Kernels.o")
add_custom_command(OUTPUT ${KERNELS_OBJ}
    COMMAND ${CLANG_EXE} -x c++ -g -O0 ${CXX_FLAGS_LIST}
            -I${CMAKE_CURRENT_SOURCE_DIR}
            -I${CMAKE_CURRENT_SOURCE_DIR}/../../../runtime/ClangRuntimeSpecializer
            -I${LLVM_MAIN_INCLUDE_DIR}
            -I${LLVM_BINARY_DIR}/include
            -fpass-plugin=${PLUGIN_LIB}
            -fexceptions -frtti
            -c ${CMAKE_CURRENT_SOURCE_DIR}/UC7Kernels.cpp
            -o ${KERNELS_OBJ}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/UC7Kernels.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/UC7Kernels.h
            LLVMRuntimeSpecializationComptimePlugin clang
    COMMENT "Compiling UC7Kernels with plugin")

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

set_source_files_properties(UC7Kernels.cpp UC7Benchmark.cpp PROPERTIES HEADER_FILE_ONLY ON)

add_benchmark(UC7DfaRegex PARTIAL_SOURCES_INTENDED
    ${KERNELS_OBJ} ${BENCH_OBJ} UC7Kernels.cpp UC7Benchmark.cpp ${IDE_STUB})

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
set(UC7_KERNELS_OBJ "${KERNELS_OBJ}"                               PARENT_SCOPE)
set(UC7_BENCH_OBJ   "${BENCH_OBJ}"                                 PARENT_SCOPE)
set(UC7_SRC_DIR     "${CMAKE_CURRENT_SOURCE_DIR}"                  PARENT_SCOPE)
set(UC7_KERNELS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/UC7Kernels.cpp"   PARENT_SCOPE)
set(UC7_BENCH_SRC   "${CMAKE_CURRENT_SOURCE_DIR}/UC7Benchmark.cpp" PARENT_SCOPE)
