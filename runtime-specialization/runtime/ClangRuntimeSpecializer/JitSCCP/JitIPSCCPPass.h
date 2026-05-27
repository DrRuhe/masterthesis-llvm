// JIT-SCCP fork — copied verbatim from LLVM 4fecb930be12af5bfe685402396cf5b3f89f175a on 2026-05-27
//===- SCCP.h - Sparse Conditional Constant Propagation ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass implements  interprocedural sparse conditional constant
// propagation and merging.
//
// Specifically, this:
//   * Assumes values are constant unless proven otherwise
//   * Assumes BasicBlocks are dead unless proven otherwise
//   * Proves values to be constant, and replaces them with constants
//   * Proves conditional branches to be unconditional
//
//===----------------------------------------------------------------------===//

#ifndef CLANG_RUNTIME_SPECIALIZER_JITSCCP_JITIPSCCPPASS_H
#define CLANG_RUNTIME_SPECIALIZER_JITSCCP_JITIPSCCPPASS_H

#include "llvm/IR/PassManager.h"

namespace clangRuntimeSpecializer {

// Forward declaration — defined in ClangRuntimeSpecializer.h at namespace scope.
struct JitFunctionSpecializationOptions;

/// A set of parameters to control various transforms performed by JitIPSCCPPass.
struct JitIPSCCPOptions {
  bool AllowFuncSpec;

  JitIPSCCPOptions(bool AllowFuncSpec = true) : AllowFuncSpec(AllowFuncSpec) {}

  JitIPSCCPOptions &setFuncSpec(bool FuncSpec) {
    AllowFuncSpec = FuncSpec;
    return *this;
  }
};

/// Pass to perform interprocedural constant propagation.
class JitIPSCCPPass : public llvm::PassInfoMixin<JitIPSCCPPass> {
  JitIPSCCPOptions Options;
  // Cost-model knobs from JitFunctionSpecializationOptions (stored by value
  // to avoid include dependency on ClangRuntimeSpecializer.h in this header).
  unsigned P2MinFunctionSize  = 1;
  unsigned P2MaxClones        = 0;
  unsigned P2FuncSpecMaxIters = 10;
  bool     P2ForceSpec        = false;
  bool     P2SpecOnAddress    = false;
  bool     P2SpecLiteral      = true;

public:
  JitIPSCCPPass() = default;
  explicit JitIPSCCPPass(JitIPSCCPOptions Options) : Options(Options) {}
  explicit JitIPSCCPPass(const JitFunctionSpecializationOptions& FSOpts);

  llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &AM);

  bool isFuncSpecEnabled() const { return Options.AllowFuncSpec; }

  unsigned getMinFunctionSize()  const { return P2MinFunctionSize; }
  unsigned getMaxClones()        const { return P2MaxClones; }
  unsigned getFuncSpecMaxIters() const { return P2FuncSpecMaxIters; }
  bool     isForceSpec()         const { return P2ForceSpec; }
  bool     isSpecOnAddress()     const { return P2SpecOnAddress; }
  bool     isSpecLiteral()       const { return P2SpecLiteral; }
};

} // namespace clangRuntimeSpecializer

#endif // CLANG_RUNTIME_SPECIALIZER_JITSCCP_JITIPSCCPPASS_H
