	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$64, %rsp
	leaq	-7(%rip), %rcx
	movabsq	$-7934, %rax
	addq	%rcx, %rax
	movabsq	$160, %rcx
	movq	(%rax,%rcx), %rcx
	movq	(%rcx), %rdx
	movabsq	$96, %rcx
	movq	(%rax,%rcx), %rcx
	movq	(%rcx), %r10
	movabsq	$64, %rcx
	movq	(%rax,%rcx), %rcx
	movq	(%rcx), %rsi
	movabsq	$72, %rcx
	movq	(%rax,%rcx), %rcx
	movq	(%rcx), %rcx
	leaq	16048(%rcx), %rdi
	movq	%rdi, -40(%rsp)
	leaq	96016(%rdx), %rdi
	movq	%rdi, 24(%rsp)
	leaq	16048(%rsi), %rdi
	movq	%rdi, -48(%rsp)
	leaq	16024(%rcx), %rdi
	movq	%rdi, -56(%rsp)
	movq	%rdx, -72(%rsp)
	leaq	48016(%rdx), %rdx
	movq	%rdx, 16(%rsp)
	leaq	16024(%rsi), %rdx
	movq	%rdx, -64(%rsp)
	leaq	31984008(%r10), %rdx
	movq	%rdx, 8(%rsp)
	movq	%rcx, -88(%rsp)
	leaq	31984(%rcx), %rcx
	movq	%rcx, (%rsp)
	movq	%rsi, -80(%rsp)
	leaq	31984(%rsi), %rcx
	movq	%rcx, -8(%rsp)
	movabsq	$-1632, %rcx
	vmovsd	(%rax,%rcx), %xmm0
	movabsq	$-1624, %rcx
	vmovsd	(%rax,%rcx), %xmm1
	movabsq	$-1616, %rcx
	vmovsd	(%rax,%rcx), %xmm2
	movabsq	$-1608, %rcx
	vmovsd	(%rax,%rcx), %xmm3
	movabsq	$-1600, %rcx
	vmovsd	(%rax,%rcx), %xmm4
	leaq	32048(%r10), %rcx
	movq	%rcx, -16(%rsp)
	leaq	32024(%r10), %rcx
	movq	%rcx, -24(%rsp)
	movl	$1, %ecx
	movq	%r10, 40(%rsp)
	movq	%rax, -32(%rsp)
	nopw	%cs:(%rax,%rax)
	movq	%rcx, 32(%rsp)
	movl	$1, %r15d
	movq	-8(%rsp), %rdx
	movq	(%rsp), %rsi
	movq	8(%rsp), %rcx
	movq	-64(%rsp), %r8
	movq	16(%rsp), %rdi
	movq	-56(%rsp), %r10
	movq	-48(%rsp), %r11
	movq	24(%rsp), %r12
	movq	-40(%rsp), %r14
	xorl	%r9d, %r9d
	nopw	(%rax,%rax)
	movq	%r12, -128(%rsp)
	movq	%rdi, -120(%rsp)
	movq	%rcx, -96(%rsp)
	movq	%r9, -104(%rsp)
	imulq	$16000, %r9, %r12
	movq	-80(%rsp), %rbp
	leaq	(%r12,%rbp), %r13
	addq	$16000, %r13
	leaq	31992(%rbp,%r12), %rcx
	movq	%rcx, -112(%rsp)
	movq	-88(%rsp), %rdi
	leaq	16000(%rdi,%r12), %rbx
	leaq	31992(%rdi,%r12), %r12
	movabsq	$4607182418800017408, %rcx
	movq	40(%rsp), %r9
	movq	%rcx, (%r9,%r15,8)
	imulq	$16000, %r15, %rcx
	movq	$0, (%rbp,%rcx)
	movq	%r9, %rbp
	vmovsd	(%r9,%r15,8), %xmm5
	vmovsd	%xmm5, (%rdi,%rcx)
	cmpq	%r12, %r13
	setae	%cl
	cmpq	-112(%rsp), %rbx
	setae	%r12b
	orb	%cl, %r12b
	jne	250
	movq	-120(%rsp), %rdi
	movq	%rdi, %r9
	xorl	%ebx, %ebx
	movq	-128(%rsp), %r12
	nopw	%cs:(%rax,%rax)
	vmovsd	-24(%r8,%rbx,8), %xmm5
	vfmadd132sd	%xmm1, %xmm0, %xmm5
	vdivsd	%xmm5, %xmm2, %xmm6
	vmovsd	%xmm6, -16(%r8,%rbx,8)
	vmulsd	-32008(%r9), %xmm3, %xmm6
	vfmadd231sd	-32016(%r9), %xmm4, %xmm6
	vfmadd231sd	-32000(%r9), %xmm4, %xmm6
	vfmadd231sd	-24(%r10,%rbx,8), %xmm2, %xmm6
	vdivsd	%xmm5, %xmm6, %xmm5
	vmovsd	%xmm5, -16(%r10,%rbx,8)
	vmovsd	-16(%r8,%rbx,8), %xmm5
	vfmadd132sd	%xmm1, %xmm0, %xmm5
	vdivsd	%xmm5, %xmm2, %xmm6
	vmovsd	%xmm6, -8(%r8,%rbx,8)
	vmulsd	-16008(%r9), %xmm3, %xmm6
	vfmadd231sd	-16016(%r9), %xmm4, %xmm6
	vfmadd231sd	-16000(%r9), %xmm4, %xmm6
	vfmadd231sd	-16(%r10,%rbx,8), %xmm2, %xmm6
	vdivsd	%xmm5, %xmm6, %xmm5
	vmovsd	%xmm5, -8(%r10,%rbx,8)
	vmovsd	-8(%r8,%rbx,8), %xmm5
	vfmadd132sd	%xmm1, %xmm0, %xmm5
	vdivsd	%xmm5, %xmm2, %xmm6
	vmovsd	%xmm6, (%r8,%rbx,8)
	vmulsd	-8(%r9), %xmm3, %xmm6
	vfmadd231sd	-16(%r9), %xmm4, %xmm6
	vfmadd231sd	(%r9), %xmm4, %xmm6
	vfmadd231sd	-8(%r10,%rbx,8), %xmm2, %xmm6
	vdivsd	%xmm5, %xmm6, %xmm5
	vmovsd	%xmm5, (%r10,%rbx,8)
	addq	$3, %rbx
	addq	$48000, %r9
	cmpq	$1998, %rbx
	jne	-216
	jmp	395
	nopl	(%rax)
	vmovsd	(%r13), %xmm5
	vmovsd	(%rbx), %xmm6
	movq	-128(%rsp), %r12
	movq	%r12, %rbx
	xorl	%r9d, %r9d
	movq	-120(%rsp), %rdi
	nopw	(%rax,%rax)
	vfmadd213sd	%xmm0, %xmm1, %xmm5
	vdivsd	%xmm5, %xmm2, %xmm7
	vmovsd	%xmm7, -40(%r11,%r9,8)
	vmulsd	-80008(%rbx), %xmm3, %xmm8
	vfmadd231sd	-80016(%rbx), %xmm4, %xmm8
	vfmadd231sd	-80000(%rbx), %xmm4, %xmm8
	vfmadd231sd	%xmm6, %xmm2, %xmm8
	vdivsd	%xmm5, %xmm8, %xmm5
	vmovsd	%xmm5, -40(%r14,%r9,8)
	vfmadd213sd	%xmm0, %xmm1, %xmm7
	vdivsd	%xmm7, %xmm2, %xmm6
	vmovsd	%xmm6, -32(%r11,%r9,8)
	vmulsd	-64008(%rbx), %xmm3, %xmm8
	vfmadd231sd	-64016(%rbx), %xmm4, %xmm8
	vfmadd231sd	-64000(%rbx), %xmm4, %xmm8
	vfmadd231sd	%xmm5, %xmm2, %xmm8
	vdivsd	%xmm7, %xmm8, %xmm5
	vmovsd	%xmm5, -32(%r14,%r9,8)
	vfmadd213sd	%xmm0, %xmm1, %xmm6
	vdivsd	%xmm6, %xmm2, %xmm7
	vmovsd	%xmm7, -24(%r11,%r9,8)
	vmulsd	-48008(%rbx), %xmm3, %xmm8
	vfmadd231sd	-48016(%rbx), %xmm4, %xmm8
	vfmadd231sd	-48000(%rbx), %xmm4, %xmm8
	vfmadd231sd	%xmm5, %xmm2, %xmm8
	vdivsd	%xmm6, %xmm8, %xmm5
	vmovsd	%xmm5, -24(%r14,%r9,8)
	vfmadd213sd	%xmm0, %xmm1, %xmm7
	vdivsd	%xmm7, %xmm2, %xmm6
	vmovsd	%xmm6, -16(%r11,%r9,8)
	vmulsd	-32008(%rbx), %xmm3, %xmm8
	vfmadd231sd	-32016(%rbx), %xmm4, %xmm8
	vfmadd231sd	-32000(%rbx), %xmm4, %xmm8
	vfmadd231sd	%xmm5, %xmm2, %xmm8
	vdivsd	%xmm7, %xmm8, %xmm5
	vmovsd	%xmm5, -16(%r14,%r9,8)
	vfmadd213sd	%xmm0, %xmm1, %xmm6
	vdivsd	%xmm6, %xmm2, %xmm7
	vmovsd	%xmm7, -8(%r11,%r9,8)
	vmulsd	-16008(%rbx), %xmm3, %xmm8
	vfmadd231sd	-16016(%rbx), %xmm4, %xmm8
	vfmadd231sd	-16000(%rbx), %xmm4, %xmm8
	vfmadd231sd	%xmm5, %xmm2, %xmm8
	vdivsd	%xmm6, %xmm8, %xmm6
	vmovsd	%xmm6, -8(%r14,%r9,8)
	vfmadd213sd	%xmm0, %xmm1, %xmm7
	vdivsd	%xmm7, %xmm2, %xmm5
	vmovsd	%xmm5, (%r11,%r9,8)
	vmulsd	-8(%rbx), %xmm3, %xmm8
	vfmadd231sd	-16(%rbx), %xmm4, %xmm8
	vfmadd231sd	(%rbx), %xmm4, %xmm8
	vfmadd231sd	%xmm6, %xmm2, %xmm8
	vdivsd	%xmm7, %xmm8, %xmm6
	vmovsd	%xmm6, (%r14,%r9,8)
	addq	$6, %r9
	addq	$96000, %rbx
	cmpq	$1998, %r9
	jne	-360
	movabsq	$4607182418800017408, %rcx
	movq	%rcx, 31984000(%rbp,%r15,8)
	incq	%r15
	movl	$1999, %r9d
	movq	-96(%rsp), %rcx
	movq	%rcx, %rbx
	nopl	(%rax,%rax)
	vmovsd	-15992(%rdx,%r9,8), %xmm5
	vmovsd	(%rbx), %xmm6
	vfmadd213sd	-15992(%rsi,%r9,8), %xmm5, %xmm6
	vmovsd	%xmm6, -16000(%rbx)
	vmovsd	-16000(%rdx,%r9,8), %xmm5
	vfmadd213sd	-16000(%rsi,%r9,8), %xmm6, %xmm5
	vmovsd	%xmm5, -32000(%rbx)
	vmovsd	-16008(%rdx,%r9,8), %xmm6
	vfmadd213sd	-16008(%rsi,%r9,8), %xmm5, %xmm6
	vmovsd	%xmm6, -48000(%rbx)
	vmovsd	-16016(%rdx,%r9,8), %xmm5
	vfmadd213sd	-16016(%rsi,%r9,8), %xmm6, %xmm5
	vmovsd	%xmm5, -64000(%rbx)
	vmovsd	-16024(%rdx,%r9,8), %xmm6
	vfmadd213sd	-16024(%rsi,%r9,8), %xmm5, %xmm6
	vmovsd	%xmm6, -80000(%rbx)
	vmovsd	-16032(%rdx,%r9,8), %xmm5
	vfmadd213sd	-16032(%rsi,%r9,8), %xmm6, %xmm5
	vmovsd	%xmm5, -96000(%rbx)
	vmovsd	-16040(%rdx,%r9,8), %xmm6
	vfmadd213sd	-16040(%rsi,%r9,8), %xmm5, %xmm6
	vmovsd	%xmm6, -112000(%rbx)
	vmovsd	-16048(%rdx,%r9,8), %xmm5
	vfmadd213sd	-16048(%rsi,%r9,8), %xmm6, %xmm5
	vmovsd	%xmm5, -128000(%rbx)
	vmovsd	-16056(%rdx,%r9,8), %xmm6
	vfmadd213sd	-16056(%rsi,%r9,8), %xmm5, %xmm6
	vmovsd	%xmm6, -144000(%rbx)
	addq	$-144000, %rbx
	addq	$-9, %r9
	cmpq	$1, %r9
	ja	-277
	movq	-104(%rsp), %r9
	incq	%r9
	addq	$16000, %r14
	addq	$8, %r12
	addq	$16000, %r11
	addq	$16000, %r10
	addq	$8, %rdi
	addq	$16000, %r8
	addq	$8, %rcx
	addq	$16000, %rsi
	addq	$16000, %rdx
	cmpq	$1999, %r15
	jne	-1184
	movl	$1, %r9d
	movq	-80(%rsp), %rsi
	movq	-88(%rsp), %rdi
	movq	-72(%rsp), %rcx
	movq	-64(%rsp), %r14
	movq	-24(%rsp), %r15
	movq	-56(%rsp), %rbx
	movq	-48(%rsp), %rdx
	movq	-16(%rsp), %r11
	movq	-40(%rsp), %r8
	xorl	%r12d, %r12d
	nopw	%cs:(%rax,%rax)
	movq	%r12, -120(%rsp)
	imulq	$16000, %r12, %r10
	movq	-80(%rsp), %r12
	leaq	(%r12,%r10), %rax
	addq	$16000, %rax
	movq	%rax, 56(%rsp)
	leaq	31992(%r12,%r10), %rax
	movq	%rax, -112(%rsp)
	movq	-88(%rsp), %r13
	leaq	16000(%r13,%r10), %rax
	movq	%rax, 48(%rsp)
	movq	%r10, -128(%rsp)
	addq	%r13, %r10
	addq	$31992, %r10
	movq	%r9, -96(%rsp)
	imulq	$16000, %r9, %rax
	movq	-72(%rsp), %r9
	movabsq	$4607182418800017408, %rbp
	movq	%rbp, (%r9,%rax)
	movq	$0, (%r12,%rax)
	vmovsd	(%r9,%rax), %xmm5
	movq	48(%rsp), %r9
	movq	%rax, -104(%rsp)
	vmovsd	%xmm5, (%r13,%rax)
	movq	56(%rsp), %rax
	cmpq	%r10, %rax
	setae	%r10b
	cmpq	-112(%rsp), %r9
	setae	%bpl
	orb	%r10b, %bpl
	jne	278
	xorl	%r12d, %r12d
	movabsq	$-1592, %r9
	movabsq	$-1584, %r10
	movabsq	$-1576, %r13
	movq	-32(%rsp), %rax
	vmovsd	(%rax,%r9), %xmm6
	vmovsd	(%rax,%r10), %xmm5
	vmovsd	-24(%r14,%r12,8), %xmm7
	vfmadd132sd	%xmm5, %xmm6, %xmm7
	vdivsd	%xmm7, %xmm4, %xmm8
	vmovsd	%xmm8, -16(%r14,%r12,8)
	vmovsd	(%rax,%r13), %xmm8
	vmulsd	-16016(%r15,%r12,8), %xmm8, %xmm9
	vfmadd231sd	-32016(%r15,%r12,8), %xmm2, %xmm9
	vfmadd231sd	-16(%r15,%r12,8), %xmm2, %xmm9
	vfmadd231sd	-24(%rbx,%r12,8), %xmm4, %xmm9
	vdivsd	%xmm7, %xmm9, %xmm7
	vmovsd	%xmm7, -16(%rbx,%r12,8)
	vmovsd	-16(%r14,%r12,8), %xmm7
	vfmadd132sd	%xmm5, %xmm6, %xmm7
	vdivsd	%xmm7, %xmm4, %xmm9
	vmovsd	%xmm9, -8(%r14,%r12,8)
	vmulsd	-16008(%r15,%r12,8), %xmm8, %xmm9
	vfmadd231sd	-32008(%r15,%r12,8), %xmm2, %xmm9
	vfmadd231sd	-8(%r15,%r12,8), %xmm2, %xmm9
	vfmadd231sd	-16(%rbx,%r12,8), %xmm4, %xmm9
	vdivsd	%xmm7, %xmm9, %xmm7
	vmovsd	%xmm7, -8(%rbx,%r12,8)
	vfmadd132sd	-8(%r14,%r12,8), %xmm6, %xmm5
	vdivsd	%xmm5, %xmm4, %xmm6
	vmovsd	%xmm6, (%r14,%r12,8)
	vmulsd	-16000(%r15,%r12,8), %xmm8, %xmm6
	vfmadd231sd	-32000(%r15,%r12,8), %xmm2, %xmm6
	vfmadd231sd	(%r15,%r12,8), %xmm2, %xmm6
	vfmadd231sd	-8(%rbx,%r12,8), %xmm4, %xmm6
	vdivsd	%xmm5, %xmm6, %xmm5
	vmovsd	%xmm5, (%rbx,%r12,8)
	addq	$3, %r12
	cmpq	$1998, %r12
	jne	-231
	jmp	448
	nopl	(%rax)
	vmovsd	(%rax), %xmm8
	vmovsd	(%r9), %xmm9
	xorl	%r12d, %r12d
	movabsq	$-1592, %r9
	movabsq	$-1584, %r10
	movabsq	$-1576, %r13
	movq	-32(%rsp), %rax
	nop
	vmovsd	(%rax,%r9), %xmm6
	vmovsd	(%rax,%r10), %xmm5
	vfmadd213sd	%xmm6, %xmm5, %xmm8
	vdivsd	%xmm8, %xmm4, %xmm10
	vmovsd	%xmm10, -40(%rdx,%r12,8)
	vmovsd	(%rax,%r13), %xmm7
	vmulsd	-16040(%r11,%r12,8), %xmm7, %xmm11
	vfmadd231sd	-32040(%r11,%r12,8), %xmm2, %xmm11
	vfmadd231sd	-40(%r11,%r12,8), %xmm2, %xmm11
	vfmadd231sd	%xmm9, %xmm4, %xmm11
	vdivsd	%xmm8, %xmm11, %xmm8
	vmovsd	%xmm8, -40(%r8,%r12,8)
	vfmadd213sd	%xmm6, %xmm5, %xmm10
	vdivsd	%xmm10, %xmm4, %xmm9
	vmovsd	%xmm9, -32(%rdx,%r12,8)
	vmulsd	-16032(%r11,%r12,8), %xmm7, %xmm11
	vfmadd231sd	-32032(%r11,%r12,8), %xmm2, %xmm11
	vfmadd231sd	-32(%r11,%r12,8), %xmm2, %xmm11
	vfmadd231sd	%xmm8, %xmm4, %xmm11
	vdivsd	%xmm10, %xmm11, %xmm8
	vmovsd	%xmm8, -32(%r8,%r12,8)
	vfmadd213sd	%xmm6, %xmm5, %xmm9
	vdivsd	%xmm9, %xmm4, %xmm10
	vmovsd	%xmm10, -24(%rdx,%r12,8)
	vmulsd	-16024(%r11,%r12,8), %xmm7, %xmm11
	vfmadd231sd	-32024(%r11,%r12,8), %xmm2, %xmm11
	vfmadd231sd	-24(%r11,%r12,8), %xmm2, %xmm11
	vfmadd231sd	%xmm8, %xmm4, %xmm11
	vdivsd	%xmm9, %xmm11, %xmm8
	vmovsd	%xmm8, -24(%r8,%r12,8)
	vfmadd213sd	%xmm6, %xmm5, %xmm10
	vdivsd	%xmm10, %xmm4, %xmm9
	vmovsd	%xmm9, -16(%rdx,%r12,8)
	vmulsd	-16016(%r11,%r12,8), %xmm7, %xmm11
	vfmadd231sd	-32016(%r11,%r12,8), %xmm2, %xmm11
	vfmadd231sd	-16(%r11,%r12,8), %xmm2, %xmm11
	vfmadd231sd	%xmm8, %xmm4, %xmm11
	vdivsd	%xmm10, %xmm11, %xmm8
	vmovsd	%xmm8, -16(%r8,%r12,8)
	vfmadd213sd	%xmm6, %xmm5, %xmm9
	vdivsd	%xmm9, %xmm4, %xmm10
	vmovsd	%xmm10, -8(%rdx,%r12,8)
	vmulsd	-16008(%r11,%r12,8), %xmm7, %xmm11
	vfmadd231sd	-32008(%r11,%r12,8), %xmm2, %xmm11
	vfmadd231sd	-8(%r11,%r12,8), %xmm2, %xmm11
	vfmadd231sd	%xmm8, %xmm4, %xmm11
	vdivsd	%xmm9, %xmm11, %xmm9
	vmovsd	%xmm9, -8(%r8,%r12,8)
	vfmadd213sd	%xmm6, %xmm10, %xmm5
	vdivsd	%xmm5, %xmm4, %xmm8
	vmovsd	%xmm8, (%rdx,%r12,8)
	vmulsd	-16000(%r11,%r12,8), %xmm7, %xmm6
	vfmadd231sd	-32000(%r11,%r12,8), %xmm2, %xmm6
	vfmadd231sd	(%r11,%r12,8), %xmm2, %xmm6
	vfmadd231sd	%xmm9, %xmm4, %xmm6
	vdivsd	%xmm5, %xmm6, %xmm9
	vmovsd	%xmm9, (%r8,%r12,8)
	addq	$6, %r12
	cmpq	$1998, %r12
	jne	-396
	movq	-72(%rsp), %r9
	movabsq	$4607182418800017408, %r10
	movq	-104(%rsp), %r12
	movq	%r10, 15992(%r9,%r12)
	movq	-128(%rsp), %r10
	vmovsd	31992(%r9,%r10), %xmm5
	movl	$3998, %r9d
	nopl	(%rax)
	vmovsd	(%rsi,%r9,8), %xmm6
	vfmadd213sd	(%rdi,%r9,8), %xmm5, %xmm6
	vmovsd	%xmm6, (%rcx,%r9,8)
	vmovsd	-8(%rsi,%r9,8), %xmm5
	vfmadd213sd	-8(%rdi,%r9,8), %xmm6, %xmm5
	vmovsd	%xmm5, -8(%rcx,%r9,8)
	vmovsd	-16(%rsi,%r9,8), %xmm6
	vfmadd213sd	-16(%rdi,%r9,8), %xmm5, %xmm6
	vmovsd	%xmm6, -16(%rcx,%r9,8)
	vmovsd	-24(%rsi,%r9,8), %xmm5
	vfmadd213sd	-24(%rdi,%r9,8), %xmm6, %xmm5
	vmovsd	%xmm5, -24(%rcx,%r9,8)
	vmovsd	-32(%rsi,%r9,8), %xmm6
	vfmadd213sd	-32(%rdi,%r9,8), %xmm5, %xmm6
	vmovsd	%xmm6, -32(%rcx,%r9,8)
	vmovsd	-40(%rsi,%r9,8), %xmm5
	vfmadd213sd	-40(%rdi,%r9,8), %xmm6, %xmm5
	vmovsd	%xmm5, -40(%rcx,%r9,8)
	vmovsd	-48(%rsi,%r9,8), %xmm6
	vfmadd213sd	-48(%rdi,%r9,8), %xmm5, %xmm6
	vmovsd	%xmm6, -48(%rcx,%r9,8)
	vmovsd	-56(%rsi,%r9,8), %xmm5
	vfmadd213sd	-56(%rdi,%r9,8), %xmm6, %xmm5
	vmovsd	%xmm5, -56(%rcx,%r9,8)
	vmovsd	-64(%rsi,%r9,8), %xmm6
	vfmadd213sd	-64(%rdi,%r9,8), %xmm5, %xmm6
	vmovsd	%xmm6, -64(%rcx,%r9,8)
	vmovsd	-72(%rsi,%r9,8), %xmm5
	vfmadd213sd	-72(%rdi,%r9,8), %xmm6, %xmm5
	vmovsd	%xmm5, -72(%rcx,%r9,8)
	vmovsd	-80(%rsi,%r9,8), %xmm6
	vfmadd213sd	-80(%rdi,%r9,8), %xmm5, %xmm6
	vmovsd	%xmm6, -80(%rcx,%r9,8)
	vmovsd	-88(%rsi,%r9,8), %xmm5
	vfmadd213sd	-88(%rdi,%r9,8), %xmm6, %xmm5
	vmovsd	%xmm5, -88(%rcx,%r9,8)
	vmovsd	-96(%rsi,%r9,8), %xmm6
	vfmadd213sd	-96(%rdi,%r9,8), %xmm5, %xmm6
	vmovsd	%xmm6, -96(%rcx,%r9,8)
	vmovsd	-104(%rsi,%r9,8), %xmm5
	vfmadd213sd	-104(%rdi,%r9,8), %xmm6, %xmm5
	vmovsd	%xmm5, -104(%rcx,%r9,8)
	vmovsd	-112(%rsi,%r9,8), %xmm6
	vfmadd213sd	-112(%rdi,%r9,8), %xmm5, %xmm6
	vmovsd	%xmm6, -112(%rcx,%r9,8)
	vmovsd	-120(%rsi,%r9,8), %xmm5
	vfmadd213sd	-120(%rdi,%r9,8), %xmm6, %xmm5
	vmovsd	%xmm5, -120(%rcx,%r9,8)
	vmovsd	-128(%rsi,%r9,8), %xmm6
	vfmadd213sd	-128(%rdi,%r9,8), %xmm5, %xmm6
	vmovsd	%xmm6, -128(%rcx,%r9,8)
	vmovsd	-136(%rsi,%r9,8), %xmm5
	vfmadd213sd	-136(%rdi,%r9,8), %xmm6, %xmm5
	vmovsd	%xmm5, -136(%rcx,%r9,8)
	addq	$-18, %r9
	cmpq	$2000, %r9
	jne	-401
	movq	-96(%rsp), %r9
	incq	%r9
	movq	-120(%rsp), %r12
	incq	%r12
	addq	$16000, %r8
	addq	$16000, %r11
	addq	$16000, %rdx
	addq	$16000, %rbx
	addq	$16000, %r15
	addq	$16000, %r14
	addq	$16000, %rcx
	addq	$16000, %rdi
	addq	$16000, %rsi
	cmpq	$1999, %r9
	jne	-1437
	movq	32(%rsp), %rcx
	cmpl	$1000, %ecx
	leal	1(%rcx), %ecx
	jne	-2769
	addq	$64, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
