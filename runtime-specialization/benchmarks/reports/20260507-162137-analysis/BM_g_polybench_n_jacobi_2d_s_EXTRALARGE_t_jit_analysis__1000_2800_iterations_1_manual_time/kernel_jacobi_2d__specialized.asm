	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	leaq	-7(%rip), %rax
	movabsq	$-7962, %r9
	addq	%rax, %r9
	movabsq	$168, %rax
	movq	(%r9,%rax), %rax
	movq	(%rax), %rax
	movabsq	$120, %rcx
	movq	(%r9,%rcx), %rcx
	movq	(%rcx), %rcx
	leaq	22408(%rax), %rdx
	leaq	62719992(%rax), %rsi
	leaq	62697592(%rax), %r8
	leaq	62719992(%rcx), %rdi
	leaq	22408(%rcx), %r10
	leaq	62697592(%rcx), %r11
	leaq	8(%rax), %rbx
	cmpq	%rsi, %r10
	setae	%r10b
	movq	%rbx, -8(%rsp)
	cmpq	%r11, %rbx
	setae	%sil
	orb	%r10b, %sil
	cmpq	%rdi, %rdx
	setae	%dl
	leaq	8(%rcx), %rdi
	movq	%rdi, -16(%rsp)
	cmpq	%r8, %rdi
	setae	%r8b
	orb	%dl, %r8b
	leaq	22416(%rcx), %rdx
	movq	%rdx, -24(%rsp)
	leaq	22416(%rax), %rdx
	movq	%rdx, -32(%rsp)
	xorl	%r11d, %r11d
	movabsq	$-1600, %rdx
	vmovapd	(%r9,%rdx), %xmm0
	movabsq	$-1584, %rdx
	vmovsd	(%r9,%rdx), %xmm1
	jmp	22
	nopw	(%rax,%rax)
	incl	%r11d
	cmpl	$1000, %r11d
	je	2485
	movl	$1, %ebx
	movq	%rax, %r14
	movq	-24(%rsp), %r15
	movq	%rcx, %r12
	movq	-8(%rsp), %r13
	jmp	53
	nopw	(%rax,%rax)
	incq	%rbx
	addq	$22400, %r13
	addq	$22400, %r12
	addq	$22400, %r15
	addq	$22400, %r14
	cmpq	$2799, %rbx
	je	1172
	movl	$1, %ebp
	testb	%sil, %sil
	je	324
	movl	$2801, %r9d
	vmovupd	-8(%r13,%r9,8), %xmm2
	vmovupd	(%r13,%r9,8), %xmm3
	vmovupd	16(%r13,%r9,8), %xmm4
	vaddpd	-16(%r13,%r9,8), %xmm2, %xmm2
	vaddpd	8(%r13,%r9,8), %xmm3, %xmm5
	vaddpd	%xmm3, %xmm2, %xmm2
	vaddpd	%xmm4, %xmm5, %xmm3
	vaddpd	22392(%r13,%r9,8), %xmm2, %xmm2
	vaddpd	22408(%r13,%r9,8), %xmm3, %xmm3
	vaddpd	-22408(%r13,%r9,8), %xmm2, %xmm2
	vaddpd	-22392(%r13,%r9,8), %xmm3, %xmm3
	vmulpd	%xmm0, %xmm2, %xmm2
	vmulpd	%xmm0, %xmm3, %xmm3
	vmovupd	%xmm2, (%r12,%r9,8)
	vmovupd	%xmm3, 16(%r12,%r9,8)
	vaddpd	24(%r13,%r9,8), %xmm4, %xmm2
	vmovupd	32(%r13,%r9,8), %xmm3
	vmovupd	48(%r13,%r9,8), %xmm4
	vaddpd	40(%r13,%r9,8), %xmm3, %xmm5
	vaddpd	%xmm3, %xmm2, %xmm2
	vaddpd	%xmm4, %xmm5, %xmm3
	vaddpd	22424(%r13,%r9,8), %xmm2, %xmm2
	vaddpd	22440(%r13,%r9,8), %xmm3, %xmm3
	vaddpd	-22376(%r13,%r9,8), %xmm2, %xmm2
	vaddpd	-22360(%r13,%r9,8), %xmm3, %xmm3
	vmulpd	%xmm0, %xmm2, %xmm2
	vmulpd	%xmm0, %xmm3, %xmm3
	vmovupd	%xmm2, 32(%r12,%r9,8)
	vmovupd	%xmm3, 48(%r12,%r9,8)
	vmovupd	64(%r13,%r9,8), %xmm2
	vaddpd	56(%r13,%r9,8), %xmm4, %xmm3
	vaddpd	72(%r13,%r9,8), %xmm2, %xmm4
	vaddpd	%xmm2, %xmm3, %xmm2
	vaddpd	80(%r13,%r9,8), %xmm4, %xmm3
	vaddpd	22456(%r13,%r9,8), %xmm2, %xmm2
	vaddpd	22472(%r13,%r9,8), %xmm3, %xmm3
	vaddpd	-22344(%r13,%r9,8), %xmm2, %xmm2
	vaddpd	-22328(%r13,%r9,8), %xmm3, %xmm3
	vmulpd	%xmm0, %xmm2, %xmm2
	vmulpd	%xmm0, %xmm3, %xmm3
	vmovupd	%xmm2, 64(%r12,%r9,8)
	vmovupd	%xmm3, 80(%r12,%r9,8)
	addq	$12, %r9
	cmpq	$5597, %r9
	jne	-313
	movl	$2797, %ebp
	leal	(,%rbp,8), %r10d
	leaq	(%r15,%r10), %rdx
	addq	%r14, %r10
	xorl	%r9d, %r9d
	vmovsd	22400(%r10,%r9,8), %xmm2
	vaddsd	22392(%r10,%r9,8), %xmm2, %xmm2
	vaddsd	22408(%r10,%r9,8), %xmm2, %xmm2
	vaddsd	44800(%r10,%r9,8), %xmm2, %xmm2
	vaddsd	(%r10,%r9,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, -16(%rdx,%r9,8)
	incq	%r9
	cmpq	$2, %r9
	jne	-66
	testb	%sil, %sil
	jne	-475
	movl	$1, %edx
	subl	%ebp, %edx
	testb	$4, %dl
	jne	261
	leaq	(%r14,%rbp,8), %rdx
	vmovsd	22400(%rdx,%r9,8), %xmm2
	vaddsd	22392(%rdx,%r9,8), %xmm2, %xmm2
	leal	(,%rbp,8), %r10d
	leaq	(%r14,%r10), %rdi
	vaddsd	22408(%rdi,%r9,8), %xmm2, %xmm2
	vaddsd	44800(%rdi,%r9,8), %xmm2, %xmm2
	vaddsd	(%rdi,%r9,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	addq	%r15, %r10
	vmovsd	%xmm2, -16(%r10,%r9,8)
	vmovsd	22408(%rdi,%r9,8), %xmm2
	vaddsd	22400(%rdx,%r9,8), %xmm2, %xmm2
	vaddsd	22416(%rdi,%r9,8), %xmm2, %xmm2
	vaddsd	44808(%rdi,%r9,8), %xmm2, %xmm2
	vaddsd	8(%rdi,%r9,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, -8(%r10,%r9,8)
	vmovsd	22416(%rdi,%r9,8), %xmm2
	vaddsd	22408(%rdi,%r9,8), %xmm2, %xmm2
	vaddsd	22424(%rdi,%r9,8), %xmm2, %xmm2
	vaddsd	44816(%rdi,%r9,8), %xmm2, %xmm2
	vaddsd	16(%rdi,%r9,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, (%r10,%r9,8)
	vmovsd	22424(%rdi,%r9,8), %xmm2
	vaddsd	22416(%rdi,%r9,8), %xmm2, %xmm2
	vaddsd	22432(%rdi,%r9,8), %xmm2, %xmm2
	vaddsd	44824(%rdi,%r9,8), %xmm2, %xmm2
	leaq	4(%rbp,%r9), %rbp
	vaddsd	24(%rdi,%r9,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, 8(%r10,%r9,8)
	jmp	8
	nopl	(%rax,%rax)
	addq	%r9, %rbp
	addq	$2799, %rbp
	nopw	(%rax,%rax)
	vmovsd	8(%r14,%rbp,8), %xmm2
	vaddsd	(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	16(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	22408(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	-22392(%r14,%rbp,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, 8(%r12,%rbp,8)
	vmovsd	16(%r14,%rbp,8), %xmm2
	vaddsd	8(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	24(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	22416(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	-22384(%r14,%rbp,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, 16(%r12,%rbp,8)
	vmovsd	24(%r14,%rbp,8), %xmm2
	vaddsd	16(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	32(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	22424(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	-22376(%r14,%rbp,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, 24(%r12,%rbp,8)
	vmovsd	32(%r14,%rbp,8), %xmm2
	vaddsd	24(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	40(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	22432(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	-22368(%r14,%rbp,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, 32(%r12,%rbp,8)
	vmovsd	40(%r14,%rbp,8), %xmm2
	vaddsd	32(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	48(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	22440(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	-22360(%r14,%rbp,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, 40(%r12,%rbp,8)
	vmovsd	48(%r14,%rbp,8), %xmm2
	vaddsd	40(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	56(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	22448(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	-22352(%r14,%rbp,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, 48(%r12,%rbp,8)
	vmovsd	56(%r14,%rbp,8), %xmm2
	vaddsd	48(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	64(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	22456(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	-22344(%r14,%rbp,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, 56(%r12,%rbp,8)
	vmovsd	64(%r14,%rbp,8), %xmm2
	vaddsd	56(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	72(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	22464(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	-22336(%r14,%rbp,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, 64(%r12,%rbp,8)
	addq	$8, %rbp
	cmpq	$5598, %rbp
	jne	-432
	jmp	-1205
	nopw	%cs:(%rax,%rax)
	movl	$1, %ebx
	movq	%rcx, %r14
	movq	-32(%rsp), %r15
	movq	%rax, %r12
	movq	-16(%rsp), %r13
	jmp	53
	nopw	(%rax,%rax)
	incq	%rbx
	addq	$22400, %r13
	addq	$22400, %r12
	addq	$22400, %r15
	addq	$22400, %r14
	cmpq	$2799, %rbx
	je	-1340
	movl	$1, %ebp
	testb	%r8b, %r8b
	je	324
	movl	$2801, %r9d
	vmovupd	-8(%r13,%r9,8), %xmm2
	vmovupd	(%r13,%r9,8), %xmm3
	vmovupd	16(%r13,%r9,8), %xmm4
	vaddpd	-16(%r13,%r9,8), %xmm2, %xmm2
	vaddpd	8(%r13,%r9,8), %xmm3, %xmm5
	vaddpd	%xmm3, %xmm2, %xmm2
	vaddpd	%xmm4, %xmm5, %xmm3
	vaddpd	22392(%r13,%r9,8), %xmm2, %xmm2
	vaddpd	22408(%r13,%r9,8), %xmm3, %xmm3
	vaddpd	-22408(%r13,%r9,8), %xmm2, %xmm2
	vaddpd	-22392(%r13,%r9,8), %xmm3, %xmm3
	vmulpd	%xmm0, %xmm2, %xmm2
	vmulpd	%xmm0, %xmm3, %xmm3
	vmovupd	%xmm2, (%r12,%r9,8)
	vmovupd	%xmm3, 16(%r12,%r9,8)
	vaddpd	24(%r13,%r9,8), %xmm4, %xmm2
	vmovupd	32(%r13,%r9,8), %xmm3
	vmovupd	48(%r13,%r9,8), %xmm4
	vaddpd	40(%r13,%r9,8), %xmm3, %xmm5
	vaddpd	%xmm3, %xmm2, %xmm2
	vaddpd	%xmm4, %xmm5, %xmm3
	vaddpd	22424(%r13,%r9,8), %xmm2, %xmm2
	vaddpd	22440(%r13,%r9,8), %xmm3, %xmm3
	vaddpd	-22376(%r13,%r9,8), %xmm2, %xmm2
	vaddpd	-22360(%r13,%r9,8), %xmm3, %xmm3
	vmulpd	%xmm0, %xmm2, %xmm2
	vmulpd	%xmm0, %xmm3, %xmm3
	vmovupd	%xmm2, 32(%r12,%r9,8)
	vmovupd	%xmm3, 48(%r12,%r9,8)
	vmovupd	64(%r13,%r9,8), %xmm2
	vaddpd	56(%r13,%r9,8), %xmm4, %xmm3
	vaddpd	72(%r13,%r9,8), %xmm2, %xmm4
	vaddpd	%xmm2, %xmm3, %xmm2
	vaddpd	80(%r13,%r9,8), %xmm4, %xmm3
	vaddpd	22456(%r13,%r9,8), %xmm2, %xmm2
	vaddpd	22472(%r13,%r9,8), %xmm3, %xmm3
	vaddpd	-22344(%r13,%r9,8), %xmm2, %xmm2
	vaddpd	-22328(%r13,%r9,8), %xmm3, %xmm3
	vmulpd	%xmm0, %xmm2, %xmm2
	vmulpd	%xmm0, %xmm3, %xmm3
	vmovupd	%xmm2, 64(%r12,%r9,8)
	vmovupd	%xmm3, 80(%r12,%r9,8)
	addq	$12, %r9
	cmpq	$5597, %r9
	jne	-313
	movl	$2797, %ebp
	leal	(,%rbp,8), %r10d
	leaq	(%r15,%r10), %rdx
	addq	%r14, %r10
	xorl	%r9d, %r9d
	vmovsd	22400(%r10,%r9,8), %xmm2
	vaddsd	22392(%r10,%r9,8), %xmm2, %xmm2
	vaddsd	22408(%r10,%r9,8), %xmm2, %xmm2
	vaddsd	44800(%r10,%r9,8), %xmm2, %xmm2
	vaddsd	(%r10,%r9,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, -16(%rdx,%r9,8)
	incq	%r9
	cmpq	$2, %r9
	jne	-66
	testb	%r8b, %r8b
	jne	-475
	movl	$1, %edx
	subl	%ebp, %edx
	testb	$4, %dl
	jne	261
	leaq	(%r14,%rbp,8), %rdx
	vmovsd	22400(%rdx,%r9,8), %xmm2
	vaddsd	22392(%rdx,%r9,8), %xmm2, %xmm2
	leal	(,%rbp,8), %r10d
	leaq	(%r14,%r10), %rdi
	vaddsd	22408(%rdi,%r9,8), %xmm2, %xmm2
	vaddsd	44800(%rdi,%r9,8), %xmm2, %xmm2
	vaddsd	(%rdi,%r9,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	addq	%r15, %r10
	vmovsd	%xmm2, -16(%r10,%r9,8)
	vmovsd	22408(%rdi,%r9,8), %xmm2
	vaddsd	22400(%rdx,%r9,8), %xmm2, %xmm2
	vaddsd	22416(%rdi,%r9,8), %xmm2, %xmm2
	vaddsd	44808(%rdi,%r9,8), %xmm2, %xmm2
	vaddsd	8(%rdi,%r9,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, -8(%r10,%r9,8)
	vmovsd	22416(%rdi,%r9,8), %xmm2
	vaddsd	22408(%rdi,%r9,8), %xmm2, %xmm2
	vaddsd	22424(%rdi,%r9,8), %xmm2, %xmm2
	vaddsd	44816(%rdi,%r9,8), %xmm2, %xmm2
	vaddsd	16(%rdi,%r9,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, (%r10,%r9,8)
	vmovsd	22424(%rdi,%r9,8), %xmm2
	vaddsd	22416(%rdi,%r9,8), %xmm2, %xmm2
	vaddsd	22432(%rdi,%r9,8), %xmm2, %xmm2
	vaddsd	44824(%rdi,%r9,8), %xmm2, %xmm2
	leaq	4(%rbp,%r9), %rbp
	vaddsd	24(%rdi,%r9,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, 8(%r10,%r9,8)
	jmp	8
	nopl	(%rax,%rax)
	addq	%r9, %rbp
	addq	$2799, %rbp
	nopw	(%rax,%rax)
	vmovsd	8(%r14,%rbp,8), %xmm2
	vaddsd	(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	16(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	22408(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	-22392(%r14,%rbp,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, 8(%r12,%rbp,8)
	vmovsd	16(%r14,%rbp,8), %xmm2
	vaddsd	8(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	24(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	22416(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	-22384(%r14,%rbp,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, 16(%r12,%rbp,8)
	vmovsd	24(%r14,%rbp,8), %xmm2
	vaddsd	16(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	32(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	22424(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	-22376(%r14,%rbp,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, 24(%r12,%rbp,8)
	vmovsd	32(%r14,%rbp,8), %xmm2
	vaddsd	24(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	40(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	22432(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	-22368(%r14,%rbp,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, 32(%r12,%rbp,8)
	vmovsd	40(%r14,%rbp,8), %xmm2
	vaddsd	32(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	48(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	22440(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	-22360(%r14,%rbp,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, 40(%r12,%rbp,8)
	vmovsd	48(%r14,%rbp,8), %xmm2
	vaddsd	40(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	56(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	22448(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	-22352(%r14,%rbp,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, 48(%r12,%rbp,8)
	vmovsd	56(%r14,%rbp,8), %xmm2
	vaddsd	48(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	64(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	22456(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	-22344(%r14,%rbp,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, 56(%r12,%rbp,8)
	vmovsd	64(%r14,%rbp,8), %xmm2
	vaddsd	56(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	72(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	22464(%r14,%rbp,8), %xmm2, %xmm2
	vaddsd	-22336(%r14,%rbp,8), %xmm2, %xmm2
	vmulsd	%xmm1, %xmm2, %xmm2
	vmovsd	%xmm2, 64(%r12,%rbp,8)
	addq	$8, %rbp
	cmpq	$5598, %rbp
	jne	-432
	jmp	-1205
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
