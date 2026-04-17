// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe 10 | FileCheck %s --check-prefix=EXE-10 --dump-input=always

// EXE-10: [Instruction Stats]
// EXE-10: Total Instructions: {{[0-9]+}}
// EXE-100: [Instruction Stats]
// EXE-100: Total Instructions: {{[0-9]+}}

#include "ClangRuntimeSpecializer.h"
#include <cstdio>
#include <iostream>

extern "C" int loop_function(int iterations) __asm__("loop_function");
int loop_function(int iterations) {
    int sum = 0;
    for (int i = 0; i < iterations; ++i) {
        std::printf("Iteration %d\n", i);
        sum += i;
    }
    return sum;
}

inline constexpr char Fn_loop_function[] = "loop_function";

int main(int argc, char** argv) {
    int iters = 10;
    if (argc > 1) iters = std::stoi(argv[1]);

    clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Debug);

    std::cout << "Calling loop_function with " << iters << " iterations." << std::endl;
    auto* RS = clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
    RS->setOptions(clangRuntimeSpecializer::ClangRuntimeSpecializer::Options::Default().withInstructionInstrumentation(true));
    int result = clangRuntimeSpecializer::specializeFunctionOrFallback(Fn_loop_function, loop_function, iters);
    RS->printCounters();
    std::cout << "Result: " << result << std::endl;

    return 0;
}
