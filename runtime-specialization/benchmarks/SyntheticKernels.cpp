// Synthetic kernel definitions for the specializer benchmark.
//
// This translation unit is intentionally lean: no benchmark-library headers,
// no LLVM headers, no template-heavy includes. The IR-dumping pass captures
// only this small module so the JIT blob for each kernel contains only what
// the kernel actually needs.

#include "SyntheticKernels.h"

// ── Scenario 1: mypow_bench ───────────────────────────────────────────────────
// Loop with known iteration count — specializing x makes the bound constant.

extern "C" int mypow_bench(int x) {
    int result = 1;
    for (int i = 0; i < x; i++)
        result *= 3;
    return result;
}

// ── Scenario 2: config_count ──────────────────────────────────────────────────
// Branch elimination via pointer-to-config: specializing cfg makes threshold
// and direction compile-time constants, eliminating the conditional branch and
// enabling loop unrolling / vectorization when n is also constant.

extern "C" int config_count(Config* cfg, int n) {
    int count = 0;
    for (int i = 0; i < n; i++)
        count += ((i >= cfg->threshold) == cfg->direction);
    return count;
}

// ── Scenario 3: A::add ────────────────────────────────────────────────────────
// Method field constant-folding: specializing `this` makes the `value` field
// a compile-time constant, folding the entire addition to a constant return.

int A::add(int a, int b) const {
    return value + a + b;
}

// ── Global instances used by the benchmarks ───────────────────────────────────

Config g_config{512, true};
A      g_a_instance{42};
