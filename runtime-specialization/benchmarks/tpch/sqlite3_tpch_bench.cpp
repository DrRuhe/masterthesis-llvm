
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

// Q2: Minimum Cost Supplier
static constexpr const char* TPCH_Q2 = R"sql(
SELECT s_acctbal, s_name, n_name, p_partkey, p_mfgr, s_address, s_phone, s_comment
FROM part, supplier, partsupp, nation, region
WHERE p_partkey = ps_partkey
  AND s_suppkey = ps_suppkey
  AND p_size = 15
  AND p_type LIKE '%BRASS'
  AND s_nationkey = n_nationkey
  AND n_regionkey = r_regionkey
  AND r_name = 'EUROPE'
  AND ps_supplycost = (
    SELECT MIN(ps_supplycost)
    FROM partsupp, supplier, nation, region
    WHERE p_partkey = ps_partkey
      AND s_suppkey = ps_suppkey
      AND s_nationkey = n_nationkey
      AND n_regionkey = r_regionkey
      AND r_name = 'EUROPE'
  )
ORDER BY s_acctbal DESC, n_name, s_name, p_partkey
LIMIT 100
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

// Q4: Order Priority Checking
static constexpr const char* TPCH_Q4 = R"sql(
SELECT o_orderpriority, COUNT(*) AS order_count
FROM orders
WHERE o_orderdate >= '1993-07-01'
  AND o_orderdate < '1993-10-01'
  AND EXISTS (
    SELECT * FROM lineitem
    WHERE l_orderkey = o_orderkey
      AND l_commitdate < l_receiptdate
  )
GROUP BY o_orderpriority
ORDER BY o_orderpriority
)sql";

// Q5: Local Supplier Volume
static constexpr const char* TPCH_Q5 = R"sql(
SELECT n_name, SUM(l_extendedprice * (1 - l_discount)) AS revenue
FROM customer, orders, lineitem, supplier, nation, region
WHERE c_custkey = o_custkey
  AND l_orderkey = o_orderkey
  AND l_suppkey = s_suppkey
  AND c_nationkey = s_nationkey
  AND s_nationkey = n_nationkey
  AND n_regionkey = r_regionkey
  AND r_name = 'ASIA'
  AND o_orderdate >= '1994-01-01'
  AND o_orderdate < '1995-01-01'
GROUP BY n_name
ORDER BY revenue DESC
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

// Q7: Volume Shipping
static constexpr const char* TPCH_Q7 = R"sql(
SELECT supp_nation, cust_nation, l_year, SUM(volume) AS revenue
FROM (
  SELECT n1.n_name AS supp_nation,
         n2.n_name AS cust_nation,
         STRFTIME('%Y', l_shipdate) AS l_year,
         l_extendedprice * (1 - l_discount) AS volume
  FROM supplier, lineitem, orders, customer, nation n1, nation n2
  WHERE s_suppkey = l_suppkey
    AND o_orderkey = l_orderkey
    AND c_custkey = o_custkey
    AND s_nationkey = n1.n_nationkey
    AND c_nationkey = n2.n_nationkey
    AND (
      (n1.n_name = 'FRANCE'  AND n2.n_name = 'GERMANY') OR
      (n1.n_name = 'GERMANY' AND n2.n_name = 'FRANCE')
    )
    AND l_shipdate BETWEEN '1995-01-01' AND '1996-12-31'
) AS shipping
GROUP BY supp_nation, cust_nation, l_year
ORDER BY supp_nation, cust_nation, l_year
)sql";

// Q8: National Market Share
static constexpr const char* TPCH_Q8 = R"sql(
SELECT o_year,
       SUM(CASE WHEN nation = 'BRAZIL' THEN volume ELSE 0 END) / SUM(volume) AS mkt_share
FROM (
  SELECT STRFTIME('%Y', o_orderdate) AS o_year,
         l_extendedprice * (1 - l_discount) AS volume,
         n2.n_name AS nation
  FROM part, supplier, lineitem, orders, customer, nation n1, nation n2, region
  WHERE p_partkey = l_partkey
    AND s_suppkey = l_suppkey
    AND l_orderkey = o_orderkey
    AND o_custkey = c_custkey
    AND c_nationkey = n1.n_nationkey
    AND n1.n_regionkey = r_regionkey
    AND r_name = 'AMERICA'
    AND s_nationkey = n2.n_nationkey
    AND o_orderdate BETWEEN '1995-01-01' AND '1996-12-31'
    AND p_type = 'ECONOMY ANODIZED STEEL'
) AS all_nations
GROUP BY o_year
ORDER BY o_year
)sql";

// Q9: Product Type Profit Measure
static constexpr const char* TPCH_Q9 = R"sql(
SELECT nation, o_year, SUM(amount) AS sum_profit
FROM (
  SELECT n_name AS nation,
         STRFTIME('%Y', o_orderdate) AS o_year,
         l_extendedprice * (1 - l_discount) - ps_supplycost * l_quantity AS amount
  FROM part, supplier, lineitem, partsupp, orders, nation
  WHERE s_suppkey = l_suppkey
    AND ps_suppkey = l_suppkey
    AND ps_partkey = l_partkey
    AND p_partkey = l_partkey
    AND o_orderkey = l_orderkey
    AND s_nationkey = n_nationkey
    AND p_name LIKE '%green%'
) AS profit
GROUP BY nation, o_year
ORDER BY nation, o_year DESC
)sql";

// Q10: Returned Item Reporting
static constexpr const char* TPCH_Q10 = R"sql(
SELECT c_custkey, c_name,
       SUM(l_extendedprice * (1 - l_discount)) AS revenue,
       c_acctbal, n_name, c_address, c_phone, c_comment
FROM customer, orders, lineitem, nation
WHERE c_custkey = o_custkey
  AND l_orderkey = o_orderkey
  AND o_orderdate >= '1993-10-01'
  AND o_orderdate < '1994-01-01'
  AND l_returnflag = 'R'
  AND c_nationkey = n_nationkey
GROUP BY c_custkey, c_name, c_acctbal, c_phone, n_name, c_address, c_comment
ORDER BY revenue DESC
LIMIT 20
)sql";

// Q11: Important Stock Identification
static constexpr const char* TPCH_Q11 = R"sql(
SELECT ps_partkey, SUM(ps_supplycost * ps_availqty) AS value
FROM partsupp, supplier, nation
WHERE ps_suppkey = s_suppkey
  AND s_nationkey = n_nationkey
  AND n_name = 'GERMANY'
GROUP BY ps_partkey
HAVING SUM(ps_supplycost * ps_availqty) > (
  SELECT SUM(ps_supplycost * ps_availqty) * 0.0001
  FROM partsupp, supplier, nation
  WHERE ps_suppkey = s_suppkey
    AND s_nationkey = n_nationkey
    AND n_name = 'GERMANY'
)
ORDER BY value DESC
)sql";

// Q12: Shipping Modes and Order Priority
static constexpr const char* TPCH_Q12 = R"sql(
SELECT l_shipmode,
       SUM(CASE WHEN o_orderpriority = '1-URGENT' OR o_orderpriority = '2-HIGH'
                THEN 1 ELSE 0 END) AS high_line_count,
       SUM(CASE WHEN o_orderpriority <> '1-URGENT' AND o_orderpriority <> '2-HIGH'
                THEN 1 ELSE 0 END) AS low_line_count
FROM orders, lineitem
WHERE o_orderkey = l_orderkey
  AND l_shipmode IN ('MAIL', 'SHIP')
  AND l_commitdate < l_receiptdate
  AND l_shipdate < l_commitdate
  AND l_receiptdate >= '1994-01-01'
  AND l_receiptdate < '1995-01-01'
GROUP BY l_shipmode
ORDER BY l_shipmode
)sql";

// Q13: Customer Distribution
static constexpr const char* TPCH_Q13 = R"sql(
SELECT c_count, COUNT(*) AS custdist
FROM (
  SELECT c_custkey, COUNT(o_orderkey) AS c_count
  FROM customer
  LEFT JOIN orders ON c_custkey = o_custkey
    AND o_comment NOT LIKE '%special%requests%'
  GROUP BY c_custkey
) AS c_orders
GROUP BY c_count
ORDER BY custdist DESC, c_count DESC
)sql";

// Q14: Promotion Effect
static constexpr const char* TPCH_Q14 = R"sql(
SELECT 100.00 * SUM(CASE WHEN p_type LIKE 'PROMO%'
                         THEN l_extendedprice * (1 - l_discount)
                         ELSE 0 END)
             / SUM(l_extendedprice * (1 - l_discount)) AS promo_revenue
FROM lineitem, part
WHERE l_partkey = p_partkey
  AND l_shipdate >= '1995-09-01'
  AND l_shipdate < '1995-10-01'
)sql";

// Q15: Top Supplier (view inlined as subquery)
static constexpr const char* TPCH_Q15 = R"sql(
SELECT s_suppkey, s_name, s_address, s_phone, total_revenue
FROM supplier,
     (SELECT l_suppkey AS supplier_no,
             SUM(l_extendedprice * (1 - l_discount)) AS total_revenue
      FROM lineitem
      WHERE l_shipdate >= '1996-01-01'
        AND l_shipdate < '1996-04-01'
      GROUP BY l_suppkey) AS revenue0
WHERE s_suppkey = supplier_no
  AND total_revenue = (
    SELECT MAX(total_revenue)
    FROM (SELECT l_suppkey AS supplier_no,
                 SUM(l_extendedprice * (1 - l_discount)) AS total_revenue
          FROM lineitem
          WHERE l_shipdate >= '1996-01-01'
            AND l_shipdate < '1996-04-01'
          GROUP BY l_suppkey) AS revenue1
  )
ORDER BY s_suppkey
)sql";

// Q16: Parts/Supplier Relationship
static constexpr const char* TPCH_Q16 = R"sql(
SELECT p_brand, p_type, p_size, COUNT(DISTINCT ps_suppkey) AS supplier_cnt
FROM partsupp, part
WHERE p_partkey = ps_partkey
  AND p_brand <> 'Brand#45'
  AND p_type NOT LIKE 'MEDIUM POLISHED%'
  AND p_size IN (49, 14, 23, 45, 19, 3, 36, 9)
  AND ps_suppkey NOT IN (
    SELECT s_suppkey FROM supplier
    WHERE s_comment LIKE '%Customer%Complaints%'
  )
GROUP BY p_brand, p_type, p_size
ORDER BY supplier_cnt DESC, p_brand, p_type, p_size
)sql";

// Q17: Small-Quantity Order Revenue
static constexpr const char* TPCH_Q17 = R"sql(
SELECT SUM(l_extendedprice) / 7.0 AS avg_yearly
FROM lineitem, part
WHERE p_partkey = l_partkey
  AND p_brand = 'Brand#23'
  AND p_container = 'MED BOX'
  AND l_quantity < (
    SELECT 0.2 * AVG(l_quantity)
    FROM lineitem
    WHERE l_partkey = p_partkey
  )
)sql";

// Q18: Large Volume Customer
static constexpr const char* TPCH_Q18 = R"sql(
SELECT c_name, c_custkey, o_orderkey, o_orderdate, o_totalprice,
       SUM(l_quantity)
FROM customer, orders, lineitem
WHERE o_orderkey IN (
  SELECT l_orderkey FROM lineitem
  GROUP BY l_orderkey
  HAVING SUM(l_quantity) > 300
)
AND c_custkey = o_custkey
AND o_orderkey = l_orderkey
GROUP BY c_name, c_custkey, o_orderkey, o_orderdate, o_totalprice
ORDER BY o_totalprice DESC, o_orderdate
LIMIT 100
)sql";

// Q19: Discounted Revenue
static constexpr const char* TPCH_Q19 = R"sql(
SELECT SUM(l_extendedprice * (1 - l_discount)) AS revenue
FROM lineitem, part
WHERE (
      p_partkey = l_partkey
  AND p_brand = 'Brand#12'
  AND p_container IN ('SM CASE', 'SM BOX', 'SM TRIM', 'SM PKG')
  AND l_quantity >= 1 AND l_quantity <= 11
  AND p_size BETWEEN 1 AND 5
  AND l_shipmode IN ('AIR', 'AIR REG')
  AND l_shipinstruct = 'DELIVER IN PERSON'
) OR (
      p_partkey = l_partkey
  AND p_brand = 'Brand#23'
  AND p_container IN ('MED BAG', 'MED BOX', 'MED TRIM', 'MED PKG')
  AND l_quantity >= 10 AND l_quantity <= 20
  AND p_size BETWEEN 1 AND 10
  AND l_shipmode IN ('AIR', 'AIR REG')
  AND l_shipinstruct = 'DELIVER IN PERSON'
) OR (
      p_partkey = l_partkey
  AND p_brand = 'Brand#34'
  AND p_container IN ('LG CASE', 'LG BOX', 'LG TRIM', 'LG PKG')
  AND l_quantity >= 20 AND l_quantity <= 30
  AND p_size BETWEEN 1 AND 15
  AND l_shipmode IN ('AIR', 'AIR REG')
  AND l_shipinstruct = 'DELIVER IN PERSON'
)
)sql";

// Q20: Potential Part Promotion
static constexpr const char* TPCH_Q20 = R"sql(
SELECT s_name, s_address
FROM supplier, nation
WHERE s_suppkey IN (
  SELECT ps_suppkey FROM partsupp
  WHERE ps_partkey IN (
    SELECT p_partkey FROM part WHERE p_name LIKE 'forest%'
  )
  AND ps_availqty > (
    SELECT 0.5 * SUM(l_quantity)
    FROM lineitem
    WHERE l_partkey = ps_partkey
      AND l_suppkey = ps_suppkey
      AND l_shipdate >= '1994-01-01'
      AND l_shipdate < '1995-01-01'
  )
)
AND s_nationkey = n_nationkey
AND n_name = 'CANADA'
ORDER BY s_name
)sql";

// Q21: Suppliers Who Kept Orders Waiting
static constexpr const char* TPCH_Q21 = R"sql(
SELECT s_name, COUNT(*) AS numwait
FROM supplier, lineitem l1, orders, nation
WHERE s_suppkey = l1.l_suppkey
  AND o_orderkey = l1.l_orderkey
  AND o_orderstatus = 'F'
  AND l1.l_receiptdate > l1.l_commitdate
  AND EXISTS (
    SELECT * FROM lineitem l2
    WHERE l2.l_orderkey = l1.l_orderkey
      AND l2.l_suppkey <> l1.l_suppkey
  )
  AND NOT EXISTS (
    SELECT * FROM lineitem l3
    WHERE l3.l_orderkey = l1.l_orderkey
      AND l3.l_suppkey <> l1.l_suppkey
      AND l3.l_receiptdate > l3.l_commitdate
  )
  AND s_nationkey = n_nationkey
  AND n_name = 'SAUDI ARABIA'
GROUP BY s_name
ORDER BY numwait DESC, s_name
LIMIT 100
)sql";

// Q22: Global Sales Opportunity
static constexpr const char* TPCH_Q22 = R"sql(
SELECT cntrycode, COUNT(*) AS numcust, SUM(c_acctbal) AS totacctbal
FROM (
  SELECT SUBSTR(c_phone, 1, 2) AS cntrycode, c_acctbal
  FROM customer
  WHERE SUBSTR(c_phone, 1, 2) IN ('13', '31', '23', '29', '30', '18', '17')
    AND c_acctbal > (
      SELECT AVG(c_acctbal)
      FROM customer
      WHERE c_acctbal > 0.00
        AND SUBSTR(c_phone, 1, 2) IN ('13', '31', '23', '29', '30', '18', '17')
    )
    AND NOT EXISTS (
      SELECT * FROM orders WHERE o_custkey = c_custkey
    )
) AS custsale
GROUP BY cntrycode
ORDER BY cntrycode
)sql";

// ── Database path ─────────────────────────────────────────────────────────────

static std::string g_db_path = "tpch/data/tpch.db";

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
static void phaseJITOverhead(benchmark::State& state, const char* sql) {
    sqlite3* db = openDB();
    sqlite3_stmt* stmt = prepareQuery(db, sql);
    auto* vdbe = reinterpret_cast<Vdbe*>(stmt);

    auto* RS = CRS::ClangRuntimeSpecializer::init();
    auto modStats = CRS::ClangRuntimeSpecializer::getModuleStats();
    auto Prev = CRS::ClangRuntimeSpecializer::getLogLevel();
    CRS::ClangRuntimeSpecializer::setLogLevel(CRS::ClangRuntimeSpecializer::LogLevel::None);
    for (auto _ : state)
        benchmark::DoNotOptimize(RS->specializeOnly<int>(Fn_sqlite3VdbeExec, vdbe));
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
    auto SpecFn = RS->specializeOnly<int>(Fn_sqlite3VdbeExec, vdbe);
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
    clangRuntimeSpecializer::benchmarkJITAnalysis<Fn_sqlite3VdbeExec>(
        state, sqlite3VdbeExec, std::make_tuple(vdbe));
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// ── Q1: Pricing Summary Report ────────────────────────────────────────────────

void BM_unspecialized_tpch_q1(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q1); }
BENCHMARK(BM_unspecialized_tpch_q1)->Name("BM_g:db/sqlite3/tpch;n:tpch_q1;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q1(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q1); }
BENCHMARK(BM_jit_overhead_tpch_q1)->Name("BM_g:db/sqlite3/tpch;n:tpch_q1;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q1(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q1); }
BENCHMARK(BM_specialized_exec_tpch_q1)->Name("BM_g:db/sqlite3/tpch;n:tpch_q1;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q1(benchmark::State& state) { phaseJITAnalysis(state, TPCH_Q1); }
BENCHMARK(BM_jit_analysis_tpch_q1)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q1;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q2: Minimum Cost Supplier ─────────────────────────────────────────────────

void BM_unspecialized_tpch_q2(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q2); }
BENCHMARK(BM_unspecialized_tpch_q2)->Name("BM_g:db/sqlite3/tpch;n:tpch_q2;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q2(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q2); }
BENCHMARK(BM_jit_overhead_tpch_q2)->Name("BM_g:db/sqlite3/tpch;n:tpch_q2;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q2(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q2); }
BENCHMARK(BM_specialized_exec_tpch_q2)->Name("BM_g:db/sqlite3/tpch;n:tpch_q2;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q2(benchmark::State& state) { phaseJITAnalysis(state, TPCH_Q2); }
BENCHMARK(BM_jit_analysis_tpch_q2)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q2;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q3: Shipping Priority ─────────────────────────────────────────────────────

void BM_unspecialized_tpch_q3(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q3); }
BENCHMARK(BM_unspecialized_tpch_q3)->Name("BM_g:db/sqlite3/tpch;n:tpch_q3;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q3(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q3); }
BENCHMARK(BM_jit_overhead_tpch_q3)->Name("BM_g:db/sqlite3/tpch;n:tpch_q3;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q3(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q3); }
BENCHMARK(BM_specialized_exec_tpch_q3)->Name("BM_g:db/sqlite3/tpch;n:tpch_q3;t:specialized_exec;")->MinTime(1.0);

// ── Q4: Order Priority Checking ───────────────────────────────────────────────

void BM_unspecialized_tpch_q4(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q4); }
BENCHMARK(BM_unspecialized_tpch_q4)->Name("BM_g:db/sqlite3/tpch;n:tpch_q4;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q4(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q4); }
BENCHMARK(BM_jit_overhead_tpch_q4)->Name("BM_g:db/sqlite3/tpch;n:tpch_q4;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q4(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q4); }
BENCHMARK(BM_specialized_exec_tpch_q4)->Name("BM_g:db/sqlite3/tpch;n:tpch_q4;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q4(benchmark::State& state) { phaseJITAnalysis(state, TPCH_Q4); }
BENCHMARK(BM_jit_analysis_tpch_q4)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q4;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q5: Local Supplier Volume ─────────────────────────────────────────────────

void BM_unspecialized_tpch_q5(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q5); }
BENCHMARK(BM_unspecialized_tpch_q5)->Name("BM_g:db/sqlite3/tpch;n:tpch_q5;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q5(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q5); }
BENCHMARK(BM_jit_overhead_tpch_q5)->Name("BM_g:db/sqlite3/tpch;n:tpch_q5;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q5(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q5); }
BENCHMARK(BM_specialized_exec_tpch_q5)->Name("BM_g:db/sqlite3/tpch;n:tpch_q5;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q5(benchmark::State& state) { phaseJITAnalysis(state, TPCH_Q5); }
BENCHMARK(BM_jit_analysis_tpch_q5)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q5;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q6: Forecasting Revenue Change ───────────────────────────────────────────

void BM_unspecialized_tpch_q6(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q6); }
BENCHMARK(BM_unspecialized_tpch_q6)->Name("BM_g:db/sqlite3/tpch;n:tpch_q6;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q6(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q6); }
BENCHMARK(BM_jit_overhead_tpch_q6)->Name("BM_g:db/sqlite3/tpch;n:tpch_q6;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q6(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q6); }
BENCHMARK(BM_specialized_exec_tpch_q6)->Name("BM_g:db/sqlite3/tpch;n:tpch_q6;t:specialized_exec;")->MinTime(1.0);

// ── Q7: Volume Shipping ───────────────────────────────────────────────────────

void BM_unspecialized_tpch_q7(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q7); }
BENCHMARK(BM_unspecialized_tpch_q7)->Name("BM_g:db/sqlite3/tpch;n:tpch_q7;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q7(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q7); }
BENCHMARK(BM_jit_overhead_tpch_q7)->Name("BM_g:db/sqlite3/tpch;n:tpch_q7;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q7(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q7); }
BENCHMARK(BM_specialized_exec_tpch_q7)->Name("BM_g:db/sqlite3/tpch;n:tpch_q7;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q7(benchmark::State& state) { phaseJITAnalysis(state, TPCH_Q7); }
BENCHMARK(BM_jit_analysis_tpch_q7)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q7;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q8: National Market Share ─────────────────────────────────────────────────

void BM_unspecialized_tpch_q8(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q8); }
BENCHMARK(BM_unspecialized_tpch_q8)->Name("BM_g:db/sqlite3/tpch;n:tpch_q8;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q8(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q8); }
BENCHMARK(BM_jit_overhead_tpch_q8)->Name("BM_g:db/sqlite3/tpch;n:tpch_q8;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q8(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q8); }
BENCHMARK(BM_specialized_exec_tpch_q8)->Name("BM_g:db/sqlite3/tpch;n:tpch_q8;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q8(benchmark::State& state) { phaseJITAnalysis(state, TPCH_Q8); }
BENCHMARK(BM_jit_analysis_tpch_q8)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q8;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q9: Product Type Profit Measure ──────────────────────────────────────────

void BM_unspecialized_tpch_q9(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q9); }
BENCHMARK(BM_unspecialized_tpch_q9)->Name("BM_g:db/sqlite3/tpch;n:tpch_q9;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q9(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q9); }
BENCHMARK(BM_jit_overhead_tpch_q9)->Name("BM_g:db/sqlite3/tpch;n:tpch_q9;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q9(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q9); }
BENCHMARK(BM_specialized_exec_tpch_q9)->Name("BM_g:db/sqlite3/tpch;n:tpch_q9;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q9(benchmark::State& state) { phaseJITAnalysis(state, TPCH_Q9); }
BENCHMARK(BM_jit_analysis_tpch_q9)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q9;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q10: Returned Item Reporting ──────────────────────────────────────────────

void BM_unspecialized_tpch_q10(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q10); }
BENCHMARK(BM_unspecialized_tpch_q10)->Name("BM_g:db/sqlite3/tpch;n:tpch_q10;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q10(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q10); }
BENCHMARK(BM_jit_overhead_tpch_q10)->Name("BM_g:db/sqlite3/tpch;n:tpch_q10;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q10(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q10); }
BENCHMARK(BM_specialized_exec_tpch_q10)->Name("BM_g:db/sqlite3/tpch;n:tpch_q10;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q10(benchmark::State& state) { phaseJITAnalysis(state, TPCH_Q10); }
BENCHMARK(BM_jit_analysis_tpch_q10)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q10;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q11: Important Stock Identification ───────────────────────────────────────

void BM_unspecialized_tpch_q11(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q11); }
BENCHMARK(BM_unspecialized_tpch_q11)->Name("BM_g:db/sqlite3/tpch;n:tpch_q11;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q11(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q11); }
BENCHMARK(BM_jit_overhead_tpch_q11)->Name("BM_g:db/sqlite3/tpch;n:tpch_q11;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q11(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q11); }
BENCHMARK(BM_specialized_exec_tpch_q11)->Name("BM_g:db/sqlite3/tpch;n:tpch_q11;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q11(benchmark::State& state) { phaseJITAnalysis(state, TPCH_Q11); }
BENCHMARK(BM_jit_analysis_tpch_q11)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q11;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q12: Shipping Modes and Order Priority ────────────────────────────────────

void BM_unspecialized_tpch_q12(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q12); }
BENCHMARK(BM_unspecialized_tpch_q12)->Name("BM_g:db/sqlite3/tpch;n:tpch_q12;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q12(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q12); }
BENCHMARK(BM_jit_overhead_tpch_q12)->Name("BM_g:db/sqlite3/tpch;n:tpch_q12;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q12(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q12); }
BENCHMARK(BM_specialized_exec_tpch_q12)->Name("BM_g:db/sqlite3/tpch;n:tpch_q12;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q12(benchmark::State& state) { phaseJITAnalysis(state, TPCH_Q12); }
BENCHMARK(BM_jit_analysis_tpch_q12)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q12;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q13: Customer Distribution ────────────────────────────────────────────────

void BM_unspecialized_tpch_q13(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q13); }
BENCHMARK(BM_unspecialized_tpch_q13)->Name("BM_g:db/sqlite3/tpch;n:tpch_q13;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q13(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q13); }
BENCHMARK(BM_jit_overhead_tpch_q13)->Name("BM_g:db/sqlite3/tpch;n:tpch_q13;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q13(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q13); }
BENCHMARK(BM_specialized_exec_tpch_q13)->Name("BM_g:db/sqlite3/tpch;n:tpch_q13;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q13(benchmark::State& state) { phaseJITAnalysis(state, TPCH_Q13); }
BENCHMARK(BM_jit_analysis_tpch_q13)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q13;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q14: Promotion Effect ─────────────────────────────────────────────────────

void BM_unspecialized_tpch_q14(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q14); }
BENCHMARK(BM_unspecialized_tpch_q14)->Name("BM_g:db/sqlite3/tpch;n:tpch_q14;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q14(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q14); }
BENCHMARK(BM_jit_overhead_tpch_q14)->Name("BM_g:db/sqlite3/tpch;n:tpch_q14;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q14(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q14); }
BENCHMARK(BM_specialized_exec_tpch_q14)->Name("BM_g:db/sqlite3/tpch;n:tpch_q14;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q14(benchmark::State& state) { phaseJITAnalysis(state, TPCH_Q14); }
BENCHMARK(BM_jit_analysis_tpch_q14)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q14;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q15: Top Supplier ─────────────────────────────────────────────────────────

void BM_unspecialized_tpch_q15(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q15); }
BENCHMARK(BM_unspecialized_tpch_q15)->Name("BM_g:db/sqlite3/tpch;n:tpch_q15;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q15(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q15); }
BENCHMARK(BM_jit_overhead_tpch_q15)->Name("BM_g:db/sqlite3/tpch;n:tpch_q15;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q15(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q15); }
BENCHMARK(BM_specialized_exec_tpch_q15)->Name("BM_g:db/sqlite3/tpch;n:tpch_q15;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q15(benchmark::State& state) { phaseJITAnalysis(state, TPCH_Q15); }
BENCHMARK(BM_jit_analysis_tpch_q15)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q15;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q16: Parts/Supplier Relationship ─────────────────────────────────────────

void BM_unspecialized_tpch_q16(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q16); }
BENCHMARK(BM_unspecialized_tpch_q16)->Name("BM_g:db/sqlite3/tpch;n:tpch_q16;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q16(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q16); }
BENCHMARK(BM_jit_overhead_tpch_q16)->Name("BM_g:db/sqlite3/tpch;n:tpch_q16;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q16(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q16); }
BENCHMARK(BM_specialized_exec_tpch_q16)->Name("BM_g:db/sqlite3/tpch;n:tpch_q16;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q16(benchmark::State& state) { phaseJITAnalysis(state, TPCH_Q16); }
BENCHMARK(BM_jit_analysis_tpch_q16)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q16;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q17: Small-Quantity Order Revenue ────────────────────────────────────────

void BM_unspecialized_tpch_q17(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q17); }
BENCHMARK(BM_unspecialized_tpch_q17)->Name("BM_g:db/sqlite3/tpch;n:tpch_q17;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q17(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q17); }
BENCHMARK(BM_jit_overhead_tpch_q17)->Name("BM_g:db/sqlite3/tpch;n:tpch_q17;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q17(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q17); }
BENCHMARK(BM_specialized_exec_tpch_q17)->Name("BM_g:db/sqlite3/tpch;n:tpch_q17;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q17(benchmark::State& state) { phaseJITAnalysis(state, TPCH_Q17); }
BENCHMARK(BM_jit_analysis_tpch_q17)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q17;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q18: Large Volume Customer ────────────────────────────────────────────────

void BM_unspecialized_tpch_q18(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q18); }
BENCHMARK(BM_unspecialized_tpch_q18)->Name("BM_g:db/sqlite3/tpch;n:tpch_q18;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q18(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q18); }
BENCHMARK(BM_jit_overhead_tpch_q18)->Name("BM_g:db/sqlite3/tpch;n:tpch_q18;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q18(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q18); }
BENCHMARK(BM_specialized_exec_tpch_q18)->Name("BM_g:db/sqlite3/tpch;n:tpch_q18;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q18(benchmark::State& state) { phaseJITAnalysis(state, TPCH_Q18); }
BENCHMARK(BM_jit_analysis_tpch_q18)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q18;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q19: Discounted Revenue ───────────────────────────────────────────────────

void BM_unspecialized_tpch_q19(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q19); }
BENCHMARK(BM_unspecialized_tpch_q19)->Name("BM_g:db/sqlite3/tpch;n:tpch_q19;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q19(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q19); }
BENCHMARK(BM_jit_overhead_tpch_q19)->Name("BM_g:db/sqlite3/tpch;n:tpch_q19;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q19(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q19); }
BENCHMARK(BM_specialized_exec_tpch_q19)->Name("BM_g:db/sqlite3/tpch;n:tpch_q19;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q19(benchmark::State& state) { phaseJITAnalysis(state, TPCH_Q19); }
BENCHMARK(BM_jit_analysis_tpch_q19)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q19;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q20: Potential Part Promotion ─────────────────────────────────────────────

void BM_unspecialized_tpch_q20(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q20); }
BENCHMARK(BM_unspecialized_tpch_q20)->Name("BM_g:db/sqlite3/tpch;n:tpch_q20;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q20(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q20); }
BENCHMARK(BM_jit_overhead_tpch_q20)->Name("BM_g:db/sqlite3/tpch;n:tpch_q20;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q20(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q20); }
BENCHMARK(BM_specialized_exec_tpch_q20)->Name("BM_g:db/sqlite3/tpch;n:tpch_q20;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q20(benchmark::State& state) { phaseJITAnalysis(state, TPCH_Q20); }
BENCHMARK(BM_jit_analysis_tpch_q20)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q20;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q21: Suppliers Who Kept Orders Waiting ────────────────────────────────────

void BM_unspecialized_tpch_q21(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q21); }
BENCHMARK(BM_unspecialized_tpch_q21)->Name("BM_g:db/sqlite3/tpch;n:tpch_q21;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q21(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q21); }
BENCHMARK(BM_jit_overhead_tpch_q21)->Name("BM_g:db/sqlite3/tpch;n:tpch_q21;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q21(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q21); }
BENCHMARK(BM_specialized_exec_tpch_q21)->Name("BM_g:db/sqlite3/tpch;n:tpch_q21;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q21(benchmark::State& state) { phaseJITAnalysis(state, TPCH_Q21); }
BENCHMARK(BM_jit_analysis_tpch_q21)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q21;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Q22: Global Sales Opportunity ────────────────────────────────────────────

void BM_unspecialized_tpch_q22(benchmark::State& state) { phaseUnspecialized(state, TPCH_Q22); }
BENCHMARK(BM_unspecialized_tpch_q22)->Name("BM_g:db/sqlite3/tpch;n:tpch_q22;t:unspecialized;")->MinTime(1.0);

void BM_jit_overhead_tpch_q22(benchmark::State& state) { phaseJITOverhead(state, TPCH_Q22); }
BENCHMARK(BM_jit_overhead_tpch_q22)->Name("BM_g:db/sqlite3/tpch;n:tpch_q22;t:jit_overhead;")->Iterations(1);

void BM_specialized_exec_tpch_q22(benchmark::State& state) { phaseSpecializedExec(state, TPCH_Q22); }
BENCHMARK(BM_specialized_exec_tpch_q22)->Name("BM_g:db/sqlite3/tpch;n:tpch_q22;t:specialized_exec;")->MinTime(1.0);

void BM_jit_analysis_tpch_q22(benchmark::State& state) { phaseJITAnalysis(state, TPCH_Q22); }
BENCHMARK(BM_jit_analysis_tpch_q22)
    ->Name("BM_g:db/sqlite3/tpch;n:tpch_q22;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

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
