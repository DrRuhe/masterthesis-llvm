<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ40create_generic_sort_tradeoff_specializediE3$_0JPvlEEET_PT0_DpT1_>:
	cmpq	$2, %rsi
	jl	1155
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$1096, %rsp
	movq	%rdi, %rbx
	movabsq	$94265977504064, %r13
	decq	%rsi
	movq	$0, 64(%rsp)
	movq	%rsi, 72(%rsp)
	leaq	8(%rdi), %rax
	movq	%rax, 40(%rsp)
	movl	$1, %ecx
	jmp	22
	nopl	(%rax)
	movq	48(%rsp), %rax
	movl	%eax, %ecx
	testl	%eax, %eax
	jle	1053
	leal	-1(%rcx), %eax
	movq	%rax, %r15
	shlq	$4, %r15
	movq	64(%rsp,%r15), %rbp
	movq	72(%rsp,%r15), %r12
	movq	%r12, %rdx
	subq	%rbp, %rdx
	jle	-38
	cmpq	$15, %rdx
	movq	%rbp, 24(%rsp)
	movq	%r12, 32(%rsp)
	jg	177
	movq	%rax, 48(%rsp)
	movq	%rdx, 56(%rsp)
	movq	%rbp, (%rsp)
	andq	$3, %rdx
	movq	%rdx, 16(%rsp)
	je	452
	movq	40(%rsp), %rax
	leaq	(%rax,%rbp,8), %rax
	movq	%rax, 8(%rsp)
	movq	%rbp, (%rsp)
	xorl	%r12d, %r12d
	jmp	38
	nopw	%cs:(%rax,%rax)
	incq	%r12
	addq	$8, 8(%rsp)
	cmpq	16(%rsp), %r12
	movq	24(%rsp), %rbp
	je	391
	movq	(%rsp), %rax
	cmpq	%rbp, %rax
	leaq	1(%rax), %rax
	movq	%rax, (%rsp)
	jl	-42
	movq	8(%rsp), %r14
	movq	(%rsp), %rbp
	nopw	%cs:(%rax,%rax)
	leaq	-8(%r14), %r15
	movq	%r15, %rdi
	movq	%r14, %rsi
	callq	*%r13
	testl	%eax, %eax
	jle	-81
	decq	%rbp
	movq	-8(%r14), %rax
	movq	(%r14), %rcx
	movq	%rcx, -8(%r14)
	movq	%rax, (%r14)
	movq	%r15, %r14
	cmpq	24(%rsp), %rbp
	jg	-44
	jmp	-110
	nop
	movq	%rcx, 8(%rsp)
	shrq	%rdx
	addq	%rbp, %rdx
	leaq	(%rbx,%rbp,8), %rdi
	leaq	(%rbx,%rdx,8), %r14
	movq	%rdi, (%rsp)
	movq	%r14, %rsi
	callq	*%r13
	testl	%eax, %eax
	jle	16
	movq	(%rsp), %rdx
	movq	(%rdx), %rax
	movq	(%r14), %rcx
	movq	%rcx, (%rdx)
	movq	%rax, (%r14)
	leaq	(%rbx,%r12,8), %r12
	movq	(%rsp), %rdi
	movq	%r12, %rsi
	callq	*%r13
	testl	%eax, %eax
	jle	18
	movq	(%rsp), %rdx
	movq	(%rdx), %rax
	movq	(%r12), %rcx
	movq	%rcx, (%rdx)
	movq	%rax, (%r12)
	movq	%r14, %rdi
	movq	%r12, %rsi
	callq	*%r13
	movq	%r14, %rsi
	testl	%eax, %eax
	jle	14
	movq	(%rsi), %rax
	movq	(%r12), %rcx
	movq	%rcx, (%rsi)
	movq	%rax, (%r12)
	leaq	(%rsp,%r15), %rax
	addq	$64, %rax
	movq	%rax, 16(%rsp)
	movq	32(%rsp), %rdx
	leaq	-1(%rdx), %r12
	leaq	-8(%rbx,%rdx,8), %r14
	movq	(%rsi), %rax
	movq	-8(%rbx,%rdx,8), %rcx
	movq	%rcx, (%rsi)
	movq	%rax, -8(%rbx,%rdx,8)
	movq	%rbp, %rdx
	nopw	%cs:(%rax,%rax)
	leaq	8(,%rdx,8), %rbp
	incq	%rdx
	movq	%rdx, %r15
	nop
	leaq	(%rbx,%rbp), %rdi
	movq	%r14, %rsi
	callq	*%r13
	incq	%r15
	addq	$8, %rbp
	testl	%eax, %eax
	js	-21
	leaq	-1(%r15), %rax
	movq	%rax, (%rsp)
	leaq	-8(,%r12,8), %rbp
	nopw	%cs:(%rax,%rax)
	decq	%r12
	leaq	(%rbx,%rbp), %rdi
	movq	%r14, %rsi
	callq	*%r13
	addq	$-8, %rbp
	testl	%eax, %eax
	jg	-21
	movq	-8(%rbx,%r15,8), %rax
	movq	(%rsp), %rdx
	cmpq	%r12, %rdx
	jge	409
	movq	(%rbx,%r12,8), %rcx
	movq	%rcx, -8(%rbx,%r15,8)
	movq	%rax, (%rbx,%r12,8)
	jmp	-118
	nopw	%cs:(%rax,%rax)
	movq	56(%rsp), %rax
	decq	%rax
	cmpq	$3, %rax
	jb	-562
	movq	40(%rsp), %rax
	movq	(%rsp), %r12
	leaq	(%rax,%r12,8), %rax
	movq	%rax, 16(%rsp)
	leaq	(%rbx,%r12,8), %rax
	movq	%rax, 8(%rsp)
	jmp	24
	nop
	addq	$32, 16(%rsp)
	addq	$32, 8(%rsp)
	cmpq	32(%rsp), %r12
	je	-615
	movq	%r12, %rax
	incq	%r12
	movq	%rax, (%rsp)
	cmpq	%rbp, %rax
	jl	54
	movq	16(%rsp), %r14
	movq	%r12, %rbp
	nop
	leaq	-8(%r14), %r15
	movq	%r15, %rdi
	movq	%r14, %rsi
	callq	*%r13
	testl	%eax, %eax
	jle	27
	decq	%rbp
	movq	-8(%r14), %rax
	movq	(%r14), %rcx
	movq	%rcx, -8(%r14)
	movq	%rax, (%r14)
	movq	%r15, %r14
	cmpq	24(%rsp), %rbp
	jg	-44
	movq	(%rsp), %rax
	leaq	2(%rax), %r15
	movq	24(%rsp), %rbp
	cmpq	%rbp, %r12
	jl	61
	movq	8(%rsp), %r14
	movq	%r15, %r12
	nopw	%cs:(%rax,%rax)
	leaq	8(%r14), %rdi
	leaq	16(%r14), %rsi
	callq	*%r13
	testl	%eax, %eax
	jle	28
	decq	%r12
	movq	8(%r14), %rax
	movq	16(%r14), %rcx
	movq	%rcx, 8(%r14)
	movq	%rax, 16(%r14)
	addq	$-8, %r14
	cmpq	%rbp, %r12
	jg	-43
	movq	(%rsp), %rax
	leaq	3(%rax), %r14
	cmpq	%rbp, %r15
	jl	51
	movq	8(%rsp), %r15
	movq	%r14, %r12
	leaq	16(%r15), %rdi
	leaq	24(%r15), %rsi
	callq	*%r13
	testl	%eax, %eax
	jle	28
	decq	%r12
	movq	16(%r15), %rax
	movq	24(%r15), %rcx
	movq	%rcx, 16(%r15)
	movq	%rax, 24(%r15)
	addq	$-8, %r15
	cmpq	%rbp, %r12
	jg	-43
	movq	(%rsp), %r12
	addq	$4, %r12
	cmpq	%rbp, %r14
	jl	-252
	movq	8(%rsp), %r14
	movq	%r12, %r15
	nopw	%cs:(%rax,%rax)
	leaq	24(%r14), %rdi
	leaq	32(%r14), %rsi
	callq	*%r13
	testl	%eax, %eax
	jle	-291
	decq	%r15
	movq	24(%r14), %rax
	movq	32(%r14), %rcx
	movq	%rcx, 24(%r14)
	movq	%rax, 32(%r14)
	addq	$-8, %r14
	cmpq	%rbp, %r15
	jg	-47
	jmp	-324
	nopw	%cs:(%rax,%rax)
	movq	32(%rsp), %rdi
	movq	-8(%rbx,%rdi,8), %rcx
	movq	%rcx, -8(%rbx,%r15,8)
	movq	%rax, -8(%rbx,%rdi,8)
	leaq	-2(%r15), %rax
	movq	%rax, %rcx
	movq	24(%rsp), %rsi
	subq	%rsi, %rcx
	movq	%rdi, %rdx
	subq	%r15, %rdx
	cmpq	%rdx, %rcx
	jle	44
	movq	16(%rsp), %rcx
	movq	%rax, 8(%rcx)
	cmpq	%r15, %rdi
	movq	8(%rsp), %rcx
	jle	68
	movl	%ecx, %eax
	incl	%ecx
	shlq	$4, %rax
	movq	%r15, 64(%rsp,%rax)
	movq	%rdi, 72(%rsp,%rax)
	movl	%ecx, %eax
	jmp	-1013
	movq	16(%rsp), %rcx
	movq	%r15, (%rcx)
	cmpq	%rax, %rsi
	movq	8(%rsp), %rdx
	jge	18
	movl	%edx, %ecx
	incl	%edx
	shlq	$4, %rcx
	movq	%rsi, 64(%rsp,%rcx)
	movq	%rax, 72(%rsp,%rcx)
	movl	%edx, %eax
	jmp	-1056
	movl	%ecx, %eax
	jmp	-1063
	addq	$1096, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
