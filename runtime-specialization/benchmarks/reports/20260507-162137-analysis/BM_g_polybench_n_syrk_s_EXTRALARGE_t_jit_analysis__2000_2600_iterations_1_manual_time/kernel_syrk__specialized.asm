	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	leaq	-7(%rip), %rax
	movabsq	$-7962, %rcx
	addq	%rax, %rcx
	movabsq	$48, %rax
	movq	(%rcx,%rax), %rax
	movq	(%rax), %rbx
	movabsq	$184, %rax
	movq	(%rcx,%rax), %rax
	movq	(%rax), %rax
	leaq	8(%rbx), %rdx
	movq	%rdx, -16(%rsp)
	leaq	16000(%rax), %rdx
	movq	%rdx, -24(%rsp)
	leaq	16(%rbx), %r8
	movq	%rax, -48(%rsp)
	leaq	48008(%rax), %rax
	movq	%rax, -32(%rsp)
	movl	$1, %r10d
	movq	$0, -56(%rsp)
	movabsq	$-1584, %rax
	vmovsd	(%rcx,%rax), %xmm0
	movabsq	$-1576, %rax
	vmovsd	(%rcx,%rax), %xmm1
	movabsq	$-1600, %rax
	vmovapd	(%rcx,%rax), %xmm2
	movq	%rbx, -8(%rsp)
	jmp	46
	nopl	(%rax)
	movq	-56(%rsp), %rcx
	incq	%rcx
	incq	%r10
	addq	$20800, %r8
	addq	$20800, %rbx
	movq	%rcx, -56(%rsp)
	cmpq	$2600, %rcx
	je	871
	movq	%r10, %r14
	andq	$-4, %r14
	cmpq	$4, %r10
	jae	8
	xorl	%r12d, %r12d
	jmp	83
	nopl	(%rax)
	movq	%r10, %r12
	movabsq	$9223372036854775804, %rax
	andq	%rax, %r12
	xorl	%eax, %eax
	nopw	%cs:(%rax,%rax)
	vmulpd	-16(%r8,%rax,8), %xmm2, %xmm3
	vmulpd	(%r8,%rax,8), %xmm2, %xmm4
	vmovupd	%xmm3, -16(%r8,%rax,8)
	vmovupd	%xmm4, (%r8,%rax,8)
	addq	$4, %rax
	cmpq	%rax, %r14
	jne	-35
	cmpq	%r12, %r10
	je	44
	nopl	(%rax,%rax)
	vmulsd	(%rbx,%r12,8), %xmm0, %xmm3
	vmovsd	%xmm3, (%rbx,%r12,8)
	incq	%r12
	cmpq	%r12, %r10
	jne	-20
	movq	%r10, %r12
	movabsq	$9223372036854775804, %rax
	andq	%rax, %r12
	movq	-56(%rsp), %rcx
	imulq	$20800, %rcx, %rsi
	addq	-8(%rsp), %rsi
	imulq	$20808, %rcx, %rax
	addq	-16(%rsp), %rax
	imulq	$16000, %rcx, %r13
	movq	-24(%rsp), %rdi
	leaq	(%rdi,%r13), %rcx
	movq	-48(%rsp), %r11
	addq	%r11, %r13
	cmpq	$4, %r10
	setb	%dl
	movq	%rsi, -40(%rsp)
	cmpq	%rcx, %rsi
	setb	%cl
	cmpq	%rax, %r11
	setb	%sil
	cmpq	%rax, %r13
	setb	%bpl
	orb	%sil, %bpl
	andb	%cl, %bpl
	orb	%dl, %bpl
	movq	-32(%rsp), %r9
	movq	%r11, %rdx
	xorl	%ecx, %ecx
	jmp	31
	nop
	addq	$2, %rcx
	addq	$16, %rdx
	addq	$16, %rdi
	addq	$16, %r9
	cmpq	$2000, %rcx
	je	-317
	leaq	(,%rcx,8), %rsi
	addq	%r13, %rsi
	testb	%bpl, %bpl
	je	19
	xorl	%r11d, %r11d
	jmp	139
	nopw	%cs:(%rax,%rax)
	vmulsd	(%rsi), %xmm1, %xmm3
	vmovddup	%xmm3, %xmm3
	movq	%rdx, %rax
	xorl	%r11d, %r11d
	nop
	vmovsd	(%rax), %xmm4
	vmovhpd	16000(%rax), %xmm4, %xmm4
	vmovsd	32000(%rax), %xmm5
	vmovhpd	48000(%rax), %xmm5, %xmm5
	vfmadd213pd	-16(%r8,%r11,8), %xmm3, %xmm4
	vfmadd213pd	(%r8,%r11,8), %xmm3, %xmm5
	vmovupd	%xmm4, -16(%r8,%r11,8)
	vmovupd	%xmm5, (%r8,%r11,8)
	addq	$4, %r11
	addq	$64000, %rax
	cmpq	%r11, %r14
	jne	-69
	movq	%r12, %r11
	cmpq	%r12, %r10
	jne	35
	movq	%rcx, %rax
	orq	$1, %rax
	leaq	8(,%rcx,8), %rsi
	addq	%r13, %rsi
	jmp	188
	nopw	%cs:(%rax,%rax)
	movq	%r11, %rax
	testb	$1, %r10b
	je	45
	vmulsd	(%rsi), %xmm1, %xmm3
	imulq	$16000, %r11, %rax
	addq	-48(%rsp), %rax
	vmovsd	(%rax,%rcx,8), %xmm4
	movq	-40(%rsp), %rax
	vfmadd213sd	(%rax,%r11,8), %xmm3, %xmm4
	vmovsd	%xmm4, (%rax,%r11,8)
	movq	%r11, %rax
	orq	$1, %rax
	cmpq	%r11, -56(%rsp)
	je	81
	imulq	$16000, %rax, %r11
	addq	%rdi, %r11
	nopw	(%rax,%rax)
	vmulsd	(%rsi), %xmm1, %xmm3
	vmovsd	-16000(%r11), %xmm4
	vfmadd213sd	(%rbx,%rax,8), %xmm3, %xmm4
	vmovsd	%xmm4, (%rbx,%rax,8)
	vmulsd	(%rsi), %xmm1, %xmm3
	vmovsd	(%r11), %xmm4
	vfmadd213sd	8(%rbx,%rax,8), %xmm3, %xmm4
	vmovsd	%xmm4, 8(%rbx,%rax,8)
	addq	$2, %rax
	addq	$32000, %r11
	cmpq	%rax, %r10
	jne	-62
	movq	%rcx, %rax
	orq	$1, %rax
	leaq	8(,%rcx,8), %rsi
	addq	%r13, %rsi
	testb	%bpl, %bpl
	je	11
	xorl	%r15d, %r15d
	jmp	108
	nopw	(%rax,%rax)
	vmulsd	(%rsi), %xmm1, %xmm3
	vmovddup	%xmm3, %xmm3
	movq	%r9, %r11
	xorl	%r15d, %r15d
	nop
	vmovsd	-48000(%r11), %xmm4
	vmovhpd	-32000(%r11), %xmm4, %xmm4
	vmovsd	-16000(%r11), %xmm5
	vmovhpd	(%r11), %xmm5, %xmm5
	vfmadd213pd	-16(%r8,%r15,8), %xmm3, %xmm4
	vfmadd213pd	(%r8,%r15,8), %xmm3, %xmm5
	vmovupd	%xmm4, -16(%r8,%r15,8)
	vmovupd	%xmm5, (%r8,%r15,8)
	addq	$4, %r15
	addq	$64000, %r11
	cmpq	%r15, %r14
	jne	-74
	movq	%r12, %r15
	cmpq	%r12, %r10
	je	-470
	movq	%r15, %r11
	testb	$1, %r10b
	je	46
	vmulsd	(%rsi), %xmm1, %xmm3
	imulq	$16000, %r15, %r11
	addq	-48(%rsp), %r11
	vmovsd	(%r11,%rax,8), %xmm4
	movq	-40(%rsp), %r11
	vfmadd213sd	(%r11,%r15,8), %xmm3, %xmm4
	vmovsd	%xmm4, (%r11,%r15,8)
	movq	%r15, %r11
	orq	$1, %r11
	cmpq	%r15, -56(%rsp)
	je	-536
	imulq	$16000, %r11, %r15
	leaq	(%r15,%rax,8), %rax
	addq	-48(%rsp), %rax
	nopl	(%rax,%rax)
	vmulsd	(%rsi), %xmm1, %xmm3
	vmovsd	(%rax), %xmm4
	vfmadd213sd	(%rbx,%r11,8), %xmm3, %xmm4
	vmovsd	%xmm4, (%rbx,%r11,8)
	vmulsd	(%rsi), %xmm1, %xmm3
	vmovsd	16000(%rax), %xmm4
	vfmadd213sd	8(%rbx,%r11,8), %xmm3, %xmm4
	vmovsd	%xmm4, 8(%rbx,%r11,8)
	addq	$2, %r11
	addq	$32000, %rax
	cmpq	%r11, %r10
	jne	-61
	jmp	-626
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
