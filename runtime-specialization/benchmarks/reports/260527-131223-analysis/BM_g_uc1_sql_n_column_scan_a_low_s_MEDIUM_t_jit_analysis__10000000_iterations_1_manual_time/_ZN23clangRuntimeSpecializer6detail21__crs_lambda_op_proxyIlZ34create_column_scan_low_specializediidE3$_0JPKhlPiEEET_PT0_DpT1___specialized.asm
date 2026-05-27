<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ34create_column_scan_low_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_>:
	leaq	-7(%rip), %rax
	movabsq	$-8112, %rcx
	addq	%rax, %rcx
	testq	%rsi, %rsi
	jle	63
	pushq	%rbx
	movabsq	$-576, %r8
	cmpq	$1, %rsi
	jne	49
	xorl	%eax, %eax
	xorl	%r9d, %r9d
	testb	$1, %sil
	je	33
	movl	$16, %esi
	addq	$8, %rdi
	imulq	%r9, %rsi
	vmovsd	(%rdi,%rsi), %xmm0
	vucomisd	(%rcx,%r8), %xmm0
	jbe	7
	movl	%r9d, (%rdx,%rax,4)
	incq	%rax
	popq	%rbx
	retq
