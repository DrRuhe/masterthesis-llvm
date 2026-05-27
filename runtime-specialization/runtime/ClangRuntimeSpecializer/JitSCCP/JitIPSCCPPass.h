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

/// A set of parameters to control various transforms performed by JitIPSCCPPass.
/// Each of the boolean parameters can be set to:
///   true - enabling the transformation.
///   false - disabling the transformation.
/// Intended use is to create a default object, modify parameters with
/// additional setters and then pass it to JitIPSCCPPass.
struct JitIPSCCPOptions {
  bool AllowFuncSpec;

  JitIPSCCPOptions(bool AllowFuncSpec = true) : AllowFuncSpec(AllowFuncSpec) {}

  /// Enables or disables Specialization of Functions.
  JitIPSCCPOptions &setFuncSpec(bool FuncSpec) {
    AllowFuncSpec = FuncSpec;
    return *this;
  }
};

/// Pass to perform interprocedural constant propagation.
class JitIPSCCPPass
    : public llvm::PassInfoMixin<JitIPSCCPPass> {
  JitIPSCCPOptions Options;

public:
  JitIPSCCPPass() = default;

  JitIPSCCPPass(JitIPSCCPOptions Options) : Options(Options) {}

  llvm::PreservedAnalyses run(llvm::Module &M,
                              llvm::ModuleAnalysisManager &AM);

  bool isFuncSpecEnabled() const { return Options.AllowFuncSpec; }
};

} // namespace clangRuntimeSpecializer

#endif // CLANG_RUNTIME_SPECIALIZER_JITSCCP_JITIPSCCPPASS_H
