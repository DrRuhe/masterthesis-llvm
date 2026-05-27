<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ32create_url_match_low_specializedvE3$_0JPKclEEET_PT0_DpT1_>:
	xorl	%eax, %eax
	testq	%rsi, %rsi
	jle	110
	movabsq	$106629574028864, %rdx
	xorl	%r9d, %r9d
	xorl	%ecx, %ecx
	xorl	%r8d, %r8d
	nopl	(%rax)
	movl	%r9d, %r10d
	movzbl	(%rdi,%r8), %r9d
	testb	%r9b, %r9b
	cmovlel	%eax, %r9d
	movzbl	%r9b, %r9d
	movl	%r10d, %r11d
	shll	$7, %r11d
	orl	%r9d, %r11d
	movslq	%r11d, %r9
	movl	(%rdx,%r9,4), %r9d
	cmpl	$7, %r9d
	sete	%r11b
	cmpl	$7, %r10d
	setne	%r10b
	andb	%r11b, %r10b
	movzbl	%r10b, %r10d
	addq	%r10, %rcx
	incq	%r8
	cmpq	%r8, %rsi
	jne	-70
	xorl	%eax, %eax
	cmpl	$7, %r9d
	sete	%al
	addq	%rax, %rcx
	movq	%rcx, %rax
	retq
