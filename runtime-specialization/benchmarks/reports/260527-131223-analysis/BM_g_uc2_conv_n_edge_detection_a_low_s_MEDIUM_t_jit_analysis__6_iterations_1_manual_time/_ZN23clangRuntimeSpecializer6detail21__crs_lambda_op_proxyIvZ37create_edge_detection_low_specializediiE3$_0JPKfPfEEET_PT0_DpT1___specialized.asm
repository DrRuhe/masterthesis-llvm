<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ37create_edge_detection_low_specializediiE3$_0JPKfPfEEET_PT0_DpT1_>:
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	leaq	-7(%rip), %rax
	movabsq	$-8122, %r10
	addq	%rax, %r10
	leaq	4(%rdi), %rax
	movl	$1, %ecx
	movl	$4294967295, %edx
	xorl	%r8d, %r8d
	movl	$3839, %r9d
	movabsq	$-496, %r11
	vmovss	(%r10,%r11), %xmm0
	movabsq	$-492, %r11
	vmovss	(%r10,%r11), %xmm1
	nopw	%cs:(%rax,%rax)
	cmpl	$3839, %ecx
	movl	$3839, %r10d
	cmovll	%ecx, %r10d
	imulq	$15360, %r10, %r10
	addq	%rdi, %r10
	movl	%edx, %r11d
	sarl	$31, %r11d
	andnl	%edx, %r11d, %r11d
	imulq	$15360, %r11, %r11
	addq	%rdi, %r11
	leal	-1(%r8), %ebx
	cmpl	$3839, %ebx
	cmovgel	%r9d, %ebx
	movl	%ebx, %ebp
	sarl	$31, %ebp
	andnl	%ebx, %ebp, %ebx
	imull	$3840, %ebx, %r15d
	cmpl	$3839, %r8d
	movl	$3839, %ebx
	cmovll	%r8d, %ebx
	movl	%ebx, %ebp
	sarl	$31, %ebp
	andnl	%ebx, %ebp, %ebx
	imull	$3840, %ebx, %r14d
	incq	%r8
	cmpl	$3839, %r8d
	movl	$3839, %ebx
	cmovll	%r8d, %ebx
	movl	%ebx, %ebp
	sarl	$31, %ebp
	andnl	%ebx, %ebp, %ebx
	imull	$3840, %ebx, %r12d
	movq	$-3839, %rbx
	nopl	(%rax,%rax)
	leal	3838(%rbx), %ebp
	movl	%ebp, %r13d
	sarl	$31, %r13d
	andnl	%ebp, %r13d, %r13d
	leal	(%r15,%r13), %ebp
	vmovss	(%rdi,%rbp,4), %xmm2
	vmovss	15360(%r11,%rbx,4), %xmm3
	leal	(%r14,%r13), %ebp
	addl	%r12d, %r13d
	vmovss	(%rdi,%r13,4), %xmm4
	vmovss	15360(%r10,%rbx,4), %xmm5
	vsubss	%xmm2, %xmm3, %xmm6
	vfmadd231ss	(%rdi,%rbp,4), %xmm0, %xmm6
	vfmadd231ss	15356(%rax,%rbx,4), %xmm1, %xmm6
	vsubss	%xmm4, %xmm6, %xmm6
	vaddss	%xmm5, %xmm6, %xmm6
	vfmsub231ss	15356(%r11,%rbx,4), %xmm0, %xmm2
	vsubss	%xmm3, %xmm2, %xmm2
	vaddss	%xmm2, %xmm4, %xmm2
	vfmadd231ss	15356(%r10,%rbx,4), %xmm1, %xmm2
	vaddss	%xmm2, %xmm5, %xmm2
	vmulss	%xmm2, %xmm2, %xmm2
	vfmadd231ss	%xmm6, %xmm6, %xmm2
	vsqrtss	%xmm2, %xmm2, %xmm2
	vmovss	%xmm2, 15356(%rsi,%rbx,4)
	incq	%rbx
	jne	-151
	vmovss	15352(%r11,%rbx,4), %xmm2
	leal	3840(%rbx), %ebp
	cmpl	$3839, %ebp
	cmovgel	%r9d, %ebp
	movl	%ebp, %r13d
	sarl	$31, %r13d
	andnl	%ebp, %r13d, %ebp
	addl	%ebp, %r15d
	vmovss	(%rdi,%r15,4), %xmm3
	addl	%ebp, %r14d
	vmovss	15352(%r10,%rbx,4), %xmm4
	addl	%ebp, %r12d
	vmovss	(%rdi,%r12,4), %xmm5
	vsubss	%xmm2, %xmm3, %xmm6
	vfmadd231ss	15348(%rax,%rbx,4), %xmm0, %xmm6
	vfmadd231ss	(%rdi,%r14,4), %xmm1, %xmm6
	vsubss	%xmm4, %xmm6, %xmm6
	vaddss	%xmm5, %xmm6, %xmm6
	vfmsub231ss	15356(%r11,%rbx,4), %xmm0, %xmm2
	vsubss	%xmm3, %xmm2, %xmm2
	vaddss	%xmm2, %xmm4, %xmm2
	vfmadd231ss	15356(%r10,%rbx,4), %xmm1, %xmm2
	vaddss	%xmm2, %xmm5, %xmm2
	vmulss	%xmm2, %xmm2, %xmm2
	vfmadd231ss	%xmm6, %xmm6, %xmm2
	vsqrtss	%xmm2, %xmm2, %xmm2
	vmovss	%xmm2, 15356(%rsi,%rbx,4)
	addq	$15360, %rsi
	incl	%ecx
	incl	%edx
	addq	$15360, %rax
	cmpq	$3840, %r8
	jne	-492
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
