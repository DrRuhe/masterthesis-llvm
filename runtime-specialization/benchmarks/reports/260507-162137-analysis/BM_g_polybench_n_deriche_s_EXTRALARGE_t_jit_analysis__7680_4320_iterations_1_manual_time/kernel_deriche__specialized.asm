	leaq	-7(%rip), %rax
	movabsq	$-7952, %r8
	addq	%rax, %r8
	movabsq	$160, %rax
	movq	(%r8,%rax), %rax
	movq	(%rax), %rsi
	movabsq	$96, %rax
	movq	(%r8,%rax), %rax
	movq	(%rax), %rax
	movabsq	$64, %rcx
	movq	(%r8,%rcx), %rcx
	movq	(%rcx), %rcx
	movabsq	$72, %rdx
	movq	(%r8,%rdx), %rdx
	movq	(%rdx), %rdx
	leaq	44(%rsi), %rdi
	leaq	44(%rcx), %r9
	xorl	%r10d, %r10d
	movabsq	$-1604, %r11
	vmovss	(%r8,%r11), %xmm4
	movabsq	$-1600, %r11
	vmovss	(%r8,%r11), %xmm5
	movabsq	$-1596, %r11
	vmovss	(%r8,%r11), %xmm0
	movabsq	$-1592, %r11
	vmovss	(%r8,%r11), %xmm1
	nopw	%cs:(%rax,%rax)
	vxorps	%xmm6, %xmm6, %xmm6
	xorl	%r11d, %r11d
	vxorps	%xmm2, %xmm2, %xmm2
	vxorps	%xmm3, %xmm3, %xmm3
	nop
	vmulss	%xmm4, %xmm6, %xmm6
	vfmadd231ss	-44(%rdi,%r11,4), %xmm5, %xmm6
	vfmadd231ss	%xmm0, %xmm2, %xmm6
	vfmadd231ss	%xmm3, %xmm1, %xmm6
	vmovss	%xmm6, -44(%r9,%r11,4)
	vmulss	-44(%rdi,%r11,4), %xmm4, %xmm3
	vfmadd231ss	-40(%rdi,%r11,4), %xmm5, %xmm3
	vfmadd231ss	%xmm0, %xmm6, %xmm3
	vfmadd231ss	%xmm2, %xmm1, %xmm3
	vmovss	%xmm3, -40(%r9,%r11,4)
	vmulss	-40(%rdi,%r11,4), %xmm4, %xmm2
	vfmadd231ss	-36(%rdi,%r11,4), %xmm5, %xmm2
	vfmadd231ss	%xmm0, %xmm3, %xmm2
	vfmadd231ss	%xmm6, %xmm1, %xmm2
	vmovss	%xmm2, -36(%r9,%r11,4)
	vmulss	-36(%rdi,%r11,4), %xmm4, %xmm6
	vfmadd231ss	-32(%rdi,%r11,4), %xmm5, %xmm6
	vfmadd231ss	%xmm0, %xmm2, %xmm6
	vfmadd231ss	%xmm3, %xmm1, %xmm6
	vmovss	%xmm6, -32(%r9,%r11,4)
	vmulss	-32(%rdi,%r11,4), %xmm4, %xmm3
	vfmadd231ss	-28(%rdi,%r11,4), %xmm5, %xmm3
	vfmadd231ss	%xmm0, %xmm6, %xmm3
	vfmadd231ss	%xmm2, %xmm1, %xmm3
	vmovss	%xmm3, -28(%r9,%r11,4)
	vmulss	-28(%rdi,%r11,4), %xmm4, %xmm2
	vfmadd231ss	-24(%rdi,%r11,4), %xmm5, %xmm2
	vfmadd231ss	%xmm0, %xmm3, %xmm2
	vfmadd231ss	%xmm6, %xmm1, %xmm2
	vmovss	%xmm2, -24(%r9,%r11,4)
	vmulss	-24(%rdi,%r11,4), %xmm4, %xmm6
	vfmadd231ss	-20(%rdi,%r11,4), %xmm5, %xmm6
	vfmadd231ss	%xmm0, %xmm2, %xmm6
	vfmadd231ss	%xmm3, %xmm1, %xmm6
	vmovss	%xmm6, -20(%r9,%r11,4)
	vmulss	-20(%rdi,%r11,4), %xmm4, %xmm3
	vfmadd231ss	-16(%rdi,%r11,4), %xmm5, %xmm3
	vfmadd231ss	%xmm0, %xmm6, %xmm3
	vfmadd231ss	%xmm2, %xmm1, %xmm3
	vmovss	%xmm3, -16(%r9,%r11,4)
	vmulss	-16(%rdi,%r11,4), %xmm4, %xmm2
	vfmadd231ss	-12(%rdi,%r11,4), %xmm5, %xmm2
	vfmadd231ss	%xmm0, %xmm3, %xmm2
	vfmadd231ss	%xmm6, %xmm1, %xmm2
	vmovss	%xmm2, -12(%r9,%r11,4)
	vmulss	-12(%rdi,%r11,4), %xmm4, %xmm6
	vfmadd231ss	-8(%rdi,%r11,4), %xmm5, %xmm6
	vfmadd231ss	%xmm0, %xmm2, %xmm6
	vfmadd231ss	%xmm3, %xmm1, %xmm6
	vmovss	%xmm6, -8(%r9,%r11,4)
	vmulss	-8(%rdi,%r11,4), %xmm4, %xmm3
	vfmadd231ss	-4(%rdi,%r11,4), %xmm5, %xmm3
	vfmadd231ss	%xmm0, %xmm6, %xmm3
	vfmadd231ss	%xmm2, %xmm1, %xmm3
	vmovss	%xmm3, -4(%r9,%r11,4)
	vmulss	-4(%rdi,%r11,4), %xmm4, %xmm2
	vfmadd231ss	(%rdi,%r11,4), %xmm5, %xmm2
	vfmadd231ss	%xmm0, %xmm3, %xmm2
	vfmadd231ss	%xmm6, %xmm1, %xmm2
	vmovss	%xmm2, (%r9,%r11,4)
	vmovss	(%rdi,%r11,4), %xmm6
	addq	$12, %r11
	cmpq	$4320, %r11
	jne	-390
	incq	%r10
	addq	$17280, %rdi
	addq	$17280, %r9
	cmpq	$7680, %r10
	jne	-436
	xorl	%edi, %edi
	movabsq	$-1588, %r9
	vmovss	(%r8,%r9), %xmm2
	movabsq	$-1584, %r9
	vmovss	(%r8,%r9), %xmm3
	movq	%rdx, %r8
	nopl	(%rax)
	vxorps	%xmm8, %xmm8, %xmm8
	movl	$4320, %r9d
	vxorps	%xmm9, %xmm9, %xmm9
	vxorps	%xmm7, %xmm7, %xmm7
	vxorps	%xmm6, %xmm6, %xmm6
	movl	$4320, %r10d
	nop
	vmulss	%xmm2, %xmm9, %xmm9
	vfmadd231ss	%xmm3, %xmm8, %xmm9
	vfmadd231ss	%xmm0, %xmm7, %xmm9
	vfmadd231ss	%xmm6, %xmm1, %xmm9
	vmovss	%xmm9, -4(%r8,%r9,4)
	vmovss	-4(%rsi,%r9,4), %xmm6
	vmulss	%xmm2, %xmm8, %xmm8
	vfmadd231ss	%xmm3, %xmm6, %xmm8
	vfmadd231ss	%xmm0, %xmm9, %xmm8
	vfmadd231ss	%xmm7, %xmm1, %xmm8
	vmovss	%xmm8, -8(%r8,%r9,4)
	vmovss	-8(%rsi,%r9,4), %xmm7
	vmulss	%xmm2, %xmm6, %xmm6
	vfmadd231ss	%xmm3, %xmm7, %xmm6
	vfmadd231ss	%xmm0, %xmm8, %xmm6
	vfmadd231ss	%xmm9, %xmm1, %xmm6
	vmovss	%xmm6, -12(%r8,%r9,4)
	vmovss	-12(%rsi,%r9,4), %xmm9
	vmulss	%xmm2, %xmm7, %xmm7
	vfmadd231ss	%xmm3, %xmm9, %xmm7
	vfmadd231ss	%xmm0, %xmm6, %xmm7
	vfmadd231ss	%xmm8, %xmm1, %xmm7
	vmovss	%xmm7, -16(%r8,%r9,4)
	vmovss	-16(%rsi,%r9,4), %xmm8
	vmulss	%xmm2, %xmm9, %xmm9
	vfmadd231ss	%xmm3, %xmm8, %xmm9
	vfmadd231ss	%xmm0, %xmm7, %xmm9
	vfmadd231ss	%xmm6, %xmm1, %xmm9
	vmovss	%xmm9, -20(%r8,%r9,4)
	vmovss	-20(%rsi,%r9,4), %xmm6
	vmulss	%xmm2, %xmm8, %xmm8
	vfmadd231ss	%xmm3, %xmm6, %xmm8
	vfmadd231ss	%xmm0, %xmm9, %xmm8
	vfmadd231ss	%xmm7, %xmm1, %xmm8
	vmovss	%xmm8, -24(%r8,%r9,4)
	vmovss	-24(%rsi,%r9,4), %xmm7
	vmulss	%xmm2, %xmm6, %xmm6
	vfmadd231ss	%xmm3, %xmm7, %xmm6
	vfmadd231ss	%xmm0, %xmm8, %xmm6
	vfmadd231ss	%xmm9, %xmm1, %xmm6
	vmovss	%xmm6, -28(%r8,%r9,4)
	vmovss	-28(%rsi,%r9,4), %xmm9
	vmulss	%xmm2, %xmm7, %xmm7
	vfmadd231ss	%xmm3, %xmm9, %xmm7
	vfmadd231ss	%xmm0, %xmm6, %xmm7
	vfmadd231ss	%xmm8, %xmm1, %xmm7
	vmovss	%xmm7, -32(%r8,%r9,4)
	vmovss	-32(%rsi,%r9,4), %xmm8
	vmulss	%xmm2, %xmm9, %xmm9
	vfmadd231ss	%xmm3, %xmm8, %xmm9
	vfmadd231ss	%xmm0, %xmm7, %xmm9
	vfmadd231ss	%xmm6, %xmm1, %xmm9
	vmovss	%xmm9, -36(%r8,%r9,4)
	vmovss	-36(%rsi,%r9,4), %xmm6
	vmulss	%xmm2, %xmm8, %xmm8
	vfmadd231ss	%xmm3, %xmm6, %xmm8
	vfmadd231ss	%xmm0, %xmm9, %xmm8
	vfmadd231ss	%xmm7, %xmm1, %xmm8
	vmovss	%xmm8, -40(%r8,%r9,4)
	vmovss	-40(%rsi,%r9,4), %xmm7
	vmulss	%xmm2, %xmm6, %xmm6
	vfmadd231ss	%xmm3, %xmm7, %xmm6
	vfmadd231ss	%xmm0, %xmm8, %xmm6
	vfmadd231ss	%xmm9, %xmm1, %xmm6
	vmovss	%xmm6, -44(%r8,%r9,4)
	vmovss	-44(%rsi,%r9,4), %xmm9
	vmulss	%xmm2, %xmm7, %xmm7
	vfmadd231ss	%xmm3, %xmm9, %xmm7
	vfmadd231ss	%xmm0, %xmm6, %xmm7
	vfmadd231ss	%xmm8, %xmm1, %xmm7
	vmovss	%xmm7, -48(%r8,%r9,4)
	vmovss	-48(%rsi,%r9,4), %xmm8
	vmulss	%xmm2, %xmm9, %xmm9
	vfmadd231ss	%xmm3, %xmm8, %xmm9
	vfmadd231ss	%xmm0, %xmm7, %xmm9
	vfmadd231ss	%xmm6, %xmm1, %xmm9
	vmovss	%xmm9, -52(%r8,%r9,4)
	vmovss	-52(%rsi,%r9,4), %xmm6
	vmulss	%xmm2, %xmm8, %xmm8
	vfmadd231ss	%xmm3, %xmm6, %xmm8
	vfmadd231ss	%xmm0, %xmm9, %xmm8
	vfmadd231ss	%xmm7, %xmm1, %xmm8
	vmovss	%xmm8, -56(%r8,%r9,4)
	vmovss	-56(%rsi,%r9,4), %xmm7
	vmulss	%xmm2, %xmm6, %xmm6
	vfmadd231ss	%xmm3, %xmm7, %xmm6
	vfmadd231ss	%xmm0, %xmm8, %xmm6
	vfmadd231ss	%xmm9, %xmm1, %xmm6
	vmovss	%xmm6, -60(%r8,%r9,4)
	vmovss	-60(%rsi,%r9,4), %xmm9
	vmulss	%xmm2, %xmm7, %xmm7
	vfmadd231ss	%xmm3, %xmm9, %xmm7
	vfmadd231ss	%xmm0, %xmm6, %xmm7
	vfmadd231ss	%xmm8, %xmm1, %xmm7
	vmovss	%xmm7, -64(%r8,%r9,4)
	vmovss	-64(%rsi,%r9,4), %xmm8
	addq	$-16, %r10
	movq	%r10, %r9
	jne	-541
	incq	%rdi
	addq	$17280, %rsi
	addq	$17280, %r8
	cmpq	$7680, %rdi
	jne	-603
	movq	%rax, %rsi
	subq	%rcx, %rsi
	movq	%rax, %rdi
	subq	%rdx, %rdi
	cmpq	$32, %rsi
	setb	%r8b
	cmpq	$32, %rdi
	setb	%sil
	xorl	%edi, %edi
	orb	%r8b, %sil
	je	464
	movq	%rcx, %r8
	movq	%rdx, %r9
	movq	%rax, %r10
	nopl	(%rax,%rax)
	movl	$17, %r11d
	nopw	%cs:(%rax,%rax)
	vmovss	-68(%r8,%r11,4), %xmm6
	vaddss	-68(%r9,%r11,4), %xmm6, %xmm6
	vmovss	%xmm6, -68(%r10,%r11,4)
	vmovss	-64(%r8,%r11,4), %xmm6
	vaddss	-64(%r9,%r11,4), %xmm6, %xmm6
	vmovss	%xmm6, -64(%r10,%r11,4)
	vmovss	-60(%r8,%r11,4), %xmm6
	vaddss	-60(%r9,%r11,4), %xmm6, %xmm6
	vmovss	%xmm6, -60(%r10,%r11,4)
	vmovss	-56(%r8,%r11,4), %xmm6
	vaddss	-56(%r9,%r11,4), %xmm6, %xmm6
	vmovss	%xmm6, -56(%r10,%r11,4)
	vmovss	-52(%r8,%r11,4), %xmm6
	vaddss	-52(%r9,%r11,4), %xmm6, %xmm6
	vmovss	%xmm6, -52(%r10,%r11,4)
	vmovss	-48(%r8,%r11,4), %xmm6
	vaddss	-48(%r9,%r11,4), %xmm6, %xmm6
	vmovss	%xmm6, -48(%r10,%r11,4)
	vmovss	-44(%r8,%r11,4), %xmm6
	vaddss	-44(%r9,%r11,4), %xmm6, %xmm6
	vmovss	%xmm6, -44(%r10,%r11,4)
	vmovss	-40(%r8,%r11,4), %xmm6
	vaddss	-40(%r9,%r11,4), %xmm6, %xmm6
	vmovss	%xmm6, -40(%r10,%r11,4)
	vmovss	-36(%r8,%r11,4), %xmm6
	vaddss	-36(%r9,%r11,4), %xmm6, %xmm6
	vmovss	%xmm6, -36(%r10,%r11,4)
	vmovss	-32(%r8,%r11,4), %xmm6
	vaddss	-32(%r9,%r11,4), %xmm6, %xmm6
	vmovss	%xmm6, -32(%r10,%r11,4)
	vmovss	-28(%r8,%r11,4), %xmm6
	vaddss	-28(%r9,%r11,4), %xmm6, %xmm6
	vmovss	%xmm6, -28(%r10,%r11,4)
	vmovss	-24(%r8,%r11,4), %xmm6
	vaddss	-24(%r9,%r11,4), %xmm6, %xmm6
	vmovss	%xmm6, -24(%r10,%r11,4)
	vmovss	-20(%r8,%r11,4), %xmm6
	vaddss	-20(%r9,%r11,4), %xmm6, %xmm6
	vmovss	%xmm6, -20(%r10,%r11,4)
	vmovss	-16(%r8,%r11,4), %xmm6
	vaddss	-16(%r9,%r11,4), %xmm6, %xmm6
	vmovss	%xmm6, -16(%r10,%r11,4)
	vmovss	-12(%r8,%r11,4), %xmm6
	vaddss	-12(%r9,%r11,4), %xmm6, %xmm6
	vmovss	%xmm6, -12(%r10,%r11,4)
	vmovss	-8(%r8,%r11,4), %xmm6
	vaddss	-8(%r9,%r11,4), %xmm6, %xmm6
	vmovss	%xmm6, -8(%r10,%r11,4)
	vmovss	-4(%r8,%r11,4), %xmm6
	vaddss	-4(%r9,%r11,4), %xmm6, %xmm6
	vmovss	%xmm6, -4(%r10,%r11,4)
	vmovss	(%r8,%r11,4), %xmm6
	vaddss	(%r9,%r11,4), %xmm6, %xmm6
	vmovss	%xmm6, (%r10,%r11,4)
	addq	$18, %r11
	cmpq	$4337, %r11
	jne	-392
	incq	%rdi
	addq	$17280, %r10
	addq	$17280, %r9
	addq	$17280, %r8
	cmpq	$7680, %rdi
	jne	-445
	jmp	540
	movq	%rcx, %r8
	movq	%rdx, %r9
	movq	%rax, %r10
	nopl	(%rax,%rax)
	movl	$68, %r11d
	nopw	%cs:(%rax,%rax)
	vmovups	-272(%r8,%r11,4), %xmm6
	vmovups	-256(%r8,%r11,4), %xmm7
	vaddps	-272(%r9,%r11,4), %xmm6, %xmm6
	vaddps	-256(%r9,%r11,4), %xmm7, %xmm7
	vmovups	%xmm6, -272(%r10,%r11,4)
	vmovups	%xmm7, -256(%r10,%r11,4)
	vmovups	-240(%r8,%r11,4), %xmm6
	vmovups	-224(%r8,%r11,4), %xmm7
	vaddps	-240(%r9,%r11,4), %xmm6, %xmm6
	vaddps	-224(%r9,%r11,4), %xmm7, %xmm7
	vmovups	%xmm6, -240(%r10,%r11,4)
	vmovups	%xmm7, -224(%r10,%r11,4)
	vmovups	-208(%r8,%r11,4), %xmm6
	vmovups	-192(%r8,%r11,4), %xmm7
	vaddps	-208(%r9,%r11,4), %xmm6, %xmm6
	vaddps	-192(%r9,%r11,4), %xmm7, %xmm7
	vmovups	%xmm6, -208(%r10,%r11,4)
	vmovups	%xmm7, -192(%r10,%r11,4)
	vmovups	-176(%r8,%r11,4), %xmm6
	vmovups	-160(%r8,%r11,4), %xmm7
	vaddps	-176(%r9,%r11,4), %xmm6, %xmm6
	vaddps	-160(%r9,%r11,4), %xmm7, %xmm7
	vmovups	%xmm6, -176(%r10,%r11,4)
	vmovups	%xmm7, -160(%r10,%r11,4)
	vmovups	-144(%r8,%r11,4), %xmm6
	vmovups	-128(%r8,%r11,4), %xmm7
	vaddps	-144(%r9,%r11,4), %xmm6, %xmm6
	vaddps	-128(%r9,%r11,4), %xmm7, %xmm7
	vmovups	%xmm6, -144(%r10,%r11,4)
	vmovups	%xmm7, -128(%r10,%r11,4)
	vmovups	-112(%r8,%r11,4), %xmm6
	vmovups	-96(%r8,%r11,4), %xmm7
	vaddps	-112(%r9,%r11,4), %xmm6, %xmm6
	vaddps	-96(%r9,%r11,4), %xmm7, %xmm7
	vmovups	%xmm6, -112(%r10,%r11,4)
	vmovups	%xmm7, -96(%r10,%r11,4)
	vmovups	-80(%r8,%r11,4), %xmm6
	vmovups	-64(%r8,%r11,4), %xmm7
	vaddps	-80(%r9,%r11,4), %xmm6, %xmm6
	vaddps	-64(%r9,%r11,4), %xmm7, %xmm7
	vmovups	%xmm6, -80(%r10,%r11,4)
	vmovups	%xmm7, -64(%r10,%r11,4)
	vmovups	-48(%r8,%r11,4), %xmm6
	vmovups	-32(%r8,%r11,4), %xmm7
	vaddps	-48(%r9,%r11,4), %xmm6, %xmm6
	vaddps	-32(%r9,%r11,4), %xmm7, %xmm7
	vmovups	%xmm6, -48(%r10,%r11,4)
	vmovups	%xmm7, -32(%r10,%r11,4)
	vmovups	-16(%r8,%r11,4), %xmm6
	vmovups	(%r8,%r11,4), %xmm7
	vaddps	-16(%r9,%r11,4), %xmm6, %xmm6
	vaddps	(%r9,%r11,4), %xmm7, %xmm7
	vmovups	%xmm6, -16(%r10,%r11,4)
	vmovups	%xmm7, (%r10,%r11,4)
	addq	$72, %r11
	cmpq	$4388, %r11
	jne	-473
	incq	%rdi
	addq	$17280, %r10
	addq	$17280, %r9
	addq	$17280, %r8
	cmpq	$7680, %rdi
	jne	-526
	xorl	%edi, %edi
	movq	%rcx, %r8
	movq	%rax, %r9
	nopw	%cs:(%rax,%rax)
	vxorps	%xmm8, %xmm8, %xmm8
	xorl	%r10d, %r10d
	vxorps	%xmm6, %xmm6, %xmm6
	vxorps	%xmm7, %xmm7, %xmm7
	vmulss	%xmm4, %xmm8, %xmm8
	vfmadd231ss	(%r9,%r10), %xmm5, %xmm8
	vfmadd231ss	%xmm0, %xmm6, %xmm8
	vfmadd231ss	%xmm7, %xmm1, %xmm8
	vmovss	%xmm8, (%r8,%r10)
	vmulss	(%r9,%r10), %xmm4, %xmm7
	vfmadd231ss	17280(%r9,%r10), %xmm5, %xmm7
	vfmadd231ss	%xmm0, %xmm8, %xmm7
	vfmadd231ss	%xmm6, %xmm1, %xmm7
	vmovss	%xmm7, 17280(%r8,%r10)
	vmulss	17280(%r9,%r10), %xmm4, %xmm6
	vfmadd231ss	34560(%r9,%r10), %xmm5, %xmm6
	vfmadd231ss	%xmm0, %xmm7, %xmm6
	vfmadd231ss	%xmm8, %xmm1, %xmm6
	vmovss	%xmm6, 34560(%r8,%r10)
	vmulss	34560(%r9,%r10), %xmm4, %xmm8
	vfmadd231ss	51840(%r9,%r10), %xmm5, %xmm8
	vfmadd231ss	%xmm0, %xmm6, %xmm8
	vfmadd231ss	%xmm7, %xmm1, %xmm8
	vmovss	%xmm8, 51840(%r8,%r10)
	vmulss	51840(%r9,%r10), %xmm4, %xmm7
	vfmadd231ss	69120(%r9,%r10), %xmm5, %xmm7
	vfmadd231ss	%xmm0, %xmm8, %xmm7
	vfmadd231ss	%xmm6, %xmm1, %xmm7
	vmovss	%xmm7, 69120(%r8,%r10)
	vmulss	69120(%r9,%r10), %xmm4, %xmm6
	vfmadd231ss	86400(%r9,%r10), %xmm5, %xmm6
	vfmadd231ss	%xmm0, %xmm7, %xmm6
	vfmadd231ss	%xmm8, %xmm1, %xmm6
	vmovss	%xmm6, 86400(%r8,%r10)
	vmulss	86400(%r9,%r10), %xmm4, %xmm8
	vfmadd231ss	103680(%r9,%r10), %xmm5, %xmm8
	vfmadd231ss	%xmm0, %xmm6, %xmm8
	vfmadd231ss	%xmm7, %xmm1, %xmm8
	vmovss	%xmm8, 103680(%r8,%r10)
	vmulss	103680(%r9,%r10), %xmm4, %xmm7
	vfmadd231ss	120960(%r9,%r10), %xmm5, %xmm7
	vfmadd231ss	%xmm0, %xmm8, %xmm7
	vfmadd231ss	%xmm6, %xmm1, %xmm7
	vmovss	%xmm7, 120960(%r8,%r10)
	vmulss	120960(%r9,%r10), %xmm4, %xmm6
	vfmadd231ss	138240(%r9,%r10), %xmm5, %xmm6
	vfmadd231ss	%xmm0, %xmm7, %xmm6
	vfmadd231ss	%xmm8, %xmm1, %xmm6
	vmovss	%xmm6, 138240(%r8,%r10)
	vmulss	138240(%r9,%r10), %xmm4, %xmm8
	vfmadd231ss	155520(%r9,%r10), %xmm5, %xmm8
	vfmadd231ss	%xmm0, %xmm6, %xmm8
	vfmadd231ss	%xmm7, %xmm1, %xmm8
	vmovss	%xmm8, 155520(%r8,%r10)
	vmulss	155520(%r9,%r10), %xmm4, %xmm7
	vfmadd231ss	172800(%r9,%r10), %xmm5, %xmm7
	vfmadd231ss	%xmm0, %xmm8, %xmm7
	vfmadd231ss	%xmm6, %xmm1, %xmm7
	vmovss	%xmm7, 172800(%r8,%r10)
	vmulss	172800(%r9,%r10), %xmm4, %xmm6
	vfmadd231ss	190080(%r9,%r10), %xmm5, %xmm6
	vfmadd231ss	%xmm0, %xmm7, %xmm6
	vfmadd231ss	%xmm8, %xmm1, %xmm6
	vmovss	%xmm6, 190080(%r8,%r10)
	vmovss	190080(%r9,%r10), %xmm8
	addq	$207360, %r10
	cmpq	$132710400, %r10
	jne	-492
	incq	%rdi
	addq	$4, %r9
	addq	$4, %r8
	cmpq	$4320, %rdi
	jne	-532
	xorl	%edi, %edi
	movq	%rdx, %r8
	movq	%rax, %r9
	nopl	(%rax)
	vxorps	%xmm6, %xmm6, %xmm6
	movl	$132433920, %r10d
	vxorps	%xmm7, %xmm7, %xmm7
	vxorps	%xmm4, %xmm4, %xmm4
	vxorps	%xmm5, %xmm5, %xmm5
	nopw	%cs:(%rax,%rax)
	vmulss	%xmm2, %xmm7, %xmm7
	vfmadd231ss	%xmm3, %xmm6, %xmm7
	vfmadd231ss	%xmm0, %xmm4, %xmm7
	vfmadd231ss	%xmm5, %xmm1, %xmm7
	vmovss	%xmm7, 259200(%r8,%r10)
	vmovss	259200(%r9,%r10), %xmm5
	vmulss	%xmm2, %xmm6, %xmm6
	vfmadd231ss	%xmm3, %xmm5, %xmm6
	vfmadd231ss	%xmm0, %xmm7, %xmm6
	vfmadd231ss	%xmm4, %xmm1, %xmm6
	vmovss	%xmm6, 241920(%r8,%r10)
	vmovss	241920(%r9,%r10), %xmm4
	vmulss	%xmm2, %xmm5, %xmm5
	vfmadd231ss	%xmm3, %xmm4, %xmm5
	vfmadd231ss	%xmm0, %xmm6, %xmm5
	vfmadd231ss	%xmm7, %xmm1, %xmm5
	vmovss	%xmm5, 224640(%r8,%r10)
	vmovss	224640(%r9,%r10), %xmm7
	vmulss	%xmm2, %xmm4, %xmm4
	vfmadd231ss	%xmm3, %xmm7, %xmm4
	vfmadd231ss	%xmm0, %xmm5, %xmm4
	vfmadd231ss	%xmm6, %xmm1, %xmm4
	vmovss	%xmm4, 207360(%r8,%r10)
	vmovss	207360(%r9,%r10), %xmm6
	vmulss	%xmm2, %xmm7, %xmm7
	vfmadd231ss	%xmm3, %xmm6, %xmm7
	vfmadd231ss	%xmm0, %xmm4, %xmm7
	vfmadd231ss	%xmm5, %xmm1, %xmm7
	vmovss	%xmm7, 190080(%r8,%r10)
	vmovss	190080(%r9,%r10), %xmm5
	vmulss	%xmm2, %xmm6, %xmm6
	vfmadd231ss	%xmm3, %xmm5, %xmm6
	vfmadd231ss	%xmm0, %xmm7, %xmm6
	vfmadd231ss	%xmm4, %xmm1, %xmm6
	vmovss	%xmm6, 172800(%r8,%r10)
	vmovss	172800(%r9,%r10), %xmm4
	vmulss	%xmm2, %xmm5, %xmm5
	vfmadd231ss	%xmm3, %xmm4, %xmm5
	vfmadd231ss	%xmm0, %xmm6, %xmm5
	vfmadd231ss	%xmm7, %xmm1, %xmm5
	vmovss	%xmm5, 155520(%r8,%r10)
	vmovss	155520(%r9,%r10), %xmm7
	vmulss	%xmm2, %xmm4, %xmm4
	vfmadd231ss	%xmm3, %xmm7, %xmm4
	vfmadd231ss	%xmm0, %xmm5, %xmm4
	vfmadd231ss	%xmm6, %xmm1, %xmm4
	vmovss	%xmm4, 138240(%r8,%r10)
	vmovss	138240(%r9,%r10), %xmm6
	vmulss	%xmm2, %xmm7, %xmm7
	vfmadd231ss	%xmm3, %xmm6, %xmm7
	vfmadd231ss	%xmm0, %xmm4, %xmm7
	vfmadd231ss	%xmm5, %xmm1, %xmm7
	vmovss	%xmm7, 120960(%r8,%r10)
	vmovss	120960(%r9,%r10), %xmm5
	vmulss	%xmm2, %xmm6, %xmm6
	vfmadd231ss	%xmm3, %xmm5, %xmm6
