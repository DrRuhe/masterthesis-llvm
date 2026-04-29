
#include <benchmark/benchmark.h>
#include "ClangRuntimeSpecializerBenchmark.h"
#include "ClangRuntimeSpecializer.h"
#include <string>
#include <string_view>
#include <vector>
#include <cmath>
#include "duckdb.h"

namespace CRS = clangRuntimeSpecializer;

// ── DuckDB C API entry point ──────────────────────────────────────────────────
extern "C" duckdb_state duckdb_execute_prepared(duckdb_prepared_statement prepared_statement,
                                                 duckdb_result* out_result);

inline constexpr char Fn_duckdb_execute_prepared[] = "duckdb_execute_prepared";

// ── Global result slot ────────────────────────────────────────────────────────
static duckdb_result g_duckdb_exec_result;

// ── Paths ─────────────────────────────────────────────────────────────────────

static std::string g_duckdb_db_path = "tpch/data/tpch.duckdb";
static std::string g_duckdb_queries_dir = "tpch/queries/duckdb";

#ifdef ALL_BENCHMARKS_BUILD
void duckdb_tpch_set_db_path(const char* path) { g_duckdb_db_path = path; }
void duckdb_tpch_set_queries_dir(const char* path) { g_duckdb_queries_dir = path; }
#endif

// ── Runtime query loading ─────────────────────────────────────────────────────
// Queries are loaded from g_duckdb_queries_dir/q<NN>.sql on first access.
// The SQL string is intentionally NOT a compile-time constant so the JIT sees
// the actual pointer value rather than a folded string literal.

static const std::string& getQuery(int q_num) {
    static std::string queries[22];
    int idx = q_num - 1;
    if (queries[idx].empty()) {
        char path[512];
        snprintf(path, sizeof(path), "%s/q%02d.sql", g_duckdb_queries_dir.c_str(), q_num);
        FILE* f = fopen(path, "r");
        if (!f) {
            fprintf(stderr, "Cannot open query file: %s\n", path);
            exit(1);
        }
        fseek(f, 0, SEEK_END);
        long sz = ftell(f);
        rewind(f);
        queries[idx].resize((size_t)sz);
        if (sz > 0)
            (void)fread(&queries[idx][0], 1, (size_t)sz, f);
        fclose(f);
    }
    return queries[idx];
}

// ── IR embedding trigger ──────────────────────────────────────────────────────

#ifndef ALL_BENCHMARKS_BUILD
volatile bool g_duckdb_tpch_dummy_trigger = false;
extern "C" __attribute__((used)) void duckdb_tpch_dummy_registration() {
    auto* RS = CRS::ClangRuntimeSpecializer::init();
    if (g_duckdb_tpch_dummy_trigger)
        RS->callSpecialized<duckdb_state>(Fn_duckdb_execute_prepared,
                                          (duckdb_prepared_statement) nullptr,
                                          &g_duckdb_exec_result);
}
#else
volatile bool g_duckdb_tpch_dummy_trigger_duckdb = false;
extern "C" __attribute__((used)) void duckdb_tpch_dummy_registration_duckdb() {
    auto* RS = CRS::ClangRuntimeSpecializer::init();
    if (g_duckdb_tpch_dummy_trigger_duckdb)
        RS->callSpecialized<duckdb_state>(Fn_duckdb_execute_prepared,
                                          (duckdb_prepared_statement) nullptr,
                                          &g_duckdb_exec_result);
}
#endif

// ── Helpers ───────────────────────────────────────────────────────────────────

static duckdb_database openDuckDB() {
    duckdb_database db;
    if (duckdb_open(g_duckdb_db_path.c_str(), &db) != DuckDBSuccess) {
        fprintf(stderr, "Cannot open DuckDB database '%s'\n", g_duckdb_db_path.c_str());
        exit(1);
    }
    return db;
}

static duckdb_prepared_statement prepareQuery(duckdb_connection con, const char* sql) {
    duckdb_prepared_statement stmt;
    if (duckdb_prepare(con, sql, &stmt) != DuckDBSuccess) {
        fprintf(stderr, "Failed to prepare DuckDB query: %s\n", duckdb_prepare_error(stmt));
        exit(1);
    }
    return stmt;
}

// ── Phase implementations ─────────────────────────────────────────────────────

static void phaseUnspecialized(benchmark::State& state, const char* sql) {
    duckdb_database db = openDuckDB();
    duckdb_connection con;
    duckdb_connect(db, &con);
    duckdb_prepared_statement stmt = prepareQuery(con, sql);

    for (auto _ : state) {
        duckdb_result res;
        duckdb_execute_prepared(stmt, &res);
        duckdb_destroy_result(&res);
    }

    duckdb_destroy_prepare(&stmt);
    duckdb_disconnect(&con);
    duckdb_close(&db);
}

static bool g_last_jit_timed_out = false;

static void phaseJITOverhead(benchmark::State& state, const char* sql) {
    g_last_jit_timed_out = false;

    duckdb_database db = openDuckDB();
    duckdb_connection con;
    duckdb_connect(db, &con);
    duckdb_prepared_statement stmt = prepareQuery(con, sql);

    auto* RS = CRS::ClangRuntimeSpecializer::init();
    auto modStats = CRS::ClangRuntimeSpecializer::getModuleStats();
    auto Prev = CRS::ClangRuntimeSpecializer::getLogLevel();
    CRS::ClangRuntimeSpecializer::setLogLevel(CRS::ClangRuntimeSpecializer::LogLevel::None);

    auto opts = CRS::ClangRuntimeSpecializer::Options::Default().withJITTimeoutSeconds(60);
    CRS::SpecializedFunction<duckdb_state> specFn;
    for (auto _ : state)
        specFn = RS->specializeOnly<duckdb_state>(Fn_duckdb_execute_prepared, opts,
                                                  stmt, &g_duckdb_exec_result);
    CRS::ClangRuntimeSpecializer::setLogLevel(Prev);

    g_last_jit_timed_out = !specFn;
    auto txStats = CRS::ClangRuntimeSpecializer::getLastTransformStats();

    state.counters["jit_module_fns"]    = (double)modStats.FunctionCount;
    state.counters["jit_module_instrs"] = (double)modStats.InstructionCount;
    state.counters["jit_blob_kb"]       = (double)(modStats.BitcodeSizeBytes / 1024);
    state.counters["jit_pruned_fns"]    = (double)txStats.FunctionCountAfterPrune;
    state.counters["jit_pruned_instrs"] = (double)txStats.InstructionCountAfterPrune;
    state.counters["jit_timeout"]       = g_last_jit_timed_out ? 1.0 : 0.0;

    duckdb_destroy_prepare(&stmt);
    duckdb_disconnect(&con);
    duckdb_close(&db);
}

static void phaseSpecializedExec(benchmark::State& state, const char* sql) {
    if (g_last_jit_timed_out) { state.SkipWithMessage("jit_timed_out"); return; }

    duckdb_database db = openDuckDB();
    duckdb_connection con;
    duckdb_connect(db, &con);
    duckdb_prepared_statement stmt = prepareQuery(con, sql);

    auto* RS = CRS::ClangRuntimeSpecializer::init();
    auto Prev = CRS::ClangRuntimeSpecializer::getLogLevel();
    CRS::ClangRuntimeSpecializer::setLogLevel(CRS::ClangRuntimeSpecializer::LogLevel::None);
    auto SpecFn = RS->specializeOnly<duckdb_state>(Fn_duckdb_execute_prepared,
                                                    stmt, &g_duckdb_exec_result);
    CRS::ClangRuntimeSpecializer::setLogLevel(Prev);

    for (auto _ : state) {
        duckdb_destroy_result(&g_duckdb_exec_result);
        SpecFn();
    }

    duckdb_result unspec_result;
    duckdb_execute_prepared(stmt, &unspec_result);

    double max_diff = 0.0;
    idx_t col_count = duckdb_column_count(&g_duckdb_exec_result);
    idx_t row_count = duckdb_row_count(&g_duckdb_exec_result);
    idx_t unspec_rows = duckdb_row_count(&unspec_result);
    if (row_count == unspec_rows) {
        for (idx_t c = 0; c < col_count; ++c) {
            auto type = duckdb_column_type(&g_duckdb_exec_result, c);
            if (type == DUCKDB_TYPE_DOUBLE || type == DUCKDB_TYPE_FLOAT) {
                for (idx_t r = 0; r < row_count; ++r) {
                    double v_spec   = duckdb_value_double(&g_duckdb_exec_result, c, r);
                    double v_unspec = duckdb_value_double(&unspec_result, c, r);
                    double diff = std::fabs(v_spec - v_unspec);
                    if (diff > max_diff) max_diff = diff;
                }
            }
        }
    }
    state.counters["result_max_diff"] = max_diff;

    duckdb_destroy_result(&unspec_result);
    duckdb_destroy_result(&g_duckdb_exec_result);
    duckdb_destroy_prepare(&stmt);
    duckdb_disconnect(&con);
    duckdb_close(&db);
}

static void phaseJITAnalysis(benchmark::State& state, const char* sql) {
    if (g_last_jit_timed_out) { state.SkipWithMessage("jit_timed_out"); return; }

    duckdb_database db = openDuckDB();
    duckdb_connection con;
    duckdb_connect(db, &con);
    duckdb_prepared_statement stmt = prepareQuery(con, sql);

    clangRuntimeSpecializer::benchmarkJITAnalysis<Fn_duckdb_execute_prepared>(
        state, duckdb_execute_prepared,
        std::make_tuple(stmt, &g_duckdb_exec_result));

    duckdb_destroy_result(&g_duckdb_exec_result);
    duckdb_destroy_prepare(&stmt);
    duckdb_disconnect(&con);
    duckdb_close(&db);
}

// ── Q1 ────────────────────────────────────────────────────────────────────────

void BM_duckdb_unspecialized_q1(benchmark::State& state) { phaseUnspecialized(state, getQuery(1).c_str()); }
BENCHMARK(BM_duckdb_unspecialized_q1)->Name("BM_g:db/duckdb/tpch;n:tpch_q1;t:unspecialized;")->MinTime(1.0);

void BM_duckdb_jit_overhead_q1(benchmark::State& state) { phaseJITOverhead(state, getQuery(1).c_str()); }
BENCHMARK(BM_duckdb_jit_overhead_q1)->Name("BM_g:db/duckdb/tpch;n:tpch_q1;t:jit_overhead;")->Iterations(1);

void BM_duckdb_specialized_exec_q1(benchmark::State& state) { phaseSpecializedExec(state, getQuery(1).c_str()); }
BENCHMARK(BM_duckdb_specialized_exec_q1)->Name("BM_g:db/duckdb/tpch;n:tpch_q1;t:specialized_exec;")->MinTime(1.0);

void BM_duckdb_jit_analysis_q1(benchmark::State& state) { phaseJITAnalysis(state, getQuery(1).c_str()); }
BENCHMARK(BM_duckdb_jit_analysis_q1)->Name("BM_g:db/duckdb/tpch;n:tpch_q1;t:jit_analysis;")->Iterations(1)->UseManualTime();

// ── Q2 ────────────────────────────────────────────────────────────────────────

void BM_duckdb_unspecialized_q2(benchmark::State& state) { phaseUnspecialized(state, getQuery(2).c_str()); }
BENCHMARK(BM_duckdb_unspecialized_q2)->Name("BM_g:db/duckdb/tpch;n:tpch_q2;t:unspecialized;")->MinTime(1.0);

void BM_duckdb_jit_overhead_q2(benchmark::State& state) { phaseJITOverhead(state, getQuery(2).c_str()); }
BENCHMARK(BM_duckdb_jit_overhead_q2)->Name("BM_g:db/duckdb/tpch;n:tpch_q2;t:jit_overhead;")->Iterations(1);

void BM_duckdb_specialized_exec_q2(benchmark::State& state) { phaseSpecializedExec(state, getQuery(2).c_str()); }
BENCHMARK(BM_duckdb_specialized_exec_q2)->Name("BM_g:db/duckdb/tpch;n:tpch_q2;t:specialized_exec;")->MinTime(1.0);

void BM_duckdb_jit_analysis_q2(benchmark::State& state) { phaseJITAnalysis(state, getQuery(2).c_str()); }
BENCHMARK(BM_duckdb_jit_analysis_q2)->Name("BM_g:db/duckdb/tpch;n:tpch_q2;t:jit_analysis;")->Iterations(1)->UseManualTime();

// ── Q3 ────────────────────────────────────────────────────────────────────────

void BM_duckdb_unspecialized_q3(benchmark::State& state) { phaseUnspecialized(state, getQuery(3).c_str()); }
BENCHMARK(BM_duckdb_unspecialized_q3)->Name("BM_g:db/duckdb/tpch;n:tpch_q3;t:unspecialized;")->MinTime(1.0);

void BM_duckdb_jit_overhead_q3(benchmark::State& state) { phaseJITOverhead(state, getQuery(3).c_str()); }
BENCHMARK(BM_duckdb_jit_overhead_q3)->Name("BM_g:db/duckdb/tpch;n:tpch_q3;t:jit_overhead;")->Iterations(1);

void BM_duckdb_specialized_exec_q3(benchmark::State& state) { phaseSpecializedExec(state, getQuery(3).c_str()); }
BENCHMARK(BM_duckdb_specialized_exec_q3)->Name("BM_g:db/duckdb/tpch;n:tpch_q3;t:specialized_exec;")->MinTime(1.0);

void BM_duckdb_jit_analysis_q3(benchmark::State& state) { phaseJITAnalysis(state, getQuery(3).c_str()); }
BENCHMARK(BM_duckdb_jit_analysis_q3)->Name("BM_g:db/duckdb/tpch;n:tpch_q3;t:jit_analysis;")->Iterations(1)->UseManualTime();

// ── Q4 ────────────────────────────────────────────────────────────────────────

void BM_duckdb_unspecialized_q4(benchmark::State& state) { phaseUnspecialized(state, getQuery(4).c_str()); }
BENCHMARK(BM_duckdb_unspecialized_q4)->Name("BM_g:db/duckdb/tpch;n:tpch_q4;t:unspecialized;")->MinTime(1.0);

void BM_duckdb_jit_overhead_q4(benchmark::State& state) { phaseJITOverhead(state, getQuery(4).c_str()); }
BENCHMARK(BM_duckdb_jit_overhead_q4)->Name("BM_g:db/duckdb/tpch;n:tpch_q4;t:jit_overhead;")->Iterations(1);

void BM_duckdb_specialized_exec_q4(benchmark::State& state) { phaseSpecializedExec(state, getQuery(4).c_str()); }
BENCHMARK(BM_duckdb_specialized_exec_q4)->Name("BM_g:db/duckdb/tpch;n:tpch_q4;t:specialized_exec;")->MinTime(1.0);

void BM_duckdb_jit_analysis_q4(benchmark::State& state) { phaseJITAnalysis(state, getQuery(4).c_str()); }
BENCHMARK(BM_duckdb_jit_analysis_q4)->Name("BM_g:db/duckdb/tpch;n:tpch_q4;t:jit_analysis;")->Iterations(1)->UseManualTime();

// ── Q5 ────────────────────────────────────────────────────────────────────────

void BM_duckdb_unspecialized_q5(benchmark::State& state) { phaseUnspecialized(state, getQuery(5).c_str()); }
BENCHMARK(BM_duckdb_unspecialized_q5)->Name("BM_g:db/duckdb/tpch;n:tpch_q5;t:unspecialized;")->MinTime(1.0);

void BM_duckdb_jit_overhead_q5(benchmark::State& state) { phaseJITOverhead(state, getQuery(5).c_str()); }
BENCHMARK(BM_duckdb_jit_overhead_q5)->Name("BM_g:db/duckdb/tpch;n:tpch_q5;t:jit_overhead;")->Iterations(1);

void BM_duckdb_specialized_exec_q5(benchmark::State& state) { phaseSpecializedExec(state, getQuery(5).c_str()); }
BENCHMARK(BM_duckdb_specialized_exec_q5)->Name("BM_g:db/duckdb/tpch;n:tpch_q5;t:specialized_exec;")->MinTime(1.0);

void BM_duckdb_jit_analysis_q5(benchmark::State& state) { phaseJITAnalysis(state, getQuery(5).c_str()); }
BENCHMARK(BM_duckdb_jit_analysis_q5)->Name("BM_g:db/duckdb/tpch;n:tpch_q5;t:jit_analysis;")->Iterations(1)->UseManualTime();

// ── Q6 ────────────────────────────────────────────────────────────────────────

void BM_duckdb_unspecialized_q6(benchmark::State& state) { phaseUnspecialized(state, getQuery(6).c_str()); }
BENCHMARK(BM_duckdb_unspecialized_q6)->Name("BM_g:db/duckdb/tpch;n:tpch_q6;t:unspecialized;")->MinTime(1.0);

void BM_duckdb_jit_overhead_q6(benchmark::State& state) { phaseJITOverhead(state, getQuery(6).c_str()); }
BENCHMARK(BM_duckdb_jit_overhead_q6)->Name("BM_g:db/duckdb/tpch;n:tpch_q6;t:jit_overhead;")->Iterations(1);

void BM_duckdb_specialized_exec_q6(benchmark::State& state) { phaseSpecializedExec(state, getQuery(6).c_str()); }
BENCHMARK(BM_duckdb_specialized_exec_q6)->Name("BM_g:db/duckdb/tpch;n:tpch_q6;t:specialized_exec;")->MinTime(1.0);

void BM_duckdb_jit_analysis_q6(benchmark::State& state) { phaseJITAnalysis(state, getQuery(6).c_str()); }
BENCHMARK(BM_duckdb_jit_analysis_q6)->Name("BM_g:db/duckdb/tpch;n:tpch_q6;t:jit_analysis;")->Iterations(1)->UseManualTime();

// ── Q7 ────────────────────────────────────────────────────────────────────────

void BM_duckdb_unspecialized_q7(benchmark::State& state) { phaseUnspecialized(state, getQuery(7).c_str()); }
BENCHMARK(BM_duckdb_unspecialized_q7)->Name("BM_g:db/duckdb/tpch;n:tpch_q7;t:unspecialized;")->MinTime(1.0);

void BM_duckdb_jit_overhead_q7(benchmark::State& state) { phaseJITOverhead(state, getQuery(7).c_str()); }
BENCHMARK(BM_duckdb_jit_overhead_q7)->Name("BM_g:db/duckdb/tpch;n:tpch_q7;t:jit_overhead;")->Iterations(1);

void BM_duckdb_specialized_exec_q7(benchmark::State& state) { phaseSpecializedExec(state, getQuery(7).c_str()); }
BENCHMARK(BM_duckdb_specialized_exec_q7)->Name("BM_g:db/duckdb/tpch;n:tpch_q7;t:specialized_exec;")->MinTime(1.0);

void BM_duckdb_jit_analysis_q7(benchmark::State& state) { phaseJITAnalysis(state, getQuery(7).c_str()); }
BENCHMARK(BM_duckdb_jit_analysis_q7)->Name("BM_g:db/duckdb/tpch;n:tpch_q7;t:jit_analysis;")->Iterations(1)->UseManualTime();

// ── Q8 ────────────────────────────────────────────────────────────────────────

void BM_duckdb_unspecialized_q8(benchmark::State& state) { phaseUnspecialized(state, getQuery(8).c_str()); }
BENCHMARK(BM_duckdb_unspecialized_q8)->Name("BM_g:db/duckdb/tpch;n:tpch_q8;t:unspecialized;")->MinTime(1.0);

void BM_duckdb_jit_overhead_q8(benchmark::State& state) { phaseJITOverhead(state, getQuery(8).c_str()); }
BENCHMARK(BM_duckdb_jit_overhead_q8)->Name("BM_g:db/duckdb/tpch;n:tpch_q8;t:jit_overhead;")->Iterations(1);

void BM_duckdb_specialized_exec_q8(benchmark::State& state) { phaseSpecializedExec(state, getQuery(8).c_str()); }
BENCHMARK(BM_duckdb_specialized_exec_q8)->Name("BM_g:db/duckdb/tpch;n:tpch_q8;t:specialized_exec;")->MinTime(1.0);

void BM_duckdb_jit_analysis_q8(benchmark::State& state) { phaseJITAnalysis(state, getQuery(8).c_str()); }
BENCHMARK(BM_duckdb_jit_analysis_q8)->Name("BM_g:db/duckdb/tpch;n:tpch_q8;t:jit_analysis;")->Iterations(1)->UseManualTime();

// ── Q9 ────────────────────────────────────────────────────────────────────────

void BM_duckdb_unspecialized_q9(benchmark::State& state) { phaseUnspecialized(state, getQuery(9).c_str()); }
BENCHMARK(BM_duckdb_unspecialized_q9)->Name("BM_g:db/duckdb/tpch;n:tpch_q9;t:unspecialized;")->MinTime(1.0);

void BM_duckdb_jit_overhead_q9(benchmark::State& state) { phaseJITOverhead(state, getQuery(9).c_str()); }
BENCHMARK(BM_duckdb_jit_overhead_q9)->Name("BM_g:db/duckdb/tpch;n:tpch_q9;t:jit_overhead;")->Iterations(1);

void BM_duckdb_specialized_exec_q9(benchmark::State& state) { phaseSpecializedExec(state, getQuery(9).c_str()); }
BENCHMARK(BM_duckdb_specialized_exec_q9)->Name("BM_g:db/duckdb/tpch;n:tpch_q9;t:specialized_exec;")->MinTime(1.0);

void BM_duckdb_jit_analysis_q9(benchmark::State& state) { phaseJITAnalysis(state, getQuery(9).c_str()); }
BENCHMARK(BM_duckdb_jit_analysis_q9)->Name("BM_g:db/duckdb/tpch;n:tpch_q9;t:jit_analysis;")->Iterations(1)->UseManualTime();

// ── Q10 ───────────────────────────────────────────────────────────────────────

void BM_duckdb_unspecialized_q10(benchmark::State& state) { phaseUnspecialized(state, getQuery(10).c_str()); }
BENCHMARK(BM_duckdb_unspecialized_q10)->Name("BM_g:db/duckdb/tpch;n:tpch_q10;t:unspecialized;")->MinTime(1.0);

void BM_duckdb_jit_overhead_q10(benchmark::State& state) { phaseJITOverhead(state, getQuery(10).c_str()); }
BENCHMARK(BM_duckdb_jit_overhead_q10)->Name("BM_g:db/duckdb/tpch;n:tpch_q10;t:jit_overhead;")->Iterations(1);

void BM_duckdb_specialized_exec_q10(benchmark::State& state) { phaseSpecializedExec(state, getQuery(10).c_str()); }
BENCHMARK(BM_duckdb_specialized_exec_q10)->Name("BM_g:db/duckdb/tpch;n:tpch_q10;t:specialized_exec;")->MinTime(1.0);

void BM_duckdb_jit_analysis_q10(benchmark::State& state) { phaseJITAnalysis(state, getQuery(10).c_str()); }
BENCHMARK(BM_duckdb_jit_analysis_q10)->Name("BM_g:db/duckdb/tpch;n:tpch_q10;t:jit_analysis;")->Iterations(1)->UseManualTime();

// ── Q11 ───────────────────────────────────────────────────────────────────────

void BM_duckdb_unspecialized_q11(benchmark::State& state) { phaseUnspecialized(state, getQuery(11).c_str()); }
BENCHMARK(BM_duckdb_unspecialized_q11)->Name("BM_g:db/duckdb/tpch;n:tpch_q11;t:unspecialized;")->MinTime(1.0);

void BM_duckdb_jit_overhead_q11(benchmark::State& state) { phaseJITOverhead(state, getQuery(11).c_str()); }
BENCHMARK(BM_duckdb_jit_overhead_q11)->Name("BM_g:db/duckdb/tpch;n:tpch_q11;t:jit_overhead;")->Iterations(1);

void BM_duckdb_specialized_exec_q11(benchmark::State& state) { phaseSpecializedExec(state, getQuery(11).c_str()); }
BENCHMARK(BM_duckdb_specialized_exec_q11)->Name("BM_g:db/duckdb/tpch;n:tpch_q11;t:specialized_exec;")->MinTime(1.0);

void BM_duckdb_jit_analysis_q11(benchmark::State& state) { phaseJITAnalysis(state, getQuery(11).c_str()); }
BENCHMARK(BM_duckdb_jit_analysis_q11)->Name("BM_g:db/duckdb/tpch;n:tpch_q11;t:jit_analysis;")->Iterations(1)->UseManualTime();

// ── Q12 ───────────────────────────────────────────────────────────────────────

void BM_duckdb_unspecialized_q12(benchmark::State& state) { phaseUnspecialized(state, getQuery(12).c_str()); }
BENCHMARK(BM_duckdb_unspecialized_q12)->Name("BM_g:db/duckdb/tpch;n:tpch_q12;t:unspecialized;")->MinTime(1.0);

void BM_duckdb_jit_overhead_q12(benchmark::State& state) { phaseJITOverhead(state, getQuery(12).c_str()); }
BENCHMARK(BM_duckdb_jit_overhead_q12)->Name("BM_g:db/duckdb/tpch;n:tpch_q12;t:jit_overhead;")->Iterations(1);

void BM_duckdb_specialized_exec_q12(benchmark::State& state) { phaseSpecializedExec(state, getQuery(12).c_str()); }
BENCHMARK(BM_duckdb_specialized_exec_q12)->Name("BM_g:db/duckdb/tpch;n:tpch_q12;t:specialized_exec;")->MinTime(1.0);

void BM_duckdb_jit_analysis_q12(benchmark::State& state) { phaseJITAnalysis(state, getQuery(12).c_str()); }
BENCHMARK(BM_duckdb_jit_analysis_q12)->Name("BM_g:db/duckdb/tpch;n:tpch_q12;t:jit_analysis;")->Iterations(1)->UseManualTime();

// ── Q13 ───────────────────────────────────────────────────────────────────────

void BM_duckdb_unspecialized_q13(benchmark::State& state) { phaseUnspecialized(state, getQuery(13).c_str()); }
BENCHMARK(BM_duckdb_unspecialized_q13)->Name("BM_g:db/duckdb/tpch;n:tpch_q13;t:unspecialized;")->MinTime(1.0);

void BM_duckdb_jit_overhead_q13(benchmark::State& state) { phaseJITOverhead(state, getQuery(13).c_str()); }
BENCHMARK(BM_duckdb_jit_overhead_q13)->Name("BM_g:db/duckdb/tpch;n:tpch_q13;t:jit_overhead;")->Iterations(1);

void BM_duckdb_specialized_exec_q13(benchmark::State& state) { phaseSpecializedExec(state, getQuery(13).c_str()); }
BENCHMARK(BM_duckdb_specialized_exec_q13)->Name("BM_g:db/duckdb/tpch;n:tpch_q13;t:specialized_exec;")->MinTime(1.0);

void BM_duckdb_jit_analysis_q13(benchmark::State& state) { phaseJITAnalysis(state, getQuery(13).c_str()); }
BENCHMARK(BM_duckdb_jit_analysis_q13)->Name("BM_g:db/duckdb/tpch;n:tpch_q13;t:jit_analysis;")->Iterations(1)->UseManualTime();

// ── Q14 ───────────────────────────────────────────────────────────────────────

void BM_duckdb_unspecialized_q14(benchmark::State& state) { phaseUnspecialized(state, getQuery(14).c_str()); }
BENCHMARK(BM_duckdb_unspecialized_q14)->Name("BM_g:db/duckdb/tpch;n:tpch_q14;t:unspecialized;")->MinTime(1.0);

void BM_duckdb_jit_overhead_q14(benchmark::State& state) { phaseJITOverhead(state, getQuery(14).c_str()); }
BENCHMARK(BM_duckdb_jit_overhead_q14)->Name("BM_g:db/duckdb/tpch;n:tpch_q14;t:jit_overhead;")->Iterations(1);

void BM_duckdb_specialized_exec_q14(benchmark::State& state) { phaseSpecializedExec(state, getQuery(14).c_str()); }
BENCHMARK(BM_duckdb_specialized_exec_q14)->Name("BM_g:db/duckdb/tpch;n:tpch_q14;t:specialized_exec;")->MinTime(1.0);

void BM_duckdb_jit_analysis_q14(benchmark::State& state) { phaseJITAnalysis(state, getQuery(14).c_str()); }
BENCHMARK(BM_duckdb_jit_analysis_q14)->Name("BM_g:db/duckdb/tpch;n:tpch_q14;t:jit_analysis;")->Iterations(1)->UseManualTime();

// ── Q15 ───────────────────────────────────────────────────────────────────────

void BM_duckdb_unspecialized_q15(benchmark::State& state) { phaseUnspecialized(state, getQuery(15).c_str()); }
BENCHMARK(BM_duckdb_unspecialized_q15)->Name("BM_g:db/duckdb/tpch;n:tpch_q15;t:unspecialized;")->MinTime(1.0);

void BM_duckdb_jit_overhead_q15(benchmark::State& state) { phaseJITOverhead(state, getQuery(15).c_str()); }
BENCHMARK(BM_duckdb_jit_overhead_q15)->Name("BM_g:db/duckdb/tpch;n:tpch_q15;t:jit_overhead;")->Iterations(1);

void BM_duckdb_specialized_exec_q15(benchmark::State& state) { phaseSpecializedExec(state, getQuery(15).c_str()); }
BENCHMARK(BM_duckdb_specialized_exec_q15)->Name("BM_g:db/duckdb/tpch;n:tpch_q15;t:specialized_exec;")->MinTime(1.0);

void BM_duckdb_jit_analysis_q15(benchmark::State& state) { phaseJITAnalysis(state, getQuery(15).c_str()); }
BENCHMARK(BM_duckdb_jit_analysis_q15)->Name("BM_g:db/duckdb/tpch;n:tpch_q15;t:jit_analysis;")->Iterations(1)->UseManualTime();

// ── Q16 ───────────────────────────────────────────────────────────────────────

void BM_duckdb_unspecialized_q16(benchmark::State& state) { phaseUnspecialized(state, getQuery(16).c_str()); }
BENCHMARK(BM_duckdb_unspecialized_q16)->Name("BM_g:db/duckdb/tpch;n:tpch_q16;t:unspecialized;")->MinTime(1.0);

void BM_duckdb_jit_overhead_q16(benchmark::State& state) { phaseJITOverhead(state, getQuery(16).c_str()); }
BENCHMARK(BM_duckdb_jit_overhead_q16)->Name("BM_g:db/duckdb/tpch;n:tpch_q16;t:jit_overhead;")->Iterations(1);

void BM_duckdb_specialized_exec_q16(benchmark::State& state) { phaseSpecializedExec(state, getQuery(16).c_str()); }
BENCHMARK(BM_duckdb_specialized_exec_q16)->Name("BM_g:db/duckdb/tpch;n:tpch_q16;t:specialized_exec;")->MinTime(1.0);

void BM_duckdb_jit_analysis_q16(benchmark::State& state) { phaseJITAnalysis(state, getQuery(16).c_str()); }
BENCHMARK(BM_duckdb_jit_analysis_q16)->Name("BM_g:db/duckdb/tpch;n:tpch_q16;t:jit_analysis;")->Iterations(1)->UseManualTime();

// ── Q17 ───────────────────────────────────────────────────────────────────────

void BM_duckdb_unspecialized_q17(benchmark::State& state) { phaseUnspecialized(state, getQuery(17).c_str()); }
BENCHMARK(BM_duckdb_unspecialized_q17)->Name("BM_g:db/duckdb/tpch;n:tpch_q17;t:unspecialized;")->MinTime(1.0);

void BM_duckdb_jit_overhead_q17(benchmark::State& state) { phaseJITOverhead(state, getQuery(17).c_str()); }
BENCHMARK(BM_duckdb_jit_overhead_q17)->Name("BM_g:db/duckdb/tpch;n:tpch_q17;t:jit_overhead;")->Iterations(1);

void BM_duckdb_specialized_exec_q17(benchmark::State& state) { phaseSpecializedExec(state, getQuery(17).c_str()); }
BENCHMARK(BM_duckdb_specialized_exec_q17)->Name("BM_g:db/duckdb/tpch;n:tpch_q17;t:specialized_exec;")->MinTime(1.0);

void BM_duckdb_jit_analysis_q17(benchmark::State& state) { phaseJITAnalysis(state, getQuery(17).c_str()); }
BENCHMARK(BM_duckdb_jit_analysis_q17)->Name("BM_g:db/duckdb/tpch;n:tpch_q17;t:jit_analysis;")->Iterations(1)->UseManualTime();

// ── Q18 ───────────────────────────────────────────────────────────────────────

void BM_duckdb_unspecialized_q18(benchmark::State& state) { phaseUnspecialized(state, getQuery(18).c_str()); }
BENCHMARK(BM_duckdb_unspecialized_q18)->Name("BM_g:db/duckdb/tpch;n:tpch_q18;t:unspecialized;")->MinTime(1.0);

void BM_duckdb_jit_overhead_q18(benchmark::State& state) { phaseJITOverhead(state, getQuery(18).c_str()); }
BENCHMARK(BM_duckdb_jit_overhead_q18)->Name("BM_g:db/duckdb/tpch;n:tpch_q18;t:jit_overhead;")->Iterations(1);

void BM_duckdb_specialized_exec_q18(benchmark::State& state) { phaseSpecializedExec(state, getQuery(18).c_str()); }
BENCHMARK(BM_duckdb_specialized_exec_q18)->Name("BM_g:db/duckdb/tpch;n:tpch_q18;t:specialized_exec;")->MinTime(1.0);

void BM_duckdb_jit_analysis_q18(benchmark::State& state) { phaseJITAnalysis(state, getQuery(18).c_str()); }
BENCHMARK(BM_duckdb_jit_analysis_q18)->Name("BM_g:db/duckdb/tpch;n:tpch_q18;t:jit_analysis;")->Iterations(1)->UseManualTime();

// ── Q19 ───────────────────────────────────────────────────────────────────────

void BM_duckdb_unspecialized_q19(benchmark::State& state) { phaseUnspecialized(state, getQuery(19).c_str()); }
BENCHMARK(BM_duckdb_unspecialized_q19)->Name("BM_g:db/duckdb/tpch;n:tpch_q19;t:unspecialized;")->MinTime(1.0);

void BM_duckdb_jit_overhead_q19(benchmark::State& state) { phaseJITOverhead(state, getQuery(19).c_str()); }
BENCHMARK(BM_duckdb_jit_overhead_q19)->Name("BM_g:db/duckdb/tpch;n:tpch_q19;t:jit_overhead;")->Iterations(1);

void BM_duckdb_specialized_exec_q19(benchmark::State& state) { phaseSpecializedExec(state, getQuery(19).c_str()); }
BENCHMARK(BM_duckdb_specialized_exec_q19)->Name("BM_g:db/duckdb/tpch;n:tpch_q19;t:specialized_exec;")->MinTime(1.0);

void BM_duckdb_jit_analysis_q19(benchmark::State& state) { phaseJITAnalysis(state, getQuery(19).c_str()); }
BENCHMARK(BM_duckdb_jit_analysis_q19)->Name("BM_g:db/duckdb/tpch;n:tpch_q19;t:jit_analysis;")->Iterations(1)->UseManualTime();

// ── Q20 ───────────────────────────────────────────────────────────────────────

void BM_duckdb_unspecialized_q20(benchmark::State& state) { phaseUnspecialized(state, getQuery(20).c_str()); }
BENCHMARK(BM_duckdb_unspecialized_q20)->Name("BM_g:db/duckdb/tpch;n:tpch_q20;t:unspecialized;")->MinTime(1.0);

void BM_duckdb_jit_overhead_q20(benchmark::State& state) { phaseJITOverhead(state, getQuery(20).c_str()); }
BENCHMARK(BM_duckdb_jit_overhead_q20)->Name("BM_g:db/duckdb/tpch;n:tpch_q20;t:jit_overhead;")->Iterations(1);

void BM_duckdb_specialized_exec_q20(benchmark::State& state) { phaseSpecializedExec(state, getQuery(20).c_str()); }
BENCHMARK(BM_duckdb_specialized_exec_q20)->Name("BM_g:db/duckdb/tpch;n:tpch_q20;t:specialized_exec;")->MinTime(1.0);

void BM_duckdb_jit_analysis_q20(benchmark::State& state) { phaseJITAnalysis(state, getQuery(20).c_str()); }
BENCHMARK(BM_duckdb_jit_analysis_q20)->Name("BM_g:db/duckdb/tpch;n:tpch_q20;t:jit_analysis;")->Iterations(1)->UseManualTime();

// ── Q21 ───────────────────────────────────────────────────────────────────────

void BM_duckdb_unspecialized_q21(benchmark::State& state) { phaseUnspecialized(state, getQuery(21).c_str()); }
BENCHMARK(BM_duckdb_unspecialized_q21)->Name("BM_g:db/duckdb/tpch;n:tpch_q21;t:unspecialized;")->MinTime(1.0);

void BM_duckdb_jit_overhead_q21(benchmark::State& state) { phaseJITOverhead(state, getQuery(21).c_str()); }
BENCHMARK(BM_duckdb_jit_overhead_q21)->Name("BM_g:db/duckdb/tpch;n:tpch_q21;t:jit_overhead;")->Iterations(1);

void BM_duckdb_specialized_exec_q21(benchmark::State& state) { phaseSpecializedExec(state, getQuery(21).c_str()); }
BENCHMARK(BM_duckdb_specialized_exec_q21)->Name("BM_g:db/duckdb/tpch;n:tpch_q21;t:specialized_exec;")->MinTime(1.0);

void BM_duckdb_jit_analysis_q21(benchmark::State& state) { phaseJITAnalysis(state, getQuery(21).c_str()); }
BENCHMARK(BM_duckdb_jit_analysis_q21)->Name("BM_g:db/duckdb/tpch;n:tpch_q21;t:jit_analysis;")->Iterations(1)->UseManualTime();

// ── Q22 ───────────────────────────────────────────────────────────────────────

void BM_duckdb_unspecialized_q22(benchmark::State& state) { phaseUnspecialized(state, getQuery(22).c_str()); }
BENCHMARK(BM_duckdb_unspecialized_q22)->Name("BM_g:db/duckdb/tpch;n:tpch_q22;t:unspecialized;")->MinTime(1.0);

void BM_duckdb_jit_overhead_q22(benchmark::State& state) { phaseJITOverhead(state, getQuery(22).c_str()); }
BENCHMARK(BM_duckdb_jit_overhead_q22)->Name("BM_g:db/duckdb/tpch;n:tpch_q22;t:jit_overhead;")->Iterations(1);

void BM_duckdb_specialized_exec_q22(benchmark::State& state) { phaseSpecializedExec(state, getQuery(22).c_str()); }
BENCHMARK(BM_duckdb_specialized_exec_q22)->Name("BM_g:db/duckdb/tpch;n:tpch_q22;t:specialized_exec;")->MinTime(1.0);

void BM_duckdb_jit_analysis_q22(benchmark::State& state) { phaseJITAnalysis(state, getQuery(22).c_str()); }
BENCHMARK(BM_duckdb_jit_analysis_q22)->Name("BM_g:db/duckdb/tpch;n:tpch_q22;t:jit_analysis;")->Iterations(1)->UseManualTime();

// ── Entry point ───────────────────────────────────────────────────────────────

#ifndef ALL_BENCHMARKS_BUILD
int main(int argc, char** argv) {
    std::vector<char*> bargs;
    bargs.push_back(argv[0]);
    for (int i = 1; i < argc; ++i) {
        std::string_view a(argv[i]);
        if (a.rfind("--duckdb-db=", 0) == 0)
            g_duckdb_db_path = std::string(a.substr(12));
        else if (a.rfind("--queries-dir=", 0) == 0)
            g_duckdb_queries_dir = std::string(a.substr(14));
        else
            bargs.push_back(argv[i]);
    }
    int bargs_count = static_cast<int>(bargs.size());
    static RSSMemoryManager g_rss_mgr;
    benchmark::RegisterMemoryManager(&g_rss_mgr);
    benchmark::Initialize(&bargs_count, bargs.data());
    if (benchmark::ReportUnrecognizedArguments(bargs_count, bargs.data())) return 1;
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
#endif
