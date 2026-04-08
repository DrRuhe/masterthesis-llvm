set(LLVM_LINK_COMPONENTS)

set(CLANG_EXE "$<TARGET_FILE:clang>")
set(PLUGIN_LIB "$<TARGET_FILE:LLVMRuntimeSpecializationComptimePlugin>")

separate_arguments(CXX_FLAGS_LIST NATIVE_COMMAND "${CMAKE_CXX_FLAGS}")

# ── SpecializerBenchmark ──────────────────────────────────────────────────────

set(OBJ_FILE "${CMAKE_CURRENT_BINARY_DIR}/SpecializerBenchmark.o")

add_custom_command(
    OUTPUT ${OBJ_FILE}
    COMMAND ${CLANG_EXE} -x c++ -g -O3 ${CXX_FLAGS_LIST}
            -I${CMAKE_CURRENT_SOURCE_DIR}
            -I${CMAKE_CURRENT_SOURCE_DIR}/../runtime/ClangRuntimeSpecializer
            -I${LLVM_MAIN_INCLUDE_DIR}
            -I${LLVM_BINARY_DIR}/include
            -I${LLVM_THIRD_PARTY_DIR}/benchmark/include
            -fpass-plugin=${PLUGIN_LIB}
            -fexceptions -frtti
            -c ${CMAKE_CURRENT_SOURCE_DIR}/SpecializerBenchmark.cpp
            -o ${OBJ_FILE}
    DEPENDS SpecializerBenchmark.cpp LLVMRuntimeSpecializationComptimePlugin clang
    COMMENT "Compiling SpecializerBenchmark with project clang and plugin"
)

# Generate a minimal stub source so CMake produces a compile group for this target.
# Without it the target has no compile groups (only a pre-compiled .o and a
# HEADER_FILE_ONLY .cpp), so target_include_directories has nothing to attach to
# and CLion cannot resolve includes for SpecializerBenchmark.cpp.
set(IDE_STUB "${CMAKE_CURRENT_BINARY_DIR}/benchmark_ide_stub.cpp")
file(WRITE "${IDE_STUB}" "")

set_source_files_properties(SpecializerBenchmark.cpp PROPERTIES HEADER_FILE_ONLY ON)
add_benchmark(SpecializerBenchmark PARTIAL_SOURCES_INTENDED ${OBJ_FILE} SpecializerBenchmark.cpp ${IDE_STUB})

target_include_directories(SpecializerBenchmark PRIVATE
    ${LLVM_THIRD_PARTY_DIR}/benchmark/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../runtime/ClangRuntimeSpecializer
    ${LLVM_MAIN_INCLUDE_DIR}
    ${LLVM_BINARY_DIR}/include
)

target_link_libraries(SpecializerBenchmark PRIVATE
    ClangRuntimeSpecializer
    benchmark
)

# Ensure symbols are exported for the JIT
set_target_properties(SpecializerBenchmark PROPERTIES ENABLE_EXPORTS ON)
if(NOT APPLE AND NOT MSVC)
  target_link_options(SpecializerBenchmark PRIVATE "-Wl,--export-dynamic")
endif()

# ── DBOperatorsBenchmark ──────────────────────────────────────────────────────

set(DB_OBJ_FILE "${CMAKE_CURRENT_BINARY_DIR}/DBOperatorsBenchmark.o")

add_custom_command(
    OUTPUT ${DB_OBJ_FILE}
    COMMAND ${CLANG_EXE} -x c++ -g -O3 ${CXX_FLAGS_LIST}
            -I${CMAKE_CURRENT_SOURCE_DIR}
            -I${CMAKE_CURRENT_SOURCE_DIR}/../runtime/ClangRuntimeSpecializer
            -I${LLVM_MAIN_INCLUDE_DIR}
            -I${LLVM_BINARY_DIR}/include
            -I${LLVM_THIRD_PARTY_DIR}/benchmark/include
            -fpass-plugin=${PLUGIN_LIB}
            -fexceptions -frtti
            -c ${CMAKE_CURRENT_SOURCE_DIR}/DBOperatorsBenchmark.cpp
            -o ${DB_OBJ_FILE}
    DEPENDS DBOperatorsBenchmark.cpp LLVMRuntimeSpecializationComptimePlugin clang
    COMMENT "Compiling DBOperatorsBenchmark with project clang and plugin"
)

set(DB_IDE_STUB "${CMAKE_CURRENT_BINARY_DIR}/db_benchmark_ide_stub.cpp")
file(WRITE "${DB_IDE_STUB}" "")

set_source_files_properties(DBOperatorsBenchmark.cpp PROPERTIES HEADER_FILE_ONLY ON)
add_benchmark(DBOperatorsBenchmark PARTIAL_SOURCES_INTENDED ${DB_OBJ_FILE} DBOperatorsBenchmark.cpp ${DB_IDE_STUB})

target_include_directories(DBOperatorsBenchmark PRIVATE
    ${LLVM_THIRD_PARTY_DIR}/benchmark/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../runtime/ClangRuntimeSpecializer
    ${LLVM_MAIN_INCLUDE_DIR}
    ${LLVM_BINARY_DIR}/include
)

target_link_libraries(DBOperatorsBenchmark PRIVATE
    ClangRuntimeSpecializer
    benchmark
)

# Ensure symbols are exported for the JIT
set_target_properties(DBOperatorsBenchmark PROPERTIES ENABLE_EXPORTS ON)
if(NOT APPLE AND NOT MSVC)
  target_link_options(DBOperatorsBenchmark PRIVATE "-Wl,--export-dynamic")
endif()

add_subdirectory(polybench)

# ── AllBenchmarks ─────────────────────────────────────────────────────────────

set(ALL_OBJ_SPEC "${CMAKE_CURRENT_BINARY_DIR}/AllBenchmarks_SpecializerBenchmark.o")
add_custom_command(
    OUTPUT ${ALL_OBJ_SPEC}
    COMMAND ${CLANG_EXE} -x c++ -g -O3 ${CXX_FLAGS_LIST}
            -DALL_BENCHMARKS_BUILD=1
            -I${CMAKE_CURRENT_SOURCE_DIR}
            -I${CMAKE_CURRENT_SOURCE_DIR}/../runtime/ClangRuntimeSpecializer
            -I${LLVM_MAIN_INCLUDE_DIR}
            -I${LLVM_BINARY_DIR}/include
            -I${LLVM_THIRD_PARTY_DIR}/benchmark/include
            -fpass-plugin=${PLUGIN_LIB}
            -fexceptions -frtti
            -c ${CMAKE_CURRENT_SOURCE_DIR}/SpecializerBenchmark.cpp
            -o ${ALL_OBJ_SPEC}
    DEPENDS SpecializerBenchmark.cpp LLVMRuntimeSpecializationComptimePlugin clang
    COMMENT "Compiling SpecializerBenchmark for AllBenchmarks"
)

set(ALL_OBJ_DB "${CMAKE_CURRENT_BINARY_DIR}/AllBenchmarks_DBOperatorsBenchmark.o")
add_custom_command(
    OUTPUT ${ALL_OBJ_DB}
    COMMAND ${CLANG_EXE} -x c++ -g -O3 ${CXX_FLAGS_LIST}
            -DALL_BENCHMARKS_BUILD=1
            -I${CMAKE_CURRENT_SOURCE_DIR}
            -I${CMAKE_CURRENT_SOURCE_DIR}/../runtime/ClangRuntimeSpecializer
            -I${LLVM_MAIN_INCLUDE_DIR}
            -I${LLVM_BINARY_DIR}/include
            -I${LLVM_THIRD_PARTY_DIR}/benchmark/include
            -fpass-plugin=${PLUGIN_LIB}
            -fexceptions -frtti
            -c ${CMAKE_CURRENT_SOURCE_DIR}/DBOperatorsBenchmark.cpp
            -o ${ALL_OBJ_DB}
    DEPENDS DBOperatorsBenchmark.cpp LLVMRuntimeSpecializationComptimePlugin clang
    COMMENT "Compiling DBOperatorsBenchmark for AllBenchmarks"
)

set(ALL_OBJ_POLY "${CMAKE_CURRENT_BINARY_DIR}/AllBenchmarks_PolyBench.o")
add_custom_command(
    OUTPUT ${ALL_OBJ_POLY}
    COMMAND ${CLANG_EXE} -x c++ -g -O3 ${CXX_FLAGS_LIST}
            -DALL_BENCHMARKS_BUILD=1
            -I${CMAKE_CURRENT_SOURCE_DIR}/polybench
            -I${CMAKE_CURRENT_SOURCE_DIR}
            -I${CMAKE_CURRENT_SOURCE_DIR}/../runtime/ClangRuntimeSpecializer
            -I${CMAKE_CURRENT_SOURCE_DIR}/polybench/PolyBenchC-4.2.1-master/utilities
            -I${LLVM_MAIN_INCLUDE_DIR}
            -I${LLVM_BINARY_DIR}/include
            -I${LLVM_THIRD_PARTY_DIR}/benchmark/include
            -fpass-plugin=${PLUGIN_LIB}
            -fexceptions -frtti
            -c ${CMAKE_CURRENT_SOURCE_DIR}/polybench/polybench_bench.cpp
            -o ${ALL_OBJ_POLY}
    DEPENDS polybench/polybench_bench.cpp LLVMRuntimeSpecializationComptimePlugin clang
    COMMENT "Compiling polybench_bench for AllBenchmarks"
)

add_benchmark(AllBenchmarks PARTIAL_SOURCES_INTENDED
    ${ALL_OBJ_SPEC} ${ALL_OBJ_DB} ${ALL_OBJ_POLY}
    AllBenchmarks_main.cpp)

target_include_directories(AllBenchmarks PRIVATE
    ${LLVM_THIRD_PARTY_DIR}/benchmark/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../runtime/ClangRuntimeSpecializer
    ${CMAKE_CURRENT_SOURCE_DIR}/polybench
    ${CMAKE_CURRENT_SOURCE_DIR}/polybench/PolyBenchC-4.2.1-master/utilities
    ${LLVM_MAIN_INCLUDE_DIR}
    ${LLVM_BINARY_DIR}/include
)
target_link_libraries(AllBenchmarks PRIVATE ClangRuntimeSpecializer benchmark)
set_target_properties(AllBenchmarks PROPERTIES ENABLE_EXPORTS ON)
if(NOT APPLE AND NOT MSVC)
    target_link_options(AllBenchmarks PRIVATE "-Wl,--export-dynamic")
endif()

# ── benchplot target ──────────────────────────────────────────────────────────

find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_FOUND)
    add_custom_target(benchplot
        COMMAND ${CMAKE_COMMAND} -E env
            "BENCHPLOT_DB_PATH=${CMAKE_CURRENT_BINARY_DIR}/benchmarks.duckdb"
            ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/benchplot.py
            --binary $<TARGET_FILE:AllBenchmarks>
            --output ${CMAKE_CURRENT_BINARY_DIR}/benchmarks.pdf
        DEPENDS AllBenchmarks
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/benchmarks.pdf
        COMMENT "Running AllBenchmarks and generating benchmarks.pdf"
        USES_TERMINAL
    )
endif()
