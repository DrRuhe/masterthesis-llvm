<_ZN23clangRuntimeSpecializer6detail21__crs_lambda_op_proxyIlZ39create_email_match_tradeoff_specializedvE3$_0JPKclEEET_PT0_DpT1_>:
	xorl	%eax, %eax
	testq	%rsi, %rsi
	jle	580
	pushq	%rbp
	pushq	%rbx
	movabsq	$94266015108672, %rcx
	movl	%esi, %edx
	andl	$7, %edx
	cmpq	$8, %rsi
	jae	10
	xorl	%r10d, %r10d
	xorl	%esi, %esi
	jmp	445
	movabsq	$9223372036854775800, %rax
	andq	%rax, %rsi
	xorl	%r8d, %r8d
	xorl	%eax, %eax
	xorl	%r10d, %r10d
	xorl	%r9d, %r9d
	nopw	%cs:(%rax,%rax)
	movzbl	(%rdi,%r9), %r11d
	testb	%r11b, %r11b
	cmovlel	%r8d, %r11d
	movzbl	%r11b, %r11d
	movl	%eax, %ebx
	shll	$7, %ebx
	orl	%r11d, %ebx
	movslq	%ebx, %r11
	movl	(%rcx,%r11,4), %r11d
	cmpl	$6, %eax
	setne	%al
	cmpl	$6, %r11d
	sete	%bl
	setne	%bpl
	andb	%al, %bl
	movzbl	%bl, %eax
	addq	%r10, %rax
	movzbl	1(%rdi,%r9), %r10d
	testb	%r10b, %r10b
	cmovlel	%r8d, %r10d
	movzbl	%r10b, %r10d
	shll	$7, %r11d
	orl	%r10d, %r11d
	movslq	%r11d, %r10
	movl	(%rcx,%r10,4), %r10d
	cmpl	$6, %r10d
	sete	%r11b
	setne	%bl
	andb	%bpl, %r11b
	movzbl	%r11b, %r11d
	addq	%rax, %r11
	movzbl	2(%rdi,%r9), %eax
	testb	%al, %al
	cmovlel	%r8d, %eax
	movzbl	%al, %eax
	shll	$7, %r10d
	orl	%eax, %r10d
	movslq	%r10d, %rax
	movl	(%rcx,%rax,4), %eax
	cmpl	$6, %eax
	sete	%r10b
	setne	%bpl
	andb	%bl, %r10b
	movzbl	%r10b, %r10d
	addq	%r11, %r10
	movzbl	3(%rdi,%r9), %r11d
	testb	%r11b, %r11b
	cmovlel	%r8d, %r11d
	movzbl	%r11b, %r11d
	shll	$7, %eax
	orl	%r11d, %eax
	cltq
	movl	(%rcx,%rax,4), %eax
	cmpl	$6, %eax
	sete	%r11b
	setne	%bl
	andb	%bpl, %r11b
	movzbl	%r11b, %r11d
	addq	%r10, %r11
	movzbl	4(%rdi,%r9), %r10d
	testb	%r10b, %r10b
	cmovlel	%r8d, %r10d
	movzbl	%r10b, %r10d
	shll	$7, %eax
	orl	%r10d, %eax
	cltq
	movl	(%rcx,%rax,4), %eax
	cmpl	$6, %eax
	sete	%r10b
	setne	%bpl
	andb	%bl, %r10b
	movzbl	%r10b, %r10d
	addq	%r11, %r10
	movzbl	5(%rdi,%r9), %r11d
	testb	%r11b, %r11b
	cmovlel	%r8d, %r11d
	movzbl	%r11b, %r11d
	shll	$7, %eax
	orl	%r11d, %eax
	cltq
	movl	(%rcx,%rax,4), %eax
	cmpl	$6, %eax
	sete	%r11b
	setne	%bl
	andb	%bpl, %r11b
	movzbl	%r11b, %r11d
	addq	%r10, %r11
	movzbl	6(%rdi,%r9), %r10d
	testb	%r10b, %r10b
	cmovlel	%r8d, %r10d
	movzbl	%r10b, %r10d
	shll	$7, %eax
	orl	%r10d, %eax
	cltq
	movl	(%rcx,%rax,4), %eax
	cmpl	$6, %eax
	sete	%r10b
	setne	%bpl
	andb	%bl, %r10b
	movzbl	%r10b, %ebx
	addq	%r11, %rbx
	movzbl	7(%rdi,%r9), %r10d
	testb	%r10b, %r10b
	cmovlel	%r8d, %r10d
	movzbl	%r10b, %r10d
	shll	$7, %eax
	orl	%r10d, %eax
	cltq
	movl	(%rcx,%rax,4), %eax
	cmpl	$6, %eax
	sete	%r11b
	andb	%r11b, %bpl
	movzbl	%bpl, %r10d
	addq	%rbx, %r10
	addq	$8, %r9
	cmpq	%r9, %rsi
	jne	-409
	testq	%rdx, %rdx
	je	85
	addq	%rsi, %rdi
	xorl	%esi, %esi
	xorl	%r8d, %r8d
	nopw	%cs:(%rax,%rax)
	movzbl	(%rdi,%r8), %r9d
	testb	%r9b, %r9b
	cmovlel	%esi, %r9d
	movzbl	%r9b, %r9d
	movl	%eax, %r11d
	shll	$7, %r11d
	orl	%r9d, %r11d
	movslq	%r11d, %r9
	movl	(%rcx,%r9,4), %r9d
	cmpl	$6, %r9d
	sete	%r11b
	cmpl	$6, %eax
	setne	%al
	andb	%r11b, %al
	movzbl	%al, %eax
	addq	%rax, %r10
	incq	%r8
	movl	%r9d, %eax
	cmpq	%r8, %rdx
	jne	-67
	movzbl	%r11b, %eax
	andl	$1, %eax
	addq	%r10, %rax
	popq	%rbx
	popq	%rbp
	retq
