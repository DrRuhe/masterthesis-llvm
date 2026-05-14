	pushq	%r15
	pushq	%r14
	pushq	%r12
	pushq	%rbx
	pushq	%rax
	leaq	-7(%rip), %rax
	movabsq	$-7992, %r12
	addq	%rax, %r12
	movabsq	$0, %rax
	movq	(%r12,%rax), %rax
	movq	(%rax), %r14
	movabsq	$128, %rax
	movq	(%r12,%rax), %rbx
	xorl	%r15d, %r15d
	movabsq	$120, %rax
	movl	$17600, %edx
	movq	%rbx, %rdi
	xorl	%esi, %esi
	callq	*(%r12,%rax)
	leaq	31680000(%r14), %rax
	cmpq	%rbx, %rax
	seta	%cl
	leaq	17600(%rbx), %rax
	cmpq	%rax, %r14
	setb	%al
	andb	%cl, %al
	movabsq	$136, %rcx
	movq	(%r12,%rcx), %rcx
	movabsq	$200, %rdx
	movq	(%r12,%rdx), %rdx
	jmp	23
	incq	%r15
	addq	$17600, %r14
	cmpq	$1800, %r15
	je	1513
	movq	$0, (%rcx,%r15,8)
	vxorpd	%xmm0, %xmm0, %xmm0
	movl	$19, %esi
	nopl	(%rax,%rax)
	vmovsd	-152(%r14,%rsi,8), %xmm1
	vfmadd132sd	-152(%rdx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rcx,%r15,8)
	vmovsd	-144(%r14,%rsi,8), %xmm0
	vfmadd132sd	-144(%rdx,%rsi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rcx,%r15,8)
	vmovsd	-136(%r14,%rsi,8), %xmm1
	vfmadd132sd	-136(%rdx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rcx,%r15,8)
	vmovsd	-128(%r14,%rsi,8), %xmm0
	vfmadd132sd	-128(%rdx,%rsi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rcx,%r15,8)
	vmovsd	-120(%r14,%rsi,8), %xmm1
	vfmadd132sd	-120(%rdx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rcx,%r15,8)
	vmovsd	-112(%r14,%rsi,8), %xmm0
	vfmadd132sd	-112(%rdx,%rsi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rcx,%r15,8)
	vmovsd	-104(%r14,%rsi,8), %xmm1
	vfmadd132sd	-104(%rdx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rcx,%r15,8)
	vmovsd	-96(%r14,%rsi,8), %xmm0
	vfmadd132sd	-96(%rdx,%rsi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rcx,%r15,8)
	vmovsd	-88(%r14,%rsi,8), %xmm1
	vfmadd132sd	-88(%rdx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rcx,%r15,8)
	vmovsd	-80(%r14,%rsi,8), %xmm0
	vfmadd132sd	-80(%rdx,%rsi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rcx,%r15,8)
	vmovsd	-72(%r14,%rsi,8), %xmm1
	vfmadd132sd	-72(%rdx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rcx,%r15,8)
	vmovsd	-64(%r14,%rsi,8), %xmm0
	vfmadd132sd	-64(%rdx,%rsi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rcx,%r15,8)
	vmovsd	-56(%r14,%rsi,8), %xmm1
	vfmadd132sd	-56(%rdx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rcx,%r15,8)
	vmovsd	-48(%r14,%rsi,8), %xmm0
	vfmadd132sd	-48(%rdx,%rsi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rcx,%r15,8)
	vmovsd	-40(%r14,%rsi,8), %xmm1
	vfmadd132sd	-40(%rdx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rcx,%r15,8)
	vmovsd	-32(%r14,%rsi,8), %xmm0
	vfmadd132sd	-32(%rdx,%rsi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rcx,%r15,8)
	vmovsd	-24(%r14,%rsi,8), %xmm1
	vfmadd132sd	-24(%rdx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rcx,%r15,8)
	vmovsd	-16(%r14,%rsi,8), %xmm0
	vfmadd132sd	-16(%rdx,%rsi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rcx,%r15,8)
	vmovsd	-8(%r14,%rsi,8), %xmm1
	vfmadd132sd	-8(%rdx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rcx,%r15,8)
	vmovsd	(%r14,%rsi,8), %xmm0
	vfmadd132sd	(%rdx,%rsi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rcx,%r15,8)
	addq	$20, %rsi
	cmpq	$2219, %rsi
	jne	-433
	testb	%al, %al
	je	455
	movl	$19, %esi
	nop
	vmovsd	-152(%r14,%rsi,8), %xmm1
	vfmadd213sd	-152(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, -152(%rbx,%rsi,8)
	vmovsd	-144(%r14,%rsi,8), %xmm1
	vfmadd213sd	-144(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, -144(%rbx,%rsi,8)
	vmovsd	-136(%r14,%rsi,8), %xmm1
	vfmadd213sd	-136(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, -136(%rbx,%rsi,8)
	vmovsd	-128(%r14,%rsi,8), %xmm1
	vfmadd213sd	-128(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, -128(%rbx,%rsi,8)
	vmovsd	-120(%r14,%rsi,8), %xmm1
	vfmadd213sd	-120(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, -120(%rbx,%rsi,8)
	vmovsd	-112(%r14,%rsi,8), %xmm1
	vfmadd213sd	-112(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, -112(%rbx,%rsi,8)
	vmovsd	-104(%r14,%rsi,8), %xmm1
	vfmadd213sd	-104(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, -104(%rbx,%rsi,8)
	vmovsd	-96(%r14,%rsi,8), %xmm1
	vfmadd213sd	-96(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, -96(%rbx,%rsi,8)
	vmovsd	-88(%r14,%rsi,8), %xmm1
	vfmadd213sd	-88(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, -88(%rbx,%rsi,8)
	vmovsd	-80(%r14,%rsi,8), %xmm1
	vfmadd213sd	-80(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, -80(%rbx,%rsi,8)
	vmovsd	-72(%r14,%rsi,8), %xmm1
	vfmadd213sd	-72(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, -72(%rbx,%rsi,8)
	vmovsd	-64(%r14,%rsi,8), %xmm1
	vfmadd213sd	-64(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, -64(%rbx,%rsi,8)
	vmovsd	-56(%r14,%rsi,8), %xmm1
	vfmadd213sd	-56(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, -56(%rbx,%rsi,8)
	vmovsd	-48(%r14,%rsi,8), %xmm1
	vfmadd213sd	-48(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, -48(%rbx,%rsi,8)
	vmovsd	-40(%r14,%rsi,8), %xmm1
	vfmadd213sd	-40(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, -40(%rbx,%rsi,8)
	vmovsd	-32(%r14,%rsi,8), %xmm1
	vfmadd213sd	-32(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, -32(%rbx,%rsi,8)
	vmovsd	-24(%r14,%rsi,8), %xmm1
	vfmadd213sd	-24(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, -24(%rbx,%rsi,8)
	vmovsd	-16(%r14,%rsi,8), %xmm1
	vfmadd213sd	-16(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, -16(%rbx,%rsi,8)
	vmovsd	-8(%r14,%rsi,8), %xmm1
	vfmadd213sd	-8(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, -8(%rbx,%rsi,8)
	vmovsd	(%r14,%rsi,8), %xmm1
	vfmadd213sd	(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rbx,%rsi,8)
	addq	$20, %rsi
	cmpq	$2219, %rsi
	jne	-441
	jmp	-942
	nop
	vmovddup	%xmm0, %xmm0
	movl	$42, %esi
	nopl	(%rax)
	vmovupd	-336(%r14,%rsi,8), %xmm1
	vmovupd	-320(%r14,%rsi,8), %xmm2
	vfmadd213pd	-336(%rbx,%rsi,8), %xmm0, %xmm1
	vfmadd213pd	-320(%rbx,%rsi,8), %xmm0, %xmm2
	vmovapd	%xmm1, -336(%rbx,%rsi,8)
	vmovapd	%xmm2, -320(%rbx,%rsi,8)
	vmovupd	-304(%r14,%rsi,8), %xmm1
	vmovupd	-288(%r14,%rsi,8), %xmm2
	vfmadd213pd	-304(%rbx,%rsi,8), %xmm0, %xmm1
	vfmadd213pd	-288(%rbx,%rsi,8), %xmm0, %xmm2
	vmovapd	%xmm1, -304(%rbx,%rsi,8)
	vmovapd	%xmm2, -288(%rbx,%rsi,8)
	vmovupd	-272(%r14,%rsi,8), %xmm1
	vmovupd	-256(%r14,%rsi,8), %xmm2
	vfmadd213pd	-272(%rbx,%rsi,8), %xmm0, %xmm1
	vfmadd213pd	-256(%rbx,%rsi,8), %xmm0, %xmm2
	vmovapd	%xmm1, -272(%rbx,%rsi,8)
	vmovapd	%xmm2, -256(%rbx,%rsi,8)
	vmovupd	-240(%r14,%rsi,8), %xmm1
	vmovupd	-224(%r14,%rsi,8), %xmm2
	vfmadd213pd	-240(%rbx,%rsi,8), %xmm0, %xmm1
	vfmadd213pd	-224(%rbx,%rsi,8), %xmm0, %xmm2
	vmovapd	%xmm1, -240(%rbx,%rsi,8)
	vmovapd	%xmm2, -224(%rbx,%rsi,8)
	vmovupd	-208(%r14,%rsi,8), %xmm1
	vmovupd	-192(%r14,%rsi,8), %xmm2
	vfmadd213pd	-208(%rbx,%rsi,8), %xmm0, %xmm1
	vfmadd213pd	-192(%rbx,%rsi,8), %xmm0, %xmm2
	vmovapd	%xmm1, -208(%rbx,%rsi,8)
	vmovapd	%xmm2, -192(%rbx,%rsi,8)
	vmovupd	-176(%r14,%rsi,8), %xmm1
	vmovupd	-160(%r14,%rsi,8), %xmm2
	vfmadd213pd	-176(%rbx,%rsi,8), %xmm0, %xmm1
	vfmadd213pd	-160(%rbx,%rsi,8), %xmm0, %xmm2
	vmovapd	%xmm1, -176(%rbx,%rsi,8)
	vmovapd	%xmm2, -160(%rbx,%rsi,8)
	vmovupd	-144(%r14,%rsi,8), %xmm1
	vmovupd	-128(%r14,%rsi,8), %xmm2
	vfmadd213pd	-144(%rbx,%rsi,8), %xmm0, %xmm1
	vfmadd213pd	-128(%rbx,%rsi,8), %xmm0, %xmm2
	vmovapd	%xmm1, -144(%rbx,%rsi,8)
	vmovapd	%xmm2, -128(%rbx,%rsi,8)
	vmovupd	-112(%r14,%rsi,8), %xmm1
	vmovupd	-96(%r14,%rsi,8), %xmm2
	vfmadd213pd	-112(%rbx,%rsi,8), %xmm0, %xmm1
	vfmadd213pd	-96(%rbx,%rsi,8), %xmm0, %xmm2
	vmovapd	%xmm1, -112(%rbx,%rsi,8)
	vmovapd	%xmm2, -96(%rbx,%rsi,8)
	vmovupd	-80(%r14,%rsi,8), %xmm1
	vmovupd	-64(%r14,%rsi,8), %xmm2
	vfmadd213pd	-80(%rbx,%rsi,8), %xmm0, %xmm1
	vfmadd213pd	-64(%rbx,%rsi,8), %xmm0, %xmm2
	vmovapd	%xmm1, -80(%rbx,%rsi,8)
	vmovapd	%xmm2, -64(%rbx,%rsi,8)
	vmovupd	-48(%r14,%rsi,8), %xmm1
	vmovupd	-32(%r14,%rsi,8), %xmm2
	vfmadd213pd	-48(%rbx,%rsi,8), %xmm0, %xmm1
	vfmadd213pd	-32(%rbx,%rsi,8), %xmm0, %xmm2
	vmovapd	%xmm1, -48(%rbx,%rsi,8)
	vmovapd	%xmm2, -32(%rbx,%rsi,8)
	vmovupd	-16(%r14,%rsi,8), %xmm1
	vmovupd	(%r14,%rsi,8), %xmm2
	vfmadd213pd	-16(%rbx,%rsi,8), %xmm0, %xmm1
	vfmadd213pd	(%rbx,%rsi,8), %xmm0, %xmm2
	vmovapd	%xmm1, -16(%rbx,%rsi,8)
	vmovapd	%xmm2, (%rbx,%rsi,8)
	addq	$44, %rsi
	cmpq	$2242, %rsi
	jne	-571
	jmp	-1536
	addq	$8, %rsp
	popq	%rbx
	popq	%r12
	popq	%r14
	popq	%r15
	retq
