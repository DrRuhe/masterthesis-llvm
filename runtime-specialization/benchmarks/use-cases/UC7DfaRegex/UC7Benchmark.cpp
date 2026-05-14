#include "UC7Kernels.h"
#include "ClangRuntimeSpecializerBenchmark.h"
#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <string>
#include <cstring>

// ---------------------------------------------------------------------------
// Synthetic 50 MB corpus: mix of random ASCII words and email-like strings.
// Fixed seed for reproducibility.
// ---------------------------------------------------------------------------

static std::vector<char> make_corpus(int64_t size) {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> kind_dist(0, 9);
    std::uniform_int_distribution<int> word_len_dist(3, 12);
    // ASCII printable chars excluding '@' and '.' for non-email segments
    std::uniform_int_distribution<int> word_char_dist('a', 'z');
    std::uniform_int_distribution<int> digit_dist('0', '9');

    // Pre-built local parts, domains, TLDs for synthetic emails
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
            // Random lowercase word (40% of tokens)
            int wlen = word_len_dist(rng);
            for (int i = 0; i < wlen; ++i)
                buf.push_back((char)word_char_dist(rng));
            buf.push_back(' ');
        } else if (kind < 6) {
            // Random digits (20%)
            int wlen = word_len_dist(rng);
            for (int i = 0; i < wlen; ++i)
                buf.push_back((char)digit_dist(rng));
            buf.push_back(' ');
        } else {
            // Email-like string (40%): most will be valid emails
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

static std::vector<char> g_corpus = make_corpus(50LL * 1024 * 1024);

// ---------------------------------------------------------------------------
// Benchmarks
// ---------------------------------------------------------------------------

// BM_UC7_unspecialized: direct call with all parameters at runtime
static void BM_UC7_unspecialized(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            dfa_match(g_corpus.data(), (int64_t)g_corpus.size(),
                      g_dfa_table, DFA_N_STATES, DFA_N_CHARS,
                      DFA_START, DFA_ACCEPT));
    }
}
BENCHMARK(BM_UC7_unspecialized)
    ->Name("BM_g:uc7_dfa;n:email;t:unspecialized;")
    ->Unit(benchmark::kMillisecond);

// BM_UC7_jit_overhead: measure JIT compilation cost (single iteration)
static void BM_UC7_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(create_dfa_specialized());
    }
}
BENCHMARK(BM_UC7_jit_overhead)
    ->Name("BM_g:uc7_dfa;n:email;t:jit_overhead;")
    ->Unit(benchmark::kMillisecond);

// BM_UC7_specialized_exec: factory called once before loop, execute specialized fn each iter
static void BM_UC7_specialized_exec(benchmark::State& state) {
    auto spec = create_dfa_specialized();
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            spec(g_corpus.data(), (int64_t)g_corpus.size()));
    }
}
BENCHMARK(BM_UC7_specialized_exec)
    ->Name("BM_g:uc7_dfa;n:email;t:specialized_exec;")
    ->Unit(benchmark::kMillisecond);

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
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
#endif // ALL_BENCHMARKS_BUILD
