// CRITICAL: DFAMatcher struct and factory live in the same TU so the JIT module
// clone contains both for full inlining.
// Do NOT include benchmark headers here.
#include "UC7Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <stdexcept>

// DFAMatcher struct: captures DFA parameters as specialization constants.
// Lambda captures by value so all fields are JIT constants.
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

int64_t email_match_tradeoff_unspecialized(const char* s, int64_t len) {
    DFAMatcher matcher{DFA_N_STATES, DFA_N_CHARS, DFA_START, DFA_ACCEPT, g_dfa_table};
    return matcher.match(s, len);
}

EmailMatchTradeoffSpecialized create_email_match_tradeoff_specialized() {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    // Capture stable global pointer only; reconstruct DFAMatcher inside so its
    // this-pointer is a local (not a stale factory-frame stack address).
    auto lam = [tbl = g_dfa_table](const char* s, int64_t len) -> int64_t {
        DFAMatcher matcher{DFA_N_STATES, DFA_N_CHARS, DFA_START, DFA_ACCEPT, tbl};
        return matcher.match(s, len);
    };
    return RS->specializeLambda<int64_t>(lam);
}

void validate_email_match_tradeoff_specialized() {
    auto spec = create_email_match_tradeoff_specialized();

    struct TestCase { const char* text; };
    TestCase cases[] = {
        { "hello user@example.com world" },
        { "no emails here 12345 @@ test" },
        { "test@example.com" },
        { "multi a@b.co and c@d.org end" },
        { "" },
        { "bad@ .com nope" },
        { "a@b.c" },
        { "a@b.co" },
    };

    for (auto& tc : cases) {
        int64_t len = (int64_t)strlen(tc.text);
        int64_t expected = dfa_match(tc.text, len, g_dfa_table, DFA_N_STATES,
                                     DFA_N_CHARS, DFA_START, DFA_ACCEPT);
        int64_t actual   = spec(tc.text, len);
        if (expected != actual)
            throw std::runtime_error("validate_email_match_tradeoff_specialized: mismatch on input");
    }
}
