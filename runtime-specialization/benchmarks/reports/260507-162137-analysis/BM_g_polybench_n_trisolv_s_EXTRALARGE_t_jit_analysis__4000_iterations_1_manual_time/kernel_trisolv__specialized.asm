	pushq	%r14
	pushq	%rbx
	leaq	-7(%rip), %rax
	movabsq	$-7987, %r8
	addq	%rax, %r8
	movabsq	$160, %rax
	movq	(%r8,%rax), %rax
	movq	(%rax), %rax
	leaq	120(%rax), %rcx
	movq	$-2, %rdx
	xorl	%esi, %esi
	movabsq	$176, %rdi
	movq	(%r8,%rdi), %rdi
	movabsq	$88, %r9
	movq	(%r8,%r9), %r8
	movq	%rax, %r9
	jmp	96
	nopw	%cs:(%rax,%rax)
	xorl	%r10d, %r10d
	imulq	$32000, %rsi, %r11
	addq	%rax, %r11
	vmovsd	(%r11,%r10,8), %xmm1
	vfnmadd231sd	(%r8,%r10,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%r8,%rsi,8)
	imulq	$32000, %rsi, %r10
	addq	%rax, %r10
	vdivsd	(%r10,%rsi,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%r8,%rsi,8)
	incq	%rsi
	addq	$32000, %rcx
	incq	%rdx
	addq	$32000, %r9
	cmpq	$4000, %rsi
	je	488
	vmovsd	(%rdi,%rsi,8), %xmm0
	vmovsd	%xmm0, (%r8,%rsi,8)
	testq	%rsi, %rsi
	je	-71
	cmpq	$1, %rsi
	je	-108
	movq	%rdx, %r11
	shrq	%r11
	leaq	-2(%rsi), %rbx
	cmpq	$14, %rbx
	jae	8
	xorl	%r10d, %r10d
	jmp	344
	leaq	1(%r11), %r14
	andq	$-8, %r14
	xorl	%r10d, %r10d
	nop
	vmovsd	-120(%rcx,%r10,8), %xmm1
	vfnmadd132sd	(%r8,%r10,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%r8,%rsi,8)
	vmovsd	-112(%rcx,%r10,8), %xmm0
	vfnmadd132sd	8(%r8,%r10,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%r8,%rsi,8)
	vmovsd	-104(%rcx,%r10,8), %xmm1
	vfnmadd132sd	16(%r8,%r10,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%r8,%rsi,8)
	vmovsd	-96(%rcx,%r10,8), %xmm0
	vfnmadd132sd	24(%r8,%r10,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%r8,%rsi,8)
	vmovsd	-88(%rcx,%r10,8), %xmm1
	vfnmadd132sd	32(%r8,%r10,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%r8,%rsi,8)
	vmovsd	-80(%rcx,%r10,8), %xmm0
	vfnmadd132sd	40(%r8,%r10,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%r8,%rsi,8)
	vmovsd	-72(%rcx,%r10,8), %xmm1
	vfnmadd132sd	48(%r8,%r10,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%r8,%rsi,8)
	vmovsd	-64(%rcx,%r10,8), %xmm0
	vfnmadd132sd	56(%r8,%r10,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%r8,%rsi,8)
	vmovsd	-56(%rcx,%r10,8), %xmm1
	vfnmadd132sd	64(%r8,%r10,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%r8,%rsi,8)
	vmovsd	-48(%rcx,%r10,8), %xmm0
	vfnmadd132sd	72(%r8,%r10,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%r8,%rsi,8)
	vmovsd	-40(%rcx,%r10,8), %xmm1
	vfnmadd132sd	80(%r8,%r10,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%r8,%rsi,8)
	vmovsd	-32(%rcx,%r10,8), %xmm0
	vfnmadd132sd	88(%r8,%r10,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%r8,%rsi,8)
	vmovsd	-24(%rcx,%r10,8), %xmm1
	vfnmadd132sd	96(%r8,%r10,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%r8,%rsi,8)
	vmovsd	-16(%rcx,%r10,8), %xmm0
	vfnmadd132sd	104(%r8,%r10,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%r8,%rsi,8)
	vmovsd	-8(%rcx,%r10,8), %xmm1
	vfnmadd132sd	112(%r8,%r10,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%r8,%rsi,8)
	vmovsd	(%rcx,%r10,8), %xmm0
	vfnmadd132sd	120(%r8,%r10,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%r8,%rsi,8)
	addq	$16, %r10
	addq	$-8, %r14
	jne	-332
	shrq	%rbx
	incq	%rbx
	testb	$7, %bl
	je	72
	incb	%r11b
	movzbl	%r11b, %r11d
	andl	$7, %r11d
	nopw	%cs:(%rax,%rax)
	vmovsd	(%r9,%r10,8), %xmm1
	vfnmadd132sd	(%r8,%r10,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%r8,%rsi,8)
	vmovsd	8(%r9,%r10,8), %xmm0
	vfnmadd132sd	8(%r8,%r10,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%r8,%rsi,8)
	addq	$2, %r10
	decq	%r11
	jne	-47
	testb	$1, %sil
	jne	-566
	jmp	-543
	popq	%rbx
	popq	%r14
	retq
