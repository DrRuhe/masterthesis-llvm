<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ47create_count_matching_rows_abstract_specializediidE3$_0JPKhlEEET_PT0_DpT1_>:
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$72, %rsp
	leaq	-7(%rip), %rax
	movabsq	$-8054, %rcx
	addq	%rax, %rcx
	movabsq	$40, %rax
	movq	(%rcx,%rax), %rax
	addq	$16, %rax
	movq	%rax, 8(%rsp)
	movabsq	$140724238851600, %rax
	movq	(%rax), %rax
	movq	%rax, 16(%rsp)
	movabsq	$4602678819172646912, %rax
	movq	%rax, 24(%rsp)
	testq	%rsi, %rsi
	jle	26
	movq	%rdi, %rbx
	movl	%esi, %r15d
	andl	$7, %r15d
	cmpq	$8, %rsi
	jae	18
	xorl	%eax, %eax
	xorl	%r12d, %r12d
	jmp	693
	xorl	%r12d, %r12d
	jmp	731
	movq	%r15, 40(%rsp)
	addq	$-8, %rsi
	cmpq	$8, %rsi
	movq	%rsi, 56(%rsp)
	movq	%rbx, 48(%rsp)
	jae	10
	xorl	%eax, %eax
	xorl	%r12d, %r12d
	jmp	407
	shrq	$3, %rsi
	incq	%rsi
	andq	$-2, %rsi
	leaq	128(%rbx), %r14
	xorl	%eax, %eax
	xorl	%r12d, %r12d
	nopl	(%rax,%rax)
	movq	%rsi, 64(%rsp)
	movq	%rax, 32(%rsp)
	leaq	-128(%r14), %rsi
	movq	8(%rsp), %rax
	leaq	8(%rsp), %rbx
	movq	%rbx, %rdi
	callq	*(%rax)
	movzbl	%al, %ebp
	addq	%r12, %rbp
	leaq	-112(%r14), %rsi
	movq	8(%rsp), %rax
	movq	%rbx, %rdi
	callq	*(%rax)
	movzbl	%al, %r15d
	leaq	-96(%r14), %rsi
	movq	8(%rsp), %rax
	movq	%rbx, %rdi
	callq	*(%rax)
	movzbl	%al, %r12d
	addq	%r15, %r12
	addq	%rbp, %r12
	leaq	-80(%r14), %rsi
	movq	8(%rsp), %rax
	movq	%rbx, %rdi
	callq	*(%rax)
	movzbl	%al, %r15d
	leaq	-64(%r14), %rsi
	movq	8(%rsp), %rax
	movq	%rbx, %rdi
	callq	*(%rax)
	movzbl	%al, %r13d
	addq	%r15, %r13
	leaq	-48(%r14), %rsi
	movq	8(%rsp), %rax
	movq	%rbx, %rdi
	callq	*(%rax)
	movzbl	%al, %ebp
	addq	%r13, %rbp
	addq	%r12, %rbp
	leaq	-32(%r14), %rsi
	movq	8(%rsp), %rax
	movq	%rbx, %rdi
	callq	*(%rax)
	movzbl	%al, %r15d
	leaq	-16(%r14), %rsi
	movq	8(%rsp), %rax
	movq	%rbx, %rdi
	callq	*(%rax)
	movzbl	%al, %r12d
	addq	%r15, %r12
	movq	8(%rsp), %rax
	movq	%rbx, %rdi
	movq	%r14, %rsi
	callq	*(%rax)
	movzbl	%al, %r15d
	addq	%r12, %r15
	leaq	16(%r14), %rsi
	movq	8(%rsp), %rax
	movq	%rbx, %rdi
	callq	*(%rax)
	movzbl	%al, %r12d
	addq	%r15, %r12
	addq	%rbp, %r12
	leaq	32(%r14), %rsi
	movq	8(%rsp), %rax
	movq	%rbx, %rdi
	callq	*(%rax)
	movzbl	%al, %r15d
	leaq	48(%r14), %rsi
	movq	8(%rsp), %rax
	movq	%rbx, %rdi
	callq	*(%rax)
	movzbl	%al, %r13d
	addq	%r15, %r13
	leaq	64(%r14), %rsi
	movq	8(%rsp), %rax
	movq	%rbx, %rdi
	callq	*(%rax)
	movzbl	%al, %r15d
	addq	%r13, %r15
	leaq	80(%r14), %rsi
	movq	8(%rsp), %rax
	movq	%rbx, %rdi
	callq	*(%rax)
	movzbl	%al, %r13d
	addq	%r15, %r13
	leaq	96(%r14), %rsi
	movq	8(%rsp), %rax
	movq	%rbx, %rdi
	callq	*(%rax)
	movzbl	%al, %r15d
	addq	%r13, %r15
	addq	%r12, %r15
	leaq	112(%r14), %rsi
	movq	8(%rsp), %rax
	movq	%rbx, %rdi
	callq	*(%rax)
	movq	64(%rsp), %rsi
	movzbl	%al, %r12d
	movq	32(%rsp), %rax
	addq	%r15, %r12
	addq	$16, %rax
	addq	$256, %r14
	addq	$-2, %rsi
	jne	-379
	testb	$8, 56(%rsp)
	movq	40(%rsp), %r15
	movq	48(%rsp), %rbx
	jne	222
	movq	%rax, %r15
	shlq	$4, %r15
	leaq	(%rbx,%r15), %rsi
	movq	%rax, 32(%rsp)
	movq	8(%rsp), %rax
	leaq	8(%rsp), %r14
	movq	%r14, %rdi
	callq	*(%rax)
	movzbl	%al, %r13d
	leaq	(%rbx,%r15), %rsi
	addq	$16, %rsi
	movq	8(%rsp), %rax
	movq	%r14, %rdi
	callq	*(%rax)
	movzbl	%al, %ebp
	addq	%r13, %rbp
	leaq	(%rbx,%r15), %rsi
	addq	$32, %rsi
	movq	8(%rsp), %rax
	movq	%r14, %rdi
	callq	*(%rax)
	movzbl	%al, %r13d
	addq	%rbp, %r13
	leaq	(%rbx,%r15), %rsi
	addq	$48, %rsi
	movq	8(%rsp), %rax
	movq	%r14, %rdi
	callq	*(%rax)
	movzbl	%al, %ebp
	addq	%r13, %rbp
	leaq	(%rbx,%r15), %rsi
	addq	$64, %rsi
	movq	8(%rsp), %rax
	movq	%r14, %rdi
	callq	*(%rax)
	movzbl	%al, %r13d
	addq	%rbp, %r13
	leaq	(%rbx,%r15), %rsi
	addq	$80, %rsi
	movq	8(%rsp), %rax
	movq	%r14, %rdi
	callq	*(%rax)
	movzbl	%al, %ebp
	addq	%r13, %rbp
	leaq	(%rbx,%r15), %rsi
	addq	$96, %rsi
	movq	8(%rsp), %rax
	movq	%r14, %rdi
	callq	*(%rax)
	movzbl	%al, %eax
	addq	%rbp, %rax
	addq	%rax, %r12
	leaq	(%rbx,%r15), %rsi
	addq	$112, %rsi
	movq	40(%rsp), %r15
	movq	8(%rsp), %rax
	movq	%r14, %rdi
	callq	*(%rax)
	movzbl	%al, %eax
	addq	%rax, %r12
	movq	32(%rsp), %rax
	addq	$8, %rax
	testq	%r15, %r15
	je	41
	shlq	$4, %rax
	addq	%rax, %rbx
	leaq	8(%rsp), %r14
	nop
	movq	8(%rsp), %rax
	movq	%r14, %rdi
	movq	%rbx, %rsi
	callq	*(%rax)
	movzbl	%al, %eax
	addq	%rax, %r12
	addq	$16, %rbx
	decq	%r15
	jne	-28
	movq	%r12, %rax
	addq	$72, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
