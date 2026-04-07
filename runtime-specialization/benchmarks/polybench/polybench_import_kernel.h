/* polybench_import_kernel.h – parameterized PolyBench kernel include.
 *
 * #include cannot appear inside a #define, so this helper file is the
 * closest equivalent.  Usage:
 *
 *   #define POLYBENCH_KERNEL      <token>        e.g. 2mm, correlation
 *   #define POLYBENCH_KERNEL_PATH <quoted-path>  e.g. "PolyBenchC-4.2.1-master/.../2mm.c"
 *   #define kernel_<token>        POLYBENCH_KERNEL_FN_IMPL
 *   #include "polybench_import_kernel.h"
 *   #undef kernel_<token>
 *
 * POLYBENCH_KERNEL_FN_IMPL acts as an indirection: after this file sets it to
 * __pb_<token>_kernel, references to kernel_<token> in the C source resolve
 * through two macro expansions to the renamed symbol.
 */

/* ── token-paste helpers ──────────────────────────────────────────────────── */
#define _PBIK_PASTE3(a, b, c)  a##b##c
#define _PBIK_PASTE3_(a, b, c) _PBIK_PASTE3(a, b, c)

/* ── rename kernel function via the fixed intermediate macro ──────────────── */
/* kernel_<K> → POLYBENCH_KERNEL_FN_IMPL → __pb_<K>_kernel               */
#define POLYBENCH_KERNEL_FN_IMPL _PBIK_PASTE3_(__pb_, POLYBENCH_KERNEL, _kernel)

/* ── reset dimension / data-type macros, then rename control-flow symbols ── */
#include "polybench_reset.h"
#define main        _PBIK_PASTE3_(__pb_, POLYBENCH_KERNEL, _main)
#define init_array  _PBIK_PASTE3_(__pb_, POLYBENCH_KERNEL, _init)
#define print_array _PBIK_PASTE3_(__pb_, POLYBENCH_KERNEL, _print)

/* ── include the kernel source ────────────────────────────────────────────── */
#include POLYBENCH_KERNEL_PATH

/* ── undo all renames ─────────────────────────────────────────────────────── */
#undef main
#undef init_array
#undef print_array
#undef POLYBENCH_KERNEL_FN_IMPL
#undef _PBIK_PASTE3
#undef _PBIK_PASTE3_
#undef POLYBENCH_KERNEL
#undef POLYBENCH_KERNEL_PATH
