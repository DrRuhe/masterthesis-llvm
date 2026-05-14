	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r12
	pushq	%rbx
	leaq	-7(%rip), %rax
	movabsq	$-7960, %r10
	addq	%rax, %r10
	movabsq	$24, %rax
	movq	(%r10,%rax), %rax
	movq	(%rax), %rax
	movabsq	$176, %rcx
	movq	(%r10,%rcx), %rcx
	movq	(%rcx), %rcx
	movabsq	$144, %rdx
	movq	(%r10,%rdx), %rdx
	movq	(%rdx), %rdx
	leaq	47840000(%rdx), %rsi
	leaq	18400(%rax), %rdi
	leaq	20800(%rcx), %r8
	xorl	%r9d, %r9d
	movabsq	$-1600, %r11
	vmovapd	(%r10,%r11), %xmm0
	movabsq	$-1584, %r11
	vmovsd	(%r10,%r11), %xmm1
	movq	%rax, %r10
	jmp	27
	nopl	(%rax)
	incq	%r9
	addq	$18400, %r10
	cmpq	$2000, %r9
	je	1399
	movl	$38, %r11d
	nopl	(%rax)
	vmulpd	-304(%r10,%r11,8), %xmm0, %xmm2
	vmulpd	-288(%r10,%r11,8), %xmm0, %xmm3
	vmovupd	%xmm2, -304(%r10,%r11,8)
	vmovupd	%xmm3, -288(%r10,%r11,8)
	vmulpd	-272(%r10,%r11,8), %xmm0, %xmm2
	vmulpd	-256(%r10,%r11,8), %xmm0, %xmm3
	vmovupd	%xmm2, -272(%r10,%r11,8)
	vmovupd	%xmm3, -256(%r10,%r11,8)
	vmulpd	-240(%r10,%r11,8), %xmm0, %xmm2
	vmulpd	-224(%r10,%r11,8), %xmm0, %xmm3
	vmovupd	%xmm2, -240(%r10,%r11,8)
	vmovupd	%xmm3, -224(%r10,%r11,8)
	vmulpd	-208(%r10,%r11,8), %xmm0, %xmm2
	vmulpd	-192(%r10,%r11,8), %xmm0, %xmm3
	vmovupd	%xmm2, -208(%r10,%r11,8)
	vmovupd	%xmm3, -192(%r10,%r11,8)
	vmulpd	-176(%r10,%r11,8), %xmm0, %xmm2
	vmulpd	-160(%r10,%r11,8), %xmm0, %xmm3
	vmovupd	%xmm2, -176(%r10,%r11,8)
	vmovupd	%xmm3, -160(%r10,%r11,8)
	cmpq	$2318, %r11
	je	155
	vmulpd	-144(%r10,%r11,8), %xmm0, %xmm2
	vmulpd	-128(%r10,%r11,8), %xmm0, %xmm3
	vmovupd	%xmm2, -144(%r10,%r11,8)
	vmovupd	%xmm3, -128(%r10,%r11,8)
	vmulpd	-112(%r10,%r11,8), %xmm0, %xmm2
	vmulpd	-96(%r10,%r11,8), %xmm0, %xmm3
	vmovupd	%xmm2, -112(%r10,%r11,8)
	vmovupd	%xmm3, -96(%r10,%r11,8)
	vmulpd	-80(%r10,%r11,8), %xmm0, %xmm2
	vmulpd	-64(%r10,%r11,8), %xmm0, %xmm3
	vmovupd	%xmm2, -80(%r10,%r11,8)
	vmovupd	%xmm3, -64(%r10,%r11,8)
	vmulpd	-48(%r10,%r11,8), %xmm0, %xmm2
	vmulpd	-32(%r10,%r11,8), %xmm0, %xmm3
	vmovupd	%xmm2, -48(%r10,%r11,8)
	vmovupd	%xmm3, -32(%r10,%r11,8)
	vmulpd	-16(%r10,%r11,8), %xmm0, %xmm2
	vmulpd	(%r10,%r11,8), %xmm0, %xmm3
	vmovupd	%xmm2, -16(%r10,%r11,8)
	vmovupd	%xmm3, (%r10,%r11,8)
	addq	$40, %r11
	jmp	-366
	nop
	imulq	$18400, %r9, %r14
	leaq	(%rax,%r14), %rbx
	addq	%rdi, %r14
	imulq	$20800, %r9, %r15
	leaq	(%rcx,%r15), %r11
	addq	%r8, %r15
	cmpq	%r15, %rbx
	setb	%r15b
	cmpq	%r14, %r11
	setb	%r12b
	cmpq	%rsi, %rbx
	setb	%bl
	cmpq	%r14, %rdx
	setb	%bpl
	testb	%r12b, %r15b
	jne	624
	andb	%bpl, %bl
	jne	615
	movq	%rdx, %rbx
	xorl	%r14d, %r14d
	jmp	38
	nopw	%cs:(%rax,%rax)
	incq	%r14
	addq	$18400, %rbx
	cmpq	$2600, %r14
	je	-519
	vmulsd	(%r11,%r14,8), %xmm1, %xmm2
	vmovddup	%xmm2, %xmm2
	movl	$38, %r15d
	nopw	(%rax,%rax)
	vmovupd	-304(%rbx,%r15,8), %xmm3
	vmovupd	-288(%rbx,%r15,8), %xmm4
	vfmadd213pd	-304(%r10,%r15,8), %xmm2, %xmm3
	vfmadd213pd	-288(%r10,%r15,8), %xmm2, %xmm4
	vmovupd	%xmm3, -304(%r10,%r15,8)
	vmovupd	%xmm4, -288(%r10,%r15,8)
	vmovupd	-272(%rbx,%r15,8), %xmm3
	vmovupd	-256(%rbx,%r15,8), %xmm4
	vfmadd213pd	-272(%r10,%r15,8), %xmm2, %xmm3
	vfmadd213pd	-256(%r10,%r15,8), %xmm2, %xmm4
	vmovupd	%xmm3, -272(%r10,%r15,8)
	vmovupd	%xmm4, -256(%r10,%r15,8)
	vmovupd	-240(%rbx,%r15,8), %xmm3
	vmovupd	-224(%rbx,%r15,8), %xmm4
	vfmadd213pd	-240(%r10,%r15,8), %xmm2, %xmm3
	vfmadd213pd	-224(%r10,%r15,8), %xmm2, %xmm4
	vmovupd	%xmm3, -240(%r10,%r15,8)
	vmovupd	%xmm4, -224(%r10,%r15,8)
	vmovupd	-208(%rbx,%r15,8), %xmm3
	vmovupd	-192(%rbx,%r15,8), %xmm4
	vfmadd213pd	-208(%r10,%r15,8), %xmm2, %xmm3
	vfmadd213pd	-192(%r10,%r15,8), %xmm2, %xmm4
	vmovupd	%xmm3, -208(%r10,%r15,8)
	vmovupd	%xmm4, -192(%r10,%r15,8)
	vmovupd	-176(%rbx,%r15,8), %xmm3
	vmovupd	-160(%rbx,%r15,8), %xmm4
	vfmadd213pd	-176(%r10,%r15,8), %xmm2, %xmm3
	vfmadd213pd	-160(%r10,%r15,8), %xmm2, %xmm4
	vmovupd	%xmm3, -176(%r10,%r15,8)
	vmovupd	%xmm4, -160(%r10,%r15,8)
	cmpq	$2318, %r15
	je	-361
	vmovupd	-144(%rbx,%r15,8), %xmm3
	vmovupd	-128(%rbx,%r15,8), %xmm4
	vfmadd213pd	-144(%r10,%r15,8), %xmm2, %xmm3
	vfmadd213pd	-128(%r10,%r15,8), %xmm2, %xmm4
	vmovupd	%xmm3, -144(%r10,%r15,8)
	vmovupd	%xmm4, -128(%r10,%r15,8)
	vmovupd	-112(%rbx,%r15,8), %xmm3
	vmovupd	-96(%rbx,%r15,8), %xmm4
	vfmadd213pd	-112(%r10,%r15,8), %xmm2, %xmm3
	vfmadd213pd	-96(%r10,%r15,8), %xmm2, %xmm4
	vmovupd	%xmm3, -112(%r10,%r15,8)
	vmovupd	%xmm4, -96(%r10,%r15,8)
	vmovupd	-80(%rbx,%r15,8), %xmm3
	vmovupd	-64(%rbx,%r15,8), %xmm4
	vfmadd213pd	-80(%r10,%r15,8), %xmm2, %xmm3
	vfmadd213pd	-64(%r10,%r15,8), %xmm2, %xmm4
	vmovupd	%xmm3, -80(%r10,%r15,8)
	vmovupd	%xmm4, -64(%r10,%r15,8)
	vmovupd	-48(%rbx,%r15,8), %xmm3
	vmovupd	-32(%rbx,%r15,8), %xmm4
	vfmadd213pd	-48(%r10,%r15,8), %xmm2, %xmm3
	vfmadd213pd	-32(%r10,%r15,8), %xmm2, %xmm4
	vmovupd	%xmm3, -48(%r10,%r15,8)
	vmovupd	%xmm4, -32(%r10,%r15,8)
	vmovupd	-16(%rbx,%r15,8), %xmm3
	vmovupd	(%rbx,%r15,8), %xmm4
	vfmadd213pd	-16(%r10,%r15,8), %xmm2, %xmm3
	vfmadd213pd	(%r10,%r15,8), %xmm2, %xmm4
	vmovupd	%xmm3, -16(%r10,%r15,8)
	vmovupd	%xmm4, (%r10,%r15,8)
	addq	$40, %r15
	jmp	-538
	nopw	(%rax,%rax)
	movq	%rdx, %rbx
	xorl	%r14d, %r14d
	nopw	%cs:(%rax,%rax)
	leaq	(%r11,%r14,8), %r15
	movl	$9, %r12d
	nopw	(%rax,%rax)
	vmulsd	(%r15), %xmm1, %xmm2
	vmovsd	-72(%rbx,%r12,8), %xmm3
	vfmadd213sd	-72(%r10,%r12,8), %xmm2, %xmm3
	vmovsd	%xmm3, -72(%r10,%r12,8)
	vmulsd	(%r15), %xmm1, %xmm2
	vmovsd	-64(%rbx,%r12,8), %xmm3
	vfmadd213sd	-64(%r10,%r12,8), %xmm2, %xmm3
	vmovsd	%xmm3, -64(%r10,%r12,8)
	vmulsd	(%r15), %xmm1, %xmm2
	vmovsd	-56(%rbx,%r12,8), %xmm3
	vfmadd213sd	-56(%r10,%r12,8), %xmm2, %xmm3
	vmovsd	%xmm3, -56(%r10,%r12,8)
	vmulsd	(%r15), %xmm1, %xmm2
	vmovsd	-48(%rbx,%r12,8), %xmm3
	vfmadd213sd	-48(%r10,%r12,8), %xmm2, %xmm3
	vmovsd	%xmm3, -48(%r10,%r12,8)
	vmulsd	(%r15), %xmm1, %xmm2
	vmovsd	-40(%rbx,%r12,8), %xmm3
	vfmadd213sd	-40(%r10,%r12,8), %xmm2, %xmm3
	vmovsd	%xmm3, -40(%r10,%r12,8)
	vmulsd	(%r15), %xmm1, %xmm2
	vmovsd	-32(%rbx,%r12,8), %xmm3
	vfmadd213sd	-32(%r10,%r12,8), %xmm2, %xmm3
	vmovsd	%xmm3, -32(%r10,%r12,8)
	vmulsd	(%r15), %xmm1, %xmm2
	vmovsd	-24(%rbx,%r12,8), %xmm3
	vfmadd213sd	-24(%r10,%r12,8), %xmm2, %xmm3
	vmovsd	%xmm3, -24(%r10,%r12,8)
	vmulsd	(%r15), %xmm1, %xmm2
	vmovsd	-16(%rbx,%r12,8), %xmm3
	vfmadd213sd	-16(%r10,%r12,8), %xmm2, %xmm3
	vmovsd	%xmm3, -16(%r10,%r12,8)
	vmulsd	(%r15), %xmm1, %xmm2
	vmovsd	-8(%rbx,%r12,8), %xmm3
	vfmadd213sd	-8(%r10,%r12,8), %xmm2, %xmm3
	vmovsd	%xmm3, -8(%r10,%r12,8)
	vmulsd	(%r15), %xmm1, %xmm2
	vmovsd	(%rbx,%r12,8), %xmm3
	vfmadd213sd	(%r10,%r12,8), %xmm2, %xmm3
	vmovsd	%xmm3, (%r10,%r12,8)
	addq	$10, %r12
	cmpq	$2309, %r12
	jne	-274
	incq	%r14
	addq	$18400, %rbx
	cmpq	$2600, %r14
	jne	-313
	jmp	-1422
	popq	%rbx
	popq	%r12
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
