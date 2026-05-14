// CRITICAL: url_match kernel and factory live in the same TU so the JIT module
// clone contains both for full inlining.
// Do NOT include benchmark headers here.
#include "UC7Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <stdexcept>

// URL DFA state indices (must match URL_N_STATES = 8 in header)
// 0: START        — no valid prefix yet
// 1: SAW_H        — consumed 'h'
// 2: SAW_HT       — consumed 'ht'
// 3: SAW_HTT      — consumed 'htt'
// 4: SAW_HTTP     — consumed 'http' (or 'https' via extra 's' loop)
// 5: SAW_COLON    — consumed 'http:' or 'https:'
// 6: SAW_SLASH1   — consumed 'http:/'
// 7: IN_PATH      — ACCEPT: consumed 'http://' followed by ≥1 non-space char

// Non-static global — non-static ensures JIT can resolve its address via
// DynamicLibrarySearchGenerator.
int g_url_dfa_table[URL_N_STATES * DFA_N_CHARS];

static void build_url_dfa(int* tbl) {
    // Initialize all transitions to 0 (reset to START on unknown chars)
    for (int s = 0; s < URL_N_STATES; ++s)
        for (int c = 0; c < DFA_N_CHARS; ++c)
            tbl[s * DFA_N_CHARS + c] = 0;

    // State 0: START — 'h' -> SAW_H(1)
    tbl[0 * DFA_N_CHARS + 'h'] = 1;

    // State 1: SAW_H — 't' -> SAW_HT(2), else -> START(0)
    tbl[1 * DFA_N_CHARS + 't'] = 2;

    // State 2: SAW_HT — 't' -> SAW_HTT(3), else -> START(0)
    tbl[2 * DFA_N_CHARS + 't'] = 3;

    // State 3: SAW_HTT — 'p' -> SAW_HTTP(4), else -> START(0)
    tbl[3 * DFA_N_CHARS + 'p'] = 4;

    // State 4: SAW_HTTP — ':' -> SAW_COLON(5), 's' -> stay SAW_HTTP(4) (for https)
    tbl[4 * DFA_N_CHARS + ':'] = 5;
    tbl[4 * DFA_N_CHARS + 's'] = 4;

    // State 5: SAW_COLON — '/' -> SAW_SLASH1(6), else -> START(0)
    tbl[5 * DFA_N_CHARS + '/'] = 6;

    // State 6: SAW_SLASH1 — '/' -> IN_PATH(7), else -> START(0)
    tbl[6 * DFA_N_CHARS + '/'] = 7;

    // State 7: IN_PATH (ACCEPT) — non-space printable ASCII stays in IN_PATH(7)
    // space/control chars -> START(0), 'h' -> SAW_H(1) (new URL could start)
    for (int c = 33; c < DFA_N_CHARS; ++c) {  // printable non-space (33..127)
        tbl[7 * DFA_N_CHARS + c] = 7;
    }
    // Override: 'h' in path could start a new URL — stay in path (greedy)
    // We intentionally keep 'h' -> IN_PATH(7) to count URL by transition into state 7.
    // space/control (0..32) -> 0 (already set); tab(9), newline(10), CR(13) already 0
}

// URLBuilder: fills the global table at program startup.
struct URLDFABuilder {
    URLDFABuilder() { build_url_dfa(g_url_dfa_table); }
} g_url_dfa_builder;

// url_match: count URL-like patterns (http:// or https://) in haystack.
// Counts a match each time DFA transitions INTO the accept state (7) from a
// non-accept state. After reaching accept, stays in accept while consuming
// non-space chars (the URL path). On whitespace/reset, goes back to START.
// Full scan — no early exit.
int64_t url_match(const char* hay, int64_t len,
                  const int* table, int n_states,
                  int n_chars, int start_state, int accept_state) {
    int state = start_state;
    int64_t matches = 0;
    for (int64_t i = 0; i < len; ++i) {
        int c = (unsigned char)hay[i];
        if (c >= n_chars) c = 0;
        int next = table[state * n_chars + c];
        if (next == accept_state && state != accept_state)
            ++matches;
        state = next;
    }
    // URL that extends to the very end of the buffer
    if (state == accept_state)
        ++matches;
    return matches;
}

// create_url_match_low_specialized: bake in the URL DFA table pointer and constants.
UrlMatchLowSpecialized create_url_match_low_specialized() {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    const int* tbl = g_url_dfa_table;
    auto lam = [tbl](const char* hay, int64_t len) -> int64_t {
        return url_match(hay, len, tbl, URL_N_STATES, DFA_N_CHARS,
                         0, 7);
    };
    return RS->specializeLambda<int64_t>(lam);
}

// validate_url_match_low_specialized: checks correctness on several test inputs.
void validate_url_match_low_specialized() {
    auto spec = create_url_match_low_specialized();

    struct TestCase { const char* text; };
    TestCase cases[] = {
        { "visit http://example.com for info" },
        { "no urls here just text" },
        { "https://foo.bar/path?q=1 end" },
        { "http://a.b and https://c.d done" },
        { "" },
        { "http:/missing-slash" },
        { "http://x" },
    };

    for (auto& tc : cases) {
        int64_t len = (int64_t)strlen(tc.text);
        int64_t expected = url_match(tc.text, len, g_url_dfa_table, URL_N_STATES,
                                     DFA_N_CHARS, 0, 7);
        int64_t actual   = spec(tc.text, len);
        if (expected != actual)
            throw std::runtime_error("validate_url_match_low_specialized: mismatch on input");
    }
}
