
#include <benchmark/benchmark.h>
#include "ClangRuntimeSpecializerBenchmark.h"
#include <chrono>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "ClangRuntimeSpecializer.h"
#include "sqlite3.h"

namespace CRS = clangRuntimeSpecializer;

// ── SQLite internal types ─────────────────────────────────────────────────────
// Vdbe is SQLite's internal bytecode VM struct. sqlite3_stmt* and Vdbe* are the
// same pointer — the public API casts between them with no indirection.
struct Vdbe;
extern "C" int sqlite3VdbeExec(Vdbe* p);

// Accessors into Vdbe internals defined in sqlite3_with_accessor.c (same TU as
// sqlite3.c, so the full Vdbe struct definition is available there).
extern "C" int  crs_vdbe_nop(sqlite3_stmt* stmt);
extern "C" int  crs_vdbe_op_size(void);
// sqlite3_reset() leaves Vdbe::pc=-1; sqlite3Step() sets it to 0 before
// calling sqlite3VdbeExec. Set it explicitly when calling VdbeExec directly.
extern "C" void crs_vdbe_set_pc(sqlite3_stmt* stmt, int pc);


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

// Named queries (e.g. "simple_scan") from the sqlite queries directory.
static const std::string& getNamedQuery(const char* name) {
    static std::unordered_map<std::string, std::string> cache;
    auto& s = cache[name];
    if (s.empty()) {
        char path[512];
        snprintf(path, sizeof(path), "%s/%s.sql", g_sqlite_queries_dir.c_str(), name);
        FILE* f = fopen(path, "r");
        if (!f) {
            fprintf(stderr, "Cannot open query file: %s\n", path);
            exit(1);
        }
        fseek(f, 0, SEEK_END);
        long sz = ftell(f);
        rewind(f);
        s.resize((size_t)sz);
        if (sz > 0)
            (void)fread(&s[0], 1, (size_t)sz, f);
        fclose(f);
    }
    return s;
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

// Tracks whether the most recent jit_overhead run timed out, so phaseSpecializedExec
// can skip execution (same pattern as duckdb_tpch_bench.cpp).
static bool g_last_jit_timed_out = false;

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
// Uses Iterations(1)/UseManualTime(). Reports query_nops and query_bytecode_bytes
// counters. Times out after 300 s and sets jit_timeout=1.
static void phaseJITOverhead(benchmark::State& state, const char* sql) {
    g_last_jit_timed_out = false;

    sqlite3* db = openDB();
    sqlite3_stmt* stmt = prepareQuery(db, sql);
    auto* vdbe = reinterpret_cast<Vdbe*>(stmt);

    int nops    = crs_vdbe_nop(stmt);
    int op_size = crs_vdbe_op_size();

    auto* RS = CRS::ClangRuntimeSpecializer::init();
    auto modStats = CRS::ClangRuntimeSpecializer::getModuleStats();
    auto Prev = CRS::ClangRuntimeSpecializer::getLogLevel();
    CRS::ClangRuntimeSpecializer::setLogLevel(CRS::ClangRuntimeSpecializer::LogLevel::None);

    auto opts = CRS::ClangRuntimeSpecializer::Options::Default().withJITTimeoutSeconds(300);
    CRS::SpecializedFunction<int> specFn;
    for (auto _ : state) {
        auto t0 = std::chrono::steady_clock::now();
        specFn = RS->specializeOnly<int>(sqlite3VdbeExec, opts, vdbe);
        auto elapsed = std::chrono::steady_clock::now() - t0;
        double elapsed_s = std::chrono::duration<double>(elapsed).count();
        state.SetIterationTime(elapsed_s);
    }
    CRS::ClangRuntimeSpecializer::setLogLevel(Prev);

    g_last_jit_timed_out = !specFn;
    auto txStats = CRS::ClangRuntimeSpecializer::getLastTransformStats();

    state.counters["jit_module_fns"]       = (double)modStats.FunctionCount;
    state.counters["jit_module_instrs"]    = (double)modStats.InstructionCount;
    state.counters["jit_blob_kb"]          = (double)(modStats.BitcodeSizeBytes / 1024);
    state.counters["jit_pruned_fns"]       = (double)txStats.FunctionCountAfterPrune;
    state.counters["jit_pruned_instrs"]    = (double)txStats.InstructionCountAfterPrune;
    state.counters["jit_timeout"]          = g_last_jit_timed_out ? 1.0 : 0.0;
    state.counters["query_nops"]           = (double)nops;
    state.counters["query_bytecode_bytes"] = (double)(nops * op_size);

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// Phase 3: Specialized exec — compile once outside loop, measure execution.
static void phaseSpecializedExec(benchmark::State& state, const char* sql) {
    if (g_last_jit_timed_out) { state.SkipWithMessage("jit_timed_out"); return; }

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
        crs_vdbe_set_pc(stmt, 0);  // sqlite3_reset leaves pc=-1; VdbeExec starts at aOp[pc]
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

// ── TPC-H Q1–Q22 ─────────────────────────────────────────────────────────────

#define DEFINE_TPCH_BM(N)                                                           \
void BM_unspecialized_tpch_q##N(benchmark::State& s)  { phaseUnspecialized(s, getQuery(N).c_str()); }  \
BENCHMARK(BM_unspecialized_tpch_q##N)->Name("BM_g:db/sqlite3/tpch;n:tpch_q" #N ";t:unspecialized;")->MinTime(1.0); \
void BM_jit_overhead_tpch_q##N(benchmark::State& s)   { phaseJITOverhead(s, getQuery(N).c_str()); }   \
BENCHMARK(BM_jit_overhead_tpch_q##N)->Name("BM_g:db/sqlite3/tpch;n:tpch_q" #N ";t:jit_overhead;")->Iterations(1)->UseManualTime(); \
void BM_specialized_exec_tpch_q##N(benchmark::State& s){ phaseSpecializedExec(s, getQuery(N).c_str()); } \
BENCHMARK(BM_specialized_exec_tpch_q##N)->Name("BM_g:db/sqlite3/tpch;n:tpch_q" #N ";t:specialized_exec;")->MinTime(1.0); \
void BM_jit_analysis_tpch_q##N(benchmark::State& s)   { phaseJITAnalysis(s, getQuery(N).c_str()); }   \
BENCHMARK(BM_jit_analysis_tpch_q##N)->Name("BM_g:db/sqlite3/tpch;n:tpch_q" #N ";t:jit_analysis;")->Iterations(1)->UseManualTime();

DEFINE_TPCH_BM(1)
DEFINE_TPCH_BM(2)
DEFINE_TPCH_BM(3)
DEFINE_TPCH_BM(4)
DEFINE_TPCH_BM(5)
DEFINE_TPCH_BM(6)
DEFINE_TPCH_BM(7)
DEFINE_TPCH_BM(8)
DEFINE_TPCH_BM(9)
DEFINE_TPCH_BM(10)
DEFINE_TPCH_BM(11)
DEFINE_TPCH_BM(12)
DEFINE_TPCH_BM(13)
DEFINE_TPCH_BM(14)
DEFINE_TPCH_BM(15)
DEFINE_TPCH_BM(16)
DEFINE_TPCH_BM(17)
DEFINE_TPCH_BM(18)
DEFINE_TPCH_BM(19)
DEFINE_TPCH_BM(20)
DEFINE_TPCH_BM(21)
DEFINE_TPCH_BM(22)

// ── Synthetic input-size benchmarks (spec 018) ───────────────────────────────
// Group: db/sqlite3/input_size — covers trivial to TPC-H-level complexity.
// Each query has four phases: unspecialized, jit_overhead, specialized_exec,
// jit_analysis.

#define DEFINE_INPUT_SIZE_BM(QUERY_NAME)                                              \
void BM_unspecialized_##QUERY_NAME(benchmark::State& s) {                             \
    phaseUnspecialized(s, getNamedQuery(#QUERY_NAME).c_str());                        \
}                                                                                      \
BENCHMARK(BM_unspecialized_##QUERY_NAME)                                              \
    ->Name("BM_g:db/sqlite3/input_size;n:" #QUERY_NAME ";t:unspecialized;")          \
    ->MinTime(1.0);                                                                    \
void BM_jit_overhead_##QUERY_NAME(benchmark::State& s) {                              \
    phaseJITOverhead(s, getNamedQuery(#QUERY_NAME).c_str());                          \
}                                                                                      \
BENCHMARK(BM_jit_overhead_##QUERY_NAME)                                               \
    ->Name("BM_g:db/sqlite3/input_size;n:" #QUERY_NAME ";t:jit_overhead;")           \
    ->Iterations(1)->UseManualTime();                                                  \
void BM_specialized_exec_##QUERY_NAME(benchmark::State& s) {                          \
    phaseSpecializedExec(s, getNamedQuery(#QUERY_NAME).c_str());                      \
}                                                                                      \
BENCHMARK(BM_specialized_exec_##QUERY_NAME)                                           \
    ->Name("BM_g:db/sqlite3/input_size;n:" #QUERY_NAME ";t:specialized_exec;")       \
    ->MinTime(1.0);                                                                    \
void BM_jit_analysis_##QUERY_NAME(benchmark::State& s) {                              \
    sqlite3* db = openDB();                                                            \
    sqlite3_stmt* stmt = prepareQuery(db, getNamedQuery(#QUERY_NAME).c_str());        \
    auto* vdbe = reinterpret_cast<Vdbe*>(stmt);                                       \
    clangRuntimeSpecializer::benchmarkJITAnalysis(                                    \
        s, sqlite3VdbeExec, std::make_tuple(vdbe));                                   \
    sqlite3_finalize(stmt);                                                            \
    sqlite3_close(db);                                                                 \
}                                                                                      \
BENCHMARK(BM_jit_analysis_##QUERY_NAME)                                               \
    ->Name("BM_g:db/sqlite3/input_size;n:" #QUERY_NAME ";t:jit_analysis;")           \
    ->Iterations(1)->UseManualTime();

DEFINE_INPUT_SIZE_BM(simple_scan)
DEFINE_INPUT_SIZE_BM(filter_small)
DEFINE_INPUT_SIZE_BM(filter_large)
DEFINE_INPUT_SIZE_BM(agg_simple)
DEFINE_INPUT_SIZE_BM(agg_group)
DEFINE_INPUT_SIZE_BM(join_simple)
DEFINE_INPUT_SIZE_BM(join_agg)

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
