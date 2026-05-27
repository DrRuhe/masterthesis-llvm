// JIT-SCCP fork — copied verbatim from LLVM 4fecb930be12af5bfe685402396cf5b3f89f175a on 2026-05-27
//===- FunctionSpecialization.h - Function Specialization -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Overview:
// ---------
// Function Specialization is a transformation which propagates the constant
// parameters of a function call from the caller to the callee. It is part of
// the Inter-Procedural Sparse Conditional Constant Propagation (IPSCCP) pass.
// The transformation runs iteratively a number of times which is controlled
// by the option `funcspec-max-iters`. Running it multiple times is needed
// for specializing recursive functions, but also exposes new opportunities
// arising from specializations which return constant values or contain calls
// which can be specialized.
//
// Function Specialization supports propagating constant parameters like
// function pointers, literal constants and addresses of global variables.
// By propagating function pointers, indirect calls become direct calls. This
// exposes inlining opportunities which we would have otherwise missed. That's
// why function specialization is run before the inliner in the optimization
// pipeline; that is by design.
//
// Cost Model:
// -----------
// The cost model facilitates a utility for estimating the specialization bonus
// from propagating a constant argument. This is the InstCostVisitor, a class
// that inherits from the InstVisitor. The bonus itself is expressed as codesize
// and latency savings. Codesize savings means the amount of code that becomes
// dead in the specialization from propagating the constant, whereas latency
// savings represents the cycles we are saving from replacing instructions with
// constant values. The InstCostVisitor overrides a set of `visit*` methods to
// be able to handle different types of instructions. These attempt to constant-
// fold the instruction in which case a constant is returned and propagated
// further.
//
// Function pointers are not handled by the InstCostVisitor. They are treated
// separately as they could expose inlining opportunities via indirect call
// promotion. The inlining bonus contributes to the total specialization score.
//
// For a specialization to be profitable its bonus needs to exceed a minimum
// threshold. There are three options for controlling the threshold which are
// expressed as percentages of the original function size:
//  * funcspec-min-codesize-savings
//  * funcspec-min-latency-savings
//  * funcspec-min-inlining-bonus
// There's also an option for controlling the codesize growth from recursive
// specializations. That is `funcspec-max-codesize-growth`.
//
// Once we have all the potential specializations with their score we need to
// choose the best ones, which fit in the module specialization budget. That
// is controlled by the option `funcspec-max-clones`. To find the best `NSpec`
// specializations we use a max-heap. For more details refer to D139346.
//
// Ideas:
// ------
// - With a function specialization attribute for arguments, we could have
//   a direct way to steer function specialization, avoiding the cost-model,
//   and thus control compile-times / code-size.
//
// - Perhaps a post-inlining function specialization pass could be more
//   aggressive on literal constants.
//
// Limitations:
// ------------
// - We are unable to consider specializations of functions called from indirect
//   callsites whose pointer operand has a lattice value that is known to be
//   constant, either from IPSCCP or previous iterations of FuncSpec. This is
//   because SCCP has not yet replaced the uses of the known constant.
//
// References:
// -----------
// 2021 LLVM Dev Mtg "Introducing function specialisation, and can we enable
// it by default?", https://www.youtube.com/watch?v=zJiCjeXgV5Q
//
//===----------------------------------------------------------------------===//

#ifndef CLANG_RUNTIME_SPECIALIZER_JITSCCP_JITFUNCTIONSPECIALIZATION_H
#define CLANG_RUNTIME_SPECIALIZER_JITSCCP_JITFUNCTIONSPECIALIZATION_H

#include "JitSCCPSolver.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/CodeMetrics.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Transforms/Scalar/SCCP.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/SizeOpts.h"

namespace clangRuntimeSpecializer {

// Map of potential specializations for each function. The JitFunctionSpecializer
// keeps the discovered specialisation opportunities for the module in a single
// vector, where the specialisations of each function form a contiguous range.
// This map's value is the beginning and the end of that range.
using SpecMap = llvm::DenseMap<llvm::Function *, std::pair<unsigned, unsigned>>;

// Just a shorter abbreviation to improve indentation.
using Cost = llvm::InstructionCost;

// Map of known constants found during the specialization bonus estimation.
using ConstMap = llvm::DenseMap<llvm::Value *, llvm::Constant *>;

// Specialization signature, used to uniquely designate a specialization within
// a function.
struct SpecSig {
  // Hashing support, used to distinguish between ordinary, empty, or tombstone
  // keys.
  unsigned Key = 0;
  llvm::SmallVector<ArgInfo, 4> Args;

  bool operator==(const SpecSig &Other) const {
    if (Key != Other.Key)
      return false;
    return Args == Other.Args;
  }

  friend llvm::hash_code hash_value(const SpecSig &S) {
    return llvm::hash_combine(llvm::hash_value(S.Key),
                              llvm::hash_combine_range(S.Args));
  }
};

// Specialization instance.
struct Spec {
  // Original function.
  llvm::Function *F;

  // Cloned function, a specialized version of the original one.
  llvm::Function *Clone = nullptr;

  // Specialization signature.
  SpecSig Sig;

  // Profitability of the specialization.
  unsigned Score;

  // Number of instructions in the specialization.
  unsigned CodeSize;

  // List of call sites, matching this specialization.
  llvm::SmallVector<llvm::CallBase *> CallSites;

  Spec(llvm::Function *F, const SpecSig &S, unsigned Score, unsigned CodeSize)
      : F(F), Sig(S), Score(Score), CodeSize(CodeSize) {}
  Spec(llvm::Function *F, const SpecSig &&S, unsigned Score, unsigned CodeSize)
      : F(F), Sig(S), Score(Score), CodeSize(CodeSize) {}
};

class InstCostVisitor
    : public llvm::InstVisitor<InstCostVisitor, llvm::Constant *> {
  std::function<llvm::BlockFrequencyInfo &(llvm::Function &)> GetBFI;
  llvm::Function *F;
  const llvm::DataLayout &DL;
  llvm::TargetTransformInfo &TTI;
  const JitSCCPSolver &Solver;

  ConstMap KnownConstants;
  // Basic blocks known to be unreachable after constant propagation.
  llvm::DenseSet<llvm::BasicBlock *> DeadBlocks;
  // PHI nodes we have visited before.
  llvm::DenseSet<llvm::Instruction *> VisitedPHIs;
  // PHI nodes we have visited once without successfully constant folding them.
  // Once the InstCostVisitor has processed all the specialization arguments,
  // it should be possible to determine whether those PHIs can be folded
  // (some of their incoming values may have become constant or dead).
  llvm::SmallVector<llvm::Instruction *> PendingPHIs;

  ConstMap::iterator LastVisited;

public:
  InstCostVisitor(
      std::function<llvm::BlockFrequencyInfo &(llvm::Function &)> GetBFI,
      llvm::Function *F, const llvm::DataLayout &DL,
      llvm::TargetTransformInfo &TTI, JitSCCPSolver &Solver)
      : GetBFI(GetBFI), F(F), DL(DL), TTI(TTI), Solver(Solver) {}

  bool isBlockExecutable(llvm::BasicBlock *BB) const {
    return Solver.isBlockExecutable(BB) && !DeadBlocks.contains(BB);
  }

  Cost getCodeSizeSavingsForArg(llvm::Argument *A, llvm::Constant *C);

  Cost getCodeSizeSavingsFromPendingPHIs();

  Cost getLatencySavingsForKnownConstants();

private:
  friend class llvm::InstVisitor<InstCostVisitor, llvm::Constant *>;

  llvm::Constant *findConstantFor(llvm::Value *V) const;

  bool canEliminateSuccessor(llvm::BasicBlock *BB,
                             llvm::BasicBlock *Succ) const;

  Cost getCodeSizeSavingsForUser(llvm::Instruction *User,
                                 llvm::Value *Use = nullptr,
                                 llvm::Constant *C = nullptr);

  Cost estimateBasicBlocks(llvm::SmallVectorImpl<llvm::BasicBlock *> &WorkList);
  Cost estimateSwitchInst(llvm::SwitchInst &I);
  Cost estimateBranchInst(llvm::BranchInst &I);

  // Transitively Incoming Values (TIV) is a set of Values that can "feed" a
  // value to the initial PHI-node. It is defined like this:
  //
  // * the initial PHI-node belongs to TIV.
  //
  // * for every PHI-node in TIV, its operands belong to TIV
  //
  // If TIV for the initial PHI-node (P) contains more than one constant or a
  // value that is not a PHI-node, then P cannot be folded to a constant.
  //
  // As soon as we detect these cases, we bail, without constructing the
  // full TIV.
  // Otherwise P can be folded to the one constant in TIV.
  bool discoverTransitivelyIncomingValues(
      llvm::Constant *Const, llvm::PHINode *Root,
      llvm::DenseSet<llvm::PHINode *> &TransitivePHIs);

  llvm::Constant *visitInstruction(llvm::Instruction &I) { return nullptr; }
  llvm::Constant *visitPHINode(llvm::PHINode &I);
  llvm::Constant *visitFreezeInst(llvm::FreezeInst &I);
  llvm::Constant *visitCallBase(llvm::CallBase &I);
  llvm::Constant *visitLoadInst(llvm::LoadInst &I);
  llvm::Constant *visitGetElementPtrInst(llvm::GetElementPtrInst &I);
  llvm::Constant *visitSelectInst(llvm::SelectInst &I);
  llvm::Constant *visitCastInst(llvm::CastInst &I);
  llvm::Constant *visitCmpInst(llvm::CmpInst &I);
  llvm::Constant *visitUnaryOperator(llvm::UnaryOperator &I);
  llvm::Constant *visitBinaryOperator(llvm::BinaryOperator &I);
};

class JitFunctionSpecializer {

  /// The IPSCCP Solver.
  JitSCCPSolver &Solver;

  llvm::Module &M;

  /// Analysis manager, needed to invalidate analyses.
  llvm::FunctionAnalysisManager *FAM;

  /// Analyses used to help determine if a function should be specialized.
  std::function<llvm::BlockFrequencyInfo &(llvm::Function &)> GetBFI;
  std::function<const llvm::TargetLibraryInfo &(llvm::Function &)> GetTLI;
  std::function<llvm::TargetTransformInfo &(llvm::Function &)> GetTTI;
  std::function<llvm::AssumptionCache &(llvm::Function &)> GetAC;

  llvm::SmallPtrSet<llvm::Function *, 32> Specializations;
  llvm::SmallPtrSet<llvm::Function *, 32> FullySpecialized;
  llvm::DenseMap<llvm::Function *, llvm::CodeMetrics> FunctionMetrics;
  llvm::DenseMap<llvm::Function *, unsigned> FunctionGrowth;
  unsigned NGlobals = 0;

public:
  JitFunctionSpecializer(
      JitSCCPSolver &Solver, llvm::Module &M,
      llvm::FunctionAnalysisManager *FAM,
      std::function<llvm::BlockFrequencyInfo &(llvm::Function &)> GetBFI,
      std::function<const llvm::TargetLibraryInfo &(llvm::Function &)> GetTLI,
      std::function<llvm::TargetTransformInfo &(llvm::Function &)> GetTTI,
      std::function<llvm::AssumptionCache &(llvm::Function &)> GetAC)
      : Solver(Solver), M(M), FAM(FAM), GetBFI(GetBFI), GetTLI(GetTLI),
        GetTTI(GetTTI), GetAC(GetAC) {}

  ~JitFunctionSpecializer();

  bool run();

  InstCostVisitor getInstCostVisitorFor(llvm::Function *F) {
    auto &TTI = GetTTI(*F);
    return InstCostVisitor(GetBFI, F, M.getDataLayout(), TTI, Solver);
  }

private:
  llvm::Constant *getPromotableAlloca(llvm::AllocaInst *Alloca,
                                      llvm::CallInst *Call);

  /// A constant stack value is an AllocaInst that has a single constant
  /// value stored to it. Return this constant if such an alloca stack value
  /// is a function argument.
  llvm::Constant *getConstantStackValue(llvm::CallInst *Call, llvm::Value *Val);

  /// See if there are any new constant values for the callers of \p F via
  /// stack variables and promote them to global variables.
  void promoteConstantStackValues(llvm::Function *F);

  /// Clean up fully specialized functions.
  void removeDeadFunctions();

  /// Remove any ssa_copy intrinsics that may have been introduced.
  void cleanUpSSA();

  /// @brief  Find potential specialization opportunities.
  /// @param F Function to specialize
  /// @param FuncSize Cost of specializing a function.
  /// @param AllSpecs A vector to add potential specializations to.
  /// @param SM  A map for a function's specialisation range
  /// @return True, if any potential specializations were found
  bool findSpecializations(llvm::Function *F, unsigned FuncSize,
                           llvm::SmallVectorImpl<Spec> &AllSpecs, SpecMap &SM);

  /// Compute the inlining bonus for replacing argument \p A with constant \p C.
  unsigned getInliningBonus(llvm::Argument *A, llvm::Constant *C);

  bool isCandidateFunction(llvm::Function *F);

  /// @brief Create a specialization of \p F and prime the JitSCCPSolver
  /// @param F Function to specialize
  /// @param S Which specialization to create
  /// @return The new, cloned function
  llvm::Function *createSpecialization(llvm::Function *F, const SpecSig &S);

  /// Determine if it is possible to specialise the function for constant values
  /// of the formal parameter \p A.
  bool isArgumentInteresting(llvm::Argument *A);

  /// Check if the value \p V  (an actual argument) is a constant or can only
  /// have a constant value. Return that constant.
  llvm::Constant *getCandidateConstant(llvm::Value *V);

  /// @brief Find and update calls to \p F, which match a specialization
  /// @param F Orginal function
  /// @param Begin Start of a range of possibly matching specialisations
  /// @param End End of a range (exclusive) of possibly matching specialisations
  void updateCallSites(llvm::Function *F, const Spec *Begin, const Spec *End);
};

} // namespace clangRuntimeSpecializer

#endif // CLANG_RUNTIME_SPECIALIZER_JITSCCP_JITFUNCTIONSPECIALIZATION_H
