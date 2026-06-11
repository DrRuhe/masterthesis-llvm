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
    struct InferStats {
        unsigned Examined = 0;       // loads whose pointer is derived from a tracked arg
        unsigned BlockedEscaped = 0; // loads blocked solely by escaped ancestor (not mutated)
        unsigned BlockedMutated = 0; // loads blocked by an actual store in this function
        unsigned Annotated = 0;      // loads that received !invariant.load
    };

    static void inferReadOnlyFields(llvm::Function& F, llvm::AAResults& AA,
                                    llvm::MemorySSA* MSSA, InferStats* Stats = nullptr);

    struct PointerInfo {
        llvm::Value* OriginArg;
        FieldPath Path;
        bool Escaped = false;
        bool Mutated = false;
    };

    static std::map<llvm::Value*, PointerInfo> runCaptureAnalysis(llvm::Function& F);

private:

    // Tracks which fields of which arguments are written to.
    std::map<llvm::Argument*, std::set<FieldPath>> MutableFields;

public:
    struct StaticMutabilityAnalysisPass : public llvm::PassInfoMixin<StaticMutabilityAnalysisPass> {
        llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM);
    };
};

} // namespace clangRuntimeSpecializer
