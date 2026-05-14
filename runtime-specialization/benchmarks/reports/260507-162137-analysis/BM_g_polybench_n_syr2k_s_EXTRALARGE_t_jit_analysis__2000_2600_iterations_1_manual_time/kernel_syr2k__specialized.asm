	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	leaq	-7(%rip), %rax
	movabsq	$-7946, %rcx
	addq	%rax, %rcx
	movabsq	$9223372036854775804, %rdx
	movabsq	$80, %rax
	movq	(%rcx,%rax), %rax
	movq	(%rax), %rbx
	movabsq	$192, %rax
	movq	(%rcx,%rax), %rax
	movq	(%rax), %rax
	movq	%rax, -64(%rsp)
	movabsq	$152, %rax
	movq	(%rcx,%rax), %rax
	movq	(%rax), %rax
	movq	%rax, -72(%rsp)
	leaq	8(%rbx), %rax
	movq	%rax, -32(%rsp)
	leaq	16(%rbx), %r8
	movl	$1, %r9d
	xorl	%r15d, %r15d
	movabsq	$-1584, %rax
	vmovsd	(%rcx,%rax), %xmm0
	leaq	2(%rdx), %rax
	movq	%rax, -40(%rsp)
	movabsq	$-1576, %rax
	vmovsd	(%rcx,%rax), %xmm1
	movabsq	$-1600, %rax
	vmovapd	(%rcx,%rax), %xmm2
	movabsq	$-1616, %rax
	vmovapd	(%rcx,%rax), %xmm3
	movq	%rbx, -24(%rsp)
	jmp	47
	nopw	(%rax,%rax)
	incq	%r15
	incq	%r9
	movq	-16(%rsp), %r8
	addq	$20800, %r8
	addq	$20800, %rbx
	cmpq	$2600, %r15
	je	977
	imulq	$20800, %r15, %rcx
	addq	-24(%rsp), %rcx
	imulq	$20808, %r15, %rax
	addq	-32(%rsp), %rax
	imulq	$16000, %r15, %r10
	movq	-64(%rsp), %rdx
	leaq	(%rdx,%r10), %rsi
	movq	%rsi, -56(%rsp)
	leaq	(%rdx,%r10), %rsi
	addq	$16000, %rsi
	movq	-72(%rsp), %rdi
	leaq	(%rdi,%r10), %rdx
	addq	$16000, %rdx
	addq	%rdi, %r10
	movq	%r10, -48(%rsp)
	cmpq	$4, %r9
	movq	%r8, -16(%rsp)
	jae	15
	xorl	%edi, %edi
	jmp	91
	nopw	%cs:(%rax,%rax)
	movq	%r9, %r10
	andq	$-4, %r10
	movq	%r9, %rdi
	movabsq	$9223372036854775804, %r11
	andq	%r11, %rdi
	xorl	%r11d, %r11d
	nopw	(%rax,%rax)
	vmulpd	-16(%r8,%r11,8), %xmm3, %xmm4
	vmulpd	(%r8,%r11,8), %xmm3, %xmm5
	vmovupd	%xmm4, -16(%r8,%r11,8)
	vmovupd	%xmm5, (%r8,%r11,8)
	addq	$4, %r11
	cmpq	%r11, %r10
	jne	-35
	jmp	24
	nopw	%cs:(%rax,%rax)
	vmulsd	(%rbx,%rdi,8), %xmm0, %xmm4
	vmovsd	%xmm4, (%rbx,%rdi,8)
	incq	%rdi
	cmpq	%rdi, %r9
	jne	-18
	movq	%r9, %r12
	andq	$-2, %r12
	cmpq	$4, %r9
	setb	%dil
	cmpq	%rsi, %rcx
	setb	%sil
	cmpq	%rax, -56(%rsp)
	setb	%r10b
	movq	-64(%rsp), %r14
	cmpq	%rax, %r14
	setb	%r11b
	orb	%r10b, %r11b
	andb	%sil, %r11b
	cmpq	%rdx, %rcx
	setb	%cl
	movq	-72(%rsp), %rsi
	cmpq	%rax, %rsi
	setb	%dl
	cmpq	%rax, -48(%rsp)
	setb	%al
	orb	%dl, %al
	andb	%cl, %al
	orb	%r11b, %al
	orb	%dil, %al
	movb	%al, -73(%rsp)
	movq	%r9, %rax
	andq	-40(%rsp), %rax
	movq	%rax, -8(%rsp)
	movq	%r14, %rdx
	xorl	%r11d, %r11d
	jmp	25
	nop
	incq	%r11
	addq	$8, %rsi
	addq	$8, %rdx
	cmpq	$2000, %r11
	je	-376
	movq	-48(%rsp), %rax
	leaq	(%rax,%r11,8), %rcx
	movq	-56(%rsp), %rax
	leaq	(%rax,%r11,8), %rax
	cmpb	$0, -73(%rsp)
	je	15
	xorl	%r10d, %r10d
	jmp	116
	nopw	%cs:(%rax,%rax)
	vmovddup	(%rcx), %xmm4
	vmovddup	(%rax), %xmm5
	xorl	%edi, %edi
	xorl	%r10d, %r10d
	nopl	(%rax)
	vmovsd	(%rdx,%rdi), %xmm6
	vmovhpd	16000(%rdx,%rdi), %xmm6, %xmm6
	vmovsd	(%rsi,%rdi), %xmm7
	vmovhpd	16000(%rsi,%rdi), %xmm7, %xmm7
	vmulpd	%xmm2, %xmm6, %xmm6
	vmulpd	%xmm2, %xmm7, %xmm7
	vmulpd	%xmm7, %xmm5, %xmm7
	vfmadd231pd	%xmm6, %xmm4, %xmm7
	vaddpd	(%rbx,%r10,8), %xmm7, %xmm6
	vmovupd	%xmm6, (%rbx,%r10,8)
	addq	$2, %r10
	addq	$32000, %rdi
	cmpq	%r10, %r12
	jne	-73
	movq	-8(%rsp), %rdi
	movq	%rdi, %r10
	cmpq	%rdi, %r9
	je	-170
	movq	%r15, %r8
	movl	%r9d, %ebp
	subl	%r10d, %ebp
	movq	%r10, %rdi
	testb	$7, %bpl
	je	98
	imulq	$16000, %r10, %r15
	leaq	(%rsi,%r15), %r14
	addq	%rdx, %r15
	movl	%r9d, %edi
	subb	%r10b, %dil
	movzbl	%dil, %edi
	andl	$7, %edi
	imulq	$16000, %rdi, %r13
	xorl	%ebp, %ebp
	movq	%r10, %rdi
	nopw	%cs:(%rax,%rax)
	vmulsd	(%r15,%rbp), %xmm1, %xmm4
	vmulsd	(%r14,%rbp), %xmm1, %xmm5
	vmulsd	(%rax), %xmm5, %xmm5
	vfmadd231sd	(%rcx), %xmm4, %xmm5
	vaddsd	(%rbx,%rdi,8), %xmm5, %xmm4
	vmovsd	%xmm4, (%rbx,%rdi,8)
	incq	%rdi
	addq	$16000, %rbp
	cmpq	%rbp, %r13
	jne	-46
	movq	%r8, %r15
	movq	%r8, %r14
	subq	%r10, %r14
	cmpq	$7, %r14
	jb	-305
	imulq	$16000, %rdi, %r10
	nopl	(%rax,%rax)
	vmulsd	(%rdx,%r10), %xmm1, %xmm4
	vmulsd	(%rsi,%r10), %xmm1, %xmm5
	vmulsd	(%rax), %xmm5, %xmm5
	vfmadd231sd	(%rcx), %xmm4, %xmm5
	vaddsd	(%rbx,%rdi,8), %xmm5, %xmm4
	vmovsd	%xmm4, (%rbx,%rdi,8)
	vmulsd	16000(%rdx,%r10), %xmm1, %xmm4
	vmulsd	16000(%rsi,%r10), %xmm1, %xmm5
	vmulsd	(%rax), %xmm5, %xmm5
	vfmadd231sd	(%rcx), %xmm4, %xmm5
	vaddsd	8(%rbx,%rdi,8), %xmm5, %xmm4
	vmovsd	%xmm4, 8(%rbx,%rdi,8)
	vmulsd	32000(%rdx,%r10), %xmm1, %xmm4
	vmulsd	32000(%rsi,%r10), %xmm1, %xmm5
	vmulsd	(%rax), %xmm5, %xmm5
	vfmadd231sd	(%rcx), %xmm4, %xmm5
	vaddsd	16(%rbx,%rdi,8), %xmm5, %xmm4
	vmovsd	%xmm4, 16(%rbx,%rdi,8)
	vmulsd	48000(%rdx,%r10), %xmm1, %xmm4
	vmulsd	48000(%rsi,%r10), %xmm1, %xmm5
	vmulsd	(%rax), %xmm5, %xmm5
	vfmadd231sd	(%rcx), %xmm4, %xmm5
	vaddsd	24(%rbx,%rdi,8), %xmm5, %xmm4
	vmovsd	%xmm4, 24(%rbx,%rdi,8)
	vmulsd	64000(%rdx,%r10), %xmm1, %xmm4
	vmulsd	64000(%rsi,%r10), %xmm1, %xmm5
	vmulsd	(%rax), %xmm5, %xmm5
	vfmadd231sd	(%rcx), %xmm4, %xmm5
	vaddsd	32(%rbx,%rdi,8), %xmm5, %xmm4
	vmovsd	%xmm4, 32(%rbx,%rdi,8)
	vmulsd	80000(%rdx,%r10), %xmm1, %xmm4
	vmulsd	80000(%rsi,%r10), %xmm1, %xmm5
	vmulsd	(%rax), %xmm5, %xmm5
	vfmadd231sd	(%rcx), %xmm4, %xmm5
	vaddsd	40(%rbx,%rdi,8), %xmm5, %xmm4
	vmovsd	%xmm4, 40(%rbx,%rdi,8)
	vmulsd	96000(%rdx,%r10), %xmm1, %xmm4
	vmulsd	96000(%rsi,%r10), %xmm1, %xmm5
	vmulsd	(%rax), %xmm5, %xmm5
	vfmadd231sd	(%rcx), %xmm4, %xmm5
	vaddsd	48(%rbx,%rdi,8), %xmm5, %xmm4
	vmovsd	%xmm4, 48(%rbx,%rdi,8)
	vmulsd	112000(%rdx,%r10), %xmm1, %xmm4
	vmulsd	112000(%rsi,%r10), %xmm1, %xmm5
	vmulsd	(%rax), %xmm5, %xmm5
	vfmadd231sd	(%rcx), %xmm4, %xmm5
	vaddsd	56(%rbx,%rdi,8), %xmm5, %xmm4
	vmovsd	%xmm4, 56(%rbx,%rdi,8)
	addq	$8, %rdi
	addq	$128000, %r10
	cmpq	%rdi, %r9
	jne	-338
	jmp	-663
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
