 Pipeline 1 Linking Issues: Root Cause Analysis

## Context

Pipeline 1 (`OptimizationPipelineToUse = 1`) uses `ConstantArgFunctionSpecializationPass` to clone the target function (e.g. `sqlite3VdbeExec`) with a baked-in constant argument (the runtime `Vdbe*` pointer), rather than inlining the entire SQLite module into the wrapper (Pipeline 0). This avoids the 234K → 3M+ instruction explosion that Pipeline 0 produces for sqlite3.

For JIT compilation to work, all symbols referenced by the JIT-compiled functions must be resolvable at link time — either from definitions in the JIT module itself, or from the host process via `DynamicLibrarySearchGenerator` (which calls `dlsym`).

SQLite is compiled as a single amalgamation (`sqlite3.c`) and embedded as a bitcode blob. This creates two hard linking constraints:

---

## Problem 1: Static C Locals Get Zero-Initialized When Functions Are Compiled by the JIT

### Root Cause

When `prepareModuleForJIT` runs, it sets all non-declaration functions (except the wrapper) to `AvailableExternallyLinkage`. For Pipeline 1 to JIT-compile any function at all, it must convert some functions back to `InternalLinkage` (since `AvailableExternal` functions are never compiled by the JIT — they're expected to be resolved from the host).

The original Pipeline 1 approach converted **all** `AvailableExternal` functions to `Internal`. This causes the JIT to compile every SQLite function from scratch. For each compiled function, all `internal`-linkage globals belonging to it (C `static` local variables) are zero-initialized in the JIT's memory.

Many SQLite functions use static locals as:
- **Initialization guards** (`static int bInit = 0;`): the host has already set these to 1; the JIT sees 0 and re-runs initialization, potentially double-initializing or re-using freed memory.
- **Function-pointer caches** (e.g. pointers to OS-layer read/write/sync functions): the host has populated these from the VFS initialization; the JIT sees null pointers.
- **String caches** or other lazily-populated data.

The observed crash is `SIGSEGV` in `strlen` called with `rdi = 0x12c0 = 4800` — a small integer treated as a pointer. This is consistent with a `char*` static local that was initialized to a valid string address in the host but is zero or garbage in the JIT.

**Trigger**: Crash occurs during execution of the JIT-compiled function, after JIT compilation succeeds.

### Affected Approach

- Converting ALL `AvailableExternal` functions to `Internal` in Pipeline 1.

---

## Problem 2: Pure-Static Functions Are Not in the Dynamic Symbol Table

### Root Cause

`DynamicLibrarySearchGenerator` resolves symbols via `dlsym(RTLD_DEFAULT, name)`. On Linux, `dlsym` only sees symbols with `STB_GLOBAL` or `STB_WEAK` binding in the ELF dynamic symbol table (`.dynsym`). Even with `--export-dynamic`, only `STB_GLOBAL` symbols are exported.

SQLite uses two distinct patterns for internal functions:

1. **`SQLITE_PRIVATE` functions**: Defined as `SQLITE_PRIVATE int foo(...)` which expands to `static int foo(...)` by default. With `-DSQLITE_PRIVATE=""`, the `static` is removed, making them globally visible (`STB_GLOBAL` in ELF). `DynamicLibrarySearchGenerator` can find these.

2. **Plain `static` functions** (not using the `SQLITE_PRIVATE` macro): Always `STB_LOCAL` in ELF regardless of any compile flags. `DynamicLibrarySearchGenerator` cannot find these. Examples from sqlite3.c:
   - `unixRead`, `unixWrite`, `unixSync`, `unixClose`, `unixLock`, `unixUnlock` — the POSIX VFS I/O layer
   - `memjrnlRead`, `memjrnlWrite`, `memjrnlTruncate`, `memjrnlClose` — the in-memory journal VFS
   - `dotlockLock`, `dotlockUnlock`, `dotlockClose` — dot-file locking
   - `nolockLock`, `nolockUnlock`, `nolockClose` — no-op locking
   - `pcache1Fetch`, `pcache1Unpin`, `pcache1Rekey`, etc. — page cache
   - `jsonEachConnect`, `jsonEachFilter`, etc. — JSON table functions
   - `pragmaVtabConnect`, `pragmaVtabFilter`, etc. — PRAGMA virtual table
   - `statInit`, `statPush`, `statGet` — sqlite_stat analysis functions
   - Many more.

### Why These Appear as Unresolved References

Many of these functions appear not as direct calls but as **function pointer initializers in static const structs** — the VFS method tables:

```c
static const sqlite3_io_methods posixIoMethods = {
    3,           /* iVersion */
    unixClose,   /* xClose */
    unixRead,    /* xRead */
    unixWrite,   /* xWrite */
    unixTruncate,/* xTruncate */
    unixSync,    /* xSync */
    ...
};

static const sqlite3_io_methods MemJournalMethods = {
    1,
    memjrnlClose,
    memjrnlRead,
    memjrnlWrite,
    memjrnlTruncate,
    ...
};
```

These structs are `static const` globals in C, which become `internal constant` globals in LLVM IR. Their initializers contain references to the static functions above. When the JIT module has an Internal function that transitively references one of these struct globals, the LLVM linker needs to resolve all function pointers mentioned in those initializer lists — even if the function pointer is never actually loaded at runtime.

A **BFS from `sqlite3VdbeExec` following direct call edges** misses these functions entirely: they are referenced through global initializers, not through `call` instructions.

**Trigger**: JIT materialization fails with `Missing definitions in module: [ memjrnlTruncate, unixRead, ... ]`.

---

## The Tension Between the Two Problems

| Approach | Problem 1 (static locals) | Problem 2 (STB_LOCAL functions) |
|---|---|---|
| ALL AvailableExternal → Internal | JIT compiles all functions; static locals zero-initialized → **crash during execution** | Functions compiled by JIT; no missing symbols |
| BFS reachable only → Internal | Only target + direct/transitive callees compiled; many functions untouched → **may still have zero-init for those converted** | VFS-table-referenced functions not in BFS → **link error** |
| Nothing → Internal (all AvailableExternal) | No static-local issue | STB_LOCAL functions can't be found → **link error** |

There is no option that simultaneously avoids zero-initialization issues AND resolves all STB_LOCAL references using the current `DynamicLibrarySearchGenerator` approach.

---

## Proposed Solutions

### Solution A: Register All Local Symbols from the Host Binary (Recommended)

Read the host executable (`/proc/self/exe`) using `llvm::object::ObjectFile` at JIT init time and extract **all** symbols, including `STB_LOCAL` ones. Register them as absolute symbols in the `JITDylib`.

This resolves both problems simultaneously:
- **Static-local globals**: Change their LLVM IR linkage from `internal` to `external` and register their actual host addresses. The JIT resolves them to the already-initialized host values instead of creating zero-initialized copies.
- **Static functions** (`unixRead`, `memjrnlTruncate`, etc.): Registered as absolute function addresses. All declarations resolve correctly.

**Implementation sketch** (in `ClangRuntimeSpecializer::init()`):

```cpp
// Read host binary and register all symbols including STB_LOCAL
auto ExeFile = llvm::object::createBinary("/proc/self/exe");
for (auto &Sym : ExeFile->symbols()) {
    auto Addr = Sym.getAddress();
    auto Name = Sym.getName();
    if (Addr && Name && *Addr != 0)
        JITD->define(absoluteSymbols({{Mangler(*Name), {*Addr, JITSymbolFlags::Exported}}}));
}
```

**Tradeoffs**:
- One-time startup cost (ELF parsing, typically < 50ms for a 100MB binary)
- Works for any library, not just SQLite
- Must handle symbol name mangling (LLVM's `DataLayout` mangler)
- Linux-specific (`/proc/self/exe`); macOS would use `_NSGetExecutablePath`

### Solution B: Trap Stubs for Provably-Dead References

For functions that appear only in global struct initializers but are never loaded/called in the actual execution path (e.g. `MemJournalMethods` when no in-memory journal is ever opened), provide a `ud2` stub. This allows linking to succeed; any accidental call traps immediately.

**Limitation**: Requires knowing a priori which functions are dead. `unixRead` is NOT dead (it IS called when reading from the database file). This solution only works for provably-never-called functions.

### Solution C: Mark VFS Struct Globals as AvailableExternal

Force `posixIoMethods`, `MemJournalMethods`, and similar VFS tables to `AvailableExternallyLinkage`. The JIT resolves the whole struct from the host — including all embedded function pointers. The functions in those pointers are then never referenced as standalone symbols.

**Limitation**: These struct globals are `static const` in C → `internal constant` in LLVM IR → `DynamicLibrarySearchGenerator` cannot find them from the host either (STB_LOCAL). This requires Solution A first to work.

### Solution D: Compile sqlite3 with `-fPIC` and Export All Symbols

Force all symbols to be globally visible by compiling sqlite3.c without any `static`, e.g. using a custom header that `#define static /* empty */`. Then all functions are `STB_GLOBAL` and `--export-dynamic` exports them.

**Limitation**: Removes all C static-local optimization from SQLite. Potentially unsafe (name collisions). Not practical for production use.

---

## Current State (as of this session)

- **`-DSQLITE_PRIVATE=""`** added to CMakeLists.txt sqlite3.c compilation: fixes `sqlite3Config`, `sqlite3VdbeExec`, and ~4000 other SQLITE_PRIVATE functions.
- **`-fvisibility=default`** also added: has no effect on `static` C symbols (visibility attribute does not override C storage class).
- **ALL-Internal approach** is the current code in `ClangRuntimeSpecializer.cpp` (Pipeline 1 setup block) but causes a crash during execution (`strlen` with invalid pointer).
- **BFS approach** was tested in this session and produces the link error above.
- **`TrapUnreachable=true`** is set in LLJIT init: makes `unreachable` IR emit `ud2` (prevents 0-byte .text section JITLink crash).
- Debug guards still in place (TODO to remove once execution is stable):
  - Initial pass skipped for Pipeline 1
  - Fixpoint iterations = 0 for Pipeline 1
  - Final O3 skipped for Pipeline 1

## Recommended Next Step

Implement **Solution A** (local symbol registration from `/proc/self/exe`) in `ClangRuntimeSpecializer::init()`. This is a self-contained change that does not depend on how Pipeline 1 converts linkages, and it directly eliminates both problems.

Once linking works, re-enable optimization passes incrementally (prune → initial → fixpoint → final) and validate correctness at each step.
