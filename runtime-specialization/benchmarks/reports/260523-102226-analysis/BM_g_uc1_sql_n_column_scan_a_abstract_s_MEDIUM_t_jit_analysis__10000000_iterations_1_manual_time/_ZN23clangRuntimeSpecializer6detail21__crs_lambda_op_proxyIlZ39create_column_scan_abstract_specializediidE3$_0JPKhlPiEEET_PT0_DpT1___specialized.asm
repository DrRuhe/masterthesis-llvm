<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ39create_column_scan_abstract_specializediidE3$_0JPKhlPiEEET_PT0_DpT1_>:
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$56, %rsp
	leaq	-7(%rip), %rax
	movabsq	$-8046, %rcx
	addq	%rax, %rcx
	movabsq	$48, %rax
	movq	(%rcx,%rax), %rax
	addq	$16, %rax
	movq	%rax, 16(%rsp)
	movq	%rdx, 24(%rsp)
	movq	$0, 32(%rsp)
	testq	%rsi, %rsi
	jle	42
	movq	%rsi, %rbx
	movl	%ebx, %r13d
	andl	$7, %r13d
	movabsq	$-688, %rdx
	cmpq	$8, %rsi
	jae	23
	xorl	%ebx, %ebx
	testq	%r13, %r13
	jne	393
	jmp	487
	xorl	%eax, %eax
	jmp	485
	movabsq	$9223372036854775800, %rax
	andq	%rax, %rbx
	movq	%rdi, 40(%rsp)
	leaq	120(%rdi), %r14
	movq	%rbx, %rbp
	negq	%rbp
	movl	$7, %r15d
	movq	%rcx, 48(%rsp)
	vmovsd	(%rcx,%rdx), %xmm1
	leaq	16(%rsp), %r12
	vmovsd	%xmm1, 8(%rsp)
	jmp	36
	nopw	%cs:(%rax,%rax)
	subq	$-128, %r14
	leaq	(%r15,%rbp), %rax
	addq	$8, %rax
	addq	$8, %r15
	cmpq	$7, %rax
	je	263
	vmovsd	-112(%r14), %xmm0
	vucomisd	%xmm1, %xmm0
	jbe	20
	leaq	-7(%r15), %rsi
	movq	16(%rsp), %rax
	movq	%r12, %rdi
	callq	*(%rax)
	vmovsd	8(%rsp), %xmm1
	vmovsd	-96(%r14), %xmm0
	vucomisd	%xmm1, %xmm0
	jbe	20
	leaq	-6(%r15), %rsi
	movq	16(%rsp), %rax
	movq	%r12, %rdi
	callq	*(%rax)
	vmovsd	8(%rsp), %xmm1
	vmovsd	-80(%r14), %xmm0
	vucomisd	%xmm1, %xmm0
	jbe	20
	leaq	-5(%r15), %rsi
	movq	16(%rsp), %rax
	movq	%r12, %rdi
	callq	*(%rax)
	vmovsd	8(%rsp), %xmm1
	vmovsd	-64(%r14), %xmm0
	vucomisd	%xmm1, %xmm0
	jbe	20
	leaq	-4(%r15), %rsi
	movq	16(%rsp), %rax
	movq	%r12, %rdi
	callq	*(%rax)
	vmovsd	8(%rsp), %xmm1
	vmovsd	-48(%r14), %xmm0
	vucomisd	%xmm1, %xmm0
	jbe	20
	leaq	-3(%r15), %rsi
	movq	16(%rsp), %rax
	movq	%r12, %rdi
	callq	*(%rax)
	vmovsd	8(%rsp), %xmm1
	vmovsd	-32(%r14), %xmm0
	vucomisd	%xmm1, %xmm0
	jbe	20
	leaq	-2(%r15), %rsi
	movq	16(%rsp), %rax
	movq	%r12, %rdi
	callq	*(%rax)
	vmovsd	8(%rsp), %xmm1
	vmovsd	-16(%r14), %xmm0
	vucomisd	%xmm1, %xmm0
	jbe	20
	leaq	-1(%r15), %rsi
	movq	16(%rsp), %rax
	movq	%r12, %rdi
	callq	*(%rax)
	vmovsd	8(%rsp), %xmm1
	vmovsd	(%r14), %xmm0
	vucomisd	%xmm1, %xmm0
	jbe	-265
	movq	16(%rsp), %rax
	movq	%r12, %rdi
	movq	%r15, %rsi
	callq	*(%rax)
	vmovsd	8(%rsp), %xmm1
	jmp	-289
	movq	48(%rsp), %rcx
	movq	40(%rsp), %rdi
	movabsq	$-688, %rdx
	testq	%r13, %r13
	je	99
	movq	%rbx, %rax
	shlq	$4, %rax
	leaq	(%rax,%rdi), %r15
	addq	$8, %r15
	shll	$4, %r13d
	xorl	%r12d, %r12d
	vmovsd	(%rcx,%rdx), %xmm1
	leaq	16(%rsp), %r14
	vmovsd	%xmm1, 8(%rsp)
	jmp	26
	nopw	%cs:(%rax,%rax)
	incq	%rbx
	addq	$16, %r12
	cmpq	%r12, %r13
	je	33
	vmovsd	(%r15,%r12), %xmm0
	vucomisd	%xmm1, %xmm0
	jbe	-24
	movq	16(%rsp), %rax
	movq	%r14, %rdi
	movq	%rbx, %rsi
	callq	*(%rax)
	vmovsd	8(%rsp), %xmm1
	jmp	-45
	movq	32(%rsp), %rax
	addq	$56, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
