// CRITICAL: multi_pattern_match kernel and factory live in the same TU so the
// JIT module clone contains both for full inlining.
// Do NOT include benchmark headers here.
#include "UC7Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <stdexcept>

// Multi-pattern DFA: matches either 'ab' or 'cd' using a small DFA.
// DFA state indices (MULTI_N_STATES = 5):
//  0: START
//  1: SAW_A  — consumed 'a'
//  2: ACCEPT_AB — consumed 'ab'  (accept state for pattern "ab")
//  3: SAW_C  — consumed 'c'
//  4: ACCEPT_CD — consumed 'cd'  (accept state for pattern "cd")
//
// After reaching an accept state the DFA resets to START and rescans current char.
// Accept states: {2, 4}

static constexpr int MULTI_N_STATES      = 5;
static constexpr int MULTI_N_ACCEPT      = 2;
static constexpr int MULTI_ACCEPT_STATES[MULTI_N_ACCEPT] = {2, 4};

// Non-static global DFA table for multi-pattern matching.
int g_multi_dfa_table[MULTI_N_STATES * DFA_N_CHARS];

static void build_multi_dfa(int* tbl) {
    for (int s = 0; s < MULTI_N_STATES; ++s)
        for (int c = 0; c < DFA_N_CHARS; ++c)
            tbl[s * DFA_N_CHARS + c] = 0;

    // State 0: START
    tbl[0 * DFA_N_CHARS + 'a'] = 1;
    tbl[0 * DFA_N_CHARS + 'c'] = 3;

    // State 1: SAW_A
    tbl[1 * DFA_N_CHARS + 'b'] = 2;
    tbl[1 * DFA_N_CHARS + 'a'] = 1;  // 'a' again: stay in SAW_A
    tbl[1 * DFA_N_CHARS + 'c'] = 3;  // could start "cd"

    // State 2: ACCEPT_AB — all transitions go back to START
    // (no self-loops; START transitions apply for 'a','c')
    tbl[2 * DFA_N_CHARS + 'a'] = 1;
    tbl[2 * DFA_N_CHARS + 'c'] = 3;

    // State 3: SAW_C
    tbl[3 * DFA_N_CHARS + 'd'] = 4;
    tbl[3 * DFA_N_CHARS + 'a'] = 1;  // could start "ab"
    tbl[3 * DFA_N_CHARS + 'c'] = 3;  // 'c' again: stay in SAW_C

    // State 4: ACCEPT_CD — transitions back to START, same as state 2
    tbl[4 * DFA_N_CHARS + 'a'] = 1;
    tbl[4 * DFA_N_CHARS + 'c'] = 3;
}

struct MultiDFABuilder {
    MultiDFABuilder() { build_multi_dfa(g_multi_dfa_table); }
} g_multi_dfa_builder;

static bool is_accept_state(int state, const int* accept_states, int n_accept) {
    for (int i = 0; i < n_accept; ++i)
        if (state == accept_states[i]) return true;
    return false;
}

// multi_pattern_match_count: scan all bytes, count each time DFA enters any accept state.
// No early exit — full buffer scan per FR-011.
int64_t multi_pattern_match_count(const char* buf, int64_t len,
                                   const int* table, int n_states, int n_chars,
                                   const int* accept_states, int n_accept_states) {
    int state = 0;
    int64_t matches = 0;
    for (int64_t i = 0; i < len; ++i) {
        int c = (unsigned char)buf[i];
        if (c >= n_chars) c = 0;
        int next = table[state * n_chars + c];
        if (is_accept_state(next, accept_states, n_accept_states) &&
            !is_accept_state(state, accept_states, n_accept_states))
            ++matches;
        state = next;
    }
    if (is_accept_state(state, accept_states, n_accept_states))
        ++matches;
    return matches;
}

MultiPatternMatchLowSpecialized create_multi_pattern_match_low_specialized() {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    const int* tbl = g_multi_dfa_table;
    // Capture accept states as local constants so the JIT can fold them.
    const int n_acc = MULTI_N_ACCEPT;
    const int acc0 = MULTI_ACCEPT_STATES[0];
    const int acc1 = MULTI_ACCEPT_STATES[1];
    auto lam = [tbl, n_acc, acc0, acc1](const char* buf, int64_t len) -> int64_t {
        int accept[2] = {acc0, acc1};
        return multi_pattern_match_count(buf, len, tbl, MULTI_N_STATES, DFA_N_CHARS,
                                          accept, n_acc);
    };
    return RS->specializeLambda<int64_t>(lam);
}

void validate_multi_pattern_match_low_specialized() {
    auto spec = create_multi_pattern_match_low_specialized();

    struct TestCase { const char* text; };
    TestCase cases[] = {
        { "ab" },              // 1 match ("ab")
        { "cd" },              // 1 match ("cd")
        { "abcd" },            // 2 matches
        { "xabxcdx" },         // 2 matches
        { "no matches here" }, // 0 matches
        { "" },
        { "ababcdab" },        // 4 matches
    };

    int accept[MULTI_N_ACCEPT] = {MULTI_ACCEPT_STATES[0], MULTI_ACCEPT_STATES[1]};
    for (auto& tc : cases) {
        int64_t len = (int64_t)strlen(tc.text);
        int64_t expected = multi_pattern_match_count(tc.text, len, g_multi_dfa_table,
                                                      MULTI_N_STATES, DFA_N_CHARS,
                                                      accept, MULTI_N_ACCEPT);
        int64_t actual   = spec(tc.text, len);
        if (expected != actual)
            throw std::runtime_error("validate_multi_pattern_match_low_specialized: mismatch on input");
    }
}
