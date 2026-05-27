<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ38create_multi_predicate_low_specializediiiddE3$_0JPKhlEEET_PT0_DpT1_>:
	leaq	-7(%rip), %rax
	movabsq	$-8120, %rcx
	addq	%rax, %rcx
	testq	%rsi, %rsi
	jle	71
	movabsq	$-568, %rdx
	cmpq	$1, %rsi
	jne	58
	xorl	%eax, %eax
	xorl	%r8d, %r8d
	testb	$1, %sil
	je	46
	movl	$16, %esi
	imulq	%rsi, %r8
	movl	$8, %esi
	addq	%r8, %rdi
	vmovsd	(%rsi,%rdi), %xmm0
	vucomisd	(%rcx,%rdx), %xmm0
	jbe	17
	vmovsd	(%rdi), %xmm0
	vucomisd	(%rcx,%rdx), %xmm0
	jbe	6
	incq	%rax
	retq
