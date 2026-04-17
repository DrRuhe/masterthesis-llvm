
#include <benchmark/benchmark.h>
#include "ClangRuntimeSpecializer.h"
#include "ClangRuntimeSpecializerBenchmark.h"

// ── DB Operator classes ───────────────────────────────────────────────────────

class Operator {
public:
    virtual int next() = 0;
    virtual ~Operator() = default;
};

// Filter: iterates the child operator until a value matching the threshold
// condition is found.  The inner loop is the key target for devirtualization.
class Filter final : public Operator {
    Operator* child;
    int threshold;
    bool direction;  // true = (val >= threshold), false = (val < threshold)
public:
    Filter(Operator* child, int threshold, bool direction)
        : child(child), threshold(threshold), direction(direction) {}

    int next() override __asm__("Filter::next") {
        int val;
        while ((val = child->next()) != -1 && ((val >= threshold) != direction));
        return val;
    }
};

// CyclicScan: produces values 0..n-1 in a repeating cycle (never returns -1),
// providing an infinite stream of data for the benchmark loop.
class CyclicScan final : public Operator {
    int value = 0;
    const int n;
public:
    explicit CyclicScan(int n) : n(n) {}

    int next() override __asm__("CyclicScan::next") {
        int v = value++;
        if (value >= n) value = 0;
        return v;
    }
};

// ── Wrapper function to specialize ───────────────────────────────────────────

// Specializing execute_query for a known op* allows LLVM to devirtualize all
// virtual calls in the operator pipeline, inline them, and eliminate branches
// on constant threshold/direction fields.
extern "C" int execute_query(Operator* op) __asm__("execute_query");
int execute_query(Operator* op) { return op->next(); }

inline constexpr char Fn_execute_query[] = "execute_query";

// ── IR embedding trigger ─────────────────────────────────────────────────────

#ifndef ALL_BENCHMARKS_BUILD
volatile bool g_dummy_trigger = false;
extern "C" __attribute__((used)) void dummy_registration() {
#else
volatile bool g_dummy_trigger_db = false;
extern "C" __attribute__((used)) void dummy_registration_db() {
#endif
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
#ifndef ALL_BENCHMARKS_BUILD
    if (g_dummy_trigger)
#else
    if (g_dummy_trigger_db)
#endif
        RS->callSpecialized<int>(Fn_execute_query, (Operator*)nullptr);
}

// ── Operator pipeline instances ───────────────────────────────────────────────

// Scenario 1: single Filter (>=50) over CyclicScan(100)
static CyclicScan g_single_scan{100};
static Filter     g_single_filter{&g_single_scan, 50, true};

// Scenario 2: chained Filter (>=10 AND <12) over CyclicScan(100)
static CyclicScan g_chained_scan{100};
static Filter     g_inner_filter{&g_chained_scan, 10, true};
static Filter     g_outer_filter{&g_inner_filter,  12, false};

// ── Shared benchmark phase helpers ───────────────────────────────────────────

// Phase 1: unspecialized — each next() call goes through two vtable dispatches.
static void phaseUnspecialized(benchmark::State& state, Operator* op) {
    for (auto _ : state)
        benchmark::DoNotOptimize(execute_query(op));
}

// Phase 2: JIT overhead — measures specialization compilation cost per call.
static void phaseJITOverhead(benchmark::State& state, Operator* op) {
    clangRuntimeSpecializer::benchmarkJITOverhead<Fn_execute_query>(
        state,
        execute_query,
        std::make_tuple(op),
        std::make_tuple(op));
}

// Phase 3: specialized exec — pre-compile once, then measure execution.
// The specialized function has all virtual dispatch and threshold branches
// eliminated; only the scan arithmetic and conditional remain.
static void phaseSpecializedExec(benchmark::State& state, Operator* op) {
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    auto Prev = clangRuntimeSpecializer::ClangRuntimeSpecializer::getLogLevel();
    clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(
        clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::None);
    uintptr_t Addr = RS->specializeOnly(Fn_execute_query, op);
    clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(Prev);
    auto Fn = reinterpret_cast<int(*)()>(Addr);
    for (auto _ : state)
        benchmark::DoNotOptimize(Fn());
}

// ── Single filter benchmarks (scenario 1) ────────────────────────────────────

void BM_unspecialized_single_filter(benchmark::State& state) {
    phaseUnspecialized(state, &g_single_filter);
}
BENCHMARK(BM_unspecialized_single_filter)->Name("BM_g:db_operators;n:single_filter;t:unspecialized;");

void BM_jit_overhead_single_filter(benchmark::State& state) {
    phaseJITOverhead(state, &g_single_filter);
}
BENCHMARK(BM_jit_overhead_single_filter)->Name("BM_g:db_operators;n:single_filter;t:jit_overhead;");

void BM_specialized_exec_single_filter(benchmark::State& state) {
    phaseSpecializedExec(state, &g_single_filter);
}
BENCHMARK(BM_specialized_exec_single_filter)->Name("BM_g:db_operators;n:single_filter;t:specialized_exec;");

// ── Analysis benchmarks (single JIT call + pass-trace JSON) ──────────────────

void BM_jit_analysis_single_filter(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkJITAnalysis<Fn_execute_query>(
        state, execute_query, std::make_tuple((Operator*)&g_single_filter));
}
BENCHMARK(BM_jit_analysis_single_filter)
    ->Name("BM_g:db_operators;n:single_filter;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

void BM_jit_analysis_chained_filter(benchmark::State& state) {
    clangRuntimeSpecializer::benchmarkJITAnalysis<Fn_execute_query>(
        state, execute_query, std::make_tuple((Operator*)&g_outer_filter));
}
BENCHMARK(BM_jit_analysis_chained_filter)
    ->Name("BM_g:db_operators;n:chained_filter;t:jit_analysis;")
    ->Iterations(1)->UseManualTime();

// ── Chained filter benchmarks (scenario 2) ───────────────────────────────────

void BM_unspecialized_chained_filter(benchmark::State& state) {
    phaseUnspecialized(state, &g_outer_filter);
}
BENCHMARK(BM_unspecialized_chained_filter)->Name("BM_g:db_operators;n:chained_filter;t:unspecialized;");

void BM_jit_overhead_chained_filter(benchmark::State& state) {
    phaseJITOverhead(state, &g_outer_filter);
}
BENCHMARK(BM_jit_overhead_chained_filter)->Name("BM_g:db_operators;n:chained_filter;t:jit_overhead;");

void BM_specialized_exec_chained_filter(benchmark::State& state) {
    phaseSpecializedExec(state, &g_outer_filter);
}
BENCHMARK(BM_specialized_exec_chained_filter)->Name("BM_g:db_operators;n:chained_filter;t:specialized_exec;");

#ifndef ALL_BENCHMARKS_BUILD
int main(int argc, char** argv) {
    static RSSMemoryManager g_rss_mgr;
    benchmark::RegisterMemoryManager(&g_rss_mgr);
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
#endif
