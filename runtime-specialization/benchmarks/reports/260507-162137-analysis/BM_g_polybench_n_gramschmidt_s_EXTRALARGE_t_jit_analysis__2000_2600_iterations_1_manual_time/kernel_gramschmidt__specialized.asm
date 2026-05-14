	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$72, %rsp
	leaq	-7(%rip), %rax
	movabsq	$-7998, %rcx
	addq	%rax, %rcx
	movabsq	$192, %rax
	movq	(%rcx,%rax), %rax
	movq	(%rax), %r14
	movabsq	$104, %rax
	movq	(%rcx,%rax), %rax
	movq	(%rax), %r10
	movabsq	$72, %rax
	movq	%rcx, 48(%rsp)
	movq	(%rcx,%rax), %rax
	movq	(%rax), %rbp
	leaq	41600000(%r14), %rdx
	leaq	41600000(%rbp), %rax
	leaq	54080000(%r10), %rcx
	leaq	8(%r14), %rsi
	movq	%rsi, 24(%rsp)
	leaq	41579208(%rbp), %rsi
	movq	%rsi, 16(%rsp)
	leaq	8(%r10), %rsi
	movq	%rsi, 8(%rsp)
	movq	%rdx, 32(%rsp)
	cmpq	%rdx, %rbp
	setb	%dl
	cmpq	%rax, %r14
	setb	%sil
	andb	%dl, %sil
	cmpq	%rcx, %rbp
	setb	%cl
	leaq	20800(%r10), %rdx
	movq	%rdx, (%rsp)
	cmpq	%rax, %r10
	setb	%r11b
	andb	%cl, %r11b
	orb	%sil, %r11b
	leaq	312008(%r14), %rbx
	leaq	187208(%r14), %r12
	movl	$1, %r15d
	xorl	%r13d, %r13d
	vxorpd	%xmm3, %xmm3, %xmm3
	movq	%rbp, 40(%rsp)
	jmp	49
	nopw	%cs:(%rax,%rax)
	incq	%r13
	incq	%r15
	addq	$8, %r14
	addq	$8, %rbp
	addq	$8, %rbx
	addq	$8, %r12
	cmpq	$2600, %r13
	je	2608
	movl	$395200, %eax
	vxorpd	%xmm0, %xmm0, %xmm0
	nopl	(%rax)
	vmovsd	-395200(%r14,%rax), %xmm1
	vfmadd213sd	%xmm0, %xmm1, %xmm1
	vmovsd	-374400(%r14,%rax), %xmm0
	vfmadd213sd	%xmm1, %xmm0, %xmm0
	vmovsd	-353600(%r14,%rax), %xmm1
	vfmadd213sd	%xmm0, %xmm1, %xmm1
	vmovsd	-332800(%r14,%rax), %xmm0
	vfmadd213sd	%xmm1, %xmm0, %xmm0
	vmovsd	-312000(%r14,%rax), %xmm1
	vfmadd213sd	%xmm0, %xmm1, %xmm1
	vmovsd	-291200(%r14,%rax), %xmm0
	vfmadd213sd	%xmm1, %xmm0, %xmm0
	vmovsd	-270400(%r14,%rax), %xmm1
	vfmadd213sd	%xmm0, %xmm1, %xmm1
	vmovsd	-249600(%r14,%rax), %xmm0
	vfmadd213sd	%xmm1, %xmm0, %xmm0
	vmovsd	-228800(%r14,%rax), %xmm1
	vfmadd213sd	%xmm0, %xmm1, %xmm1
	vmovsd	-208000(%r14,%rax), %xmm0
	vfmadd213sd	%xmm1, %xmm0, %xmm0
	vmovsd	-187200(%r14,%rax), %xmm1
	vfmadd213sd	%xmm0, %xmm1, %xmm1
	vmovsd	-166400(%r14,%rax), %xmm0
	vfmadd213sd	%xmm1, %xmm0, %xmm0
	vmovsd	-145600(%r14,%rax), %xmm1
	vfmadd213sd	%xmm0, %xmm1, %xmm1
	vmovsd	-124800(%r14,%rax), %xmm0
	vfmadd213sd	%xmm1, %xmm0, %xmm0
	vmovsd	-104000(%r14,%rax), %xmm1
	vfmadd213sd	%xmm0, %xmm1, %xmm1
	vmovsd	-83200(%r14,%rax), %xmm0
	vfmadd213sd	%xmm1, %xmm0, %xmm0
	vmovsd	-62400(%r14,%rax), %xmm1
	vfmadd213sd	%xmm0, %xmm1, %xmm1
	vmovsd	-41600(%r14,%rax), %xmm0
	vfmadd213sd	%xmm1, %xmm0, %xmm0
	vmovsd	-20800(%r14,%rax), %xmm1
	vfmadd213sd	%xmm0, %xmm1, %xmm1
	vmovsd	(%r14,%rax), %xmm0
	vfmadd213sd	%xmm1, %xmm0, %xmm0
	addq	$416000, %rax
	cmpq	$41995200, %rax
	jne	-314
	vucomisd	%xmm3, %xmm0
	jb	16
	vsqrtsd	%xmm0, %xmm0, %xmm0
	jmp	76
	nopw	%cs:(%rax,%rax)
	movq	48(%rsp), %rax
	movq	%r13, 56(%rsp)
	movq	%rbx, %r13
	movq	%r10, %rbx
	movq	%r15, 64(%rsp)
	movq	%r12, %r15
	movl	%r11d, %r12d
	movabsq	$120, %rcx
	callq	*(%rax,%rcx)
	vxorpd	%xmm3, %xmm3, %xmm3
	movl	%r12d, %r11d
	movq	%r15, %r12
	movq	64(%rsp), %r15
	movq	%rbx, %r10
	movq	%r13, %rbx
	movq	56(%rsp), %r13
	imulq	$20800, %r13, %rcx
	leaq	(%r10,%rcx), %rax
	vmovsd	%xmm0, (%rax,%r13,8)
	testb	%r11b, %r11b
	je	276
	leaq	(%rax,%r13,8), %rdx
	movl	$187200, %esi
	nopw	%cs:(%rax,%rax)
	vmovsd	-187200(%r14,%rsi), %xmm0
	vdivsd	(%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, -187200(%rbp,%rsi)
	vmovsd	-166400(%r14,%rsi), %xmm0
	vdivsd	(%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, -166400(%rbp,%rsi)
	vmovsd	-145600(%r14,%rsi), %xmm0
	vdivsd	(%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, -145600(%rbp,%rsi)
	vmovsd	-124800(%r14,%rsi), %xmm0
	vdivsd	(%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, -124800(%rbp,%rsi)
	vmovsd	-104000(%r14,%rsi), %xmm0
	vdivsd	(%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, -104000(%rbp,%rsi)
	vmovsd	-83200(%r14,%rsi), %xmm0
	vdivsd	(%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, -83200(%rbp,%rsi)
	vmovsd	-62400(%r14,%rsi), %xmm0
	vdivsd	(%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, -62400(%rbp,%rsi)
	vmovsd	-41600(%r14,%rsi), %xmm0
	vdivsd	(%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, -41600(%rbp,%rsi)
	vmovsd	-20800(%r14,%rsi), %xmm0
	vdivsd	(%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, -20800(%rbp,%rsi)
	vmovsd	(%r14,%rsi), %xmm0
	vdivsd	(%rdx), %xmm0, %xmm0
	vmovsd	%xmm0, (%rbp,%rsi)
	addq	$208000, %rsi
	cmpq	$41787200, %rsi
	jne	-243
	jmp	373
	nopl	(%rax,%rax)
	vmovddup	%xmm0, %xmm0
	xorl	%edx, %edx
	nopw	%cs:(%rax,%rax)
	vmovsd	(%r14,%rdx), %xmm1
	vmovhpd	20800(%r14,%rdx), %xmm1, %xmm1
	vdivpd	%xmm0, %xmm1, %xmm1
	vmovlpd	%xmm1, (%rbp,%rdx)
	vmovhpd	%xmm1, 20800(%rbp,%rdx)
	vmovsd	41600(%r14,%rdx), %xmm1
	vmovhpd	62400(%r14,%rdx), %xmm1, %xmm1
	vdivpd	%xmm0, %xmm1, %xmm1
	vmovlpd	%xmm1, 41600(%rbp,%rdx)
	vmovhpd	%xmm1, 62400(%rbp,%rdx)
	vmovsd	83200(%r14,%rdx), %xmm1
	vmovhpd	104000(%r14,%rdx), %xmm1, %xmm1
	vdivpd	%xmm0, %xmm1, %xmm1
	vmovlpd	%xmm1, 83200(%rbp,%rdx)
	vmovhpd	%xmm1, 104000(%rbp,%rdx)
	vmovsd	124800(%r14,%rdx), %xmm1
	vmovhpd	145600(%r14,%rdx), %xmm1, %xmm1
	vdivpd	%xmm0, %xmm1, %xmm1
	vmovlpd	%xmm1, 124800(%rbp,%rdx)
	vmovhpd	%xmm1, 145600(%rbp,%rdx)
	vmovsd	166400(%r14,%rdx), %xmm1
	vmovhpd	187200(%r14,%rdx), %xmm1, %xmm1
	vdivpd	%xmm0, %xmm1, %xmm1
	vmovlpd	%xmm1, 166400(%rbp,%rdx)
	vmovhpd	%xmm1, 187200(%rbp,%rdx)
	vmovsd	208000(%r14,%rdx), %xmm1
	vmovhpd	228800(%r14,%rdx), %xmm1, %xmm1
	vdivpd	%xmm0, %xmm1, %xmm1
	vmovlpd	%xmm1, 208000(%rbp,%rdx)
	vmovhpd	%xmm1, 228800(%rbp,%rdx)
	vmovsd	249600(%r14,%rdx), %xmm1
	vmovhpd	270400(%r14,%rdx), %xmm1, %xmm1
	vdivpd	%xmm0, %xmm1, %xmm1
	vmovlpd	%xmm1, 249600(%rbp,%rdx)
	vmovhpd	%xmm1, 270400(%rbp,%rdx)
	vmovsd	291200(%r14,%rdx), %xmm1
	vmovhpd	312000(%r14,%rdx), %xmm1, %xmm1
	vdivpd	%xmm0, %xmm1, %xmm1
	vmovlpd	%xmm1, 291200(%rbp,%rdx)
	vmovhpd	%xmm1, 312000(%rbp,%rdx)
	addq	$332800, %rdx
	cmpq	$41600000, %rdx
	jne	-349
	cmpq	$2598, %r13
	ja	-1130
	movq	24(%rsp), %rdx
	leaq	(%rdx,%r13,8), %rdx
	movq	40(%rsp), %rsi
	leaq	(%rsi,%r13,8), %rsi
	movq	16(%rsp), %rdi
	leaq	(%rdi,%r13,8), %rdi
	imulq	$20808, %r13, %r8
	addq	8(%rsp), %r8
	addq	(%rsp), %rcx
	cmpq	%rdi, %rdx
	setb	%dil
	movq	32(%rsp), %r9
	cmpq	%r9, %rsi
	setb	%sil
	andb	%dil, %sil
	cmpq	%rcx, %rdx
	setb	%dl
	cmpq	%r9, %r8
	setb	%cl
	andb	%dl, %cl
	orb	%sil, %cl
	movq	%r12, %rdx
	movq	%rbx, %rsi
	movq	%r15, %rdi
	jmp	33
	nopw	(%rax,%rax)
	incq	%rdi
	addq	$8, %rsi
	addq	$8, %rdx
	cmpq	$2600, %rdi
	je	-1256
	leaq	(%rax,%rdi,8), %r8
	movq	$0, (%rax,%rdi,8)
	vxorpd	%xmm0, %xmm0, %xmm0
	xorl	%r9d, %r9d
	nopl	(%rax,%rax)
	vmovsd	(%rbp,%r9), %xmm1
	vfmadd132sd	-312000(%rsi,%r9), %xmm0, %xmm1
	vmovsd	%xmm1, (%r8)
	vmovsd	20800(%rbp,%r9), %xmm0
	vfmadd132sd	-291200(%rsi,%r9), %xmm1, %xmm0
	vmovsd	%xmm0, (%r8)
	vmovsd	41600(%rbp,%r9), %xmm1
	vfmadd132sd	-270400(%rsi,%r9), %xmm0, %xmm1
	vmovsd	%xmm1, (%r8)
	vmovsd	62400(%rbp,%r9), %xmm0
	vfmadd132sd	-249600(%rsi,%r9), %xmm1, %xmm0
	vmovsd	%xmm0, (%r8)
	vmovsd	83200(%rbp,%r9), %xmm1
	vfmadd132sd	-228800(%rsi,%r9), %xmm0, %xmm1
	vmovsd	%xmm1, (%r8)
	vmovsd	104000(%rbp,%r9), %xmm0
	vfmadd132sd	-208000(%rsi,%r9), %xmm1, %xmm0
	vmovsd	%xmm0, (%r8)
	vmovsd	124800(%rbp,%r9), %xmm1
	vfmadd132sd	-187200(%rsi,%r9), %xmm0, %xmm1
	vmovsd	%xmm1, (%r8)
	vmovsd	145600(%rbp,%r9), %xmm0
	vfmadd132sd	-166400(%rsi,%r9), %xmm1, %xmm0
	vmovsd	%xmm0, (%r8)
	vmovsd	166400(%rbp,%r9), %xmm1
	vfmadd132sd	-145600(%rsi,%r9), %xmm0, %xmm1
	vmovsd	%xmm1, (%r8)
	vmovsd	187200(%rbp,%r9), %xmm0
	vfmadd132sd	-124800(%rsi,%r9), %xmm1, %xmm0
	vmovsd	%xmm0, (%r8)
	vmovsd	208000(%rbp,%r9), %xmm1
	vfmadd132sd	-104000(%rsi,%r9), %xmm0, %xmm1
	vmovsd	%xmm1, (%r8)
	vmovsd	228800(%rbp,%r9), %xmm0
	vfmadd132sd	-83200(%rsi,%r9), %xmm1, %xmm0
	vmovsd	%xmm0, (%r8)
	vmovsd	249600(%rbp,%r9), %xmm1
	vfmadd132sd	-62400(%rsi,%r9), %xmm0, %xmm1
	vmovsd	%xmm1, (%r8)
	vmovsd	270400(%rbp,%r9), %xmm0
	vfmadd132sd	-41600(%rsi,%r9), %xmm1, %xmm0
	vmovsd	%xmm0, (%r8)
	vmovsd	291200(%rbp,%r9), %xmm1
	vfmadd132sd	-20800(%rsi,%r9), %xmm0, %xmm1
	vmovsd	%xmm1, (%r8)
	vmovsd	312000(%rbp,%r9), %xmm0
	vfmadd132sd	(%rsi,%r9), %xmm1, %xmm0
	vmovsd	%xmm0, (%r8)
	addq	$332800, %r9
	cmpq	$41600000, %r9
	jne	-413
	testb	%cl, %cl
	je	587
	xorl	%r9d, %r9d
	nopl	(%rax,%rax)
	vmovsd	(%rbp,%r9), %xmm0
	vmovsd	(%r8), %xmm1
	vfnmadd213sd	-312000(%rsi,%r9), %xmm0, %xmm1
	vmovsd	%xmm1, -312000(%rsi,%r9)
	vmovsd	20800(%rbp,%r9), %xmm0
	vmovsd	(%r8), %xmm1
	vfnmadd213sd	-291200(%rsi,%r9), %xmm0, %xmm1
	vmovsd	%xmm1, -291200(%rsi,%r9)
	vmovsd	41600(%rbp,%r9), %xmm0
	vmovsd	(%r8), %xmm1
	vfnmadd213sd	-270400(%rsi,%r9), %xmm0, %xmm1
	vmovsd	%xmm1, -270400(%rsi,%r9)
	vmovsd	62400(%rbp,%r9), %xmm0
	vmovsd	(%r8), %xmm1
	vfnmadd213sd	-249600(%rsi,%r9), %xmm0, %xmm1
	vmovsd	%xmm1, -249600(%rsi,%r9)
	vmovsd	83200(%rbp,%r9), %xmm0
	vmovsd	(%r8), %xmm1
	vfnmadd213sd	-228800(%rsi,%r9), %xmm0, %xmm1
	vmovsd	%xmm1, -228800(%rsi,%r9)
	vmovsd	104000(%rbp,%r9), %xmm0
	vmovsd	(%r8), %xmm1
	vfnmadd213sd	-208000(%rsi,%r9), %xmm0, %xmm1
	vmovsd	%xmm1, -208000(%rsi,%r9)
	vmovsd	124800(%rbp,%r9), %xmm0
	vmovsd	(%r8), %xmm1
	vfnmadd213sd	-187200(%rsi,%r9), %xmm0, %xmm1
	vmovsd	%xmm1, -187200(%rsi,%r9)
	vmovsd	145600(%rbp,%r9), %xmm0
	vmovsd	(%r8), %xmm1
	vfnmadd213sd	-166400(%rsi,%r9), %xmm0, %xmm1
	vmovsd	%xmm1, -166400(%rsi,%r9)
	vmovsd	166400(%rbp,%r9), %xmm0
	vmovsd	(%r8), %xmm1
	vfnmadd213sd	-145600(%rsi,%r9), %xmm0, %xmm1
	vmovsd	%xmm1, -145600(%rsi,%r9)
	vmovsd	187200(%rbp,%r9), %xmm0
	vmovsd	(%r8), %xmm1
	vfnmadd213sd	-124800(%rsi,%r9), %xmm0, %xmm1
	vmovsd	%xmm1, -124800(%rsi,%r9)
	vmovsd	208000(%rbp,%r9), %xmm0
	vmovsd	(%r8), %xmm1
	vfnmadd213sd	-104000(%rsi,%r9), %xmm0, %xmm1
	vmovsd	%xmm1, -104000(%rsi,%r9)
	vmovsd	228800(%rbp,%r9), %xmm0
	vmovsd	(%r8), %xmm1
	vfnmadd213sd	-83200(%rsi,%r9), %xmm0, %xmm1
	vmovsd	%xmm1, -83200(%rsi,%r9)
	vmovsd	249600(%rbp,%r9), %xmm0
	vmovsd	(%r8), %xmm1
	vfnmadd213sd	-62400(%rsi,%r9), %xmm0, %xmm1
	vmovsd	%xmm1, -62400(%rsi,%r9)
	vmovsd	270400(%rbp,%r9), %xmm0
	vmovsd	(%r8), %xmm1
	vfnmadd213sd	-41600(%rsi,%r9), %xmm0, %xmm1
	vmovsd	%xmm1, -41600(%rsi,%r9)
	vmovsd	291200(%rbp,%r9), %xmm0
	vmovsd	(%r8), %xmm1
	vfnmadd213sd	-20800(%rsi,%r9), %xmm0, %xmm1
	vmovsd	%xmm1, -20800(%rsi,%r9)
	vmovsd	312000(%rbp,%r9), %xmm0
	vmovsd	(%r8), %xmm1
	vfnmadd213sd	(%rsi,%r9), %xmm0, %xmm1
	vmovsd	%xmm1, (%rsi,%r9)
	addq	$332800, %r9
	cmpq	$41600000, %r9
	jne	-569
	jmp	-1054
	nop
	vmovddup	%xmm0, %xmm0
	xorl	%r8d, %r8d
	nopw	(%rax,%rax)
	vmovsd	-187200(%rdx,%r8), %xmm1
	vmovhpd	-166400(%rdx,%r8), %xmm1, %xmm1
	vmovsd	(%rbp,%r8), %xmm2
	vmovhpd	20800(%rbp,%r8), %xmm2, %xmm2
	vfnmadd213pd	%xmm1, %xmm0, %xmm2
	vmovlpd	%xmm2, -187200(%rdx,%r8)
	vmovhpd	%xmm2, -166400(%rdx,%r8)
	vmovsd	-145600(%rdx,%r8), %xmm1
	vmovhpd	-124800(%rdx,%r8), %xmm1, %xmm1
	vmovsd	41600(%rbp,%r8), %xmm2
	vmovhpd	62400(%rbp,%r8), %xmm2, %xmm2
	vfnmadd213pd	%xmm1, %xmm0, %xmm2
	vmovlpd	%xmm2, -145600(%rdx,%r8)
	vmovhpd	%xmm2, -124800(%rdx,%r8)
	vmovsd	-104000(%rdx,%r8), %xmm1
	vmovhpd	-83200(%rdx,%r8), %xmm1, %xmm1
	vmovsd	83200(%rbp,%r8), %xmm2
	vmovhpd	104000(%rbp,%r8), %xmm2, %xmm2
	vfnmadd213pd	%xmm1, %xmm0, %xmm2
	vmovlpd	%xmm2, -104000(%rdx,%r8)
	vmovhpd	%xmm2, -83200(%rdx,%r8)
	vmovsd	-62400(%rdx,%r8), %xmm1
	vmovhpd	-41600(%rdx,%r8), %xmm1, %xmm1
	vmovsd	124800(%rbp,%r8), %xmm2
	vmovhpd	145600(%rbp,%r8), %xmm2, %xmm2
	vfnmadd213pd	%xmm1, %xmm0, %xmm2
	vmovlpd	%xmm2, -62400(%rdx,%r8)
	vmovhpd	%xmm2, -41600(%rdx,%r8)
	vmovsd	-20800(%rdx,%r8), %xmm1
	vmovhpd	(%rdx,%r8), %xmm1, %xmm1
	vmovsd	166400(%rbp,%r8), %xmm2
	vmovhpd	187200(%rbp,%r8), %xmm2, %xmm2
	vfnmadd213pd	%xmm1, %xmm0, %xmm2
	vmovlpd	%xmm2, -20800(%rdx,%r8)
	vmovhpd	%xmm2, (%rdx,%r8)
	addq	$208000, %r8
	cmpq	$41600000, %r8
	jne	-334
	jmp	-1411
	addq	$72, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
