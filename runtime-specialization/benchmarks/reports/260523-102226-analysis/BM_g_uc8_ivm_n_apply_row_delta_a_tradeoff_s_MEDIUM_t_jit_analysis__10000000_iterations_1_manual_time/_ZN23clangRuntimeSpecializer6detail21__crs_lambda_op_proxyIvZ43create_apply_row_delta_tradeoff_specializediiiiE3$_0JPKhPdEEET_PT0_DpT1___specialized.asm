<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ43create_apply_row_delta_tradeoff_specializediiiiE3$_0JPKhPdEEET_PT0_DpT1_>:
	movl	$1023, %eax
	andl	4(%rdi), %eax
	vmovsd	8(%rdi), %xmm0
	vaddsd	(%rsi,%rax,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rsi,%rax,8)
	retq
