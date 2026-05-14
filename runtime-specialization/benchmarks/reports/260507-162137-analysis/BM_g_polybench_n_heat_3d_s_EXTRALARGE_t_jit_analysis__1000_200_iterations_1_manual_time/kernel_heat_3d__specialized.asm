	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	leaq	-7(%rip), %rax
	movabsq	$-7946, %rdx
	addq	%rax, %rdx
	movabsq	$176, %rax
	movq	(%rdx,%rax), %rax
	movq	(%rax), %rax
	movabsq	$144, %rcx
	movq	(%rdx,%rcx), %rcx
	movq	(%rcx), %rcx
	leaq	321608(%rcx), %rsi
	movq	%rsi, -8(%rsp)
	leaq	638392(%rcx), %rsi
	movq	%rsi, -16(%rsp)
	leaq	1608(%rax), %rsi
	movq	%rsi, -24(%rsp)
	leaq	958392(%rax), %rsi
	movq	%rsi, -32(%rsp)
	leaq	321608(%rax), %r9
	leaq	638392(%rax), %r10
	leaq	1608(%rcx), %r11
	leaq	958392(%rcx), %rbx
	movl	$1, %edi
	movabsq	$-1584, %rsi
	vmovsd	(%rdx,%rsi), %xmm0
	movabsq	$-1576, %rsi
	vmovsd	(%rdx,%rsi), %xmm1
	movabsq	$-1616, %rsi
	vmovapd	(%rdx,%rsi), %xmm2
	movabsq	$-1600, %rsi
	vmovapd	(%rdx,%rsi), %xmm3
	jmp	19
	nop
	movl	-36(%rsp), %edi
	incl	%edi
	cmpl	$1001, %edi
	je	1770
	movl	%edi, -36(%rsp)
	movl	$1, %r14d
	movq	%rcx, %rdx
	movq	%rax, %rsi
	xorl	%r13d, %r13d
	jmp	42
	nopw	(%rax,%rax)
	incq	%r14
	incq	%r13
	addq	$320000, %rsi
	addq	$320000, %rdx
	cmpq	$199, %r14
	je	847
	imulq	$320000, %r13, %rdi
	movq	-8(%rsp), %r8
	leaq	(%r8,%rdi), %r15
	movq	-16(%rsp), %r8
	leaq	(%r8,%rdi), %r12
	movq	-24(%rsp), %r8
	leaq	(%r8,%rdi), %rbp
	addq	-32(%rsp), %rdi
	cmpq	%rdi, %r15
	setb	%dil
	cmpq	%r12, %rbp
	setb	%bpl
	testb	%bpl, %dil
	je	369
	movl	$1, %r15d
	movq	%rdx, %r12
	movq	%rsi, %rbp
	nopl	(%rax,%rax)
	incq	%r15
	movl	$40201, %edi
	nopl	(%rax,%rax)
	vmovsd	(%rbp,%rdi,8), %xmm4
	vmovsd	320000(%rbp,%rdi,8), %xmm5
	vfmadd231sd	%xmm0, %xmm4, %xmm5
	vaddsd	-320000(%rbp,%rdi,8), %xmm5, %xmm5
	vmovsd	1600(%rbp,%rdi,8), %xmm6
	vfmadd231sd	%xmm0, %xmm4, %xmm6
	vaddsd	-1600(%rbp,%rdi,8), %xmm6, %xmm6
	vmulsd	%xmm1, %xmm6, %xmm6
	vfmadd231sd	%xmm5, %xmm1, %xmm6
	vmovsd	8(%rbp,%rdi,8), %xmm5
	vfmadd231sd	%xmm0, %xmm4, %xmm5
	vaddsd	-8(%rbp,%rdi,8), %xmm5, %xmm5
	vfmadd213sd	%xmm6, %xmm1, %xmm5
	vaddsd	%xmm5, %xmm4, %xmm4
	vmovsd	%xmm4, (%r12,%rdi,8)
	vmovsd	8(%rbp,%rdi,8), %xmm4
	vmovsd	320008(%rbp,%rdi,8), %xmm5
	vfmadd231sd	%xmm0, %xmm4, %xmm5
	vaddsd	-319992(%rbp,%rdi,8), %xmm5, %xmm5
	vmovsd	1608(%rbp,%rdi,8), %xmm6
	vfmadd231sd	%xmm0, %xmm4, %xmm6
	vaddsd	-1592(%rbp,%rdi,8), %xmm6, %xmm6
	vmulsd	%xmm1, %xmm6, %xmm6
	vfmadd231sd	%xmm5, %xmm1, %xmm6
	vmovsd	16(%rbp,%rdi,8), %xmm5
	vfmadd231sd	%xmm0, %xmm4, %xmm5
	vaddsd	(%rbp,%rdi,8), %xmm5, %xmm5
	vfmadd213sd	%xmm6, %xmm1, %xmm5
	vaddsd	%xmm5, %xmm4, %xmm4
	vmovsd	%xmm4, 8(%r12,%rdi,8)
	vmovsd	16(%rbp,%rdi,8), %xmm4
	vmovsd	320016(%rbp,%rdi,8), %xmm5
	vfmadd231sd	%xmm0, %xmm4, %xmm5
	vaddsd	-319984(%rbp,%rdi,8), %xmm5, %xmm5
	vmovsd	1616(%rbp,%rdi,8), %xmm6
	vfmadd231sd	%xmm0, %xmm4, %xmm6
	vaddsd	-1584(%rbp,%rdi,8), %xmm6, %xmm6
	vmulsd	%xmm1, %xmm6, %xmm6
	vfmadd231sd	%xmm5, %xmm1, %xmm6
	vmovsd	24(%rbp,%rdi,8), %xmm5
	vfmadd231sd	%xmm0, %xmm4, %xmm5
	vaddsd	8(%rbp,%rdi,8), %xmm5, %xmm5
	vfmadd213sd	%xmm6, %xmm1, %xmm5
	vaddsd	%xmm5, %xmm4, %xmm4
	vmovsd	%xmm4, 16(%r12,%rdi,8)
	addq	$3, %rdi
	cmpq	$40399, %rdi
	jne	-298
	addq	$1600, %rbp
	addq	$1600, %r12
	cmpq	$199, %r15
	jne	-341
	jmp	-458
	nopw	(%rax,%rax)
	movl	$1, %ebp
	movq	%rdx, %r15
	movq	%rsi, %r12
	nopl	(%rax,%rax)
	imulq	$320000, %r14, %rdi
	addq	%rax, %rdi
	imulq	$1600, %rbp, %r8
	incq	%rbp
	vmovupd	(%r8,%rdi), %xmm4
	movl	$40201, %edi
	nop
	vmovupd	(%r12,%rdi,8), %xmm6
	vmovupd	8(%r12,%rdi,8), %xmm5
	vmovupd	320000(%r12,%rdi,8), %xmm7
	vfmadd231pd	%xmm2, %xmm6, %xmm7
	vaddpd	-320000(%r12,%rdi,8), %xmm7, %xmm7
	vmovupd	1600(%r12,%rdi,8), %xmm8
	vfmadd231pd	%xmm2, %xmm6, %xmm8
	vaddpd	-1600(%r12,%rdi,8), %xmm8, %xmm8
	vmulpd	%xmm3, %xmm8, %xmm8
	vfmadd231pd	%xmm7, %xmm3, %xmm8
	vmovapd	%xmm2, %xmm7
	vfmadd213pd	%xmm5, %xmm6, %xmm7
	vaddpd	%xmm4, %xmm7, %xmm4
	vfmadd213pd	%xmm8, %xmm3, %xmm4
	vaddpd	%xmm4, %xmm6, %xmm4
	vmovupd	%xmm4, (%r15,%rdi,8)
	vmovupd	16(%r12,%rdi,8), %xmm4
	vmovupd	24(%r12,%rdi,8), %xmm6
	vmovupd	320016(%r12,%rdi,8), %xmm7
	vfmadd231pd	%xmm2, %xmm4, %xmm7
	vaddpd	-319984(%r12,%rdi,8), %xmm7, %xmm7
	vmovupd	1616(%r12,%rdi,8), %xmm8
	vfmadd231pd	%xmm2, %xmm4, %xmm8
	vaddpd	-1584(%r12,%rdi,8), %xmm8, %xmm8
	vmulpd	%xmm3, %xmm8, %xmm8
	vfmadd231pd	%xmm7, %xmm3, %xmm8
	vmovapd	%xmm2, %xmm7
	vfmadd213pd	%xmm6, %xmm4, %xmm7
	vaddpd	%xmm7, %xmm5, %xmm5
	vfmadd213pd	%xmm8, %xmm3, %xmm5
	vaddpd	%xmm5, %xmm4, %xmm4
	vmovupd	%xmm4, 16(%r15,%rdi,8)
	vmovupd	32(%r12,%rdi,8), %xmm5
	vmovupd	40(%r12,%rdi,8), %xmm4
	vmovupd	320032(%r12,%rdi,8), %xmm7
	vfmadd231pd	%xmm2, %xmm5, %xmm7
	vaddpd	-319968(%r12,%rdi,8), %xmm7, %xmm7
	vmovupd	1632(%r12,%rdi,8), %xmm8
	vfmadd231pd	%xmm2, %xmm5, %xmm8
	vaddpd	-1568(%r12,%rdi,8), %xmm8, %xmm8
	vmulpd	%xmm3, %xmm8, %xmm8
	vfmadd231pd	%xmm7, %xmm3, %xmm8
	vmovapd	%xmm2, %xmm7
	vfmadd213pd	%xmm4, %xmm5, %xmm7
	vaddpd	%xmm7, %xmm6, %xmm6
	vfmadd213pd	%xmm8, %xmm3, %xmm6
	vaddpd	%xmm6, %xmm5, %xmm5
	vmovupd	%xmm5, 32(%r15,%rdi,8)
	addq	$6, %rdi
	cmpq	$40399, %rdi
	jne	-321
	addq	$1600, %r12
	addq	$1600, %r15
	cmpq	$199, %rbp
	jne	-380
	jmp	-865
	nopw	%cs:(%rax,%rax)
	movl	$1, %r14d
	movq	%rax, %r15
	movq	%rcx, %r12
	xorl	%r13d, %r13d
	jmp	48
	nopw	%cs:(%rax,%rax)
	incq	%r14
	incq	%r13
	addq	$320000, %r12
	addq	$320000, %r15
	cmpq	$199, %r14
	je	-993
	imulq	$320000, %r13, %rdx
	leaq	(%r9,%rdx), %rsi
	leaq	(%r10,%rdx), %rdi
	leaq	(%r11,%rdx), %r8
	addq	%rbx, %rdx
	cmpq	%rdx, %rsi
	setb	%dl
	cmpq	%rdi, %r8
	setb	%sil
	testb	%sil, %dl
	je	371
	movl	$1, %edx
	movq	%r15, %rsi
	movq	%r12, %rbp
	nopl	(%rax,%rax)
	incq	%rdx
	movl	$40201, %edi
	nopl	(%rax,%rax)
	vmovsd	(%rbp,%rdi,8), %xmm4
	vmovsd	320000(%rbp,%rdi,8), %xmm5
	vfmadd231sd	%xmm0, %xmm4, %xmm5
	vaddsd	-320000(%rbp,%rdi,8), %xmm5, %xmm5
	vmovsd	1600(%rbp,%rdi,8), %xmm6
	vfmadd231sd	%xmm0, %xmm4, %xmm6
	vaddsd	-1600(%rbp,%rdi,8), %xmm6, %xmm6
	vmulsd	%xmm1, %xmm6, %xmm6
	vfmadd231sd	%xmm5, %xmm1, %xmm6
	vmovsd	8(%rbp,%rdi,8), %xmm5
	vfmadd231sd	%xmm0, %xmm4, %xmm5
	vaddsd	-8(%rbp,%rdi,8), %xmm5, %xmm5
	vfmadd213sd	%xmm6, %xmm1, %xmm5
	vaddsd	%xmm5, %xmm4, %xmm4
	vmovsd	%xmm4, (%rsi,%rdi,8)
	vmovsd	8(%rbp,%rdi,8), %xmm4
	vmovsd	320008(%rbp,%rdi,8), %xmm5
	vfmadd231sd	%xmm0, %xmm4, %xmm5
	vaddsd	-319992(%rbp,%rdi,8), %xmm5, %xmm5
	vmovsd	1608(%rbp,%rdi,8), %xmm6
	vfmadd231sd	%xmm0, %xmm4, %xmm6
	vaddsd	-1592(%rbp,%rdi,8), %xmm6, %xmm6
	vmulsd	%xmm1, %xmm6, %xmm6
	vfmadd231sd	%xmm5, %xmm1, %xmm6
	vmovsd	16(%rbp,%rdi,8), %xmm5
	vfmadd231sd	%xmm0, %xmm4, %xmm5
	vaddsd	(%rbp,%rdi,8), %xmm5, %xmm5
	vfmadd213sd	%xmm6, %xmm1, %xmm5
	vaddsd	%xmm5, %xmm4, %xmm4
	vmovsd	%xmm4, 8(%rsi,%rdi,8)
	vmovsd	16(%rbp,%rdi,8), %xmm4
	vmovsd	320016(%rbp,%rdi,8), %xmm5
	vfmadd231sd	%xmm0, %xmm4, %xmm5
	vaddsd	-319984(%rbp,%rdi,8), %xmm5, %xmm5
	vmovsd	1616(%rbp,%rdi,8), %xmm6
	vfmadd231sd	%xmm0, %xmm4, %xmm6
	vaddsd	-1584(%rbp,%rdi,8), %xmm6, %xmm6
	vmulsd	%xmm1, %xmm6, %xmm6
	vfmadd231sd	%xmm5, %xmm1, %xmm6
	vmovsd	24(%rbp,%rdi,8), %xmm5
	vfmadd231sd	%xmm0, %xmm4, %xmm5
	vaddsd	8(%rbp,%rdi,8), %xmm5, %xmm5
	vfmadd213sd	%xmm6, %xmm1, %xmm5
	vaddsd	%xmm5, %xmm4, %xmm4
	vmovsd	%xmm4, 16(%rsi,%rdi,8)
	addq	$3, %rdi
	cmpq	$40399, %rdi
	jne	-295
	addq	$1600, %rbp
	addq	$1600, %rsi
	cmpq	$199, %rdx
	jne	-338
	jmp	-439
	nopw	(%rax,%rax)
	movl	$1, %ebp
	movq	%r15, %rdx
	movq	%r12, %rsi
	nopl	(%rax,%rax)
	imulq	$320000, %r14, %rdi
	addq	%rcx, %rdi
	imulq	$1600, %rbp, %r8
	incq	%rbp
	vmovupd	(%r8,%rdi), %xmm4
	movl	$40201, %edi
	nop
	vmovupd	(%rsi,%rdi,8), %xmm6
	vmovupd	8(%rsi,%rdi,8), %xmm5
	vmovupd	320000(%rsi,%rdi,8), %xmm7
	vfmadd231pd	%xmm2, %xmm6, %xmm7
	vaddpd	-320000(%rsi,%rdi,8), %xmm7, %xmm7
	vmovupd	1600(%rsi,%rdi,8), %xmm8
	vfmadd231pd	%xmm2, %xmm6, %xmm8
	vaddpd	-1600(%rsi,%rdi,8), %xmm8, %xmm8
	vmulpd	%xmm3, %xmm8, %xmm8
	vfmadd231pd	%xmm7, %xmm3, %xmm8
	vmovapd	%xmm2, %xmm7
	vfmadd213pd	%xmm5, %xmm6, %xmm7
	vaddpd	%xmm4, %xmm7, %xmm4
	vfmadd213pd	%xmm8, %xmm3, %xmm4
	vaddpd	%xmm4, %xmm6, %xmm4
	vmovupd	%xmm4, (%rdx,%rdi,8)
	vmovupd	16(%rsi,%rdi,8), %xmm4
	vmovupd	24(%rsi,%rdi,8), %xmm6
	vmovupd	320016(%rsi,%rdi,8), %xmm7
	vfmadd231pd	%xmm2, %xmm4, %xmm7
	vaddpd	-319984(%rsi,%rdi,8), %xmm7, %xmm7
	vmovupd	1616(%rsi,%rdi,8), %xmm8
	vfmadd231pd	%xmm2, %xmm4, %xmm8
	vaddpd	-1584(%rsi,%rdi,8), %xmm8, %xmm8
	vmulpd	%xmm3, %xmm8, %xmm8
	vfmadd231pd	%xmm7, %xmm3, %xmm8
	vmovapd	%xmm2, %xmm7
	vfmadd213pd	%xmm6, %xmm4, %xmm7
	vaddpd	%xmm7, %xmm5, %xmm5
	vfmadd213pd	%xmm8, %xmm3, %xmm5
	vaddpd	%xmm5, %xmm4, %xmm4
	vmovupd	%xmm4, 16(%rdx,%rdi,8)
	vmovupd	32(%rsi,%rdi,8), %xmm5
	vmovupd	40(%rsi,%rdi,8), %xmm4
	vmovupd	320032(%rsi,%rdi,8), %xmm7
	vfmadd231pd	%xmm2, %xmm5, %xmm7
	vaddpd	-319968(%rsi,%rdi,8), %xmm7, %xmm7
	vmovupd	1632(%rsi,%rdi,8), %xmm8
	vfmadd231pd	%xmm2, %xmm5, %xmm8
	vaddpd	-1568(%rsi,%rdi,8), %xmm8, %xmm8
	vmulpd	%xmm3, %xmm8, %xmm8
	vfmadd231pd	%xmm7, %xmm3, %xmm8
	vmovapd	%xmm2, %xmm7
	vfmadd213pd	%xmm4, %xmm5, %xmm7
	vaddpd	%xmm7, %xmm6, %xmm6
	vfmadd213pd	%xmm8, %xmm3, %xmm6
	vaddpd	%xmm6, %xmm5, %xmm5
	vmovupd	%xmm5, 32(%rdx,%rdi,8)
	addq	$6, %rdi
	cmpq	$40399, %rdi
	jne	-300
	addq	$1600, %rsi
	addq	$1600, %rdx
	cmpq	$199, %rbp
	jne	-359
	jmp	-828
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
