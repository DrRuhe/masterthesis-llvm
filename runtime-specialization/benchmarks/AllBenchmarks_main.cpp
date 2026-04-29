#include <benchmark/benchmark.h>
#include "ClangRuntimeSpecializerBenchmark.h"
#include <string>
#include <string_view>
#include <vector>

// Implemented in sqlite3_tpch_bench.cpp (linked into AllBenchmarks).
void tpch_set_db_path(const char* path);
// Implemented in duckdb_tpch_bench.cpp (linked into AllBenchmarks).
void duckdb_tpch_set_db_path(const char* path);

// Default filter: fast benchmarks only.
// Pass --benchmark_filter=.* to also run polybench/TPCH.
static const std::string kDefaultFilter =
    "BM_.*_(mypow|config_count|method_add|single_filter|chained_filter)";

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
            // Strip --db=<path> and forward it to the SQLite TPCH benchmark module.
            tpch_set_db_path(std::string(a.substr(5)).c_str());
        } else if (a.rfind("--duckdb-db=", 0) == 0) {
            // Strip --duckdb-db=<path> and forward it to the DuckDB TPCH benchmark module.
            duckdb_tpch_set_db_path(std::string(a.substr(12)).c_str());
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
