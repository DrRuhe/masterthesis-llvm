// CRITICAL: MultiPatternMatcher struct and factory live in the same TU so the
// JIT module clone contains both for full inlining.
// Do NOT include benchmark headers here.
#include "UC7Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <stdexcept>

// Multi-pattern DFA constants (matches 'ab' or 'cd')
static constexpr int MP_N_STATES  = 5;
static constexpr int MP_N_CHARS   = 128;
static constexpr int MP_MAX_ACC   = 8;

// Non-static global multi-pattern DFA table for tradeoff TU.
int g_multi_dfa_table_tradeoff[MP_N_STATES * MP_N_CHARS];

static void build_multi_dfa_tradeoff(int* tbl) {
    for (int s = 0; s < MP_N_STATES; ++s)
        for (int c = 0; c < MP_N_CHARS; ++c)
            tbl[s * MP_N_CHARS + c] = 0;

    tbl[0 * MP_N_CHARS + 'a'] = 1;
    tbl[0 * MP_N_CHARS + 'c'] = 3;

    tbl[1 * MP_N_CHARS + 'b'] = 2;
    tbl[1 * MP_N_CHARS + 'a'] = 1;
    tbl[1 * MP_N_CHARS + 'c'] = 3;

    tbl[2 * MP_N_CHARS + 'a'] = 1;
    tbl[2 * MP_N_CHARS + 'c'] = 3;

    tbl[3 * MP_N_CHARS + 'd'] = 4;
    tbl[3 * MP_N_CHARS + 'a'] = 1;
    tbl[3 * MP_N_CHARS + 'c'] = 3;

    tbl[4 * MP_N_CHARS + 'a'] = 1;
    tbl[4 * MP_N_CHARS + 'c'] = 3;
}

struct MultiDFABuilderTradeoff {
    MultiDFABuilderTradeoff() { build_multi_dfa_tradeoff(g_multi_dfa_table_tradeoff); }
} g_multi_dfa_builder_tradeoff;

// MultiPatternMatcher: captures DFA table, state/char counts, and fixed accept array.
// Lambda captures by value so all fields are JIT constants.
struct MultiPatternMatcher {
    int n_states;
    int n_chars;
    int accept_states[MP_MAX_ACC];
    int n_accept_states;
    const int* table;

    // Full scan — no early exit per FR-011.
    int64_t match_all_count(const char* buf, int64_t len) const {
        int state = 0;
        int64_t matches = 0;
        for (int64_t i = 0; i < len; ++i) {
            int c = (unsigned char)buf[i];
            if (c >= n_chars) c = 0;
            int next = table[state * n_chars + c];
            // Check if transitioning into any accept state
            bool prev_accept = false;
            bool next_accept = false;
            for (int j = 0; j < n_accept_states; ++j) {
                if (state == accept_states[j]) prev_accept = true;
                if (next == accept_states[j]) next_accept = true;
            }
            if (next_accept && !prev_accept)
                ++matches;
            state = next;
        }
        // Check if final state is an accept state
        for (int j = 0; j < n_accept_states; ++j)
            if (state == accept_states[j]) { ++matches; break; }
        return matches;
    }
};

MultiPatternMatchTradeoffSpecialized create_multi_pattern_match_tradeoff_specialized() {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    // Capture stable global pointer only; reconstruct MultiPatternMatcher inside so
    // its this-pointer is a local (not a stale factory-frame stack address).
    auto lam = [tbl = g_multi_dfa_table_tradeoff](const char* buf, int64_t len) -> int64_t {
        MultiPatternMatcher matcher;
        matcher.n_states = MP_N_STATES;
        matcher.n_chars  = MP_N_CHARS;
        matcher.n_accept_states = 2;
        matcher.accept_states[0] = 2;  // ACCEPT_AB
        matcher.accept_states[1] = 4;  // ACCEPT_CD
        for (int i = 2; i < MP_MAX_ACC; ++i) matcher.accept_states[i] = -1;
        matcher.table = tbl;
        return matcher.match_all_count(buf, len);
    };
    return RS->specializeLambda<int64_t>(lam);
}

void validate_multi_pattern_match_tradeoff_specialized() {
    auto spec = create_multi_pattern_match_tradeoff_specialized();

    MultiPatternMatcher ref;
    ref.n_states = MP_N_STATES;
    ref.n_chars  = MP_N_CHARS;
    ref.n_accept_states = 2;
    ref.accept_states[0] = 2;
    ref.accept_states[1] = 4;
    for (int i = 2; i < MP_MAX_ACC; ++i) ref.accept_states[i] = -1;
    ref.table = g_multi_dfa_table_tradeoff;

    struct TestCase { const char* text; };
    TestCase cases[] = {
        { "ab" },
        { "cd" },
        { "abcd" },
        { "xabxcdx" },
        { "no matches here" },
        { "" },
        { "ababcdab" },
    };

    for (auto& tc : cases) {
        int64_t len = (int64_t)strlen(tc.text);
        int64_t expected = ref.match_all_count(tc.text, len);
        int64_t actual   = spec(tc.text, len);
        if (expected != actual)
            throw std::runtime_error("validate_multi_pattern_match_tradeoff_specialized: mismatch on input");
    }
}
