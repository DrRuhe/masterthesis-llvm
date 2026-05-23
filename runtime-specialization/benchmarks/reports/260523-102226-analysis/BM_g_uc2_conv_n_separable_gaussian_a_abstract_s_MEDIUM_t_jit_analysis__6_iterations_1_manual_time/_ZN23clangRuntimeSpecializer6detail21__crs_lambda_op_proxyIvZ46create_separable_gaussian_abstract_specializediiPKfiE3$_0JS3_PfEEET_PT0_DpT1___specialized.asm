<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ46create_separable_gaussian_abstract_specializediiPKfiE3$_0JS3_PfEEET_PT0_DpT1_>:
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$24, %rsp
	movq	%rsi, %rbx
	movq	%rdi, %r12
	leaq	-7(%rip), %rax
	movabsq	$-8076, %r15
	addq	%rax, %r15
	movabsq	$0, %rax
	movl	$58982400, %edi
	callq	*(%r15,%rax)
	movq	%rax, %rbp
	movq	%rax, %r14
	xorl	%r13d, %r13d
	movabsq	$16, %rax
	movl	$58982400, %edx
	movq	%rbp, %rdi
	xorl	%esi, %esi
	callq	*(%r15,%rax)
	leaq	44(%r12), %rax
	leaq	44(%rbp), %rcx
	leaq	15404(%r12), %rdx
	movq	%rbp, 8(%rsp)
	leaq	15404(%rbp), %rsi
	movabsq	$-600, %rdi
	vmovss	(%r15,%rdi), %xmm0
	vxorps	%xmm3, %xmm3, %xmm3
	movabsq	$-596, %rdi
	vmovss	(%r15,%rdi), %xmm1
	movabsq	$-592, %rdi
	movq	%r15, 16(%rsp)
	vmovss	(%r15,%rdi), %xmm2
	movl	$3836, %edi
	nopw	(%rax,%rax)
	imulq	$3840, %r13, %r8
	movl	%r8d, %r11d
	andl	$4294966784, %r11d
	vmovss	(%r12,%r11,4), %xmm5
	vmovaps	%xmm0, %xmm7
	vfmadd213ss	%xmm3, %xmm5, %xmm7
	vfmadd213ss	%xmm7, %xmm1, %xmm5
	vmovss	(%r12,%r8,4), %xmm8
	vmovss	4(%r12,%r8,4), %xmm6
	vfmadd231ss	%xmm2, %xmm8, %xmm5
	vfmadd231ss	%xmm1, %xmm6, %xmm5
	vmovss	8(%r12,%r8,4), %xmm4
	vfmadd231ss	%xmm0, %xmm4, %xmm5
	vmovss	%xmm5, (%r14,%r8,4)
	leal	2(%r8), %r9d
	vfmadd231ss	%xmm1, %xmm8, %xmm7
	vfmadd231ss	%xmm2, %xmm6, %xmm7
	vfmadd231ss	%xmm1, %xmm4, %xmm7
	vmovss	12(%r12,%r8,4), %xmm5
	vfmadd231ss	%xmm0, %xmm5, %xmm7
	vmovss	%xmm7, 4(%r14,%r8,4)
	vfmadd213ss	%xmm3, %xmm0, %xmm8
	vfmadd231ss	%xmm1, %xmm6, %xmm8
	vfmadd231ss	%xmm2, %xmm4, %xmm8
	vfmadd231ss	%xmm1, %xmm5, %xmm8
	vmovss	16(%r12,%r8,4), %xmm7
	vfmadd231ss	%xmm0, %xmm7, %xmm8
	vmovss	%xmm8, 8(%r14,%r8,4)
	vfmadd213ss	%xmm3, %xmm0, %xmm6
	vfmadd231ss	%xmm1, %xmm4, %xmm6
	vfmadd231ss	%xmm2, %xmm5, %xmm6
	vfmadd231ss	%xmm1, %xmm7, %xmm6
	vmovss	20(%r12,%r8,4), %xmm8
	vfmadd231ss	%xmm0, %xmm8, %xmm6
	vmovss	%xmm6, 12(%r14,%r8,4)
	leal	3(%r8), %r10d
	vfmadd213ss	%xmm3, %xmm0, %xmm4
	vfmadd231ss	%xmm1, %xmm5, %xmm4
	vfmadd231ss	%xmm2, %xmm7, %xmm4
	vfmadd231ss	%xmm1, %xmm8, %xmm4
	vmovss	24(%r12,%r11,4), %xmm6
	vfmadd231ss	%xmm0, %xmm6, %xmm4
	vmovss	%xmm4, 16(%r14,%r8,4)
	vfmadd213ss	%xmm3, %xmm0, %xmm5
	vfmadd231ss	%xmm7, %xmm1, %xmm5
	vfmadd231ss	%xmm8, %xmm2, %xmm5
	vfmadd231ss	%xmm6, %xmm1, %xmm5
	vfmadd231ss	28(%r12,%r11,4), %xmm0, %xmm5
	vmovss	%xmm5, 20(%r14,%r8,4)
	xorl	%r11d, %r11d
	nopw	%cs:(%rax,%rax)
	vmovss	-28(%rax,%r11,4), %xmm4
	vfmadd132ss	%xmm0, %xmm3, %xmm4
	vmovss	-24(%rax,%r11,4), %xmm7
	vmovss	-20(%rax,%r11,4), %xmm8
	vfmadd231ss	%xmm1, %xmm7, %xmm4
	vfmadd231ss	%xmm2, %xmm8, %xmm4
	vmovss	-16(%rax,%r11,4), %xmm6
	vfmadd231ss	%xmm1, %xmm6, %xmm4
	vmovss	-12(%rax,%r11,4), %xmm5
	vfmadd231ss	%xmm0, %xmm5, %xmm4
	vmovss	%xmm4, -20(%rcx,%r11,4)
	vfmadd213ss	%xmm3, %xmm0, %xmm7
	vfmadd231ss	%xmm1, %xmm8, %xmm7
	vfmadd231ss	%xmm2, %xmm6, %xmm7
	vfmadd231ss	%xmm1, %xmm5, %xmm7
	vmovss	-8(%rax,%r11,4), %xmm4
	vfmadd231ss	%xmm0, %xmm4, %xmm7
	vmovss	%xmm7, -16(%rcx,%r11,4)
	vfmadd213ss	%xmm3, %xmm0, %xmm8
	vfmadd231ss	%xmm1, %xmm6, %xmm8
	vfmadd231ss	%xmm2, %xmm5, %xmm8
	vfmadd231ss	%xmm1, %xmm4, %xmm8
	vmovss	-4(%rax,%r11,4), %xmm7
	vfmadd231ss	%xmm0, %xmm7, %xmm8
	vmovss	%xmm8, -12(%rcx,%r11,4)
	vfmadd213ss	%xmm3, %xmm0, %xmm6
	vfmadd231ss	%xmm1, %xmm5, %xmm6
	vfmadd231ss	%xmm2, %xmm4, %xmm6
	vfmadd231ss	%xmm1, %xmm7, %xmm6
	vmovss	(%rax,%r11,4), %xmm8
	vfmadd231ss	%xmm0, %xmm8, %xmm6
	vmovss	%xmm6, -8(%rcx,%r11,4)
	leal	10(%r11), %ebp
	vfmadd213ss	%xmm3, %xmm0, %xmm5
	vfmadd231ss	%xmm1, %xmm4, %xmm5
	vfmadd231ss	%xmm2, %xmm7, %xmm5
	vfmadd231ss	%xmm1, %xmm8, %xmm5
	cmpl	$3837, %ebp
	movl	$3837, %r15d
	cmovll	%ebp, %r15d
	addl	%r9d, %r15d
	vmovss	(%r12,%r15,4), %xmm6
	vfmadd231ss	%xmm0, %xmm6, %xmm5
	vmovss	%xmm5, -4(%rcx,%r11,4)
	vfmadd213ss	%xmm3, %xmm0, %xmm4
	vfmadd231ss	%xmm7, %xmm1, %xmm4
	vfmadd231ss	%xmm8, %xmm2, %xmm4
	vfmadd231ss	%xmm6, %xmm1, %xmm4
	cmpl	$3836, %ebp
	cmovgel	%edi, %ebp
	addl	%r10d, %ebp
	vfmadd231ss	(%r12,%rbp,4), %xmm0, %xmm4
	vmovss	%xmm4, (%rcx,%r11,4)
	addq	$6, %r11
	cmpq	$3834, %r11
	jne	-305
	leaq	3840(%r8), %r11
	movl	%r11d, %r9d
	movl	%r11d, %r10d
	andl	$4294967040, %r11d
	vmovss	(%r12,%r11,4), %xmm5
	vmovaps	%xmm0, %xmm7
	vfmadd213ss	%xmm3, %xmm5, %xmm7
	vfmadd213ss	%xmm7, %xmm1, %xmm5
	vmovss	15360(%r12,%r8,4), %xmm8
	vmovss	15364(%r12,%r8,4), %xmm6
	vfmadd231ss	%xmm2, %xmm8, %xmm5
	vfmadd231ss	%xmm1, %xmm6, %xmm5
	vmovss	15368(%r12,%r8,4), %xmm4
	vfmadd231ss	%xmm0, %xmm4, %xmm5
	vmovss	%xmm5, 15360(%r14,%r8,4)
	orl	$2, %r9d
	vfmadd231ss	%xmm1, %xmm8, %xmm7
	vfmadd231ss	%xmm2, %xmm6, %xmm7
	vfmadd231ss	%xmm1, %xmm4, %xmm7
	vmovss	15372(%r12,%r8,4), %xmm5
	vfmadd231ss	%xmm0, %xmm5, %xmm7
	vmovss	%xmm7, 15364(%r14,%r8,4)
	vfmadd213ss	%xmm3, %xmm0, %xmm8
	vfmadd231ss	%xmm1, %xmm6, %xmm8
	vfmadd231ss	%xmm2, %xmm4, %xmm8
	vfmadd231ss	%xmm1, %xmm5, %xmm8
	vmovss	15376(%r12,%r8,4), %xmm7
	vfmadd231ss	%xmm0, %xmm7, %xmm8
	vmovss	%xmm8, 15368(%r14,%r8,4)
	vfmadd213ss	%xmm3, %xmm0, %xmm6
	vfmadd231ss	%xmm1, %xmm4, %xmm6
	vfmadd231ss	%xmm2, %xmm5, %xmm6
	vfmadd231ss	%xmm1, %xmm7, %xmm6
	vmovss	15380(%r12,%r8,4), %xmm8
	vfmadd231ss	%xmm0, %xmm8, %xmm6
	vmovss	%xmm6, 15372(%r14,%r8,4)
	orl	$3, %r10d
	vfmadd213ss	%xmm3, %xmm0, %xmm4
	vfmadd231ss	%xmm1, %xmm5, %xmm4
	vfmadd231ss	%xmm2, %xmm7, %xmm4
	vfmadd231ss	%xmm1, %xmm8, %xmm4
	vmovss	24(%r12,%r11,4), %xmm6
	vfmadd231ss	%xmm0, %xmm6, %xmm4
	vmovss	%xmm4, 15376(%r14,%r8,4)
	vfmadd213ss	%xmm3, %xmm0, %xmm5
	vfmadd231ss	%xmm7, %xmm1, %xmm5
	vfmadd231ss	%xmm8, %xmm2, %xmm5
	vfmadd231ss	%xmm6, %xmm1, %xmm5
	vfmadd231ss	28(%r12,%r11,4), %xmm0, %xmm5
	vmovss	%xmm5, 15380(%r14,%r8,4)
	xorl	%r8d, %r8d
	nopl	(%rax)
	vmovss	-28(%rdx,%r8,4), %xmm4
	vfmadd132ss	%xmm0, %xmm3, %xmm4
	vmovss	-24(%rdx,%r8,4), %xmm7
	vmovss	-20(%rdx,%r8,4), %xmm8
	vfmadd231ss	%xmm1, %xmm7, %xmm4
	vfmadd231ss	%xmm2, %xmm8, %xmm4
	vmovss	-16(%rdx,%r8,4), %xmm6
	vfmadd231ss	%xmm1, %xmm6, %xmm4
	vmovss	-12(%rdx,%r8,4), %xmm5
	vfmadd231ss	%xmm0, %xmm5, %xmm4
	vmovss	%xmm4, -20(%rsi,%r8,4)
	vfmadd213ss	%xmm3, %xmm0, %xmm7
	vfmadd231ss	%xmm1, %xmm8, %xmm7
	vfmadd231ss	%xmm2, %xmm6, %xmm7
	vfmadd231ss	%xmm1, %xmm5, %xmm7
	vmovss	-8(%rdx,%r8,4), %xmm4
	vfmadd231ss	%xmm0, %xmm4, %xmm7
	vmovss	%xmm7, -16(%rsi,%r8,4)
	vfmadd213ss	%xmm3, %xmm0, %xmm8
	vfmadd231ss	%xmm1, %xmm6, %xmm8
	vfmadd231ss	%xmm2, %xmm5, %xmm8
	vfmadd231ss	%xmm1, %xmm4, %xmm8
	vmovss	-4(%rdx,%r8,4), %xmm7
	vfmadd231ss	%xmm0, %xmm7, %xmm8
	vmovss	%xmm8, -12(%rsi,%r8,4)
	vfmadd213ss	%xmm3, %xmm0, %xmm6
	vfmadd231ss	%xmm1, %xmm5, %xmm6
	vfmadd231ss	%xmm2, %xmm4, %xmm6
	vfmadd231ss	%xmm1, %xmm7, %xmm6
	vmovss	(%rdx,%r8,4), %xmm8
	vfmadd231ss	%xmm0, %xmm8, %xmm6
	vmovss	%xmm6, -8(%rsi,%r8,4)
	leal	10(%r8), %r11d
	vfmadd213ss	%xmm3, %xmm0, %xmm5
	vfmadd231ss	%xmm1, %xmm4, %xmm5
	vfmadd231ss	%xmm2, %xmm7, %xmm5
	vfmadd231ss	%xmm1, %xmm8, %xmm5
	cmpl	$3837, %r11d
	movl	$3837, %r15d
	cmovll	%r11d, %r15d
	addl	%r9d, %r15d
	vmovss	(%r12,%r15,4), %xmm6
	vfmadd231ss	%xmm0, %xmm6, %xmm5
	vmovss	%xmm5, -4(%rsi,%r8,4)
	vfmadd213ss	%xmm3, %xmm0, %xmm4
	vfmadd231ss	%xmm7, %xmm1, %xmm4
	vfmadd231ss	%xmm8, %xmm2, %xmm4
	vfmadd231ss	%xmm6, %xmm1, %xmm4
	cmpl	$3836, %r11d
	cmovgel	%edi, %r11d
	addl	%r10d, %r11d
	vfmadd231ss	(%r12,%r11,4), %xmm0, %xmm4
	vmovss	%xmm4, (%rsi,%r8,4)
	addq	$6, %r8
	cmpq	$3834, %r8
	jne	-308
	addq	$2, %r13
	addq	$30720, %rax
	addq	$30720, %rcx
	addq	$30720, %rdx
	addq	$30720, %rsi
	cmpq	$3840, %r13
	jne	-1264
	movq	8(%rsp), %r8
	addq	$20, %r8
	addq	$20, %rbx
	movl	$2, %eax
	movl	$1, %ecx
	movl	$4294967295, %edx
	movl	$4294967294, %esi
	xorl	%edi, %edi
	vxorps	%xmm3, %xmm3, %xmm3
	movq	%r8, 8(%rsp)
	nopl	(%rax)
	cmpq	$3839, %rax
	movl	$3839, %r9d
	cmovbq	%rax, %r9
	imulq	$15360, %r9, %r9
	movq	8(%rsp), %r13
	addq	%r13, %r9
	cmpq	$3839, %rcx
	movl	$3839, %r10d
	cmovbq	%rcx, %r10
	imulq	$15360, %r10, %r10
	addq	%r13, %r10
	movl	%edx, %r11d
	sarl	$31, %r11d
	andnl	%edx, %r11d, %r11d
	imulq	$15360, %r11, %r11
	addq	%r13, %r11
	movl	%esi, %ebp
	sarl	$31, %ebp
	andnl	%esi, %ebp, %r15d
	imulq	$15360, %r15, %r12
	addq	%r13, %r12
	xorl	%r13d, %r13d
	nopw	(%rax,%rax)
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
	movabsq	$56, %rax
	movl	$58982400, %esi
	movq	%r14, %rdi
	movq	16(%rsp), %rcx
	addq	$24, %rsp
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
