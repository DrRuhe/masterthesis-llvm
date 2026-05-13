#pragma once
#include "ClangRuntimeSpecializer.h"
#include <cstdint>

static constexpr int DFA_N_STATES = 10;
static constexpr int DFA_N_CHARS  = 128;
static constexpr int DFA_START    = 0;
static constexpr int DFA_ACCEPT   = 6;  // IN_TLD_2PLUS: 2+ alpha chars of TLD

// Non-static global DFA table; filled at startup by DFABuilder.
// Size: DFA_N_STATES * DFA_N_CHARS entries.
extern int g_dfa_table[];

// DFA loop — table, n_states, n_chars, start_state, accept_state are specialization constants
int64_t dfa_match(const char* haystack, int64_t len,
                  const int* dfa_table, int n_states,
                  int n_chars, int start_state, int accept_state);

using DFASpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const char*, int64_t>;

DFASpecialized create_dfa_specialized();
void validate_dfa_specialized();
