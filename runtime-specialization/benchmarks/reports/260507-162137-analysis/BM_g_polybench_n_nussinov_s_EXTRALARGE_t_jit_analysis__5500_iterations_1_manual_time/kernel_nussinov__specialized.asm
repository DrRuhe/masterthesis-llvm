	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	leaq	-7(%rip), %rax
	movabsq	$-7994, %rcx
	addq	%rax, %rcx
	movl	$4294967295, %r15d
	movabsq	$104, %rax
	movq	(%rcx,%rax), %rax
	movq	(%rax), %rax
	leaq	121044000(%rax), %rcx
	movq	%rcx, -24(%rsp)
	leaq	121000000(%rax), %rsi
	leaq	121022000(%rax), %rcx
	movq	%rcx, -32(%rsp)
	leaq	121022004(%rax), %rcx
	movq	%rcx, -40(%rsp)
	leaq	121022008(%rax), %rcx
	movq	%rcx, -48(%rsp)
	movq	%rax, -16(%rsp)
	addq	$121022012, %rax
	movq	%rax, -56(%rsp)
	movl	$5500, %eax
	movq	%rax, -72(%rsp)
	movq	$0, -80(%rsp)
	movl	$5501, %ebx
	jmp	89
	nopw	%cs:(%rax,%rax)
	movq	-80(%rsp), %rax
	incl	%eax
	movq	%rax, -80(%rsp)
	decq	%rbx
	addq	$-22004, -24(%rsp)
	addq	$-22004, %rsi
	addq	$-22000, -32(%rsp)
	addq	$-22000, -40(%rsp)
	addq	$-22000, -48(%rsp)
	addq	$-22000, -56(%rsp)
	cmpq	$2, -88(%rsp)
	jb	858
	movq	-72(%rsp), %rcx
	leaq	-1(%rcx), %rax
	movq	%rax, -72(%rsp)
	movq	%rcx, -88(%rsp)
	cmpq	$5499, %rcx
	ja	-107
	movq	-80(%rsp), %rax
	movl	%eax, %edx
	movq	-88(%rsp), %r12
	andl	$3, %edx
	je	192
	xorl	%edi, %edi
	movq	-24(%rsp), %r8
	movq	-88(%rsp), %r12
	jmp	19
	nop
	incq	%r12
	incl	%edi
	addq	$4, %r8
	cmpl	%edx, %edi
	je	159
	leal	(%r12,%r15), %r9d
	imulq	$22000, -72(%rsp), %rcx
	movq	-16(%rsp), %r11
	addq	%r11, %rcx
	movl	(%rcx,%r12,4), %r10d
	movl	(%rcx,%r9,4), %eax
	cmpl	%eax, %r10d
	cmovgl	%r10d, %eax
	movq	-88(%rsp), %r14
	imulq	$22000, %r14, %r10
	addq	%r11, %r10
	movl	(%r10,%r12,4), %r11d
	cmpl	%r11d, %eax
	cmovlel	%r11d, %eax
	movl	%eax, (%rcx,%r12,4)
	movl	(%r10,%r9,4), %r9d
	cmpl	%r9d, %eax
	cmovlel	%r9d, %eax
	movl	%eax, (%rcx,%r12,4)
	cmpq	%r12, %r14
	jae	-103
	leaq	(%rcx,%r12,4), %r9
	movq	%rsi, %rcx
	movq	%r8, %r10
	movq	%rbx, %r11
	nopw	%cs:(%rax,%rax)
	movl	(%r10), %ebp
	addl	(%rcx), %ebp
	cmpl	%ebp, %eax
	cmovlel	%ebp, %eax
	movl	%eax, (%r9)
	movl	%r11d, %r14d
	incq	%r11
	addq	$22000, %r10
	addq	$4, %rcx
	cmpq	%r12, %r14
	jne	-35
	jmp	-168
	nopl	(%rax,%rax)
	movq	-80(%rsp), %rax
	decl	%eax
	cmpl	$3, %eax
	jb	-336
	movq	-32(%rsp), %rax
	leaq	(%rax,%r12,4), %r13
	movq	-40(%rsp), %rax
	leaq	(%rax,%r12,4), %rbp
	movq	-48(%rsp), %rax
	leaq	(%rax,%r12,4), %rdi
	movq	-56(%rsp), %rax
	leaq	(%rax,%r12,4), %r8
	imulq	$22000, -72(%rsp), %r9
	movq	-16(%rsp), %rax
	addq	%rax, %r9
	imulq	$22000, -88(%rsp), %rcx
	addq	%rax, %rcx
	movq	%rcx, -64(%rsp)
	jmp	53
	nopl	(%rax,%rax)
	movq	-8(%rsp), %r13
	addq	$16, %r13
	addq	$16, %rbp
	addq	$16, %rdi
	addq	$16, %r8
	cmpq	$5496, %r12
	leaq	4(%r12), %r12
	movl	$4294967295, %r15d
	jae	-461
	leal	(%r12,%r15), %edx
	movl	(%r9,%r12,4), %r11d
	movl	(%r9,%rdx,4), %eax
	cmpl	%eax, %r11d
	cmovgl	%r11d, %eax
	movq	-64(%rsp), %rcx
	movl	(%rcx,%r12,4), %r11d
	cmpl	%r11d, %eax
	cmovlel	%r11d, %eax
	movl	%eax, (%r9,%r12,4)
	movl	(%rcx,%rdx,4), %edx
	cmpl	%edx, %eax
	cmovlel	%edx, %eax
	movl	%eax, (%r9,%r12,4)
	cmpq	%r12, -88(%rsp)
	movq	%r13, -8(%rsp)
	jae	68
	leaq	(%r9,%r12,4), %r14
	xorl	%r15d, %r15d
	nopw	%cs:(%rax,%rax)
	leal	(%rbx,%r15), %ecx
	movl	(%r13), %edx
	addl	(%rsi,%r15,4), %edx
	cmpl	%edx, %eax
	cmovgl	%eax, %edx
	movl	%edx, (%r14)
	incq	%r15
	addq	$22000, %r13
	movl	%edx, %eax
	cmpq	%r12, %rcx
	jne	-37
	jmp	11
	nopw	(%rax,%rax)
	movl	%eax, %edx
	movl	4(%r9,%r12,4), %eax
	cmpl	%edx, %eax
	cmovgl	%eax, %edx
	movq	-64(%rsp), %rax
	movl	4(%rax,%r12,4), %eax
	cmpl	%eax, %edx
	cmovlel	%eax, %edx
	cmpl	%r11d, %edx
	cmovlel	%r11d, %edx
	movl	%edx, 4(%r9,%r12,4)
	cmpq	%r12, -88(%rsp)
	ja	68
	leaq	1(%r12), %r11
	leaq	(%r9,%r12,4), %r14
	addq	$4, %r14
	movq	%rbp, %r15
	xorl	%r13d, %r13d
	nopw	%cs:(%rax,%rax)
	leal	(%rbx,%r13), %r10d
	movl	(%r15), %ecx
	addl	(%rsi,%r13,4), %ecx
	cmpl	%ecx, %edx
	cmovlel	%ecx, %edx
	movl	%edx, (%r14)
	incq	%r13
	addq	$22000, %r15
	cmpq	%r11, %r10
	jne	-34
	leaq	2(%r12), %r14
	movl	8(%r9,%r12,4), %ecx
	cmpl	%edx, %ecx
	cmovgl	%ecx, %edx
	movq	-64(%rsp), %rcx
	movl	8(%rcx,%r12,4), %r11d
	cmpl	%r11d, %edx
	cmovlel	%r11d, %edx
	cmpl	%eax, %edx
	cmovlel	%eax, %edx
	movl	%edx, 8(%r9,%r12,4)
	cmpq	%r14, -88(%rsp)
	jae	64
	leaq	(%r9,%r12,4), %rax
	addq	$8, %rax
	movq	%rdi, %r15
	xorl	%r13d, %r13d
	nopw	%cs:(%rax,%rax)
	leal	(%rbx,%r13), %ecx
	movl	(%r15), %r10d
	addl	(%rsi,%r13,4), %r10d
	cmpl	%r10d, %edx
	cmovlel	%r10d, %edx
	movl	%edx, (%rax)
	incq	%r13
	addq	$22000, %r15
	cmpq	%r14, %rcx
	jne	-35
	leaq	3(%r12), %rax
	movl	12(%r9,%r12,4), %ecx
	cmpl	%edx, %ecx
	cmovgl	%ecx, %edx
	movq	-64(%rsp), %rcx
	movl	12(%rcx,%r12,4), %ecx
	cmpl	%ecx, %edx
	cmovlel	%ecx, %edx
	cmpl	%r11d, %edx
	cmovlel	%r11d, %edx
	movl	%edx, 12(%r9,%r12,4)
	cmpq	%rax, -88(%rsp)
	jae	-456
	leaq	(%r9,%r12,4), %r11
	addq	$12, %r11
	movq	%r8, %r14
	xorl	%r15d, %r15d
	nopw	%cs:(%rax,%rax)
	leal	(%rbx,%r15), %ecx
	movl	(%r14), %r10d
	addl	(%rsi,%r15,4), %r10d
	cmpl	%r10d, %edx
	cmovlel	%r10d, %edx
	movl	%edx, (%r11)
	incq	%r15
	addq	$22000, %r14
	cmpq	%rax, %rcx
	jne	-36
	jmp	-521
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
