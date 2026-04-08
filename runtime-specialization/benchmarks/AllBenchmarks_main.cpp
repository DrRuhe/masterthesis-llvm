#include <benchmark/benchmark.h>
#include <string>
#include <vector>

// Runs only the fast benchmarks by default.
// Pass --benchmark_filter=.* to include polybench kernels.
static const std::string kDefaultFilter =
    "BM_.*_(mypow|config_count|method_add|single_filter|chained_filter)";

int main(int argc, char** argv) {
    bool has_filter = false;
    for (int i = 1; i < argc; ++i)
        if (std::string_view(argv[i]).rfind("--benchmark_filter", 0) == 0)
            has_filter = true;

    std::vector<char*> new_argv(argv, argv + argc);
    std::string filter_flag;
    if (!has_filter) {
        filter_flag = "--benchmark_filter=" + kDefaultFilter;
        new_argv.push_back(filter_flag.data());
    }
    int new_argc = static_cast<int>(new_argv.size());
    benchmark::Initialize(&new_argc, new_argv.data());
    if (benchmark::ReportUnrecognizedArguments(new_argc, new_argv.data())) return 1;
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
