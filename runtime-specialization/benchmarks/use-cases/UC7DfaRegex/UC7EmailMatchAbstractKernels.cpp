// CRITICAL: Matcher/DFAMatcher hierarchy and factory live in the same TU so the
// JIT module clone contains the vtable as a JIT constant.
// Do NOT include benchmark headers here.
#include "UC7Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <stdexcept>

// Abstract matcher interface.
struct Matcher {
    virtual int64_t match(const char* s, int64_t len) const = 0;
    virtual ~Matcher() = default;
};

// DFAMatcher subclass: stores table and DFA constants.
// Both base class and subclass defined in this TU so the vtable is a JIT
// constant when the lambda captures a DFAMatcher by value.
struct DFAMatcher : Matcher {
    const int* table;
    int n_states;
    int n_chars;
    int start_state;
    int accept_state;

    DFAMatcher(const int* tbl, int ns, int nc, int ss, int as)
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

EmailMatchAbstractSpecialized create_email_match_abstract_specialized() {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    // Capture DFAMatcher BY VALUE so the vtable pointer is a JIT constant.
    DFAMatcher m(g_dfa_table, DFA_N_STATES, DFA_N_CHARS, DFA_START, DFA_ACCEPT);
    auto lam = [m](const char* s, int64_t len) -> int64_t {
        return m.match(s, len);
    };
    return RS->specializeLambda<int64_t>(lam);
}

void validate_email_match_abstract_specialized() {
    auto spec = create_email_match_abstract_specialized();

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

    DFAMatcher ref(g_dfa_table, DFA_N_STATES, DFA_N_CHARS, DFA_START, DFA_ACCEPT);
    for (auto& tc : cases) {
        int64_t len = (int64_t)strlen(tc.text);
        int64_t expected = ref.match(tc.text, len);
        int64_t actual   = spec(tc.text, len);
        if (expected != actual)
            throw std::runtime_error("validate_email_match_abstract_specialized: mismatch on input");
    }
}
