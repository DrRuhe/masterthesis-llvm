<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ46create_separable_gaussian_tradeoff_specializediiPKfiE3$_0JS3_PfEEET_PT0_DpT1_>:
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	pushq	%rax
	movq	%rsi, %rbx
	movq	%rdi, %r12
	leaq	-7(%rip), %rax
	movabsq	$-8121, %rbp
	addq	%rax, %rbp
	movabsq	$48, %rax
	movl	$58982400, %edi
	callq	*(%rbp,%rax)
	movq	%rax, %r15
	movq	%rax, %r14
	xorl	%r13d, %r13d
	movabsq	$56, %rax
	movl	$58982400, %edx
	movq	%r15, %rdi
	xorl	%esi, %esi
	callq	*(%rbp,%rax)
	leaq	44(%r12), %rax
	leaq	44(%r15), %rcx
	movabsq	$-504, %rdx
	vmovss	(%rbp,%rdx), %xmm0
	vxorps	%xmm3, %xmm3, %xmm3
	movabsq	$-500, %rdx
	vmovss	(%rbp,%rdx), %xmm1
	movabsq	$-496, %rdx
	movq	%rbp, (%rsp)
	vmovss	(%rbp,%rdx), %xmm2
	movl	$3837, %edx
	nop
	imulq	$15360, %r13, %rdi
	vmovss	(%r12,%rdi), %xmm4
	vmovss	4(%r12,%rdi), %xmm6
	vmovaps	%xmm0, %xmm7
	vfmadd213ss	%xmm3, %xmm4, %xmm7
	vmovaps	%xmm1, %xmm8
	vfmadd213ss	%xmm7, %xmm4, %xmm8
	vfmadd213ss	%xmm8, %xmm2, %xmm4
	vfmadd231ss	%xmm1, %xmm6, %xmm4
	vmovss	8(%r12,%rdi), %xmm5
	vfmadd231ss	%xmm0, %xmm5, %xmm4
	vmovss	%xmm4, (%r14,%rdi)
	vfmadd231ss	%xmm2, %xmm6, %xmm8
	vfmadd231ss	%xmm1, %xmm5, %xmm8
	vmovss	12(%r12,%rdi), %xmm4
	vfmadd231ss	%xmm0, %xmm4, %xmm8
	vmovss	%xmm8, 4(%r14,%rdi)
	vfmadd231ss	%xmm1, %xmm6, %xmm7
	vfmadd231ss	%xmm2, %xmm5, %xmm7
	vfmadd231ss	%xmm1, %xmm4, %xmm7
	vmovss	16(%r12,%rdi), %xmm8
	vfmadd231ss	%xmm0, %xmm8, %xmm7
	vmovss	%xmm7, 8(%r14,%rdi)
	vfmadd213ss	%xmm3, %xmm0, %xmm6
	vfmadd231ss	%xmm1, %xmm5, %xmm6
	vfmadd231ss	%xmm2, %xmm4, %xmm6
	vfmadd231ss	%xmm1, %xmm8, %xmm6
	vmovss	20(%r12,%rdi), %xmm7
	vfmadd231ss	%xmm0, %xmm7, %xmm6
	vmovss	%xmm6, 12(%r14,%rdi)
	leaq	8(%r12,%rdi), %rsi
	vfmadd213ss	%xmm3, %xmm0, %xmm5
	vfmadd231ss	%xmm1, %xmm4, %xmm5
	vfmadd231ss	%xmm2, %xmm8, %xmm5
	vfmadd231ss	%xmm1, %xmm7, %xmm5
	vmovss	24(%r12,%rdi), %xmm6
	vfmadd231ss	%xmm0, %xmm6, %xmm5
	vmovss	%xmm5, 16(%r14,%rdi)
	vfmadd213ss	%xmm3, %xmm0, %xmm4
	vfmadd231ss	%xmm8, %xmm1, %xmm4
	vfmadd231ss	%xmm7, %xmm2, %xmm4
	vfmadd231ss	%xmm6, %xmm1, %xmm4
	vfmadd231ss	28(%r12,%rdi), %xmm0, %xmm4
	vmovss	%xmm4, 20(%r14,%rdi)
	xorl	%edi, %edi
	nopl	(%rax,%rax)
	vmovss	-28(%rax,%rdi,4), %xmm4
	vfmadd132ss	%xmm0, %xmm3, %xmm4
	vmovss	-24(%rax,%rdi,4), %xmm7
	vmovss	-20(%rax,%rdi,4), %xmm8
	vfmadd231ss	%xmm1, %xmm7, %xmm4
	vfmadd231ss	%xmm2, %xmm8, %xmm4
	vmovss	-16(%rax,%rdi,4), %xmm6
	vfmadd231ss	%xmm1, %xmm6, %xmm4
	vmovss	-12(%rax,%rdi,4), %xmm5
	vfmadd231ss	%xmm0, %xmm5, %xmm4
	vmovss	%xmm4, -20(%rcx,%rdi,4)
	vfmadd213ss	%xmm3, %xmm0, %xmm7
	vfmadd231ss	%xmm1, %xmm8, %xmm7
	vfmadd231ss	%xmm2, %xmm6, %xmm7
	vfmadd231ss	%xmm1, %xmm5, %xmm7
	vmovss	-8(%rax,%rdi,4), %xmm4
	vfmadd231ss	%xmm0, %xmm4, %xmm7
	vmovss	%xmm7, -16(%rcx,%rdi,4)
	vfmadd213ss	%xmm3, %xmm0, %xmm8
	vfmadd231ss	%xmm1, %xmm6, %xmm8
	vfmadd231ss	%xmm2, %xmm5, %xmm8
	vfmadd231ss	%xmm1, %xmm4, %xmm8
	vmovss	-4(%rax,%rdi,4), %xmm7
	vfmadd231ss	%xmm0, %xmm7, %xmm8
	vmovss	%xmm8, -12(%rcx,%rdi,4)
	vfmadd213ss	%xmm3, %xmm0, %xmm6
	vfmadd231ss	%xmm1, %xmm5, %xmm6
	vfmadd231ss	%xmm2, %xmm4, %xmm6
	vfmadd231ss	%xmm1, %xmm7, %xmm6
	vmovss	(%rax,%rdi,4), %xmm8
	vfmadd231ss	%xmm0, %xmm8, %xmm6
	vmovss	%xmm6, -8(%rcx,%rdi,4)
	leal	10(%rdi), %r8d
	vfmadd213ss	%xmm3, %xmm0, %xmm5
	vfmadd231ss	%xmm1, %xmm4, %xmm5
	vfmadd231ss	%xmm2, %xmm7, %xmm5
	vfmadd231ss	%xmm1, %xmm8, %xmm5
	cmpl	$3837, %r8d
	cmovgel	%edx, %r8d
	movslq	%r8d, %r8
	vmovss	(%rsi,%r8,4), %xmm6
	vfmadd231ss	%xmm0, %xmm6, %xmm5
	vmovss	%xmm5, -4(%rcx,%rdi,4)
	leal	11(%rdi), %r8d
	vfmadd213ss	%xmm3, %xmm0, %xmm4
	vfmadd231ss	%xmm7, %xmm1, %xmm4
	vfmadd231ss	%xmm8, %xmm2, %xmm4
	vfmadd231ss	%xmm6, %xmm1, %xmm4
	cmpl	$3837, %r8d
	cmovgel	%edx, %r8d
	movslq	%r8d, %r8
	vfmadd231ss	(%rsi,%r8,4), %xmm0, %xmm4
	vmovss	%xmm4, (%rcx,%rdi,4)
	addq	$6, %rdi
	cmpq	$3834, %rdi
	jne	-292
	incq	%r13
	addq	$15360, %rax
	addq	$15360, %rcx
	cmpq	$3840, %r13
	jne	-577
	addq	$20, %r15
	addq	$20, %rbx
	movl	$2, %eax
	movl	$1, %ecx
	movl	$4294967295, %edx
	movl	$4294967294, %esi
	xorl	%edi, %edi
	vxorps	%xmm3, %xmm3, %xmm3
	movq	%r15, %r8
	nopw	%cs:(%rax,%rax)
	cmpq	$3839, %rax
	movl	$3839, %r9d
	cmovbq	%rax, %r9
	imulq	$15360, %r9, %r9
	addq	%r15, %r9
	cmpq	$3839, %rcx
	movl	$3839, %r10d
	cmovbq	%rcx, %r10
	imulq	$15360, %r10, %r10
	addq	%r15, %r10
	movl	%edx, %r11d
	sarl	$31, %r11d
	andnl	%edx, %r11d, %r11d
	imulq	$15360, %r11, %r11
	addq	%r15, %r11
	movl	%esi, %ebp
	sarl	$31, %ebp
	andnl	%esi, %ebp, %r12d
	imulq	$15360, %r12, %r12
	addq	%r15, %r12
	xorl	%r13d, %r13d
	nopw	%cs:(%rax,%rax)
	vmovss	-20(%r12,%r13), %xmm4
	vfmadd132ss	%xmm0, %xmm3, %xmm4
	vfmadd231ss	-20(%r11,%r13), %xmm1, %xmm4
	vfmadd231ss	-20(%r8,%r13), %xmm2, %xmm4
	vfmadd231ss	-20(%r10,%r13), %xmm1, %xmm4
	vfmadd231ss	-20(%r9,%r13), %xmm0, %xmm4
	vmovss	%xmm4, -20(%rbx,%r13)
	vmovss	-16(%r12,%r13), %xmm4
	vfmadd132ss	%xmm0, %xmm3, %xmm4
	vfmadd231ss	-16(%r11,%r13), %xmm1, %xmm4
	vfmadd231ss	-16(%r8,%r13), %xmm2, %xmm4
	vfmadd231ss	-16(%r10,%r13), %xmm1, %xmm4
	vfmadd231ss	-16(%r9,%r13), %xmm0, %xmm4
	vmovss	%xmm4, -16(%rbx,%r13)
	vmovss	-12(%r12,%r13), %xmm4
	vfmadd132ss	%xmm0, %xmm3, %xmm4
	vfmadd231ss	-12(%r11,%r13), %xmm1, %xmm4
	vfmadd231ss	-12(%r8,%r13), %xmm2, %xmm4
	vfmadd231ss	-12(%r10,%r13), %xmm1, %xmm4
	vfmadd231ss	-12(%r9,%r13), %xmm0, %xmm4
	vmovss	%xmm4, -12(%rbx,%r13)
	vmovss	-8(%r12,%r13), %xmm4
	vfmadd132ss	%xmm0, %xmm3, %xmm4
	vfmadd231ss	-8(%r11,%r13), %xmm1, %xmm4
	vfmadd231ss	-8(%r8,%r13), %xmm2, %xmm4
	vfmadd231ss	-8(%r10,%r13), %xmm1, %xmm4
	vfmadd231ss	-8(%r9,%r13), %xmm0, %xmm4
	vmovss	%xmm4, -8(%rbx,%r13)
	vmovss	-4(%r12,%r13), %xmm4
	vfmadd132ss	%xmm0, %xmm3, %xmm4
	vfmadd231ss	-4(%r11,%r13), %xmm1, %xmm4
	vfmadd231ss	-4(%r8,%r13), %xmm2, %xmm4
	vfmadd231ss	-4(%r10,%r13), %xmm1, %xmm4
	vfmadd231ss	-4(%r9,%r13), %xmm0, %xmm4
	vmovss	%xmm4, -4(%rbx,%r13)
	vmovss	(%r12,%r13), %xmm4
	vfmadd132ss	%xmm0, %xmm3, %xmm4
	vfmadd231ss	(%r11,%r13), %xmm1, %xmm4
	vfmadd231ss	(%r8,%r13), %xmm2, %xmm4
	vfmadd231ss	(%r10,%r13), %xmm1, %xmm4
	vfmadd231ss	(%r9,%r13), %xmm0, %xmm4
	vmovss	%xmm4, (%rbx,%r13)
	addq	$24, %r13
	cmpq	$15360, %r13
	jne	-293
	incq	%rdi
	incq	%rax
	incq	%rcx
	addq	$15360, %r8
	incl	%edx
	incl	%esi
	addq	$15360, %rbx
	cmpq	$3840, %rdi
	jne	-445
	movabsq	$32, %rax
	movl	$58982400, %esi
	movq	%r14, %rdi
	movq	(%rsp), %rcx
	addq	$8, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	jmpq	*(%rcx,%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
	addb	%al, (%rax)
