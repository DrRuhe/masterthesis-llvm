<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ37create_edge_detection_low_specializediiE3$_0JPKfPfEEET_PT0_DpT1_>:
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	leaq	-7(%rip), %rax
	movabsq	$-8202, %r10
	addq	%rax, %r10
	leaq	16(%rdi), %rax
	leaq	12(%rsi), %rcx
	movl	$4294967295, %edx
	movl	$1, %r8d
	xorl	%r9d, %r9d
	movabsq	$-416, %r11
	vmovss	(%r10,%r11), %xmm0
	movabsq	$-412, %r11
	vmovss	(%r10,%r11), %xmm1
	movq	%rax, %r11
	jmp	154
	nopl	(%rax)
	vmovss	15352(%rdi,%r12,4), %xmm2
	vmovss	15356(%rdi,%r12,4), %xmm3
	vmovss	15352(%rdi,%rbp,4), %xmm4
	vmovss	15356(%rdi,%rbp,4), %xmm5
	vsubss	%xmm2, %xmm3, %xmm6
	vfmadd231ss	15352(%rdi,%r13,4), %xmm0, %xmm6
	vfmadd231ss	15356(%rdi,%r13,4), %xmm1, %xmm6
	vsubss	%xmm4, %xmm6, %xmm6
	vaddss	%xmm5, %xmm6, %xmm6
	vfmsub231ss	-4(%r14,%r10), %xmm0, %xmm2
	vsubss	%xmm3, %xmm2, %xmm2
	vaddss	%xmm4, %xmm2, %xmm2
	vfmadd231ss	-4(%r15,%r10), %xmm1, %xmm2
	vaddss	%xmm2, %xmm5, %xmm2
	vmulss	%xmm2, %xmm2, %xmm2
	vfmadd231ss	%xmm6, %xmm6, %xmm2
	vsqrtss	%xmm2, %xmm2, %xmm2
	vmovss	%xmm2, 15356(%rsi,%rbx,4)
	incl	%edx
	addq	$15360, %r11
	incq	%r8
	addq	$15360, %rcx
	cmpq	$3840, %r9
	je	564
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
	vmovss	(%rdi,%r12,4), %xmm2
	vmovss	4(%rdi,%r12,4), %xmm3
	vmovss	(%rdi,%rbp,4), %xmm4
	vmovss	4(%rdi,%rbp,4), %xmm5
	vsubss	%xmm2, %xmm3, %xmm6
	vfmadd231ss	(%rdi,%r13,4), %xmm0, %xmm6
	vfmadd231ss	4(%rdi,%r13,4), %xmm1, %xmm6
	vsubss	%xmm4, %xmm6, %xmm6
	vaddss	%xmm5, %xmm6, %xmm6
	vfmsub231ss	%xmm2, %xmm0, %xmm2
	vsubss	%xmm3, %xmm2, %xmm2
	vaddss	%xmm4, %xmm2, %xmm2
	vfmadd231ss	%xmm4, %xmm1, %xmm2
	vaddss	%xmm2, %xmm5, %xmm2
	vmulss	%xmm2, %xmm2, %xmm2
	vfmadd231ss	%xmm6, %xmm6, %xmm2
	vsqrtss	%xmm2, %xmm2, %xmm2
	vmovss	%xmm2, (%rsi,%rbx,4)
	vmovss	(%rdi,%r12,4), %xmm2
	vmovss	8(%rdi,%r12,4), %xmm3
	vmovss	(%rdi,%rbp,4), %xmm4
	vsubss	%xmm2, %xmm3, %xmm5
	vfmadd231ss	(%rdi,%r13,4), %xmm0, %xmm5
	vmovss	8(%rdi,%rbp,4), %xmm6
	vfmadd231ss	8(%rdi,%r13,4), %xmm1, %xmm5
	vsubss	%xmm4, %xmm5, %xmm5
	vfmsub231ss	4(%rdi,%r12,4), %xmm0, %xmm2
	vaddss	%xmm6, %xmm5, %xmm5
	vsubss	%xmm3, %xmm2, %xmm2
	vaddss	%xmm4, %xmm2, %xmm2
	vfmadd231ss	4(%rdi,%rbp,4), %xmm1, %xmm2
	vaddss	%xmm2, %xmm6, %xmm2
	vmulss	%xmm2, %xmm2, %xmm2
	vfmadd231ss	%xmm5, %xmm5, %xmm2
	vsqrtss	%xmm2, %xmm2, %xmm2
	vmovss	%xmm2, 4(%rsi,%rbx,4)
	xorl	%r10d, %r10d
	nopl	(%rax)
	vmovss	-12(%r14,%r10), %xmm2
	vmovss	-4(%r14,%r10), %xmm3
	vmovss	-12(%r15,%r10), %xmm4
	vsubss	%xmm2, %xmm3, %xmm5
	vfmadd231ss	-12(%r11,%r10), %xmm0, %xmm5
	vmovss	-4(%r15,%r10), %xmm6
	vfmadd231ss	-4(%r11,%r10), %xmm1, %xmm5
	vsubss	%xmm4, %xmm5, %xmm5
	vfmsub231ss	-8(%r14,%r10), %xmm0, %xmm2
	vaddss	%xmm6, %xmm5, %xmm5
	vsubss	%xmm3, %xmm2, %xmm2
	vaddss	%xmm4, %xmm2, %xmm2
	vfmadd231ss	-8(%r15,%r10), %xmm1, %xmm2
	vaddss	%xmm2, %xmm6, %xmm2
	vmulss	%xmm2, %xmm2, %xmm2
	vfmadd231ss	%xmm5, %xmm5, %xmm2
	vsqrtss	%xmm2, %xmm2, %xmm2
	vmovss	%xmm2, -4(%rcx,%r10)
	cmpq	$15344, %r10
	je	-609
	vmovss	-8(%r14,%r10), %xmm2
	vmovss	(%r14,%r10), %xmm3
	vmovss	-8(%r15,%r10), %xmm4
	vsubss	%xmm2, %xmm3, %xmm5
	vfmadd231ss	-8(%r11,%r10), %xmm0, %xmm5
	vmovss	(%r15,%r10), %xmm6
	vfmadd231ss	(%r11,%r10), %xmm1, %xmm5
	vsubss	%xmm4, %xmm5, %xmm5
	vfmsub231ss	-4(%r14,%r10), %xmm0, %xmm2
	vaddss	%xmm6, %xmm5, %xmm5
	vsubss	%xmm3, %xmm2, %xmm2
	vaddss	%xmm4, %xmm2, %xmm2
	vfmadd231ss	-4(%r15,%r10), %xmm1, %xmm2
	vaddss	%xmm2, %xmm6, %xmm2
	vmulss	%xmm2, %xmm2, %xmm2
	vfmadd231ss	%xmm5, %xmm5, %xmm2
	vsqrtss	%xmm2, %xmm2, %xmm2
	vmovss	%xmm2, (%rcx,%r10)
	addq	$8, %r10
	jmp	-218
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
