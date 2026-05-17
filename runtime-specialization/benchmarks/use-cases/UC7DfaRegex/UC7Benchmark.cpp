#include "UC7Kernels.h"
#include "ClangRuntimeSpecializerBenchmark.h"
#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <string>
#include <cstring>

// ---------------------------------------------------------------------------
// Synthetic corpus: mix of random ASCII words, email-like strings, and URLs.
// Allocated to EXTRALARGE size; smaller benchmarks use a prefix slice.
// ---------------------------------------------------------------------------

static std::vector<char> make_corpus(int64_t size) {
    // Fast deterministic corpus: ~25% email addresses, ~25% URLs, ~50% word tokens.
    static constexpr char WORDS[] =
        "the quick brown fox jumps over the lazy dog hello world "
        "lorem ipsum dolor sit amet foo bar baz random text here ";
    static constexpr char EMAILS[] =
        "alice@example.com bob@test.org carol@foo.net dave@bar.io "
        "eve@baz.edu frank@mail.org grace@domain.net henry@web.co ";
    static constexpr char URLS[] =
        "http://example.com https://foo.bar/path http://test.org/q "
        "https://abc.def/page?x=1 http://short.ly/abc visit here ";
    constexpr int64_t WL = (int64_t)(sizeof(WORDS) - 1);
    constexpr int64_t EL = (int64_t)(sizeof(EMAILS) - 1);
    constexpr int64_t UL = (int64_t)(sizeof(URLS) - 1);

    std::vector<char> buf(size);
    int64_t pos = 0;
    while (pos < size) {
        for (int b = 0; b < 2 && pos < size; ++b) {
            int64_t n = std::min(WL, size - pos);
            memcpy(buf.data() + pos, WORDS, (size_t)n);
            pos += n;
        }
        if (pos < size) {
            int64_t n = std::min(EL, size - pos);
            memcpy(buf.data() + pos, EMAILS, (size_t)n);
            pos += n;
        }
        if (pos < size) {
            int64_t n = std::min(UL, size - pos);
            memcpy(buf.data() + pos, URLS, (size_t)n);
            pos += n;
        }
    }
    return buf;
}

#ifdef ALL_BENCHMARKS_BUILD
static constexpr int64_t CORPUS_MAX = 1000LL * 1024 * 1024;   // 1 GB for AllBenchmarks
#else
static constexpr int64_t CORPUS_MAX = 500LL * 1024 * 1024;  // 500 MB streaming chunk
#endif

static std::vector<char> g_corpus;

static int g_uc7_refcount = 0;
static void setup_uc7(const benchmark::State&) {
    if (g_uc7_refcount++ == 0) {
        g_corpus = make_corpus(CORPUS_MAX);
    }
}
static void teardown_uc7(const benchmark::State&) {
    if (--g_uc7_refcount == 0) {
        g_corpus.clear(); g_corpus.shrink_to_fit();
    }
}

// Multi-pattern accept states for unspecialized call (low-tier only)
static constexpr int MULTI_ACCEPT[2] = {2, 4};

// ---------------------------------------------------------------------------
// email_match benchmarks (all 3 tiers)
// ---------------------------------------------------------------------------

// --- a:low ---
static void BM_UC7_email_low_unspecialized(benchmark::State& state) {
    int64_t corpus_size = state.range(0);
    for (auto _ : state) {
        int64_t result = 0;
        for (int64_t rem = corpus_size; rem > 0; rem -= CORPUS_MAX)
            result += dfa_match(g_corpus.data(), std::min(rem, CORPUS_MAX),
                                g_dfa_table, DFA_N_STATES, DFA_N_CHARS,
                                DFA_START, DFA_ACCEPT);
        benchmark::DoNotOptimize(result);
    }
}

static void BM_UC7_email_low_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(create_dfa_specialized());
    }
}

static void BM_UC7_email_low_specialized_exec(benchmark::State& state) {
    int64_t corpus_size = state.range(0);
    auto spec = create_dfa_specialized();
    for (auto _ : state) {
        int64_t result = 0;
        for (int64_t rem = corpus_size; rem > 0; rem -= CORPUS_MAX)
            result += spec(g_corpus.data(), std::min(rem, CORPUS_MAX));
        benchmark::DoNotOptimize(result);
    }
}

// --- a:tradeoff ---
static void BM_UC7_email_tradeoff_unspecialized(benchmark::State& state) {
    int64_t corpus_size = state.range(0);
    for (auto _ : state) {
        int64_t result = 0;
        for (int64_t rem = corpus_size; rem > 0; rem -= CORPUS_MAX)
            result += dfa_match(g_corpus.data(), std::min(rem, CORPUS_MAX),
                                g_dfa_table, DFA_N_STATES, DFA_N_CHARS,
                                DFA_START, DFA_ACCEPT);
        benchmark::DoNotOptimize(result);
    }
}

static void BM_UC7_email_tradeoff_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(create_email_match_tradeoff_specialized());
    }
}

static void BM_UC7_email_tradeoff_specialized_exec(benchmark::State& state) {
    int64_t corpus_size = state.range(0);
    auto spec = create_email_match_tradeoff_specialized();
    for (auto _ : state) {
        int64_t result = 0;
        for (int64_t rem = corpus_size; rem > 0; rem -= CORPUS_MAX)
            result += spec(g_corpus.data(), std::min(rem, CORPUS_MAX));
        benchmark::DoNotOptimize(result);
    }
}

// --- a:abstract ---
static void BM_UC7_email_abstract_unspecialized(benchmark::State& state) {
    int64_t corpus_size = state.range(0);
    for (auto _ : state) {
        int64_t result = 0;
        for (int64_t rem = corpus_size; rem > 0; rem -= CORPUS_MAX)
            result += dfa_match(g_corpus.data(), std::min(rem, CORPUS_MAX),
                                g_dfa_table, DFA_N_STATES, DFA_N_CHARS,
                                DFA_START, DFA_ACCEPT);
        benchmark::DoNotOptimize(result);
    }
}

static void BM_UC7_email_abstract_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(create_email_match_abstract_specialized());
    }
}

static void BM_UC7_email_abstract_specialized_exec(benchmark::State& state) {
    int64_t corpus_size = state.range(0);
    auto spec = create_email_match_abstract_specialized();
    for (auto _ : state) {
        int64_t result = 0;
        for (int64_t rem = corpus_size; rem > 0; rem -= CORPUS_MAX)
            result += spec(g_corpus.data(), std::min(rem, CORPUS_MAX));
        benchmark::DoNotOptimize(result);
    }
}

// ---------------------------------------------------------------------------
// url_match benchmarks (all 3 tiers)
// ---------------------------------------------------------------------------

// --- a:low ---
static void BM_UC7_url_low_unspecialized(benchmark::State& state) {
    int64_t corpus_size = state.range(0);
    for (auto _ : state) {
        int64_t result = 0;
        for (int64_t rem = corpus_size; rem > 0; rem -= CORPUS_MAX)
            result += url_match(g_corpus.data(), std::min(rem, CORPUS_MAX),
                                g_url_dfa_table, URL_N_STATES, DFA_N_CHARS, 0, 7);
        benchmark::DoNotOptimize(result);
    }
}

static void BM_UC7_url_low_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(create_url_match_low_specialized());
    }
}

static void BM_UC7_url_low_specialized_exec(benchmark::State& state) {
    int64_t corpus_size = state.range(0);
    auto spec = create_url_match_low_specialized();
    for (auto _ : state) {
        int64_t result = 0;
        for (int64_t rem = corpus_size; rem > 0; rem -= CORPUS_MAX)
            result += spec(g_corpus.data(), std::min(rem, CORPUS_MAX));
        benchmark::DoNotOptimize(result);
    }
}

// --- a:tradeoff ---
static void BM_UC7_url_tradeoff_unspecialized(benchmark::State& state) {
    int64_t corpus_size = state.range(0);
    for (auto _ : state) {
        int64_t result = 0;
        for (int64_t rem = corpus_size; rem > 0; rem -= CORPUS_MAX)
            result += url_match(g_corpus.data(), std::min(rem, CORPUS_MAX),
                                g_url_dfa_table, URL_N_STATES, DFA_N_CHARS, 0, 7);
        benchmark::DoNotOptimize(result);
    }
}

static void BM_UC7_url_tradeoff_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(create_url_match_tradeoff_specialized());
    }
}

static void BM_UC7_url_tradeoff_specialized_exec(benchmark::State& state) {
    int64_t corpus_size = state.range(0);
    auto spec = create_url_match_tradeoff_specialized();
    for (auto _ : state) {
        int64_t result = 0;
        for (int64_t rem = corpus_size; rem > 0; rem -= CORPUS_MAX)
            result += spec(g_corpus.data(), std::min(rem, CORPUS_MAX));
        benchmark::DoNotOptimize(result);
    }
}

// --- a:abstract ---
static void BM_UC7_url_abstract_unspecialized(benchmark::State& state) {
    int64_t corpus_size = state.range(0);
    for (auto _ : state) {
        int64_t result = 0;
        for (int64_t rem = corpus_size; rem > 0; rem -= CORPUS_MAX)
            result += url_match(g_corpus.data(), std::min(rem, CORPUS_MAX),
                                g_url_dfa_table, URL_N_STATES, DFA_N_CHARS, 0, 7);
        benchmark::DoNotOptimize(result);
    }
}

static void BM_UC7_url_abstract_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(create_url_match_abstract_specialized());
    }
}

static void BM_UC7_url_abstract_specialized_exec(benchmark::State& state) {
    int64_t corpus_size = state.range(0);
    auto spec = create_url_match_abstract_specialized();
    for (auto _ : state) {
        int64_t result = 0;
        for (int64_t rem = corpus_size; rem > 0; rem -= CORPUS_MAX)
            result += spec(g_corpus.data(), std::min(rem, CORPUS_MAX));
        benchmark::DoNotOptimize(result);
    }
}

// ---------------------------------------------------------------------------
// multi_pattern_match benchmarks (all 3 tiers)
// ---------------------------------------------------------------------------

// --- a:low ---
static void BM_UC7_multi_low_unspecialized(benchmark::State& state) {
    int64_t corpus_size = state.range(0);
    for (auto _ : state) {
        int64_t result = 0;
        for (int64_t rem = corpus_size; rem > 0; rem -= CORPUS_MAX)
            result += multi_pattern_match_count(g_corpus.data(), std::min(rem, CORPUS_MAX),
                                                g_multi_dfa_table, 5, DFA_N_CHARS,
                                                MULTI_ACCEPT, 2);
        benchmark::DoNotOptimize(result);
    }
}

static void BM_UC7_multi_low_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(create_multi_pattern_match_low_specialized());
    }
}

static void BM_UC7_multi_low_specialized_exec(benchmark::State& state) {
    int64_t corpus_size = state.range(0);
    auto spec = create_multi_pattern_match_low_specialized();
    for (auto _ : state) {
        int64_t result = 0;
        for (int64_t rem = corpus_size; rem > 0; rem -= CORPUS_MAX)
            result += spec(g_corpus.data(), std::min(rem, CORPUS_MAX));
        benchmark::DoNotOptimize(result);
    }
}

// --- a:tradeoff ---
static void BM_UC7_multi_tradeoff_unspecialized(benchmark::State& state) {
    int64_t corpus_size = state.range(0);
    for (auto _ : state) {
        int64_t result = 0;
        for (int64_t rem = corpus_size; rem > 0; rem -= CORPUS_MAX)
            result += multi_pattern_match_count(g_corpus.data(), std::min(rem, CORPUS_MAX),
                                                g_multi_dfa_table, 5, DFA_N_CHARS,
                                                MULTI_ACCEPT, 2);
        benchmark::DoNotOptimize(result);
    }
}

static void BM_UC7_multi_tradeoff_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(create_multi_pattern_match_tradeoff_specialized());
    }
}

static void BM_UC7_multi_tradeoff_specialized_exec(benchmark::State& state) {
    int64_t corpus_size = state.range(0);
    auto spec = create_multi_pattern_match_tradeoff_specialized();
    for (auto _ : state) {
        int64_t result = 0;
        for (int64_t rem = corpus_size; rem > 0; rem -= CORPUS_MAX)
            result += spec(g_corpus.data(), std::min(rem, CORPUS_MAX));
        benchmark::DoNotOptimize(result);
    }
}

// --- a:abstract ---
static void BM_UC7_multi_abstract_unspecialized(benchmark::State& state) {
    int64_t corpus_size = state.range(0);
    for (auto _ : state) {
        int64_t result = 0;
        for (int64_t rem = corpus_size; rem > 0; rem -= CORPUS_MAX)
            result += multi_pattern_match_count(g_corpus.data(), std::min(rem, CORPUS_MAX),
                                                g_multi_dfa_table, 5, DFA_N_CHARS,
                                                MULTI_ACCEPT, 2);
        benchmark::DoNotOptimize(result);
    }
}

static void BM_UC7_multi_abstract_jit_overhead(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(create_multi_pattern_match_abstract_specialized());
    }
}

static void BM_UC7_multi_abstract_specialized_exec(benchmark::State& state) {
    int64_t corpus_size = state.range(0);
    auto spec = create_multi_pattern_match_abstract_specialized();
    for (auto _ : state) {
        int64_t result = 0;
        for (int64_t rem = corpus_size; rem > 0; rem -= CORPUS_MAX)
            result += spec(g_corpus.data(), std::min(rem, CORPUS_MAX));
        benchmark::DoNotOptimize(result);
    }
}

// ---------------------------------------------------------------------------
// Benchmark registration macros
// ---------------------------------------------------------------------------

#define UC7_VARIANT_SPEC(BM_UNSPE, BM_JIT, BM_EXEC, VARIANT, ABSTRACTION, SMALL, MEDIUM, LARGE, EXTRALARGE) \
BENCHMARK(BM_UNSPE)->Name("BM_g:uc7_dfa;n:" VARIANT ";a:" ABSTRACTION ";s:SMALL;t:unspecialized;")->SMALL->Setup(setup_uc7)->Teardown(teardown_uc7)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UNSPE)->Name("BM_g:uc7_dfa;n:" VARIANT ";a:" ABSTRACTION ";s:MEDIUM;t:unspecialized;")->MEDIUM->Setup(setup_uc7)->Teardown(teardown_uc7)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UNSPE)->Name("BM_g:uc7_dfa;n:" VARIANT ";a:" ABSTRACTION ";s:LARGE;t:unspecialized;")->LARGE->Setup(setup_uc7)->Teardown(teardown_uc7)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_UNSPE)->Name("BM_g:uc7_dfa;n:" VARIANT ";a:" ABSTRACTION ";s:EXTRALARGE;t:unspecialized;")->EXTRALARGE->Setup(setup_uc7)->Teardown(teardown_uc7)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_JIT)->Name("BM_g:uc7_dfa;n:" VARIANT ";a:" ABSTRACTION ";s:SMALL;t:jit_overhead;")->SMALL->Setup(setup_uc7)->Teardown(teardown_uc7)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_JIT)->Name("BM_g:uc7_dfa;n:" VARIANT ";a:" ABSTRACTION ";s:MEDIUM;t:jit_overhead;")->MEDIUM->Setup(setup_uc7)->Teardown(teardown_uc7)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_JIT)->Name("BM_g:uc7_dfa;n:" VARIANT ";a:" ABSTRACTION ";s:LARGE;t:jit_overhead;")->LARGE->Setup(setup_uc7)->Teardown(teardown_uc7)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_JIT)->Name("BM_g:uc7_dfa;n:" VARIANT ";a:" ABSTRACTION ";s:EXTRALARGE;t:jit_overhead;")->EXTRALARGE->Setup(setup_uc7)->Teardown(teardown_uc7)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_EXEC)->Name("BM_g:uc7_dfa;n:" VARIANT ";a:" ABSTRACTION ";s:SMALL;t:specialized_exec;")->SMALL->Setup(setup_uc7)->Teardown(teardown_uc7)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_EXEC)->Name("BM_g:uc7_dfa;n:" VARIANT ";a:" ABSTRACTION ";s:MEDIUM;t:specialized_exec;")->MEDIUM->Setup(setup_uc7)->Teardown(teardown_uc7)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_EXEC)->Name("BM_g:uc7_dfa;n:" VARIANT ";a:" ABSTRACTION ";s:LARGE;t:specialized_exec;")->LARGE->Setup(setup_uc7)->Teardown(teardown_uc7)->Unit(benchmark::kMillisecond); \
BENCHMARK(BM_EXEC)->Name("BM_g:uc7_dfa;n:" VARIANT ";a:" ABSTRACTION ";s:EXTRALARGE;t:specialized_exec;")->EXTRALARGE->Setup(setup_uc7)->Teardown(teardown_uc7)->Unit(benchmark::kMillisecond);

#define UC7_BENCHMARK_SPEC(SMALL, MEDIUM, LARGE, EXTRALARGE) \
UC7_VARIANT_SPEC(BM_UC7_email_low_unspecialized,      BM_UC7_email_low_jit_overhead,      BM_UC7_email_low_specialized_exec,      "email_match",         "low",      SMALL, MEDIUM, LARGE, EXTRALARGE) \
UC7_VARIANT_SPEC(BM_UC7_email_tradeoff_unspecialized, BM_UC7_email_tradeoff_jit_overhead, BM_UC7_email_tradeoff_specialized_exec, "email_match",         "tradeoff", SMALL, MEDIUM, LARGE, EXTRALARGE) \
UC7_VARIANT_SPEC(BM_UC7_email_abstract_unspecialized, BM_UC7_email_abstract_jit_overhead, BM_UC7_email_abstract_specialized_exec, "email_match",         "abstract", SMALL, MEDIUM, LARGE, EXTRALARGE) \
UC7_VARIANT_SPEC(BM_UC7_url_low_unspecialized,        BM_UC7_url_low_jit_overhead,        BM_UC7_url_low_specialized_exec,        "url_match",           "low",      SMALL, MEDIUM, LARGE, EXTRALARGE) \
UC7_VARIANT_SPEC(BM_UC7_url_tradeoff_unspecialized,   BM_UC7_url_tradeoff_jit_overhead,   BM_UC7_url_tradeoff_specialized_exec,   "url_match",           "tradeoff", SMALL, MEDIUM, LARGE, EXTRALARGE) \
UC7_VARIANT_SPEC(BM_UC7_url_abstract_unspecialized,   BM_UC7_url_abstract_jit_overhead,   BM_UC7_url_abstract_specialized_exec,   "url_match",           "abstract", SMALL, MEDIUM, LARGE, EXTRALARGE) \
UC7_VARIANT_SPEC(BM_UC7_multi_low_unspecialized,      BM_UC7_multi_low_jit_overhead,      BM_UC7_multi_low_specialized_exec,      "multi_pattern_match", "low",      SMALL, MEDIUM, LARGE, EXTRALARGE) \
UC7_VARIANT_SPEC(BM_UC7_multi_tradeoff_unspecialized, BM_UC7_multi_tradeoff_jit_overhead, BM_UC7_multi_tradeoff_specialized_exec, "multi_pattern_match", "tradeoff", SMALL, MEDIUM, LARGE, EXTRALARGE) \
UC7_VARIANT_SPEC(BM_UC7_multi_abstract_unspecialized, BM_UC7_multi_abstract_jit_overhead, BM_UC7_multi_abstract_specialized_exec, "multi_pattern_match", "abstract", SMALL, MEDIUM, LARGE, EXTRALARGE)

#ifdef ALL_BENCHMARKS_BUILD
UC7_BENCHMARK_SPEC(
    Arg(5LL * 1024 * 1024),
    Arg(50LL * 1024 * 1024),
    Arg(500LL * 1024 * 1024),
    Arg(1000LL * 1024 * 1024)
)
#else
UC7_BENCHMARK_SPEC(
    Arg(44LL * 1024 * 1024),
    Arg(440LL * 1024 * 1024),
    Arg(4350LL * 1024 * 1024),
    Arg(26000LL * 1024 * 1024)
)
#endif

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

#ifndef ALL_BENCHMARKS_BUILD
int main(int argc, char** argv) {
    validate_dfa_specialized();
    validate_email_match_tradeoff_specialized();
    validate_email_match_abstract_specialized();
    validate_url_match_low_specialized();
    validate_url_match_tradeoff_specialized();
    validate_url_match_abstract_specialized();
    validate_multi_pattern_match_low_specialized();
    validate_multi_pattern_match_tradeoff_specialized();
    validate_multi_pattern_match_abstract_specialized();

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
