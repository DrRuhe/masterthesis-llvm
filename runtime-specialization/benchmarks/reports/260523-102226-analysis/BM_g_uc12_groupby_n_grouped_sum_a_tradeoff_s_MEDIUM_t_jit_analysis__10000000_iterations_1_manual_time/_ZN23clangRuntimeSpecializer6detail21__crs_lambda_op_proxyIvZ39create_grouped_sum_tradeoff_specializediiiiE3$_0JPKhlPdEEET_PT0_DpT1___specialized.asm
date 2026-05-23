<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ39create_grouped_sum_tradeoff_specializediiiiE3$_0JPKhlPdEEET_PT0_DpT1_>:
	testq	%rsi, %rsi
	jle	489
	movl	%esi, %eax
	andl	$3, %eax
	cmpq	$4, %rsi
	jae	7
	xorl	%ecx, %ecx
	jmp	405
	addq	$-4, %rsi
	cmpq	$4, %rsi
	jae	7
	xorl	%ecx, %ecx
	jmp	268
	movq	%rsi, %r8
	shrq	$2, %r8
	incq	%r8
	andq	$-2, %r8
	leaq	176(%rdi), %r9
	xorl	%ecx, %ecx
	movl	$1023, %r10d
	nopl	(%rax)
	movl	-176(%r9), %r11d
	andl	%r10d, %r11d
	vmovsd	-168(%r9), %xmm0
	vaddsd	(%rdx,%r11,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rdx,%r11,8)
	movl	-152(%r9), %r11d
	andl	%r10d, %r11d
	vmovsd	-144(%r9), %xmm0
	vaddsd	(%rdx,%r11,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rdx,%r11,8)
	movl	-128(%r9), %r11d
	andl	%r10d, %r11d
	vmovsd	-120(%r9), %xmm0
	vaddsd	(%rdx,%r11,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rdx,%r11,8)
	movl	-104(%r9), %r11d
	andl	%r10d, %r11d
	vmovsd	-96(%r9), %xmm0
	vaddsd	(%rdx,%r11,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rdx,%r11,8)
	movl	-80(%r9), %r11d
	andl	%r10d, %r11d
	vmovsd	-72(%r9), %xmm0
	vaddsd	(%rdx,%r11,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rdx,%r11,8)
	movl	-56(%r9), %r11d
	andl	%r10d, %r11d
	vmovsd	-48(%r9), %xmm0
	vaddsd	(%rdx,%r11,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rdx,%r11,8)
	movl	-32(%r9), %r11d
	andl	%r10d, %r11d
	vmovsd	-24(%r9), %xmm0
	vaddsd	(%rdx,%r11,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rdx,%r11,8)
	movl	-8(%r9), %r11d
	andl	%r10d, %r11d
	vmovsd	(%r9), %xmm0
	vaddsd	(%rdx,%r11,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rdx,%r11,8)
	addq	$8, %rcx
	addq	$192, %r9
	addq	$-2, %r8
	jne	-232
	testb	$4, %sil
	jne	114
	leaq	(%rcx,%rcx,2), %rsi
	movl	$1023, %r8d
	movl	(%rdi,%rsi,8), %r9d
	andl	%r8d, %r9d
	vmovsd	8(%rdi,%rsi,8), %xmm0
	vaddsd	(%rdx,%r9,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rdx,%r9,8)
	movl	24(%rdi,%rsi,8), %r9d
	andl	%r8d, %r9d
	vmovsd	32(%rdi,%rsi,8), %xmm0
	vaddsd	(%rdx,%r9,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rdx,%r9,8)
	movl	48(%rdi,%rsi,8), %r9d
	andl	%r8d, %r9d
	vmovsd	56(%rdi,%rsi,8), %xmm0
	vaddsd	(%rdx,%r9,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rdx,%r9,8)
	andl	72(%rdi,%rsi,8), %r8d
	vmovsd	80(%rdi,%rsi,8), %xmm0
	vaddsd	(%rdx,%r8,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rdx,%r8,8)
	addq	$4, %rcx
	testq	%rax, %rax
	je	61
	leaq	(%rcx,%rcx,2), %rcx
	leaq	(%rdi,%rcx,8), %rcx
	addq	$8, %rcx
	shll	$3, %eax
	leaq	(%rax,%rax,2), %rax
	xorl	%esi, %esi
	movl	$1023, %edi
	nop
	movl	-8(%rcx,%rsi), %r8d
	andl	%edi, %r8d
	vmovsd	(%rcx,%rsi), %xmm0
	vaddsd	(%rdx,%r8,8), %xmm0, %xmm0
	vmovsd	%xmm0, (%rdx,%r8,8)
	addq	$24, %rsi
	cmpq	%rsi, %rax
	jne	-34
	retq
