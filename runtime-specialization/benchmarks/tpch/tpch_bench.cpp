
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

inline constexpr char Fn_sqlite3VdbeExec[] = "sqlite3VdbeExec";

// ── TPC-H query strings ───────────────────────────────────────────────────────

// Q1: Pricing Summary Report — pure aggregation + group-by on lineitem
static constexpr const char* TPCH_Q1 = R"sql(
SELECT l_returnflag, l_linestatus,
       SUM(l_quantity)                                    AS sum_qty,
       SUM(l_extendedprice)                               AS sum_base_price,
       SUM(l_extendedprice*(1-l_discount))                AS sum_disc_price,
       SUM(l_extendedprice*(1-l_discount)*(1+l_tax))      AS sum_charge,
       AVG(l_quantity)                                    AS avg_qty,
       AVG(l_extendedprice)                               AS avg_price,
       AVG(l_discount)                                    AS avg_disc,
       COUNT(*)                                           AS count_order
FROM lineitem
WHERE l_shipdate <= DATE('1998-12-01', '-90 days')
GROUP BY l_returnflag, l_linestatus
ORDER BY l_returnflag, l_linestatus
)sql";

// Q6: Forecasting Revenue Change — single-table filter + aggregation (no joins)
static constexpr const char* TPCH_Q6 = R"sql(
SELECT SUM(l_extendedprice * l_discount) AS revenue
FROM lineitem
WHERE l_shipdate >= '1994-01-01'
  AND l_shipdate <  '1995-01-01'
  AND l_discount BETWEEN 0.05 AND 0.07
  AND l_quantity < 24
)sql";

// Q3: Shipping Priority — 3-table join + aggregation
static constexpr const char* TPCH_Q3 = R"sql(
SELECT l_orderkey,
       SUM(l_extendedprice*(1-l_discount)) AS revenue,
       o_orderdate,
       o_shippriority
FROM customer, orders, lineitem
WHERE c_mktsegment = 'BUILDING'
  AND c_custkey    = o_custkey
  AND l_orderkey   = o_orderkey
  AND o_orderdate  < '1995-03-15'
  AND l_shipdate   > '1995-03-15'
GROUP BY l_orderkey, o_orderdate, o_shippriority
ORDER BY revenue DESC, o_orderdate
LIMIT 10
)sql";

// ── Database path ─────────────────────────────────────────────────────────────

static std::string g_db_path = "tpch.db";

#ifdef ALL_BENCHMARKS_BUILD
// Called by AllBenchmarks_main.cpp to forward --db=<path> from the command line.
void tpch_set_db_path(const char* path) { g_db_path = path; }
#endif

// ── IR embedding trigger ──────────────────────────────────────────────────────
// The volatile + attribute((used)) guard ensures callSpecialized appears in IR
// so the runtime can find sqlite3VdbeExec in the embedded IR blob from sqlite3.c.

#ifndef ALL_BENCHMARKS_BUILD
volatile bool g_tpch_dummy_trigger = false;
extern "C" __attribute__((used)) void tpch_dummy_registration() {
    auto* RS = CRS::ClangRuntimeSpecializer::init();
    if (g_tpch_dummy_trigger)
        RS->callSpecialized<int>(Fn_sqlite3VdbeExec, (Vdbe*)nullptr);
}
#else
volatile bool g_tpch_dummy_trigger_tpch = false;
extern "C" __attribute__((used)) void tpch_dummy_registration_tpch() {
    auto* RS = CRS::ClangRuntimeSpecializer::init();
    if (g_tpch_dummy_trigger_tpch)
        RS->callSpecialized<int>(Fn_sqlite3VdbeExec, (Vdbe*)nullptr);
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
    // Read-optimized settings for the benchmark
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
// Prepare once, reset between iterations to amortize query compilation.
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
// Each iteration (re-)compiles a specialized sqlite3VdbeExec for this Vdbe ptr.
// The first call compiles; subsequent calls return the cached result.
static void phaseJITOverhead(benchmark::State& state, const char* sql) {
    sqlite3* db = openDB();
    sqlite3_stmt* stmt = prepareQuery(db, sql);
    auto* vdbe = reinterpret_cast<Vdbe*>(stmt);

    auto* RS = CRS::ClangRuntimeSpecializer::init();
    auto modStats = CRS::ClangRuntimeSpecializer::getModuleStats();
    auto Prev = CRS::ClangRuntimeSpecializer::getLogLevel();
    CRS::ClangRuntimeSpecializer::setLogLevel(CRS::ClangRuntimeSpecializer::LogLevel::None);
    for (auto _ : state)
        benchmark::DoNotOptimize(RS->specializeOnly(Fn_sqlite3VdbeExec, vdbe));
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
// Specializing sqlite3VdbeExec for a known Vdbe* makes the entire opcode
// dispatch switch constant-foldable: IPSCCP sees p->aOp[pc].opcode as known
// constants and eliminates the switch, inlining only relevant handlers.
static void phaseSpecializedExec(benchmark::State& state, const char* sql) {
    sqlite3* db = openDB();
    sqlite3_stmt* stmt = prepareQuery(db, sql);
    auto* vdbe = reinterpret_cast<Vdbe*>(stmt);

    auto* RS = CRS::ClangRuntimeSpecializer::init();
    auto Prev = CRS::ClangRuntimeSpecializer::getLogLevel();
    CRS::ClangRuntimeSpecializer::setLogLevel(CRS::ClangRuntimeSpecializer::LogLevel::None);
    uintptr_t Addr = RS->specializeOnly(Fn_sqlite3VdbeExec, vdbe);
    CRS::ClangRuntimeSpecializer::setLogLevel(Prev);
    // Specialized wrapper takes no args (vdbe is baked in as a constant)
    auto SpecFn = reinterpret_cast<int(*)()>(Addr);

    for (auto _ : state) {
        sqlite3_reset(stmt);
        while (SpecFn() == SQLITE_ROW) {}
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// ── Q1: Pricing Summary Report ────────────────────────────────────────────────

void BM_unspecialized_tpch_q1(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q1); }
BENCHMARK(BM_unspecialized_tpch_q1)->Name("BM_g:tpch;n:tpch_q1;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q1(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q1); }
BENCHMARK(BM_jit_overhead_tpch_q1)->Name("BM_g:tpch;n:tpch_q1;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q1(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q1); }
BENCHMARK(BM_specialized_exec_tpch_q1)->Name("BM_g:tpch;n:tpch_q1;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q1(benchmark::State& state) {
    sqlite3* db = openDB();
    sqlite3_stmt* stmt = prepareQuery(db, TPCH_Q1);
    auto* vdbe = reinterpret_cast<Vdbe*>(stmt);
    clangRuntimeSpecializer::benchmarkJITAnalysis<Fn_sqlite3VdbeExec>(
        state, sqlite3VdbeExec, std::make_tuple(vdbe));
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}
BENCHMARK(BM_jit_analysis_tpch_q1)
    ->Name("BM_g:tpch;n:tpch_q1;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q6: Forecasting Revenue Change ───────────────────────────────────────────

void BM_unspecialized_tpch_q6(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q6); }
BENCHMARK(BM_unspecialized_tpch_q6)->Name("BM_g:tpch;n:tpch_q6;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q6(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q6); }
BENCHMARK(BM_jit_overhead_tpch_q6)->Name("BM_g:tpch;n:tpch_q6;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q6(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q6); }
BENCHMARK(BM_specialized_exec_tpch_q6)->Name("BM_g:tpch;n:tpch_q6;t:specialized_exec;")->MinTime(1.0);

// ── Q3: Shipping Priority ─────────────────────────────────────────────────────

void BM_unspecialized_tpch_q3(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q3); }
BENCHMARK(BM_unspecialized_tpch_q3)->Name("BM_g:tpch;n:tpch_q3;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q3(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q3); }
BENCHMARK(BM_jit_overhead_tpch_q3)->Name("BM_g:tpch;n:tpch_q3;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q3(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q3); }
BENCHMARK(BM_specialized_exec_tpch_q3)->Name("BM_g:tpch;n:tpch_q3;t:specialized_exec;")->MinTime(1.0);

// ── Entry point ───────────────────────────────────────────────────────────────

#ifndef ALL_BENCHMARKS_BUILD
int main(int argc, char** argv) {
    // Strip --db=<path> before passing remaining flags to Google Benchmark.
    std::vector<char*> bargs;
    bargs.push_back(argv[0]);
    for (int i = 1; i < argc; ++i) {
        std::string_view a(argv[i]);
        if (a.rfind("--db=", 0) == 0)
            g_db_path = std::string(a.substr(5));
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
