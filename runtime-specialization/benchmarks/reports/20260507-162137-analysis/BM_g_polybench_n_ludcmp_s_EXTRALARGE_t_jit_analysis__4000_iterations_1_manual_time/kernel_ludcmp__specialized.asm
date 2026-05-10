	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	leaq	-7(%rip), %rax
	movabsq	$-7978, %rcx
	addq	%rax, %rcx
	movabsq	$64, %rax
	movq	(%rcx,%rax), %rax
	movq	(%rax), %r9
	leaq	120(%r9), %rbx
	movq	$-2, %r15
	xorl	%edi, %edi
	movabsq	$-1584, %rax
	movq	%rcx, -56(%rsp)
	vmovapd	(%rcx,%rax), %xmm0
	movq	%r9, %rbp
	movq	%r9, %r12
	jmp	45
	nopl	(%rax,%rax)
	incq	%rdi
	incq	%r15
	addq	$32000, %rbx
	addq	$32000, %r12
	addq	$8, %rbp
	cmpq	$4000, %rdi
	je	1172
	testq	%rdi, %rdi
	je	-42
	movq	%rbp, -48(%rsp)
	movq	%rbx, -64(%rsp)
	movq	%r15, %rcx
	shrq	%rcx
	leal	1(%rcx), %eax
	movzbl	%al, %r11d
	andl	$7, %r11d
	shll	$4, %r11d
	incq	%rcx
	andq	$-8, %rcx
	movq	%rcx, -32(%rsp)
	leal	2(%r15), %eax
	andl	$14, %eax
	movq	%rax, -24(%rsp)
	movq	%r15, -40(%rsp)
	leaq	-14(%r15), %r14
	andq	$-16, %r14
	addq	$16, %r14
	leaq	-2(%rdi), %rax
	movq	%rax, -8(%rsp)
	shrq	%rax
	incq	%rax
	movq	%rax, -16(%rsp)
	movq	$-2, %rbp
	imulq	$32000, %rdi, %r13
	addq	%r9, %r13
	movq	%r9, %r8
	xorl	%ecx, %ecx
	jmp	76
	nop
	xorl	%esi, %esi
	vmovsd	(%r13,%rsi,8), %xmm2
	imulq	$32000, %rsi, %rax
	addq	%r9, %rax
	vfnmadd231sd	(%rax,%rcx,8), %xmm2, %xmm1
	leaq	(,%rcx,8), %rax
	addq	%r13, %rax
	imulq	$32000, %rcx, %rdx
	addq	%r9, %rdx
	vdivsd	(%rdx,%rcx,8), %xmm1, %xmm1
	vmovsd	%xmm1, (%rax)
	incq	%rcx
	incq	%rbp
	addq	$8, %r8
	cmpq	%rdi, %rcx
	je	422
	vmovsd	(%r13,%rcx,8), %xmm1
	testq	%rcx, %rcx
	je	-61
	cmpq	$1, %rcx
	je	-92
	movq	%rbp, %rbx
	shrq	%rbx
	leaq	-14(%rbp), %rsi
	andq	$-16, %rsi
	addq	$16, %rsi
	leaq	-2(%rcx), %r10
	cmpq	$14, %r10
	jae	7
	xorl	%edx, %edx
	jmp	259
	leaq	1(%rbx), %r15
	andq	$-8, %r15
	movq	-64(%rsp), %rdx
	movq	%r8, %rax
	nop
	vmovsd	-120(%rdx), %xmm2
	vfnmadd132sd	(%rax), %xmm1, %xmm2
	vmovsd	-112(%rdx), %xmm1
	vfnmadd231sd	32000(%rax), %xmm1, %xmm2
	vmovsd	-104(%rdx), %xmm1
	vfnmadd132sd	64000(%rax), %xmm2, %xmm1
	vmovsd	-96(%rdx), %xmm2
	vfnmadd132sd	96000(%rax), %xmm1, %xmm2
	vmovsd	-88(%rdx), %xmm1
	vfnmadd132sd	128000(%rax), %xmm2, %xmm1
	vmovsd	-80(%rdx), %xmm2
	vfnmadd132sd	160000(%rax), %xmm1, %xmm2
	vmovsd	-72(%rdx), %xmm1
	vfnmadd132sd	192000(%rax), %xmm2, %xmm1
	vmovsd	-64(%rdx), %xmm2
	vfnmadd132sd	224000(%rax), %xmm1, %xmm2
	vmovsd	-56(%rdx), %xmm1
	vfnmadd132sd	256000(%rax), %xmm2, %xmm1
	vmovsd	-48(%rdx), %xmm2
	vfnmadd132sd	288000(%rax), %xmm1, %xmm2
	vmovsd	-40(%rdx), %xmm1
	vfnmadd132sd	320000(%rax), %xmm2, %xmm1
	vmovsd	-32(%rdx), %xmm2
	vfnmadd132sd	352000(%rax), %xmm1, %xmm2
	vmovsd	-24(%rdx), %xmm1
	vfnmadd132sd	384000(%rax), %xmm2, %xmm1
	vmovsd	-16(%rdx), %xmm2
	vfnmadd132sd	416000(%rax), %xmm1, %xmm2
	vmovsd	-8(%rdx), %xmm3
	vfnmadd132sd	448000(%rax), %xmm2, %xmm3
	vmovsd	(%rdx), %xmm1
	vfnmadd132sd	480000(%rax), %xmm3, %xmm1
	addq	$512000, %rax
	subq	$-128, %rdx
	addq	$-8, %r15
	jne	-239
	movq	%rsi, %rdx
	shrq	%r10
	incq	%r10
	testb	$7, %r10b
	je	83
	incb	%bl
	movzbl	%bl, %eax
	andl	$7, %eax
	shll	$4, %eax
	leal	2(%rbp), %esi
	andl	$14, %esi
	leaq	(%r12,%rdx,8), %r10
	imulq	$32000, %rdx, %rbx
	addq	%r8, %rbx
	xorl	%r15d, %r15d
	vmovsd	(%r10,%r15), %xmm2
	vfnmadd231sd	(%rbx), %xmm2, %xmm1
	vmovsd	8(%r10,%r15), %xmm2
	vfnmadd231sd	32000(%rbx), %xmm2, %xmm1
	addq	$16, %r15
	addq	$64000, %rbx
	cmpq	%r15, %rax
	jne	-43
	addq	%rsi, %rdx
	movq	%rdx, %rsi
	testb	$1, %cl
	jne	-488
	jmp	-470
	nop
	movq	-48(%rsp), %rbp
	movq	%rbp, %rax
	movq	%rdi, %rcx
	cmpq	$1, %rdi
	movq	-64(%rsp), %rbx
	movq	-40(%rsp), %r15
	jne	104
	vmovsd	32000(%r9), %xmm1
	vxorpd	%xmm0, %xmm1, %xmm1
	xorl	%eax, %eax
	nopw	(%rax,%rax)
	vmovsd	8(%r9,%rax,8), %xmm2
	vfmadd213sd	32008(%r9,%rax,8), %xmm1, %xmm2
	vmovsd	%xmm2, 32008(%r9,%rax,8)
	incq	%rax
	cmpq	$3999, %rax
	jne	-38
	jmp	-747
	nopl	(%rax,%rax)
	leaq	(,%rcx,8), %rdx
	addq	%r13, %rdx
	vmovsd	%xmm1, (%rdx)
	incq	%rcx
	addq	$8, %rax
	cmpq	$4000, %rcx
	je	-787
	vmovsd	(%r13,%rcx,8), %xmm1
	cmpq	$14, -8(%rsp)
	jae	14
	xorl	%edx, %edx
	jmp	282
	nopl	(%rax)
	movq	-32(%rsp), %rsi
	movq	%rbx, %r10
	movq	%rax, %rdx
	nopl	(%rax,%rax)
	vmovsd	-120(%r10), %xmm2
	vfnmadd132sd	(%rdx), %xmm1, %xmm2
	vmovsd	-112(%r10), %xmm1
	vfnmadd231sd	32000(%rdx), %xmm1, %xmm2
	vmovsd	-104(%r10), %xmm1
	vfnmadd132sd	64000(%rdx), %xmm2, %xmm1
	vmovsd	-96(%r10), %xmm2
	vfnmadd132sd	96000(%rdx), %xmm1, %xmm2
	vmovsd	-88(%r10), %xmm1
	vfnmadd132sd	128000(%rdx), %xmm2, %xmm1
	vmovsd	-80(%r10), %xmm2
	vfnmadd132sd	160000(%rdx), %xmm1, %xmm2
	vmovsd	-72(%r10), %xmm1
	vfnmadd132sd	192000(%rdx), %xmm2, %xmm1
	vmovsd	-64(%r10), %xmm2
	vfnmadd132sd	224000(%rdx), %xmm1, %xmm2
	vmovsd	-56(%r10), %xmm1
	vfnmadd132sd	256000(%rdx), %xmm2, %xmm1
	vmovsd	-48(%r10), %xmm2
	vfnmadd132sd	288000(%rdx), %xmm1, %xmm2
	vmovsd	-40(%r10), %xmm1
	vfnmadd132sd	320000(%rdx), %xmm2, %xmm1
	vmovsd	-32(%r10), %xmm2
	vfnmadd132sd	352000(%rdx), %xmm1, %xmm2
	vmovsd	-24(%r10), %xmm1
	vfnmadd132sd	384000(%rdx), %xmm2, %xmm1
	vmovsd	-16(%r10), %xmm2
	vfnmadd132sd	416000(%rdx), %xmm1, %xmm2
	vmovsd	-8(%r10), %xmm3
	vfnmadd132sd	448000(%rdx), %xmm2, %xmm3
	vmovsd	(%r10), %xmm1
	vfnmadd132sd	480000(%rdx), %xmm3, %xmm1
	addq	$512000, %rdx
	subq	$-128, %r10
	addq	$-8, %rsi
	jne	-256
	movq	%r14, %rdx
	movq	%r14, %rsi
	testb	$7, -16(%rsp)
	je	70
	leaq	(%r12,%rdx,8), %rsi
	imulq	$32000, %rdx, %r8
	addq	%rax, %r8
	xorl	%r10d, %r10d
	nop
	vmovsd	(%rsi,%r10), %xmm2
	vfnmadd231sd	(%r8), %xmm2, %xmm1
	vmovsd	8(%rsi,%r10), %xmm2
	vfnmadd231sd	32000(%r8), %xmm2, %xmm1
	addq	$16, %r10
	addq	$64000, %r8
	cmpq	%r10, %r11
	jne	-43
	addq	-24(%rsp), %rdx
	movq	%rdx, %rsi
	testb	$1, %dil
	je	-429
	vmovsd	(%r13,%rsi,8), %xmm2
	imulq	$32000, %rsi, %rdx
	addq	%r9, %rdx
	vfnmadd231sd	(%rdx,%rcx,8), %xmm2, %xmm1
	jmp	-457
	movq	$-2, %rax
	xorl	%ecx, %ecx
	movabsq	$160, %rdx
	movq	-56(%rsp), %r12
	movq	(%r12,%rdx), %rsi
	movabsq	$200, %rdx
	movq	(%r12,%rdx), %rdx
	movq	%r9, %rdi
	jmp	64
	nopl	(%rax,%rax)
	xorl	%r8d, %r8d
	imulq	$32000, %rcx, %r10
	addq	%r9, %r10
	vmovsd	(%r10,%r8,8), %xmm1
	vfnmadd231sd	(%rdx,%r8,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rdx,%rcx,8)
	incq	%rcx
	incq	%rax
	addq	$32000, %rdi
	cmpq	$4000, %rcx
	je	431
	vmovsd	(%rsi,%rcx,8), %xmm0
	testq	%rcx, %rcx
	je	-41
	cmpq	$1, %rcx
	je	-72
	movq	%rax, %r10
	shrq	%r10
	leaq	-14(%rax), %r8
	andq	$-16, %r8
	addq	$16, %r8
	leaq	-2(%rcx), %r11
	cmpq	$14, %r11
	jae	7
	xorl	%ebx, %ebx
	jmp	260
	leaq	1(%r10), %rbx
	andq	$-8, %rbx
	movl	$120, %r14d
	nopl	(%rax)
	vmovsd	-120(%rdi,%r14), %xmm1
	vfnmadd132sd	-120(%rdx,%r14), %xmm0, %xmm1
	vmovsd	-112(%rdi,%r14), %xmm0
	vfnmadd231sd	-112(%rdx,%r14), %xmm0, %xmm1
	vmovsd	-104(%rdi,%r14), %xmm0
	vfnmadd132sd	-104(%rdx,%r14), %xmm1, %xmm0
	vmovsd	-96(%rdi,%r14), %xmm1
	vfnmadd132sd	-96(%rdx,%r14), %xmm0, %xmm1
	vmovsd	-88(%rdi,%r14), %xmm0
	vfnmadd132sd	-88(%rdx,%r14), %xmm1, %xmm0
	vmovsd	-80(%rdi,%r14), %xmm1
	vfnmadd132sd	-80(%rdx,%r14), %xmm0, %xmm1
	vmovsd	-72(%rdi,%r14), %xmm0
	vfnmadd132sd	-72(%rdx,%r14), %xmm1, %xmm0
	vmovsd	-64(%rdi,%r14), %xmm1
	vfnmadd132sd	-64(%rdx,%r14), %xmm0, %xmm1
	vmovsd	-56(%rdi,%r14), %xmm0
	vfnmadd132sd	-56(%rdx,%r14), %xmm1, %xmm0
	vmovsd	-48(%rdi,%r14), %xmm1
	vfnmadd132sd	-48(%rdx,%r14), %xmm0, %xmm1
	vmovsd	-40(%rdi,%r14), %xmm0
	vfnmadd132sd	-40(%rdx,%r14), %xmm1, %xmm0
	vmovsd	-32(%rdi,%r14), %xmm1
	vfnmadd132sd	-32(%rdx,%r14), %xmm0, %xmm1
	vmovsd	-24(%rdi,%r14), %xmm0
	vfnmadd132sd	-24(%rdx,%r14), %xmm1, %xmm0
	vmovsd	-16(%rdi,%r14), %xmm1
	vfnmadd132sd	-16(%rdx,%r14), %xmm0, %xmm1
	vmovsd	-8(%rdi,%r14), %xmm2
	vfnmadd132sd	-8(%rdx,%r14), %xmm1, %xmm2
	vmovsd	(%rdi,%r14), %xmm0
	vfnmadd132sd	(%rdx,%r14), %xmm2, %xmm0
	subq	$-128, %r14
	addq	$-8, %rbx
	jne	-236
	movq	%r8, %rbx
	shrq	%r11
	incq	%r11
	testb	$7, %r11b
	je	94
	incb	%r10b
	movzbl	%r10b, %r8d
	andl	$7, %r8d
	shll	$4, %r8d
	leal	2(%rax), %r10d
	andl	$14, %r10d
	leaq	(%rdx,%rbx,8), %r11
	addq	$8, %r11
	leaq	(%rdi,%rbx,8), %r14
	xorl	%r15d, %r15d
	nopw	%cs:(%rax,%rax)
	vmovsd	(%r14,%r15), %xmm1
	vmovsd	8(%r14,%r15), %xmm2
	vfnmadd231sd	-8(%r11,%r15), %xmm1, %xmm0
	vfnmadd231sd	(%r11,%r15), %xmm2, %xmm0
	addq	$16, %r15
	cmpq	%r15, %r8
	jne	-35
	addq	%r10, %rbx
	movq	%rbx, %r8
	testb	$1, %cl
	jne	-479
	jmp	-462
	movabsq	$152, %r10
	movl	$32000, %eax
	addq	(%r12,%r10), %rax
	leaq	128000000(%r9), %rcx
	leaq	127968000(%r9), %rsi
	movl	$4000, %r11d
	xorl	%edi, %edi
	movq	$-4000, %r8
	movq	(%r12,%r10), %r10
	jmp	65
	nopl	(%rax)
	leaq	-1(%r11), %rbx
	imulq	$32000, %rbx, %r14
	addq	%r9, %r14
	vdivsd	(%r14,%rbx,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%r10,%rbx,8)
	incl	%edi
	incq	%r8
	addq	$-8, %rax
	addq	$-32008, %rcx
	addq	$-32000, %rsi
	cmpq	$1, %r11
	movq	%rbx, %r11
	jbe	204
	vmovsd	-8(%rdx,%r11,8), %xmm0
	cmpq	$3999, %r11
	ja	-78
	testb	$7, %dil
	je	36
	movl	%edi, %r14d
	andl	$7, %r14d
	xorl	%ebx, %ebx
	nopl	(%rax)
	vmovsd	(%rcx,%rbx,8), %xmm1
	vfnmadd231sd	(%rax,%rbx,8), %xmm1, %xmm0
	incq	%rbx
	cmpl	%ebx, %r14d
	jne	-19
	subq	%r8, %rbx
	jmp	3
	movq	%r11, %rbx
	leal	-1(%rdi), %ebp
	cmpl	$7, %ebp
	jb	-135
	addq	$-8, %rbx
	nopl	(%rax,%rax)
	vmovsd	64(%rsi,%rbx,8), %xmm1
	vfnmadd132sd	64(%r10,%rbx,8), %xmm0, %xmm1
	vmovsd	72(%rsi,%rbx,8), %xmm0
	vfnmadd231sd	72(%r10,%rbx,8), %xmm0, %xmm1
	vmovsd	80(%rsi,%rbx,8), %xmm0
	vfnmadd132sd	80(%r10,%rbx,8), %xmm1, %xmm0
	vmovsd	88(%rsi,%rbx,8), %xmm1
	vfnmadd132sd	88(%r10,%rbx,8), %xmm0, %xmm1
	vmovsd	96(%rsi,%rbx,8), %xmm0
	vfnmadd132sd	96(%r10,%rbx,8), %xmm1, %xmm0
	vmovsd	104(%rsi,%rbx,8), %xmm1
	vfnmadd132sd	104(%r10,%rbx,8), %xmm0, %xmm1
	vmovsd	112(%rsi,%rbx,8), %xmm2
	vfnmadd132sd	112(%r10,%rbx,8), %xmm1, %xmm2
	vmovsd	120(%rsi,%rbx,8), %xmm0
	vfnmadd132sd	120(%r10,%rbx,8), %xmm2, %xmm0
	addq	$8, %rbx
	cmpq	$3992, %rbx
	jb	-117
	jmp	-266
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
