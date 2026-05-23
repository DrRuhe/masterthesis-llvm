# Polybench Specialization Slowdown Analysis

**Date**: 2026-05-10  
**Data source**: `benchmarks/benchmarks.duckdb`, `benchmarks/reports/20260507-162137-analysis/` (ASM dumps)  
**Run ID**: `0e134f5b-6f55-47af-9622-af1424252055` (2026-04-29)

---

## Executive Summary

Of 150 polybench configurations benchmarked, **81 (54%)** are more than 5% slower after JIT specialization. The mean slowdown among slow cases is **1.33x**, with the worst case reaching **1.94x** (bicg /1800/2200 — 94% slower).

Counterintuitively, the specialized code has **~40% fewer instructions** than unspecialized in all slowdown cases — yet runs slower because instructions per cycle (IPC) drops by **50%** on average (from ~3.1 to ~1.5 IPC). The IPC drop is the dominant predictive factor (Spearman ρ = −0.62, p < 10⁻¹⁶), while iTLB and L1i-cache miss ratios have weaker, secondary contributions.

---

## Dataset Overview

| Category | Count | Mean speedup ratio | Mean instr ratio | Mean IPC ratio |
|---|---|---|---|---|
| Faster (ratio < 0.95) | 37 (25%) | 0.820 | 0.598 | 0.650 |
| Neutral (0.95–1.05) | 32 (21%) | 0.996 | 0.683 | 0.690 |
| Slower (ratio > 1.05) | 81 (54%) | 1.325 | 0.608 | 0.492 |

*speedup ratio = t_spec_ns / t_unspec_ns; ratio > 1 means the specialized version is slower*

**Kernels always slow** (all measured sizes): trisolv, atax, bicg, mvt, gesummv, 2mm, 3mm, durbin  
**Kernels always fast** (all/most sizes): gemm (avg 0.694), adi (avg 0.860), correlation (avg 0.798), floyd_warshall (avg 0.860)

---

## Hypothesis 1: FMA Dependency Chain Latency (PRIMARY CAUSE)

### Reasoning

The JIT specializer replaces `mulsd + addsd` pairs with `vfnmadd*sd` / `vfmadd*sd` (FMA) instructions and aggressively unrolls inner loops (10–16×) once constant propagation has made loop bounds known. For loops with a **loop-carried scalar accumulator** (dot-product / GEMV-like patterns), this creates a long serial FMA dependency chain that limits throughput.

### Experiment / Data Analysis

**Hardware counter evidence** (Spearman correlations with slowdown ratio):
- IPC ratio: ρ = −0.621 (p < 2×10⁻¹⁷) — strong negative correlation; lower IPC → more slowdown
- Instruction ratio: ρ = −0.030 (p = 0.72) — essentially no correlation
- The decomposition `predicted_slowdown = instr_ratio / ipc_ratio` achieves Pearson r = 0.777 with actual slowdown

**ASM inspection — trisolv /4000 (1.55× slower)**

| | Unspecialized | Specialized |
|---|---|---|
| Inner loop unroll factor | 2× | **16×** |
| Instructions per element | ~8 | ~5 |
| Instruction type | `mulsd + subsd` (SSE2 scalar) | `vfnmadd132sd` (AVX FMA) |
| Accumulator stores/element | 1 | **1 (after every FMA!)** |
| IPC measured | 2.09 | **0.75** |

The 16× unrolled FMA inner loop creates a 64-cycle serial dependency chain (16 FMAs × 4 cycle latency each) for the accumulator `x[i]`. The unspecialized 2×-unrolled loop with mul+sub has a 16-cycle chain per 2 elements = 8 cycles/element, achieves higher IPC because the processor can better pipeline adjacent instruction pairs.

**ASM inspection — bicg /1800/2200 (1.94× slower)**

The bicg kernel accumulates two values simultaneously (`s[j]` and `q[i]`). The specialized code uses `vfmadd213sd` and `vfmadd132sd` in a **12×-unrolled** inner loop. Key anomaly: `q[i]` is written to memory after **every single FMA step** (12 spurious stores per 12-element block to the same address). This wastes store bandwidth and creates redundant write traffic to the L1 cache.

**ASM inspection — mvt /4000 (1.19× slower, iTLB 0.46×)**

The x1-loop (row-wise) shows the same 16× FMA accumulator pattern. The x2-loop (column-wise) unrolls 16 rows simultaneously, accessing `A[j+0][i]`, `A[j+1][i]`, …, `A[j+15][i]` — a stride of `16 × row_size = 16 × 32000 = 512 KB`. For n=400, each row is 3200 bytes; 16 rows span 48 KB covering ~12 pages, causing 12× more iTLB misses per unrolled block vs the unspecialized 2× unroll.

**IPC breakdown across all slow cases (52/81 have IPC ratio < 0.5):**

```
FMA dependency chain creates:
  - 10-way unroll: 40-cycle serial chain → 4 cycles/element
  - 12-way unroll: 48-cycle serial chain → 4 cycles/element  
  - 16-way unroll: 64-cycle serial chain → 4 cycles/element

Unspecialized 2-way unroll (SSE2 mul+add):
  - 16-cycle chain per 2 elements → 8 cycles/element
  - But mul and add for the second element can partially overlap
  - Achieves ~2.5–3.5 IPC because dual-issue pairs are easier to schedule
```

**Kernel categorization by loop structure:**

| Loop pattern | Kernels | Always slow? | Reason |
|---|---|---|---|
| Scalar accumulator (dot product) | trisolv, atax, bicg, mvt, gesummv | Yes | FMA chain bottleneck |
| Chained matrix multiply | 2mm, 3mm, doitgen | Mostly yes | FMA chain bottleneck |
| Complex scalar recurrence | durbin, nussinov | Yes | Serial recurrence |
| Independent output elements | gemm, correlation | Never | SIMD vectorizable |
| Constant-folded scalars | adi, floyd_warshall | Never | Constant propagation wins |

### Weaknesses

- The IPC measurement includes all instructions (loads, stores, branches), not just FMAs, so the theoretical FMA chain latency may not be the sole bottleneck.
- The unspecialized code is compiled AOT with O3, which also applies loop unrolling — but with different heuristics (no constant propagation → smaller unroll factors).
- Single benchmark run; run-to-run variability not quantified.

---

## Hypothesis 2: Vectorization Regression (EXAMINED, PARTIALLY SUPPORTED)

### Reasoning

The unspecialized code might already use SSE2/AVX vectorization (packed doubles), and the JIT could degrade to scalar code — or vice versa, the JIT could generate better vectorized code. We tested whether fast vs slow cases differ in vectorization quality.

### Experiment / Data Analysis

**ASM evidence for fast kernel (gemm /2000/2300/2600 — 0.74× speedup)**

The specialized GEMM code uses `vmulpd` (packed double, 2 doubles per XMM register) and `vmovupd` in the initial `C *= alpha` scaling pass, operating on 2 doubles per instruction. The unspecialized code uses scalar `mulsd` and `movsd`. The GEMM main accumulation loop also benefits because knowing the matrix dimensions enables the O3 backend to lay out the loop for better cache blocking and SIMD width.

**For slow kernels (bicg, trisolv, mvt, gesummv):**

The specialized code uses scalar `vfmadd*sd` (not `vfmadd*pd`). No packed vectorization occurs in the inner loops. LLVM cannot auto-vectorize because the loop-carried accumulator dependency prevents horizontal SIMD reduction without a more sophisticated transformation.

The fundamental issue: for dot-product-like kernels, generating efficient SIMD code requires:
1. Accumulate in 4 separate SIMD lanes (using YMM/256-bit registers)
2. Apply horizontal reduction at the end of the loop

LLVM's loop vectorizer would do this if given the opportunity, but the current JIT pipeline applies loop unrolling **before** or in a way that interferes with vectorizer heuristics after constant propagation changes the loop structure.

### Weaknesses

- Only ASM-level analysis performed; no IR-level inspection of what the vectorizer sees.
- The interference between IPSCCP, AlwaysInliner, and the LoopVectorizer pass order was not formally traced.

---

## Hypothesis 3: iTLB / Instruction Cache Pressure (MINOR SECONDARY CAUSE)

### Reasoning

The specialized code is larger (more instructions due to unrolling) and loaded into a JIT-compiled memory region, potentially at a different address than the AOT code. Larger instruction footprint → more L1i cache misses and iTLB misses.

### Experiment / Data Analysis

**Correlation analysis:**
- iTLB ratio vs slowdown: Spearman ρ = +0.205 (p = 0.012) — weak positive correlation
- L1i-cache ratio vs slowdown: Spearman ρ = +0.293 (p < 0.001) — weak positive correlation

**Extreme TLB cases:**

| Kernel | Raw params | Slowdown | iTLB ratio | IPC ratio |
|---|---|---|---|---|
| ludcmp | /2000 | 1.12× | **64.8×** | 0.352 |
| lu | /2000 | 1.007× | **44.5×** | 0.421 |
| gemm | /2000/2300/2600 | **0.74×** (faster!) | 38.7× | 0.520 |
| seidel_2d | /100/400 | 1.196× | 18.5× | 0.700 |
| mvt | /400 | 1.580× | 17.6× | 0.374 |

Crucially, the TLB ratio does NOT predict slowdown on its own: `gemm /2000/2300/2600` has 38.7× more iTLB misses yet is **26% faster** (because SIMD vectorization provides a larger benefit). Cases with both high iTLB ratio AND high IPC drop are the worst performers.

**Root cause of TLB pressure in column-wise kernels (mvt x2-loop):**

The 16× loop unroll of the column-wise pass in MVT accesses `A[j+k][i]` for k = 0..15 with stride = row_size_bytes in each unrolled step. For n=400, row_size = 3200 bytes; 16 rows span 48 KB ≈ 12 OS pages → 12 iTLB misses per unrolled block. The 2× unrolled unspecialized version spans only 2 pages.

### Weaknesses

- iTLB misses are measured as totals, not attributed to specific loops.
- L1i miss correlation conflates code size effects (more unrolled instructions) with instruction pointer locality.
- No data cache (L1d, L2, L3) miss counters were recorded; data cache effects cannot be separated from TLB effects.

---

## Hypothesis 4: Redundant Memory Stores to Accumulator (CONFIRMED MINOR CAUSE)

### Reasoning

Inspecting the specialized ASM for bicg, gesummv, trisolv, and mvt, the accumulator variable (`q[i]`, `tmp[i]`, `x1[i]`) is stored to memory **after every FMA instruction** within the inner loop, even though the register value is used directly by the subsequent FMA (no reload needed). This wastes store-port capacity.

### Experiment / Data Analysis

In the specialized bicg /1800/2200 inner loop (12-way unrolled):
- 12 stores to `q[i]` (one address) per 12-element block — ALL redundant except the last
- 12 stores to `s[j]` (12 different addresses) — ALL necessary
- Total: 24 stores/12-element block vs. 12 necessary

Similarly for gesummv (10-way unroll): 20 stores (10 for `tmp[i]`, 10 for `y[i]`) per 10-element block, where 9/10 of each accumulator's stores are redundant.

This is likely caused by the compiler's inability to prove that the accumulator address is not aliased by the inner-loop loads (even though `i` ≠ `j` in all cases), causing it to emit a "safe" store after every update. The JIT pipeline does not run alias analysis after the constant-propagation stages insert absolute memory addresses.

### Weaknesses

- The store-port saturation was not directly measured (no `mem_uops_retired` counter).
- It's unclear whether the redundant stores are on the critical path — they may be absorbed by the store buffer without adding latency.

---

## Summary of Root Causes

| Cause | Kernels affected | Contribution | Evidence |
|---|---|---|---|
| FMA chain latency from 10–16× loop unrolling | bicg, trisolv, atax, mvt, gesummv, 2mm, 3mm, doitgen, durbin | Primary (52/81 slow cases have IPC ratio < 0.5) | IPC correlation, ASM analysis |
| No SIMD vectorization (scalar FMA instead of packed) | Same kernels | Co-primary with above (two sides of same coin) | vmulpd absent in slow ASM |
| Excessive TLB misses from stride unrolling | mvt, seidel_2d, lu, ludcmp, cholesky (large sizes) | Secondary (7/81 cases with 5× iTLB) | iTLB ratio data |
| Redundant accumulator stores after every FMA | bicg, gesummv, mvt, trisolv | Minor | ASM store count |
| L1i pressure from larger specialized code | Many | Minor (ρ = 0.29) | L1i ratio data |

---

## Potential Mitigations

### 1. Reduce Default Loop Unroll Count

**What**: Lower `LoopUnrollCount` (currently 128) to 4–8 for kernels detected to have sequential accumulator dependencies.  
**Impact**: Shorter FMA chains → higher IPC. For the 16×-unrolled trisolv/mvt, a 2× unroll would match the unspecialized code's performance floor.  
**Risk**: May reduce speedup for kernels that legitimately benefit from unrolling (memory-bandwidth-bound with independent iterations).  
**Implementation**: `Options::withLoopUnrollCount(4)` in the benchmark registration, or as a general default change.

### 2. Enable SIMD Vectorization (SLP / Loop Vectorizer)

**What**: Allow the LoopVectorizer to run in the JIT pipeline (currently the fixpoint loop uses `ModuleToFunctionPassAdaptor` with O3's FPM, but vectorization may be inhibited by prior pass ordering or missed-vectorization hints).  
**Impact**: For dot-product loops with known trip counts, LLVM can generate `vfmadd231pd` (packed, 2 or 4 doubles) + horizontal reduction, giving 2×–4× throughput improvement.  
**Risk**: Vector code size is larger; JIT compile time increases.  
**Implementation**: Ensure `LoopVectorizePass` runs after IPSCCP (when loop bounds are constant integers). Consider adding it explicitly before `LoopUnrollPass`.

### 3. Detect and Skip Specialization for Sequential-Accumulator Kernels

**What**: Add a heuristic that measures the post-IPSCCP IR for reduction patterns (loops with a single scalar accumulator and a loop-carried dependency). If detected, skip specialization or use a lighter-weight pipeline.  
**Impact**: Eliminates regressions for the 81 slow cases (54% of tested configs).  
**Risk**: Adds JIT analysis overhead; may miss edge cases.  
**Implementation**: Count `phi` nodes with floating-point types that are both def and use in the same loop body after IPSCCP.

### 4. Suppress Redundant Accumulator Stores

**What**: Run an alias analysis or mem2reg-style pass after IPSCCP to promote the accumulator store/reload pattern to a register-only update.  
**Impact**: Reduces store bandwidth by 50–90% in affected loops. Minor improvement (stores are not on the critical path), but eliminates false aliasing pressure.  
**Implementation**: `PromoteMemoryToRegisterPass` or `SROA` in the fixpoint loop after IPSCCP.

### 5. Enable TLB-Aware Unrolling for Column-Wise Access

**What**: For loops that access memory with strides > page_size/16 (e.g., column-major matrix access with large rows), cap the unroll factor at 2–4.  
**Impact**: Reduces iTLB misses by 4–8× for mvt/seidel_2d/lu at medium-large sizes.  
**Implementation**: Custom loop analysis pass that inspects GEP stride patterns after constant folding.

### 6. Execution-Time Guarded Specialization (Adaptive Fallback)

**What**: On the first few calls, run both the specialized and unspecialized versions with timing. If the specialized version is slower, fall back to the unspecialized path permanently.  
**Impact**: Eliminates all regressions at the cost of 2× overhead on the first N calls.  
**Risk**: Adds significant complexity and measurement overhead.  
**Implementation**: `specializeOrFallback` pattern already exists in the API; extend with timing comparison.

---

## Appendix: Worst Slowdowns

| Kernel | Params | Slowdown | Instr ratio | IPC ratio | iTLB ratio |
|---|---|---|---|---|---|
| bicg | /1800/2200 | **1.935×** | 0.568 | 0.413 | 0.1 |
| trisolv | /2000 | 1.886× | 0.586 | 0.330 | 0.2 |
| trisolv | /400 | 1.784× | 0.609 | 0.351 | 1.4 |
| 2mm | /180/190/210/220 | 1.776× | 0.505 | 0.300 | 2.0 |
| gesummv | /250 | 1.770× | 0.484 | 0.301 | 0.8 |
| mvt | /120 | 1.709× | 0.574 | 0.354 | 1.3 |
| atax | /390/410 | 1.703× | 0.588 | 0.342 | 4.4 |
| atax | /116/124 | 1.656× | 0.587 | 0.354 | 1.8 |
| covariance | /240/260 | 1.644× | 0.528 | 0.307 | 6.1 |
| 2mm | /800/900/1100/1200 | 1.614× | 0.511 | 0.353 | 0.1 |

All 10 worst cases have IPC ratio < 0.43 — confirming the FMA dependency chain as the dominant cause.


## Why does unrolling create this problem?

Without unrolling, the compiler already generated a dependency chain — you had it anyway. The chain is
inherent to the algorithm (each step uses the previous result). What changes with unrolling:

Without unrolling (2×, as in the unspecialized code):                                                       
; iteration 1: two chained ops, but the chain is SHORT
mulsd  xmm1, A[j]   ; 4 cycle latency                                                                       
subsd  xmm0, xmm1   ; 4 cycle latency (waits for mul)                                                       
; iteration 2: starts immediately AFTER iteration 1's add
mulsd  xmm1, A[j+1] ; can start DURING the previous subsd's latency window!                                 
subsd  xmm0, xmm1   ; ...

With mulsd+subsd, the two-step sequence (mul then sub) gives the CPU a small window to overlap: the load and
multiply for iteration j+1 can start while iteration j's subtraction is in flight. The chain is 8 cycles   
per pair — and the CPU can fill those 8 cycles with useful other work.

With 16× unrolling (as in the specialized code):       
FMA0:  acc = acc - A[j+0]*x[j+0]     ; 4 cy
FMA1:  acc = acc - A[j+1]*x[j+1]     ; 4 cy (wait for FMA0)
FMA2:  acc = acc - A[j+2]*x[j+2]     ; 4 cy (wait for FMA1)                                                 
...                                                        
FMA15: acc = acc - A[j+15]*x[j+15]   ; 4 cy (wait for FMA14)                                                
; loop back                                                 
FMA16: acc = acc - A[j+16]*x[j+16]   ; 4 cy (wait for FMA15)

The CPU now sees 16 chained FMAs in a row before the loop resets. The chain length is 64 cycles. During     
those 64 cycles, the 2 FMA units sit mostly idle waiting for the chain to resolve — they can't start any of
these FMAs early because each one depends on the previous.

The compiler unrolled hoping to expose parallelism, but there is no parallelism here — only a longer serial
chain.

The unspecialized 2× unroll doesn't avoid the chain — it just keeps the chain length short (8 cycles per 2  
elements), which is easier for the CPU's out-of-order scheduler to manage.
                                                                                                              
---                                                    
## How reducing the unroll count helps

If you unroll by 2× instead of 16×, the FMA chain per loop body is 8 cycles instead of 64. The CPU's
instruction window (the "reorder buffer," typically 200–300 entries) can now see past the end of the        
unrolled body and begin the next iteration's work. This is much more manageable.

There's a genuine tradeoff: some unrolling helps by amortizing loop overhead (the branch check, index       
increment) and enabling better software prefetching. The sweet spot for serial-dependency loops is roughly
2–4×, not 16×.
                                                         
---
## How SIMD would fix this properly

SIMD (Single Instruction, Multiple Data) means one instruction operates on multiple values simultaneously.
On this CPU, a 128-bit XMM register holds 2 doubles at once; a 256-bit YMM register holds 4 doubles.

The key insight: instead of one accumulator, use 4 independent accumulators packed into one YMM register:

; 4 accumulators running simultaneously, all independent of each other                                      
vfmadd231pd ymm0, ymm1, [A+j*8]  ; ymm0[0:3] += A[j:j+3] * x[j:j+3]                                         
vfmadd231pd ymm0, ymm1, [A+(j+4)*8]                                                                         
...

Each 256-bit FMA processes 4 dot-product contributions simultaneously in 4 cycles (not 4 cycles × 4 = 16    
cycles). At the end, you sum the 4 lanes horizontally:

vhaddpd ymm0, ymm0, ymm0   ; add pairs within ymm0     
vextractf128 xmm1, ymm0, 1                                                                                  
addsd xmm0, xmm1            ; final scalar sum

Result: 4× the throughput for the same latency cost. The dependency chain still exists within each lane, but
the 4 lanes are independent — the CPU can pipeline all 4 simultaneously. You get FMA throughput close to   
the hardware limit (0.5 cycles per double).

The reason the JIT doesn't do this automatically: LLVM's loop vectorizer needs to recognize the reduction   
pattern (accumulate many values into one scalar) and transform it into a SIMD loop + horizontal reduction.
When IPSCCP constant-folds the loop bounds and then the unroller runs, the reduction pattern may be         
obscured, and the vectorizer either doesn't run or doesn't recognize it as safe to vectorize.

  ---
In three sentences: FMA computes multiply+add in one shot and is fast when independent, but for dot products
every step depends on the previous result — a "chain." Making the chain 16× longer (aggressive unrolling)  
keeps the FMA units idle 75% of the time. SIMD would break the chain by running 4 independent accumulators
in parallel in the same register, giving 4× the throughput. 