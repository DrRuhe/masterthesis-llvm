// RUN: %clangxx -g -O0 -fpass-plugin=%llvmshlibdir/LLVMRuntimeSpecializationComptimePlugin%shlibext %s -o %t.exe
// RUN: %t.exe '>=95' '<100' | FileCheck %s --check-prefix=EXE --dump-input=always

#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include "ClangRuntimeSpecializer.h"

class Operator {
public:
    virtual int next() = 0;
    virtual ~Operator() {}
};
  
// Filters values based on a threshold
class Filter final : public Operator {
    Operator* child;
    int threshold;
    bool direction;
public:
    Filter(Operator* child, int threshold, bool direction) 
        : child(child), threshold(threshold), direction(direction) {}
  
    // Iterate until finding and returning a value that matches the condition
    int next() override __asm__("Filter::next") {
        int val;
        while ((val = child->next()) != -1 && ((val >= threshold) != direction));
        return val;
    }
};

// Iterates over numbers 0 to 100
class Scan final : public Operator {
    int value = 0;
public:
    int next() override __asm__("Scan::next") {
        return (value > 100) ? -1 : value++;
    }
};



int wrapped(Operator* op)
{
    return op->next();
}

// Wrapper function to be specialized
extern "C" int execute_query(int i) __asm__("execute_query");
int execute_query(int i) {
    Scan scan;
    Operator* op = new Filter(&scan, i, true);
    return op->next();
}


inline constexpr char Fn_execute_query[] = "execute_query";

std::string argsToString(int argc, char* argv[]) {
    std::string res;
    for (int i = 1; i < argc; ++i) {
        if (i > 1) res += " ";
        res += argv[i];
    }
    return res;
}

namespace SqlParser {
    Operator* parse(const std::string& query) {
        Operator* op = new Scan();
        size_t last = 0;
        size_t next = 0;
        while ((next = query.find(' ', last)) != std::string::npos) {
            std::string arg = query.substr(last, next - last);
            if (!arg.empty()) {
                if (arg.find(">=") == 0) {
                    op = new Filter(op, std::stoi(arg.substr(2)), true);
                } else if (arg.find("<") == 0) {
                    op = new Filter(op, std::stoi(arg.substr(1)), false);
                }
            }
            last = next + 1;
        }
        std::string arg = query.substr(last);
        if (!arg.empty()) {
            if (arg.find(">=") == 0) {
                op = new Filter(op, std::stoi(arg.substr(2)), true);
            } else if (arg.find("<") == 0) {
                op = new Filter(op, std::stoi(arg.substr(1)), false);
            }
        }
        return op;
    }
}


int main(int argc, char* argv[]) {
    std::string sql_query = argsToString(argc, argv);
    Operator* query_plan = SqlParser::parse(sql_query);

    int result = clangRuntimeSpecializer::specializeOrFallback(Fn_execute_query, &execute_query, 5);
    std::fprintf(stdout, "Operators returned %d \n",result);
    return 0;
}

// EXE-NOT: ERROR: Specialization failed:
// EXE: DEBUG: Optimized specialized function IR:
// EXE-NOT: call noundef i32 @"Filter::next"
// EXE-NOT: load ptr, ptr %vtable
// EXE: Operators returned 5