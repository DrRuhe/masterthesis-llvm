// CRITICAL: Matcher/AbstractDFAMatcher hierarchy and URL factory live in the same TU
// so the JIT module clone contains the vtable as a JIT constant.
// Do NOT include benchmark headers here.
#include "UC7Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <stdexcept>

// Abstract matcher interface — local definition (not shared with other TUs).
struct Matcher {
    virtual int64_t match(const char* s, int64_t len) const = 0;

};

// AbstractDFAMatcher subclass with URL DFA.
struct AbstractDFAMatcher : Matcher {
    const int* table;
    int n_states;
    int n_chars;
    int start_state;
    int accept_state;

    AbstractDFAMatcher(const int* tbl, int ns, int nc, int ss, int as)
        : table(tbl), n_states(ns), n_chars(nc), start_state(ss), accept_state(as) {}

    // Full scan — no early exit per FR-011.
    int64_t match(const char* s, int64_t len) const override {
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

int64_t url_match_abstract_unspecialized(const char* s, int64_t len) {
    AbstractDFAMatcher m(g_url_dfa_table, URL_N_STATES, DFA_N_CHARS, 0, 7);
    return m.match(s, len);
}

UrlMatchAbstractSpecialized create_url_match_abstract_specialized() {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    // Capture the DFA table pointer (a stable global address, not a stack address).
    // Reconstruct AbstractDFAMatcher inside the lambda so its this-pointer is local.
    // URL DFA accept state is 7 (IN_PATH).
    auto lam = [dfa_table = g_url_dfa_table](const char* s, int64_t len) -> int64_t {
        AbstractDFAMatcher m(dfa_table, URL_N_STATES, DFA_N_CHARS, 0, 7);
        return m.match(s, len);
    };
    return RS->specializeLambda<int64_t>(lam);
}

void validate_url_match_abstract_specialized() {
    auto spec = create_url_match_abstract_specialized();

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

    AbstractDFAMatcher ref(g_url_dfa_table, URL_N_STATES, DFA_N_CHARS, 0, 7);
    for (auto& tc : cases) {
        int64_t len = (int64_t)strlen(tc.text);
        int64_t expected = ref.match(tc.text, len);
        int64_t actual   = spec(tc.text, len);
        if (expected != actual)
            throw std::runtime_error("validate_url_match_abstract_specialized: mismatch on input");
    }
}
