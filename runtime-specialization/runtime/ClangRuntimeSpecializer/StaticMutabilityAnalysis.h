#pragma once

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/MemorySSA.h"
#include <set>
#include <vector>
#include <map>

namespace clangRuntimeSpecializer {

struct FieldPath {
    std::vector<uint64_t> Indices;

    bool operator<(const FieldPath& Other) const {
        return Indices < Other.Indices;
    }
};

class StaticMutabilityAnalysis {
public:
    static void inferReadOnlyFields(llvm::Function& F, llvm::AAResults& AA, llvm::MemorySSA* MSSA);

private:
    struct PointerInfo {
        llvm::Value* OriginArg;
        FieldPath Path;
        bool Escaped = false;
    };

    // Tracks which fields of which arguments are written to.
    std::map<llvm::Argument*, std::set<FieldPath>> MutableFields;

public:
    struct StaticMutabilityAnalysisPass : public llvm::PassInfoMixin<StaticMutabilityAnalysisPass> {
        llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM);
    };
};

} // namespace clangRuntimeSpecializer
