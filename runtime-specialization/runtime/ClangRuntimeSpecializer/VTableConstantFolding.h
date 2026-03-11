//===- VTableConstantFolding.h - Fold constant vtable pointers --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass identifies vtable pointers stored as constants to allocas and
// replaces loads of those pointers with the known constants. This enables
// devirtualization of virtual calls in JIT-compiled code.
//
// This is a custom pass for the ClangRuntimeSpecializer JIT system.
//
//===----------------------------------------------------------------------===//

#ifndef CLANG_RUNTIME_SPECIALIZER_VTABLECONSTANTFOLDING_H
#define CLANG_RUNTIME_SPECIALIZER_VTABLECONSTANTFOLDING_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class VTableConstantFoldingPass : public PassInfoMixin<VTableConstantFoldingPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // end namespace llvm

#endif // CLANG_RUNTIME_SPECIALIZER_VTABLECONSTANTFOLDING_H
