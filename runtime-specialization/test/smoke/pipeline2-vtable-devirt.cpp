// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: CRS_DEFAULT_PIPELINE=2 %t.exe '>=95' '<100' | FileCheck %s --check-prefix=EXE --dump-input=always
// RUN: %clangxx -g -O3 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.o3.exe
// RUN: CRS_DEFAULT_PIPELINE=2 %t.o3.exe '>=95' '<100' | FileCheck %s --check-prefix=EXE --dump-input=always

// Verify that Pipeline 2 (JIT-IPSCCP) eliminates virtual dispatch for the
// virtual-methods-simple pattern: a Scan/Filter operator pipeline where the
// concrete types are constant at the specialization call site.

#include <iostream>
#include <string>
#include <cstdio>
#include "ClangRuntimeSpecializer.h"

class Operator {
public:
    virtual int next() = 0;
    virtual ~Operator() {}
};

class Filter final : public Operator {
    Operator* child;
    int threshold;
    bool direction;
public:
    Filter(Operator* child, int threshold, bool direction)
        : child(child), threshold(threshold), direction(direction) {}

    int next() override {
        int val;
        while ((val = child->next()) != -1 && ((val >= threshold) != direction));
        return val;
    }
};

class Scan final : public Operator {
    int value = 0;
public:
    int next() override {
        return (value > 100) ? -1 : value++;
    }
};

extern "C" int execute_query(int i);
int execute_query(int i) {
    Scan scan;
    Filter op = Filter(&scan, i, true);
    return op.next();
}

std::string argsToString(int argc, char* argv[]) {
    std::string res;
    for (int i = 1; i < argc; ++i) {
        if (i > 1) res += " ";
        res += argv[i];
    }
    return res;
}

int main(int argc, char* argv[]) {
    clangRuntimeSpecializer::ClangRuntimeSpecializer::setLogLevel(
        clangRuntimeSpecializer::ClangRuntimeSpecializer::LogLevel::Debug);

    int result = clangRuntimeSpecializer::specializeOrFallback(&execute_query, 5);
    std::fprintf(stdout, "Operators returned %d \n", result);
    return 0;
}

// EXE-NOT: ERROR: Specialization failed:
// EXE: DEBUG: Optimized specialized function IR:
// EXE-NOT: call ptr
// EXE-NOT: load ptr, ptr %vtable
// EXE: Operators returned 5
