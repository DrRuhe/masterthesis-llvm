<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ37create_grouped_minmax_low_specializediiiiE3$_0JPKhlPdS5_EEET_PT0_DpT1_>:
	testq	%rsi, %rsi
	jle	870
	movl	%esi, %eax
	andl	$3, %eax
	cmpq	$4, %rsi
	jae	8
	xorl	%r8d, %r8d
	jmp	754
	addq	$-4, %rsi
	cmpq	$4, %rsi
	jae	18
	xorl	%r8d, %r8d
	testb	$4, %sil
	je	505
	jmp	726
	pushq	%rbx
	movq	%rsi, %r9
	shrq	$2, %r9
	incq	%r9
	andq	$-2, %r9
	leaq	176(%rdi), %r10
	xorl	%r8d, %r8d
	jmp	34
	nopw	%cs:(%rax,%rax)
	addq	$8, %r8
	addq	$192, %r10
	addq	$-2, %r9
	je	428
	movl	-176(%r10), %r11d
	andl	$1023, %r11d
	vmovsd	-168(%r10), %xmm0
	movl	%r11d, %ebx
	vmovsd	(%rdx,%rbx,8), %xmm1
	vucomisd	%xmm0, %xmm1
	jbe	6
	vmovsd	%xmm0, (%rdx,%r11,8)
	vucomisd	(%rcx,%r11,8), %xmm0
	jbe	6
	vmovsd	%xmm0, (%rcx,%r11,8)
	movl	-152(%r10), %r11d
	andl	$1023, %r11d
	vmovsd	-144(%r10), %xmm0
	movl	%r11d, %ebx
	vmovsd	(%rdx,%rbx,8), %xmm1
	vucomisd	%xmm0, %xmm1
	jbe	6
	vmovsd	%xmm0, (%rdx,%r11,8)
	vucomisd	(%rcx,%r11,8), %xmm0
	jbe	6
	vmovsd	%xmm0, (%rcx,%r11,8)
	movl	-128(%r10), %r11d
	andl	$1023, %r11d
	vmovsd	-120(%r10), %xmm0
	movl	%r11d, %ebx
	vmovsd	(%rdx,%rbx,8), %xmm1
	vucomisd	%xmm0, %xmm1
	jbe	6
	vmovsd	%xmm0, (%rdx,%r11,8)
	vucomisd	(%rcx,%r11,8), %xmm0
	jbe	6
	vmovsd	%xmm0, (%rcx,%r11,8)
	movl	-104(%r10), %r11d
	andl	$1023, %r11d
	vmovsd	-96(%r10), %xmm0
	movl	%r11d, %ebx
	vmovsd	(%rdx,%rbx,8), %xmm1
	vucomisd	%xmm0, %xmm1
	jbe	6
	vmovsd	%xmm0, (%rdx,%r11,8)
	vucomisd	(%rcx,%r11,8), %xmm0
	jbe	6
	vmovsd	%xmm0, (%rcx,%r11,8)
	movl	-80(%r10), %r11d
	andl	$1023, %r11d
	vmovsd	-72(%r10), %xmm0
	movl	%r11d, %ebx
	vmovsd	(%rdx,%rbx,8), %xmm1
	vucomisd	%xmm0, %xmm1
	jbe	6
	vmovsd	%xmm0, (%rdx,%r11,8)
	vucomisd	(%rcx,%r11,8), %xmm0
	jbe	6
	vmovsd	%xmm0, (%rcx,%r11,8)
	movl	-56(%r10), %r11d
	andl	$1023, %r11d
	vmovsd	-48(%r10), %xmm0
	movl	%r11d, %ebx
	vmovsd	(%rdx,%rbx,8), %xmm1
	vucomisd	%xmm0, %xmm1
	jbe	6
	vmovsd	%xmm0, (%rdx,%r11,8)
	vucomisd	(%rcx,%r11,8), %xmm0
	jbe	6
	vmovsd	%xmm0, (%rcx,%r11,8)
	movl	-32(%r10), %r11d
	andl	$1023, %r11d
	vmovsd	-24(%r10), %xmm0
	movl	%r11d, %ebx
	vmovsd	(%rdx,%rbx,8), %xmm1
	vucomisd	%xmm0, %xmm1
	jbe	6
	vmovsd	%xmm0, (%rdx,%r11,8)
	vucomisd	(%rcx,%r11,8), %xmm0
	jbe	6
	vmovsd	%xmm0, (%rcx,%r11,8)
	movl	-8(%r10), %r11d
	andl	$1023, %r11d
	vmovsd	(%r10), %xmm0
	movl	%r11d, %ebx
	vmovsd	(%rdx,%rbx,8), %xmm1
	vucomisd	%xmm0, %xmm1
	jbe	6
	vmovsd	%xmm0, (%rdx,%r11,8)
	vucomisd	(%rcx,%r11,8), %xmm0
	jbe	-438
	vmovsd	%xmm0, (%rcx,%r11,8)
	jmp	-449
	popq	%rbx
	testb	$4, %sil
	jne	226
	leaq	(,%r8,8), %rsi
	leaq	(%rsi,%rsi,2), %rsi
	movl	(%rdi,%rsi), %r9d
	andl	$1023, %r9d
	vmovsd	8(%rdi,%rsi), %xmm0
	movl	%r9d, %r10d
	vmovsd	(%rdx,%r10,8), %xmm1
	vucomisd	%xmm0, %xmm1
	jbe	6
	vmovsd	%xmm0, (%rdx,%r9,8)
	vucomisd	(%rcx,%r9,8), %xmm0
	jbe	6
	vmovsd	%xmm0, (%rcx,%r9,8)
	movl	24(%rdi,%rsi), %r9d
	andl	$1023, %r9d
	vmovsd	32(%rdi,%rsi), %xmm0
	movl	%r9d, %r10d
	vmovsd	(%rdx,%r10,8), %xmm1
	vucomisd	%xmm0, %xmm1
	jbe	6
	vmovsd	%xmm0, (%rdx,%r9,8)
	vucomisd	(%rcx,%r9,8), %xmm0
	jbe	6
	vmovsd	%xmm0, (%rcx,%r9,8)
	movl	48(%rdi,%rsi), %r9d
	andl	$1023, %r9d
	vmovsd	56(%rdi,%rsi), %xmm0
	movl	%r9d, %r10d
	vmovsd	(%rdx,%r10,8), %xmm1
	vucomisd	%xmm0, %xmm1
	jbe	6
	vmovsd	%xmm0, (%rdx,%r9,8)
	vucomisd	(%rcx,%r9,8), %xmm0
	jbe	6
	vmovsd	%xmm0, (%rcx,%r9,8)
	movl	72(%rdi,%rsi), %r9d
	andl	$1023, %r9d
	vmovsd	80(%rdi,%rsi), %xmm0
	movl	%r9d, %esi
	vmovsd	(%rdx,%rsi,8), %xmm1
	vucomisd	%xmm0, %xmm1
	jbe	6
	vmovsd	%xmm0, (%rdx,%r9,8)
	vucomisd	(%rcx,%r9,8), %xmm0
	jbe	6
	vmovsd	%xmm0, (%rcx,%r9,8)
	addq	$4, %r8
	testq	%rax, %rax
	je	92
	leaq	(%r8,%r8,2), %rsi
	leaq	(%rdi,%rsi,8), %rsi
	addq	$8, %rsi
	shll	$3, %eax
	leaq	(%rax,%rax,2), %rax
	xorl	%edi, %edi
	jmp	15
	nopw	(%rax,%rax)
	addq	$24, %rdi
	cmpq	%rdi, %rax
	je	54
	movl	-8(%rsi,%rdi), %r8d
	andl	$1023, %r8d
	vmovsd	(%rsi,%rdi), %xmm0
	movl	%r8d, %r9d
	vmovsd	(%rdx,%r9,8), %xmm1
	vucomisd	%xmm0, %xmm1
	jbe	6
	vmovsd	%xmm0, (%rdx,%r8,8)
	vucomisd	(%rcx,%r8,8), %xmm0
	jbe	-55
	vmovsd	%xmm0, (%rcx,%r8,8)
	jmp	-63
	retq
