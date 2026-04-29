set(LLVM_LINK_COMPONENTS)

set(CLANG_EXE "$<TARGET_FILE:clang>")
set(PLUGIN_LIB "$<TARGET_FILE:LLVMRuntimeSpecializationComptimePlugin>")

separate_arguments(CXX_FLAGS_LIST NATIVE_COMMAND "${CMAKE_CXX_FLAGS}")

# ── SyntheticKernels ──────────────────────────────────────────────────────────
# Compiled separately from SpecializerBenchmark.cpp so the JIT blob for each
# kernel contains only the kernel code, not the benchmark-library infrastructure.

set(KERNELS_OBJ_FILE "${CMAKE_CURRENT_BINARY_DIR}/SyntheticKernels.o")

add_custom_command(
    OUTPUT ${KERNELS_OBJ_FILE}
    COMMAND ${CLANG_EXE} -x c++ -g -O3 ${CXX_FLAGS_LIST}
            -I${CMAKE_CURRENT_SOURCE_DIR}
            -fpass-plugin=${PLUGIN_LIB}
            -fexceptions -frtti
            -c ${CMAKE_CURRENT_SOURCE_DIR}/SyntheticKernels.cpp
            -o ${KERNELS_OBJ_FILE}
    DEPENDS SyntheticKernels.cpp SyntheticKernels.h LLVMRuntimeSpecializationComptimePlugin clang
    COMMENT "Compiling SyntheticKernels with project clang and plugin"
)

set_source_files_properties(SyntheticKernels.cpp PROPERTIES HEADER_FILE_ONLY ON)

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
add_benchmark(SpecializerBenchmark PARTIAL_SOURCES_INTENDED
    ${KERNELS_OBJ_FILE} ${OBJ_FILE} SpecializerBenchmark.cpp SyntheticKernels.cpp ${IDE_STUB})

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
add_subdirectory(tpch)

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

# ── AllBenchmarks TPCH (SQLite) object ───────────────────────────────────────
# Custom commands must be defined in the same CMakeLists.txt as the target that
# uses their output (CMake limitation). We define it here in the parent using
# paths exported from tpch/CMakeLists.txt via PARENT_SCOPE.
set(ALL_OBJ_TPCH "${CMAKE_CURRENT_BINARY_DIR}/AllBenchmarks_sqlite3_tpch_bench.o")
add_custom_command(
    OUTPUT ${ALL_OBJ_TPCH}
    COMMAND ${CLANG_EXE} -x c++ -g -O3 ${CXX_FLAGS_LIST}
            -DALL_BENCHMARKS_BUILD=1
            -I${TPCH_SRC_DIR}
            -I${CMAKE_CURRENT_SOURCE_DIR}
            -I${CMAKE_CURRENT_SOURCE_DIR}/../runtime/ClangRuntimeSpecializer
            -I${TPCH_SQLITE3_SRC_DIR}
            -I${LLVM_MAIN_INCLUDE_DIR}
            -I${LLVM_BINARY_DIR}/include
            -I${LLVM_THIRD_PARTY_DIR}/benchmark/include
            -fpass-plugin=${PLUGIN_LIB}
            -fexceptions -frtti
            -c ${TPCH_SRC_FILE}
            -o ${ALL_OBJ_TPCH}
    DEPENDS ${TPCH_SRC_FILE} LLVMRuntimeSpecializationComptimePlugin clang
            ${TPCH_SQLITE3_IR_OBJ}
    COMMENT "Compiling sqlite3_tpch_bench.cpp for AllBenchmarks"
)

# ── AllBenchmarks DuckDB TPCH object ─────────────────────────────────────────
set(ALL_OBJ_DUCKDB_TPCH "${CMAKE_CURRENT_BINARY_DIR}/AllBenchmarks_duckdb_tpch_bench.o")
add_custom_command(
    OUTPUT ${ALL_OBJ_DUCKDB_TPCH}
    COMMAND ${CLANG_EXE} -x c++ -g -O3 ${CXX_FLAGS_LIST}
            -DALL_BENCHMARKS_BUILD=1
            -I${DUCKDB_TPCH_SRC_DIR}
            -I${CMAKE_CURRENT_SOURCE_DIR}
            -I${CMAKE_CURRENT_SOURCE_DIR}/../runtime/ClangRuntimeSpecializer
            -I${DUCKDB_SRC_DIR}
            -I${LLVM_MAIN_INCLUDE_DIR}
            -I${LLVM_BINARY_DIR}/include
            -I${LLVM_THIRD_PARTY_DIR}/benchmark/include
            -fpass-plugin=${PLUGIN_LIB}
            -fexceptions -frtti
            -c ${DUCKDB_TPCH_SRC_FILE}
            -o ${ALL_OBJ_DUCKDB_TPCH}
    DEPENDS ${DUCKDB_TPCH_SRC_FILE} LLVMRuntimeSpecializationComptimePlugin clang
            ${DUCKDB_IR_OBJ}
    COMMENT "Compiling duckdb_tpch_bench.cpp for AllBenchmarks"
)

add_benchmark(AllBenchmarks PARTIAL_SOURCES_INTENDED
    ${KERNELS_OBJ_FILE} ${ALL_OBJ_SPEC} ${ALL_OBJ_DB} ${ALL_OBJ_POLY}
    ${ALL_OBJ_TPCH} ${TPCH_SQLITE3_IR_OBJ}
    ${ALL_OBJ_DUCKDB_TPCH} ${DUCKDB_IR_OBJ}
    AllBenchmarks_main.cpp)

target_include_directories(AllBenchmarks PRIVATE
    ${LLVM_THIRD_PARTY_DIR}/benchmark/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../runtime/ClangRuntimeSpecializer
    ${CMAKE_CURRENT_SOURCE_DIR}/polybench
    ${CMAKE_CURRENT_SOURCE_DIR}/polybench/PolyBenchC-4.2.1-master/utilities
    ${CMAKE_CURRENT_SOURCE_DIR}/tpch
    ${TPCH_SQLITE3_SRC_DIR}
    ${DUCKDB_SRC_DIR}
    ${LLVM_MAIN_INCLUDE_DIR}
    ${LLVM_BINARY_DIR}/include
)
target_link_libraries(AllBenchmarks PRIVATE ClangRuntimeSpecializer benchmark)
# AllBenchmarks_main.cpp includes ClangRuntimeSpecializerBenchmark.h which
# transitively pulls in LLVM headers; those require exceptions to be enabled.
target_compile_options(AllBenchmarks PRIVATE -fexceptions -frtti)
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
