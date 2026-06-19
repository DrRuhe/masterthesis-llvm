#include <benchmark/benchmark.h>
#include "ClangRuntimeSpecializerBenchmark.h"
#include <string>
#include <string_view>
#include <vector>
#include <chrono>

// SyntheticKernels.o is linked into AllBenchmarks; mypow_bench is a trivial
// function whose IR blob is registered at startup — suitable for JIT warmup.
extern "C" int mypow_bench(int x);

// Benchmark that measures the one-time JIT initialization cost: LLJIT creation,
// TargetMachine detection, DynamicLibrarySearchGenerator setup, and first IR →
// machine-code compilation. This is the startup overhead that the first JIT call
// in any process run pays; subsequent calls only pay the per-kernel compilation cost
// (~40–60ms for small UC kernels).
//
// IMPORTANT: Run this benchmark BEFORE any other JIT benchmark to see the true
// startup cost. Filter example:
//   --benchmark_filter="BM_g:synthetic;n:jit_init.*|BM_g:uc1.*"
static void BM_jit_init_warmup(benchmark::State& state) {
    namespace CRS = clangRuntimeSpecializer;
    for (auto _ : state) {
        auto t0 = std::chrono::steady_clock::now();
        auto* RS = CRS::ClangRuntimeSpecializer::init();
        // specializeOnly on a trivial 1-argument function forces LLVM to create
        // the JIT engine and compile a minimal module — paying all startup costs.
        auto warmup = RS->specializeOnly<int>(mypow_bench, 2);
        auto t1 = std::chrono::steady_clock::now();
        state.SetIterationTime(
            std::chrono::duration<double>(t1 - t0).count());
        benchmark::DoNotOptimize(warmup);
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
    "BM_.*(jit_init|mypow|config_count|method_add|single_filter|chained_filter).*";

int main(int argc, char** argv) {
    bool has_filter = false;
    std::vector<char*> new_argv;
    new_argv.push_back(argv[0]);
    for (int i = 1; i < argc; ++i) {
        std::string_view a(argv[i]);
        if (a.rfind("--benchmark_filter", 0) == 0) {
            has_filter = true;
            new_argv.push_back(argv[i]);
        } else if (a.rfind("--uc-pipeline-config=", 0) == 0) {
            std::string path(a.substr(21));
            if (setenv("CRS_UC_PIPELINE_CONFIG_JSON", path.c_str(), 1) != 0) {
                std::perror("setenv(CRS_UC_PIPELINE_CONFIG_JSON)");
                return 1;
            }
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
    static RSSMemoryManager g_rss_mgr;
    benchmark::RegisterMemoryManager(&g_rss_mgr);
    benchmark::Initialize(&new_argc, new_argv.data());
    if (benchmark::ReportUnrecognizedArguments(new_argc, new_argv.data())) return 1;
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
