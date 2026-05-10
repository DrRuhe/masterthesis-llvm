	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	leaq	-7(%rip), %rcx
	movabsq	$-7962, %rax
	addq	%rcx, %rax
	movabsq	$96, %rcx
	movq	(%rax,%rcx), %rcx
	movq	(%rcx), %rdx
	movabsq	$88, %rcx
	movq	(%rax,%rcx), %rcx
	movq	(%rcx), %rsi
	leaq	166400(%rsi), %rcx
	movq	%rcx, -32(%rsp)
	leaq	240000(%rdx), %rbx
	movl	$1, %r9d
	xorl	%r8d, %r8d
	movabsq	$-1584, %rcx
	vmovsd	(%rax,%rcx), %xmm0
	movabsq	$-1600, %rdi
	movq	%rdx, -48(%rsp)
	movq	%rsi, %r11
	movq	%rdx, -24(%rsp)
	movq	%rax, -40(%rsp)
	jmp	68
	nopl	(%rax)
	movq	-24(%rsp), %rdx
	movq	-40(%rsp), %rax
	movabsq	$-1600, %rdi
	movq	-56(%rsp), %r10
	incq	%r8
	addq	$20800, %r11
	addq	$8, -48(%rsp)
	addq	$8, %rbx
	movq	%r10, %r9
	cmpq	$2000, %r8
	je	1368
	cmpq	$1999, %r8
	jae	106
	imulq	$16000, %r9, %rcx
	addq	%rdx, %rcx
	leaq	(%rcx,%r8,8), %r15
	leaq	1(%r9), %r10
	cmpq	$1998, %r8
	jne	603
	xorl	%ecx, %ecx
	nopw	(%rax,%rax)
	vmovsd	(%r15), %xmm1
	vmovsd	20800(%r11,%rcx,8), %xmm2
	vfmadd213sd	41558400(%rsi,%rcx,8), %xmm1, %xmm2
	vmulsd	%xmm0, %xmm2, %xmm1
	vmovsd	%xmm1, 41558400(%rsi,%rcx,8)
	incq	%rcx
	cmpq	$2600, %rcx
	jne	-50
	jmp	-142
	nopw	(%rax,%rax)
	movl	$50, %ecx
	nopw	%cs:(%rax,%rax)
	vmovapd	(%rax,%rdi), %xmm1
	vmulpd	-400(%r11,%rcx,8), %xmm1, %xmm2
	vmulpd	-384(%r11,%rcx,8), %xmm1, %xmm3
	vmovupd	%xmm2, -400(%r11,%rcx,8)
	vmovupd	%xmm3, -384(%r11,%rcx,8)
	vmulpd	-368(%r11,%rcx,8), %xmm1, %xmm2
	vmulpd	-352(%r11,%rcx,8), %xmm1, %xmm3
	vmovupd	%xmm2, -368(%r11,%rcx,8)
	vmovupd	%xmm3, -352(%r11,%rcx,8)
	vmulpd	-336(%r11,%rcx,8), %xmm1, %xmm2
	vmulpd	-320(%r11,%rcx,8), %xmm1, %xmm3
	vmovupd	%xmm2, -336(%r11,%rcx,8)
	vmovupd	%xmm3, -320(%r11,%rcx,8)
	vmulpd	-304(%r11,%rcx,8), %xmm1, %xmm2
	vmulpd	-288(%r11,%rcx,8), %xmm1, %xmm3
	vmovupd	%xmm2, -304(%r11,%rcx,8)
	vmovupd	%xmm3, -288(%r11,%rcx,8)
	vmulpd	-272(%r11,%rcx,8), %xmm1, %xmm2
	vmulpd	-256(%r11,%rcx,8), %xmm1, %xmm3
	vmovupd	%xmm2, -272(%r11,%rcx,8)
	vmovupd	%xmm3, -256(%r11,%rcx,8)
	vmulpd	-240(%r11,%rcx,8), %xmm1, %xmm2
	vmulpd	-224(%r11,%rcx,8), %xmm1, %xmm3
	vmovupd	%xmm2, -240(%r11,%rcx,8)
	vmovupd	%xmm3, -224(%r11,%rcx,8)
	vmulpd	-208(%r11,%rcx,8), %xmm1, %xmm2
	vmulpd	-192(%r11,%rcx,8), %xmm1, %xmm3
	vmovupd	%xmm2, -208(%r11,%rcx,8)
	vmovupd	%xmm3, -192(%r11,%rcx,8)
	vmulpd	-176(%r11,%rcx,8), %xmm1, %xmm2
	vmulpd	-160(%r11,%rcx,8), %xmm1, %xmm3
	vmovupd	%xmm2, -176(%r11,%rcx,8)
	vmovupd	%xmm3, -160(%r11,%rcx,8)
	vmulpd	-144(%r11,%rcx,8), %xmm1, %xmm2
	vmulpd	-128(%r11,%rcx,8), %xmm1, %xmm3
	vmovupd	%xmm2, -144(%r11,%rcx,8)
	vmovupd	%xmm3, -128(%r11,%rcx,8)
	vmulpd	-112(%r11,%rcx,8), %xmm1, %xmm2
	vmulpd	-96(%r11,%rcx,8), %xmm1, %xmm3
	vmovupd	%xmm2, -112(%r11,%rcx,8)
	vmovupd	%xmm3, -96(%r11,%rcx,8)
	vmulpd	-80(%r11,%rcx,8), %xmm1, %xmm2
	vmulpd	-64(%r11,%rcx,8), %xmm1, %xmm3
	vmovupd	%xmm2, -80(%r11,%rcx,8)
	vmovupd	%xmm3, -64(%r11,%rcx,8)
	vmulpd	-48(%r11,%rcx,8), %xmm1, %xmm2
	vmulpd	-32(%r11,%rcx,8), %xmm1, %xmm3
	vmovupd	%xmm2, -48(%r11,%rcx,8)
	vmovupd	%xmm3, -32(%r11,%rcx,8)
	vmulpd	-16(%r11,%rcx,8), %xmm1, %xmm2
	vmulpd	(%r11,%rcx,8), %xmm1, %xmm1
	vmovupd	%xmm2, -16(%r11,%rcx,8)
	vmovupd	%xmm1, (%r11,%rcx,8)
	addq	$52, %rcx
	cmpq	$2650, %rcx
	jne	-486
	incq	%r9
	movq	%r9, %r10
	jmp	-664
	nopw	%cs:(%rax,%rax)
	movq	%r10, -56(%rsp)
	imulq	$20800, %r8, %r12
	addq	%rsi, %r12
	movq	%r9, -8(%rsp)
	imulq	$20800, %r9, %rax
	addq	%rsi, %rax
	movq	%rax, -16(%rsp)
	movq	-32(%rsp), %rbp
	movq	%rsi, %rdx
	xorl	%ecx, %ecx
	jmp	35
	nop
	vmulsd	%xmm0, %xmm1, %xmm1
	vmovsd	%xmm1, (%r13)
	incq	%rcx
	addq	$8, %rdx
	addq	$8, %rbp
	cmpq	$2600, %rcx
	je	-786
	leaq	(%r12,%rcx,8), %r13
	vmovsd	(%r12,%rcx,8), %xmm1
	testb	$1, %r8b
	jne	30
	vmovsd	(%r15), %xmm2
	movq	-16(%rsp), %rax
	vfmadd231sd	(%rax,%rcx,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%r13)
	movq	-56(%rsp), %r10
	jmp	6
	nop
	movq	-8(%rsp), %r10
	movl	$1998, %r9d
	subq	%r10, %r9
	movl	%r9d, %edi
	notl	%edi
	testb	$14, %dil
	je	125
	movq	%rbx, %rax
	movl	%r9d, %r14d
	shrl	%r14d
	incl	%r14d
	andl	$7, %r14d
	negq	%r14
	imulq	$16000, %r10, %rbx
	addq	-48(%rsp), %rbx
	imulq	$20800, %r10, %rdi
	addq	%rdx, %rdi
	nopw	%cs:(%rax,%rax)
	vmovsd	(%rbx), %xmm2
	vfmadd132sd	(%rdi), %xmm1, %xmm2
	vmovsd	%xmm2, (%r13)
	vmovsd	16000(%rbx), %xmm1
	vfmadd132sd	20800(%rdi), %xmm2, %xmm1
	vmovsd	%xmm1, (%r13)
	addq	$2, %r10
	addq	$32000, %rbx
	addq	$41600, %rdi
	incq	%r14
	jne	-61
	cmpq	$14, %r9
	movq	%rax, %rbx
	jb	-234
	leaq	-2000(%r10), %r9
	imulq	$20800, %r10, %r14
	addq	%rbp, %r14
	imulq	$16000, %r10, %r10
	addq	%rbx, %r10
	nopw	%cs:(%rax,%rax)
	vmovsd	-240000(%r10), %xmm2
	vfmadd132sd	-166400(%r14), %xmm1, %xmm2
	vmovsd	%xmm2, (%r13)
	vmovsd	-224000(%r10), %xmm1
	vfmadd132sd	-145600(%r14), %xmm2, %xmm1
	vmovsd	%xmm1, (%r13)
	vmovsd	-208000(%r10), %xmm2
	vfmadd132sd	-124800(%r14), %xmm1, %xmm2
	vmovsd	%xmm2, (%r13)
	vmovsd	-192000(%r10), %xmm1
	vfmadd132sd	-104000(%r14), %xmm2, %xmm1
	vmovsd	%xmm1, (%r13)
	vmovsd	-176000(%r10), %xmm2
	vfmadd132sd	-83200(%r14), %xmm1, %xmm2
	vmovsd	%xmm2, (%r13)
	vmovsd	-160000(%r10), %xmm1
	vfmadd132sd	-62400(%r14), %xmm2, %xmm1
	vmovsd	%xmm1, (%r13)
	vmovsd	-144000(%r10), %xmm2
	vfmadd132sd	-41600(%r14), %xmm1, %xmm2
	vmovsd	%xmm2, (%r13)
	vmovsd	-128000(%r10), %xmm1
	vfmadd132sd	-20800(%r14), %xmm2, %xmm1
	vmovsd	%xmm1, (%r13)
	vmovsd	-112000(%r10), %xmm2
	vfmadd132sd	(%r14), %xmm1, %xmm2
	vmovsd	%xmm2, (%r13)
	vmovsd	-96000(%r10), %xmm1
	vfmadd132sd	20800(%r14), %xmm2, %xmm1
	vmovsd	%xmm1, (%r13)
	vmovsd	-80000(%r10), %xmm2
	vfmadd132sd	41600(%r14), %xmm1, %xmm2
	vmovsd	%xmm2, (%r13)
	vmovsd	-64000(%r10), %xmm1
	vfmadd132sd	62400(%r14), %xmm2, %xmm1
	vmovsd	%xmm1, (%r13)
	vmovsd	-48000(%r10), %xmm2
	vfmadd132sd	83200(%r14), %xmm1, %xmm2
	vmovsd	%xmm2, (%r13)
	vmovsd	-32000(%r10), %xmm1
	vfmadd132sd	104000(%r14), %xmm2, %xmm1
	vmovsd	%xmm1, (%r13)
	vmovsd	-16000(%r10), %xmm2
	vfmadd132sd	124800(%r14), %xmm1, %xmm2
	vmovsd	%xmm2, (%r13)
	vmovsd	(%r10), %xmm1
	vfmadd132sd	145600(%r14), %xmm2, %xmm1
	vmovsd	%xmm1, (%r13)
	addq	$332800, %r14
	addq	$256000, %r10
	addq	$16, %r9
	jne	-400
	jmp	-677
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
