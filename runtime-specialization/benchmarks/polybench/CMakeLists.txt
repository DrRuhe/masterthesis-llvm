set(LLVM_LINK_COMPONENTS)

set(POLYBENCH_OBJ "${CMAKE_CURRENT_BINARY_DIR}/polybench_bench.o")

add_custom_command(
    OUTPUT ${POLYBENCH_OBJ}
    COMMAND ${CLANG_EXE} -x c++ -g -O3 ${CXX_FLAGS_LIST}
            -Wno-unknown-pragmas -Wno-misleading-indentation -Wno-unused-variable
            -I${CMAKE_CURRENT_SOURCE_DIR}
            -I${CMAKE_CURRENT_SOURCE_DIR}/..
            -I${CMAKE_CURRENT_SOURCE_DIR}/../../runtime/ClangRuntimeSpecializer
            -I${CMAKE_CURRENT_SOURCE_DIR}/PolyBenchC-4.2.1-master/utilities
            -I${LLVM_MAIN_INCLUDE_DIR}
            -I${LLVM_BINARY_DIR}/include
            -I${LLVM_THIRD_PARTY_DIR}/benchmark/include
            -fpass-plugin=${PLUGIN_LIB}
            -fexceptions -frtti
            -c ${CMAKE_CURRENT_SOURCE_DIR}/polybench_bench.cpp
            -o ${POLYBENCH_OBJ}
    DEPENDS polybench_bench.cpp LLVMRuntimeSpecializationComptimePlugin clang
    COMMENT "Compiling polybench_bench with project clang and plugin"
)

set(POLYBENCH_IDE_STUB "${CMAKE_CURRENT_BINARY_DIR}/polybench_ide_stub.cpp")
file(WRITE "${POLYBENCH_IDE_STUB}" "")

set_source_files_properties(polybench_bench.cpp PROPERTIES HEADER_FILE_ONLY ON)
add_benchmark(PolyBenchBenchmark PARTIAL_SOURCES_INTENDED
    ${POLYBENCH_OBJ} polybench_bench.cpp ${POLYBENCH_IDE_STUB})

target_include_directories(PolyBenchBenchmark PRIVATE
    ${LLVM_THIRD_PARTY_DIR}/benchmark/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../runtime/ClangRuntimeSpecializer
    ${CMAKE_CURRENT_SOURCE_DIR}/PolyBenchC-4.2.1-master/utilities
    ${LLVM_MAIN_INCLUDE_DIR}
    ${LLVM_BINARY_DIR}/include
)
target_link_libraries(PolyBenchBenchmark PRIVATE ClangRuntimeSpecializer benchmark)
set_target_properties(PolyBenchBenchmark PROPERTIES ENABLE_EXPORTS ON)
if(NOT APPLE AND NOT MSVC)
    target_link_options(PolyBenchBenchmark PRIVATE "-Wl,--export-dynamic")
endif()
