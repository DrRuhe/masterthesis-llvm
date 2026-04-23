set(LLVM_LINK_COMPONENTS)

include(FetchContent)

# ── Fetch SQLite amalgamation ─────────────────────────────────────────────────
# SQLite 3.49.1 amalgamation — single C source + header.
FetchContent_Declare(sqlite3_src
    URL https://www.sqlite.org/2025/sqlite-amalgamation-3490100.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(sqlite3_src)

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

# ── Compile tpch_bench.cpp with the IR-dumping pass ──────────────────────────
set(TPCH_OBJ "${CMAKE_CURRENT_BINARY_DIR}/tpch_bench.o")

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
            -fpass-plugin=${PLUGIN_LIB}
            -fexceptions -frtti
            -c ${CMAKE_CURRENT_SOURCE_DIR}/tpch_bench.cpp
            -o ${TPCH_OBJ}
    DEPENDS tpch_bench.cpp LLVMRuntimeSpecializationComptimePlugin clang ${SQLITE3_IR_OBJ}
    COMMENT "Compiling tpch_bench.cpp with IR-dumping plugin"
)

# IDE stub: empty .cpp so CMake has a compile group and CLion resolves includes.
set(TPCH_IDE_STUB "${CMAKE_CURRENT_BINARY_DIR}/tpch_ide_stub.cpp")
file(WRITE "${TPCH_IDE_STUB}" "")

set_source_files_properties(tpch_bench.cpp PROPERTIES HEADER_FILE_ONLY ON)

add_benchmark(TPCHBenchmark PARTIAL_SOURCES_INTENDED
    ${TPCH_OBJ} ${SQLITE3_IR_OBJ} tpch_bench.cpp ${TPCH_IDE_STUB})

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

# Export paths to parent scope so benchmarks/CMakeLists.txt can define the
# AllBenchmarks compilation command for tpch_bench.cpp. Custom command outputs
# must be defined in the same CMakeLists.txt directory as the consuming target,
# so the actual add_custom_command lives in the parent.
set(TPCH_SQLITE3_IR_OBJ  "${SQLITE3_IR_OBJ}"                          PARENT_SCOPE)
set(TPCH_SQLITE3_SRC_DIR "${sqlite3_src_SOURCE_DIR}"                   PARENT_SCOPE)
set(TPCH_SRC_DIR         "${CMAKE_CURRENT_SOURCE_DIR}"                 PARENT_SCOPE)
set(TPCH_SRC_FILE        "${CMAKE_CURRENT_SOURCE_DIR}/tpch_bench.cpp"  PARENT_SCOPE)
