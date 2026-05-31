#include <benchmark/benchmark.h>
#include "ClangRuntimeSpecializerBenchmark.h"
#include <string>
#include <string_view>
#include <vector>
#include <chrono>

// SyntheticKernels.o is linked into AllBenchmarks; mypow_bench is a trivial
// function whose IR blob is registered at startup — suitable for JIT warmup.
extern "C" int mypow_bench(int x);

// Populated in main() before benchmark::Initialize() by calling specializeOnly
// on mypow_bench. This forces LLVM's JIT infrastructure to initialize once,
// preventing any UC/PolyBench/TPC-H benchmark from paying the startup cost.
static double g_jit_init_warmup_ms = 0.0;

// Benchmark that reports the one-time JIT initialization cost measured in
// main(). Registered before benchmark::Initialize() so it sorts first.
// The startup cost includes: LLJIT creation, TargetMachine detection,
// DynamicLibrarySearchGenerator setup, first IR→machine-code compilation.
static void BM_jit_init_warmup(benchmark::State& state) {
    for (auto _ : state) {
        state.SetIterationTime(g_jit_init_warmup_ms / 1000.0);
    }
}
BENCHMARK(BM_jit_init_warmup)
    ->Name("BM_g:synthetic;n:jit_init;t:jit_overhead;")
    ->Iterations(1)
    ->UseManualTime();

// Implemented in sqlite3_tpch_bench.cpp (linked into AllBenchmarks).
void tpch_set_db_path(const char* path);
void sqlite_tpch_set_queries_dir(const char* path);
// Implemented in duckdb_tpch_bench.cpp (linked into AllBenchmarks).
void duckdb_tpch_set_db_path(const char* path);
void duckdb_tpch_set_queries_dir(const char* path);

// Default filter: fast benchmarks only.
// Pass --benchmark_filter=.* to also run polybench/TPCH.
static const std::string kDefaultFilter =
    "BM_.*(mypow|config_count|method_add|single_filter|chained_filter).*";

int main(int argc, char** argv) {
    bool has_filter = false;
    std::vector<char*> new_argv;
    new_argv.push_back(argv[0]);
    for (int i = 1; i < argc; ++i) {
        std::string_view a(argv[i]);
        if (a.rfind("--benchmark_filter", 0) == 0) {
            has_filter = true;
            new_argv.push_back(argv[i]);
        } else if (a.rfind("--db=", 0) == 0) {
            tpch_set_db_path(std::string(a.substr(5)).c_str());
        } else if (a.rfind("--duckdb-db=", 0) == 0) {
            duckdb_tpch_set_db_path(std::string(a.substr(12)).c_str());
        } else if (a.rfind("--queries-dir=", 0) == 0) {
            // Forward <path>/sqlite and <path>/duckdb to the respective modules.
            std::string base(a.substr(14));
            sqlite_tpch_set_queries_dir((base + "/sqlite").c_str());
            duckdb_tpch_set_queries_dir((base + "/duckdb").c_str());
        } else {
            new_argv.push_back(argv[i]);
        }
    }
    std::string filter_flag;
    if (!has_filter) {
        filter_flag = "--benchmark_filter=" + kDefaultFilter;
        new_argv.push_back(filter_flag.data());
    }
    int new_argc = static_cast<int>(new_argv.size());
    // Pre-warm the LLVM JIT infrastructure before any benchmark runs.
    // Without this, the first JIT benchmark in the binary (whichever kernel
    // happens to be registered first) pays ~2200ms of one-time startup cost
    // (LLJIT creation, TargetMachine init, DynamicLibrarySearchGenerator,
    // first IR parse + codegen). With this warmup, every benchmark sees an
    // already-initialized JIT and pays only the actual compilation cost.
    {
        namespace chr = std::chrono;
        auto t0 = chr::steady_clock::now();
        namespace CRS = clangRuntimeSpecializer;
        auto* RS = CRS::ClangRuntimeSpecializer::init();
        // mypow_bench is a trivial 1-argument function; specializeOnly on it
        // forces LLVM to create the JIT engine and compile a minimal module.
        RS->specializeOnly<int>(mypow_bench, 2);
        auto t1 = chr::steady_clock::now();
        g_jit_init_warmup_ms =
            chr::duration<double, std::milli>(t1 - t0).count();
    }

    static RSSMemoryManager g_rss_mgr;
    benchmark::RegisterMemoryManager(&g_rss_mgr);
    benchmark::Initialize(&new_argc, new_argv.data());
    if (benchmark::ReportUnrecognizedArguments(new_argc, new_argv.data())) return 1;
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
