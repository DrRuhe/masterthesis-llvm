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
    // Fast deterministic corpus: ~25% email addresses, ~75% word tokens.
    // Uses memcpy of small cache-hot templates for multi-GB/s generation speed.
    static constexpr char WORDS[] =
        "the quick brown fox jumps over the lazy dog hello world "
        "lorem ipsum dolor sit amet foo bar baz random text here ";
    static constexpr char EMAILS[] =
        "alice@example.com bob@test.org carol@foo.net dave@bar.io "
        "eve@baz.edu frank@mail.org grace@domain.net henry@web.co ";
    constexpr int64_t WL = (int64_t)(sizeof(WORDS) - 1);
    constexpr int64_t EL = (int64_t)(sizeof(EMAILS) - 1);

    std::vector<char> buf(size);
    int64_t pos = 0;
    while (pos < size) {
        for (int b = 0; b < 3 && pos < size; ++b) {
            int64_t n = std::min(WL, size - pos);
            memcpy(buf.data() + pos, WORDS, (size_t)n);
            pos += n;
        }
        if (pos < size) {
            int64_t n = std::min(EL, size - pos);
            memcpy(buf.data() + pos, EMAILS, (size_t)n);
            pos += n;
        }
    }
    return buf;
}

#ifdef ALL_BENCHMARKS_BUILD
static constexpr int64_t CORPUS_MAX = 1000LL * 1024 * 1024;   // 1 GB for AllBenchmarks
#else
static constexpr int64_t CORPUS_MAX = 15000LL * 1024 * 1024;  // 15 GB for standalone
#endif

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
BENCHMARK(BM_UC7_unspecialized)->Name("BM_g:uc7_dfa;n:email_match;a:low;s:SMALL;t:unspecialized;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC7_unspecialized)->Name("BM_g:uc7_dfa;n:email_match;a:low;s:MEDIUM;t:unspecialized;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC7_unspecialized)->Name("BM_g:uc7_dfa;n:email_match;a:low;s:LARGE;t:unspecialized;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC7_unspecialized)->Name("BM_g:uc7_dfa;n:email_match;a:low;s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC7_jit_overhead)->Name("BM_g:uc7_dfa;n:email_match;a:low;s:SMALL;t:jit_overhead;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC7_jit_overhead)->Name("BM_g:uc7_dfa;n:email_match;a:low;s:MEDIUM;t:jit_overhead;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC7_jit_overhead)->Name("BM_g:uc7_dfa;n:email_match;a:low;s:LARGE;t:jit_overhead;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC7_jit_overhead)->Name("BM_g:uc7_dfa;n:email_match;a:low;s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC7_specialized_exec)->Name("BM_g:uc7_dfa;n:email_match;a:low;s:SMALL;t:specialized_exec;")->SMALL->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC7_specialized_exec)->Name("BM_g:uc7_dfa;n:email_match;a:low;s:MEDIUM;t:specialized_exec;")->MEDIUM->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC7_specialized_exec)->Name("BM_g:uc7_dfa;n:email_match;a:low;s:LARGE;t:specialized_exec;")->LARGE->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UC7_specialized_exec)->Name("BM_g:uc7_dfa;n:email_match;a:low;s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Unit(benchmark::kMillisecond);

#ifdef ALL_BENCHMARKS_BUILD
UC7_BENCHMARK_SPEC(
    Arg(5LL * 1024 * 1024),
    Arg(50LL * 1024 * 1024),
    Arg(500LL * 1024 * 1024),
    Arg(1000LL * 1024 * 1024)
)
#else
UC7_BENCHMARK_SPEC(
    Arg(50LL * 1024 * 1024),
    Arg(500LL * 1024 * 1024),
    Arg(5000LL * 1024 * 1024),
    Arg(15000LL * 1024 * 1024)
)
#endif

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
