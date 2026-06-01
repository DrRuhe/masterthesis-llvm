# Details 017: InternalLinkage Global Variable Resolution

## Key Files

| File | Role |
|------|------|
| `comptime/IRDumpingPass.cpp` | Collect globals, emit registration arrays, call v3 |
| `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.cpp` | BlobEntry extension, v3 handler, init() population |
| `runtime/ClangRuntimeSpecializer/ClangRuntimeSpecializer.h` | BlobInternalGlobals member, specializeOnlyImpl patch loop |

## Data Flow

```
compile time:
  IRDumpingPass (on clone post-GlobalDCE)
    → collect InternalLinkage GVs
    → emit RuntimeSpecializeableIR_global_names [N x ptr]
    → emit RuntimeSpecializeableIR_global_addrs  [N x ptr]
    → constructor calls register_blob_v3(blob, len, funcs, nfuncs, names, addrs, nglobals)

startup:
  register_blob_v3 → BlobEntry.InternalGlobals = [(name, addr), ...]
  init()           → BlobInternalGlobals[i] = map<name, addr> from BlobEntry[i]

specializeOnly:
  clone module
  for each InternalLinkage GV in clone:
    if name in BlobInternalGlobals[blobIdx]:
      replace GV with inttoptr(HOST_ADDR to ptr)
      erase GV
  prepareModuleForJIT (set wrapper ExternalLinkage)
  run optimization pipeline
```

## IRDumpingPass: Global Collection

Collection happens AFTER GlobalDCE on the preprocessed clone (line ~225 in
IRDumpingPass.cpp), BEFORE serialization. Use the clone's globals (post-DCE
survivors) for names; look up corresponding globals in the ORIGINAL module `M`
(not the clone) for addresses, since `M`'s globals are the actual host-process
globals.

```cpp
// After GlobalDCE on ClonedM, before Bytes = serializeModule(ClonedM):
SmallVector<std::pair<Constant*, Constant*>, 16> GlobalEntries;
for (auto &GV : ClonedM.globals()) {
  if (!GV.hasInternalLinkage()) continue;
  if (GV.getValueType()->isSized() == false) continue;  // zero-sized
  if (GV.getName().starts_with("llvm.")) continue;       // LLVM-reserved
  if (GV.getName().starts_with("_ZTV")) continue;        // vtables (WeakODR already)
  if (GV.getName().starts_with("RuntimeSpecializeableIR")) continue; // our own globals
  GlobalVariable *OrigGV = M.getNamedGlobal(GV.getName());
  if (!OrigGV) continue;  // DCE'd from original (shouldn't happen, but be safe)
  // Emit name string
  ...
  // Emit address: ConstantExpr::getPtrToInt(OrigGV, Int64Ty) then IntToPtr
  // (or simply store pointer-to-pointer: the address of OrigGV is &OrigGV,
  //  i.e., the GlobalVariable pointer itself as an inttoptr constant)
  auto *Addr = ConstantExpr::getPtrToInt(OrigGV, Type::getInt64Ty(Ctx));
  GlobalEntries.push_back({/*name_ptr*/, Addr});
}
```

Note: `ConstantExpr::getPtrToInt(OrigGV, Int64Ty)` captures the address of the
original global as a compile-time constant expression. At link time this resolves
to the actual runtime address. This is the same technique used for function pointers
in vtable initializers.

## register_blob_v3 Signature

```cpp
extern "C" void clang_runtime_specializer_register_blob_v3(
    const void* ptr, std::uint64_t len,
    const char* const* funcs, std::uint64_t nfuncs,
    const char* const* global_names,
    const void* const* global_addrs,
    std::uint64_t nglobals);
```

`global_addrs[i]` is the host address of the global (a `void*` that is the actual
memory location of the `static` variable in the host process at startup).

## specializeOnlyImpl Patching Loop

Insertion point: after the module clone (lines ~603-612 of `.h`), before
`prepareModuleForJIT` (line ~643). Both release and debug paths produce a
`std::unique_ptr<llvm::Module> NewModule`; patch that.

```cpp
// Get blob index for this function
size_t BlobIdx = FuncToBlobIdx.at(std::string(funcName));
auto &GlobalMap = BlobInternalGlobals[BlobIdx];

auto &Ctx = NewModule->getContext();
auto *Int64Ty = llvm::Type::getInt64Ty(Ctx);
auto *PtrTy = llvm::PointerType::getUnqual(Ctx);

for (auto &GV : llvm::make_early_inc_range(NewModule->globals())) {
  if (!GV.hasInternalLinkage()) continue;
  auto It = GlobalMap.find(std::string(GV.getName()));
  if (It == GlobalMap.end()) continue;
  auto *Addr = llvm::ConstantInt::get(Int64Ty,
      reinterpret_cast<uint64_t>(It->second));
  auto *PtrConst = llvm::ConstantExpr::getIntToPtr(Addr, PtrTy);
  GV.replaceAllUsesWith(PtrConst);
  GV.eraseFromParent();
}
```

## Why inttoptr and not AvailableExternally

`AvailableExternally` requires the symbol to be resolvable by name via
`DynamicLibrarySearchGenerator`. InternalLinkage globals are not in `.dynsym`.
`inttoptr(HOST_ADDR to ptr)` embeds the address directly as a compile-time constant,
bypassing symbol resolution entirely. This is the same technique used for vtable
devirtualization in P2 (see `PointerChainResolver::resolveInvariantLoadToConstant`).

## P2 Interaction

After patching, loads from the inttoptr constant are of the form:
```llvm
%ptr = getelementptr ..., ptr inttoptr(i64 0x7fff... to ptr), i64 0, i64 %i
%val = load T, ptr %ptr
```

`StaticMutabilityAnalysis` already handles "constant inttoptr values" in its capture
analysis (see `StaticMutabilityAnalysis.cpp`). If no mutation is detected:
1. `inferReadOnlyFields` adds `!invariant.load` to the load
2. `InvariantLoadToConstantPass` (Phase 3 of P2) resolves the constant address and
   folds the load to a literal — provided the address is fully constant (i.e., `%i`
   was also baked in as a constant argument)

No changes to `StaticMutabilityAnalysis` or P2 are required.

## Test Pattern

All tests are single-TU (as per project convention). Static globals and their
target functions MUST be in the same file as the `main()`/`specializeOrFallback`
call (single TU = single blob). The IRDumpingPass sees the static global and the
target function in the same module.

Example structure for Scenario C:
```cpp
// RUN: %clangxx -g -O0 -fpass-plugin=... %s -o %t.exe
// RUN: %t.exe | FileCheck %s

#include "ClangRuntimeSpecializer.h"
static double data[8];
struct DataInit { DataInit() { for (int i=0;i<8;i++) data[i]=i*1.5; } } _init;

extern "C" double read_data(int i);
double read_data(int i) { return data[i]; }

int main() {
  clangRuntimeSpecializer::ClangRuntimeSpecializer::init();
  int idx = 3;
  clangRuntimeSpecializer::assertSpecializedIsEquivalent(read_data, std::tie(idx), std::tie(idx), []{});
  // CHECK: Successfully specialized! No differences could be observed.
}
```
