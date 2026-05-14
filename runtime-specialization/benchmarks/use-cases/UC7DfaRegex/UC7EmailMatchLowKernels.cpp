// CRITICAL: dfa_match must live in the same TU as the lambda factory (create_dfa_specialized)
// so the JIT module clone contains both for full inlining.
// Do NOT include benchmark headers here.
#include "UC7Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <stdexcept>

// DFA state indices (must match DFA_N_STATES = 10 in header)
// 0: START        — no valid prefix yet
// 1: IN_LOCAL     — consuming local part chars [a-zA-Z0-9._%+-]
// 2: SAW_AT       — just saw '@', need at least one domain label char
// 3: IN_DOMAIN    — consuming domain label chars [a-zA-Z0-9-] after '@'
// 4: SAW_DOT      — just saw '.' after at least one domain label char
// 5: IN_TLD_1     — first alpha char of TLD [a-zA-Z]
// 6: IN_TLD_2PLUS — 2+ alpha chars of TLD (ACCEPT STATE: valid TLD length)
// 7: (unused/reserved)
// 8: IN_DOMAIN2   — domain label after a dot (subdomain label)
// 9: DEAD/RESET   — behaves same as START (redirect to 0 via transition)

// Non-static global — non-static ensures JIT can resolve its address via
// DynamicLibrarySearchGenerator.
int g_dfa_table[DFA_N_STATES * DFA_N_CHARS];

static bool is_local_char(unsigned char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') ||
           c == '.' || c == '_' || c == '%' || c == '+' || c == '-';
}

static bool is_alnum(unsigned char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9');
}

static bool is_alpha(unsigned char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool is_domain_char(unsigned char c) {
    return is_alnum(c) || c == '-';
}

static void build_email_dfa(int* tbl) {
    // Initialize all transitions to 0 (reset to START on unknown chars)
    for (int s = 0; s < DFA_N_STATES; ++s)
        for (int c = 0; c < DFA_N_CHARS; ++c)
            tbl[s * DFA_N_CHARS + c] = 0;

    for (int c = 0; c < DFA_N_CHARS; ++c) {
        unsigned char uc = (unsigned char)c;

        // State 0: START
        // local-part char -> IN_LOCAL(1), else stay at START(0)
        if (is_local_char(uc))
            tbl[0 * DFA_N_CHARS + c] = 1;

        // State 1: IN_LOCAL
        // local-part char -> stay IN_LOCAL(1)
        // '@' -> SAW_AT(2)
        // else -> START(0)
        if (is_local_char(uc))
            tbl[1 * DFA_N_CHARS + c] = 1;
        else if (uc == '@')
            tbl[1 * DFA_N_CHARS + c] = 2;
        // (else already 0)

        // State 2: SAW_AT
        // alnum -> IN_DOMAIN(3), else -> START(0)
        if (is_alnum(uc))
            tbl[2 * DFA_N_CHARS + c] = 3;

        // State 3: IN_DOMAIN
        // domain char -> stay IN_DOMAIN(3)
        // '.' -> SAW_DOT(4)
        // else -> START(0)
        if (is_domain_char(uc))
            tbl[3 * DFA_N_CHARS + c] = 3;
        else if (uc == '.')
            tbl[3 * DFA_N_CHARS + c] = 4;

        // State 4: SAW_DOT
        // alpha -> IN_TLD_1(5)   (potential TLD start)
        // digit -> IN_DOMAIN2(8) (another domain label)
        // '-'   -> IN_DOMAIN2(8) (label char, not TLD)
        // else  -> START(0)
        if (is_alpha(uc))
            tbl[4 * DFA_N_CHARS + c] = 5;
        else if ((uc >= '0' && uc <= '9') || uc == '-')
            tbl[4 * DFA_N_CHARS + c] = 8;

        // State 5: IN_TLD_1
        // alpha -> IN_TLD_2PLUS(6) (now 2 alpha chars — valid minimum TLD)
        // else  -> START(0)
        if (is_alpha(uc))
            tbl[5 * DFA_N_CHARS + c] = 6;

        // State 6: IN_TLD_2PLUS (ACCEPT)
        // alpha -> stay IN_TLD_2PLUS(6) (longer TLD like .com, .info)
        // '.'   -> SAW_DOT(4)           (dot after TLD — e.g. foo@bar.co.uk)
        // else  -> START(0)             (end of this email)
        if (is_alpha(uc))
            tbl[6 * DFA_N_CHARS + c] = 6;
        else if (uc == '.')
            tbl[6 * DFA_N_CHARS + c] = 4;

        // State 7: unused — all transitions remain 0

        // State 8: IN_DOMAIN2 (subdomain label after a dot)
        // domain char -> stay IN_DOMAIN2(8)
        // '.' -> SAW_DOT(4)
        // else -> START(0)
        if (is_domain_char(uc))
            tbl[8 * DFA_N_CHARS + c] = 8;
        else if (uc == '.')
            tbl[8 * DFA_N_CHARS + c] = 4;

        // State 9: DEAD — all transitions go to 0 (already set)
    }
}

// DFABuilder: fills the global table at program startup.
struct DFABuilder {
    DFABuilder() { build_email_dfa(g_dfa_table); }
} g_dfa_builder;

// dfa_match: count email-like patterns in haystack.
// The DFA counts a match each time we transition INTO the accept state from a
// non-accept state. When state == accept_state at end of buffer, an extra match
// is counted (the email extends to the end of the input).
//
// table, n_states, n_chars, start_state, accept_state are specialization
// constants: the JIT folds them into the specialized function body.
int64_t dfa_match(const char* hay, int64_t len,
                  const int* table, int n_states,
                  int n_chars, int start_state, int accept_state) {
    int state = start_state;
    int64_t matches = 0;
    for (int64_t i = 0; i < len; ++i) {
        int c = (unsigned char)hay[i];
        if (c >= n_chars) c = 0;  // map high bytes to reset
        int next = table[state * n_chars + c];
        if (next == accept_state && state != accept_state)
            ++matches;
        state = next;
    }
    // Email that extends to the very end of the buffer
    if (state == accept_state)
        ++matches;
    return matches;
}

// create_dfa_specialized: bake in the DFA table pointer and all constants.
// Both dfa_match and this factory are in the same TU so the JIT clone can
// inline dfa_match into the specialized lambda body.
DFASpecialized create_dfa_specialized() {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    const int* tbl = g_dfa_table;
    auto lam = [tbl](const char* hay, int64_t len) -> int64_t {
        return dfa_match(hay, len, tbl, DFA_N_STATES, DFA_N_CHARS,
                         DFA_START, DFA_ACCEPT);
    };
    return RS->specializeLambda<int64_t>(lam);
}

// validate_dfa_specialized: checks that the specialized function produces the
// same results as the direct dfa_match call on several test inputs.
// Uses create_dfa_specialized() (the factory) so no additional specializeLambda
// call sites are needed here — mirrors UC1's validation pattern.
void validate_dfa_specialized() {
    auto spec = create_dfa_specialized();

    struct TestCase { const char* text; };
    TestCase cases[] = {
        { "hello user@example.com world" },
        { "no emails here 12345 @@ test" },
        { "test@example.com" },
        { "multi a@b.co and c@d.org end" },
        { "" },
        { "bad@ .com nope" },
        { "a@b.c" },          // TLD only 1 char — should NOT match
        { "a@b.co" },         // TLD 2 chars — should match
    };

    for (auto& tc : cases) {
        int64_t len = (int64_t)strlen(tc.text);
        int64_t expected = dfa_match(tc.text, len, g_dfa_table, DFA_N_STATES,
                                     DFA_N_CHARS, DFA_START, DFA_ACCEPT);
        int64_t actual   = spec(tc.text, len);
        if (expected != actual)
            throw std::runtime_error("validate_dfa_specialized: mismatch on input");
    }
}
