	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	leaq	-7(%rip), %rcx
	movabsq	$-7946, %rax
	addq	%rcx, %rax
	movabsq	$64, %rcx
	movq	(%rax,%rcx), %rcx
	movq	(%rcx), %r9
	movabsq	$184, %rcx
	movq	(%rax,%rcx), %rcx
	movq	(%rcx), %r10
	movabsq	$160, %rcx
	movq	(%rax,%rcx), %rcx
	movq	(%rcx), %rdx
	leaq	20800(%r9), %rcx
	leaq	20800(%rdx), %rsi
	leaq	32000000(%r10), %rdi
	cmpq	%rsi, %r9
	setb	%sil
	cmpq	%rcx, %rdx
	setb	%r8b
	andb	%sil, %r8b
	cmpq	%rdi, %r9
	setb	%sil
	cmpq	%rcx, %r10
	setb	%cl
	andb	%sil, %cl
	orb	%r8b, %cl
	movb	%cl, -33(%rsp)
	leaq	56(%r10), %rdi
	xorl	%r8d, %r8d
	movabsq	$-1616, %rcx
	vmovsd	(%rax,%rcx), %xmm0
	movabsq	$-1608, %rcx
	vmovsd	(%rax,%rcx), %xmm1
	vxorpd	%xmm2, %xmm2, %xmm2
	movabsq	$-1600, %rcx
	vmovapd	(%rax,%rcx), %xmm3
	movabsq	$-1584, %rcx
	vmovapd	(%rax,%rcx), %xmm4
	vxorpd	%xmm5, %xmm5, %xmm5
	movq	%r10, -24(%rsp)
	movq	%r10, -32(%rsp)
	jmp	43
	nopw	%cs:(%rax,%rax)
	incq	%r8
	addq	$16000, %rdi
	addq	$16000, -32(%rsp)
	cmpq	$2000, %r8
	je	1347
	imulq	$16000, %r8, %rax
	addq	-24(%rsp), %rax
	leaq	(%rax,%r8,8), %r11
	testq	%r8, %r8
	je	679
	movl	%r8d, %ebx
	andl	$7, %ebx
	movq	%r8, %r14
	andq	$-8, %r14
	movq	%r8, %rcx
	movabsq	$9223372036854775800, %rax
	andq	%rax, %rcx
	movq	%rcx, -16(%rsp)
	imulq	$20800, %r8, %r12
	leaq	(%rdx,%r12), %rax
	movq	%rax, -8(%rsp)
	movq	%rdx, %rbp
	movq	%r9, %rax
	xorl	%esi, %esi
	jmp	65
	nopw	%cs:(%rax,%rax)
	leaq	(%r9,%r12), %r10
	vmulsd	(%rcx), %xmm0, %xmm7
	vmulsd	(%r11), %xmm7, %xmm7
	vfmadd231sd	(%r10,%rsi,8), %xmm1, %xmm7
	vfmadd231sd	%xmm6, %xmm0, %xmm7
	vmovsd	%xmm7, (%r10,%rsi,8)
	incq	%rsi
	addq	$8, %rax
	addq	$8, %rbp
	cmpq	$2600, %rsi
	je	-182
	movq	-8(%rsp), %rcx
	leaq	(%rcx,%rsi,8), %rcx
	vxorpd	%xmm6, %xmm6, %xmm6
	cmpq	$8, %r8
	jae	23
	xorl	%r15d, %r15d
	testb	$7, %r8b
	jne	432
	jmp	-88
	nopl	(%rax,%rax)
	xorl	%r10d, %r10d
	xorl	%r15d, %r15d
	nopw	%cs:(%rax,%rax)
	vmulsd	(%rcx), %xmm0, %xmm7
	vmovsd	-56(%rdi,%r15,8), %xmm8
	vfmadd213sd	(%rax,%r10), %xmm7, %xmm8
	vmovsd	%xmm8, (%rax,%r10)
	vmovsd	(%rbp,%r10), %xmm7
	vfmadd132sd	-56(%rdi,%r15,8), %xmm6, %xmm7
	vmulsd	(%rcx), %xmm0, %xmm6
	vmovsd	-48(%rdi,%r15,8), %xmm8
	vfmadd213sd	20800(%rax,%r10), %xmm6, %xmm8
	vmovsd	%xmm8, 20800(%rax,%r10)
	vmovsd	20800(%rbp,%r10), %xmm6
	vfmadd132sd	-48(%rdi,%r15,8), %xmm7, %xmm6
	vmulsd	(%rcx), %xmm0, %xmm7
	vmovsd	-40(%rdi,%r15,8), %xmm8
	vfmadd213sd	41600(%rax,%r10), %xmm7, %xmm8
	vmovsd	%xmm8, 41600(%rax,%r10)
	vmovsd	41600(%rbp,%r10), %xmm7
	vfmadd132sd	-40(%rdi,%r15,8), %xmm6, %xmm7
	vmulsd	(%rcx), %xmm0, %xmm6
	vmovsd	-32(%rdi,%r15,8), %xmm8
	vfmadd213sd	62400(%rax,%r10), %xmm6, %xmm8
	vmovsd	%xmm8, 62400(%rax,%r10)
	vmovsd	62400(%rbp,%r10), %xmm6
	vfmadd132sd	-32(%rdi,%r15,8), %xmm7, %xmm6
	vmulsd	(%rcx), %xmm0, %xmm7
	vmovsd	-24(%rdi,%r15,8), %xmm8
	vfmadd213sd	83200(%rax,%r10), %xmm7, %xmm8
	vmovsd	%xmm8, 83200(%rax,%r10)
	vmovsd	83200(%rbp,%r10), %xmm7
	vfmadd132sd	-24(%rdi,%r15,8), %xmm6, %xmm7
	vmulsd	(%rcx), %xmm0, %xmm6
	vmovsd	-16(%rdi,%r15,8), %xmm8
	vfmadd213sd	104000(%rax,%r10), %xmm6, %xmm8
	vmovsd	%xmm8, 104000(%rax,%r10)
	vmovsd	104000(%rbp,%r10), %xmm6
	vfmadd132sd	-16(%rdi,%r15,8), %xmm7, %xmm6
	vmulsd	(%rcx), %xmm0, %xmm7
	vmovsd	-8(%rdi,%r15,8), %xmm8
	vfmadd213sd	124800(%rax,%r10), %xmm7, %xmm8
	vmovsd	%xmm8, 124800(%rax,%r10)
	vmovsd	124800(%rbp,%r10), %xmm7
	vfmadd132sd	-8(%rdi,%r15,8), %xmm6, %xmm7
	vmulsd	(%rcx), %xmm0, %xmm6
	vmovsd	(%rdi,%r15,8), %xmm8
	vfmadd213sd	145600(%rax,%r10), %xmm6, %xmm8
	vmovsd	%xmm8, 145600(%rax,%r10)
	vmovsd	145600(%rbp,%r10), %xmm6
	vfmadd132sd	(%rdi,%r15,8), %xmm7, %xmm6
	addq	$8, %r15
	addq	$166400, %r10
	cmpq	%r15, %r14
	jne	-391
	movq	-16(%rsp), %r15
	testb	$7, %r8b
	je	-518
	imulq	$20800, %r15, %r10
	movq	-32(%rsp), %r13
	leaq	(,%r15,8), %r15
	addq	%r13, %r15
	xorl	%r13d, %r13d
	vmulsd	(%rcx), %xmm0, %xmm7
	vmovsd	(%r15,%r13,8), %xmm8
	vfmadd213sd	(%rax,%r10), %xmm7, %xmm8
	vmovsd	%xmm8, (%rax,%r10)
	vmovsd	(%rbp,%r10), %xmm7
	vfmadd231sd	(%r15,%r13,8), %xmm7, %xmm6
	incq	%r13
	addq	$20800, %r10
	cmpq	%r13, %rbx
	jne	-50
	jmp	-599
	nopw	(%rax,%rax)
	cmpb	$0, -33(%rsp)
	je	313
	movl	$9, %eax
	vmulsd	-72(%rdx,%rax,8), %xmm0, %xmm6
	vmulsd	(%r11), %xmm6, %xmm6
	vfmadd231sd	-72(%r9,%rax,8), %xmm1, %xmm6
	vaddsd	%xmm2, %xmm6, %xmm6
	vmovsd	%xmm6, -72(%r9,%rax,8)
	vmulsd	-64(%rdx,%rax,8), %xmm0, %xmm6
	vmulsd	(%r11), %xmm6, %xmm6
	vfmadd231sd	-64(%r9,%rax,8), %xmm1, %xmm6
	vaddsd	%xmm2, %xmm6, %xmm6
	vmovsd	%xmm6, -64(%r9,%rax,8)
	vmulsd	-56(%rdx,%rax,8), %xmm0, %xmm6
	vmulsd	(%r11), %xmm6, %xmm6
	vfmadd231sd	-56(%r9,%rax,8), %xmm1, %xmm6
	vaddsd	%xmm2, %xmm6, %xmm6
	vmovsd	%xmm6, -56(%r9,%rax,8)
	vmulsd	-48(%rdx,%rax,8), %xmm0, %xmm6
	vmulsd	(%r11), %xmm6, %xmm6
	vfmadd231sd	-48(%r9,%rax,8), %xmm1, %xmm6
	vaddsd	%xmm2, %xmm6, %xmm6
	vmovsd	%xmm6, -48(%r9,%rax,8)
	vmulsd	-40(%rdx,%rax,8), %xmm0, %xmm6
	vmulsd	(%r11), %xmm6, %xmm6
	vfmadd231sd	-40(%r9,%rax,8), %xmm1, %xmm6
	vaddsd	%xmm2, %xmm6, %xmm6
	vmovsd	%xmm6, -40(%r9,%rax,8)
	vmulsd	-32(%rdx,%rax,8), %xmm0, %xmm6
	vmulsd	(%r11), %xmm6, %xmm6
	vfmadd231sd	-32(%r9,%rax,8), %xmm1, %xmm6
	vaddsd	%xmm2, %xmm6, %xmm6
	vmovsd	%xmm6, -32(%r9,%rax,8)
	vmulsd	-24(%rdx,%rax,8), %xmm0, %xmm6
	vmulsd	(%r11), %xmm6, %xmm6
	vfmadd231sd	-24(%r9,%rax,8), %xmm1, %xmm6
	vaddsd	%xmm2, %xmm6, %xmm6
	vmovsd	%xmm6, -24(%r9,%rax,8)
	vmulsd	-16(%rdx,%rax,8), %xmm0, %xmm6
	vmulsd	(%r11), %xmm6, %xmm6
	vfmadd231sd	-16(%r9,%rax,8), %xmm1, %xmm6
	vaddsd	%xmm2, %xmm6, %xmm6
	vmovsd	%xmm6, -16(%r9,%rax,8)
	vmulsd	-8(%rdx,%rax,8), %xmm0, %xmm6
	vmulsd	(%r11), %xmm6, %xmm6
	vfmadd231sd	-8(%r9,%rax,8), %xmm1, %xmm6
	vaddsd	%xmm2, %xmm6, %xmm6
	vmovsd	%xmm6, -8(%r9,%rax,8)
	vmulsd	(%rdx,%rax,8), %xmm0, %xmm6
	vmulsd	(%r11), %xmm6, %xmm6
	vfmadd231sd	(%r9,%rax,8), %xmm1, %xmm6
	vaddsd	%xmm2, %xmm6, %xmm6
	vmovsd	%xmm6, (%r9,%rax,8)
	addq	$10, %rax
	cmpq	$2609, %rax
	jne	-303
	jmp	-1060
	vmovddup	(%r11), %xmm6
	movl	$18, %eax
	nop
	vmulpd	-144(%rdx,%rax,8), %xmm3, %xmm7
	vmulpd	-128(%rdx,%rax,8), %xmm3, %xmm8
	vmulpd	%xmm7, %xmm6, %xmm7
	vmulpd	%xmm6, %xmm8, %xmm8
	vfmadd231pd	-144(%r9,%rax,8), %xmm4, %xmm7
	vfmadd231pd	-128(%r9,%rax,8), %xmm4, %xmm8
	vaddpd	%xmm5, %xmm7, %xmm7
	vaddpd	%xmm5, %xmm8, %xmm8
	vmovupd	%xmm7, -144(%r9,%rax,8)
	vmovupd	%xmm8, -128(%r9,%rax,8)
	vmulpd	-112(%rdx,%rax,8), %xmm3, %xmm7
	vmulpd	-96(%rdx,%rax,8), %xmm3, %xmm8
	vmulpd	%xmm7, %xmm6, %xmm7
	vmulpd	%xmm6, %xmm8, %xmm8
	vfmadd231pd	-112(%r9,%rax,8), %xmm4, %xmm7
	vfmadd231pd	-96(%r9,%rax,8), %xmm4, %xmm8
	vaddpd	%xmm5, %xmm7, %xmm7
	vaddpd	%xmm5, %xmm8, %xmm8
	vmovupd	%xmm7, -112(%r9,%rax,8)
	vmovupd	%xmm8, -96(%r9,%rax,8)
	vmulpd	-80(%rdx,%rax,8), %xmm3, %xmm7
	vmulpd	-64(%rdx,%rax,8), %xmm3, %xmm8
	vmulpd	%xmm7, %xmm6, %xmm7
	vmulpd	%xmm6, %xmm8, %xmm8
	vfmadd231pd	-80(%r9,%rax,8), %xmm4, %xmm7
	vfmadd231pd	-64(%r9,%rax,8), %xmm4, %xmm8
	vaddpd	%xmm5, %xmm7, %xmm7
	vaddpd	%xmm5, %xmm8, %xmm8
	vmovupd	%xmm7, -80(%r9,%rax,8)
	vmovupd	%xmm8, -64(%r9,%rax,8)
	vmulpd	-48(%rdx,%rax,8), %xmm3, %xmm7
	vmulpd	-32(%rdx,%rax,8), %xmm3, %xmm8
	vmulpd	%xmm7, %xmm6, %xmm7
	vmulpd	%xmm6, %xmm8, %xmm8
	vfmadd231pd	-48(%r9,%rax,8), %xmm4, %xmm7
	vfmadd231pd	-32(%r9,%rax,8), %xmm4, %xmm8
	vaddpd	%xmm5, %xmm7, %xmm7
	vaddpd	%xmm5, %xmm8, %xmm8
	vmovupd	%xmm7, -48(%r9,%rax,8)
	vmovupd	%xmm8, -32(%r9,%rax,8)
	vmulpd	-16(%rdx,%rax,8), %xmm3, %xmm7
	vmulpd	(%rdx,%rax,8), %xmm3, %xmm8
	vmulpd	%xmm7, %xmm6, %xmm7
	vmulpd	%xmm6, %xmm8, %xmm8
	vfmadd231pd	-16(%r9,%rax,8), %xmm4, %xmm7
	vfmadd231pd	(%r9,%rax,8), %xmm4, %xmm8
	vaddpd	%xmm5, %xmm7, %xmm7
	vaddpd	%xmm5, %xmm8, %xmm8
	vmovupd	%xmm7, -16(%r9,%rax,8)
	vmovupd	%xmm8, (%r9,%rax,8)
	addq	$20, %rax
	cmpq	$2618, %rax
	jne	-302
	jmp	-1379
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
