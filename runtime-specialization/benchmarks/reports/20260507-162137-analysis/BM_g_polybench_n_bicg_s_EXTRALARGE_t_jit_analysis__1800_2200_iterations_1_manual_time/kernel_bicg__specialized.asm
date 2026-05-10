	pushq	%r15
	pushq	%r14
	pushq	%r12
	pushq	%rbx
	pushq	%rax
	leaq	-7(%rip), %rax
	movabsq	$-7992, %r12
	addq	%rax, %r12
	movabsq	$80, %rax
	movq	(%r12,%rax), %rax
	movq	(%rax), %r14
	movabsq	$64, %rax
	movq	(%r12,%rax), %rbx
	xorl	%r15d, %r15d
	movabsq	$0, %rax
	movl	$14400, %edx
	movq	%rbx, %rdi
	xorl	%esi, %esi
	callq	*(%r12,%rax)
	movabsq	$152, %rax
	movq	(%r12,%rax), %rax
	movabsq	$128, %rcx
	movq	(%r12,%rcx), %rcx
	movabsq	$112, %rdx
	movq	(%r12,%rdx), %rdx
	movq	$0, (%rax,%r15,8)
	vmovsd	(%rcx,%r15,8), %xmm0
	vxorpd	%xmm1, %xmm1, %xmm1
	movl	$11, %esi
	nopw	(%rax,%rax)
	vmovsd	-88(%r14,%rsi,8), %xmm2
	vfmadd213sd	-88(%rbx,%rsi,8), %xmm0, %xmm2
	vmovsd	%xmm2, -88(%rbx,%rsi,8)
	vmovsd	-88(%r14,%rsi,8), %xmm2
	vfmadd132sd	-88(%rdx,%rsi,8), %xmm1, %xmm2
	vmovsd	%xmm2, (%rax,%r15,8)
	vmovsd	-80(%r14,%rsi,8), %xmm1
	vfmadd213sd	-80(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, -80(%rbx,%rsi,8)
	vmovsd	-80(%r14,%rsi,8), %xmm1
	vfmadd132sd	-80(%rdx,%rsi,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%rax,%r15,8)
	vmovsd	-72(%r14,%rsi,8), %xmm2
	vfmadd213sd	-72(%rbx,%rsi,8), %xmm0, %xmm2
	vmovsd	%xmm2, -72(%rbx,%rsi,8)
	vmovsd	-72(%r14,%rsi,8), %xmm2
	vfmadd132sd	-72(%rdx,%rsi,8), %xmm1, %xmm2
	vmovsd	%xmm2, (%rax,%r15,8)
	vmovsd	-64(%r14,%rsi,8), %xmm1
	vfmadd213sd	-64(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, -64(%rbx,%rsi,8)
	vmovsd	-64(%r14,%rsi,8), %xmm1
	vfmadd132sd	-64(%rdx,%rsi,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%rax,%r15,8)
	vmovsd	-56(%r14,%rsi,8), %xmm2
	vfmadd213sd	-56(%rbx,%rsi,8), %xmm0, %xmm2
	vmovsd	%xmm2, -56(%rbx,%rsi,8)
	vmovsd	-56(%r14,%rsi,8), %xmm2
	vfmadd132sd	-56(%rdx,%rsi,8), %xmm1, %xmm2
	vmovsd	%xmm2, (%rax,%r15,8)
	vmovsd	-48(%r14,%rsi,8), %xmm1
	vfmadd213sd	-48(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, -48(%rbx,%rsi,8)
	vmovsd	-48(%r14,%rsi,8), %xmm1
	vfmadd132sd	-48(%rdx,%rsi,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%rax,%r15,8)
	vmovsd	-40(%r14,%rsi,8), %xmm2
	vfmadd213sd	-40(%rbx,%rsi,8), %xmm0, %xmm2
	vmovsd	%xmm2, -40(%rbx,%rsi,8)
	vmovsd	-40(%r14,%rsi,8), %xmm2
	vfmadd132sd	-40(%rdx,%rsi,8), %xmm1, %xmm2
	vmovsd	%xmm2, (%rax,%r15,8)
	vmovsd	-32(%r14,%rsi,8), %xmm1
	vfmadd213sd	-32(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, -32(%rbx,%rsi,8)
	vmovsd	-32(%r14,%rsi,8), %xmm1
	vfmadd132sd	-32(%rdx,%rsi,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%rax,%r15,8)
	vmovsd	-24(%r14,%rsi,8), %xmm2
	vfmadd213sd	-24(%rbx,%rsi,8), %xmm0, %xmm2
	vmovsd	%xmm2, -24(%rbx,%rsi,8)
	vmovsd	-24(%r14,%rsi,8), %xmm2
	vfmadd132sd	-24(%rdx,%rsi,8), %xmm1, %xmm2
	vmovsd	%xmm2, (%rax,%r15,8)
	vmovsd	-16(%r14,%rsi,8), %xmm1
	vfmadd213sd	-16(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, -16(%rbx,%rsi,8)
	vmovsd	-16(%r14,%rsi,8), %xmm1
	vfmadd132sd	-16(%rdx,%rsi,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%rax,%r15,8)
	vmovsd	-8(%r14,%rsi,8), %xmm2
	vfmadd213sd	-8(%rbx,%rsi,8), %xmm0, %xmm2
	vmovsd	%xmm2, -8(%rbx,%rsi,8)
	vmovsd	-8(%r14,%rsi,8), %xmm2
	vfmadd132sd	-8(%rdx,%rsi,8), %xmm1, %xmm2
	vmovsd	%xmm2, (%rax,%r15,8)
	vmovsd	(%r14,%rsi,8), %xmm1
	vfmadd213sd	(%rbx,%rsi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rbx,%rsi,8)
	vmovsd	(%r14,%rsi,8), %xmm1
	vfmadd132sd	(%rdx,%rsi,8), %xmm2, %xmm1
	vmovsd	%xmm1, (%rax,%r15,8)
	addq	$12, %rsi
	cmpq	$1811, %rsi
	jne	-492
	incq	%r15
	addq	$14400, %r14
	cmpq	$2200, %r15
	jne	-547
	addq	$8, %rsp
	popq	%rbx
	popq	%r12
	popq	%r14
	popq	%r15
	retq
