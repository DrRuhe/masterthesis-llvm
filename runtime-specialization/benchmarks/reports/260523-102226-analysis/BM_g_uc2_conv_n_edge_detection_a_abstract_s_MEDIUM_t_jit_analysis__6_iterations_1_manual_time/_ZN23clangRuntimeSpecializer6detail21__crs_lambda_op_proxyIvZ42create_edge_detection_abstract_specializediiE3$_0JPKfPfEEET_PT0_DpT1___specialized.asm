<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ42create_edge_detection_abstract_specializediiE3$_0JPKfPfEEET_PT0_DpT1_>:
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	leaq	-7(%rip), %rax
	movabsq	$-9650, %r10
	addq	%rax, %r10
	leaq	16(%rdi), %rax
	leaq	12(%rsi), %rcx
	movl	$4294967295, %edx
	movl	$1, %r8d
	xorl	%r9d, %r9d
	vxorps	%xmm0, %xmm0, %xmm0
	movabsq	$-536, %r11
	vmovss	(%r10,%r11), %xmm1
	movabsq	$-532, %r11
	vmovss	(%r10,%r11), %xmm2
	movq	%rax, %r11
	jmp	213
	vmovss	-4(%r14,%r10), %xmm3
	vmovss	15356(%rdi,%r12,4), %xmm4
	vmovss	-4(%r11,%r10), %xmm5
	vmovss	15352(%rdi,%r13,4), %xmm6
	vmovss	15356(%rdi,%r13,4), %xmm7
	vmovss	-4(%r15,%r10), %xmm8
	vmovss	15352(%rdi,%rbp,4), %xmm9
	vmovss	15356(%rdi,%rbp,4), %xmm10
	vsubss	15352(%rdi,%r12,4), %xmm0, %xmm11
	vxorps	%xmm12, %xmm12, %xmm12
	vfmadd213ss	%xmm11, %xmm3, %xmm12
	vfmadd213ss	%xmm11, %xmm1, %xmm3
	vaddss	%xmm4, %xmm12, %xmm11
	vsubss	%xmm4, %xmm3, %xmm3
	vfmadd231ss	%xmm1, %xmm6, %xmm11
	vfmadd231ss	%xmm6, %xmm0, %xmm3
	vfmadd231ss	%xmm0, %xmm5, %xmm11
	vfmadd231ss	%xmm5, %xmm0, %xmm3
	vfmadd231ss	%xmm2, %xmm7, %xmm11
	vfmadd231ss	%xmm7, %xmm0, %xmm3
	vsubss	%xmm9, %xmm11, %xmm4
	vaddss	%xmm3, %xmm9, %xmm3
	vfmadd231ss	%xmm0, %xmm8, %xmm4
	vfmadd231ss	%xmm8, %xmm2, %xmm3
	vaddss	%xmm4, %xmm10, %xmm4
	vaddss	%xmm3, %xmm10, %xmm3
	vmulss	%xmm3, %xmm3, %xmm3
	vfmadd231ss	%xmm4, %xmm4, %xmm3
	vsqrtss	%xmm3, %xmm3, %xmm3
	vmovss	%xmm3, 15356(%rsi,%rbx,4)
	incl	%edx
	addq	$15360, %r11
	incq	%r8
	addq	$15360, %rcx
	cmpq	$3840, %r9
	je	803
	movl	%edx, %r10d
	sarl	$31, %r10d
	andnl	%edx, %r10d, %r10d
	imulq	$15360, %r10, %r14
	addq	%rax, %r14
	cmpq	$3839, %r8
	movl	$3839, %r10d
	cmovbq	%r8, %r10
	imulq	$15360, %r10, %r15
	addq	%rax, %r15
	imulq	$3840, %r9, %rbx
	leal	-1(%r9), %r10d
	movl	%r10d, %ebp
	sarl	$31, %ebp
	andnl	%r10d, %ebp, %r10d
	cmpl	$3839, %r10d
	movl	$3839, %ebp
	cmovgel	%ebp, %r10d
	imull	$3840, %r10d, %r12d
	movl	%r9d, %r10d
	sarl	$31, %r10d
	andnl	%r9d, %r10d, %r10d
	cmpl	$3839, %r10d
	cmovgel	%ebp, %r10d
	imull	$3840, %r10d, %r13d
	incq	%r9
	movl	%r9d, %r10d
	sarl	$31, %r10d
	andnl	%r9d, %r10d, %r10d
	cmpl	$3839, %r10d
	cmovgel	%ebp, %r10d
	imull	$3840, %r10d, %ebp
	vmovss	(%rdi,%r12,4), %xmm3
	vmovss	4(%rdi,%r12,4), %xmm4
	vmovss	(%rdi,%r13,4), %xmm5
	vmovss	4(%rdi,%r13,4), %xmm6
	vmovss	(%rdi,%rbp,4), %xmm7
	vmovss	4(%rdi,%rbp,4), %xmm8
	vsubss	%xmm3, %xmm0, %xmm9
	vxorps	%xmm10, %xmm10, %xmm10
	vfmadd213ss	%xmm9, %xmm3, %xmm10
	vfmadd213ss	%xmm9, %xmm1, %xmm3
	vaddss	%xmm4, %xmm10, %xmm9
	vsubss	%xmm4, %xmm3, %xmm3
	vfmadd231ss	%xmm1, %xmm5, %xmm9
	vfmadd231ss	%xmm0, %xmm5, %xmm3
	vfmadd231ss	%xmm0, %xmm5, %xmm9
	vfmadd231ss	%xmm5, %xmm0, %xmm3
	vfmadd231ss	%xmm2, %xmm6, %xmm9
	vfmadd231ss	%xmm6, %xmm0, %xmm3
	vsubss	%xmm7, %xmm9, %xmm4
	vaddss	%xmm3, %xmm7, %xmm3
	vfmadd231ss	%xmm0, %xmm7, %xmm4
	vfmadd231ss	%xmm7, %xmm2, %xmm3
	vaddss	%xmm4, %xmm8, %xmm4
	vaddss	%xmm3, %xmm8, %xmm3
	vmulss	%xmm3, %xmm3, %xmm3
	vfmadd231ss	%xmm4, %xmm4, %xmm3
	vsqrtss	%xmm3, %xmm3, %xmm3
	vmovss	%xmm3, (%rsi,%rbx,4)
	vmovss	4(%rdi,%r12,4), %xmm3
	vmovss	8(%rdi,%r12,4), %xmm4
	vmovss	(%rdi,%r13,4), %xmm5
	vmovss	4(%rdi,%r13,4), %xmm6
	vmovss	8(%rdi,%r13,4), %xmm7
	vmovss	(%rdi,%rbp,4), %xmm8
	vmovss	4(%rdi,%rbp,4), %xmm9
	vsubss	(%rdi,%r12,4), %xmm0, %xmm10
	vmovss	8(%rdi,%rbp,4), %xmm11
	vxorps	%xmm12, %xmm12, %xmm12
	vfmadd213ss	%xmm10, %xmm3, %xmm12
	vfmadd213ss	%xmm10, %xmm1, %xmm3
	vaddss	%xmm4, %xmm12, %xmm10
	vsubss	%xmm4, %xmm3, %xmm3
	vfmadd231ss	%xmm1, %xmm5, %xmm10
	vfmadd231ss	%xmm5, %xmm0, %xmm3
	vfmadd231ss	%xmm0, %xmm6, %xmm10
	vfmadd231ss	%xmm6, %xmm0, %xmm3
	vfmadd231ss	%xmm2, %xmm7, %xmm10
	vfmadd231ss	%xmm7, %xmm0, %xmm3
	vsubss	%xmm8, %xmm10, %xmm4
	vaddss	%xmm3, %xmm8, %xmm3
	vfmadd231ss	%xmm0, %xmm9, %xmm4
	vfmadd231ss	%xmm9, %xmm2, %xmm3
	vaddss	%xmm4, %xmm11, %xmm4
	vaddss	%xmm3, %xmm11, %xmm3
	vmulss	%xmm3, %xmm3, %xmm3
	vfmadd231ss	%xmm4, %xmm4, %xmm3
	vsqrtss	%xmm3, %xmm3, %xmm3
	vmovss	%xmm3, 4(%rsi,%rbx,4)
	xorl	%r10d, %r10d
	nopl	(%rax,%rax)
	vmovss	-8(%r14,%r10), %xmm3
	vmovss	-4(%r14,%r10), %xmm4
	vmovss	-12(%r11,%r10), %xmm5
	vmovss	-8(%r11,%r10), %xmm6
	vmovss	-4(%r11,%r10), %xmm7
	vmovss	-12(%r15,%r10), %xmm8
	vmovss	-8(%r15,%r10), %xmm9
	vsubss	-12(%r14,%r10), %xmm0, %xmm10
	vmovss	-4(%r15,%r10), %xmm11
	vxorps	%xmm12, %xmm12, %xmm12
	vfmadd213ss	%xmm10, %xmm3, %xmm12
	vfmadd213ss	%xmm10, %xmm1, %xmm3
	vaddss	%xmm4, %xmm12, %xmm10
	vsubss	%xmm4, %xmm3, %xmm3
	vfmadd231ss	%xmm1, %xmm5, %xmm10
	vfmadd231ss	%xmm5, %xmm0, %xmm3
	vfmadd231ss	%xmm0, %xmm6, %xmm10
	vfmadd231ss	%xmm6, %xmm0, %xmm3
	vfmadd231ss	%xmm2, %xmm7, %xmm10
	vfmadd231ss	%xmm7, %xmm0, %xmm3
	vsubss	%xmm8, %xmm10, %xmm4
	vaddss	%xmm3, %xmm8, %xmm3
	vfmadd231ss	%xmm0, %xmm9, %xmm4
	vfmadd231ss	%xmm9, %xmm2, %xmm3
	vaddss	%xmm4, %xmm11, %xmm4
	vaddss	%xmm3, %xmm11, %xmm3
	vmulss	%xmm3, %xmm3, %xmm3
	vfmadd231ss	%xmm4, %xmm4, %xmm3
	vsqrtss	%xmm3, %xmm3, %xmm3
	vmovss	%xmm3, -4(%rcx,%r10)
	cmpq	$15344, %r10
	je	-848
	vmovss	-4(%r14,%r10), %xmm3
	vmovss	(%r14,%r10), %xmm4
	vmovss	-8(%r11,%r10), %xmm5
	vmovss	-4(%r11,%r10), %xmm6
	vmovss	(%r11,%r10), %xmm7
	vmovss	-8(%r15,%r10), %xmm8
	vmovss	-4(%r15,%r10), %xmm9
	vsubss	-8(%r14,%r10), %xmm0, %xmm10
	vmovss	(%r15,%r10), %xmm11
	vxorps	%xmm12, %xmm12, %xmm12
	vfmadd213ss	%xmm10, %xmm3, %xmm12
	vfmadd213ss	%xmm10, %xmm1, %xmm3
	vaddss	%xmm4, %xmm12, %xmm10
	vsubss	%xmm4, %xmm3, %xmm3
	vfmadd231ss	%xmm1, %xmm5, %xmm10
	vfmadd231ss	%xmm5, %xmm0, %xmm3
	vfmadd231ss	%xmm0, %xmm6, %xmm10
	vfmadd231ss	%xmm6, %xmm0, %xmm3
	vfmadd231ss	%xmm2, %xmm7, %xmm10
	vfmadd231ss	%xmm7, %xmm0, %xmm3
	vsubss	%xmm8, %xmm10, %xmm4
	vaddss	%xmm3, %xmm8, %xmm3
	vfmadd231ss	%xmm0, %xmm9, %xmm4
	vfmadd231ss	%xmm9, %xmm2, %xmm3
	vaddss	%xmm4, %xmm11, %xmm4
	vaddss	%xmm3, %xmm11, %xmm3
	vmulss	%xmm3, %xmm3, %xmm3
	vfmadd231ss	%xmm4, %xmm4, %xmm3
	vsqrtss	%xmm3, %xmm3, %xmm3
	vmovss	%xmm3, (%rcx,%r10)
	addq	$8, %r10
	jmp	-344
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
