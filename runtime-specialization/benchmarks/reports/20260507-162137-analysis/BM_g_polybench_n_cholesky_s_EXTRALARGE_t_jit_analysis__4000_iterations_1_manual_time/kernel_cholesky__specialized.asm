	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$56, %rsp
	leaq	-7(%rip), %rax
	movabsq	$-7998, %rcx
	addq	%rax, %rcx
	movabsq	$88, %rax
	movq	(%rcx,%rax), %rax
	movq	(%rax), %rbx
	leaq	120(%rbx), %r14
	movq	$-2, %r15
	xorl	%r12d, %r12d
	vxorpd	%xmm2, %xmm2, %xmm2
	movabsq	$136, %rax
	movq	(%rcx,%rax), %rax
	movq	%rax, 16(%rsp)
	movq	%rbx, %rbp
	movq	%r14, 24(%rsp)
	jmp	73
	nopw	%cs:(%rax,%rax)
	callq	*16(%rsp)
	vxorpd	%xmm2, %xmm2, %xmm2
	imulq	$32000, %r12, %rax
	addq	%rbx, %rax
	leaq	(%rax,%r12,8), %rax
	vmovsd	%xmm0, (%rax)
	incq	%r12
	addq	$32000, %r14
	addq	$32000, %rbp
	incq	%r15
	cmpq	$4000, %r12
	je	1169
	testq	%r12, %r12
	je	732
	movq	%r15, 8(%rsp)
	movq	%r15, %rdx
	shrq	%rdx
	leal	1(%rdx), %eax
	movzbl	%al, %ecx
	andl	$7, %ecx
	incq	%rdx
	andq	$-8, %rdx
	leaq	-2(%r12), %rax
	movq	%rax, 32(%rsp)
	shrq	%rax
	incq	%rax
	movq	%rax, 40(%rsp)
	imulq	$32000, %r12, %rax
	addq	%rbx, %rax
	movq	$-2, %r8
	movq	%rbx, %r9
	movq	24(%rsp), %r10
	xorl	%r11d, %r11d
	movq	%r12, 48(%rsp)
	jmp	64
	nopw	(%rax,%rax)
	vmovsd	(%rax), %xmm0
	imulq	$32000, %r11, %rsi
	addq	%rbx, %rsi
	vdivsd	(%rsi,%r11,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rax,%r11,8)
	incq	%r11
	addq	$32000, %r10
	incq	%r8
	addq	$32000, %r9
	cmpq	%r12, %r11
	je	553
	testq	%r11, %r11
	je	-60
	leaq	(%rax,%r11,8), %r13
	vmovsd	(%rax,%r11,8), %xmm0
	cmpq	$1, %r11
	jne	20
	xorl	%edi, %edi
	jmp	493
	nopw	%cs:(%rax,%rax)
	movq	%rbx, %r12
	movq	%r8, %rsi
	shrq	%rsi
	leaq	-2(%r11), %r15
	cmpq	$14, %r15
	jae	7
	xorl	%edi, %edi
	jmp	354
	leaq	1(%rsi), %rbx
	andq	$-8, %rbx
	xorl	%edi, %edi
	nopw	%cs:(%rax,%rax)
	vmovsd	-120(%r14,%rdi,8), %xmm1
	vfnmadd132sd	-120(%r10,%rdi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%r13)
	vmovsd	-112(%r14,%rdi,8), %xmm0
	vfnmadd132sd	-112(%r10,%rdi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%r13)
	vmovsd	-104(%r14,%rdi,8), %xmm1
	vfnmadd132sd	-104(%r10,%rdi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%r13)
	vmovsd	-96(%r14,%rdi,8), %xmm0
	vfnmadd132sd	-96(%r10,%rdi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%r13)
	vmovsd	-88(%r14,%rdi,8), %xmm1
	vfnmadd132sd	-88(%r10,%rdi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%r13)
	vmovsd	-80(%r14,%rdi,8), %xmm0
	vfnmadd132sd	-80(%r10,%rdi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%r13)
	vmovsd	-72(%r14,%rdi,8), %xmm1
	vfnmadd132sd	-72(%r10,%rdi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%r13)
	vmovsd	-64(%r14,%rdi,8), %xmm0
	vfnmadd132sd	-64(%r10,%rdi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%r13)
	vmovsd	-56(%r14,%rdi,8), %xmm1
	vfnmadd132sd	-56(%r10,%rdi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%r13)
	vmovsd	-48(%r14,%rdi,8), %xmm0
	vfnmadd132sd	-48(%r10,%rdi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%r13)
	vmovsd	-40(%r14,%rdi,8), %xmm1
	vfnmadd132sd	-40(%r10,%rdi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%r13)
	vmovsd	-32(%r14,%rdi,8), %xmm0
	vfnmadd132sd	-32(%r10,%rdi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%r13)
	vmovsd	-24(%r14,%rdi,8), %xmm1
	vfnmadd132sd	-24(%r10,%rdi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%r13)
	vmovsd	-16(%r14,%rdi,8), %xmm0
	vfnmadd132sd	-16(%r10,%rdi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%r13)
	vmovsd	-8(%r14,%rdi,8), %xmm1
	vfnmadd132sd	-8(%r10,%rdi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%r13)
	vmovsd	(%r14,%rdi,8), %xmm0
	vfnmadd132sd	(%r10,%rdi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%r13)
	addq	$16, %rdi
	addq	$-8, %rbx
	jne	-332
	shrq	%r15
	incq	%r15
	testb	$7, %r15b
	je	70
	incb	%sil
	movzbl	%sil, %esi
	andl	$7, %esi
	nopw	%cs:(%rax,%rax)
	vmovsd	(%rbp,%rdi,8), %xmm1
	vfnmadd132sd	(%r9,%rdi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%r13)
	vmovsd	8(%rbp,%rdi,8), %xmm0
	vfnmadd132sd	8(%r9,%rdi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%r13)
	addq	$2, %rdi
	decq	%rsi
	jne	-46
	testb	$1, %r11b
	movq	%r12, %rbx
	movq	48(%rsp), %r12
	je	-572
	vmovsd	(%rax,%rdi,8), %xmm1
	imulq	$32000, %r11, %rsi
	addq	%rbx, %rsi
	vfnmadd231sd	(%rsi,%rdi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%r13)
	jmp	-604
	vmovsd	(%rax,%r12,8), %xmm0
	cmpq	$1, %r12
	jne	36
	xorl	%edi, %edi
	movq	8(%rsp), %r15
	jmp	407
	nopl	(%rax,%rax)
	vmovsd	(%rbx), %xmm0
	jmp	400
	nopl	(%rax)
	leaq	(%rax,%r12,8), %r8
	cmpq	$14, 32(%rsp)
	movq	8(%rsp), %r15
	jae	7
	xorl	%edi, %edi
	jmp	293
	xorl	%edi, %edi
	nopw	(%rax,%rax)
	vmovsd	-120(%r14,%rdi,8), %xmm1
	vfnmadd213sd	%xmm0, %xmm1, %xmm1
	vmovsd	%xmm1, (%r8)
	vmovsd	-112(%r14,%rdi,8), %xmm0
	vfnmadd213sd	%xmm1, %xmm0, %xmm0
	vmovsd	%xmm0, (%r8)
	vmovsd	-104(%r14,%rdi,8), %xmm1
	vfnmadd213sd	%xmm0, %xmm1, %xmm1
	vmovsd	%xmm1, (%r8)
	vmovsd	-96(%r14,%rdi,8), %xmm0
	vfnmadd213sd	%xmm1, %xmm0, %xmm0
	vmovsd	%xmm0, (%r8)
	vmovsd	-88(%r14,%rdi,8), %xmm1
	vfnmadd213sd	%xmm0, %xmm1, %xmm1
	vmovsd	%xmm1, (%r8)
	vmovsd	-80(%r14,%rdi,8), %xmm0
	vfnmadd213sd	%xmm1, %xmm0, %xmm0
	vmovsd	%xmm0, (%r8)
	vmovsd	-72(%r14,%rdi,8), %xmm1
	vfnmadd213sd	%xmm0, %xmm1, %xmm1
	vmovsd	%xmm1, (%r8)
	vmovsd	-64(%r14,%rdi,8), %xmm0
	vfnmadd213sd	%xmm1, %xmm0, %xmm0
	vmovsd	%xmm0, (%r8)
	vmovsd	-56(%r14,%rdi,8), %xmm1
	vfnmadd213sd	%xmm0, %xmm1, %xmm1
	vmovsd	%xmm1, (%r8)
	vmovsd	-48(%r14,%rdi,8), %xmm0
	vfnmadd213sd	%xmm1, %xmm0, %xmm0
	vmovsd	%xmm0, (%r8)
	vmovsd	-40(%r14,%rdi,8), %xmm1
	vfnmadd213sd	%xmm0, %xmm1, %xmm1
	vmovsd	%xmm1, (%r8)
	vmovsd	-32(%r14,%rdi,8), %xmm0
	vfnmadd213sd	%xmm1, %xmm0, %xmm0
	vmovsd	%xmm0, (%r8)
	vmovsd	-24(%r14,%rdi,8), %xmm1
	vfnmadd213sd	%xmm0, %xmm1, %xmm1
	vmovsd	%xmm1, (%r8)
	vmovsd	-16(%r14,%rdi,8), %xmm0
	vfnmadd213sd	%xmm1, %xmm0, %xmm0
	vmovsd	%xmm0, (%r8)
	vmovsd	-8(%r14,%rdi,8), %xmm1
	vfnmadd213sd	%xmm0, %xmm1, %xmm1
	vmovsd	%xmm1, (%r8)
	vmovsd	(%r14,%rdi,8), %xmm0
	vfnmadd213sd	%xmm1, %xmm0, %xmm0
	vmovsd	%xmm0, (%r8)
	addq	$16, %rdi
	addq	$-8, %rdx
	jne	-285
	testb	$7, 40(%rsp)
	je	53
	nopw	%cs:(%rax,%rax)
	vmovsd	(%rbp,%rdi,8), %xmm1
	vfnmadd213sd	%xmm0, %xmm1, %xmm1
	vmovsd	%xmm1, (%r8)
	vmovsd	8(%rbp,%rdi,8), %xmm0
	vfnmadd213sd	%xmm1, %xmm0, %xmm0
	vmovsd	%xmm0, (%r8)
	addq	$2, %rdi
	decq	%rcx
	jne	-41
	testb	$1, %r12b
	je	10
	vmovsd	(%rax,%rdi,8), %xmm1
	vfnmadd231sd	%xmm1, %xmm1, %xmm0
	vucomisd	%xmm2, %xmm0
	jb	-1219
	vsqrtsd	%xmm0, %xmm0, %xmm0
	jmp	-1220
	addq	$56, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
