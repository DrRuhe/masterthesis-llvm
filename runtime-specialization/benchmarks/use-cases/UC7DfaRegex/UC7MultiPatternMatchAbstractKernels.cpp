// CRITICAL: PatternSet/DFAPatternSet hierarchy and factory live in the same TU
// so the JIT module clone contains the vtable as a JIT constant.
// Do NOT include benchmark headers here.
#include "UC7Kernels.h"
#include "ClangRuntimeSpecializer.h"
#include <cstring>
#include <stdexcept>

// Multi-pattern DFA constants (matches 'ab' or 'cd')
static constexpr int MPA_N_STATES  = 5;
static constexpr int MPA_N_CHARS   = 128;
static constexpr int MPA_MAX_ACC   = 8;

// Non-static global multi-pattern DFA table for abstract TU.
int g_multi_dfa_table_abstract[MPA_N_STATES * MPA_N_CHARS];

static void build_multi_dfa_abstract(int* tbl) {
    for (int s = 0; s < MPA_N_STATES; ++s)
        for (int c = 0; c < MPA_N_CHARS; ++c)
            tbl[s * MPA_N_CHARS + c] = 0;

    tbl[0 * MPA_N_CHARS + 'a'] = 1;
    tbl[0 * MPA_N_CHARS + 'c'] = 3;

    tbl[1 * MPA_N_CHARS + 'b'] = 2;
    tbl[1 * MPA_N_CHARS + 'a'] = 1;
    tbl[1 * MPA_N_CHARS + 'c'] = 3;

    tbl[2 * MPA_N_CHARS + 'a'] = 1;
    tbl[2 * MPA_N_CHARS + 'c'] = 3;

    tbl[3 * MPA_N_CHARS + 'd'] = 4;
    tbl[3 * MPA_N_CHARS + 'a'] = 1;
    tbl[3 * MPA_N_CHARS + 'c'] = 3;

    tbl[4 * MPA_N_CHARS + 'a'] = 1;
    tbl[4 * MPA_N_CHARS + 'c'] = 3;
}

struct MultiDFABuilderAbstract {
    MultiDFABuilderAbstract() { build_multi_dfa_abstract(g_multi_dfa_table_abstract); }
} g_multi_dfa_builder_abstract;

// Abstract pattern set interface.
struct PatternSet {
    virtual int64_t match_all_count(const char* buf, int64_t len) const = 0;

};

// DFAPatternSet subclass: stores DFA table + accept_states fixed array.
// Both base and subclass in this TU so the vtable is a JIT constant.
struct DFAPatternSet : PatternSet {
    const int* table;
    int n_states;
    int n_chars;
    int accept_states[MPA_MAX_ACC];
    int n_accept_states;

    DFAPatternSet(const int* tbl, int ns, int nc,
                   const int* acc, int n_acc)
        : table(tbl), n_states(ns), n_chars(nc), n_accept_states(n_acc) {
        for (int i = 0; i < MPA_MAX_ACC; ++i)
            accept_states[i] = (i < n_acc) ? acc[i] : -1;
    }

    // Full scan — no early exit per FR-011.
    int64_t match_all_count(const char* buf, int64_t len) const override {
        int state = 0;
        int64_t matches = 0;
        for (int64_t i = 0; i < len; ++i) {
            int c = (unsigned char)buf[i];
            if (c >= n_chars) c = 0;
            int next = table[state * n_chars + c];
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
        for (int j = 0; j < n_accept_states; ++j)
            if (state == accept_states[j]) { ++matches; break; }
        return matches;
    }
};

MultiPatternMatchAbstractSpecialized create_multi_pattern_match_abstract_specialized() {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    // Capture the DFA table pointer (a stable global address, not a stack address).
    // Reconstruct DFAPatternSet inside the lambda so its this-pointer is local.
    auto lam = [dfa_table = g_multi_dfa_table_abstract](const char* buf, int64_t len) -> int64_t {
        const int acc[2] = {2, 4};  // ACCEPT_AB, ACCEPT_CD
        DFAPatternSet ps(dfa_table, MPA_N_STATES, MPA_N_CHARS, acc, 2);
        return ps.match_all_count(buf, len);
    };
    return RS->specializeLambda<int64_t>(lam);
}

void validate_multi_pattern_match_abstract_specialized() {
    auto spec = create_multi_pattern_match_abstract_specialized();

    const int acc[2] = {2, 4};
    DFAPatternSet ref(g_multi_dfa_table_abstract, MPA_N_STATES, MPA_N_CHARS, acc, 2);

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
            throw std::runtime_error("validate_multi_pattern_match_abstract_specialized: mismatch on input");
    }
}
