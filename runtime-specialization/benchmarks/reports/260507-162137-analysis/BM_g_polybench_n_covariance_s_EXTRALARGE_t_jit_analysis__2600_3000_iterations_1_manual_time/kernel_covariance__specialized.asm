	leaq	-7(%rip), %rax
	movabsq	$-7968, %rdx
	addq	%rax, %rdx
	movabsq	$184, %rax
	movq	(%rdx,%rax), %rax
	movq	(%rax), %rax
	movabsq	$32, %rcx
	movq	(%rdx,%rcx), %rcx
	movq	(%rcx), %rcx
	xorl	%edi, %edi
	movabsq	$200, %rsi
	movq	(%rdx,%rsi), %rsi
	movabsq	$-1584, %r8
	vmovsd	(%rdx,%r8), %xmm0
	movq	%rax, %r8
	nopl	(%rax)
	movq	$0, (%rsi,%rdi,8)
	vxorpd	%xmm1, %xmm1, %xmm1
	xorl	%r9d, %r9d
	nop
	vaddsd	(%r8,%r9), %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	vaddsd	20800(%r8,%r9), %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	vaddsd	41600(%r8,%r9), %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	vaddsd	62400(%r8,%r9), %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	vaddsd	83200(%r8,%r9), %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	vaddsd	104000(%r8,%r9), %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	vaddsd	124800(%r8,%r9), %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	vaddsd	145600(%r8,%r9), %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	vaddsd	166400(%r8,%r9), %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	vaddsd	187200(%r8,%r9), %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	vaddsd	208000(%r8,%r9), %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	vaddsd	228800(%r8,%r9), %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	vaddsd	249600(%r8,%r9), %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	vaddsd	270400(%r8,%r9), %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	vaddsd	291200(%r8,%r9), %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	vaddsd	312000(%r8,%r9), %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	vaddsd	332800(%r8,%r9), %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	vaddsd	353600(%r8,%r9), %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	vaddsd	374400(%r8,%r9), %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	vaddsd	395200(%r8,%r9), %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	vaddsd	416000(%r8,%r9), %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	vaddsd	436800(%r8,%r9), %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	vaddsd	457600(%r8,%r9), %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	vaddsd	478400(%r8,%r9), %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	addq	$499200, %r9
	cmpq	$62400000, %r9
	jne	-376
	vdivsd	%xmm0, %xmm1, %xmm1
	vmovsd	%xmm1, (%rsi,%rdi,8)
	incq	%rdi
	addq	$8, %r8
	cmpq	$2600, %rdi
	jne	-421
	leaq	62400000(%rax), %rdi
	leaq	20800(%rsi), %r8
	cmpq	%r8, %rax
	setb	%r8b
	cmpq	%rsi, %rdi
	seta	%dil
	testb	%dil, %r8b
	je	511
	xorl	%edi, %edi
	movq	%rax, %r8
	nop
	movl	$19, %r9d
	nopw	%cs:(%rax,%rax)
	vmovsd	-152(%r8,%r9,8), %xmm0
	vmovsd	-144(%r8,%r9,8), %xmm1
	vsubsd	-152(%rsi,%r9,8), %xmm0, %xmm0
	vmovsd	%xmm0, -152(%r8,%r9,8)
	vsubsd	-144(%rsi,%r9,8), %xmm1, %xmm0
	vmovsd	%xmm0, -144(%r8,%r9,8)
	vmovsd	-136(%r8,%r9,8), %xmm0
	vsubsd	-136(%rsi,%r9,8), %xmm0, %xmm0
	vmovsd	%xmm0, -136(%r8,%r9,8)
	vmovsd	-128(%r8,%r9,8), %xmm0
	vsubsd	-128(%rsi,%r9,8), %xmm0, %xmm0
	vmovsd	%xmm0, -128(%r8,%r9,8)
	vmovsd	-120(%r8,%r9,8), %xmm0
	vsubsd	-120(%rsi,%r9,8), %xmm0, %xmm0
	vmovsd	%xmm0, -120(%r8,%r9,8)
	vmovsd	-112(%r8,%r9,8), %xmm0
	vsubsd	-112(%rsi,%r9,8), %xmm0, %xmm0
	vmovsd	%xmm0, -112(%r8,%r9,8)
	vmovsd	-104(%r8,%r9,8), %xmm0
	vsubsd	-104(%rsi,%r9,8), %xmm0, %xmm0
	vmovsd	%xmm0, -104(%r8,%r9,8)
	vmovsd	-96(%r8,%r9,8), %xmm0
	vsubsd	-96(%rsi,%r9,8), %xmm0, %xmm0
	vmovsd	%xmm0, -96(%r8,%r9,8)
	vmovsd	-88(%r8,%r9,8), %xmm0
	vsubsd	-88(%rsi,%r9,8), %xmm0, %xmm0
	vmovsd	%xmm0, -88(%r8,%r9,8)
	vmovsd	-80(%r8,%r9,8), %xmm0
	vsubsd	-80(%rsi,%r9,8), %xmm0, %xmm0
	vmovsd	%xmm0, -80(%r8,%r9,8)
	vmovsd	-72(%r8,%r9,8), %xmm0
	vsubsd	-72(%rsi,%r9,8), %xmm0, %xmm0
	vmovsd	%xmm0, -72(%r8,%r9,8)
	vmovsd	-64(%r8,%r9,8), %xmm0
	vsubsd	-64(%rsi,%r9,8), %xmm0, %xmm0
	vmovsd	%xmm0, -64(%r8,%r9,8)
	vmovsd	-56(%r8,%r9,8), %xmm0
	vsubsd	-56(%rsi,%r9,8), %xmm0, %xmm0
	vmovsd	%xmm0, -56(%r8,%r9,8)
	vmovsd	-48(%r8,%r9,8), %xmm0
	vsubsd	-48(%rsi,%r9,8), %xmm0, %xmm0
	vmovsd	%xmm0, -48(%r8,%r9,8)
	vmovsd	-40(%r8,%r9,8), %xmm0
	vsubsd	-40(%rsi,%r9,8), %xmm0, %xmm0
	vmovsd	%xmm0, -40(%r8,%r9,8)
	vmovsd	-32(%r8,%r9,8), %xmm0
	vsubsd	-32(%rsi,%r9,8), %xmm0, %xmm0
	vmovsd	%xmm0, -32(%r8,%r9,8)
	vmovsd	-24(%r8,%r9,8), %xmm0
	vsubsd	-24(%rsi,%r9,8), %xmm0, %xmm0
	vmovsd	%xmm0, -24(%r8,%r9,8)
	vmovsd	-16(%r8,%r9,8), %xmm0
	vsubsd	-16(%rsi,%r9,8), %xmm0, %xmm0
	vmovsd	%xmm0, -16(%r8,%r9,8)
	vmovsd	-8(%r8,%r9,8), %xmm0
	vsubsd	-8(%rsi,%r9,8), %xmm0, %xmm0
	vmovsd	%xmm0, -8(%r8,%r9,8)
	vmovsd	(%r8,%r9,8), %xmm0
	vsubsd	(%rsi,%r9,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%r8,%r9,8)
	addq	$20, %r9
	cmpq	$2619, %r9
	jne	-461
	incq	%rdi
	addq	$20800, %r8
	cmpq	$3000, %rdi
	jne	-500
	jmp	579
	xorl	%edi, %edi
	movq	%rax, %r8
	nop
	movl	$38, %r9d
	nopw	%cs:(%rax,%rax)
	vmovupd	-304(%r8,%r9,8), %xmm0
	vmovupd	-288(%r8,%r9,8), %xmm1
	vmovupd	-272(%r8,%r9,8), %xmm2
	vmovupd	-256(%r8,%r9,8), %xmm3
	vsubpd	-304(%rsi,%r9,8), %xmm0, %xmm0
	vsubpd	-288(%rsi,%r9,8), %xmm1, %xmm1
	vmovupd	%xmm0, -304(%r8,%r9,8)
	vmovupd	%xmm1, -288(%r8,%r9,8)
	vsubpd	-272(%rsi,%r9,8), %xmm2, %xmm0
	vsubpd	-256(%rsi,%r9,8), %xmm3, %xmm1
	vmovupd	%xmm0, -272(%r8,%r9,8)
	vmovupd	%xmm1, -256(%r8,%r9,8)
	vmovupd	-240(%r8,%r9,8), %xmm0
	vmovupd	-224(%r8,%r9,8), %xmm1
	vsubpd	-240(%rsi,%r9,8), %xmm0, %xmm0
	vsubpd	-224(%rsi,%r9,8), %xmm1, %xmm1
	vmovupd	%xmm0, -240(%r8,%r9,8)
	vmovupd	%xmm1, -224(%r8,%r9,8)
	vmovupd	-208(%r8,%r9,8), %xmm0
	vmovupd	-192(%r8,%r9,8), %xmm1
	vsubpd	-208(%rsi,%r9,8), %xmm0, %xmm0
	vsubpd	-192(%rsi,%r9,8), %xmm1, %xmm1
	vmovupd	%xmm0, -208(%r8,%r9,8)
	vmovupd	%xmm1, -192(%r8,%r9,8)
	vmovupd	-176(%r8,%r9,8), %xmm0
	vmovupd	-160(%r8,%r9,8), %xmm1
	vsubpd	-176(%rsi,%r9,8), %xmm0, %xmm0
	vsubpd	-160(%rsi,%r9,8), %xmm1, %xmm1
	vmovupd	%xmm0, -176(%r8,%r9,8)
	vmovupd	%xmm1, -160(%r8,%r9,8)
	vmovupd	-144(%r8,%r9,8), %xmm0
	vmovupd	-128(%r8,%r9,8), %xmm1
	vsubpd	-144(%rsi,%r9,8), %xmm0, %xmm0
	vsubpd	-128(%rsi,%r9,8), %xmm1, %xmm1
	vmovupd	%xmm0, -144(%r8,%r9,8)
	vmovupd	%xmm1, -128(%r8,%r9,8)
	vmovupd	-112(%r8,%r9,8), %xmm0
	vmovupd	-96(%r8,%r9,8), %xmm1
	vsubpd	-112(%rsi,%r9,8), %xmm0, %xmm0
	vsubpd	-96(%rsi,%r9,8), %xmm1, %xmm1
	vmovupd	%xmm0, -112(%r8,%r9,8)
	vmovupd	%xmm1, -96(%r8,%r9,8)
	vmovupd	-80(%r8,%r9,8), %xmm0
	vmovupd	-64(%r8,%r9,8), %xmm1
	vsubpd	-80(%rsi,%r9,8), %xmm0, %xmm0
	vsubpd	-64(%rsi,%r9,8), %xmm1, %xmm1
	vmovupd	%xmm0, -80(%r8,%r9,8)
	vmovupd	%xmm1, -64(%r8,%r9,8)
	vmovupd	-48(%r8,%r9,8), %xmm0
	vmovupd	-32(%r8,%r9,8), %xmm1
	vsubpd	-48(%rsi,%r9,8), %xmm0, %xmm0
	vsubpd	-32(%rsi,%r9,8), %xmm1, %xmm1
	vmovupd	%xmm0, -48(%r8,%r9,8)
	vmovupd	%xmm1, -32(%r8,%r9,8)
	vmovupd	-16(%r8,%r9,8), %xmm0
	vmovupd	(%r8,%r9,8), %xmm1
	vsubpd	-16(%rsi,%r9,8), %xmm0, %xmm0
	vsubpd	(%rsi,%r9,8), %xmm1, %xmm1
	vmovupd	%xmm0, -16(%r8,%r9,8)
	vmovupd	%xmm1, (%r8,%r9,8)
	addq	$40, %r9
	cmpq	$2638, %r9
	jne	-533
	incq	%rdi
	addq	$20800, %r8
	cmpq	$3000, %rdi
	jne	-572
	xorl	%esi, %esi
	movabsq	$-1576, %rdi
	vmovsd	(%rdx,%rdi), %xmm0
	nopl	(%rax)
	imulq	$20800, %rsi, %rdx
	addq	%rcx, %rdx
	movq	%rax, %rdi
	movq	%rsi, %r8
	leaq	(%rdx,%r8,8), %r9
	movq	$0, (%rdx,%r8,8)
	vxorpd	%xmm1, %xmm1, %xmm1
	xorl	%r10d, %r10d
	nopw	%cs:(%rax,%rax)
	vmovsd	(%rax,%r10), %xmm2
	vfmadd132sd	(%rdi,%r10), %xmm1, %xmm2
	vmovsd	%xmm2, (%r9)
	vmovsd	20800(%rax,%r10), %xmm1
	vfmadd132sd	20800(%rdi,%r10), %xmm2, %xmm1
	vmovsd	%xmm1, (%r9)
	vmovsd	41600(%rax,%r10), %xmm2
	vfmadd132sd	41600(%rdi,%r10), %xmm1, %xmm2
	vmovsd	%xmm2, (%r9)
	vmovsd	62400(%rax,%r10), %xmm1
	vfmadd132sd	62400(%rdi,%r10), %xmm2, %xmm1
	vmovsd	%xmm1, (%r9)
	vmovsd	83200(%rax,%r10), %xmm2
	vfmadd132sd	83200(%rdi,%r10), %xmm1, %xmm2
	vmovsd	%xmm2, (%r9)
	vmovsd	104000(%rax,%r10), %xmm1
	vfmadd132sd	104000(%rdi,%r10), %xmm2, %xmm1
	vmovsd	%xmm1, (%r9)
	vmovsd	124800(%rax,%r10), %xmm2
	vfmadd132sd	124800(%rdi,%r10), %xmm1, %xmm2
	vmovsd	%xmm2, (%r9)
	vmovsd	145600(%rax,%r10), %xmm1
	vfmadd132sd	145600(%rdi,%r10), %xmm2, %xmm1
	vmovsd	%xmm1, (%r9)
	vmovsd	166400(%rax,%r10), %xmm2
	vfmadd132sd	166400(%rdi,%r10), %xmm1, %xmm2
	vmovsd	%xmm2, (%r9)
	vmovsd	187200(%rax,%r10), %xmm1
	vfmadd132sd	187200(%rdi,%r10), %xmm2, %xmm1
	vmovsd	%xmm1, (%r9)
	vmovsd	208000(%rax,%r10), %xmm2
	vfmadd132sd	208000(%rdi,%r10), %xmm1, %xmm2
	vmovsd	%xmm2, (%r9)
	vmovsd	228800(%rax,%r10), %xmm1
	vfmadd132sd	228800(%rdi,%r10), %xmm2, %xmm1
	vmovsd	%xmm1, (%r9)
	addq	$249600, %r10
	cmpq	$62400000, %r10
	jne	-312
	vdivsd	%xmm0, %xmm1, %xmm1
	vmovsd	%xmm1, (%r9)
	imulq	$20800, %r8, %r9
	addq	%rcx, %r9
	vmovsd	%xmm1, (%r9,%rsi,8)
	incq	%r8
	addq	$8, %rdi
	cmpq	$2600, %r8
	jne	-389
	incq	%rsi
	addq	$8, %rax
	cmpq	$2600, %rsi
	jne	-425
	retq
