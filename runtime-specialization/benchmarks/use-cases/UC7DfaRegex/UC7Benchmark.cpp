#include "UC7Kernels.h"
#include "ClangRuntimeSpecializerBenchmark.h"
#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <string>
#include <cstring>

// ---------------------------------------------------------------------------
// Synthetic corpus: mix of random ASCII words and email-like strings.
// Allocated to EXTRALARGE size; smaller benchmarks use a prefix slice.
// ---------------------------------------------------------------------------

static std::vector<char> make_corpus(int64_t size) {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> kind_dist(0, 9);
    std::uniform_int_distribution<int> word_len_dist(3, 12);
    std::uniform_int_distribution<int> word_char_dist('a', 'z');
    std::uniform_int_distribution<int> digit_dist('0', '9');

    static const char* local_chars  = "abcdefghijklmnopqrstuvwxyz0123456789._+-";
    static const char* domain_chars = "abcdefghijklmnopqrstuvwxyz0123456789-";
    static const char* tlds[]       = { "com", "org", "net", "io", "co", "de", "uk", "fr", "jp", "edu" };
    constexpr int N_TLDS = 10;

    std::uniform_int_distribution<int> local_char_idx(0, (int)strlen(local_chars) - 1);
    std::uniform_int_distribution<int> domain_char_idx(0, (int)strlen(domain_chars) - 1);
    std::uniform_int_distribution<int> tld_idx(0, N_TLDS - 1);
    std::uniform_int_distribution<int> local_len_dist(4, 10);
    std::uniform_int_distribution<int> domain_len_dist(3, 8);

    std::vector<char> buf;
    buf.reserve((size_t)size + 64);

    while ((int64_t)buf.size() < size) {
        int kind = kind_dist(rng);
        if (kind < 4) {
            int wlen = word_len_dist(rng);
            for (int i = 0; i < wlen; ++i)
                buf.push_back((char)word_char_dist(rng));
            buf.push_back(' ');
        } else if (kind < 6) {
            int wlen = word_len_dist(rng);
            for (int i = 0; i < wlen; ++i)
                buf.push_back((char)digit_dist(rng));
            buf.push_back(' ');
        } else {
            int llen = local_len_dist(rng);
            int dlen = domain_len_dist(rng);
            const char* tld = tlds[tld_idx(rng)];

            for (int i = 0; i < llen; ++i)
                buf.push_back(local_chars[local_char_idx(rng)]);
            buf.push_back('@');
            for (int i = 0; i < dlen; ++i)
                buf.push_back(domain_chars[domain_char_idx(rng)]);
            buf.push_back('.');
            for (int i = 0; tld[i]; ++i)
                buf.push_back(tld[i]);
            buf.push_back(' ');
        }
    }

    buf.resize((size_t)size);
    return buf;
}

static constexpr int64_t CORPUS_MAX = 1000LL * 1024 * 1024;  // 1 GB

static std::vector<char> g_corpus = make_corpus(CORPUS_MAX);

// ---------------------------------------------------------------------------
// Benchmarks
// ---------------------------------------------------------------------------

static void BM_UC7_unspecialized(benchmark::State& state) {
    int64_t corpus_size = state.range(0);
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            dfa_match(g_corpus.data(), corpus_size,
                      g_dfa_table, DFA_N_STATES, DFA_N_CHARS,
                      DFA_START, DFA_ACCEPT));
    }
}

static void BM_UC7_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(create_dfa_specialized());
    }
}

static void BM_UC7_specialized_exec(benchmark::State& state) {
    int64_t corpus_size = state.range(0);
    auto spec = create_dfa_specialized();
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            spec(g_corpus.data(), corpus_size));
    }
}

#define UC7_BENCHMARK_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UC7_unspecialized)->Name("BM_g:uc7_dfa;n:email;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC7_unspecialized)->Name("BM_g:uc7_dfa;n:email;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC7_unspecialized)->Name("BM_g:uc7_dfa;n:email;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC7_unspecialized)->Name("BM_g:uc7_dfa;n:email;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC7_jit_overhead)->Name("BM_g:uc7_dfa;n:email;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC7_jit_overhead)->Name("BM_g:uc7_dfa;n:email;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC7_jit_overhead)->Name("BM_g:uc7_dfa;n:email;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC7_jit_overhead)->Name("BM_g:uc7_dfa;n:email;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC7_specialized_exec)->Name("BM_g:uc7_dfa;n:email;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC7_specialized_exec)->Name("BM_g:uc7_dfa;n:email;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC7_specialized_exec)->Name("BM_g:uc7_dfa;n:email;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC7_specialized_exec)->Name("BM_g:uc7_dfa;n:email;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

UC7_BENCHMARK_SPEC(
    Arg(5LL * 1024 * 1024),
    Arg(50LL * 1024 * 1024),
    Arg(500LL * 1024 * 1024),
    Arg(1000LL * 1024 * 1024)
)

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

#ifndef ALL_BENCHMARKS_BUILD
int main(int argc, char** argv) {
    validate_dfa_specialized();

    static RSSMemoryManager g_rss_mgr;
    benchmark::RegisterMemoryManager(&g_rss_mgr);
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    if (benchmark::GetBenchmarkFilter() == "")
        benchmark::SetBenchmarkFilter("s:SMALL|s:MEDIUM|s:LARGE");
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
#endif // ALL_BENCHMARKS_BUILD
