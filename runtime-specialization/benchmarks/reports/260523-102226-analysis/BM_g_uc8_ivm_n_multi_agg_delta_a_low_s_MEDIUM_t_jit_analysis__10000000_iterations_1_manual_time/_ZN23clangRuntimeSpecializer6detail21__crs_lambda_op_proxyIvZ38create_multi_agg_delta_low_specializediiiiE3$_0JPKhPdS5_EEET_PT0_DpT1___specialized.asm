<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ38create_multi_agg_delta_low_specializediiiiE3$_0JPKhPdS5_EEET_PT0_DpT1_>:
	leaq	-7(%rip), %rax
	movabsq	$-8192, %rcx
	addq	%rax, %rcx
	movl	$1023, %eax
	andl	4(%rdi), %eax
	vmovsd	8(%rdi), %xmm0
	shll	$3, %eax
	vaddsd	(%rsi,%rax), %xmm0, %xmm0
	vmovsd	%xmm0, (%rsi,%rax)
	vmovsd	(%rdx,%rax), %xmm0
	movabsq	$-416, %rsi
	vaddsd	(%rcx,%rsi), %xmm0, %xmm0
	vmovsd	%xmm0, (%rdx,%rax)
	retq
