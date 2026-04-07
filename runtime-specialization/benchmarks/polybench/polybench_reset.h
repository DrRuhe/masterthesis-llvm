/* polybench_reset.h – include before each PolyBench kernel section to reset
 * all dimension and data-type macros so the next kernel's .h can define the
 * correct EXTRALARGE values from scratch.  No include guard – intentionally
 * re-includable.
 */

/* Dimension macros */
#undef N
#undef M
#undef NI
#undef NJ
#undef NK
#undef NL
#undef NM
#undef NR
#undef NQ
#undef NP
#undef W
#undef H
#undef TSTEPS
#undef TMAX
#undef NX
#undef NY

/* Parametric loop-bound aliases */
#undef _PB_N
#undef _PB_M
#undef _PB_NI
#undef _PB_NJ
#undef _PB_NK
#undef _PB_NL
#undef _PB_NM
#undef _PB_NR
#undef _PB_NQ
#undef _PB_NP
#undef _PB_W
#undef _PB_H
#undef _PB_TSTEPS
#undef _PB_TMAX
#undef _PB_NX
#undef _PB_NY

/* Data-type selection macros */
#undef DATA_TYPE_IS_INT
#undef DATA_TYPE_IS_FLOAT
#undef DATA_TYPE_IS_DOUBLE
#undef DATA_TYPE
#undef DATA_PRINTF_MODIFIER
#undef SCALAR_VAL
#undef SQRT_FUN
#undef EXP_FUN
#undef POW_FUN
