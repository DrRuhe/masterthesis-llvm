#pragma once
#include "ClangRuntimeSpecializer.h"
#include <cstdint>

// ---------------------------------------------------------------------------
// DFA table constants shared by all email-match variants
// ---------------------------------------------------------------------------
static constexpr int DFA_N_STATES = 10;
static constexpr int DFA_N_CHARS  = 128;
static constexpr int DFA_START    = 0;
static constexpr int DFA_ACCEPT   = 6;  // IN_TLD_2PLUS: 2+ alpha chars of TLD

// Non-static global DFA table (email); filled at startup by DFABuilder.
// Size: DFA_N_STATES * DFA_N_CHARS entries.
extern int g_dfa_table[];

// ---------------------------------------------------------------------------
// URL DFA constants
// ---------------------------------------------------------------------------
static constexpr int URL_N_STATES = 8;
// URL state indices:
//  0: START, 1: SAW_H, 2: SAW_HT, 3: SAW_HTT, 4: SAW_HTTP,
//  5: SAW_COLON, 6: SAW_SLASH1, 7: IN_PATH (ACCEPT)

// ---------------------------------------------------------------------------
// Type aliases — one per variant × level
// ---------------------------------------------------------------------------

// email_match
using EmailMatchLowSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const char*, int64_t>;
using EmailMatchTradeoffSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const char*, int64_t>;
using EmailMatchAbstractSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const char*, int64_t>;

// url_match
using UrlMatchLowSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const char*, int64_t>;
using UrlMatchTradeoffSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const char*, int64_t>;
using UrlMatchAbstractSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const char*, int64_t>;

// multi_pattern_match
using MultiPatternMatchLowSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const char*, int64_t>;
using MultiPatternMatchTradeoffSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const char*, int64_t>;
using MultiPatternMatchAbstractSpecialized =
    clangRuntimeSpecializer::SpecializedLambda<int64_t, const char*, int64_t>;

// Backward-compatibility alias
using DFASpecialized = EmailMatchLowSpecialized;

// ---------------------------------------------------------------------------
// Kernel declarations
// ---------------------------------------------------------------------------

// DFA loop — table, n_states, n_chars, start_state, accept_state are specialization constants
int64_t dfa_match(const char* haystack, int64_t len,
                  const int* dfa_table, int n_states,
                  int n_chars, int start_state, int accept_state);

// URL DFA loop (same signature, URL table)
int64_t url_match(const char* hay, int64_t len,
                  const int* table, int n_states,
                  int n_chars, int start_state, int accept_state);

// Multi-pattern DFA loop
int64_t multi_pattern_match_count(const char* buf, int64_t len,
                                   const int* table, int n_states, int n_chars,
                                   const int* accept_states, int n_accept_states);

// Non-static global URL DFA table defined in UC7UrlMatchLowKernels.cpp
extern int g_url_dfa_table[];

// Multi-pattern DFA tables (defined in their respective low-tier TUs)
extern int g_multi_dfa_table[];
extern int g_multi_dfa_table_tradeoff[];
extern int g_multi_dfa_table_abstract[];

// ---------------------------------------------------------------------------
// Factory + validation — email_match
// ---------------------------------------------------------------------------

DFASpecialized create_dfa_specialized();
void validate_dfa_specialized();

EmailMatchTradeoffSpecialized create_email_match_tradeoff_specialized();
void validate_email_match_tradeoff_specialized();

EmailMatchAbstractSpecialized create_email_match_abstract_specialized();
void validate_email_match_abstract_specialized();

// ---------------------------------------------------------------------------
// Factory + validation — url_match
// ---------------------------------------------------------------------------

UrlMatchLowSpecialized create_url_match_low_specialized();
void validate_url_match_low_specialized();

UrlMatchTradeoffSpecialized create_url_match_tradeoff_specialized();
void validate_url_match_tradeoff_specialized();

UrlMatchAbstractSpecialized create_url_match_abstract_specialized();
void validate_url_match_abstract_specialized();

// ---------------------------------------------------------------------------
// Factory + validation — multi_pattern_match
// ---------------------------------------------------------------------------

MultiPatternMatchLowSpecialized create_multi_pattern_match_low_specialized();
void validate_multi_pattern_match_low_specialized();

MultiPatternMatchTradeoffSpecialized create_multi_pattern_match_tradeoff_specialized();
void validate_multi_pattern_match_tradeoff_specialized();

MultiPatternMatchAbstractSpecialized create_multi_pattern_match_abstract_specialized();
void validate_multi_pattern_match_abstract_specialized();
