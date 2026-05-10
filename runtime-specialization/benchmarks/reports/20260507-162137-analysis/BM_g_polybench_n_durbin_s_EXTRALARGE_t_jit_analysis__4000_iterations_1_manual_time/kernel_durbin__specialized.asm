	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$32120, %rsp
	leaq	-7(%rip), %rax
	movabsq	$-7969, %rcx
	addq	%rax, %rcx
	movabsq	$48, %rax
	movq	(%rcx,%rax), %rsi
	vmovsd	(%rsi), %xmm0
	movabsq	$-1600, %rax
	vxorpd	(%rcx,%rax), %xmm0, %xmm2
	movabsq	$32, %rdx
	movq	(%rcx,%rdx), %rbx
	vmovlpd	%xmm2, (%rbx)
	leaq	-8(%rbx), %rdx
	movq	%rdx, 24(%rsp)
	movl	$1, %r13d
	movq	$-1, %rdi
	movabsq	$-1584, %rdx
	vmovsd	(%rcx,%rdx), %xmm3
	xorl	%r8d, %r8d
	vmovapd	(%rcx,%rax), %xmm1
	vmovapd	%xmm1, 96(%rsp)
	movabsq	$152, %rax
	movq	(%rcx,%rax), %rax
	movq	%rax, 80(%rsp)
	movq	%rsi, %rdx
	movabsq	$2305843009213693946, %r12
	movabsq	$2305843009213693945, %rsi
	vmovsd	%xmm3, 88(%rsp)
	movabsq	$2305843009213693944, %r15
	movabsq	$2305843009213693943, %r14
	movabsq	$2305843009213693942, %rcx
	movabsq	$2305843009213693941, %r9
	movabsq	$2305843009213693940, %r10
	movq	%rbx, 16(%rsp)
	movq	%rdx, (%rsp)
	movq	%rdx, 32(%rsp)
	jmp	175
	leaq	8(,%r8,8), %rdx
	movq	%rbx, %rdi
	leaq	112(%rsp), %rsi
	vmovapd	%xmm3, 48(%rsp)
	movq	%r8, %r14
	callq	*80(%rsp)
	movq	%r14, %r8
	vmovapd	48(%rsp), %xmm2
	vmovsd	%xmm2, (%rbx,%r13,8)
	incq	%r13
	incq	%r8
	movq	40(%rsp), %rdi
	incq	%rdi
	addq	$8, %r12
	movq	%r12, (%rsp)
	addq	$8, 24(%rsp)
	addq	$8, %r15
	movq	%r15, 16(%rsp)
	cmpq	$3999, %r8
	movq	32(%rsp), %rdx
	movabsq	$2305843009213693946, %r12
	movabsq	$2305843009213693945, %rsi
	vmovsd	8(%rsp), %xmm3
	movabsq	$2305843009213693944, %r15
	movabsq	$2305843009213693943, %r14
	movabsq	$2305843009213693942, %rcx
	movabsq	$2305843009213693941, %r9
	movabsq	$2305843009213693940, %r10
	je	946
	leaq	-1(%r8), %rbp
	leaq	(,%r13,8), %rax
	addq	%rdx, %rax
	cmpq	$1, %r8
	movq	%rdi, 40(%rsp)
	jae	23
	vxorpd	%xmm0, %xmm0, %xmm0
	xorl	%edx, %edx
	movq	16(%rsp), %r15
	movq	(%rsp), %r12
	jmp	667
	nopl	(%rax)
	movq	%rbp, 48(%rsp)
	movq	%rdi, %rbp
	shrq	%rbp
	leaq	-14(%rdi), %rdx
	andq	$-16, %rdx
	addq	$16, %rdx
	cmpq	$15, %r8
	jae	35
	vxorpd	%xmm0, %xmm0, %xmm0
	xorl	%edi, %edi
	movabsq	$2305843009213693950, %r11
	jmp	484
	nopw	%cs:(%rax,%rax)
	movq	%rdx, 72(%rsp)
	movq	%r8, 8(%rsp)
	movq	%rbp, 64(%rsp)
	leaq	1(%rbp), %rdi
	andq	$-8, %rdi
	vxorpd	%xmm0, %xmm0, %xmm0
	movq	(%rsp), %r8
	movq	%r9, %r11
	xorl	%r9d, %r9d
	movq	%rcx, %rdx
	movq	%r10, %rcx
	nopl	(%rax,%rax)
	vmovsd	(%r8), %xmm1
	vfmadd132sd	(%rbx,%r9,8), %xmm0, %xmm1
	movq	%r9, %r10
	movabsq	$2305843009213693950, %rbp
	xorq	%rbp, %r10
	vmovsd	(%rax,%r10,8), %xmm0
	vfmadd132sd	8(%rbx,%r9,8), %xmm1, %xmm0
	movq	%r9, %r10
	movabsq	$2305843009213693949, %rbp
	xorq	%rbp, %r10
	vmovsd	(%rax,%r10,8), %xmm1
	vfmadd132sd	16(%rbx,%r9,8), %xmm0, %xmm1
	movq	%r9, %r10
	movabsq	$2305843009213693948, %rbp
	xorq	%rbp, %r10
	vmovsd	(%rax,%r10,8), %xmm0
	vfmadd132sd	24(%rbx,%r9,8), %xmm1, %xmm0
	movq	%r9, %r10
	movabsq	$2305843009213693947, %rbp
	xorq	%rbp, %r10
	vmovsd	(%rax,%r10,8), %xmm1
	vfmadd132sd	32(%rbx,%r9,8), %xmm0, %xmm1
	movq	%r9, %r10
	xorq	%r12, %r10
	vmovsd	(%rax,%r10,8), %xmm0
	vfmadd132sd	40(%rbx,%r9,8), %xmm1, %xmm0
	movq	%r9, %r10
	xorq	%rsi, %r10
	vmovsd	(%rax,%r10,8), %xmm1
	vfmadd132sd	48(%rbx,%r9,8), %xmm0, %xmm1
	movq	%r9, %r10
	xorq	%r15, %r10
	vmovsd	(%rax,%r10,8), %xmm0
	vfmadd132sd	56(%rbx,%r9,8), %xmm1, %xmm0
	movq	%r9, %r10
	xorq	%r14, %r10
	vmovsd	(%rax,%r10,8), %xmm1
	vfmadd132sd	64(%rbx,%r9,8), %xmm0, %xmm1
	movq	%r9, %r10
	xorq	%rdx, %r10
	vmovsd	(%rax,%r10,8), %xmm0
	vfmadd132sd	72(%rbx,%r9,8), %xmm1, %xmm0
	movq	%r9, %r10
	xorq	%r11, %r10
	vmovsd	(%rax,%r10,8), %xmm1
	vfmadd132sd	80(%rbx,%r9,8), %xmm0, %xmm1
	movq	%r9, %r10
	xorq	%rcx, %r10
	vmovsd	(%rax,%r10,8), %xmm0
	vfmadd132sd	88(%rbx,%r9,8), %xmm1, %xmm0
	movq	%r9, %r10
	movabsq	$2305843009213693939, %rbp
	xorq	%rbp, %r10
	vmovsd	(%rax,%r10,8), %xmm1
	vfmadd132sd	96(%rbx,%r9,8), %xmm0, %xmm1
	movq	%r9, %r10
	movabsq	$2305843009213693938, %rbp
	xorq	%rbp, %r10
	vmovsd	(%rax,%r10,8), %xmm0
	vfmadd132sd	104(%rbx,%r9,8), %xmm1, %xmm0
	movq	%r9, %r10
	movabsq	$2305843009213693937, %rbp
	xorq	%rbp, %r10
	vmovsd	(%rax,%r10,8), %xmm1
	vfmadd132sd	112(%rbx,%r9,8), %xmm0, %xmm1
	movq	%r9, %r10
	movabsq	$2305843009213693936, %rbp
	xorq	%rbp, %r10
	vmovsd	(%rax,%r10,8), %xmm0
	vfmadd132sd	120(%rbx,%r9,8), %xmm1, %xmm0
	addq	$16, %r9
	addq	$-128, %r8
	addq	$-8, %rdi
	jne	-394
	movq	72(%rsp), %rdx
	movq	%rdx, %rdi
	movabsq	$2305843009213693950, %r11
	movq	8(%rsp), %r8
	movq	64(%rsp), %rbp
	movq	48(%rsp), %rcx
	shrq	%rcx
	incq	%rcx
	testb	$7, %cl
	movq	16(%rsp), %r15
	movq	(%rsp), %r12
	je	105
	movq	%r8, %r14
	incb	%bpl
	movzbl	%bpl, %ecx
	andl	$7, %ecx
	shll	$3, %ecx
	movq	40(%rsp), %rdx
	addl	$2, %edx
	andl	$14, %edx
	leaq	(,%rdi,8), %r8
	movq	%r12, %rsi
	subq	%r8, %rsi
	xorl	%r8d, %r8d
	movq	%rdi, %r9
	nop
	vmovsd	(%rsi,%r8,2), %xmm1
	vfmadd132sd	(%rbx,%r9,8), %xmm0, %xmm1
	movq	%r9, %r10
	xorq	%r11, %r10
	vmovsd	(%rax,%r10,8), %xmm0
	vfmadd132sd	8(%rbx,%r9,8), %xmm1, %xmm0
	addq	$2, %r9
	addq	$-8, %r8
	movq	%rcx, %r10
	addq	%r8, %r10
	jne	-47
	addq	%rdx, %rdi
	movq	%rdi, %rdx
	movq	%r14, %r8
	testb	$1, %r13b
	je	17
	movq	%rdx, %rcx
	notq	%rdx
	vmovsd	(%rax,%rdx,8), %xmm1
	vfmadd231sd	(%rbx,%rcx,8), %xmm1, %xmm0
	vfnmadd213sd	88(%rsp), %xmm2, %xmm2
	vmulsd	%xmm2, %xmm3, %xmm3
	movq	32(%rsp), %rax
	vaddsd	(%rax,%r13,8), %xmm0, %xmm0
	vxorpd	96(%rsp), %xmm0, %xmm0
	vmovsd	%xmm3, 8(%rsp)
	vdivsd	%xmm3, %xmm0, %xmm3
	cmpq	$4, %r13
	jae	5
	xorl	%eax, %eax
	jmp	112
	nop
	movq	%r13, %rcx
	andq	$-4, %rcx
	movq	%r13, %rax
	movabsq	$9223372036854775804, %rdx
	andq	%rdx, %rax
	vmovddup	%xmm3, %xmm0
	movq	24(%rsp), %rdx
	xorl	%esi, %esi
	nopw	%cs:(%rax,%rax)
	vpermilpd	$1, (%rdx), %xmm1
	vpermilpd	$1, -16(%rdx), %xmm2
	vfmadd213pd	(%rbx,%rsi,8), %xmm0, %xmm1
	vfmadd213pd	16(%rbx,%rsi,8), %xmm0, %xmm2
	vmovapd	%xmm1, 112(%rsp,%rsi,8)
	vmovapd	%xmm2, 128(%rsp,%rsi,8)
	addq	$4, %rsi
	addq	$-32, %rdx
	cmpq	%rsi, %rcx
	jne	-54
	cmpq	%rax, %r13
	je	-1071
	leaq	(,%rax,8), %rdx
	movq	%r15, %rcx
	subq	%rdx, %rcx
	nopl	(%rax)
	vmovsd	(%rcx), %xmm0
	vfmadd213sd	(%rbx,%rax,8), %xmm3, %xmm0
	vmovsd	%xmm0, 112(%rsp,%rax,8)
	incq	%rax
	addq	$-8, %rcx
	cmpq	%rax, %r13
	jne	-28
	jmp	-1121
	addq	$32120, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
