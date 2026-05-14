	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%rbx
	leaq	-7(%rip), %rax
	movabsq	$-7990, %rcx
	addq	%rax, %rcx
	movabsq	$96, %rax
	movq	(%rcx,%rax), %rax
	movq	(%rax), %rax
	leaq	125440000(%rax), %rcx
	leaq	125417604(%rax), %rdx
	leaq	22400(%rax), %rsi
	xorl	%edi, %edi
	movq	%rax, %r8
	jmp	32
	nopw	(%rax,%rax)
	incq	%rdi
	addq	$22400, %r8
	cmpq	$5600, %rdi
	je	995
	leaq	(%rax,%rdi,4), %r9
	leaq	(%rdx,%rdi,4), %r10
	imulq	$22400, %rdi, %r11
	leaq	(%rax,%r11), %rbx
	addq	%rsi, %r11
	cmpq	%r10, %rax
	setb	%bpl
	cmpq	%rcx, %r9
	setb	%r14b
	cmpq	%r11, %rax
	setb	%r9b
	cmpq	%rcx, %rbx
	setb	%r10b
	testb	%r14b, %bpl
	jne	446
	andb	%r10b, %r9b
	jne	437
	movq	%rax, %r9
	xorl	%r10d, %r10d
	nopw	%cs:(%rax,%rax)
	imulq	$22400, %r10, %r11
	addq	%rax, %r11
	vpbroadcastd	(%r11,%rdi,4), %xmm0
	movl	$52, %r11d
	nopw	%cs:(%rax,%rax)
	vpaddd	-208(%r8,%r11,4), %xmm0, %xmm1
	vpaddd	-192(%r8,%r11,4), %xmm0, %xmm2
	vpminsd	-208(%r9,%r11,4), %xmm1, %xmm1
	vpminsd	-192(%r9,%r11,4), %xmm2, %xmm2
	vmovdqu	%xmm1, -208(%r9,%r11,4)
	vmovdqu	%xmm2, -192(%r9,%r11,4)
	vpaddd	-176(%r8,%r11,4), %xmm0, %xmm1
	vpaddd	-160(%r8,%r11,4), %xmm0, %xmm2
	vpminsd	-176(%r9,%r11,4), %xmm1, %xmm1
	vpminsd	-160(%r9,%r11,4), %xmm2, %xmm2
	vmovdqu	%xmm1, -176(%r9,%r11,4)
	vmovdqu	%xmm2, -160(%r9,%r11,4)
	vpaddd	-144(%r8,%r11,4), %xmm0, %xmm1
	vpaddd	-128(%r8,%r11,4), %xmm0, %xmm2
	vpminsd	-144(%r9,%r11,4), %xmm1, %xmm1
	vpminsd	-128(%r9,%r11,4), %xmm2, %xmm2
	vmovdqu	%xmm1, -144(%r9,%r11,4)
	vmovdqu	%xmm2, -128(%r9,%r11,4)
	vpaddd	-112(%r8,%r11,4), %xmm0, %xmm1
	vpaddd	-96(%r8,%r11,4), %xmm0, %xmm2
	vpminsd	-112(%r9,%r11,4), %xmm1, %xmm1
	vpminsd	-96(%r9,%r11,4), %xmm2, %xmm2
	vmovdqu	%xmm1, -112(%r9,%r11,4)
	vmovdqu	%xmm2, -96(%r9,%r11,4)
	vpaddd	-80(%r8,%r11,4), %xmm0, %xmm1
	vpaddd	-64(%r8,%r11,4), %xmm0, %xmm2
	vpminsd	-80(%r9,%r11,4), %xmm1, %xmm1
	vpminsd	-64(%r9,%r11,4), %xmm2, %xmm2
	vmovdqu	%xmm1, -80(%r9,%r11,4)
	vmovdqu	%xmm2, -64(%r9,%r11,4)
	vpaddd	-48(%r8,%r11,4), %xmm0, %xmm1
	vpaddd	-32(%r8,%r11,4), %xmm0, %xmm2
	vpminsd	-48(%r9,%r11,4), %xmm1, %xmm1
	vpminsd	-32(%r9,%r11,4), %xmm2, %xmm2
	vmovdqu	%xmm1, -48(%r9,%r11,4)
	vmovdqu	%xmm2, -32(%r9,%r11,4)
	vpaddd	-16(%r8,%r11,4), %xmm0, %xmm1
	vpaddd	(%r8,%r11,4), %xmm0, %xmm2
	vpminsd	-16(%r9,%r11,4), %xmm1, %xmm1
	vpminsd	(%r9,%r11,4), %xmm2, %xmm2
	vmovdqu	%xmm1, -16(%r9,%r11,4)
	vmovdqu	%xmm2, (%r9,%r11,4)
	addq	$56, %r11
	cmpq	$5652, %r11
	jne	-353
	incq	%r10
	addq	$22400, %r9
	cmpq	$5600, %r10
	jne	-408
	jmp	-525
	nopl	(%rax)
	movq	%rax, %r9
	xorl	%r10d, %r10d
	nopw	%cs:(%rax,%rax)
	imulq	$22400, %r10, %r11
	addq	%rax, %r11
	leaq	(%r11,%rdi,4), %r11
	movl	$15, %ebx
	nopw	%cs:(%rax,%rax)
	movl	-60(%r8,%rbx,4), %ebp
	addl	(%r11), %ebp
	movl	-60(%r9,%rbx,4), %r14d
	movl	-56(%r9,%rbx,4), %r15d
	cmpl	%ebp, %r14d
	cmovll	%r14d, %ebp
	movl	%ebp, -60(%r9,%rbx,4)
	movl	-56(%r8,%rbx,4), %ebp
	addl	(%r11), %ebp
	cmpl	%ebp, %r15d
	cmovll	%r15d, %ebp
	movl	%ebp, -56(%r9,%rbx,4)
	movl	-52(%r9,%rbx,4), %ebp
	movl	-52(%r8,%rbx,4), %r14d
	addl	(%r11), %r14d
	cmpl	%r14d, %ebp
	cmovll	%ebp, %r14d
	movl	%r14d, -52(%r9,%rbx,4)
	movl	-48(%r9,%rbx,4), %ebp
	movl	-48(%r8,%rbx,4), %r14d
	addl	(%r11), %r14d
	cmpl	%r14d, %ebp
	cmovll	%ebp, %r14d
	movl	%r14d, -48(%r9,%rbx,4)
	movl	-44(%r9,%rbx,4), %ebp
	movl	-44(%r8,%rbx,4), %r14d
	addl	(%r11), %r14d
	cmpl	%r14d, %ebp
	cmovll	%ebp, %r14d
	movl	%r14d, -44(%r9,%rbx,4)
	movl	-40(%r9,%rbx,4), %ebp
	movl	-40(%r8,%rbx,4), %r14d
	addl	(%r11), %r14d
	cmpl	%r14d, %ebp
	cmovll	%ebp, %r14d
	movl	%r14d, -40(%r9,%rbx,4)
	movl	-36(%r9,%rbx,4), %ebp
	movl	-36(%r8,%rbx,4), %r14d
	addl	(%r11), %r14d
	cmpl	%r14d, %ebp
	cmovll	%ebp, %r14d
	movl	%r14d, -36(%r9,%rbx,4)
	movl	-32(%r9,%rbx,4), %ebp
	movl	-32(%r8,%rbx,4), %r14d
	addl	(%r11), %r14d
	cmpl	%r14d, %ebp
	cmovll	%ebp, %r14d
	movl	%r14d, -32(%r9,%rbx,4)
	movl	-28(%r9,%rbx,4), %ebp
	movl	-28(%r8,%rbx,4), %r14d
	addl	(%r11), %r14d
	cmpl	%r14d, %ebp
	cmovll	%ebp, %r14d
	movl	%r14d, -28(%r9,%rbx,4)
	movl	-24(%r9,%rbx,4), %ebp
	movl	-24(%r8,%rbx,4), %r14d
	addl	(%r11), %r14d
	cmpl	%r14d, %ebp
	cmovll	%ebp, %r14d
	movl	%r14d, -24(%r9,%rbx,4)
	movl	-20(%r9,%rbx,4), %ebp
	movl	-20(%r8,%rbx,4), %r14d
	addl	(%r11), %r14d
	cmpl	%r14d, %ebp
	cmovll	%ebp, %r14d
	movl	%r14d, -20(%r9,%rbx,4)
	movl	-16(%r9,%rbx,4), %ebp
	movl	-16(%r8,%rbx,4), %r14d
	addl	(%r11), %r14d
	cmpl	%r14d, %ebp
	cmovll	%ebp, %r14d
	movl	%r14d, -16(%r9,%rbx,4)
	movl	-12(%r9,%rbx,4), %ebp
	movl	-12(%r8,%rbx,4), %r14d
	addl	(%r11), %r14d
	cmpl	%r14d, %ebp
	cmovll	%ebp, %r14d
	movl	%r14d, -12(%r9,%rbx,4)
	movl	-8(%r9,%rbx,4), %ebp
	movl	-8(%r8,%rbx,4), %r14d
	addl	(%r11), %r14d
	cmpl	%r14d, %ebp
	cmovll	%ebp, %r14d
	movl	%r14d, -8(%r9,%rbx,4)
	movl	-4(%r9,%rbx,4), %ebp
	movl	-4(%r8,%rbx,4), %r14d
	addl	(%r11), %r14d
	cmpl	%r14d, %ebp
	cmovll	%ebp, %r14d
	movl	%r14d, -4(%r9,%rbx,4)
	movl	(%r9,%rbx,4), %ebp
	movl	(%r8,%rbx,4), %r14d
	addl	(%r11), %r14d
	cmpl	%r14d, %ebp
	cmovll	%ebp, %r14d
	movl	%r14d, (%r9,%rbx,4)
	addq	$16, %rbx
	cmpq	$5615, %rbx
	jne	-414
	incq	%r10
	addq	$22400, %r9
	cmpq	$5600, %r10
	jne	-469
	jmp	-1018
	popq	%rbx
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
