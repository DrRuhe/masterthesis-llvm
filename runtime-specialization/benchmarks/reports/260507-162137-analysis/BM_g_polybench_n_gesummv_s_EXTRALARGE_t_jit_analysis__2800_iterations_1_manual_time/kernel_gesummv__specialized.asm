	leaq	-7(%rip), %rax
	movabsq	$-7968, %r9
	addq	%rax, %r9
	movabsq	$104, %rax
	movq	(%r9,%rax), %rax
	movq	(%rax), %rax
	movabsq	$200, %rcx
	movq	(%r9,%rcx), %rcx
	movq	(%rcx), %rcx
	xorl	%edx, %edx
	movabsq	$160, %rsi
	movq	(%r9,%rsi), %rsi
	movabsq	$112, %rdi
	movq	(%r9,%rdi), %rdi
	movabsq	$136, %r8
	movq	(%r9,%r8), %r8
	movabsq	$-1584, %r10
	vmovsd	(%r9,%r10), %xmm0
	movabsq	$-1576, %r10
	vmovsd	(%r9,%r10), %xmm1
	nopw	%cs:(%rax,%rax)
	movq	$0, (%rsi,%rdx,8)
	movq	$0, (%rdi,%rdx,8)
	vxorpd	%xmm2, %xmm2, %xmm2
	movl	$9, %r9d
	vxorpd	%xmm3, %xmm3, %xmm3
	nop
	vmovsd	-72(%r8,%r9,8), %xmm4
	vmovsd	-64(%r8,%r9,8), %xmm5
	vfmadd231sd	-72(%rax,%r9,8), %xmm4, %xmm3
	vmovsd	%xmm3, (%rsi,%rdx,8)
	vfmadd231sd	-72(%rcx,%r9,8), %xmm4, %xmm2
	vmovsd	%xmm2, (%rdi,%rdx,8)
	vfmadd231sd	-64(%rax,%r9,8), %xmm5, %xmm3
	vmovsd	%xmm3, (%rsi,%rdx,8)
	vfmadd231sd	-64(%rcx,%r9,8), %xmm5, %xmm2
	vmovsd	%xmm2, (%rdi,%rdx,8)
	vmovsd	-56(%r8,%r9,8), %xmm4
	vfmadd231sd	-56(%rax,%r9,8), %xmm4, %xmm3
	vmovsd	%xmm3, (%rsi,%rdx,8)
	vfmadd231sd	-56(%rcx,%r9,8), %xmm4, %xmm2
	vmovsd	%xmm2, (%rdi,%rdx,8)
	vmovsd	-48(%r8,%r9,8), %xmm4
	vfmadd231sd	-48(%rax,%r9,8), %xmm4, %xmm3
	vmovsd	%xmm3, (%rsi,%rdx,8)
	vfmadd231sd	-48(%rcx,%r9,8), %xmm4, %xmm2
	vmovsd	%xmm2, (%rdi,%rdx,8)
	vmovsd	-40(%r8,%r9,8), %xmm4
	vfmadd231sd	-40(%rax,%r9,8), %xmm4, %xmm3
	vmovsd	%xmm3, (%rsi,%rdx,8)
	vfmadd231sd	-40(%rcx,%r9,8), %xmm4, %xmm2
	vmovsd	%xmm2, (%rdi,%rdx,8)
	vmovsd	-32(%r8,%r9,8), %xmm4
	vfmadd231sd	-32(%rax,%r9,8), %xmm4, %xmm3
	vmovsd	%xmm3, (%rsi,%rdx,8)
	vfmadd231sd	-32(%rcx,%r9,8), %xmm4, %xmm2
	vmovsd	%xmm2, (%rdi,%rdx,8)
	vmovsd	-24(%r8,%r9,8), %xmm4
	vfmadd231sd	-24(%rax,%r9,8), %xmm4, %xmm3
	vmovsd	%xmm3, (%rsi,%rdx,8)
	vfmadd231sd	-24(%rcx,%r9,8), %xmm4, %xmm2
	vmovsd	%xmm2, (%rdi,%rdx,8)
	vmovsd	-16(%r8,%r9,8), %xmm4
	vfmadd231sd	-16(%rax,%r9,8), %xmm4, %xmm3
	vmovsd	%xmm3, (%rsi,%rdx,8)
	vfmadd231sd	-16(%rcx,%r9,8), %xmm4, %xmm2
	vmovsd	%xmm2, (%rdi,%rdx,8)
	vmovsd	-8(%r8,%r9,8), %xmm4
	vfmadd231sd	-8(%rax,%r9,8), %xmm4, %xmm3
	vmovsd	%xmm3, (%rsi,%rdx,8)
	vfmadd231sd	-8(%rcx,%r9,8), %xmm4, %xmm2
	vmovsd	%xmm2, (%rdi,%rdx,8)
	vmovsd	(%r8,%r9,8), %xmm4
	vfmadd231sd	(%rax,%r9,8), %xmm4, %xmm3
	vmovsd	%xmm3, (%rsi,%rdx,8)
	vfmadd231sd	(%rcx,%r9,8), %xmm4, %xmm2
	vmovsd	%xmm2, (%rdi,%rdx,8)
	addq	$10, %r9
	cmpq	$2809, %r9
	jne	-324
	vmulsd	%xmm0, %xmm2, %xmm2
	vfmadd231sd	%xmm1, %xmm3, %xmm2
	vmovsd	%xmm2, (%rdi,%rdx,8)
	incq	%rdx
	addq	$22400, %rax
	addq	$22400, %rcx
	cmpq	$2800, %rdx
	jne	-399
	retq
