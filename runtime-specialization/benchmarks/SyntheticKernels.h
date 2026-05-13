#pragma once

// Forward declarations / definitions for the three synthetic kernel scenarios.
// This header is shared by SyntheticKernels.cpp (definitions) and
// SpecializerBenchmark.cpp (benchmark registration). Method bodies are NOT
// inline here so that the JIT blob for SyntheticKernels.cpp remains clean and
// contains no benchmark-library dependencies.

struct Config {
    int threshold;
    bool direction;  // true = (val >= threshold), false = (val < threshold)
};

class A {
    int value;
public:
    explicit A(int val) : value(val) {}
    // noinline + custom asm name so the specializer can look it up by "A::add".
    __attribute__((noinline)) int add(int a, int b) const __asm__("A::add");
};

extern "C" int mypow_bench(int x);
extern "C" int config_count(Config* cfg, int n);

extern Config g_config;
extern A      g_a_instance;
