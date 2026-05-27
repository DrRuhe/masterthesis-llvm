# Binary Size Impact of JIT Integration

**Date**: 2026-05-27  
**Build**: release (`llvm/build/release`)  
**Measurement tool**: `nm -S --defined-only`, `size -A`, `wc -c`, `ldd`

---

## Why We Measure Binary Size

Embedding a JIT compiler into an application binary is not free from a storage
perspective. Our approach introduces overhead through two distinct mechanisms that
must be accounted for before a deployment decision can be made:

1. **IR dump** — `IRDumpingPass` embeds a preprocessed copy of each compiled
   translation unit's LLVM bitcode into the binary at compile time. This blob is
   stored in the `.rodata` section and is loaded by the runtime when
   `callSpecialized()` is first invoked. Because the blob is a copy of the
   compiled IR in a higher-level representation, its size is directly proportional
   to the complexity of the target TU.

2. **JIT infrastructure** — the `ClangRuntimeSpecializer` library and the LLVM
   ORC/JIT subsystem implement the runtime compilation pipeline. In our
   configuration these are deployed as shared libraries
   (`libClangRuntimeSpecializer.so`, `libLLVM.so`). A shared library does not
   add bytes to any individual application binary, but it must exist on the
   deployment system — it is a system-level storage cost that every host running
   JIT-enabled binaries must pay exactly once.

Understanding the split between these two components matters because they have
very different characteristics: the JIT infrastructure is a fixed, per-host cost
shared across all processes and binaries on a machine, while the IR dump is a
per-binary, per-TU variable cost that is paid independently for every
compiled binary.

---

## How We Measure Binary Size

We use two complementary methods.

**Method 1 — LIT smoke tests (methodology illustration).**
Two self-contained LIT tests (`test/smoke/binary-size-small.cpp`,
`test/smoke/binary-size-large.cpp`) each compile the same source file twice:
once with `-fpass-plugin` (IR dump embedded) and once without (no blob, JIT
infrastructure still linked). The binary size difference isolates the IR dump
overhead:

```
IR dump overhead = size(with-plugin binary) − size(without-plugin binary)
```

As a cross-check, the compiled program calls
`ClangRuntimeSpecializer::getModuleStats().BitcodeSizeBytes`, which reports the
raw bitcode blob size at runtime. The two measurements should agree within ~20%
(the file-size difference is slightly smaller because the bitcode is stored
alongside compact ELF metadata; the `getModuleStats()` value reflects the raw
blob before ELF alignment).

**Method 2 — `nm` symbol analysis on real benchmark binaries.**
For the six use-case benchmark executables the IR dump is embedded as a
per-TU symbol named `RuntimeSpecializeableIR_data`. Summing the sizes of all
such symbols gives the total IR dump contribution to the binary:

```bash
nm -S --defined-only <binary> \
  | awk '/RuntimeSpecializeableIR_data/{sum += strtonum("0x"$2)} END{print sum}'
```

This directly attributes bytes from the live binary, requires no rebuild, and
works for binaries with multiple blobs (one per compiled TU).

---

## Results

### Micro-kernel smoke tests

The two LIT tests bracket the per-TU overhead for simple and complex kernels:

| Test | Functions in TU | Instructions | Blob size (`getModuleStats`) | File-size diff |
|---|---|---|---|---|
| `binary-size-small` | 2 | 53 | 5.3 KB | 4.6 KB |
| `binary-size-large` | 5 | 333 | 10.6 KB | 8.6 KB |

The large kernel (four floating-point kernels: dot product, matrix-vector,
GEMM, 1-D convolution) produces a blob 2.0× the size of the single-loop small
kernel, confirming that blob size scales with the amount of IR the TU contains.

### JIT infrastructure (shared libraries)

Two shared libraries must be present on any host that runs a JIT-enabled binary.
Neither contributes bytes to the application binary itself, but both must be
installed on every deployment machine.

| Library | File size | `.text` | `.data` + `.bss` | Purpose |
|---|---|---|---|---|
| `libClangRuntimeSpecializer.so` | 0.5 MB | 294 KB | 8 KB | Our JIT engine (specializer pipeline, IR transform, API) |
| `libLLVM.so.21.1` | 112.1 MB | 82.2 MB | 1.1 MB | Full LLVM monolith (codegen, ORC JIT, optimisers, parsers) |
| **Total** | **112.6 MB** | | | |

`libLLVM.so` dominates at 112 MB. It is the monolithic LLVM shared library and
includes substantially more than the ORC JIT subsystem used at runtime — all
codegen backends, all IR analysis passes, and all LLVM toolchain support are
bundled in the same `.so`. In practice, LLVM is already present on many
development and build machines (compilers, debuggers, and other toolchain
components depend on it), but a minimal deployment environment that does not
otherwise use LLVM must provision all 112 MB.

### Use-case benchmark binaries

The six use-case benchmarks (UC1–UC14) each follow the same structure: 10
compiled TUs per binary — nine kernel TUs (3 variants × 3 abstraction levels)
and one benchmark-registration TU.

| Binary | Total size | IR dump (`.rodata`) | Non-dump remainder | Dump fraction |
|---|---|---|---|---|
| UC1 SQL Predicate | 12.3 MB | 8.6 MB | 3.6 MB | 69% |
| UC2 Convolution | 12.2 MB | 8.6 MB | 3.6 MB | 70% |
| UC7 DFA Regex | 12.0 MB | 8.3 MB | 3.6 MB | 69% |
| UC8 IVM | 12.5 MB | 8.7 MB | 3.7 MB | 70% |
| UC12 Group-By | 12.6 MB | 8.8 MB | 3.8 MB | 69% |
| UC14 Sort | 12.4 MB | 8.6 MB | 3.8 MB | 69% |

Per-TU breakdown for a representative binary (UC1):

| TU type | # TUs | Blob per TU | Subtotal |
|---|---|---|---|
| Kernel TUs (3 variants × 3 levels) | 9 | ~950 KB | 8.4 MB |
| Benchmark registration TU | 1 | ~195 KB | 0.2 MB |
| **Total** | **10** | — | **8.6 MB** |

The kernel TU blobs are substantially uniform across use-cases (~950 KB each)
because each kernel TU contains the same support code (allocators, benchmark
helpers, C++ runtime pieces) reachable from the kernel functions; GlobalDCE
at JIT time prunes this down to the 5–15 functions that are actually needed for
specialization.

---

## Debug Symbols in the IR Dump

The LLVM bitcode format embeds debug metadata (source locations, variable
names, type annotations) inline with the IR instructions whenever the source is
compiled with `-g`. Because the IR dump captures the TU at the bitcode level,
this debug metadata is included in the blob and inflates its size substantially.

To quantify the effect, we compile the two LIT micro-kernel tests with and
without `-g` and compare `getModuleStats().BitcodeSizeBytes`:

| Kernel | Blob with `-g` | Blob without `-g` | Inflation factor | Debug fraction |
|---|---|---|---|---|
| Small (1 fn, 53 instrs) | 39 KB | 5.2 KB | **7.5×** | 87% |
| Large (4 fn, 333 instrs) | 48 KB | 10.6 KB | **4.5×** | 78% |

The debug fraction decreases with larger kernels (more actual IR dilutes the
fixed per-function debug overhead), but even for the large kernel **78% of the
blob is debug metadata**, not IR.

The use-case benchmark kernels are compiled with `-g -O3`. Applying the 4.5×
inflation factor measured for the large kernel as a conservative estimate, the
without-debug blob size for each UC TU would be roughly:

```
950 KB (measured, with -g)  ÷  4.5  ≈  210 KB  (estimated, without -g)
```

A UC binary without debug info in its blobs would therefore shrink from ~8.6 MB
to roughly ~2.1 MB in IR dump content.

**Debug symbols in the ELF binary itself** are a separate consideration.
The UC release binaries contain ~3.1 MB of ELF `.debug_*` sections from the
benchmark framework code (google benchmark, registration infrastructure). After
`strip`, the non-blob remainder shrinks from ~3.9 MB to ~0.6 MB. The blob
survives `strip` intact — it lives in `.rodata`, not a debug section, because it
is needed at runtime.

---

## Size Formula

The formula depends on whether the source is compiled with `-g` (debug info)
and whether the final binary is stripped. Throughout, **BINSIZE** refers to the
baseline binary compiled *without* the IR plugin — the binary that would exist
if JIT specialisation were not used — compiled and stripped to the same level as
the JIT-enabled binary.

### Measured: debug build (-g -O3), unstripped

The UC benchmarks as built include both ELF debug sections and debug metadata
inside the blobs. The blob is consistently 2.2–2.3× the unstripped non-blob
binary size:

| Binary | BINSIZE (no dump) | Blob | Ratio (blob / BINSIZE) |
|---|---|---|---|
| UC1 SQL Predicate | 3.88 MB | 8.62 MB | 2.22 |
| UC2 Convolution | 3.86 MB | 8.62 MB | 2.23 |
| UC7 DFA Regex | 3.82 MB | 8.38 MB | 2.19 |
| UC8 IVM | 3.94 MB | 8.79 MB | 2.23 |
| UC12 Group-By | 4.01 MB | 8.82 MB | 2.20 |
| UC14 Sort | 3.99 MB | 8.63 MB | 2.16 |

```
Binary size with JIT  ≈  3.2 × BINSIZE        [-g, unstripped]
Total storage         ≈  3.2 × BINSIZE  +  112 MB
```

### Estimated: release build (-O3, no -g), stripped

For a stripped production binary without debug info, the blob shrinks by ~4.5×
while the non-blob baseline shrinks to ~0.6 MB. Applying both adjustments:

```
Binary size with JIT  ≈  4.1 × BINSIZE        [-O3 only, stripped]
Total storage         ≈  4.1 × BINSIZE  +  112 MB
```

The coefficient is *higher* here because BINSIZE (the stripped baseline) is much
smaller — the blob is now the overwhelming contributor. The absolute binary size
is smaller (2.5 MB vs 12.5 MB) but the *multiplier* is larger because the
denominator shrank more than the blob did.

### Summary table

| Build scenario | BINSIZE proxy | Blob size | Formula |
|---|---|---|---|
| `-g -O3`, unstripped (as measured) | ~3.9 MB | ~8.6 MB | `3.2 × BINSIZE + 112 MB` |
| `-O3`, stripped (production) | ~0.6 MB | ~2.1 MB | `4.1 × BINSIZE + 112 MB` |

For N binaries on the same host, the per-binary term scales linearly; the 112 MB
shared library cost is paid once:

```
Total storage ≈  C × N × BINSIZE  +  112 MB
```

where C ≈ 3.2 (debug build) or C ≈ 4.1 (stripped release).

**Caveat.** The coefficients are empirical estimates derived from UC benchmark
kernels, which have a moderate amount of reachable support code per TU. TUs with
very little support code (like the LIT micro-kernels with no benchmark framework)
produce smaller blobs and lower coefficients. TUs that pull in large libraries
would produce larger coefficients. The formula is most reliable for this class of
use-case kernels.

---

## Conclusions

**The IR dump is the dominant binary size overhead.** Across all six use-case
benchmarks the IR dump accounts for 69–70% of the total release binary size.
The compiled application code (`.text`) accounts for only ~240 KB, meaning the
embedded bitcode is roughly 35× the size of the native code it represents.
This ratio is expected: LLVM bitcode is a typed, annotated intermediate
representation that retains type information, metadata, debug records, and
attribute strings that native code does not.

**The overhead is proportional to TU complexity, not just the target kernel.**
Each kernel TU's blob (~950 KB) is determined by the total IR that survives
compile-time GlobalDCE for that TU — including template instantiations and
runtime-support functions transitively reachable from the kernel. A simpler TU
with fewer reachable functions (like the 5.3 KB micro-kernel smoke test) produces
a proportionally smaller blob. Applications that place specialisation targets in
dedicated, minimal TUs can reduce per-binary overhead significantly.

**The JIT infrastructure is a fixed per-host cost, not a per-binary cost.**
The two required shared libraries (`libClangRuntimeSpecializer.so` at 0.5 MB and
`libLLVM.so` at 112 MB) must be present on every deployment machine but are
shared across all processes and binaries running on that host. The dominant item
is `libLLVM.so` at 112 MB, which is the full LLVM monolith and already a standard
dependency on most development and build systems. A stripped deployment
environment with no prior LLVM dependency must provision all 112 MB.

**Total storage footprint.** A system deploying one JIT-enabled use-case
benchmark for the first time requires:

| Component | Storage |
|---|---|
| Application binary (UC1, including IR dump) | 12.3 MB |
| `libClangRuntimeSpecializer.so` | 0.5 MB |
| `libLLVM.so.21.1` | 112.1 MB |
| **Total (first deployment)** | **~125 MB** |

For each additional JIT-enabled binary added to the same host, only the per-binary
cost recurs (~12 MB per UC binary); the shared libraries are already present.

**The trade-off.** The per-binary cost is dominated by the IR dump, which
increases binary size by approximately 2.4× (from ~3.6 MB to ~12.3 MB). The
per-host infrastructure cost is dominated by `libLLVM.so` at 112 MB, which is
substantial on a pristine system but negligible on any machine already using LLVM
toolchain components. Whether the combined overhead is acceptable depends on
deployment context: server-side or desktop environments typically tolerate it
without difficulty, while embedded or storage-constrained targets may not.
The overhead could in principle be reduced by deploying IR blobs as separate
sidecar files loaded on demand, and by linking only the required LLVM JIT
subsystems rather than the full monolith — neither is implemented in the current
system.
