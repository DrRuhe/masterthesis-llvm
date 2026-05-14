// CRITICAL: DFAMatcher struct and URL factory live in the same TU so the JIT
// module clone contains both for full inlining.
// Do NOT include benchmark headers here.
#include "UC7Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <stdexcept>

// DFAMatcher struct: captures DFA parameters as specialization constants.
// Same struct definition as in the email tradeoff TU — each TU is independent.
struct DFAMatcher {
    int n_states;
    int n_chars;
    int start_state;
    int accept_state;
    const int* table;

    // Full scan — no early exit per FR-011.
    int64_t match(const char* s, int64_t len) const {
        int state = start_state;
        int64_t matches = 0;
        for (int64_t i = 0; i < len; ++i) {
            int c = (unsigned char)s[i];
            if (c >= n_chars) c = 0;
            int next = table[state * n_chars + c];
            if (next == accept_state && state != accept_state)
                ++matches;
            state = next;
        }
        if (state == accept_state)
            ++matches;
        return matches;
    }
};

UrlMatchTradeoffSpecialized create_url_match_tradeoff_specialized() {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    // URL DFA accept state is 7 (IN_PATH).
    // Capture stable global pointer only; reconstruct DFAMatcher inside so its
    // this-pointer is a local (not a stale factory-frame stack address).
    auto lam = [tbl = g_url_dfa_table](const char* s, int64_t len) -> int64_t {
        DFAMatcher matcher{URL_N_STATES, DFA_N_CHARS, 0, 7, tbl};
        return matcher.match(s, len);
    };
    return RS->specializeLambda<int64_t>(lam);
}

void validate_url_match_tradeoff_specialized() {
    auto spec = create_url_match_tradeoff_specialized();

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
        // Reference: DFAMatcher with URL table
        DFAMatcher ref{URL_N_STATES, DFA_N_CHARS, 0, 7, g_url_dfa_table};
        int64_t expected = ref.match(tc.text, len);
        int64_t actual   = spec(tc.text, len);
        if (expected != actual)
            throw std::runtime_error("validate_url_match_tradeoff_specialized: mismatch on input");
    }
}
