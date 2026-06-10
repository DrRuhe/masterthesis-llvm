set(LLVM_LINK_COMPONENTS)

include(FetchContent)

# ── Fetch SQLite amalgamation ─────────────────────────────────────────────────
# SQLite 3.49.1 amalgamation — single C source + header.
FetchContent_Declare(sqlite3_src
    URL https://www.sqlite.org/2025/sqlite-amalgamation-3490100.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(sqlite3_src)

# ── Fetch + build DuckDB v1.5.2 from full source ─────────────────────────────
# Full source build bundles core_functions statically (needed for sum/count/avg
# in v1.1.0+). Defaults: DISABLE_BUILTIN_EXTENSIONS=OFF, EXTENSION_STATIC_BUILD=ON.
FetchContent_Declare(duckdb
    URL https://github.com/duckdb/duckdb/archive/refs/tags/v1.5.2.tar.gz
    URL_HASH SHA256=6d8612fc87115cf4d3512a934ada5d1669db29378b4cc8e226fdfa8f5c537385
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(BUILD_UNITTESTS  OFF CACHE BOOL "" FORCE)
set(BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(BUILD_SHELL      OFF CACHE BOOL "" FORCE)
set(ENABLE_SANITIZER OFF CACHE BOOL "" FORCE)
set(ENABLE_UBSAN     OFF CACHE BOOL "" FORCE)

# Strip libc++ flags so DuckDB uses libstdc++ via the --gcc-toolchain injected by
# the devshell. CMAKE_CXX_FLAGS is set before MakeAvailable so all DuckDB targets
# inherit the stripped flags; restored afterwards for our own targets.
set(_duckdb_cxx_flags "${CMAKE_CXX_FLAGS}")
string(REGEX REPLACE "-nostdinc\\+\\+" "" _duckdb_cxx_flags "${_duckdb_cxx_flags}")
string(REGEX REPLACE "-stdlib=libc\\+\\+" "" _duckdb_cxx_flags "${_duckdb_cxx_flags}")
string(REGEX REPLACE "-isystem [^ ]+/c\\+\\+/v1/?" "" _duckdb_cxx_flags "${_duckdb_cxx_flags}")

set(_saved_cxx_flags "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${_duckdb_cxx_flags}")
# DuckDB is compiled without the IR-dumping plugin. Injecting it requires the
# plugin to load into the host clang (CMAKE_CXX_COMPILER, which may be the nix
# release clang). The release libLLVM exports DisableABIBreakingChecks, not
# EnableABIBreakingChecks, so the plugin fails to load against a release host.
# DuckDB TPC-H specialization is impractical (module too large), so no blobs
# are needed from DuckDB object files.
add_compile_options(-fvisibility=default -w -Wno-error)
FetchContent_MakeAvailable(duckdb)
set(CMAKE_CXX_FLAGS "${_saved_cxx_flags}")

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
            -I${duckdb_SOURCE_DIR}/src/include
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
    ${DUCKDB_TPCH_OBJ} duckdb_tpch_bench.cpp ${DUCKDB_TPCH_IDE_STUB})

target_include_directories(DuckDBTpchBenchmark PRIVATE
    ${LLVM_THIRD_PARTY_DIR}/benchmark/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../runtime/ClangRuntimeSpecializer
    ${duckdb_SOURCE_DIR}/src/include
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
# Link duckdb_static with --whole-archive so blob-registering constructors survive
# dead-strip.  Extension implementations live in separate static archives
# (core_functions_extension, parquet_extension, jemalloc_extension) that are only
# linked into the shared duckdb lib by default — we must pull them in explicitly.
# duckdb_generated_extension_loader contains LoadAllExtensions().
if(NOT APPLE AND NOT MSVC)
    target_link_libraries(DuckDBTpchBenchmark PRIVATE
        "-Wl,--whole-archive"
        "$<TARGET_FILE:duckdb_static>"
        "$<TARGET_FILE:duckdb_generated_extension_loader>"
        "$<TARGET_FILE:core_functions_extension>"
        "$<TARGET_FILE:parquet_extension>"
        "$<TARGET_FILE:jemalloc_extension>"
        "-Wl,--no-whole-archive"
    )
    add_dependencies(DuckDBTpchBenchmark duckdb_static duckdb_generated_extension_loader
        core_functions_extension parquet_extension jemalloc_extension)
endif()
if(NOT APPLE AND NOT MSVC AND CRS_GCC_TOOLCHAIN_PATH)
    target_link_libraries(DuckDBTpchBenchmark PRIVATE
        "${CRS_GCC_TOOLCHAIN_PATH}/lib/libstdc++.a"
    )
endif()

# Export paths to parent scope so benchmarks/CMakeLists.txt can define the
# AllBenchmarks compilation commands. Custom command outputs must be defined
# in the same CMakeLists.txt directory as the consuming target.
set(TPCH_SQLITE3_IR_OBJ  "${SQLITE3_IR_OBJ}"                                    PARENT_SCOPE)
set(TPCH_SQLITE3_SRC_DIR "${sqlite3_src_SOURCE_DIR}"                             PARENT_SCOPE)
set(TPCH_SRC_DIR         "${CMAKE_CURRENT_SOURCE_DIR}"                           PARENT_SCOPE)
set(TPCH_SRC_FILE        "${CMAKE_CURRENT_SOURCE_DIR}/sqlite3_tpch_bench.cpp"    PARENT_SCOPE)
set(DUCKDB_SRC_DIR       "${duckdb_SOURCE_DIR}/src/include"                      PARENT_SCOPE)
set(DUCKDB_TPCH_OBJ      "${DUCKDB_TPCH_OBJ}"                                   PARENT_SCOPE)
set(DUCKDB_TPCH_SRC_FILE "${CMAKE_CURRENT_SOURCE_DIR}/duckdb_tpch_bench.cpp"    PARENT_SCOPE)
set(DUCKDB_TPCH_SRC_DIR  "${CMAKE_CURRENT_SOURCE_DIR}"                           PARENT_SCOPE)
