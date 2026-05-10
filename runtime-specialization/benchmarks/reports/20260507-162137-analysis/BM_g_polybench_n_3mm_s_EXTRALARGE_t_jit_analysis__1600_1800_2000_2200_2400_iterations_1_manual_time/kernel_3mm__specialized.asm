	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	leaq	-7(%rip), %rax
	movabsq	$-7993, %rdx
	addq	%rax, %rdx
	movabsq	$48, %rax
	movq	(%rdx,%rax), %rax
	movq	(%rax), %rax
	movabsq	$40, %rcx
	movq	(%rdx,%rcx), %rcx
	movq	(%rcx), %r8
	movabsq	$152, %rcx
	movq	(%rdx,%rcx), %rcx
	movq	(%rcx), %r9
	movabsq	$144, %rcx
	movq	(%rdx,%rcx), %rcx
	movq	(%rcx), %rcx
	movabsq	$88, %rsi
	movq	(%rdx,%rsi), %rsi
	movq	(%rsi), %rsi
	movabsq	$192, %rdi
	movq	(%rdx,%rdi), %rdi
	movq	(%rdi), %rdi
	movabsq	$168, %r10
	movq	(%rdx,%r10), %rdx
	movq	(%rdx), %rdx
	xorl	%r10d, %r10d
	nopw	(%rax,%rax)
	imulq	$14400, %r10, %r11
	addq	%rax, %r11
	movq	%r9, %rbx
	xorl	%r14d, %r14d
	leaq	(%r11,%r14,8), %r15
	movq	$0, (%r11,%r14,8)
	vxorpd	%xmm0, %xmm0, %xmm0
	movl	$15, %r12d
	movq	%rbx, %r13
	nopl	(%rax)
	vmovsd	-120(%r8,%r12,8), %xmm1
	vfmadd132sd	(%r13), %xmm0, %xmm1
	vmovsd	%xmm1, (%r15)
	vmovsd	-112(%r8,%r12,8), %xmm0
	vfmadd132sd	14400(%r13), %xmm1, %xmm0
	vmovsd	%xmm0, (%r15)
	vmovsd	-104(%r8,%r12,8), %xmm1
	vfmadd132sd	28800(%r13), %xmm0, %xmm1
	vmovsd	%xmm1, (%r15)
	vmovsd	-96(%r8,%r12,8), %xmm0
	vfmadd132sd	43200(%r13), %xmm1, %xmm0
	vmovsd	%xmm0, (%r15)
	vmovsd	-88(%r8,%r12,8), %xmm1
	vfmadd132sd	57600(%r13), %xmm0, %xmm1
	vmovsd	%xmm1, (%r15)
	vmovsd	-80(%r8,%r12,8), %xmm0
	vfmadd132sd	72000(%r13), %xmm1, %xmm0
	vmovsd	%xmm0, (%r15)
	vmovsd	-72(%r8,%r12,8), %xmm1
	vfmadd132sd	86400(%r13), %xmm0, %xmm1
	vmovsd	%xmm1, (%r15)
	vmovsd	-64(%r8,%r12,8), %xmm0
	vfmadd132sd	100800(%r13), %xmm1, %xmm0
	vmovsd	%xmm0, (%r15)
	vmovsd	-56(%r8,%r12,8), %xmm1
	vfmadd132sd	115200(%r13), %xmm0, %xmm1
	vmovsd	%xmm1, (%r15)
	vmovsd	-48(%r8,%r12,8), %xmm0
	vfmadd132sd	129600(%r13), %xmm1, %xmm0
	vmovsd	%xmm0, (%r15)
	vmovsd	-40(%r8,%r12,8), %xmm1
	vfmadd132sd	144000(%r13), %xmm0, %xmm1
	vmovsd	%xmm1, (%r15)
	vmovsd	-32(%r8,%r12,8), %xmm0
	vfmadd132sd	158400(%r13), %xmm1, %xmm0
	vmovsd	%xmm0, (%r15)
	vmovsd	-24(%r8,%r12,8), %xmm1
	vfmadd132sd	172800(%r13), %xmm0, %xmm1
	vmovsd	%xmm1, (%r15)
	vmovsd	-16(%r8,%r12,8), %xmm0
	vfmadd132sd	187200(%r13), %xmm1, %xmm0
	vmovsd	%xmm0, (%r15)
	vmovsd	-8(%r8,%r12,8), %xmm1
	vfmadd132sd	201600(%r13), %xmm0, %xmm1
	vmovsd	%xmm1, (%r15)
	vmovsd	(%r8,%r12,8), %xmm0
	vfmadd132sd	216000(%r13), %xmm1, %xmm0
	vmovsd	%xmm0, (%r15)
	addq	$230400, %r13
	addq	$16, %r12
	cmpq	$2015, %r12
	jne	-356
	incq	%r14
	addq	$8, %rbx
	cmpq	$1800, %r14
	jne	-408
	incq	%r10
	addq	$16000, %r8
	cmpq	$1600, %r10
	jne	-447
	xorl	%r8d, %r8d
	nopw	%cs:(%rax,%rax)
	imulq	$17600, %r8, %r9
	addq	%rcx, %r9
	movq	%rdi, %r10
	xorl	%r11d, %r11d
	leaq	(%r9,%r11,8), %rbx
	movq	$0, (%r9,%r11,8)
	vxorpd	%xmm0, %xmm0, %xmm0
	movl	$15, %r14d
	movq	%r10, %r15
	nopl	(%rax)
	vmovsd	-120(%rsi,%r14,8), %xmm1
	vfmadd132sd	(%r15), %xmm0, %xmm1
	vmovsd	%xmm1, (%rbx)
	vmovsd	-112(%rsi,%r14,8), %xmm0
	vfmadd132sd	17600(%r15), %xmm1, %xmm0
	vmovsd	%xmm0, (%rbx)
	vmovsd	-104(%rsi,%r14,8), %xmm1
	vfmadd132sd	35200(%r15), %xmm0, %xmm1
	vmovsd	%xmm1, (%rbx)
	vmovsd	-96(%rsi,%r14,8), %xmm0
	vfmadd132sd	52800(%r15), %xmm1, %xmm0
	vmovsd	%xmm0, (%rbx)
	vmovsd	-88(%rsi,%r14,8), %xmm1
	vfmadd132sd	70400(%r15), %xmm0, %xmm1
	vmovsd	%xmm1, (%rbx)
	vmovsd	-80(%rsi,%r14,8), %xmm0
	vfmadd132sd	88000(%r15), %xmm1, %xmm0
	vmovsd	%xmm0, (%rbx)
	vmovsd	-72(%rsi,%r14,8), %xmm1
	vfmadd132sd	105600(%r15), %xmm0, %xmm1
	vmovsd	%xmm1, (%rbx)
	vmovsd	-64(%rsi,%r14,8), %xmm0
	vfmadd132sd	123200(%r15), %xmm1, %xmm0
	vmovsd	%xmm0, (%rbx)
	vmovsd	-56(%rsi,%r14,8), %xmm1
	vfmadd132sd	140800(%r15), %xmm0, %xmm1
	vmovsd	%xmm1, (%rbx)
	vmovsd	-48(%rsi,%r14,8), %xmm0
	vfmadd132sd	158400(%r15), %xmm1, %xmm0
	vmovsd	%xmm0, (%rbx)
	vmovsd	-40(%rsi,%r14,8), %xmm1
	vfmadd132sd	176000(%r15), %xmm0, %xmm1
	vmovsd	%xmm1, (%rbx)
	vmovsd	-32(%rsi,%r14,8), %xmm0
	vfmadd132sd	193600(%r15), %xmm1, %xmm0
	vmovsd	%xmm0, (%rbx)
	vmovsd	-24(%rsi,%r14,8), %xmm1
	vfmadd132sd	211200(%r15), %xmm0, %xmm1
	vmovsd	%xmm1, (%rbx)
	vmovsd	-16(%rsi,%r14,8), %xmm0
	vfmadd132sd	228800(%r15), %xmm1, %xmm0
	vmovsd	%xmm0, (%rbx)
	vmovsd	-8(%rsi,%r14,8), %xmm1
	vfmadd132sd	246400(%r15), %xmm0, %xmm1
	vmovsd	%xmm1, (%rbx)
	vmovsd	(%rsi,%r14,8), %xmm0
	vfmadd132sd	264000(%r15), %xmm1, %xmm0
	vmovsd	%xmm0, (%rbx)
	addq	$281600, %r15
	addq	$16, %r14
	cmpq	$2415, %r14
	jne	-339
	incq	%r11
	addq	$8, %r10
	cmpq	$2200, %r11
	jne	-391
	incq	%r8
	addq	$19200, %rsi
	cmpq	$1800, %r8
	jne	-430
	xorl	%esi, %esi
	imulq	$17600, %rsi, %rdi
	addq	%rdx, %rdi
	movq	%rcx, %r8
	xorl	%r9d, %r9d
	leaq	(%rdi,%r9,8), %r10
	movq	$0, (%rdi,%r9,8)
	vxorpd	%xmm0, %xmm0, %xmm0
	movl	$17, %r11d
	movq	%r8, %rbx
	nopl	(%rax)
	vmovsd	-136(%rax,%r11,8), %xmm1
	vfmadd132sd	(%rbx), %xmm0, %xmm1
	vmovsd	%xmm1, (%r10)
	vmovsd	-128(%rax,%r11,8), %xmm0
	vfmadd132sd	17600(%rbx), %xmm1, %xmm0
	vmovsd	%xmm0, (%r10)
	vmovsd	-120(%rax,%r11,8), %xmm1
	vfmadd132sd	35200(%rbx), %xmm0, %xmm1
	vmovsd	%xmm1, (%r10)
	vmovsd	-112(%rax,%r11,8), %xmm0
	vfmadd132sd	52800(%rbx), %xmm1, %xmm0
	vmovsd	%xmm0, (%r10)
	vmovsd	-104(%rax,%r11,8), %xmm1
	vfmadd132sd	70400(%rbx), %xmm0, %xmm1
	vmovsd	%xmm1, (%r10)
	vmovsd	-96(%rax,%r11,8), %xmm0
	vfmadd132sd	88000(%rbx), %xmm1, %xmm0
	vmovsd	%xmm0, (%r10)
	vmovsd	-88(%rax,%r11,8), %xmm1
	vfmadd132sd	105600(%rbx), %xmm0, %xmm1
	vmovsd	%xmm1, (%r10)
	vmovsd	-80(%rax,%r11,8), %xmm0
	vfmadd132sd	123200(%rbx), %xmm1, %xmm0
	vmovsd	%xmm0, (%r10)
	vmovsd	-72(%rax,%r11,8), %xmm1
	vfmadd132sd	140800(%rbx), %xmm0, %xmm1
	vmovsd	%xmm1, (%r10)
	vmovsd	-64(%rax,%r11,8), %xmm0
	vfmadd132sd	158400(%rbx), %xmm1, %xmm0
	vmovsd	%xmm0, (%r10)
	vmovsd	-56(%rax,%r11,8), %xmm1
	vfmadd132sd	176000(%rbx), %xmm0, %xmm1
	vmovsd	%xmm1, (%r10)
	vmovsd	-48(%rax,%r11,8), %xmm0
	vfmadd132sd	193600(%rbx), %xmm1, %xmm0
	vmovsd	%xmm0, (%r10)
	vmovsd	-40(%rax,%r11,8), %xmm1
	vfmadd132sd	211200(%rbx), %xmm0, %xmm1
	vmovsd	%xmm1, (%r10)
	vmovsd	-32(%rax,%r11,8), %xmm0
	vfmadd132sd	228800(%rbx), %xmm1, %xmm0
	vmovsd	%xmm0, (%r10)
	vmovsd	-24(%rax,%r11,8), %xmm1
	vfmadd132sd	246400(%rbx), %xmm0, %xmm1
	vmovsd	%xmm1, (%r10)
	vmovsd	-16(%rax,%r11,8), %xmm0
	vfmadd132sd	264000(%rbx), %xmm1, %xmm0
	vmovsd	%xmm0, (%r10)
	vmovsd	-8(%rax,%r11,8), %xmm1
	vfmadd132sd	281600(%rbx), %xmm0, %xmm1
	vmovsd	%xmm1, (%r10)
	vmovsd	(%rax,%r11,8), %xmm0
	vfmadd132sd	299200(%rbx), %xmm1, %xmm0
	vmovsd	%xmm0, (%r10)
	addq	$18, %r11
	addq	$316800, %rbx
	cmpq	$1817, %r11
	jne	-400
	incq	%r9
	addq	$8, %r8
	cmpq	$2200, %r9
	jne	-452
	incq	%rsi
	addq	$14400, %rax
	cmpq	$1600, %rsi
	jne	-490
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	retq
