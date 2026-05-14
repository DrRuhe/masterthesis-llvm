	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	leaq	-7(%rip), %rax
	movabsq	$-7978, %rdx
	addq	%rax, %rdx
	movabsq	$8, %rax
	movq	(%rdx,%rax), %rax
	movq	(%rax), %rbx
	leaq	120(%rbx), %rcx
	movq	$-2, %r11
	xorl	%esi, %esi
	movabsq	$-1584, %rax
	vmovapd	(%rdx,%rax), %xmm0
	movq	%rbx, %rbp
	movq	%rbx, %r8
	jmp	50
	nopw	%cs:(%rax,%rax)
	incq	%rsi
	addq	$32000, %rcx
	addq	$32000, %r8
	addq	$8, %rbp
	incq	%r11
	cmpq	$4000, %rsi
	je	1369
	testq	%rsi, %rsi
	je	-42
	movq	%rbp, -40(%rsp)
	movq	%r11, -32(%rsp)
	shrq	%r11
	leal	1(%r11), %eax
	movzbl	%al, %eax
	andl	$7, %eax
	imulq	$64000, %rax, %r10
	incq	%r11
	andq	$-8, %r11
	movq	%r11, -24(%rsp)
	leaq	-2(%rsi), %rax
	movq	%rax, -8(%rsp)
	shrq	%rax
	incq	%rax
	movq	%rax, -16(%rsp)
	imulq	$32000, %rsi, %r14
	addq	%rbx, %r14
	movq	$-2, %r15
	movq	%rbx, %rax
	xorl	%r13d, %r13d
	jmp	61
	nopw	%cs:(%rax,%rax)
	vmovsd	(%r14), %xmm1
	imulq	$32000, %r13, %rdx
	addq	%rbx, %rdx
	vdivsd	(%rdx,%r13,8), %xmm1, %xmm1
	vmovsd	%xmm1, (%r14,%r13,8)
	incq	%r13
	addq	$8, %rax
	incq	%r15
	cmpq	%rsi, %r13
	je	594
	testq	%r13, %r13
	je	-51
	leaq	(%r14,%r13,8), %rbp
	vmovsd	(%r14,%r13,8), %xmm1
	cmpq	$1, %r13
	jne	13
	xorl	%edx, %edx
	jmp	519
	nopw	(%rax,%rax)
	movq	%r15, %rdi
	shrq	%rdi
	leaq	-2(%r13), %r9
	cmpq	$14, %r9
	jae	7
	xorl	%edx, %edx
	jmp	377
	leaq	1(%rdi), %r11
	andq	$-8, %r11
	movq	%rax, %r12
	xorl	%edx, %edx
	nopw	%cs:(%rax,%rax)
	vmovsd	-120(%rcx,%rdx,8), %xmm2
	vfnmadd132sd	(%r12), %xmm1, %xmm2
	vmovsd	%xmm2, (%rbp)
	vmovsd	-112(%rcx,%rdx,8), %xmm1
	vfnmadd132sd	32000(%r12), %xmm2, %xmm1
	vmovsd	%xmm1, (%rbp)
	vmovsd	-104(%rcx,%rdx,8), %xmm2
	vfnmadd132sd	64000(%r12), %xmm1, %xmm2
	vmovsd	%xmm2, (%rbp)
	vmovsd	-96(%rcx,%rdx,8), %xmm1
	vfnmadd132sd	96000(%r12), %xmm2, %xmm1
	vmovsd	%xmm1, (%rbp)
	vmovsd	-88(%rcx,%rdx,8), %xmm2
	vfnmadd132sd	128000(%r12), %xmm1, %xmm2
	vmovsd	%xmm2, (%rbp)
	vmovsd	-80(%rcx,%rdx,8), %xmm1
	vfnmadd132sd	160000(%r12), %xmm2, %xmm1
	vmovsd	%xmm1, (%rbp)
	vmovsd	-72(%rcx,%rdx,8), %xmm2
	vfnmadd132sd	192000(%r12), %xmm1, %xmm2
	vmovsd	%xmm2, (%rbp)
	vmovsd	-64(%rcx,%rdx,8), %xmm1
	vfnmadd132sd	224000(%r12), %xmm2, %xmm1
	vmovsd	%xmm1, (%rbp)
	vmovsd	-56(%rcx,%rdx,8), %xmm2
	vfnmadd132sd	256000(%r12), %xmm1, %xmm2
	vmovsd	%xmm2, (%rbp)
	vmovsd	-48(%rcx,%rdx,8), %xmm1
	vfnmadd132sd	288000(%r12), %xmm2, %xmm1
	vmovsd	%xmm1, (%rbp)
	vmovsd	-40(%rcx,%rdx,8), %xmm2
	vfnmadd132sd	320000(%r12), %xmm1, %xmm2
	vmovsd	%xmm2, (%rbp)
	vmovsd	-32(%rcx,%rdx,8), %xmm1
	vfnmadd132sd	352000(%r12), %xmm2, %xmm1
	vmovsd	%xmm1, (%rbp)
	vmovsd	-24(%rcx,%rdx,8), %xmm2
	vfnmadd132sd	384000(%r12), %xmm1, %xmm2
	vmovsd	%xmm2, (%rbp)
	vmovsd	-16(%rcx,%rdx,8), %xmm1
	vfnmadd132sd	416000(%r12), %xmm2, %xmm1
	vmovsd	%xmm1, (%rbp)
	vmovsd	-8(%rcx,%rdx,8), %xmm2
	vfnmadd132sd	448000(%r12), %xmm1, %xmm2
	vmovsd	%xmm2, (%rbp)
	vmovsd	(%rcx,%rdx,8), %xmm1
	vfnmadd132sd	480000(%r12), %xmm2, %xmm1
	vmovsd	%xmm1, (%rbp)
	addq	$16, %rdx
	addq	$512000, %r12
	addq	$-8, %r11
	jne	-352
	shrq	%r9
	incq	%r9
	testb	$7, %r9b
	je	91
	incb	%dil
	movzbl	%dil, %edi
	andl	$7, %edi
	imulq	$64000, %rdi, %rdi
	imulq	$32000, %rdx, %r9
	addq	%rax, %r9
	xorl	%r11d, %r11d
	nopw	(%rax,%rax)
	vmovsd	(%r8,%rdx,8), %xmm2
	vfnmadd132sd	(%r9,%r11), %xmm1, %xmm2
	vmovsd	%xmm2, (%rbp)
	vmovsd	8(%r8,%rdx,8), %xmm1
	vfnmadd132sd	32000(%r9,%r11), %xmm2, %xmm1
	vmovsd	%xmm1, (%rbp)
	addq	$2, %rdx
	addq	$64000, %r11
	cmpq	%r11, %rdi
	jne	-55
	testb	$1, %r13b
	je	-588
	vmovsd	(%r14,%rdx,8), %xmm2
	imulq	$32000, %rdx, %rdx
	addq	%rbx, %rdx
	vfnmadd231sd	(%rdx,%r13,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%rbp)
	jmp	-620
	nopw	%cs:(%rax,%rax)
	movq	-40(%rsp), %rbp
	movq	%rbp, %r15
	movq	%rsi, %r12
	cmpq	$1, %rsi
	movq	-32(%rsp), %r11
	jne	94
	vmovsd	32000(%rbx), %xmm1
	vxorpd	%xmm0, %xmm1, %xmm1
	xorl	%eax, %eax
	nopw	%cs:(%rax,%rax)
	vmovsd	8(%rbx,%rax,8), %xmm2
	vfmadd213sd	32008(%rbx,%rax,8), %xmm1, %xmm2
	vmovsd	%xmm2, 32008(%rbx,%rax,8)
	incq	%rax
	cmpq	$3999, %rax
	jne	-36
	jmp	-873
	nopl	(%rax)
	incq	%r12
	addq	$8, %r15
	cmpq	$4000, %r12
	je	-900
	leaq	(%r14,%r12,8), %rdx
	vmovsd	(%r14,%r12,8), %xmm1
	cmpq	$14, -8(%rsp)
	jae	10
	xorl	%r13d, %r13d
	jmp	354
	nop
	movq	-24(%rsp), %rdi
	movq	%r15, %r9
	xorl	%r13d, %r13d
	nopl	(%rax,%rax)
	vmovsd	-120(%rcx,%r13,8), %xmm2
	vfnmadd132sd	(%r9), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdx)
	vmovsd	-112(%rcx,%r13,8), %xmm1
	vfnmadd132sd	32000(%r9), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdx)
	vmovsd	-104(%rcx,%r13,8), %xmm2
	vfnmadd132sd	64000(%r9), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdx)
	vmovsd	-96(%rcx,%r13,8), %xmm1
	vfnmadd132sd	96000(%r9), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdx)
	vmovsd	-88(%rcx,%r13,8), %xmm2
	vfnmadd132sd	128000(%r9), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdx)
	vmovsd	-80(%rcx,%r13,8), %xmm1
	vfnmadd132sd	160000(%r9), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdx)
	vmovsd	-72(%rcx,%r13,8), %xmm2
	vfnmadd132sd	192000(%r9), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdx)
	vmovsd	-64(%rcx,%r13,8), %xmm1
	vfnmadd132sd	224000(%r9), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdx)
	vmovsd	-56(%rcx,%r13,8), %xmm2
	vfnmadd132sd	256000(%r9), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdx)
	vmovsd	-48(%rcx,%r13,8), %xmm1
	vfnmadd132sd	288000(%r9), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdx)
	vmovsd	-40(%rcx,%r13,8), %xmm2
	vfnmadd132sd	320000(%r9), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdx)
	vmovsd	-32(%rcx,%r13,8), %xmm1
	vfnmadd132sd	352000(%r9), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdx)
	vmovsd	-24(%rcx,%r13,8), %xmm2
	vfnmadd132sd	384000(%r9), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdx)
	vmovsd	-16(%rcx,%r13,8), %xmm1
	vfnmadd132sd	416000(%r9), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdx)
	vmovsd	-8(%rcx,%r13,8), %xmm2
	vfnmadd132sd	448000(%r9), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdx)
	vmovsd	(%rcx,%r13,8), %xmm1
	vfnmadd132sd	480000(%r9), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdx)
	addq	$16, %r13
	addq	$512000, %r9
	addq	$-8, %rdi
	jne	-336
	testb	$7, -16(%rsp)
	je	78
	imulq	$32000, %r13, %rax
	addq	%r15, %rax
	xorl	%edi, %edi
	nopw	%cs:(%rax,%rax)
	vmovsd	(%r8,%r13,8), %xmm2
	vfnmadd132sd	(%rax,%rdi), %xmm1, %xmm2
	vmovsd	%xmm2, (%rdx)
	vmovsd	8(%r8,%r13,8), %xmm1
	vfnmadd132sd	32000(%rax,%rdi), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdx)
	addq	$2, %r13
	addq	$64000, %rdi
	cmpq	%rdi, %r10
	jne	-53
	testb	$1, %sil
	je	-495
	vmovsd	(%r14,%r13,8), %xmm2
	imulq	$32000, %r13, %rax
	addq	%rbx, %rax
	vfnmadd231sd	(%rax,%r12,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%rdx)
	jmp	-526
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
