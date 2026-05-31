
#include <benchmark/benchmark.h>
#include "ClangRuntimeSpecializerBenchmark.h"
#include <string>
#include <string_view>
#include <vector>
#include "ClangRuntimeSpecializer.h"
#include "sqlite3.h"

namespace CRS = clangRuntimeSpecializer;

// ── SQLite internal types ─────────────────────────────────────────────────────
// Vdbe is SQLite's internal bytecode VM struct. sqlite3_stmt* and Vdbe* are the
// same pointer — the public API casts between them with no indirection.
struct Vdbe;
extern "C" int sqlite3VdbeExec(Vdbe* p);


// ── Paths ─────────────────────────────────────────────────────────────────────

static std::string g_db_path = "tpch/data/tpch.db";
static std::string g_sqlite_queries_dir = "tpch/queries/sqlite";

#ifdef ALL_BENCHMARKS_BUILD
void tpch_set_db_path(const char* path) { g_db_path = path; }
void sqlite_tpch_set_queries_dir(const char* path) { g_sqlite_queries_dir = path; }
#endif

// ── Runtime query loading ─────────────────────────────────────────────────────
// Queries are loaded from g_sqlite_queries_dir/q<NN>.sql on first access.
// The SQL string is intentionally NOT a compile-time constant so the JIT sees
// the actual pointer value rather than a folded string literal.

static const std::string& getQuery(int q_num) {
    static std::string queries[22];
    int idx = q_num - 1;
    if (queries[idx].empty()) {
        char path[512];
        snprintf(path, sizeof(path), "%s/q%02d.sql", g_sqlite_queries_dir.c_str(), q_num);
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
volatile bool g_tpch_dummy_trigger = false;
extern "C" __attribute__((used)) void tpch_dummy_registration() {
    auto* RS = CRS::ClangRuntimeSpecializer::init();
    if (g_tpch_dummy_trigger)
        RS->callSpecialized<int>(sqlite3VdbeExec, (Vdbe*)nullptr);
}
#else
volatile bool g_tpch_dummy_trigger_tpch = false;
extern "C" __attribute__((used)) void tpch_dummy_registration_tpch() {
    auto* RS = CRS::ClangRuntimeSpecializer::init();
    if (g_tpch_dummy_trigger_tpch)
        RS->callSpecialized<int>(sqlite3VdbeExec, (Vdbe*)nullptr);
}
#endif

// ── Helpers ───────────────────────────────────────────────────────────────────

static sqlite3* openDB() {
    sqlite3* db = nullptr;
    int rc = sqlite3_open(g_db_path.c_str(), &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open TPC-H database '%s': %s\n",
                g_db_path.c_str(), db ? sqlite3_errmsg(db) : "unknown");
        if (db) sqlite3_close(db);
        exit(1);
    }
    sqlite3_exec(db, "PRAGMA cache_size = -65536", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "PRAGMA journal_mode = OFF",  nullptr, nullptr, nullptr);
    sqlite3_exec(db, "PRAGMA synchronous  = OFF",  nullptr, nullptr, nullptr);
    sqlite3_exec(db, "PRAGMA temp_store   = MEMORY", nullptr, nullptr, nullptr);
    return db;
}

static sqlite3_stmt* prepareQuery(sqlite3* db, const char* sql) {
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK || stmt == nullptr) {
        fprintf(stderr, "Failed to prepare query: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }
    return stmt;
}

// Phase 1: Unspecialized — normal sqlite3_step() execution.
static void phaseUnspecialized(benchmark::State& state, const char* sql) {
    sqlite3* db = openDB();
    sqlite3_stmt* stmt = prepareQuery(db, sql);
    for (auto _ : state) {
        sqlite3_reset(stmt);
        while (sqlite3_step(stmt) == SQLITE_ROW) {}
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// Phase 2: JIT overhead — measures specialization (JIT compile) cost.
static void phaseJITOverhead(benchmark::State& state, const char* sql) {
    sqlite3* db = openDB();
    sqlite3_stmt* stmt = prepareQuery(db, sql);
    auto* vdbe = reinterpret_cast<Vdbe*>(stmt);

    auto* RS = CRS::ClangRuntimeSpecializer::init();
    auto modStats = CRS::ClangRuntimeSpecializer::getModuleStats();
    auto Prev = CRS::ClangRuntimeSpecializer::getLogLevel();
    CRS::ClangRuntimeSpecializer::setLogLevel(CRS::ClangRuntimeSpecializer::LogLevel::None);
    for (auto _ : state)
        benchmark::DoNotOptimize(RS->specializeOnly<int>(sqlite3VdbeExec, vdbe));
    CRS::ClangRuntimeSpecializer::setLogLevel(Prev);
    auto txStats = CRS::ClangRuntimeSpecializer::getLastTransformStats();

    state.counters["jit_module_fns"]    = (double)modStats.FunctionCount;
    state.counters["jit_module_instrs"] = (double)modStats.InstructionCount;
    state.counters["jit_blob_kb"]       = (double)(modStats.BitcodeSizeBytes / 1024);
    state.counters["jit_pruned_fns"]    = (double)txStats.FunctionCountAfterPrune;
    state.counters["jit_pruned_instrs"] = (double)txStats.InstructionCountAfterPrune;

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// Phase 3: Specialized exec — compile once outside loop, measure execution.
static void phaseSpecializedExec(benchmark::State& state, const char* sql) {
    sqlite3* db = openDB();
    sqlite3_stmt* stmt = prepareQuery(db, sql);
    auto* vdbe = reinterpret_cast<Vdbe*>(stmt);

    auto* RS = CRS::ClangRuntimeSpecializer::init();
    auto Prev = CRS::ClangRuntimeSpecializer::getLogLevel();
    CRS::ClangRuntimeSpecializer::setLogLevel(CRS::ClangRuntimeSpecializer::LogLevel::None);
    auto SpecFn = RS->specializeOnly<int>(sqlite3VdbeExec, vdbe);
    CRS::ClangRuntimeSpecializer::setLogLevel(Prev);

    for (auto _ : state) {
        sqlite3_reset(stmt);
        while (SpecFn() == SQLITE_ROW) {}
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// Phase 4: JIT analysis — single-invocation pass-trace capture.
static void phaseJITAnalysis(benchmark::State& state, const char* sql) {
    sqlite3* db = openDB();
    sqlite3_stmt* stmt = prepareQuery(db, sql);
    auto* vdbe = reinterpret_cast<Vdbe*>(stmt);
    clangRuntimeSpecializer::benchmarkJITAnalysis(
        state, sqlite3VdbeExec, std::make_tuple(vdbe));
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// ── Q1: Pricing Summary Report ────────────────────────────────────────────────

void BM_unspecialized_tpch_q1(benchmark::State& state) { phaseUnspecialized(state, getQuery(1).c_str()); }
BENCHMARK(BM_unspecialized_tpch_q1)->Name("BM_g:db/sqlite3/tpch;n:tpch_q1;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q1(benchmark::State& state) { phaseJITOverhead(state, getQuery(1).c_str()); }
BENCHMARK(BM_jit_overhead_tpch_q1)->Name("BM_g:db/sqlite3/tpch;n:tpch_q1;t:jit_overhead;")->Iterations(1)->UseManualTime();

void BM_specialized_exec_tpch_q1(benchmark::State& state) { phaseSpecializedExec(state, getQuery(1).c_str()); }
BENCHMARK(BM_specialized_exec_tpch_q1)->Name("BM_g:db/sqlite3/tpch;n:tpch_q1;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q1(benchmark::State& state) { phaseJITAnalysis(state, getQuery(1).c_str()); }
BENCHMARK(BM_jit_analysis_tpch_q1)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q1;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q2: Minimum Cost Supplier ─────────────────────────────────────────────────

void BM_unspecialized_tpch_q2(benchmark::State& state) { phaseUnspecialized(state, getQuery(2).c_str()); }
BENCHMARK(BM_unspecialized_tpch_q2)->Name("BM_g:db/sqlite3/tpch;n:tpch_q2;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q2(benchmark::State& state) { phaseJITOverhead(state, getQuery(2).c_str()); }
BENCHMARK(BM_jit_overhead_tpch_q2)->Name("BM_g:db/sqlite3/tpch;n:tpch_q2;t:jit_overhead;")->Iterations(1)->UseManualTime();

void BM_specialized_exec_tpch_q2(benchmark::State& state) { phaseSpecializedExec(state, getQuery(2).c_str()); }
BENCHMARK(BM_specialized_exec_tpch_q2)->Name("BM_g:db/sqlite3/tpch;n:tpch_q2;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q2(benchmark::State& state) { phaseJITAnalysis(state, getQuery(2).c_str()); }
BENCHMARK(BM_jit_analysis_tpch_q2)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q2;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q3: Shipping Priority ─────────────────────────────────────────────────────

void BM_unspecialized_tpch_q3(benchmark::State& state) { phaseUnspecialized(state, getQuery(3).c_str()); }
BENCHMARK(BM_unspecialized_tpch_q3)->Name("BM_g:db/sqlite3/tpch;n:tpch_q3;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q3(benchmark::State& state) { phaseJITOverhead(state, getQuery(3).c_str()); }
BENCHMARK(BM_jit_overhead_tpch_q3)->Name("BM_g:db/sqlite3/tpch;n:tpch_q3;t:jit_overhead;")->Iterations(1)->UseManualTime();

void BM_specialized_exec_tpch_q3(benchmark::State& state) { phaseSpecializedExec(state, getQuery(3).c_str()); }
BENCHMARK(BM_specialized_exec_tpch_q3)->Name("BM_g:db/sqlite3/tpch;n:tpch_q3;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q3(benchmark::State& state) { phaseJITAnalysis(state, getQuery(3).c_str()); }
BENCHMARK(BM_jit_analysis_tpch_q3)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q3;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q4: Order Priority Checking ───────────────────────────────────────────────

void BM_unspecialized_tpch_q4(benchmark::State& state) { phaseUnspecialized(state, getQuery(4).c_str()); }
BENCHMARK(BM_unspecialized_tpch_q4)->Name("BM_g:db/sqlite3/tpch;n:tpch_q4;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q4(benchmark::State& state) { phaseJITOverhead(state, getQuery(4).c_str()); }
BENCHMARK(BM_jit_overhead_tpch_q4)->Name("BM_g:db/sqlite3/tpch;n:tpch_q4;t:jit_overhead;")->Iterations(1)->UseManualTime();

void BM_specialized_exec_tpch_q4(benchmark::State& state) { phaseSpecializedExec(state, getQuery(4).c_str()); }
BENCHMARK(BM_specialized_exec_tpch_q4)->Name("BM_g:db/sqlite3/tpch;n:tpch_q4;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q4(benchmark::State& state) { phaseJITAnalysis(state, getQuery(4).c_str()); }
BENCHMARK(BM_jit_analysis_tpch_q4)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q4;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q5: Local Supplier Volume ─────────────────────────────────────────────────

void BM_unspecialized_tpch_q5(benchmark::State& state) { phaseUnspecialized(state, getQuery(5).c_str()); }
BENCHMARK(BM_unspecialized_tpch_q5)->Name("BM_g:db/sqlite3/tpch;n:tpch_q5;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q5(benchmark::State& state) { phaseJITOverhead(state, getQuery(5).c_str()); }
BENCHMARK(BM_jit_overhead_tpch_q5)->Name("BM_g:db/sqlite3/tpch;n:tpch_q5;t:jit_overhead;")->Iterations(1)->UseManualTime();

void BM_specialized_exec_tpch_q5(benchmark::State& state) { phaseSpecializedExec(state, getQuery(5).c_str()); }
BENCHMARK(BM_specialized_exec_tpch_q5)->Name("BM_g:db/sqlite3/tpch;n:tpch_q5;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q5(benchmark::State& state) { phaseJITAnalysis(state, getQuery(5).c_str()); }
BENCHMARK(BM_jit_analysis_tpch_q5)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q5;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q6: Forecasting Revenue Change ───────────────────────────────────────────

void BM_unspecialized_tpch_q6(benchmark::State& state) { phaseUnspecialized(state, getQuery(6).c_str()); }
BENCHMARK(BM_unspecialized_tpch_q6)->Name("BM_g:db/sqlite3/tpch;n:tpch_q6;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q6(benchmark::State& state) { phaseJITOverhead(state, getQuery(6).c_str()); }
BENCHMARK(BM_jit_overhead_tpch_q6)->Name("BM_g:db/sqlite3/tpch;n:tpch_q6;t:jit_overhead;")->Iterations(1)->UseManualTime();

void BM_specialized_exec_tpch_q6(benchmark::State& state) { phaseSpecializedExec(state, getQuery(6).c_str()); }
BENCHMARK(BM_specialized_exec_tpch_q6)->Name("BM_g:db/sqlite3/tpch;n:tpch_q6;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q6(benchmark::State& state) { phaseJITAnalysis(state, getQuery(6).c_str()); }
BENCHMARK(BM_jit_analysis_tpch_q6)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q6;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q7: Volume Shipping ───────────────────────────────────────────────────────

void BM_unspecialized_tpch_q7(benchmark::State& state) { phaseUnspecialized(state, getQuery(7).c_str()); }
BENCHMARK(BM_unspecialized_tpch_q7)->Name("BM_g:db/sqlite3/tpch;n:tpch_q7;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q7(benchmark::State& state) { phaseJITOverhead(state, getQuery(7).c_str()); }
BENCHMARK(BM_jit_overhead_tpch_q7)->Name("BM_g:db/sqlite3/tpch;n:tpch_q7;t:jit_overhead;")->Iterations(1)->UseManualTime();

void BM_specialized_exec_tpch_q7(benchmark::State& state) { phaseSpecializedExec(state, getQuery(7).c_str()); }
BENCHMARK(BM_specialized_exec_tpch_q7)->Name("BM_g:db/sqlite3/tpch;n:tpch_q7;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q7(benchmark::State& state) { phaseJITAnalysis(state, getQuery(7).c_str()); }
BENCHMARK(BM_jit_analysis_tpch_q7)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q7;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q8: National Market Share ─────────────────────────────────────────────────

void BM_unspecialized_tpch_q8(benchmark::State& state) { phaseUnspecialized(state, getQuery(8).c_str()); }
BENCHMARK(BM_unspecialized_tpch_q8)->Name("BM_g:db/sqlite3/tpch;n:tpch_q8;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q8(benchmark::State& state) { phaseJITOverhead(state, getQuery(8).c_str()); }
BENCHMARK(BM_jit_overhead_tpch_q8)->Name("BM_g:db/sqlite3/tpch;n:tpch_q8;t:jit_overhead;")->Iterations(1)->UseManualTime();

void BM_specialized_exec_tpch_q8(benchmark::State& state) { phaseSpecializedExec(state, getQuery(8).c_str()); }
BENCHMARK(BM_specialized_exec_tpch_q8)->Name("BM_g:db/sqlite3/tpch;n:tpch_q8;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q8(benchmark::State& state) { phaseJITAnalysis(state, getQuery(8).c_str()); }
BENCHMARK(BM_jit_analysis_tpch_q8)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q8;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q9: Product Type Profit Measure ──────────────────────────────────────────

void BM_unspecialized_tpch_q9(benchmark::State& state) { phaseUnspecialized(state, getQuery(9).c_str()); }
BENCHMARK(BM_unspecialized_tpch_q9)->Name("BM_g:db/sqlite3/tpch;n:tpch_q9;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q9(benchmark::State& state) { phaseJITOverhead(state, getQuery(9).c_str()); }
BENCHMARK(BM_jit_overhead_tpch_q9)->Name("BM_g:db/sqlite3/tpch;n:tpch_q9;t:jit_overhead;")->Iterations(1)->UseManualTime();

void BM_specialized_exec_tpch_q9(benchmark::State& state) { phaseSpecializedExec(state, getQuery(9).c_str()); }
BENCHMARK(BM_specialized_exec_tpch_q9)->Name("BM_g:db/sqlite3/tpch;n:tpch_q9;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q9(benchmark::State& state) { phaseJITAnalysis(state, getQuery(9).c_str()); }
BENCHMARK(BM_jit_analysis_tpch_q9)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q9;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q10: Returned Item Reporting ──────────────────────────────────────────────

void BM_unspecialized_tpch_q10(benchmark::State& state) { phaseUnspecialized(state, getQuery(10).c_str()); }
BENCHMARK(BM_unspecialized_tpch_q10)->Name("BM_g:db/sqlite3/tpch;n:tpch_q10;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q10(benchmark::State& state) { phaseJITOverhead(state, getQuery(10).c_str()); }
BENCHMARK(BM_jit_overhead_tpch_q10)->Name("BM_g:db/sqlite3/tpch;n:tpch_q10;t:jit_overhead;")->Iterations(1)->UseManualTime();

void BM_specialized_exec_tpch_q10(benchmark::State& state) { phaseSpecializedExec(state, getQuery(10).c_str()); }
BENCHMARK(BM_specialized_exec_tpch_q10)->Name("BM_g:db/sqlite3/tpch;n:tpch_q10;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q10(benchmark::State& state) { phaseJITAnalysis(state, getQuery(10).c_str()); }
BENCHMARK(BM_jit_analysis_tpch_q10)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q10;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q11: Important Stock Identification ───────────────────────────────────────

void BM_unspecialized_tpch_q11(benchmark::State& state) { phaseUnspecialized(state, getQuery(11).c_str()); }
BENCHMARK(BM_unspecialized_tpch_q11)->Name("BM_g:db/sqlite3/tpch;n:tpch_q11;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q11(benchmark::State& state) { phaseJITOverhead(state, getQuery(11).c_str()); }
BENCHMARK(BM_jit_overhead_tpch_q11)->Name("BM_g:db/sqlite3/tpch;n:tpch_q11;t:jit_overhead;")->Iterations(1)->UseManualTime();

void BM_specialized_exec_tpch_q11(benchmark::State& state) { phaseSpecializedExec(state, getQuery(11).c_str()); }
BENCHMARK(BM_specialized_exec_tpch_q11)->Name("BM_g:db/sqlite3/tpch;n:tpch_q11;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q11(benchmark::State& state) { phaseJITAnalysis(state, getQuery(11).c_str()); }
BENCHMARK(BM_jit_analysis_tpch_q11)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q11;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q12: Shipping Modes and Order Priority ────────────────────────────────────

void BM_unspecialized_tpch_q12(benchmark::State& state) { phaseUnspecialized(state, getQuery(12).c_str()); }
BENCHMARK(BM_unspecialized_tpch_q12)->Name("BM_g:db/sqlite3/tpch;n:tpch_q12;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q12(benchmark::State& state) { phaseJITOverhead(state, getQuery(12).c_str()); }
BENCHMARK(BM_jit_overhead_tpch_q12)->Name("BM_g:db/sqlite3/tpch;n:tpch_q12;t:jit_overhead;")->Iterations(1)->UseManualTime();

void BM_specialized_exec_tpch_q12(benchmark::State& state) { phaseSpecializedExec(state, getQuery(12).c_str()); }
BENCHMARK(BM_specialized_exec_tpch_q12)->Name("BM_g:db/sqlite3/tpch;n:tpch_q12;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q12(benchmark::State& state) { phaseJITAnalysis(state, getQuery(12).c_str()); }
BENCHMARK(BM_jit_analysis_tpch_q12)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q12;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q13: Customer Distribution ────────────────────────────────────────────────

void BM_unspecialized_tpch_q13(benchmark::State& state) { phaseUnspecialized(state, getQuery(13).c_str()); }
BENCHMARK(BM_unspecialized_tpch_q13)->Name("BM_g:db/sqlite3/tpch;n:tpch_q13;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q13(benchmark::State& state) { phaseJITOverhead(state, getQuery(13).c_str()); }
BENCHMARK(BM_jit_overhead_tpch_q13)->Name("BM_g:db/sqlite3/tpch;n:tpch_q13;t:jit_overhead;")->Iterations(1)->UseManualTime();

void BM_specialized_exec_tpch_q13(benchmark::State& state) { phaseSpecializedExec(state, getQuery(13).c_str()); }
BENCHMARK(BM_specialized_exec_tpch_q13)->Name("BM_g:db/sqlite3/tpch;n:tpch_q13;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q13(benchmark::State& state) { phaseJITAnalysis(state, getQuery(13).c_str()); }
BENCHMARK(BM_jit_analysis_tpch_q13)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q13;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q14: Promotion Effect ─────────────────────────────────────────────────────

void BM_unspecialized_tpch_q14(benchmark::State& state) { phaseUnspecialized(state, getQuery(14).c_str()); }
BENCHMARK(BM_unspecialized_tpch_q14)->Name("BM_g:db/sqlite3/tpch;n:tpch_q14;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q14(benchmark::State& state) { phaseJITOverhead(state, getQuery(14).c_str()); }
BENCHMARK(BM_jit_overhead_tpch_q14)->Name("BM_g:db/sqlite3/tpch;n:tpch_q14;t:jit_overhead;")->Iterations(1)->UseManualTime();

void BM_specialized_exec_tpch_q14(benchmark::State& state) { phaseSpecializedExec(state, getQuery(14).c_str()); }
BENCHMARK(BM_specialized_exec_tpch_q14)->Name("BM_g:db/sqlite3/tpch;n:tpch_q14;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q14(benchmark::State& state) { phaseJITAnalysis(state, getQuery(14).c_str()); }
BENCHMARK(BM_jit_analysis_tpch_q14)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q14;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q15: Top Supplier ─────────────────────────────────────────────────────────

void BM_unspecialized_tpch_q15(benchmark::State& state) { phaseUnspecialized(state, getQuery(15).c_str()); }
BENCHMARK(BM_unspecialized_tpch_q15)->Name("BM_g:db/sqlite3/tpch;n:tpch_q15;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q15(benchmark::State& state) { phaseJITOverhead(state, getQuery(15).c_str()); }
BENCHMARK(BM_jit_overhead_tpch_q15)->Name("BM_g:db/sqlite3/tpch;n:tpch_q15;t:jit_overhead;")->Iterations(1)->UseManualTime();

void BM_specialized_exec_tpch_q15(benchmark::State& state) { phaseSpecializedExec(state, getQuery(15).c_str()); }
BENCHMARK(BM_specialized_exec_tpch_q15)->Name("BM_g:db/sqlite3/tpch;n:tpch_q15;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q15(benchmark::State& state) { phaseJITAnalysis(state, getQuery(15).c_str()); }
BENCHMARK(BM_jit_analysis_tpch_q15)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q15;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q16: Parts/Supplier Relationship ─────────────────────────────────────────

void BM_unspecialized_tpch_q16(benchmark::State& state) { phaseUnspecialized(state, getQuery(16).c_str()); }
BENCHMARK(BM_unspecialized_tpch_q16)->Name("BM_g:db/sqlite3/tpch;n:tpch_q16;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q16(benchmark::State& state) { phaseJITOverhead(state, getQuery(16).c_str()); }
BENCHMARK(BM_jit_overhead_tpch_q16)->Name("BM_g:db/sqlite3/tpch;n:tpch_q16;t:jit_overhead;")->Iterations(1)->UseManualTime();

void BM_specialized_exec_tpch_q16(benchmark::State& state) { phaseSpecializedExec(state, getQuery(16).c_str()); }
BENCHMARK(BM_specialized_exec_tpch_q16)->Name("BM_g:db/sqlite3/tpch;n:tpch_q16;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q16(benchmark::State& state) { phaseJITAnalysis(state, getQuery(16).c_str()); }
BENCHMARK(BM_jit_analysis_tpch_q16)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q16;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q17: Small-Quantity Order Revenue ────────────────────────────────────────

void BM_unspecialized_tpch_q17(benchmark::State& state) { phaseUnspecialized(state, getQuery(17).c_str()); }
BENCHMARK(BM_unspecialized_tpch_q17)->Name("BM_g:db/sqlite3/tpch;n:tpch_q17;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q17(benchmark::State& state) { phaseJITOverhead(state, getQuery(17).c_str()); }
BENCHMARK(BM_jit_overhead_tpch_q17)->Name("BM_g:db/sqlite3/tpch;n:tpch_q17;t:jit_overhead;")->Iterations(1)->UseManualTime();

void BM_specialized_exec_tpch_q17(benchmark::State& state) { phaseSpecializedExec(state, getQuery(17).c_str()); }
BENCHMARK(BM_specialized_exec_tpch_q17)->Name("BM_g:db/sqlite3/tpch;n:tpch_q17;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q17(benchmark::State& state) { phaseJITAnalysis(state, getQuery(17).c_str()); }
BENCHMARK(BM_jit_analysis_tpch_q17)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q17;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q18: Large Volume Customer ────────────────────────────────────────────────

void BM_unspecialized_tpch_q18(benchmark::State& state) { phaseUnspecialized(state, getQuery(18).c_str()); }
BENCHMARK(BM_unspecialized_tpch_q18)->Name("BM_g:db/sqlite3/tpch;n:tpch_q18;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q18(benchmark::State& state) { phaseJITOverhead(state, getQuery(18).c_str()); }
BENCHMARK(BM_jit_overhead_tpch_q18)->Name("BM_g:db/sqlite3/tpch;n:tpch_q18;t:jit_overhead;")->Iterations(1)->UseManualTime();

void BM_specialized_exec_tpch_q18(benchmark::State& state) { phaseSpecializedExec(state, getQuery(18).c_str()); }
BENCHMARK(BM_specialized_exec_tpch_q18)->Name("BM_g:db/sqlite3/tpch;n:tpch_q18;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q18(benchmark::State& state) { phaseJITAnalysis(state, getQuery(18).c_str()); }
BENCHMARK(BM_jit_analysis_tpch_q18)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q18;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q19: Discounted Revenue ───────────────────────────────────────────────────

void BM_unspecialized_tpch_q19(benchmark::State& state) { phaseUnspecialized(state, getQuery(19).c_str()); }
BENCHMARK(BM_unspecialized_tpch_q19)->Name("BM_g:db/sqlite3/tpch;n:tpch_q19;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q19(benchmark::State& state) { phaseJITOverhead(state, getQuery(19).c_str()); }
BENCHMARK(BM_jit_overhead_tpch_q19)->Name("BM_g:db/sqlite3/tpch;n:tpch_q19;t:jit_overhead;")->Iterations(1)->UseManualTime();

void BM_specialized_exec_tpch_q19(benchmark::State& state) { phaseSpecializedExec(state, getQuery(19).c_str()); }
BENCHMARK(BM_specialized_exec_tpch_q19)->Name("BM_g:db/sqlite3/tpch;n:tpch_q19;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q19(benchmark::State& state) { phaseJITAnalysis(state, getQuery(19).c_str()); }
BENCHMARK(BM_jit_analysis_tpch_q19)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q19;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q20: Potential Part Promotion ─────────────────────────────────────────────

void BM_unspecialized_tpch_q20(benchmark::State& state) { phaseUnspecialized(state, getQuery(20).c_str()); }
BENCHMARK(BM_unspecialized_tpch_q20)->Name("BM_g:db/sqlite3/tpch;n:tpch_q20;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q20(benchmark::State& state) { phaseJITOverhead(state, getQuery(20).c_str()); }
BENCHMARK(BM_jit_overhead_tpch_q20)->Name("BM_g:db/sqlite3/tpch;n:tpch_q20;t:jit_overhead;")->Iterations(1)->UseManualTime();

void BM_specialized_exec_tpch_q20(benchmark::State& state) { phaseSpecializedExec(state, getQuery(20).c_str()); }
BENCHMARK(BM_specialized_exec_tpch_q20)->Name("BM_g:db/sqlite3/tpch;n:tpch_q20;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q20(benchmark::State& state) { phaseJITAnalysis(state, getQuery(20).c_str()); }
BENCHMARK(BM_jit_analysis_tpch_q20)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q20;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q21: Suppliers Who Kept Orders Waiting ────────────────────────────────────

void BM_unspecialized_tpch_q21(benchmark::State& state) { phaseUnspecialized(state, getQuery(21).c_str()); }
BENCHMARK(BM_unspecialized_tpch_q21)->Name("BM_g:db/sqlite3/tpch;n:tpch_q21;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q21(benchmark::State& state) { phaseJITOverhead(state, getQuery(21).c_str()); }
BENCHMARK(BM_jit_overhead_tpch_q21)->Name("BM_g:db/sqlite3/tpch;n:tpch_q21;t:jit_overhead;")->Iterations(1)->UseManualTime();

void BM_specialized_exec_tpch_q21(benchmark::State& state) { phaseSpecializedExec(state, getQuery(21).c_str()); }
BENCHMARK(BM_specialized_exec_tpch_q21)->Name("BM_g:db/sqlite3/tpch;n:tpch_q21;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q21(benchmark::State& state) { phaseJITAnalysis(state, getQuery(21).c_str()); }
BENCHMARK(BM_jit_analysis_tpch_q21)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q21;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q22: Global Sales Opportunity ────────────────────────────────────────────

void BM_unspecialized_tpch_q22(benchmark::State& state) { phaseUnspecialized(state, getQuery(22).c_str()); }
BENCHMARK(BM_unspecialized_tpch_q22)->Name("BM_g:db/sqlite3/tpch;n:tpch_q22;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q22(benchmark::State& state) { phaseJITOverhead(state, getQuery(22).c_str()); }
BENCHMARK(BM_jit_overhead_tpch_q22)->Name("BM_g:db/sqlite3/tpch;n:tpch_q22;t:jit_overhead;")->Iterations(1)->UseManualTime();

void BM_specialized_exec_tpch_q22(benchmark::State& state) { phaseSpecializedExec(state, getQuery(22).c_str()); }
BENCHMARK(BM_specialized_exec_tpch_q22)->Name("BM_g:db/sqlite3/tpch;n:tpch_q22;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q22(benchmark::State& state) { phaseJITAnalysis(state, getQuery(22).c_str()); }
BENCHMARK(BM_jit_analysis_tpch_q22)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q22;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Entry point ───────────────────────────────────────────────────────────────

#ifndef ALL_BENCHMARKS_BUILD
int main(int argc, char** argv) {
    std::vector<char*> bargs;
    bargs.push_back(argv[0]);
    for (int i = 1; i < argc; ++i) {
        std::string_view a(argv[i]);
        if (a.rfind("--db=", 0) == 0)
            g_db_path = std::string(a.substr(5));
        else if (a.rfind("--queries-dir=", 0) == 0)
            g_sqlite_queries_dir = std::string(a.substr(14));
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
