	leaq	-7(%rip), %rax
	movabsq	$-7984, %rcx
	addq	%rax, %rcx
	movabsq	$200, %rax
	movq	(%rcx,%rax), %rax
	movq	(%rax), %rax
	xorl	%edx, %edx
	movabsq	$8, %rsi
	movq	(%rcx,%rsi), %rsi
	movabsq	$192, %rdi
	movq	(%rcx,%rdi), %rdi
	movq	%rax, %r8
	nopw	%cs:(%rax,%rax)
	vmovsd	(%rsi,%rdx,8), %xmm0
	movl	$15, %r9d
	nopl	(%rax,%rax)
	vmovsd	-120(%r8,%r9,8), %xmm1
	vfmadd132sd	-120(%rdi,%r9,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rsi,%rdx,8)
	vmovsd	-112(%r8,%r9,8), %xmm0
	vfmadd132sd	-112(%rdi,%r9,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	vmovsd	-104(%r8,%r9,8), %xmm1
	vfmadd132sd	-104(%rdi,%r9,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rsi,%rdx,8)
	vmovsd	-96(%r8,%r9,8), %xmm0
	vfmadd132sd	-96(%rdi,%r9,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	vmovsd	-88(%r8,%r9,8), %xmm1
	vfmadd132sd	-88(%rdi,%r9,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rsi,%rdx,8)
	vmovsd	-80(%r8,%r9,8), %xmm0
	vfmadd132sd	-80(%rdi,%r9,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	vmovsd	-72(%r8,%r9,8), %xmm1
	vfmadd132sd	-72(%rdi,%r9,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rsi,%rdx,8)
	vmovsd	-64(%r8,%r9,8), %xmm0
	vfmadd132sd	-64(%rdi,%r9,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	vmovsd	-56(%r8,%r9,8), %xmm1
	vfmadd132sd	-56(%rdi,%r9,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rsi,%rdx,8)
	vmovsd	-48(%r8,%r9,8), %xmm0
	vfmadd132sd	-48(%rdi,%r9,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	vmovsd	-40(%r8,%r9,8), %xmm1
	vfmadd132sd	-40(%rdi,%r9,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rsi,%rdx,8)
	vmovsd	-32(%r8,%r9,8), %xmm0
	vfmadd132sd	-32(%rdi,%r9,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	vmovsd	-24(%r8,%r9,8), %xmm1
	vfmadd132sd	-24(%rdi,%r9,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rsi,%rdx,8)
	vmovsd	-16(%r8,%r9,8), %xmm0
	vfmadd132sd	-16(%rdi,%r9,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	vmovsd	-8(%r8,%r9,8), %xmm1
	vfmadd132sd	-8(%rdi,%r9,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rsi,%rdx,8)
	vmovsd	(%r8,%r9,8), %xmm0
	vfmadd132sd	(%rdi,%r9,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	addq	$16, %r9
	cmpq	$4015, %r9
	jne	-319
	incq	%rdx
	addq	$32000, %r8
	cmpq	$4000, %rdx
	jne	-358
	xorl	%edx, %edx
	movabsq	$144, %rsi
	movq	(%rcx,%rsi), %rsi
	movabsq	$104, %rdi
	movq	(%rcx,%rdi), %rcx
	nopw	%cs:(%rax,%rax)
	vmovsd	(%rsi,%rdx,8), %xmm0
	movl	$15, %edi
	movq	%rax, %r8
	nopl	(%rax)
	vmovsd	(%r8), %xmm1
	vfmadd132sd	-120(%rcx,%rdi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rsi,%rdx,8)
	vmovsd	32000(%r8), %xmm0
	vfmadd132sd	-112(%rcx,%rdi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	vmovsd	64000(%r8), %xmm1
	vfmadd132sd	-104(%rcx,%rdi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rsi,%rdx,8)
	vmovsd	96000(%r8), %xmm0
	vfmadd132sd	-96(%rcx,%rdi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	vmovsd	128000(%r8), %xmm1
	vfmadd132sd	-88(%rcx,%rdi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rsi,%rdx,8)
	vmovsd	160000(%r8), %xmm0
	vfmadd132sd	-80(%rcx,%rdi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	vmovsd	192000(%r8), %xmm1
	vfmadd132sd	-72(%rcx,%rdi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rsi,%rdx,8)
	vmovsd	224000(%r8), %xmm0
	vfmadd132sd	-64(%rcx,%rdi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	vmovsd	256000(%r8), %xmm1
	vfmadd132sd	-56(%rcx,%rdi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rsi,%rdx,8)
	vmovsd	288000(%r8), %xmm0
	vfmadd132sd	-48(%rcx,%rdi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	vmovsd	320000(%r8), %xmm1
	vfmadd132sd	-40(%rcx,%rdi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rsi,%rdx,8)
	vmovsd	352000(%r8), %xmm0
	vfmadd132sd	-32(%rcx,%rdi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	vmovsd	384000(%r8), %xmm1
	vfmadd132sd	-24(%rcx,%rdi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rsi,%rdx,8)
	vmovsd	416000(%r8), %xmm0
	vfmadd132sd	-16(%rcx,%rdi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	vmovsd	448000(%r8), %xmm1
	vfmadd132sd	-8(%rcx,%rdi,8), %xmm0, %xmm1
	vmovsd	%xmm1, (%rsi,%rdx,8)
	vmovsd	480000(%r8), %xmm0
	vfmadd132sd	(%rcx,%rdi,8), %xmm1, %xmm0
	vmovsd	%xmm0, (%rsi,%rdx,8)
	addq	$16, %rdi
	addq	$512000, %r8
	cmpq	$4015, %rdi
	jne	-355
	incq	%rdx
	addq	$8, %rax
	cmpq	$4000, %rdx
	jne	-391
	retq
