	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%rbx
	leaq	-7(%rip), %rax
	movabsq	$-7990, %rdi
	addq	%rax, %rdi
	movabsq	$160, %rax
	movq	(%rdi,%rax), %rax
	movq	(%rax), %rax
	movabsq	$192, %rcx
	movq	(%rdi,%rcx), %rcx
	movq	(%rcx), %rcx
	movabsq	$32, %r8
	movq	%rax, %rdx
	subq	(%rdi,%r8), %rdx
	xorl	%esi, %esi
	movq	(%rdi,%r8), %rdi
	jmp	33
	nopw	%cs:(%rax,%rax)
	incq	%rsi
	addq	$475200, %rax
	cmpq	$250, %rsi
	je	1294
	imulq	$475200, %rsi, %r8
	addq	%rdx, %r8
	movq	%rax, %r9
	xorl	%r10d, %r10d
	jmp	27
	nopl	(%rax,%rax)
	incq	%r10
	addq	$2160, %r9
	cmpq	$220, %r10
	je	-67
	movq	%rcx, %r11
	xorl	%ebx, %ebx
	nopl	(%rax,%rax)
	movq	$0, (%rdi,%rbx,8)
	vxorpd	%xmm0, %xmm0, %xmm0
	movl	$17, %r14d
	movq	%r11, %r15
	nopw	%cs:(%rax,%rax)
	vmovsd	-136(%r9,%r14,8), %xmm1
	vfmadd132sd	(%r15), %xmm0, %xmm1
	vmovsd	%xmm1, (%rdi,%rbx,8)
	vmovsd	-128(%r9,%r14,8), %xmm0
	vfmadd132sd	2160(%r15), %xmm1, %xmm0
	vmovsd	%xmm0, (%rdi,%rbx,8)
	vmovsd	-120(%r9,%r14,8), %xmm1
	vfmadd132sd	4320(%r15), %xmm0, %xmm1
	vmovsd	%xmm1, (%rdi,%rbx,8)
	vmovsd	-112(%r9,%r14,8), %xmm0
	vfmadd132sd	6480(%r15), %xmm1, %xmm0
	vmovsd	%xmm0, (%rdi,%rbx,8)
	vmovsd	-104(%r9,%r14,8), %xmm1
	vfmadd132sd	8640(%r15), %xmm0, %xmm1
	vmovsd	%xmm1, (%rdi,%rbx,8)
	vmovsd	-96(%r9,%r14,8), %xmm0
	vfmadd132sd	10800(%r15), %xmm1, %xmm0
	vmovsd	%xmm0, (%rdi,%rbx,8)
	vmovsd	-88(%r9,%r14,8), %xmm1
	vfmadd132sd	12960(%r15), %xmm0, %xmm1
	vmovsd	%xmm1, (%rdi,%rbx,8)
	vmovsd	-80(%r9,%r14,8), %xmm0
	vfmadd132sd	15120(%r15), %xmm1, %xmm0
	vmovsd	%xmm0, (%rdi,%rbx,8)
	vmovsd	-72(%r9,%r14,8), %xmm1
	vfmadd132sd	17280(%r15), %xmm0, %xmm1
	vmovsd	%xmm1, (%rdi,%rbx,8)
	vmovsd	-64(%r9,%r14,8), %xmm0
	vfmadd132sd	19440(%r15), %xmm1, %xmm0
	vmovsd	%xmm0, (%rdi,%rbx,8)
	vmovsd	-56(%r9,%r14,8), %xmm1
	vfmadd132sd	21600(%r15), %xmm0, %xmm1
	vmovsd	%xmm1, (%rdi,%rbx,8)
	vmovsd	-48(%r9,%r14,8), %xmm0
	vfmadd132sd	23760(%r15), %xmm1, %xmm0
	vmovsd	%xmm0, (%rdi,%rbx,8)
	vmovsd	-40(%r9,%r14,8), %xmm1
	vfmadd132sd	25920(%r15), %xmm0, %xmm1
	vmovsd	%xmm1, (%rdi,%rbx,8)
	vmovsd	-32(%r9,%r14,8), %xmm0
	vfmadd132sd	28080(%r15), %xmm1, %xmm0
	vmovsd	%xmm0, (%rdi,%rbx,8)
	vmovsd	-24(%r9,%r14,8), %xmm1
	vfmadd132sd	30240(%r15), %xmm0, %xmm1
	vmovsd	%xmm1, (%rdi,%rbx,8)
	vmovsd	-16(%r9,%r14,8), %xmm0
	vfmadd132sd	32400(%r15), %xmm1, %xmm0
	vmovsd	%xmm0, (%rdi,%rbx,8)
	vmovsd	-8(%r9,%r14,8), %xmm1
	vfmadd132sd	34560(%r15), %xmm0, %xmm1
	vmovsd	%xmm1, (%rdi,%rbx,8)
	vmovsd	(%r9,%r14,8), %xmm0
	vfmadd132sd	36720(%r15), %xmm1, %xmm0
	vmovsd	%xmm0, (%rdi,%rbx,8)
	addq	$18, %r14
	addq	$38880, %r15
	cmpq	$287, %r14
	jne	-400
	incq	%rbx
	addq	$8, %r11
	cmpq	$270, %rbx
	jne	-452
	imulq	$2160, %r10, %r11
	addq	%r8, %r11
	cmpq	$32, %r11
	jae	12
	xorl	%r14d, %r14d
	jmp	346
	nopl	(%rax)
	movl	$62, %r11d
	nopw	%cs:(%rax,%rax)
	vmovups	-496(%rdi,%r11,8), %ymm0
	vmovups	%ymm0, -496(%r9,%r11,8)
	vmovups	-464(%rdi,%r11,8), %ymm0
	vmovups	%ymm0, -464(%r9,%r11,8)
	vmovupd	-432(%rdi,%r11,8), %ymm0
	vmovupd	%ymm0, -432(%r9,%r11,8)
	cmpq	$318, %r11
	je	247
	vmovups	-400(%rdi,%r11,8), %ymm0
	vmovups	%ymm0, -400(%r9,%r11,8)
	vmovups	-368(%rdi,%r11,8), %ymm0
	vmovups	%ymm0, -368(%r9,%r11,8)
	vmovups	-336(%rdi,%r11,8), %ymm0
	vmovups	%ymm0, -336(%r9,%r11,8)
	vmovups	-304(%rdi,%r11,8), %ymm0
	vmovups	%ymm0, -304(%r9,%r11,8)
	vmovups	-272(%rdi,%r11,8), %ymm0
	vmovups	%ymm0, -272(%r9,%r11,8)
	vmovups	-240(%rdi,%r11,8), %ymm0
	vmovups	%ymm0, -240(%r9,%r11,8)
	vmovups	-208(%rdi,%r11,8), %ymm0
	vmovups	%ymm0, -208(%r9,%r11,8)
	vmovups	-176(%rdi,%r11,8), %ymm0
	vmovups	%ymm0, -176(%r9,%r11,8)
	vmovups	-144(%rdi,%r11,8), %ymm0
	vmovups	%ymm0, -144(%r9,%r11,8)
	vmovups	-112(%rdi,%r11,8), %ymm0
	vmovups	%ymm0, -112(%r9,%r11,8)
	vmovups	-80(%rdi,%r11,8), %ymm0
	vmovups	%ymm0, -80(%r9,%r11,8)
	vmovups	-48(%rdi,%r11,8), %ymm0
	vmovups	%ymm0, -48(%r9,%r11,8)
	vmovupd	-16(%rdi,%r11,8), %ymm0
	vmovupd	%ymm0, -16(%r9,%r11,8)
	addq	$64, %r11
	jmp	-318
	nop
	movl	$268, %r14d
	movl	$2, %ebx
	movq	%r14, %r11
	nop
	vmovsd	(%rdi,%r11,8), %xmm0
	vmovsd	%xmm0, (%r9,%r11,8)
	incq	%r11
	decq	%rbx
	jne	-20
	movq	%r14, %rbx
	orq	$-270, %rbx
	cmpq	$-4, %rbx
	ja	-904
	movl	$264, %ebx
	subq	%r14, %rbx
	movl	%ebx, %ebp
	notl	%ebp
	testb	$12, %bpl
	je	85
	movl	%ebx, %r14d
	shrl	$2, %r14d
	incl	%r14d
	andl	$3, %r14d
	negq	%r14
	nopl	(%rax,%rax)
	vmovsd	(%rdi,%r11,8), %xmm0
	vmovsd	%xmm0, (%r9,%r11,8)
	vmovsd	8(%rdi,%r11,8), %xmm0
	vmovsd	%xmm0, 8(%r9,%r11,8)
	vmovsd	16(%rdi,%r11,8), %xmm0
	vmovsd	%xmm0, 16(%r9,%r11,8)
	vmovsd	24(%rdi,%r11,8), %xmm0
	vmovsd	%xmm0, 24(%r9,%r11,8)
	addq	$4, %r11
	incq	%r14
	jne	-63
	cmpq	$12, %rbx
	jb	-1017
	addq	$15, %r11
	nopl	(%rax)
	vmovsd	-120(%rdi,%r11,8), %xmm0
	vmovsd	%xmm0, -120(%r9,%r11,8)
	vmovsd	-112(%rdi,%r11,8), %xmm0
	vmovsd	%xmm0, -112(%r9,%r11,8)
	vmovsd	-104(%rdi,%r11,8), %xmm0
	vmovsd	%xmm0, -104(%r9,%r11,8)
	vmovsd	-96(%rdi,%r11,8), %xmm0
	vmovsd	%xmm0, -96(%r9,%r11,8)
	vmovsd	-88(%rdi,%r11,8), %xmm0
	vmovsd	%xmm0, -88(%r9,%r11,8)
	vmovsd	-80(%rdi,%r11,8), %xmm0
	vmovsd	%xmm0, -80(%r9,%r11,8)
	vmovsd	-72(%rdi,%r11,8), %xmm0
	vmovsd	%xmm0, -72(%r9,%r11,8)
	vmovsd	-64(%rdi,%r11,8), %xmm0
	vmovsd	%xmm0, -64(%r9,%r11,8)
	vmovsd	-56(%rdi,%r11,8), %xmm0
	vmovsd	%xmm0, -56(%r9,%r11,8)
	vmovsd	-48(%rdi,%r11,8), %xmm0
	vmovsd	%xmm0, -48(%r9,%r11,8)
	vmovsd	-40(%rdi,%r11,8), %xmm0
	vmovsd	%xmm0, -40(%r9,%r11,8)
	vmovsd	-32(%rdi,%r11,8), %xmm0
	vmovsd	%xmm0, -32(%r9,%r11,8)
	vmovsd	-24(%rdi,%r11,8), %xmm0
	vmovsd	%xmm0, -24(%r9,%r11,8)
	vmovsd	-16(%rdi,%r11,8), %xmm0
	vmovsd	%xmm0, -16(%r9,%r11,8)
	vmovsd	-8(%rdi,%r11,8), %xmm0
	vmovsd	%xmm0, -8(%r9,%r11,8)
	vmovsd	(%rdi,%r11,8), %xmm0
	vmovsd	%xmm0, (%r9,%r11,8)
	addq	$16, %r11
	cmpq	$285, %r11
	jne	-239
	jmp	-1268
	popq	%rbx
	popq	%r14
	popq	%r15
	popq	%rbp
	vzeroupper
	retq
