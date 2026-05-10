	pushq	%r15
	pushq	%r14
	pushq	%r12
	pushq	%rbx
	leaq	-7(%rip), %rax
	movabsq	$-7975, %rdi
	addq	%rax, %rdi
	movabsq	$48, %rax
	movq	(%rdi,%rax), %rax
	movq	(%rax), %rax
	movabsq	$40, %rcx
	movq	(%rdi,%rcx), %rcx
	movq	(%rcx), %rsi
	movabsq	$152, %rcx
	movq	(%rdi,%rcx), %rcx
	movq	(%rcx), %r8
	movabsq	$144, %rcx
	movq	(%rdi,%rcx), %rcx
	movq	(%rcx), %rcx
	movabsq	$96, %rdx
	movq	(%rdi,%rdx), %rdx
	movq	(%rdx), %rdx
	xorl	%r9d, %r9d
	movabsq	$-1584, %r10
	vmovsd	(%rdi,%r10), %xmm0
	nopw	%cs:(%rax,%rax)
	imulq	$14400, %r9, %r10
	addq	%rax, %r10
	movq	%r8, %r11
	xorl	%ebx, %ebx
	nop
	leaq	(%r10,%rbx,8), %r14
	movq	$0, (%r10,%rbx,8)
	vxorpd	%xmm1, %xmm1, %xmm1
	movl	$9, %r15d
	movq	%r11, %r12
	nopl	(%rax)
	vmulsd	-72(%rsi,%r15,8), %xmm0, %xmm2
	vfmadd132sd	(%r12), %xmm1, %xmm2
	vmovsd	%xmm2, (%r14)
	vmulsd	-64(%rsi,%r15,8), %xmm0, %xmm1
	vfmadd132sd	14400(%r12), %xmm2, %xmm1
	vmovsd	%xmm1, (%r14)
	vmulsd	-56(%rsi,%r15,8), %xmm0, %xmm2
	vfmadd132sd	28800(%r12), %xmm1, %xmm2
	vmovsd	%xmm2, (%r14)
	vmulsd	-48(%rsi,%r15,8), %xmm0, %xmm1
	vfmadd132sd	43200(%r12), %xmm2, %xmm1
	vmovsd	%xmm1, (%r14)
	vmulsd	-40(%rsi,%r15,8), %xmm0, %xmm2
	vfmadd132sd	57600(%r12), %xmm1, %xmm2
	vmovsd	%xmm2, (%r14)
	vmulsd	-32(%rsi,%r15,8), %xmm0, %xmm1
	vfmadd132sd	72000(%r12), %xmm2, %xmm1
	vmovsd	%xmm1, (%r14)
	vmulsd	-24(%rsi,%r15,8), %xmm0, %xmm2
	vfmadd132sd	86400(%r12), %xmm1, %xmm2
	vmovsd	%xmm2, (%r14)
	vmulsd	-16(%rsi,%r15,8), %xmm0, %xmm1
	vfmadd132sd	100800(%r12), %xmm2, %xmm1
	vmovsd	%xmm1, (%r14)
	vmulsd	-8(%rsi,%r15,8), %xmm0, %xmm2
	vfmadd132sd	115200(%r12), %xmm1, %xmm2
	vmovsd	%xmm2, (%r14)
	vmulsd	(%rsi,%r15,8), %xmm0, %xmm1
	vfmadd132sd	129600(%r12), %xmm2, %xmm1
	vmovsd	%xmm1, (%r14)
	addq	$10, %r15
	addq	$144000, %r12
	cmpq	$2209, %r15
	jne	-239
	incq	%rbx
	addq	$8, %r11
	cmpq	$1800, %rbx
	jne	-291
	incq	%r9
	addq	$17600, %rsi
	cmpq	$1600, %r9
	jne	-330
	xorl	%esi, %esi
	movabsq	$-1576, %r8
	vmovsd	(%rdi,%r8), %xmm0
	nopl	(%rax)
	imulq	$19200, %rsi, %rdi
	addq	%rdx, %rdi
	movq	%rcx, %r8
	xorl	%r9d, %r9d
	leaq	(%rdi,%r9,8), %r10
	vmulsd	(%rdi,%r9,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rdi,%r9,8)
	movl	$17, %r11d
	movq	%r8, %rbx
	nopl	(%rax)
	vmovsd	-136(%rax,%r11,8), %xmm2
	vfmadd132sd	(%rbx), %xmm1, %xmm2
	vmovsd	%xmm2, (%r10)
	vmovsd	-128(%rax,%r11,8), %xmm1
	vfmadd132sd	19200(%rbx), %xmm2, %xmm1
	vmovsd	%xmm1, (%r10)
	vmovsd	-120(%rax,%r11,8), %xmm2
	vfmadd132sd	38400(%rbx), %xmm1, %xmm2
	vmovsd	%xmm2, (%r10)
	vmovsd	-112(%rax,%r11,8), %xmm1
	vfmadd132sd	57600(%rbx), %xmm2, %xmm1
	vmovsd	%xmm1, (%r10)
	vmovsd	-104(%rax,%r11,8), %xmm2
	vfmadd132sd	76800(%rbx), %xmm1, %xmm2
	vmovsd	%xmm2, (%r10)
	vmovsd	-96(%rax,%r11,8), %xmm1
	vfmadd132sd	96000(%rbx), %xmm2, %xmm1
	vmovsd	%xmm1, (%r10)
	vmovsd	-88(%rax,%r11,8), %xmm2
	vfmadd132sd	115200(%rbx), %xmm1, %xmm2
	vmovsd	%xmm2, (%r10)
	vmovsd	-80(%rax,%r11,8), %xmm1
	vfmadd132sd	134400(%rbx), %xmm2, %xmm1
	vmovsd	%xmm1, (%r10)
	vmovsd	-72(%rax,%r11,8), %xmm2
	vfmadd132sd	153600(%rbx), %xmm1, %xmm2
	vmovsd	%xmm2, (%r10)
	vmovsd	-64(%rax,%r11,8), %xmm1
	vfmadd132sd	172800(%rbx), %xmm2, %xmm1
	vmovsd	%xmm1, (%r10)
	vmovsd	-56(%rax,%r11,8), %xmm2
	vfmadd132sd	192000(%rbx), %xmm1, %xmm2
	vmovsd	%xmm2, (%r10)
	vmovsd	-48(%rax,%r11,8), %xmm1
	vfmadd132sd	211200(%rbx), %xmm2, %xmm1
	vmovsd	%xmm1, (%r10)
	vmovsd	-40(%rax,%r11,8), %xmm2
	vfmadd132sd	230400(%rbx), %xmm1, %xmm2
	vmovsd	%xmm2, (%r10)
	vmovsd	-32(%rax,%r11,8), %xmm1
	vfmadd132sd	249600(%rbx), %xmm2, %xmm1
	vmovsd	%xmm1, (%r10)
	vmovsd	-24(%rax,%r11,8), %xmm2
	vfmadd132sd	268800(%rbx), %xmm1, %xmm2
	vmovsd	%xmm2, (%r10)
	vmovsd	-16(%rax,%r11,8), %xmm1
	vfmadd132sd	288000(%rbx), %xmm2, %xmm1
	vmovsd	%xmm1, (%r10)
	vmovsd	-8(%rax,%r11,8), %xmm2
	vfmadd132sd	307200(%rbx), %xmm1, %xmm2
	vmovsd	%xmm2, (%r10)
	vmovsd	(%rax,%r11,8), %xmm1
	vfmadd132sd	326400(%rbx), %xmm2, %xmm1
	vmovsd	%xmm1, (%r10)
	addq	$18, %r11
	addq	$345600, %rbx
	cmpq	$1817, %r11
	jne	-400
	incq	%r9
	addq	$8, %r8
	cmpq	$2400, %r9
	jne	-452
	incq	%rsi
	addq	$14400, %rax
	cmpq	$1600, %rsi
	jne	-490
	popq	%rbx
	popq	%r12
	popq	%r14
	popq	%r15
	retq
