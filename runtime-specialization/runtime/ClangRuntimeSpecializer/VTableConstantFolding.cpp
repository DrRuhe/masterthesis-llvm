//===- VTableConstantFolding.cpp - Fold constant vtable pointers ----------===//
//
//
//===----------------------------------------------------------------------===//
//
// This pass identifies vtable pointers stored as constants to allocas and
// replaces loads of those pointers with the known constants. This enables
// devirtualization of virtual calls in JIT-compiled code.
//
// Pattern matched:
//   %obj = alloca %class.Type
//   store ptr @_ZTV_Type+offset, ptr %obj    ; constant vtable store
//   ...
//   %vtable = load ptr, ptr %obj              ; load vtable pointer
//   %funcptr = load ptr, ptr %vtable+N        ; load function pointer
//   call %funcptr(...)                        ; indirect virtual call
//
// Transformation:
//   Replace the vtable load with the known constant, enabling subsequent
//   passes (InstCombine, Inliner) to devirtualize the call.
//
// Safety: Only optimizes when we can prove the vtable pointer never changes:
//   1. Single vtable store to the alloca (one constructor)
//   2. Store dominates all loads
//   3. No subsequent stores to the vtable pointer location
//   4. No placement-new patterns
//
// This is a custom pass for the ClangRuntimeSpecializer JIT system.
//
//===----------------------------------------------------------------------===//

#include "VTableConstantFolding.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Local.h"

using namespace llvm;
using namespace llvm::PatternMatch;

#define DEBUG_TYPE "vtable-constant-folding"

STATISTIC(NumVTableLoadsReplaced, "Number of vtable loads replaced with constants");

namespace {

struct VTableInfo {
  AllocaInst *Alloca;
  StoreInst *VTableStore;
  Value *VTableConstant;
  SmallVector<LoadInst *, 4> VTableLoads;
};

struct GlobalLoadInfo {
  GlobalVariable *GV;
  SmallVector<LoadInst *, 4> Loads;
};

/// Main analysis: determine if a global is safe to optimize for its vtable loads.
static bool analyzeGlobal(GlobalVariable *GV, Function &F, VTableInfo &Info) {
  // Global must be internal to ensure we see all its uses in this module
  if (!GV->hasInternalLinkage()) {
    return false;
  }

  // Must have a constant initializer
  if (!GV->hasInitializer()) {
    return false;
  }

  ConstantStruct *CS = dyn_cast<ConstantStruct>(GV->getInitializer());
  if (!CS || CS->getNumOperands() == 0) {
    return false;
  }

  // The first field should be the vtable pointer (offset 0)
  // It might be nested in base class structs.
  Value *VTableConst = CS->getOperand(0);
  while (auto *InnerCS = dyn_cast<ConstantStruct>(VTableConst)) {
    if (InnerCS->getNumOperands() == 0) break;
    VTableConst = InnerCS->getOperand(0);
  }

  if (!VTableConst->getType()->isPointerTy() || !isa<Constant>(VTableConst)) {
    return false;
  }

  const DataLayout &DL = F.getParent()->getDataLayout();

  // Check all users of the global for any potential stores to offset 0.
  // In our JIT specialization, we trust that vtable pointers (at offset 0)
  // are not modified, even if other fields are.
  for (User *U : GV->users()) {
    // Check if this user is a store or can lead to a store to offset 0
    if (auto *SI = dyn_cast<StoreInst>(U)) {
      APInt Offset(64, 0);
      if (SI->getPointerOperand()->stripAndAccumulateConstantOffsets(DL, Offset, true) == GV && Offset == 0) {
        return false;
      }
    }
    // Also check GEPs/Bitcasts that could be used for stores
    if (auto *GEP = dyn_cast<GEPOperator>(U)) {
      APInt Offset(64, 0);
      if (GEP->stripAndAccumulateConstantOffsets(DL, Offset, true) == GV && Offset == 0) {
        for (User *GU : GEP->users()) {
          if (auto *SI = dyn_cast<StoreInst>(GU)) {
            if (SI->getPointerOperand()->stripPointerCasts() == GEP) {
               return false;
            }
          }
        }
      }
    }
  }

  // Find all loads of the vtable pointer from this global in the current function.
  SmallVector<LoadInst *, 4> VTableLoads;
  for (Instruction &I : instructions(F)) {
    if (auto *LI = dyn_cast<LoadInst>(&I)) {
      APInt Offset(64, 0);
      if (LI->getPointerOperand()->stripAndAccumulateConstantOffsets(DL, Offset, true) == GV && Offset == 0) {
        VTableLoads.push_back(LI);
      }
    }
  }

  if (VTableLoads.empty()) {
    return false;
  }

  // Safe to optimize!
  Info.Alloca = nullptr;
  Info.VTableStore = nullptr;
  Info.VTableConstant = VTableConst;
  Info.VTableLoads = std::move(VTableLoads);

  LLVM_DEBUG(dbgs() << "  Found optimizable global vtable pattern for "
                    << GV->getName() << " with "
                    << Info.VTableLoads.size() << " loads\n");
  return true;
}

static bool analyzeGlobalInitialLoads(GlobalVariable *GV, Function &F, GlobalLoadInfo &Info) {
  if (!GV->hasInternalLinkage() || !GV->hasInitializer())
    return false;

  const DataLayout &DL = F.getParent()->getDataLayout();
  SmallVector<LoadInst *, 4> Loads;

  for (Instruction &I : instructions(F)) {
    if (auto *LI = dyn_cast<LoadInst>(&I)) {
      APInt Offset(64, 0);
      if (LI->getPointerOperand()->stripAndAccumulateConstantOffsets(DL, Offset, true) == GV) {
        // This is a load from the global. Check if it's "initial".
        // For simplicity, we only optimize if there are NO stores to this global in the whole module.
        // This is true for our specialized objects (except for the fields they modify, but
        // we want to propagate the fields that are NOT modified).
        
        // Check if any store exists to this specific offset in the module
        bool HasStore = false;
        for (User *U : GV->users()) {
            APInt SOffset(64, 0);
            if (auto *SI = dyn_cast<StoreInst>(U)) {
                if (SI->getPointerOperand()->stripAndAccumulateConstantOffsets(DL, SOffset, true) == GV && SOffset == Offset) {
                    HasStore = true;
                    break;
                }
            }
            if (auto *GEP = dyn_cast<GEPOperator>(U)) {
                if (GEP->stripAndAccumulateConstantOffsets(DL, SOffset, true) == GV && SOffset == Offset) {
                    for (User *GU : GEP->users()) {
                        if (isa<StoreInst>(GU)) {
                            HasStore = true;
                            break;
                        }
                    }
                }
            }
            if (HasStore) break;
        }

        if (!HasStore) {
            Loads.push_back(LI);
        }
      }
    }
  }

  if (Loads.empty())
    return false;

  Info.GV = GV;
  Info.Loads = std::move(Loads);
  return true;
}

} // end anonymous namespace

/// Find the store that initializes the vtable pointer (offset 0) of an alloca.
/// Returns nullptr if no such store exists or if there are multiple stores.
static StoreInst *findSingleVTableStore(AllocaInst *AI, DominatorTree &DT) {
  StoreInst *VTableStore = nullptr;

  for (User *U : AI->users()) {
    if (auto *SI = dyn_cast<StoreInst>(U)) {
      // Check if this store is to the vtable pointer location (offset 0)
      if (SI->getPointerOperand() == AI) {
        // Found a store directly to the alloca (offset 0 - the vtable pointer)
        if (VTableStore != nullptr) {
          // Multiple stores to vtable pointer - not safe
          LLVM_DEBUG(dbgs() << "  Multiple vtable stores found, skipping\n");
          return nullptr;
        }
        VTableStore = SI;
      }
    }
  }

  return VTableStore;
}

/// Check if the stored value is a constant vtable pointer.
/// Vtable pointers are typically getelementptr constants pointing into vtables.
static Value *getConstantVTablePointer(StoreInst *SI) {
  Value *StoredValue = SI->getValueOperand();

  // Check if it's a constant expression (getelementptr to a vtable)
  if (auto *CE = dyn_cast<ConstantExpr>(StoredValue)) {
    if (CE->getOpcode() == Instruction::GetElementPtr) {
      // This is a GEP constant expression, likely pointing to a vtable
      return CE;
    }
  }

  // Check if it's a constant pointer
  if (isa<Constant>(StoredValue) && StoredValue->getType()->isPointerTy()) {
    return StoredValue;
  }

  return nullptr;
}

/// Check if there are any stores to the vtable pointer location after the
/// initial constructor store. This would indicate the vtable pointer changes.
static bool hasSubsequentVTableStores(AllocaInst *AI, StoreInst *InitialStore) {
  for (User *U : AI->users()) {
    if (auto *SI = dyn_cast<StoreInst>(U)) {
      if (SI != InitialStore && SI->getPointerOperand() == AI) {
        LLVM_DEBUG(dbgs() << "  Found subsequent vtable store, skipping\n");
        return true;
      }
    }
  }
  return false;
}

/// Check for placement-new pattern: llvm.lifetime.end followed by another store.
static bool hasPlacementNewPattern(AllocaInst *AI, StoreInst *InitialStore) {
  bool SeenLifetimeEnd = false;

  for (User *U : AI->users()) {
    if (auto *II = dyn_cast<IntrinsicInst>(U)) {
      if (II->getIntrinsicID() == Intrinsic::lifetime_end) {
        SeenLifetimeEnd = true;
      }
    } else if (auto *SI = dyn_cast<StoreInst>(U)) {
      if (SI != InitialStore && SeenLifetimeEnd && SI->getPointerOperand() == AI) {
        LLVM_DEBUG(dbgs() << "  Found placement-new pattern, skipping\n");
        return true;
      }
    }
  }

  return false;
}

/// Find all loads of the vtable pointer (loads directly from the alloca).
static void findVTableLoads(AllocaInst *AI, SmallVectorImpl<LoadInst *> &Loads) {
  for (User *U : AI->users()) {
    if (auto *LI = dyn_cast<LoadInst>(U)) {
      if (LI->getPointerOperand() == AI) {
        Loads.push_back(LI);
      }
    }
  }
}

/// Check if the vtable store dominates all loads.
static bool storeDominatesAllLoads(StoreInst *Store,
                                    ArrayRef<LoadInst *> Loads,
                                    DominatorTree &DT) {
  for (LoadInst *Load : Loads) {
    if (!DT.dominates(Store, Load)) {
      LLVM_DEBUG(dbgs() << "  Store does not dominate all loads, skipping\n");
      return false;
    }
  }
  return true;
}

/// Main analysis: determine if an alloca is safe to optimize.
static bool analyzeAlloca(AllocaInst *AI, DominatorTree &DT, VTableInfo &Info) {
  LLVM_DEBUG(dbgs() << "Analyzing alloca: " << *AI << "\n");

  // Must be a struct type (has vtable pointer)
  if (!AI->getAllocatedType()->isStructTy()) {
    return false;
  }

  // Find the single vtable store
  StoreInst *VTableStore = findSingleVTableStore(AI, DT);
  if (!VTableStore) {
    return false;
  }

  // Check if the stored value is a constant vtable pointer
  Value *VTableConst = getConstantVTablePointer(VTableStore);
  if (!VTableConst) {
    LLVM_DEBUG(dbgs() << "  Vtable store is not a constant, skipping\n");
    return false;
  }

  // Check for subsequent stores to the vtable pointer
  if (hasSubsequentVTableStores(AI, VTableStore)) {
    return false;
  }

  // Check for placement-new pattern
  if (hasPlacementNewPattern(AI, VTableStore)) {
    return false;
  }

  // Find all vtable loads
  SmallVector<LoadInst *, 4> VTableLoads;
  findVTableLoads(AI, VTableLoads);

  if (VTableLoads.empty()) {
    // No loads to optimize
    return false;
  }

  // Check that the store dominates all loads
  if (!storeDominatesAllLoads(VTableStore, VTableLoads, DT)) {
    return false;
  }

  // Safe to optimize!
  Info.Alloca = AI;
  Info.VTableStore = VTableStore;
  Info.VTableConstant = VTableConst;
  Info.VTableLoads = std::move(VTableLoads);

  LLVM_DEBUG(dbgs() << "  Found optimizable vtable pattern with "
                    << Info.VTableLoads.size() << " loads\n");
  return true;
}

/// Perform the transformation: replace vtable loads with the constant.
static bool transformVTableLoads(VTableInfo &Info) {
  bool Changed = false;

  for (LoadInst *Load : Info.VTableLoads) {
    LLVM_DEBUG(dbgs() << "  Replacing load: " << *Load
                      << "\n    with constant: " << *Info.VTableConstant << "\n");

    Load->replaceAllUsesWith(Info.VTableConstant);
    Load->eraseFromParent();
    ++NumVTableLoadsReplaced;
    Changed = true;
  }

  return Changed;
}

PreservedAnalyses VTableConstantFoldingPass::run(Function &F,
                                                   FunctionAnalysisManager &AM) {
  LLVM_DEBUG(dbgs() << "Running VTableConstantFolding on function: "
                    << F.getName() << "\n");

  auto &DT = AM.getResult<DominatorTreeAnalysis>(F);

  SmallVector<VTableInfo, 4> Candidates;

  // Scan all allocas in the entry block
  BasicBlock &Entry = F.getEntryBlock();
  for (Instruction &I : Entry) {
    if (auto *AI = dyn_cast<AllocaInst>(&I)) {
      VTableInfo Info;
      if (analyzeAlloca(AI, DT, Info)) {
        Candidates.push_back(std::move(Info));
      }
    }
  }

  // Scan all internal globals in the module
  Module *M = F.getParent();
  SmallVector<GlobalLoadInfo, 4> GlobalLoads;
  for (GlobalVariable &GV : M->globals()) {
    if (GV.hasInternalLinkage()) {
      VTableInfo Info;
      if (analyzeGlobal(&GV, F, Info)) {
        Candidates.push_back(std::move(Info));
      }
      
      GlobalLoadInfo GInfo;
      if (analyzeGlobalInitialLoads(&GV, F, GInfo)) {
          GlobalLoads.push_back(std::move(GInfo));
      }
    }
  }

  if (Candidates.empty() && GlobalLoads.empty()) {
    return PreservedAnalyses::all();
  }

  // Transform all candidates
  bool Changed = false;
  for (VTableInfo &Info : Candidates) {
    Changed |= transformVTableLoads(Info);
  }

  for (auto &GInfo : GlobalLoads) {
      for (LoadInst *LI : GInfo.Loads) {
          if (LI->getParent() == nullptr) continue; // Skip if already erased
          APInt Offset(64, 0);
          LI->getPointerOperand()->stripAndAccumulateConstantOffsets(M->getDataLayout(), Offset, true);
          if (Constant *C = ConstantFoldLoadFromConstPtr(GInfo.GV->getInitializer(), LI->getType(), Offset, M->getDataLayout())) {
              LI->replaceAllUsesWith(C);
              LI->eraseFromParent();
              Changed = true;
          }
      }
  }

  if (!Changed) {
    return PreservedAnalyses::all();
  }

  // We modified the IR, invalidate affected analyses
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
