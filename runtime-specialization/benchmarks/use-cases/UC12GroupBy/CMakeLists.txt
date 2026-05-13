set(LLVM_LINK_COMPONENTS)

set(KERNELS_OBJ "${CMAKE_CURRENT_BINARY_DIR}/UC12Kernels.o")
add_custom_command(OUTPUT ${KERNELS_OBJ}
    COMMAND ${CLANG_EXE} -x c++ -g -O0 ${CXX_FLAGS_LIST}
            -I${CMAKE_CURRENT_SOURCE_DIR}
            -I${CMAKE_CURRENT_SOURCE_DIR}/../../../runtime/ClangRuntimeSpecializer
            -I${LLVM_MAIN_INCLUDE_DIR}
            -I${LLVM_BINARY_DIR}/include
            -fpass-plugin=${PLUGIN_LIB}
            -fexceptions -frtti
            -c ${CMAKE_CURRENT_SOURCE_DIR}/UC12Kernels.cpp
            -o ${KERNELS_OBJ}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/UC12Kernels.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/UC12Kernels.h
            LLVMRuntimeSpecializationComptimePlugin clang
    COMMENT "Compiling UC12Kernels with plugin")

set(BENCH_OBJ "${CMAKE_CURRENT_BINARY_DIR}/UC12Benchmark.o")
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
            -c ${CMAKE_CURRENT_SOURCE_DIR}/UC12Benchmark.cpp
            -o ${BENCH_OBJ}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/UC12Benchmark.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/UC12Kernels.h
            LLVMRuntimeSpecializationComptimePlugin clang
    COMMENT "Compiling UC12Benchmark with plugin")

set(IDE_STUB "${CMAKE_CURRENT_BINARY_DIR}/UC12_ide_stub.cpp")
file(WRITE "${IDE_STUB}" "")

set_source_files_properties(UC12Kernels.cpp UC12Benchmark.cpp PROPERTIES HEADER_FILE_ONLY ON)

add_benchmark(UC12GroupBy PARTIAL_SOURCES_INTENDED
    ${KERNELS_OBJ} ${BENCH_OBJ} UC12Kernels.cpp UC12Benchmark.cpp ${IDE_STUB})

target_include_directories(UC12GroupBy PRIVATE
    ${LLVM_THIRD_PARTY_DIR}/benchmark/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../runtime/ClangRuntimeSpecializer
    ${LLVM_MAIN_INCLUDE_DIR}
    ${LLVM_BINARY_DIR}/include
)

target_link_libraries(UC12GroupBy PRIVATE ClangRuntimeSpecializer benchmark)
set_target_properties(UC12GroupBy PROPERTIES ENABLE_EXPORTS ON)
if(NOT APPLE AND NOT MSVC)
    target_link_options(UC12GroupBy PRIVATE "-Wl,--export-dynamic")
endif()

set(UC12_KERNELS_OBJ "${KERNELS_OBJ}"                               PARENT_SCOPE)
set(UC12_BENCH_OBJ   "${BENCH_OBJ}"                                 PARENT_SCOPE)
set(UC12_SRC_DIR     "${CMAKE_CURRENT_SOURCE_DIR}"                  PARENT_SCOPE)
set(UC12_KERNELS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/UC12Kernels.cpp"  PARENT_SCOPE)
set(UC12_BENCH_SRC   "${CMAKE_CURRENT_SOURCE_DIR}/UC12Benchmark.cpp" PARENT_SCOPE)
