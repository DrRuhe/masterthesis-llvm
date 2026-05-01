set(LLVM_LINK_COMPONENTS)

include(FetchContent)

# ── Fetch SQLite amalgamation ─────────────────────────────────────────────────
# SQLite 3.49.1 amalgamation — single C source + header.
FetchContent_Declare(sqlite3_src
    URL https://www.sqlite.org/2025/sqlite-amalgamation-3490100.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(sqlite3_src)

# ── Fetch DuckDB amalgamation ─────────────────────────────────────────────────
# DuckDB v1.5.2 amalgamation — single C++ source + header.
# v1.1.0+ requires libstdc++ for core_functions; see DUCKDB_CXX_FLAGS_LIST below.
FetchContent_Declare(duckdb_src
    URL https://github.com/duckdb/duckdb/releases/download/v1.5.2/libduckdb-src.zip
    URL_HASH SHA256=36388f54d4e73c7148895f9b075c063189d47df8687db237f765f74a7ff5d8f6
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(duckdb_src)

# Build DuckDB-specific compile flags: strip libc++ flags so clang finds libstdc++
# via the --gcc-toolchain already present in CMAKE_CXX_FLAGS (set by the devshell).
set(_duckdb_cxx_flags "${CMAKE_CXX_FLAGS}")
string(REGEX REPLACE "-nostdinc\\+\\+" "" _duckdb_cxx_flags "${_duckdb_cxx_flags}")
string(REGEX REPLACE "-stdlib=libc\\+\\+" "" _duckdb_cxx_flags "${_duckdb_cxx_flags}")
string(REGEX REPLACE "-isystem [^ ]+/c\\+\\+/v1/?" "" _duckdb_cxx_flags "${_duckdb_cxx_flags}")
separate_arguments(DUCKDB_CXX_FLAGS_LIST NATIVE_COMMAND "${_duckdb_cxx_flags}")

# ── Compile sqlite3.c with the IR-dumping pass ────────────────────────────────
# The pass runs at OptimizerLastEP and embeds the entire SQLite module's IR
# (including sqlite3VdbeExec) as a bitcode blob registered at startup.
set(SQLITE3_IR_OBJ "${CMAKE_CURRENT_BINARY_DIR}/sqlite3_with_ir.o")

add_custom_command(
    OUTPUT ${SQLITE3_IR_OBJ}
    COMMAND ${CLANG_EXE}
            -x c -O2
            -DSQLITE_THREADSAFE=0
            -DSQLITE_OMIT_LOAD_EXTENSION=1
            "-DSQLITE_PRIVATE="
            -fvisibility=default
            -I${sqlite3_src_SOURCE_DIR}
            -fpass-plugin=${PLUGIN_LIB}
            -c ${sqlite3_src_SOURCE_DIR}/sqlite3.c
            -o ${SQLITE3_IR_OBJ}
    DEPENDS ${sqlite3_src_SOURCE_DIR}/sqlite3.c LLVMRuntimeSpecializationComptimePlugin clang
    COMMENT "Compiling sqlite3.c with IR-dumping plugin (embeds sqlite3VdbeExec IR)"
)

# ── Compile duckdb.cpp with the IR-dumping pass ───────────────────────────────
# Embeds the entire DuckDB module's IR (including duckdb_execute_prepared) as a
# bitcode blob registered at startup.
set(DUCKDB_IR_OBJ "${CMAKE_CURRENT_BINARY_DIR}/duckdb_with_ir.o")

add_custom_command(
    OUTPUT ${DUCKDB_IR_OBJ}
    COMMAND ${CLANG_EXE}
            -x c++ -std=c++17 -O2
            ${DUCKDB_CXX_FLAGS_LIST}
            -w
            -Wno-error
            -Wno-missing-template-arg-list-after-template-kw
            -fvisibility=default
            -fexceptions -frtti
            -I${duckdb_src_SOURCE_DIR}
            -fpass-plugin=${PLUGIN_LIB}
            -c ${duckdb_src_SOURCE_DIR}/duckdb.cpp
            -o ${DUCKDB_IR_OBJ}
    DEPENDS ${duckdb_src_SOURCE_DIR}/duckdb.cpp LLVMRuntimeSpecializationComptimePlugin clang
    COMMENT "Compiling duckdb.cpp with IR-dumping plugin (embeds duckdb_execute_prepared IR)"
)

# ── Compile sqlite3_tpch_bench.cpp with the IR-dumping pass ──────────────────
set(TPCH_OBJ "${CMAKE_CURRENT_BINARY_DIR}/sqlite3_tpch_bench.o")

add_custom_command(
    OUTPUT ${TPCH_OBJ}
    COMMAND ${CLANG_EXE} -x c++ -g -O3 ${CXX_FLAGS_LIST}
            -I${CMAKE_CURRENT_SOURCE_DIR}
            -I${CMAKE_CURRENT_SOURCE_DIR}/..
            -I${CMAKE_CURRENT_SOURCE_DIR}/../../runtime/ClangRuntimeSpecializer
            -I${sqlite3_src_SOURCE_DIR}
            -I${LLVM_MAIN_INCLUDE_DIR}
            -I${LLVM_BINARY_DIR}/include
            -I${LLVM_THIRD_PARTY_DIR}/benchmark/include
            -fexceptions -frtti
            -c ${CMAKE_CURRENT_SOURCE_DIR}/sqlite3_tpch_bench.cpp
            -o ${TPCH_OBJ}
    DEPENDS sqlite3_tpch_bench.cpp clang
    COMMENT "Compiling sqlite3_tpch_bench.cpp"
)

# IDE stub: empty .cpp so CMake has a compile group and CLion resolves includes.
set(TPCH_IDE_STUB "${CMAKE_CURRENT_BINARY_DIR}/tpch_ide_stub.cpp")
file(WRITE "${TPCH_IDE_STUB}" "")

set_source_files_properties(sqlite3_tpch_bench.cpp PROPERTIES HEADER_FILE_ONLY ON)

add_benchmark(TPCHBenchmark PARTIAL_SOURCES_INTENDED
    ${TPCH_OBJ} ${SQLITE3_IR_OBJ} sqlite3_tpch_bench.cpp ${TPCH_IDE_STUB})

target_include_directories(TPCHBenchmark PRIVATE
    ${LLVM_THIRD_PARTY_DIR}/benchmark/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../runtime/ClangRuntimeSpecializer
    ${sqlite3_src_SOURCE_DIR}
    ${LLVM_MAIN_INCLUDE_DIR}
    ${LLVM_BINARY_DIR}/include
)

target_link_libraries(TPCHBenchmark PRIVATE
    ClangRuntimeSpecializer
    benchmark
)

set_target_properties(TPCHBenchmark PROPERTIES ENABLE_EXPORTS ON)
if(NOT APPLE AND NOT MSVC)
    target_link_options(TPCHBenchmark PRIVATE "-Wl,--export-dynamic")
endif()

# ── Compile duckdb_tpch_bench.cpp with the IR-dumping pass ───────────────────
set(DUCKDB_TPCH_OBJ "${CMAKE_CURRENT_BINARY_DIR}/duckdb_tpch_bench.o")

add_custom_command(
    OUTPUT ${DUCKDB_TPCH_OBJ}
    COMMAND ${CLANG_EXE} -x c++ -g -O3 ${CXX_FLAGS_LIST}
            "-DDUCKDB_TPCH_DEFAULT_DB_PATH=\\\"${CMAKE_CURRENT_SOURCE_DIR}/data/tpch.duckdb\\\""
            "-DDUCKDB_TPCH_DEFAULT_QUERIES_DIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}/queries/duckdb\\\""
            -I${CMAKE_CURRENT_SOURCE_DIR}
            -I${CMAKE_CURRENT_SOURCE_DIR}/..
            -I${CMAKE_CURRENT_SOURCE_DIR}/../../runtime/ClangRuntimeSpecializer
            -I${duckdb_src_SOURCE_DIR}
            -I${LLVM_MAIN_INCLUDE_DIR}
            -I${LLVM_BINARY_DIR}/include
            -I${LLVM_THIRD_PARTY_DIR}/benchmark/include
            -fexceptions -frtti
            -c ${CMAKE_CURRENT_SOURCE_DIR}/duckdb_tpch_bench.cpp
            -o ${DUCKDB_TPCH_OBJ}
    DEPENDS duckdb_tpch_bench.cpp clang
    COMMENT "Compiling duckdb_tpch_bench.cpp"
)

set(DUCKDB_TPCH_IDE_STUB "${CMAKE_CURRENT_BINARY_DIR}/duckdb_tpch_ide_stub.cpp")
file(WRITE "${DUCKDB_TPCH_IDE_STUB}" "")

set_source_files_properties(duckdb_tpch_bench.cpp PROPERTIES HEADER_FILE_ONLY ON)

add_benchmark(DuckDBTpchBenchmark PARTIAL_SOURCES_INTENDED
    ${DUCKDB_TPCH_OBJ} ${DUCKDB_IR_OBJ} duckdb_tpch_bench.cpp ${DUCKDB_TPCH_IDE_STUB})

target_include_directories(DuckDBTpchBenchmark PRIVATE
    ${LLVM_THIRD_PARTY_DIR}/benchmark/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../runtime/ClangRuntimeSpecializer
    ${duckdb_src_SOURCE_DIR}
    ${LLVM_MAIN_INCLUDE_DIR}
    ${LLVM_BINARY_DIR}/include
)

target_link_libraries(DuckDBTpchBenchmark PRIVATE
    ClangRuntimeSpecializer
    benchmark
)

set_target_properties(DuckDBTpchBenchmark PROPERTIES ENABLE_EXPORTS ON)
if(NOT APPLE AND NOT MSVC)
    target_link_options(DuckDBTpchBenchmark PRIVATE "-Wl,--export-dynamic")
endif()
# Link libstdc++ for DuckDB's core_functions (v1.1.0+) and embed rpath so the
# binary finds libstdc++.so without LD_LIBRARY_PATH.
if(NOT APPLE AND NOT MSVC AND CRS_GCC_TOOLCHAIN_PATH)
    target_link_options(DuckDBTpchBenchmark PRIVATE
        "-L${CRS_GCC_TOOLCHAIN_PATH}/lib"
        "-lstdc++"
        "-Wl,-rpath,${CRS_GCC_TOOLCHAIN_PATH}/lib"
    )
endif()

# Export paths to parent scope so benchmarks/CMakeLists.txt can define the
# AllBenchmarks compilation commands. Custom command outputs must be defined
# in the same CMakeLists.txt directory as the consuming target.
set(TPCH_SQLITE3_IR_OBJ  "${SQLITE3_IR_OBJ}"                                    PARENT_SCOPE)
set(TPCH_SQLITE3_SRC_DIR "${sqlite3_src_SOURCE_DIR}"                             PARENT_SCOPE)
set(TPCH_SRC_DIR         "${CMAKE_CURRENT_SOURCE_DIR}"                           PARENT_SCOPE)
set(TPCH_SRC_FILE        "${CMAKE_CURRENT_SOURCE_DIR}/sqlite3_tpch_bench.cpp"    PARENT_SCOPE)
set(DUCKDB_IR_OBJ        "${DUCKDB_IR_OBJ}"                                      PARENT_SCOPE)
set(DUCKDB_SRC_DIR       "${duckdb_src_SOURCE_DIR}"                              PARENT_SCOPE)
set(DUCKDB_TPCH_OBJ      "${DUCKDB_TPCH_OBJ}"                                   PARENT_SCOPE)
set(DUCKDB_TPCH_SRC_FILE "${CMAKE_CURRENT_SOURCE_DIR}/duckdb_tpch_bench.cpp"    PARENT_SCOPE)
set(DUCKDB_TPCH_SRC_DIR  "${CMAKE_CURRENT_SOURCE_DIR}"                           PARENT_SCOPE)
