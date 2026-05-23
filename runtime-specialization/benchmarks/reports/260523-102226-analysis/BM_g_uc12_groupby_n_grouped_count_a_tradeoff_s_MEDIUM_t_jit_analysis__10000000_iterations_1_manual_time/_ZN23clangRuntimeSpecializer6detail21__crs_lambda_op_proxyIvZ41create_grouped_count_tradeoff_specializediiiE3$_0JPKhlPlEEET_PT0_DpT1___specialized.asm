<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIvZ41create_grouped_count_tradeoff_specializediiiE3$_0JPKhlPlEEET_PT0_DpT1_>:
	testq	%rsi, %rsi
	jle	475
	movl	%esi, %eax
	andl	$7, %eax
	cmpq	$8, %rsi
	jae	7
	xorl	%ecx, %ecx
	jmp	396
	addq	$-8, %rsi
	cmpq	$8, %rsi
	jae	7
	xorl	%ecx, %ecx
	jmp	262
	movq	%rsi, %r8
	shrq	$3, %r8
	incq	%r8
	andq	$-2, %r8
	xorl	%ecx, %ecx
	movl	$1023, %r9d
	movq	%rdi, %r10
	nopw	%cs:(%rax,%rax)
	movl	(%r10), %r11d
	andl	%r9d, %r11d
	incq	(%rdx,%r11,8)
	movl	24(%r10), %r11d
	andl	%r9d, %r11d
	incq	(%rdx,%r11,8)
	movl	48(%r10), %r11d
	andl	%r9d, %r11d
	incq	(%rdx,%r11,8)
	movl	72(%r10), %r11d
	andl	%r9d, %r11d
	incq	(%rdx,%r11,8)
	movl	96(%r10), %r11d
	andl	%r9d, %r11d
	incq	(%rdx,%r11,8)
	movl	120(%r10), %r11d
	andl	%r9d, %r11d
	incq	(%rdx,%r11,8)
	movl	144(%r10), %r11d
	andl	%r9d, %r11d
	incq	(%rdx,%r11,8)
	movl	168(%r10), %r11d
	andl	%r9d, %r11d
	incq	(%rdx,%r11,8)
	movl	192(%r10), %r11d
	andl	%r9d, %r11d
	incq	(%rdx,%r11,8)
	movl	216(%r10), %r11d
	andl	%r9d, %r11d
	incq	(%rdx,%r11,8)
	movl	240(%r10), %r11d
	andl	%r9d, %r11d
	incq	(%rdx,%r11,8)
	movl	264(%r10), %r11d
	andl	%r9d, %r11d
	incq	(%rdx,%r11,8)
	movl	288(%r10), %r11d
	andl	%r9d, %r11d
	incq	(%rdx,%r11,8)
	movl	312(%r10), %r11d
	andl	%r9d, %r11d
	incq	(%rdx,%r11,8)
	movl	336(%r10), %r11d
	andl	%r9d, %r11d
	incq	(%rdx,%r11,8)
	movl	360(%r10), %r11d
	andl	%r9d, %r11d
	incq	(%rdx,%r11,8)
	addq	$16, %rcx
	addq	$384, %r10
	addq	$-2, %r8
	jne	-226
	testb	$8, %sil
	jne	111
	leaq	(%rcx,%rcx,2), %r8
	movl	$1023, %esi
	movl	(%rdi,%r8,8), %r9d
	andl	%esi, %r9d
	incq	(%rdx,%r9,8)
	movl	24(%rdi,%r8,8), %r9d
	andl	%esi, %r9d
	incq	(%rdx,%r9,8)
	movl	48(%rdi,%r8,8), %r9d
	andl	%esi, %r9d
	incq	(%rdx,%r9,8)
	movl	72(%rdi,%r8,8), %r9d
	andl	%esi, %r9d
	incq	(%rdx,%r9,8)
	movl	96(%rdi,%r8,8), %r9d
	andl	%esi, %r9d
	incq	(%rdx,%r9,8)
	movl	120(%rdi,%r8,8), %r9d
	andl	%esi, %r9d
	incq	(%rdx,%r9,8)
	movl	144(%rdi,%r8,8), %r9d
	andl	%esi, %r9d
	incq	(%rdx,%r9,8)
	andl	168(%rdi,%r8,8), %esi
	incq	(%rdx,%rsi,8)
	addq	$8, %rcx
	testq	%rax, %rax
	je	56
	leaq	(%rcx,%rcx,2), %rcx
	leaq	(%rdi,%rcx,8), %rcx
	shll	$3, %eax
	leaq	(%rax,%rax,2), %rax
	xorl	%esi, %esi
	movl	$1023, %edi
	nopw	%cs:(%rax,%rax)
	movl	(%rcx,%rsi), %r8d
	andl	%edi, %r8d
	incq	(%rdx,%r8,8)
	addq	$24, %rsi
	cmpq	%rsi, %rax
	jne	-20
	retq
