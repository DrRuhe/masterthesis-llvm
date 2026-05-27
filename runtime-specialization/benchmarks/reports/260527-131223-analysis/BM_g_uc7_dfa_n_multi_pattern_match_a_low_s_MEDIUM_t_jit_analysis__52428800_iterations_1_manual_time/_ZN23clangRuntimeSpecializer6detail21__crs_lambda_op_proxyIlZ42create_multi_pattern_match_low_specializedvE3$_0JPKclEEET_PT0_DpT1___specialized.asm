<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ42create_multi_pattern_match_low_specializedvE3$_0JPKclEEET_PT0_DpT1_>:
	xorl	%ecx, %ecx
	testq	%rsi, %rsi
	jle	114
	movabsq	$102438775565888, %rdx
	xorl	%r10d, %r10d
	xorl	%eax, %eax
	xorl	%r8d, %r8d
	jmp	16
	nopl	(%rax,%rax)
	incq	%r8
	movl	%r9d, %r10d
	cmpq	%r8, %rsi
	je	62
	movzbl	(%rdi,%r8), %r9d
	testb	%r9b, %r9b
	cmovlel	%ecx, %r9d
	movzbl	%r9b, %r9d
	movl	%r10d, %r11d
	shll	$7, %r11d
	orl	%r9d, %r11d
	movslq	%r11d, %r9
	movl	(%rdx,%r9,4), %r9d
	cmpl	$4, %r9d
	je	6
	cmpl	$2, %r9d
	jne	-56
	cmpl	$2, %r10d
	je	-62
	cmpl	$4, %r10d
	je	-68
	incq	%rax
	jmp	-73
	cmpl	$4, %r9d
	je	6
	cmpl	$2, %r9d
	jne	3
	incq	%rax
	retq
