# ── Use-Case Benchmark Suite ──────────────────────────────────────────────────
#
# Each subdirectory defines its own benchmark target and exports PARENT_SCOPE
# variables so benchmarks/CMakeLists.txt can include the objects in AllBenchmarks.
#
# Variables exported to parent (benchmarks/) scope per use case:
#   UC<N>_KERNELS_OBJS — list of precompiled kernel .o files (with plugin, for AllBenchmarks re-link)
#   UC<N>_KERNELS_SRCS — list of absolute paths to all UC<N>*Kernels.cpp sources
#   UC<N>_BENCH_OBJ    — precompiled benchmark .o (with plugin)
#   UC<N>_SRC_DIR      — directory containing UC<N>*.h (for target_include_directories)
#   UC<N>_BENCH_SRC    — absolute path to UC<N>Benchmark.cpp

# Shared CMake variables inherited from benchmarks/CMakeLists.txt:
#   CLANG_EXE, PLUGIN_LIB, CXX_FLAGS_LIST
# (No need to redefine — CMake child scopes see parent scope variables.)

add_subdirectory(UC1SqlPredicate)
add_subdirectory(UC2Convolution)
add_subdirectory(UC7DfaRegex)
add_subdirectory(UC8IVM)
add_subdirectory(UC12GroupBy)
add_subdirectory(UC14Sort)

# Re-export per-use-case variables from each subdir's PARENT_SCOPE into
# benchmarks/CMakeLists.txt scope so AllBenchmarks targets can consume them.
foreach(UC IN ITEMS UC1 UC2 UC7 UC8 UC12 UC14)
    set(${UC}_KERNELS_OBJS "${${UC}_KERNELS_OBJS}" PARENT_SCOPE)
    set(${UC}_KERNELS_SRCS "${${UC}_KERNELS_SRCS}" PARENT_SCOPE)
    set(${UC}_BENCH_OBJ    "${${UC}_BENCH_OBJ}"    PARENT_SCOPE)
    set(${UC}_SRC_DIR      "${${UC}_SRC_DIR}"      PARENT_SCOPE)
    set(${UC}_BENCH_SRC    "${${UC}_BENCH_SRC}"    PARENT_SCOPE)
endforeach()
